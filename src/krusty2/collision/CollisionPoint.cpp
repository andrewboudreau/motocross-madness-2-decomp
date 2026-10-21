// CollisionPoint.cpp -- reconstruction of D:\aardvark\VC\krusty2\CollisionPoint.cpp
// (__FILE__ string 0x00568730, pushed by AddCollisionPoint's debug allocation at 0x0043a346; tier 1).
//
// Extent (strong inference): 0x0043a1e0..0x0043b320, between CollisionObject.cpp's last
// function 0x0043a050 and the constraint helpers at 0x0043b320.  RTTI CollisionPoint
// (vtable 0x005511d8: deleting destructor 0x0043b300, slot 1 0x0043b240) and the
// .CRT$XCU 62-65 vector set 0x0043b050..0x0043b18b sit inside it.  The functions here
// are the unit's strict exact ones (src/krusty2/collision/targets.json).  The other
// functions of the extent: the contact merge 0x0043aa30 and refresh 0x0043ad80 are near
// misses in samples/physics/collision/CollisionPointNearMisses.cpp (which includes this
// file); the two sphere tests 0x0043a1e0/0x0043a270 are exact in
// src/reconstructed/CollisionQueries.cpp (whether they open this unit or close
// CollisionObject.cpp is open).
#include "collision/CollisionPoint.h"

#include "collision/CollisionPointCtor.h"
#include "math/Vec3.h"
#include "../../reconstructed/Matrix4.h"
#include "core/SoultreeObject.h"

// Same canonical dot declaration as Math3D.h, without its per-unit vector initializers.
float Vec3DotCall(const Vec3* a, const Vec3* b);

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

// Inline vector helpers of UpdateRelativeMotion.  Their shapes are those of the header
// inlines the original used (Math3D.h has the same operators): results returned through
// the three-float constructor, the cross product with parenthesised first products, and
// a dot product grouped z + (x + (y)), which is the term and operand order retail emits
// at both expanded dot sites.  UpdateRelativeMotion exhausts VC6's inline budget, so
// retail calls the out-of-line constructor 0x00404e60 inside the expanded operators
// (CollisionVec3's declared-only constructor stands for it) and the out-of-line dot
// 0x0040ae30, length 0x00435ec0 and division 0x0043c890 at the later sites (the call
// views Vec3DotCall, CollisionLength and CollisionDivide).  Tier 3 names.
static inline float CollisionPointDot(const CollisionVec3& a, const CollisionVec3& b) {
    return a.z * b.z + (a.x * b.x + (a.y * b.y));
}

static inline CollisionVec3 CollisionPointCross(const CollisionVec3& a, const CollisionVec3& b) {
    CollisionVec3 r;
    r.x = (a.y * b.z) - (a.z * b.y);
    r.y = (a.z * b.x) - (a.x * b.z);
    r.z = (a.x * b.y) - (a.y * b.x);
    return r;
}

static inline CollisionVec3 ScaledVec3(const CollisionVec3& v, float s) {
    return CollisionVec3(v.x * s, v.y * s, v.z * s);
}

