// CollisionPointCtor.h -- inline CollisionPoint::CollisionPoint(float, int) (retail 0x0043a3c0 region).
// Shared so other translation units (e.g. Tire) can inline it with the collision globals.
#ifndef COLLISION_POINT_CTOR_H
#define COLLISION_POINT_CTOR_H

#include "collision/CollisionPoint.h"

inline CollisionPoint::CollisionPoint(float force, int owner)
    : normalForce(force), surfaceOwner(owner) {
    localPosition = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
    ownerNode = 0;
    worldPosition = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
    surfacePosition = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
    surfaceNormal = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointYAxis));
    relativePosition = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
    appliedShare = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
    frictionDirection = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
    field_0x5c = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
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
    blendWeight = 0.5f;
    surfaceType = 0;
    frictionMagnitude = 0;
    frictionForce = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
    frictionCoefficient = 0;
    field_0x68 = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
}

#endif
