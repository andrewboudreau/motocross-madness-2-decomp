// Group 3 bring-up: CollisionObject.cpp's capsule-vs-object test 0x00439600, called only by
// BikeAI.cpp's obstacle query (Group3_KrustyBikeAI.cpp QuerySegmentObstacles).
//
// Status: best-guess-unreviewed for the build.  The body is the near miss of
// samples/physics/collision/CollisionObjectNearMisses.cpp (recorded there as exact with every
// relocation resolved), extracted without that sample's whole-TU include and re-read against
// the retail disassembly: flag bit 0 of +0x31, closest segment point to the bounding-sphere
// centre (+0x40), FastSqrt (0x00460b50), then the hull / model capsule tree tests
// (0x00429890, model broad phase 0x00425900).
#include "math/Math3D.h"
#include "collision/CollisionObject.h"
#include "collision/CollisionShapeTests.h"
#include "bvh/BoundingBoxTreeQueryFunctions.h"

float FastSqrt(float x);   // 0x00460b50 (FastMath.cpp)

// Dot product grouped (x + y) + z, by reference.
static inline float CapsuleSegmentDot(const CollisionVec3& a, const CollisionVec3& b)
{
    return (a.x * b.x + a.y * b.y) + a.z * b.z;
}

// 0x00439600 (cdecl): does the capsule ends[0]..ends[1] of the given radius touch object?
int Fn_00439600(CollisionVec3* ends, float radius, float radiusSq, CollisionObject* object)
{
    if (object->isEnabled) {
        const CollisionVec3* center = (const CollisionVec3*)&object->boundCenter;
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
                return CapsuleTreeQueryWithVertices(reinterpret_cast<const Vec3*>(ends), radius, radiusSq,
                    reinterpret_cast<QueryTreeNode*>(hull->triangleTree), &hull->worldTransform, 1,
                    reinterpret_cast<Vec3*>(hull->vertices));
            }
            case 1: {
                CollisionModelBody* model = (CollisionModelBody*)object->shape;
                if (BoxCapsuleOverlap(&model->center, &model->halfExtents, ends, radius, radiusSq,
                                      &model->bodyTransform)) {
                    for (int i = 0; i < model->elementCount; i++) {
                        CollisionHullBody* hull = &model->elements[i];
                        if (CapsuleTreeQueryWithVertices(reinterpret_cast<const Vec3*>(ends), radius, radiusSq,
                                reinterpret_cast<QueryTreeNode*>(hull->triangleTree), &hull->worldTransform, 1,
                                reinterpret_cast<Vec3*>(hull->vertices)))
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
