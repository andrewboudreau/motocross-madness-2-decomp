#include "CollisionPoint.h"

#include "CollisionPointCtor.h"

void CollisionPoint::CollisionPointVirtualSlot1() {
    if (!(tangentSpeed > 0.001f) && !(spinSpeed > 0.001f)) {
        frictionMagnitude = 0.0f;
        frictionForce = g_CollisionZeroVec3;
    } else {
        frictionMagnitude = (-normalForce) * surfaceGrip * frictionCoefficient;
        frictionForce = frictionDirection * frictionMagnitude;
        if (frictionMagnitude < 0.0f)
            frictionMagnitude = -frictionMagnitude;
    }
}

CollisionPoint* AddCollisionPoint(int capacity, CollisionPoint** points, const CollisionVec3* position,
                                  CollisionContactOwner* owner, float a4, int* count, float a6,
                                  int a7) {
    if (*count < capacity) {
        points[*count] = new(__FILE__, 27) CollisionPoint(a6, a7);
        CollisionPoint* p = points[*count];
        p->localPosition = *position;
        p->penetration = a4 - 999.0f;
        p->ownerNode = owner;
        p->penetrationThreshold = a4;
        (*count)++;
        return p;
    }
    return 0;
}

// 0x0043a640.  Tier 3 semantics.  Given the contact normal n = field_0x2c:
//   field_0x38 = field_0x14 - a1                      (position relative to a1)
//   c          = (n * (n . a2)) x a4                   (normal component of a2 crossed with a4)
//   a5 > 0.001: t = a3 with its component along n removed and renormalised; field_0x90 = t . a3;
//               if that is positive the tangent is normalise(t * field_0x90 + c),
//               otherwise normalise(c); field_0x94 = |c|
//   otherwise:  field_0x90 = 0, tangent = normalise(c), field_0x94 = |c|
// The tangent is stored in field_0x50; a degenerate (|c| <= 0.001) tangent is zero.
void CollisionPoint::Fn_0043a640(const CollisionVec3* a1, const CollisionVec3* a2, const CollisionVec3* a3,
                                 const CollisionVec3* a4, float a5)
{
    relativePosition = worldPosition - *a1;

    float s = surfaceNormal.y * a2->y + surfaceNormal.x * a2->x + surfaceNormal.z * a2->z;
    CollisionVec3 u = surfaceNormal * s;
    CollisionVec3 c = CollisionCross(u, *a4);

    if (a5 > 0.001f) {
        CollisionVec3 t;
        if (CollisionRejectFrom(&t, a3, &surfaceNormal)) {
            CollisionVec3 n;
            t = *Fn_005087b0(&n, &t);
        }
        float f = Fn_0040ae30(&t, a3);
        tangentSpeed = f;
        if (f > 0.001f) {
            CollisionVec3 v1 = CollisionVec3(t.x * f, t.y * f, t.z * f);
            CollisionVec3 w = CollisionVec3(v1.x + c.x, v1.y + c.y, v1.z + c.z);
            float m = Fn_0040ae30(&w, &w);
            float len;
            if (m == 0.0f)
                len = 0.0f;
            else if (m == 1.0f)
                len = 1.0f;
            else
                len = 1.0f / FastInvSqrt(m);
            float k = 1.0f / len;
            frictionDirection = CollisionVec3(w.x * k, w.y * k, w.z * k);
            float cm = Fn_0040ae30(&c, &c);
            if (cm == 1.0f)
                spinSpeed = 1.0f;
            else
                spinSpeed = FastSqrt(cm);
        } else {
            tangentSpeed = 0.0f;
            float cm = Fn_0040ae30(&c, &c);
            float len;
            if (cm == 1.0f)
                len = 1.0f;
            else
                len = FastSqrt(cm);
            spinSpeed = len;
            if (len > 0.001f) {
                float k = 1.0f / len;
                frictionDirection = CollisionVec3(c.x * k, c.y * k, c.z * k);
            } else {
                frictionDirection = g_CollisionZeroVec3;
                spinSpeed = 0.0f;
            }
        }
    } else {
        tangentSpeed = 0.0f;
        float len = CollisionLength(&c);
        spinSpeed = len;
        if (len > 0.001f) {
            CollisionVec3 d;
            frictionDirection = *CollisionDivide(&d, &c, len);
        } else {
            frictionDirection = g_CollisionZeroVec3;
            spinSpeed = 0.0f;
        }
    }
}

// 0x0043a570: AddCollisionPoint for a point the caller already owns (`point` may be null to reuse the
// slot's current pointer): resets its position to zero and records the penetration threshold.
CollisionPoint* AddExistingCollisionPoint(int capacity, CollisionPoint** points, float a4, int* count,
                                          CollisionPoint* point) {
    if (*count < capacity) {
        if (point)
            points[*count] = point;
        CollisionPoint* p = points[*count];
        p->localPosition = g_CollisionZeroVec3;
        p->penetration = a4 - 999.0f;
        p->ownerNode = 0;
        p->penetrationThreshold = a4;
        (*count)++;
        return p;
    }
    return 0;
}

// 0x0043a5e0: removes `point` from the pointer list by shifting the tail down; the vacated last slot is cleared.
void RemoveCollisionPoint(CollisionPoint** points, CollisionPoint* point, int* count) {
    for (int i = 0; i < *count; i++) {
        if (points[i] == point) {
            for (int j = i; j < *count - 1; j++)
                points[j] = points[j + 1];
            (*count)--;
            points[*count] = 0;
            return;
        }
    }
}

// 0x0043aff0: refreshes the world position (field_0x14) of every owned, active contact point from the
// owner's transform of its local position (field_0x08).
void UpdateCollisionPointWorldPositions(int count, CollisionPoint** points) {
    CollisionVec3 tmp;
    for (int i = count; i > 0; i--, points++) {
        CollisionPoint* p = *points;
        if (p->ownerNode && *(int*)&p->inContact) {
            CollisionVec3* world = p->ownerNode->Fn_004fd660(&tmp, &p->localPosition);
            (*points)->worldPosition = *world;
        }
    }
}
