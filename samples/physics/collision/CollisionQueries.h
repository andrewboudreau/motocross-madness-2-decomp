// Collision overlap queries found between 0x0043a1e0 and 0x0043a330 (tier 3 names).
#ifndef COLLISION_QUERIES_H
#define COLLISION_QUERIES_H

#include "math/Math3D.h"

// Bounding sphere as read by 0x0043a1e0/0x0043a270: center at +0x40, radius at +0x4c
// (this is the same location CollisionObject keeps its bounds; tier 3 layout).
struct CollisionBoundingSphere {
    char field_0x00[0x40];
    Vec3 center;      // +0x40
    float radius;     // +0x4c
};

// 0x0043a1e0 (cdecl): does the circle in the XZ plane of radius (r + sphere.radius)
// around sphere.center contain point p?  Per-axis |d| > R early-outs, then d.x^2+d.z^2 <= R^2.
int SphereContainsPointXZ(const CollisionBoundingSphere* sphere, const Vec3* p, float r);

// 0x0043a270 (cdecl): do two bounding spheres overlap?  Per-axis |d| > R+R early-outs,
// then |d|^2 <= (ra+rb)^2.
int BoundingSpheresOverlap(const CollisionBoundingSphere* a, const CollisionBoundingSphere* b);

#endif
