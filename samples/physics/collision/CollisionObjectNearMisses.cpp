// Near misses for src/krusty2/collision/CollisionObject.cpp (TU CollisionObject.cpp).
//
// Fn_00439600 (0x00439600, 532 bytes; cdecl, called from BikeAI.cpp 0x00416105, 0x00416a9c,
// 0x00416ccf): 516/532 with every relocation resolved.  Only the numerator of the segment
// parameter differs: retail multiplies v.y*d.y and v.x*d.x before v.z*d.z (v.z stays on the
// x87 stack); every source order tried (natural, reversed, parenthesised, CollisionDot with
// either operand order) emits v.z*d.z first.  The denominator needs the explicit
// (y*y + x*x) + z*z grouping and the distance (y*y + x*x) + z*z; the rest matches.
#include "collision/CollisionObject.cpp"

// 0x00439600 (cdecl; BikeAI.cpp 0x00416105, 0x00416a9c, 0x00416ccf): the capsule version of
// 0x004394f0.  The point of the segment closest to the object's bounding-sphere centre
// (+0x40) must lie within radius + boundRadius before the hull / model tests run.
int Fn_00439600(CollisionVec3* ends, float radius, float radiusSq, CollisionObject* object)
{
    if (object->statusFlags & 1) {
        const CollisionVec3* center = (const CollisionVec3*)&object->field_0x34;
        CollisionVec3 d = ends[1] - ends[0];
        CollisionVec3 v = *center - ends[0];
        float t = (v.x * d.x + v.y * d.y + v.z * d.z) / ((d.y * d.y + d.x * d.x) + d.z * d.z);
        if (t >= 1.0f)
            t = 1.0f;
        else if (t <= 0.0f)
            t = 0.0f;
        CollisionVec3 closest = ends[0] + d * t;
        CollisionVec3 diff = *center - closest;
        float distSq = (diff.y * diff.y + diff.x * diff.x) + diff.z * diff.z;
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
