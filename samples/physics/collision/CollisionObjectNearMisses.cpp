// Near misses for src/krusty2/collision/CollisionObject.cpp (TU CollisionObject.cpp).
//
// Fn_00439600 (0x00439600, 532 bytes; cdecl, called from BikeAI.cpp 0x00416105, 0x00416a9c,
// 0x00416ccf): exact with every relocation resolved.  The segment-parameter numerator and the
// distance go through a by-reference inline dot whose body is grouped (x + y) + z: retail
// multiplies v.y*d.y and v.x*d.x before v.z*d.z and consumes v.z in place.  The same grouping
// written inline in the expression ((v.y*d.y + v.x*d.x) + v.z*d.z, with or without
// parenthesised products) keeps v.z alive on the x87 stack and pops it after the divide (four
// bytes longer); the flat sum in any order, CollisionDot and by-value helpers emit v.z*d.z
// first.  The denominator must stay written out as (d.y*d.y + d.x*d.x) + d.z*d.z: the helper
// with d twice is 2 bytes shorter.
#include "collision/CollisionObject.cpp"

// Dot product grouped (x + y) + z, by reference (see the file header).
inline float CapsuleSegmentDot(const CollisionVec3& a, const CollisionVec3& b)
{
    return (a.x * b.x + a.y * b.y) + a.z * b.z;
}

// 0x00439600 (cdecl; BikeAI.cpp 0x00416105, 0x00416a9c, 0x00416ccf): the capsule version of
// 0x004394f0.  The point of the segment closest to the object's bounding-sphere centre
// (+0x40) must lie within radius + boundRadius before the hull / model tests run.
int Fn_00439600(CollisionVec3* ends, float radius, float radiusSq, CollisionObject* object)
{
    if (object->statusFlags & 1) {
        const CollisionVec3* center = (const CollisionVec3*)&object->field_0x34;
        CollisionVec3 d = ends[1] - ends[0];
        CollisionVec3 v = *center - ends[0];
        float t = CapsuleSegmentDot(v, d) / ((d.y * d.y + d.x * d.x) + d.z * d.z);
        if (t >= 1.0f)
            t = 1.0f;
        else if (t <= 0.0f)
            t = 0.0f;
        CollisionVec3 closest = ends[0] + d * t;
        CollisionVec3 diff = *center - closest;
        float distSq = CapsuleSegmentDot(diff, diff);
        float dist = distSq == 1.0f ? 1.0f : FastSqrt(distSq);
        if (dist < radius + object->boundRadius) {
            switch (object->shapeType) {
            case 0: {
                CollisionHullBody* hull = (CollisionHullBody*)object->shape;
                return CapsuleTreeQueryWithVertices(ends, radius, radiusSq, hull->triangleTree, &hull->worldTransform, 1,
                                   hull->vertices);
            }
            case 1: {
                CollisionModelBody* model = (CollisionModelBody*)object->shape;
                if (BoxCapsuleOverlap(&model->center, &model->halfExtents, ends, radius, radiusSq, &model->field_0x88)) {
                    for (int i = 0; i < model->elementCount; i++) {
                        CollisionHullBody* hull = &model->elements[i];
                        if (CapsuleTreeQueryWithVertices(ends, radius, radiusSq, hull->triangleTree, &hull->worldTransform, 1,
                                        hull->vertices))
                            return 1;
                    }
                }
                return 0;
            }
            case 2:
                return 0;
            }
        }
    }
    return 0;
}
