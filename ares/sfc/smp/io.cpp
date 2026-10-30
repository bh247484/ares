auto SMP::portRead(n2 port) const -> n8 {
  if(port == 0) return io.cpu0;
  if(port == 1) return io.cpu1;
  if(port == 2) return io.cpu2;
  if(port == 3) return io.cpu3;
  unreachable;
}

auto SMP::portWrite(n2 port, n8 data) -> void {
  // 1. Update hardware state (Always do this first)
  if(port == 0) io.apu0 = data;
  if(port == 1) io.apu1 = data;
  if(port == 2) io.apu2 = data;
  if(port == 3) io.apu3 = data;

  // 2. Filter: We only care about control signals on Port 0
  if (port != 0) return;

  // --- N-SPC PROTOCOL MONITOR ---

  // CASE 1: RESET / START LOAD ($FF)
  if (data == 0xFF) {
       isUploading = true;
       stagingHash = 0xFFFFFFFF; // Reset CRC
       lastPort0 = 0xFF;
       return;
  }

  // CASE 2: SEQUENTIAL UPLOAD
  // Detects incrementing counters (including FF->00 wrap)
  if (data == (n8)(lastPort0 + 1)) {
      if (isUploading) {
        stagingHash = crc32_update(stagingHash, io.apu1);
      }
      lastPort0 = data;
      return; 
  }

  // CASE 3: NON-SEQUENTIAL (Commands or Handshakes)
  if (data != lastPort0) {
      
      // Handshake Filter:
      // If we are "uploading" but have 0 data bytes (when stagingHash == 0xFFFFFFFF), this is protocol noise (e.g., $CC ready signal).
      // In short, we may have initiated the upload but commands that are not actually data could be sent first.
      if (isUploading && stagingHash == 0xFFFFFFFF) {
        // If the noise is $00, that is actually the first valid data byte (byte 0).
        // And lastPort0 could be $FF (Reset) or $CC (Handshake) and neither $FF + 1 nor $CC + 1 is equal to 0x00.
        // So neither would read sequential data. Everything thereafter (meaning after 0x00) should be sequential, however.
          if (data == 0x00) {
            stagingHash = crc32_update(stagingHash, io.apu1);
          }
          lastPort0 = data;
          return;
      }
      
      lastPort0 = data;
      
      // A. END OF UPLOAD: Resolve the Bank Hash
      if (isUploading) {
          if (stagingHash != 0xFFFFFFFF) {
              activeBankHash = stagingHash;
              if (LOG_SCENES) printf("[MEM] NEW BANK INSTALLED: %08X\n", activeBankHash);
          } else {
              // Valid reset occurred, but no data was sent. Bank remains unchanged.
              if (LOG_SCENES) printf("[MEM] RESIDENT BANK PRESERVED: %08X\n", activeBankHash);
          }
          isUploading = false; 
      }

      // B. FILTER ACKS: Ignore '00' acknowledgments during normal playback
      if (data == 0x00) return;

      // C. LOG SCENE SIGNATURE
      // This is the "Fingerprint" used to map specific songs/scenes
      if (LOG_SCENES) printf(">>> SCENE: [%08X] - CMD:%02X [P1:%02X P2:%02X P3:%02X]\n", 
             activeBankHash, data, io.apu1, io.apu2, io.apu3);
  }
}

inline auto SMP::readIO(n16 address) -> n8 {
  n8 data;

  switch(address) {
  case 0xf0:  //TEST (write-only register)
    return 0x00;

  case 0xf1:  //CONTROL (write-only register)
    return 0x00;

  case 0xf2:  //DSPADDR
    return io.dspAddress;

  case 0xf3:  //DSPDATA
    //0x80-0xff are read-only mirrors of 0x00-0x7f
    return dsp.read(io.dspAddress);

  case 0xf4:  //CPUIO0
    synchronize(cpu);
    return io.apu0;

  case 0xf5:  //CPUIO1
    synchronize(cpu);
    return io.apu1;

  case 0xf6:  //CPUIO2
    synchronize(cpu);
    return io.apu2;

  case 0xf7:  //CPUIO3
    synchronize(cpu);
    return io.apu3;

  case 0xf8:  //AUXIO4
    return io.aux4;

  case 0xf9:  //AUXIO5
    return io.aux5;

  case 0xfa:  //T0TARGET
  case 0xfb:  //T1TARGET
  case 0xfc:  //T2TARGET (write-only registers)
    return 0x00;

  case 0xfd:  //T0OUT (4-bit counter value)
    data = timer0.stage3;
    timer0.stage3 = 0;
    return data;

  case 0xfe:  //T1OUT (4-bit counter value)
    data = timer1.stage3;
    timer1.stage3 = 0;
    return data;

  case 0xff:  //T2OUT (4-bit counter value)
    data = timer2.stage3;
    timer2.stage3 = 0;
    return data;
  }

  return data;  //unreachable
}

