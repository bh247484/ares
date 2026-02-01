//Sony CXP1100Q-1
#include <array>

struct SMP : SPC700, Thread {
  Node::Object node;

  struct Debugger {
    //debugger.cpp
    auto load(Node::Object) -> void;
    auto instruction() -> void;

    struct Tracer {
      Node::Debugger::Tracer::Instruction instruction;
    } tracer;
  } debugger;

  auto synchronizing() const -> bool override { return scheduler.synchronizing(); }

  //smp.cpp
  auto load(Node::Object) -> void;
  auto unload() -> void;

  auto main() -> void;
  auto power(bool reset) -> void;

  //io.cpp
  auto portRead(n2 port) const -> n8;
  auto portWrite(n2 port, n8 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  n8 iplrom[64];

private:
  struct IO {
    //timing
    u32 clockCounter = 0;
    u32 dspCounter = 0;

    //external
    n8 apu0;
    n8 apu1;
    n8 apu2;
    n8 apu3;

    //$00f0
    n1 timersDisable;
    n1 ramWritable = true;
    n1 ramDisable;
    n1 timersEnable = true;
    n2 externalWaitStates;
    n2 internalWaitStates;

    //$00f1
    n1 iplromEnable = true;

    //$00f2
    n8 dspAddress;

    //$00f4-00f7
    n8 cpu0;
    n8 cpu1;
    n8 cpu2;
    n8 cpu3;

    //$00f8-00f9
    n8 aux4;
    n8 aux5;
  } io;

  uint32_t activeBankHash = 0;   // The hash of the music currently residing in RAM
  uint32_t stagingHash = 0;      // The hash we are currently building during a load
  bool isUploading = false;      // Are we in the middle of a data stream?
  uint8_t lastPort0;             // Used to detect sequential data, i.e. (lastPort0 + 1).

  auto crc32_update(uint32_t crc, uint8_t data) -> uint32_t {
    crc = crc ^ data;
    for (int j = 0; j < 8; j++) {
      if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
      else crc >>= 1;
    }
    return crc;
  }

  // Constants for DSP Registers
  static const uint8_t DSP_KON   = 0x4C; // Key On
  static const uint8_t DSP_EON   = 0x4D; // Echo Enable
  static const uint8_t DSP_VOL_L = 0x00; // Volume Left
  static const uint8_t DSP_VOL_R = 0x01; // Volume Right
  static const uint8_t DSP_P_L   = 0x02; // Pitch Low
  static const uint8_t DSP_P_H   = 0x03; // Pitch High
  static const uint8_t DSP_SRC   = 0x04; // Source Number
  static const uint8_t DSP_ADSR1 = 0x05; // ADSR 1
  static const uint8_t DSP_ADSR2 = 0x06; // ADSR 2
  static const uint8_t DSP_GAIN  = 0x07; // Gain


static inline std::array<bool, 256> blockedSamples = [] {
    std::array<bool, 256> b{}; // Initializes all to false
    
    // Add IDs you want to silence here
    b[0x03] = true;
    b[0x07] = true;
    b[0x10] = true;
    b[0x12] = true;
    b[0x14] = true;
    b[0x22] = true;
    b[0x0A] = true;
    b[0x1C] = true;
    
    return b;
}();

  //memory.cpp
  auto readRAM(n16 address) -> n8;
  auto writeRAM(n16 address, n8 data) -> void;

  auto idle() -> void override;
  auto read(n16 address) -> n8 override;
  auto write(n16 address, n8 data) -> void override;

  auto readDisassembler(n16 address) -> n8 override;

  //io.cpp
  auto readIO(n16 address) -> n8;
  auto writeIO(n16 address, n8 data) -> void;

  template<u32 Frequency>
  struct Timer {
    n8 stage0;
    n8 stage1;
    n8 stage2;
    n4 stage3;
    b1 line;
    b1 enable;
    n8 target;

    //timing.cpp
    auto step(u32 clocks) -> void;
    auto synchronizeStage1() -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;
  };

  Timer<128> timer0;
  Timer<128> timer1;
  Timer< 16> timer2;

  //timing.cpp
  auto wait(bool halve, maybe<n16> address = nothing) -> void;
  auto step(u32 clocks) -> void;
  auto stepTimers(u32 clocks) -> void;
};

extern SMP smp;
