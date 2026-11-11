// CollisionPoint.cpp -- reconstruction of D:\aardvark\VC\krusty2\CollisionPoint.cpp
// (__FILE__ string 0x00568730, pushed by AddCollisionPoint's debug allocation at 0x0043a346; tier 1).
//
// Extent (strong inference): 0x0043a1e0..0x0043b320, between CollisionObject.cpp's last
// function 0x0043a050 and the constraint helpers at 0x0043b320.  RTTI CollisionPoint
// (vtable 0x005511d8: deleting destructor 0x0043b300, slot 1 0x0043b240) and the
// .CRT$XCU 62-65 vector set 0x0043b050..0x0043b18b sit inside it.  The functions here
// are the unit's strict exact ones (src/krusty2/collision/targets.json).  The other
// functions of the extent: the
// two sphere tests 0x0043a1e0/0x0043a270 are exact in
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
                                  CollisionContactOwner* owner, float threshold, int* count, float normalForce,
                                  int surfaceOwner) {
    if (*count < capacity) {
        points[*count] = new(__FILE__, 27) CollisionPoint(normalForce, surfaceOwner);
        CollisionPoint* added = points[*count];
        added->localPosition = *position;
        added->penetration = threshold - 999.0f;
        added->ownerNode = owner;
        added->penetrationThreshold = threshold;
        (*count)++;
        return added;
    }
    return 0;
}