inline auto SMP::writeIO(n16 address, n8 data) -> void {
  switch(address) {
  case 0xf0:  //TEST
    if(r.p.p) break;  //writes only valid when P flag is clear

    io.timersDisable      = data.bit(0);
    io.ramWritable        = data.bit(1);
    io.ramDisable         = data.bit(2);
    io.timersEnable       = data.bit(3);
    io.externalWaitStates = data.bit(4,5);
    io.internalWaitStates = data.bit(6,7);

    timer0.synchronizeStage1();
    timer1.synchronizeStage1();
    timer2.synchronizeStage1();
    break;

  case 0xf1:  //CONTROL
    //0->1 transistion resets timers
    if(timer0.enable.raise(data.bit(0))) {
      timer0.stage2 = 0;
      timer0.stage3 = 0;
    }

    if(timer1.enable.raise(data.bit(1))) {
      timer1.stage2 = 0;
      timer1.stage3 = 0;
    }

    if(!timer2.enable.raise(data.bit(2))) {
      timer2.stage2 = 0;
      timer2.stage3 = 0;
    }

    if(data.bit(4)) {
      synchronize(cpu);
      io.apu0 = 0x00;
      io.apu1 = 0x00;
    }

    if(data.bit(5)) {
      synchronize(cpu);
      io.apu2 = 0x00;
      io.apu3 = 0x00;
    }

    io.iplromEnable = data.bit(7);
    break;

  case 0xf2:  //DSPADDR
    io.dspAddress = data;
    break;

  case 0xf3: // DSPDATA
  {
    if(io.dspAddress.bit(7)) break; // Read-only mirrors

    // --- SFX INTERCEPTOR ---
    if (io.dspAddress == DSP_KON) {
      // Iterate through all 8 voices to see which ones are being keyed ON
      for (int v = 0; v < 8; v++) {
        // Check if this specific voice bit is set in the data being written
        if (data & (1 << v)) {
          // Base Register Offset for this voice (Voice * 0x10)
          uint8_t voiceOffset = v * 0x10;


          // Identify the Instrument (Source Number)
          // Register math: (Voice * 0x10) + 0x04
          uint8_t sampleID = dsp.read(voiceOffset + DSP_SRC);

          // Filter Check: Skip if in blocklist
          // Used to denoise console logs when manually identifying sfx in real time during gameplay.
          if (blockedSamples[sampleID]) continue;

          // 2. Identify the Context (Echo Enable)
          // We check the Echo Enable (EON) bit for this specific voice
          uint8_t echoFlags = dsp.read(DSP_EON);
          bool isEchoOn = echoFlags & (1 << v);

          // PITCH: The 14-bit pitch value. 
          uint8_t pitchL = dsp.read(voiceOffset + DSP_P_L);
          uint8_t pitchH = dsp.read(voiceOffset + DSP_P_H);
          uint16_t pitch = pitchL | (pitchH << 8);

          // ENVELOPE: ADSR and GAIN
          uint8_t adsr1 = dsp.read(voiceOffset + DSP_ADSR1);
          uint8_t adsr2 = dsp.read(voiceOffset + DSP_ADSR2);
          uint8_t gain  = dsp.read(voiceOffset + DSP_GAIN);
          
          // Envelope Mode Heuristic:
          // If ADSR1 bit 7 is 0, the channel is using GAIN (Direct Volume) mode.
          bool useADSR = adsr1 & 0x80;

          if (LOG_SFX) printf(">>> SFX: [ID:%02X] [Pitch:%04X] [ADSR:%02X%02X] [Gain:%02X] [Echo:%d]\n", 
               sampleID, pitch, adsr1, adsr2, gain, isEchoOn);
        }
      }
    }
    // -----------------------

    dsp.write(io.dspAddress, data);
    break;
  }

  case 0xf4:  //CPUIO0
    synchronize(cpu);
    io.cpu0 = data;
    break;

  case 0xf5:  //CPUIO1
    synchronize(cpu);
    io.cpu1 = data;
    break;

  case 0xf6:  //CPUIO2
    synchronize(cpu);
    io.cpu2 = data;
    break;

  case 0xf7:  //CPUIO3
    synchronize(cpu);
    io.cpu3 = data;
    break;

  case 0xf8:  //AUXIO4
    io.aux4 = data;
    break;

  case 0xf9:  //AUXIO5
    io.aux5 = data;
    break;

  case 0xfa:  //T0TARGET
    timer0.target = data;
    break;

  case 0xfb:  //T1TARGET
    timer1.target = data;
    break;

  case 0xfc:  //T2TARGET
    timer2.target = data;
    break;

  case 0xfd:  //T0OUT
  case 0xfe:  //T1OUT
  case 0xff:  //T2OUT (read-only registers)
    break;
  }
}
