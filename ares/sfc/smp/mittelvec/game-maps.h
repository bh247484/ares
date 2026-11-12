#include <vector>
#include "mittelvec.h"


// -------------------
// >> Super Metroid <<
// -------------------

// ## SFX
// Fields...
//   { slug (string - {ID}:{Pitch}), fileName (string), polyphony (int = 1), loop (bool = false), gain (float = 1.0f), pitchShift (int = 0), envConfig (optional), filterConfig (optional) }
// Example
//   {"00:04A0", "laser-perc.wav", 6, false, 1.0f, 0}
std::vector<MittelVec::SamplePackItem> metroidSamplePack = {
    // main menu
    {"0C:1D06", "map-scroll.wav", 6, false, 0.0, 0},
    {"03:0A09", "menu-click.wav", 6, false, 0.0, 0},
    // {"25:06A5", "metroid-chirp.wav", 6, false, 0.0, 0},
    {"07:02A3", "save-file-select.wav", 6, false, 0.0, 0},

    // crateria
    {"02:04D2", "door-open.wav", 6, false, 0.0, 0},
    {"02:0A4C", "door-close.wav", 6, false, 0.0, 0},
    {"09:0075", "footstep.wav", 6, false, 0.0, 0},
    {"08:02AC", "impact-explosion.wav", 6, false, 0.0, 0}, // both morphball bomb and missile
    {"03:0280", "jump-landing.wav", 6, false, 0.0, 0},
    {"07:02F6", "jump-start.wav", 6, false, 0.0, 0},
    // {"04:01F7", "laser-after.wav", 6, false, 0.0, 0},
    {"00:04A0", "laser-revised.wav", 6, false, 0.0, 0},
    {"00:03E6", "missile-launch.wav", 6, false, 0.0, 0},
    {"0B:01FF", "ship-hover.wav", 6, false, 0.0, 0},
    {"03:0A09", "weapon-toggle.wav", 6, false, 0.0, 0},
};

// ## Music Cues
// Fields...
//   { slug (string), fileName (string), loop (bool = true), gain (float = 1.0f) }
// 
// Example...
//   {"CAD7C2A8", "main-menu.wav", true, 1.0f}
std::vector<MittelVec::MusicCue> metroidMusicCues = {
    {"CAD7C2A8", "metroid-theme.wav", true, -1.0 },
    {"C347DF0E", "metroid-level.wav", true, -1.0 },
};