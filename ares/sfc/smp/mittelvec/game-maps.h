#include <vector>
#include "mittelvec.h"


// -------------------
// >> Super Metroid <<
// -------------------

// ## SFX
std::vector<MittelVec::SamplePackItem> samplePackItems = {
    {"nbc-chimes", "nbc-chimes.wav", 6, false, 5.0, 0},
    {"nbc-chimes-high", "nbc-chimes.wav", 6, false, 5.0, 7},
    {"nbc-chimes-low", "nbc-chimes.wav", 6, false, 5.0, -7},
    {"blip", "blip.wav", 6, false, 5.0, 0},
    {"blip-high", "blip.wav", 6, false, 5.0, 8},
    {"blip-low", "blip.wav", 6, false, 5.0, -5},
    {"perc2", "perc.wav", 6, false, 5.0, 2},
    {"perc3", "perc.wav", 6, false, 5.0, -3},
    {"perc", "perc.wav", 6, false, 5.0, 0},
};

// ## Music Cues
std::vector<MittelVec::MusicCue> musicCues = {
    {"cue-1", "cue-1.wav", true},
    {"cue-2", "cue-2.wav", true},
};