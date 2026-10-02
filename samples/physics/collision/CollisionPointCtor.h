// CollisionPointCtor.h -- inline CollisionPoint::CollisionPoint(float, int) (retail 0x0043a3c0 region).
// Shared so other translation units (e.g. Tire) can inline it with the collision globals.
#ifndef COLLISION_POINT_CTOR_H
#define COLLISION_POINT_CTOR_H

#include "CollisionPoint.h"

inline CollisionPoint::CollisionPoint(float a, int b)
    : field_0x88(a), field_0xc0(b) {
    field_0x08 = g_CollisionZeroVec3;
    field_0x04 = 0;
    field_0x14 = g_CollisionZeroVec3;
    field_0x20 = g_CollisionZeroVec3;
    field_0x2c = g_CollisionVec3_579810;
    field_0x38 = g_CollisionZeroVec3;
    field_0x44 = g_CollisionZeroVec3;
    field_0x50 = g_CollisionZeroVec3;
    field_0x5c = g_CollisionZeroVec3;
    field_0x90 = 0;
    field_0x94 = 0;
    field_0xb8 = 1.0f;
    field_0x8c = 1.0f;
    field_0x98 = -999.0f;               // 0xc479c000 at 0x0043a48c
    field_0x9c = 0;
    field_0xa4 = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0xb4 = 0;
    field_0xa0 = 0.5f;
    field_0xbc = 0;
    field_0x84 = 0;
    field_0x78 = g_CollisionZeroVec3;
    field_0x74 = 0;
    field_0x68 = g_CollisionZeroVec3;
}

#endif
