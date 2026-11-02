#define MITTELVEC_IMPLEMENTATION
#include "mittelvec/mittelvec.h"

#include <sfc/sfc.hpp>

const int BUFFER_SIZE = 512;
const int NUM_CHANNELS = 1;
const float SAMPLE_RATE = 44100.0f;

#include "mittelvec/game-maps.h"

namespace ares::SuperFamicom {

SMP smp;

static inline auto resolveResourcePath(string relDevPath, string resourceName, bool isDirectory) -> string {
  if (isDirectory) {
    if (directory::exists({Path::program(), resourceName})) return {Path::program(), resourceName};
    if (directory::exists({Path::userData(), "ares/", resourceName})) return {Path::userData(), "ares/", resourceName};
  } else {
    if (file::exists({Path::program(), resourceName})) return {Path::program(), resourceName};
    if (file::exists({Path::userData(), "ares/", resourceName})) return {Path::userData(), "ares/", resourceName};
  }
  return relDevPath;
}

SMP::SMP() :
  mittelVecContext{ BUFFER_SIZE, NUM_CHANNELS, SAMPLE_RATE },
  engine(mittelVecContext),
  graph(engine.graph)
{
  engine.start();

  string sfxPath = resolveResourcePath("ares/sfc/smp/mittelvec/sfx/", "sfx/", true);
  string musicPath = resolveResourcePath("ares/sfc/smp/mittelvec/music-cues/", "music-cues/", true);
  debugConfigPath = resolveResourcePath("ares/sfc/smp/debug-config.bml", "debug-config.bml", false);

  samplePack = std::make_unique<MittelVec::SamplePack>(graph, metroidSamplePack, (std::string)sfxPath);
  musicOrchestrator = std::make_unique<MittelVec::MusicCueOrchestrator>(graph, metroidMusicCues, (std::string)musicPath);

  loadDebugConfig();
}

SMP::~SMP() {
  printf("smp deinitialized\n");
  engine.stop();
}

#include "memory.cpp"
#include "io.cpp"
#include "interceptor.cpp"
#include "timing.cpp"
#include "debugger.cpp"
#include "serialization.cpp"

auto SMP::load(Node::Object parent) -> void {
  node = parent->append<Node::Object>("SMP");

  debugger.load(node);
}

auto SMP::unload() -> void {
  debugger = {};
  node = {};
}

auto SMP::main() -> void {
  pollDebugConfig();

  if(r.wait) return instructionWait();
  if(r.stop) return instructionStop();

  debugger.instruction();
  instruction();
}

auto SMP::power(bool reset) -> void {
  if(auto fp = system.pak->read("ipl.rom")) {
    fp->read(iplrom, 64);
  }

  SPC700::power();
  create(system.apuFrequency() / 12.0, std::bind_front(&SMP::main, this));

  r.pc.byte.l = iplrom[62];
  r.pc.byte.h = iplrom[63];

  io = {};
  timer0 = {};
  timer1 = {};
  timer2 = {};
}

}