// 0x0043a570: AddCollisionPoint for a point the caller already owns (`point` may be null to reuse the
// slot's current pointer): resets its position to zero and records the penetration threshold.
CollisionPoint* AddExistingCollisionPoint(int capacity, CollisionPoint** points, float threshold, int* count,
                                          CollisionPoint* point) {
    if (*count < capacity) {
        if (point)
            points[*count] = point;
        CollisionPoint* added = points[*count];
        added->localPosition = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
        added->penetration = threshold - 999.0f;
        added->ownerNode = 0;
        added->penetrationThreshold = threshold;
        (*count)++;
        return added;
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
//   relativePosition = worldPosition - center
//   spin = (n * (n . angularVelocity)) x leverArm   (the spin about the normal at the lever arm)
//   speed > 0.001: tangent = velocity with its component along n removed, normalised;
//               tangentSpeed = tangent . velocity;
//               if that is above 0.001 the direction is normalise(spin + tangent * tangentSpeed)
//               and spinSpeed = |spin|; otherwise tangentSpeed = 0 and the direction is
//               normalise(spin) with spinSpeed = |spin|
//   otherwise:  tangentSpeed = 0, direction normalise(spin), spinSpeed = |spin|
// The direction goes to frictionDirection; a degenerate one (|spin| <= 0.001) is zero with
// spinSpeed 0.
void CollisionPoint::UpdateRelativeMotion(const CollisionVec3* center, const CollisionVec3* angularVelocity,
                                          const CollisionVec3* velocity, const CollisionVec3* leverArm, float speed)
{
    relativePosition = worldPosition - *center;
    float spinRate = CollisionPointDot(surfaceNormal, *angularVelocity);
    CollisionVec3 normalSpin = surfaceNormal * spinRate;
    CollisionVec3 spin = CollisionPointCross(normalSpin, *leverArm);
    CollisionVec3 direction;
    if (speed > 0.001f) {
        CollisionVec3 tangent;
        if (CollisionRejectFrom(&tangent, velocity, &surfaceNormal)) {
            CollisionVec3 normalized;
            tangent = *Vec3Normalize(&normalized, &tangent);
        }
        float along = CollisionPointDot(tangent, *velocity);
        tangentSpeed = along;
        if (along > 0.001f) {
            CollisionVec3 v1 = ScaledVec3(tangent, along);
            direction = SummedVec3(spin, v1);
            float lenSq = Vec3DotCall(reinterpret_cast<const Vec3*>(&direction),
                                   reinterpret_cast<const Vec3*>(&direction));
            float len;
            if (lenSq == 0.0f)
                len = 0.0f;
            else if (lenSq == 1.0f)
                len = 1.0f;
            else
                len = 1.0f / FastInvSqrt(lenSq);
            float inv = 1.0f / len;
            frictionDirection = ScaledVec3(direction, inv);
            float cm = Vec3DotCall(reinterpret_cast<const Vec3*>(&spin), reinterpret_cast<const Vec3*>(&spin));
            float cl;
            if (cm == 1.0f)
                cl = 1.0f;
            else
                cl = FastSqrt(cm);
            spinSpeed = cl;
        } else {
            tangentSpeed = 0.0f;
            direction = spin;
            float cm = Vec3DotCall(reinterpret_cast<const Vec3*>(&direction),
                                   reinterpret_cast<const Vec3*>(&direction));
            float len;
            if (cm == 1.0f)
                len = 1.0f;
            else
                len = FastSqrt(cm);
            spinSpeed = len;
            if (len > 0.001f) {
                frictionDirection = DividedVec3(direction, len);
            } else {
                frictionDirection = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
                spinSpeed = 0.0f;
            }
        }
    } else {
        tangentSpeed = 0.0f;
        direction = spin;
        float len = CollisionLength(&direction);
        spinSpeed = len;
        if (len > 0.001f) {
            CollisionVec3 divided;
            frictionDirection = *CollisionDivide(&divided, &direction, len);
        } else {
            frictionDirection = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
            spinSpeed = 0.0f;
        }
    }
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

// Helpers of the contact merge 0x0043aa30, in the header-inline shapes the rest of the unit
// uses (results through the inline constructor of CollisionPointVec3).  Tier 3 names.
static inline float ClampNonNegative(float v) { return v > 0.0f ? v : 0.0f; }
static inline CollisionPointVec3 InlineSum(const CollisionVec3& a, const CollisionVec3& b) {
    return CollisionPointVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline CollisionPointVec3 InlineDifference(const CollisionVec3& a, const CollisionVec3& b) {
    return CollisionPointVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
// a is taken by value: retail copies *scaleB to a stack temporary before the product
// (the output may alias it).
static inline CollisionVec3 ComponentProduct(CollisionVec3 a, const CollisionVec3& b) {
    CollisionVec3 r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
    return r;
}
// CollisionCross with each product parenthesised: retail's operand order for the angular
// term of 0x0043aa30 (the shared helper's bare form schedules the products differently).
static inline CollisionVec3 GroupedCross(const CollisionVec3& a, const CollisionVec3& b) {
    CollisionVec3 r;
    r.x = (a.y * b.z) - (a.z * b.y);
    r.y = (a.z * b.x) - (a.x * b.z);
    r.z = (a.x * b.y) - (a.y * b.x);
    return r;
}

// 0x0043aa30 (cdecl, provisional signature).  Merges the penetrating contacts of a manifold
// (tier 3 semantics): mean position, summed and normalised normal, deepest penetration, and
// the resulting linear (linA/origin) and angular response terms.  Returns the merged count.
// The deepest penetration is homed in the dead `points` argument slot and the normal's
// squared length in the `normal` slot, as in retail.  The first-contact clamp is by value
// (the value stays on the x87 stack across the compare), the loop loads the penetration
// once, the normal's squared length is accumulated y, then x, then z (retail squares y in
// place), and the angular term crosses through GroupedCross.
// The loop adds the contact's y into the sum through a pointer to it.  sum.y is then never
// stored directly, so in sum * (1/merged) VC6 multiplies it component-first
// (`fld sum.y; fmul st(1)`) while x and z, stored directly, are scalar-first, as in retail
// (docs/VC6_PATTERN_CORPUS.md section 6).  Accumulating all three through the pointer gives
// the component-first form for all three; direct accumulation, every scale helper shape and
// CollisionObject's mean idiom give it for none.
int SoultreeMergeContacts(int count, CollisionPoint** points, const CollisionVec3* scaleA, const CollisionVec3* scaleB,
                          const CollisionVec3* bias, CollisionVec3* origin, CollisionVec3* linA,
                          CollisionVec3* delta, CollisionVec3* normal, CollisionVec3* angular)
{
    CollisionVec3 sum;
    float deepest;
    int merged;
    CollisionPoint* first = points[0];
    if (first->inContact) {
        merged = 1;
        deepest = ClampNonNegative(first->penetration);
        sum = first->worldPosition;
        *normal = first->surfaceNormal;
    } else {
        sum = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
        *normal = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
        merged = 0;
        deepest = 0.0f;
    }
    for (int i = 1; i < count; ++i) {
        CollisionPoint* point = points[i];
        if (point->inContact) {
            CollisionVec3* total = &sum;
            float pen = point->penetration;
            if (pen > 0.0f && pen > deepest)
                deepest = point->penetration;
            ++merged;
            sum.x += point->worldPosition.x;
            total->y += point->worldPosition.y;
            sum.z += point->worldPosition.z;
            normal->x += point->surfaceNormal.x;
            normal->y += point->surfaceNormal.y;
            normal->z += point->surfaceNormal.z;
        }
    }
    if (merged > 0) {
        if (merged > 1) {
            sum = ScaleVec3(sum, 1.0f / merged);
            float len2 = normal->y * normal->y;
            len2 += normal->x * normal->x;
            len2 += normal->z * normal->z;
            if (len2 == 0.0f) {
                *normal = (*reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero));
            } else {
                float invLen = FastInvSqrt(len2);
                normal->x *= invLen;
                normal->y *= invLen;
                normal->z *= invLen;
            }
        }
        *delta = InlineDifference(sum, *origin);
        *angular = GroupedCross(ComponentProduct(*scaleB, *scaleA), *delta);
        *angular = InlineSum(*bias, *angular);
        CollisionVec3 push = *normal * deepest;
        linA->x += push.x;
        linA->y += push.y;
        linA->z += push.z;
        origin->x += push.x;
        origin->y += push.y;
        origin->z += push.z;
        return merged;
    }
    return 0;
}

// Provisional view of the frame object passed to 0x0043ad80 (thiscall callee, owner class
// unknown).  The contact owner view (0x004fd660) is in CollisionPoint.h.
class CollisionFrameHelper {
public:
    // 0x00507c10, ret 0x10: snaps v to the surface along n; the last argument receives the
    // surface flag byte.
    void QueryGround(CollisionVec3* v, const CollisionVec3* n, int a, char* flagOut);
};

// Recomputes the contact position/penetration and tallies the penetrating contacts; expanded
// in both paths of 0x0043ad80 (retail repeats it, including the grip store and return).
static inline void CollisionRefreshContact(CollisionPoint* p, CollisionFrameHelper* frame, int* penetrating)
{
    frame->QueryGround(&p->surfacePosition, &p->surfaceNormal, 0, &p->surfaceType);
    p->penetration = (p->surfacePosition.y - p->worldPosition.y) * p->surfaceNormal.y;
    p->surfaceType &= 7;
    p->inContact = (p->penetration >= p->penetrationThreshold) ? 1 : 0;
    if (p->inContact)
        ++*penetrating;
    if (p->surfaceOwner) {
        // surfaceOwner -> object whose +0xa4 table holds per-surface grip values at +0x3a0 (tier 3).
        char* table = *(char**)(p->surfaceOwner + 0xa4);
        p->surfaceGrip = ((float*)(table + 0x3a0))[(unsigned char)p->surfaceType];
    } else {
        p->surfaceGrip = 1.0f;
    }
}

// 0x0043ad80 (cdecl, provisional signature).  Refreshes every owned contact's world and
// surface position (a single contact in mode 1 is placed at offset - k * normal instead) and
// its penetration, flag and grip.  The two paths are an if/else with one trailing
// `return 1`: with a return inside the single-contact path VC6 pushes ebx/ebp/esi/edi at
// entry, with a single exit after the null-a1 return it delays them past it as retail does.
// The loop walks a pointer local `pp` copied from points after an `if (count > 0)` guard:
// then VC6 loads points before frame and homes the loop counter in points' dead argument
// slot, as retail does.  The single contact's position is summed through InlineSum (the
// sum returned through the three-float constructor): with the member-store operator+ VC6
// stores its y before the fstp of z, retail after.  The owner's transform is the canonical
// SoultreeObject::LocalToWorldPoint, its returned temporary bound to a const reference, so
// both copies read the result through the returned pointer as in retail.
int SoultreeRefreshContacts(void* a1, int* penetrating, int count, CollisionPoint** points,
                            CollisionFrameHelper* frame, CollisionVec3* offset, int mode, float k)
{
    if (!a1)
        return 0;
    *penetrating = 0;
    if (mode == 1 && count == 1) {
        CollisionPoint* point = points[0];
        if (point->ownerNode) {
            CollisionVec3 tmp = *offset;
            frame->QueryGround(&tmp, &point->surfaceNormal, 0, 0);
            CollisionVec3 contactPos = InlineSum(point->surfaceNormal * -k, *offset);
            point->worldPosition = contactPos;
            point->surfacePosition = contactPos;
            CollisionRefreshContact(point, frame, penetrating);
        }
    } else {
        if (count > 0) {
            CollisionPoint** pp = points;
            for (; count > 0; count--, pp++) {
                CollisionPoint* point = *pp;
                if (point->ownerNode) {
                    const CollisionVec3& world = reinterpret_cast<const CollisionVec3&>(
                        reinterpret_cast<SoultreeObject*>((*pp)->ownerNode)->LocalToWorldPoint(
                            reinterpret_cast<const Vec3&>(point->localPosition)));
                    point->worldPosition = world;
                    point->surfacePosition = world;
                    CollisionRefreshContact(point, frame, penetrating);
                }
            }
        }
    }
    return 1;
}

// 0x0043aff0: refreshes the world position (field_0x14) of every owned, active contact point from the
// owner's transform of its local position (field_0x08).
// Bike passes primary scene nodes to AddCollisionPoint; the canonical Vec3
// return uses the same hidden 12-byte result buffer as the old call view.
void UpdateCollisionPointWorldPositions(int count, CollisionPoint** points) {
    for (int i = count; i > 0; i--, points++) {
        CollisionPoint* point = *points;
        if (point->ownerNode && point->inContact) {
            reinterpret_cast<Vec3&>((*points)->worldPosition) =
                reinterpret_cast<SoultreeObject*>((*points)->ownerNode)->LocalToWorldPoint(
                    reinterpret_cast<const Vec3&>(point->localPosition));
        }
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
