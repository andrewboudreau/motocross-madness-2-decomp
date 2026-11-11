// FistpFloatToInt.h -- retail's inline-asm float-to-int helper, in ordinary C++ for the
// candidate sources the game build compiles with /QIfist (config/game_build.json).
//
// Retail converts with an inlined helper of the shape
//   fstp [t]; fld [t]; mov eax, [out]; fistp dword ptr [eax]
// in an ebp frame (for example D3DIMSoultreeShadow slot 30 at 0x00447301, TerrainShadow slot 30
// at 0x00509eef, KrustyBike 0x0048d82b, DropTextOverlay::Setup 0x0051af35).  The argument is
// rounded to binary32, then FISTP rounds it under the current x87 rounding mode, so the callers'
// `fistp(v - 0.5f)` is floor(v) apart from exact ties.  A VC6 `(int)` cast without /QIfist
// truncates through __ftol instead and is one too small whenever v's fraction is below one half.
//
// memcpy makes VC6 store and reload the argument as binary32, as retail's `fstp [t]; fld [t]`
// does.  Under /QIfist the cast is a FISTP under the current rounding mode.  VC6 /QIfist always
// stores 64 bits (`fistp qword ptr [t]`, low dword kept), which equals retail's 32-bit store for
// every result inside int; every caller documents why its values stay in range.  Outside int it
// would keep the low dword where retail stores 0x80000000 (ProjectedShadowNearMisses.cpp's
// ProjectedShadowFloatToInt reproduces that with a range check, about ten more instructions per
// site).  Exception flags are not reproduced.  Without /QIfist (calibration and the physics
// runner, which score shape only) the cast truncates through __ftol.
#ifndef FISTP_FLOAT_TO_INT_H
#define FISTP_FLOAT_TO_INT_H

#include <string.h>

static inline void FistpFloatToInt(int* out, float value)
{
    float stored;
    memcpy(&stored, &value, sizeof(stored));
    *out = (int)stored;
}

#endif
