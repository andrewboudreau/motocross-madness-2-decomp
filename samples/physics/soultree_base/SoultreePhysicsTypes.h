// Shared types for the SoultreePhysicsBaseObject reconstruction; Vec3 comes from
// ../common/Math3D.h.  Tier 3: names are invented; only the 12-byte {x,y,z} float
// layout is confirmed (3 dword moves for copies, fld/fmul at +0/+4/+8).
#ifndef SOULTREE_PHYSICS_TYPES_H
#define SOULTREE_PHYSICS_TYPES_H

#include "../common/Math3D.h"
typedef Vec3 SoultreeVec3;

// Global Vec3 constants in .data used to reset vectors (0x00689ee8 is all zero
// bits; 0x00685190 is the vector loaded by slot 36).
extern SoultreeVec3 g_SoultreeZeroVec3;   // 0x00689ee8
extern SoultreeVec3 g_SoultreeVec3_685190; // 0x00685190

#endif
