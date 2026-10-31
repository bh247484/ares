/**
 * Monitors the SNES CPU <-> SMP communication protocol across Port 0 ($2140 / $00F4).
 *
 * SNES Hardware Context:
 * Communication between the main 65816 CPU and the SPC700 audio processor occurs through
 * four 8-bit memory-mapped I/O ports ($2140-$2143 on CPU, mirrored as $00F4-$00F7 on SMP).
 * Standard Nintendo sound drivers (N-SPC) use Port 0 as a command/handshake counter:
 * - A byte of $FF signals the start of an audio data upload/reset sequence.
 * - Sequential incrementing counter values on Port 0 indicate streaming audio data payload
 *   delivered over Port 1 ($2141). We stream these bytes into a rolling CRC-32 hash.
 * - Non-sequential values represent control commands, ready handshakes ($CC/$00), or
 *   the end of an upload block.
 *
 * Once an upload finishes, the accumulated CRC-32 produces a stable fingerprint
 * ("scene hash") of the resident audio bank, distinguishing song/cue transitions.
 */
auto SMP::detectScene(n2 port, n8 data) -> void {
  // Only Port 0 carries control / handshake signals
  if (port != 0) return;

  // CASE 1: RESET / START LOAD ($FF)
  if (data == 0xFF) {
    sceneDetector.isUploading = true;
    sceneDetector.stagingHash = 0xFFFFFFFF; // Reset CRC
    sceneDetector.lastPort0 = 0xFF;
    return;
  }

  // CASE 2: SEQUENTIAL UPLOAD
  // Detects incrementing counters (including FF->00 wrap)
  if (data == (n8)(sceneDetector.lastPort0 + 1)) {
    if (sceneDetector.isUploading) {
      sceneDetector.stagingHash = crc32_update(sceneDetector.stagingHash, io.apu1);
    }
    sceneDetector.lastPort0 = data;
    return;
  }

  // CASE 3: NON-SEQUENTIAL (Commands or Handshakes)
  if (data != sceneDetector.lastPort0) {
    // Handshake Filter:
    // If we are "uploading" but have 0 data bytes (stagingHash == 0xFFFFFFFF), this is protocol noise (e.g. $CC ready signal).
    if (sceneDetector.isUploading && sceneDetector.stagingHash == 0xFFFFFFFF) {
      if (data == 0x00) {
        sceneDetector.stagingHash = crc32_update(sceneDetector.stagingHash, io.apu1);
      }
      sceneDetector.lastPort0 = data;
      return;
    }

    sceneDetector.lastPort0 = data;

    // A. END OF UPLOAD: Resolve the Bank Hash
    if (sceneDetector.isUploading) {
      if (sceneDetector.stagingHash != 0xFFFFFFFF) {
        sceneDetector.activeBankHash = sceneDetector.stagingHash;
        if (LOG_SCENES) printf("[MEM] NEW BANK INSTALLED: %08X\n", sceneDetector.activeBankHash);
      } else {
        if (LOG_SCENES) printf("[MEM] RESIDENT BANK PRESERVED: %08X\n", sceneDetector.activeBankHash);
      }
      sceneDetector.isUploading = false;
      onBankChange(sceneDetector.activeBankHash);
    }

    // B. FILTER ACKS: Ignore '00' acknowledgments during normal playback
    if (data == 0x00) return;

    // C. LOG SCENE SIGNATURE
    if (LOG_SCENES) {
      printf(">>> SCENE: [%08X] - CMD:%02X [P1:%02X P2:%02X P3:%02X]\n",
             sceneDetector.activeBankHash, (unsigned)data, (unsigned)io.apu1, (unsigned)io.apu2, (unsigned)io.apu3);
    }
  }
}

/**
 * Reads voice parameters from DSP register space for a given voice channel (0-7).
 *
 * Channel registers are arranged in contiguous 16-byte ($10) blocks:
 *   Voice 0 starts at $00, Voice 1 at $10 ... Voice 7 at $70 (`voiceOffset = voiceIndex * 0x10`).
 *
 * Parameters within each voice's 16-byte block:
 *   +$00: VOL_L,  +$01: VOL_R
 *   +$02: P_L,    +$03: P_H   (14-bit pitch divider)
 *   +$04: SRC                 (BRR sample directory index / instrument ID)
 *   +$05: ADSR1,  +$06: ADSR2, +$07: GAIN
 *
 * Global registers like $4D (EON) mirror the voice index via matching bit positions (1 << voiceIndex).
 */
auto SMP::inspectVoice(uint8_t voiceIndex) -> VoiceInfo {
  uint8_t voiceOffset = voiceIndex * 0x10;
  uint8_t echoFlags = dsp.read(DSPReg::EON);

  VoiceInfo info;
  info.index    = voiceIndex;
  info.sampleID = dsp.read(voiceOffset + DSPReg::SRC);
  info.pitch    = dsp.read(voiceOffset + DSPReg::P_L) | (dsp.read(voiceOffset + DSPReg::P_H) << 8);
  info.adsr1    = dsp.read(voiceOffset + DSPReg::ADSR1);
  info.adsr2    = dsp.read(voiceOffset + DSPReg::ADSR2);
  info.gain     = dsp.read(voiceOffset + DSPReg::GAIN);
  info.echo     = (echoFlags & (1 << voiceIndex)) != 0;
  return info;
}

/**
 * Intercepts writes to DSP Key-On ($4C / DSP_KON).
 *
 * The incoming byte is an 8-bit mask where each bit represents a voice channel (0-7).
 * We scan the mask using Ares's `data.bit(v)` helper to inspect and dispatch callbacks
 * for each keyed-on voice.
 */
auto SMP::interceptVoiceKeyOn(n8 data) -> void {
  for (int v = 0; v < 8; v++) {
    // In Ares, `n8` provides `.bit(v)` which tests if bit `v` is set (equivalent to `data & (1 << v)`)
    if (data.bit(v)) {
      VoiceInfo voice = inspectVoice(v);
      if (blockedSamples[voice.sampleID]) continue;

      if (LOG_SFX) {
        printf(">>> SFX: [ID:%02X] [Pitch:%04X] [ADSR:%02X%02X] [Gain:%02X] [Echo:%d]\n",
               voice.sampleID, voice.pitch, voice.adsr1, voice.adsr2, voice.gain, voice.echo);
      }

      onVoiceTrigger(voice);
    }
  }
}

auto SMP::onBankChange(uint32_t bankHash) -> void {
  // Hook for MittelVec MusicCue triggering
}

auto SMP::onVoiceTrigger(const VoiceInfo& voice) -> void {
  // Hook for MittelVec SFX triggering
}
