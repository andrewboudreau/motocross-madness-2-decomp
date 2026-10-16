#pragma once

// The detail-level settings that D3DIMSoultreeObject::ApplyLodSettings
// (0x004451e0) copies into a model: one 16-byte row per detail level,
// indexed `level << 4` off the pointer at 0x00689f18.
//
// Confirmed (retail bytes): BikeRace::SetDetailLevel (0x00423790) and
// Scene::SetDetailLevel (0x004eff30) store 0x005744c8 into 0x00689f18 when
// Game+0x2d0 (bladeRenderer) is set and 0x00574428 otherwise. The two tables
// are 0xa0 bytes (10 rows) apart and the second is followed by string data.
// 0x00574428 holds thresholds 5.5..1.0 with every flag 1; 0x005744c8 holds
// 7.0..1.0 with the flags turning on only at the coarser levels.
struct SoultreeLodSetting {
    float lodThreshold;                        // +0x00 SetLodThreshold
    int texturePerspective;                    // +0x04 copied to texturePerspective (+0x1b4)
    int gouraudShading;                        // +0x08 copied to gouraudShading (+0x1b8)
    int useLighting;                           // +0x0c SetUseLighting
};

extern SoultreeLodSetting* g_ActiveLodSettings;          // 0x00689f18
extern SoultreeLodSetting g_BladeLodSettings[10];        // 0x005744c8 (bladeRenderer set)
extern SoultreeLodSetting g_NonBladeLodSettings[10];     // 0x00574428 (bladeRenderer clear)
