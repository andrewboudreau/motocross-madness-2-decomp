// CollisionPointCtor.h -- inline CollisionPoint::CollisionPoint(float, int) (retail 0x0043a3c0 region).
// Shared so other translation units (e.g. Tire) can inline it with the collision globals.
#ifndef COLLISION_POINT_CTOR_H
#define COLLISION_POINT_CTOR_H

#include "CollisionPoint.h"

inline CollisionPoint::CollisionPoint(float a, int b)
    : normalForce(a), surfaceOwner(b) {
    localPosition = g_CollisionZeroVec3;
    ownerNode = 0;
    worldPosition = g_CollisionZeroVec3;
    surfacePosition = g_CollisionZeroVec3;
    surfaceNormal = g_CollisionVec3_579810;
    relativePosition = g_CollisionZeroVec3;
    field_0x44 = g_CollisionZeroVec3;
    frictionDirection = g_CollisionZeroVec3;
    field_0x5c = g_CollisionZeroVec3;
    tangentSpeed = 0;
    spinSpeed = 0;
    field_0xb8 = 1.0f;
    surfaceGrip = 1.0f;
    penetration = -999.0f;               // 0xc479c000 at 0x0043a48c
    penetrationThreshold = 0;
    inContact = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0xb4 = 0;
    field_0xa0 = 0.5f;
    surfaceType = 0;
    frictionMagnitude = 0;
    frictionForce = g_CollisionZeroVec3;
    frictionCoefficient = 0;
    field_0x68 = g_CollisionZeroVec3;
}

#endif
