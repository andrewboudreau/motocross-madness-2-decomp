// CollisionQueries.cpp -- overlap tests near CollisionPoint.cpp (0x0043a1e0-0x0043a330).
// TU ownership is tier 3 (proximity to CollisionPoint.cpp's __FILE__ at 0x0043a346).
#include "CollisionQueries.h"

int SphereContainsPointXZ(const CollisionBoundingSphere* sphere, const Vec3* p, float r)
{
    r += sphere->radius;
    float dx = sphere->center.x - p->x;
    if (dx < 0.0f) dx = -dx;
    if (dx > r) return 0;
    float dz = sphere->center.z - p->z;
    if (dz < 0.0f) dz = -dz;
    if (dz > r) return 0;
    if (dx * dx + dz * dz > r * r) return 0;
    return 1;
}

int BoundingSpheresOverlap(const CollisionBoundingSphere* a, const CollisionBoundingSphere* b)
{
    float r = a->radius + b->radius;
    float dx = a->center.x - b->center.x;
    if (dx < 0.0f) dx = -dx;
    if (dx > r) return 0;
    float dy = a->center.y - b->center.y;
    if (dy < 0.0f) dy = -dy;
    if (dy > r) return 0;
    float dz = a->center.z - b->center.z;
    if (dz < 0.0f) dz = -dz;
    if (dz > r) return 0;
    if (dx * dx + dy * dy + dz * dz > r * r) return 0;
    return 1;
}