static inline CollisionVec3 SummedVec3(const CollisionVec3& a, const CollisionVec3& b) {
    return CollisionVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline CollisionVec3 DividedVec3(const CollisionVec3& v, float s) {
    return ScaledVec3(v, 1.0f / s);
}

// 0x0043a640 (thiscall, ret 0x14).  Tier 3 semantics.  With the contact normal n:
//   relativePosition = worldPosition - a1
//   c = (n * (n . a2)) x a4          (normal component of a2 crossed with a4)
//   a5 > 0.001: t = a3 with its component along n removed, normalised; tangentSpeed = t . a3;
//               if that is above 0.001 the direction is normalise(c + t * tangentSpeed)
//               and spinSpeed = |c|; otherwise tangentSpeed = 0 and the direction is
//               normalise(c) with spinSpeed = |c|
//   otherwise:  tangentSpeed = 0, direction normalise(c), spinSpeed = |c|
// The direction goes to frictionDirection; a degenerate one (|c| <= 0.001) is zero with
// spinSpeed 0.
void CollisionPoint::UpdateRelativeMotion(const CollisionVec3* a1, const CollisionVec3* a2, const CollisionVec3* a3,
                                          const CollisionVec3* a4, float a5)
{
    relativePosition = worldPosition - *a1;
    float s = CollisionPointDot(surfaceNormal, *a2);
    CollisionVec3 u = surfaceNormal * s;
    CollisionVec3 c = CollisionPointCross(u, *a4);
    CollisionVec3 w;
    if (a5 > 0.001f) {
        CollisionVec3 t;
        if (CollisionRejectFrom(&t, a3, &surfaceNormal)) {
            CollisionVec3 n;
            t = *Vec3Normalize(&n, &t);
        }
        float f = CollisionPointDot(t, *a3);
        tangentSpeed = f;
        if (f > 0.001f) {
            CollisionVec3 v1 = ScaledVec3(t, f);
            w = SummedVec3(c, v1);
            float m = Vec3DotCall(reinterpret_cast<const Vec3*>(&w), reinterpret_cast<const Vec3*>(&w));
            float len;
            if (m == 0.0f)
                len = 0.0f;
            else if (m == 1.0f)
                len = 1.0f;
            else
                len = 1.0f / FastInvSqrt(m);
            float k = 1.0f / len;
            frictionDirection = ScaledVec3(w, k);
            float cm = Vec3DotCall(reinterpret_cast<const Vec3*>(&c), reinterpret_cast<const Vec3*>(&c));
            float cl;
            if (cm == 1.0f)
                cl = 1.0f;
            else
                cl = FastSqrt(cm);
            spinSpeed = cl;
        } else {
            tangentSpeed = 0.0f;
            w = c;
            float cm = Vec3DotCall(reinterpret_cast<const Vec3*>(&w), reinterpret_cast<const Vec3*>(&w));
            float len;
            if (cm == 1.0f)
                len = 1.0f;
            else
                len = FastSqrt(cm);
            spinSpeed = len;
            if (len > 0.001f) {
                frictionDirection = DividedVec3(w, len);
            } else {
                frictionDirection = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
                spinSpeed = 0.0f;
            }
        }
    } else {
        tangentSpeed = 0.0f;
        w = c;
        float len = CollisionLength(&w);
        spinSpeed = len;
        if (len > 0.001f) {
            CollisionVec3 d;
            frictionDirection = *CollisionDivide(&d, &w, len);
        } else {
            frictionDirection = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
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
        p->localPosition = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
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
// Bike passes primary scene nodes to AddCollisionPoint; the canonical Vec3
// return uses the same hidden 12-byte result buffer as the old call view.
void UpdateCollisionPointWorldPositions(int count, CollisionPoint** points) {
    for (int i = count; i > 0; i--, points++) {
        CollisionPoint* p = *points;
        if (p->ownerNode && p->inContact) {
            reinterpret_cast<Vec3&>((*points)->worldPosition) =
                reinterpret_cast<SoultreeObject*>((*points)->ownerNode)->LocalToWorldPoint(
                    reinterpret_cast<const Vec3&>(p->localPosition));
        }
    }
}

// 0x0043b190 (cdecl): out = a - b * (a . b), the part of a perpendicular to the (unit)
// vector b; returns 0 when the result is exactly the zero vector, else out.  Retail
// emits the dot product's y term first with b's component loaded first; the
// parenthesised middle product gives that order (docs/VC6_OPERAND_ORDER.md section 3).
CollisionVec3* CollisionRejectFrom(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b)
{
    float d = a->x * b->x + (a->y * b->y) + a->z * b->z;
    *out = *a - *b * d;
    if (out->x == 0.0f && out->y == 0.0f && out->z == 0.0f)
        return 0;
    return out;
}

// The original vector type's three-float constructor and operators are inline in a
// header (retail calls their out-of-line copies, e.g. the constructor 0x00404e60, only
// where a function exhausts VC6's inline budget, docs/VC6_INLINE_BUDGET.md).
// CollisionTypes.h declares CollisionVec3's constructor out of line for the sites that
// call it, so this unit builds inline results through a local derived type (tier 3).
struct CollisionPointVec3 : CollisionVec3 {
    CollisionPointVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};

// v * s returned through the constructor, as Math3D.h's operator* (retail builds the
// three products in a stack temporary, then copies it out).
static inline CollisionPointVec3 ScaleVec3(const CollisionVec3& v, float s) {
    return CollisionPointVec3(s * v.x, s * v.y, s * v.z);
}

// 0x0043b240 (slot 1): the friction force of the contact.  Without tangential or
// rotational slip (both <= 0.001) it is zero; otherwise its magnitude is
// -normalForce * surfaceGrip * frictionCoefficient along frictionDirection, and the
// stored magnitude is made non-negative.
void CollisionPoint::CollisionPointVirtualSlot1() {
    if (!(tangentSpeed > 0.001f) && !(spinSpeed > 0.001f)) {
        frictionMagnitude = 0.0f;
        frictionForce = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
    } else {
        frictionMagnitude = (-normalForce) * surfaceGrip * frictionCoefficient;
        frictionForce = ScaleVec3(frictionDirection, frictionMagnitude);
        if (frictionMagnitude < 0.0f)
            frictionMagnitude = -frictionMagnitude;
    }
}

// The file's four dynamically initialized vector constants, .CRT$XCU entries 62-65: thunks
// 0x0043b050/0x0043b0a0/0x0043b0f0/0x0043b140 jump to 60-byte bodies that build the value in a
// stack temporary and copy it out (copy-initialization from a constructor call, as for
// ConstraintMethodCollisionModel.cpp's 0x00579830).  The zero vector 0x005797f0 and the y axis
// 0x00579810 are the existing objects read through explicit CollisionVec3 views.
// They have external linkage so all consumers use this storage and its original
// initialization. The x/z axes remain local. This constructor-bearing type is a
// provisional view of the unrecovered header vector
// (CollisionVec3's three-float constructor is out of line, 0x00404e60).
struct CollisionPointConstVec3 {
    float x, y, z;
    CollisionPointConstVec3(float a, float b, float c) : x(a), y(b), z(c) {}
};
const CollisionPointConstVec3 kCollisionPointZero = CollisionPointConstVec3(0.0f, 0.0f, 0.0f);
static const CollisionPointConstVec3 kCollisionPointXAxis = CollisionPointConstVec3(1.0f, 0.0f, 0.0f);
const CollisionPointConstVec3 kCollisionPointYAxis = CollisionPointConstVec3(0.0f, 1.0f, 0.0f);
static const CollisionPointConstVec3 kCollisionPointZAxis = CollisionPointConstVec3(0.0f, 0.0f, 1.0f);
