#include <vector>
#include "mittelvec.h"


// -------------------
// >> Super Metroid <<
// -------------------

// ## SFX
std::vector<MittelVec::SamplePackItem> metroidSamplePack = {
    // menu
    {"ID:03 Pitch:0A09", "menu-click.wav", 6, false, 5.0, 0},
    {"ID:25 Pitch:06A5", "metroid-chirp.wav", 6, false, 5.0, 0},
    {"ID:07 Pitch:02A3", "save-file-select.wav", 6, false, 5.0, 0},
    {"ID:0C Pitch:1D06", "map-scroll.wav", 6, false, 5.0, 0},

    // crateria
    {"ID:0B Pitch:01FF", "ship-hover.wav", 6, false, 5.0, 0},
    {"ID:00 Pitch:04A0", "laser-perc.wav", 6, false, 5.0, 0},
    {"ID:04 Pitch:01F7", "laser-after.wav", 6, false, 5.0, 0},
    {"ID:03 Pitch:0280", "jump-land.wav", 6, false, 5.0, 0},
    {"ID:09 Pitch:0075", "muted-footstep.wav", 6, false, 5.0, 0},
    {"ID:03 Pitch:0A09", "weapon-toggle.wav", 6, false, 5.0, 0},
    {"ID:07 Pitch:02F6", "morph-jump-start.wav", 6, false, 5.0, 0},
    {"ID:00 Pitch:03E6", "missile-launch.wav", 6, false, 5.0, 0},
    {"ID:08 Pitch:02AC", "explosion-impact.wav", 6, false, 5.0, 0}, // both morphball bomb and missile
    {"ID:02 Pitch:04D2", "door-open.wav", 6, false, 5.0, 0},
    {"ID:02 Pitch:0A4C", "door-close.wav", 6, false, 5.0, 0},
};

// ## Music Cues
std::vector<MittelVec::MusicCue> metroidMusicCues = {
    {"SCENE: [CAD7C2A8] ", "main-menu.wav", true},
    {"SCENE: [C347DF0E]", "crateria.wav", true},
};