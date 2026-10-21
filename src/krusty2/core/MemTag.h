// MemTag.h -- the allocation-category tracker reached through the global pointer at
// 0x0056df04.  Callers bracket allocations with Push("Terrain"/"Collision"/...) and
// Pop(previous) (Terrain dtor 0x005079f0, SoultreePhysicsBaseObject slot 2,
// CollisionCharacter ctor 0x004318d0).  Tier 1: both are thiscall with one stack
// argument (ret 4) and Push's eax result is what the caller later passes to Pop.
// The class and method names are tier 3.  The class is defined once, in
// src/reconstructed/MemTag.h, for both header trees.
#ifndef MCM2_PHYSICS_COMMON_MEMTAG_H
#define MCM2_PHYSICS_COMMON_MEMTAG_H

#include "../../reconstructed/MemTag.h"

#endif
