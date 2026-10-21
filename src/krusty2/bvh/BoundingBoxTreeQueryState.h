#pragma once

#include "../math/Vec3.h"

// Hit record the leaf tests update through 0x00579058 (the 0x14-byte mesh record
// CollisionObject allocates): the hit fraction along the segment, the segment leaf's first
// point, the face normal.
struct QueryHit {
    float fraction;             // +0x00
    const Vec3* point;          // +0x04
    Vec3 normal;                // +0x08
};

// Shared query state; the existing definitions remain in BoundingBoxTreeQuery.cpp.
extern int g_CollisionScratchCount;       // 0x00579014
extern QueryHit* g_CollisionBoxResult;    // 0x00579058, mutable hit-record pointer
