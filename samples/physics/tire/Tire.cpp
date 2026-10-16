// Tire.cpp -- Tire (CollisionObject + MovingPart + CollisionPoint), see Tire.h.
#include "Tire.h"
#include <float.h>
#include <math.h>

#define TIRE_PI 3.14159265358979f

// The four Math3D vector constants of this unit (.CRT$XCU 319-322, initialisers
// 0x00515740..0x0051587b): 0x0068a3d0, 0x0068a3e0, 0x0068a3f0 and 0x0068a3c0.
// The constructor, UpdateSuspensionProbe and CollisionPoint slot 1 read them.
// CollisionVec3's (x, y, z) constructor is out of line, so the values go through
// an inline helper; it gives the same initialiser code as Math3D.h's.
static inline CollisionVec3 TireVec3(float x, float y, float z)
{
    CollisionVec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}
static const CollisionVec3 kVec3Zero = TireVec3(0.0f, 0.0f, 0.0f);
static const CollisionVec3 kVec3XAxis = TireVec3(1.0f, 0.0f, 0.0f);
static const CollisionVec3 kVec3YAxis = TireVec3(0.0f, 1.0f, 0.0f);
static const CollisionVec3 kVec3ZAxis = TireVec3(0.0f, 0.0f, 1.0f);

// Callback stored in CollisionObject::field_0x88 by slot 8 (0x00512e30).  a is the
// tire's own CollisionObject (field_0x60 = owner, the Tire), b is the object that was hit.
static void TireCollisionCallback(CollisionObject* a, CollisionObject* b)
{
    Tire* tire = (Tire*)a->ownerObject;
    int tag = b->ownerType;
    if (tag == 0x64) {
        // virtual call, vtable offset 0xcc, on b's owner
        if ((*(int (__fastcall**)(void*))(**(int**)&b->ownerObject + 0xcc))(b->ownerObject))
            return;
    } else if (tag == 0x65)
        return;
    tire->HandleContact(1, tag, b);
}

// 0x0040ae30 (cdecl, out of line): dot product of two vectors, result in st(0).
float TireDot(const CollisionVec3* a, const CollisionVec3* b);

// Inline form of the dot product (the out-of-line dot is TireDot).
static inline float TireDotInline(const CollisionVec3& a, const CollisionVec3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}

// Normalised copy of v (returned unchanged when |v|^2 is exactly 1); retail's y-axis path
// (inline dot, out-of-line constructor).
static inline CollisionVec3 TireNormalizeY(const CollisionVec3& v)
{
    float lenSq = TireDotInline(v, v);
    if (lenSq == 1.0f)
        return v;
    float s = FastInvSqrt(lenSq);
    return CollisionVec3(v.x * s, v.y * s, v.z * s);
}

// Normalised copy of v (returned unchanged when |v|^2 is exactly 1); retail's z-axis path.
static inline CollisionVec3 TireNormalizeZ(const CollisionVec3& v)
{
    float lenSq = TireDot(&v, &v);
    if (lenSq == 1.0f)
        return v;
    return v * FastInvSqrt(lenSq);
}

// 0x00515880 (cdecl, 703 bytes; single caller 0x00515376 inside 0x00514550).  Tier 3 name.
// Builds a rotation frame (row-vector convention, translation in row 3) from two axes:
//   row 2 = yAxis, row 3 = zAxis, row 1 = yAxis x zAxis, row 4 = pos, last column 0,0,0,1.
// Both axes are normalised (skipped when |v|^2 is exactly 1).  When `orthogonalize` is set
// the vectors are first made perpendicular using c = y x z:
//   keepZ != 0 : y = z x c  (z is kept)      keepZ == 0 : z = c x y  (y is kept).
void TireBuildFrame(CollisionMatrix4* out, const CollisionVec3* pos, const CollisionVec3* zAxis,
                           const CollisionVec3* yAxis, int orthogonalize, int keepZ)
{
    CollisionVec3 z = *zAxis;
    CollisionVec3 y = *yAxis;

    if (orthogonalize) {
        CollisionVec3 c;
        c.x = y.y * z.z - y.z * z.y;
        c.y = y.z * z.x - y.x * z.z;
        c.z = y.x * z.y - y.y * z.x;
        CollisionVec3 r;
        if (keepZ) {
            r.x = c.z * z.y - c.y * z.z;
            r.y = c.x * z.z - c.z * z.x;
            r.z = c.y * z.x - c.x * z.y;
            y = r;
        } else {
            r.x = c.y * y.z - c.z * y.y;
            r.y = c.z * y.x - c.x * y.z;
            r.z = c.x * y.y - c.y * y.x;
            z = r;
        }
    }

    y = TireNormalizeY(y);

    CollisionVec3 n = TireNormalizeZ(z);

    CollisionVec3 t;
    t.x = y.y * n.z - y.z * n.y;
    t.y = n.x * y.z - y.x * n.z;
    t.z = y.x * n.y - n.x * y.y;
    out->m[0] = t.x;
    out->m[1] = t.y;
    out->m[2] = t.z;
    out->m[3] = 0.0f;
    out->m[4] = y.x;
    out->m[5] = y.y;
    out->m[6] = y.z;
    out->m[7] = 0.0f;
    out->m[8] = n.x;
    out->m[9] = n.y;
    out->m[10] = n.z;
    out->m[11] = 0.0f;
    out->m[12] = pos->x;
    out->m[13] = pos->y;
    out->m[14] = pos->z;
    out->m[15] = 1.0f;
}

// The only layout facts known about the other object's owner (tier 3): three Vec3 members.
struct TireContactOwner {
    char pad0[0x40];
    CollisionVec3 field_0x40;
    char pad1[0x64 - 0x4c];
    CollisionVec3 field_0x64;
    char pad2[0x224 - 0x70];
    CollisionVec3 field_0x224;
};

// 0x00512e80.  Records a contact vector taken from the other object's owner:
// field_0x1e4 = "contact valid", field_0x1f4 = the vector.  Which vector is chosen by the
// tag of the other object (tier 3 semantics; the owner offsets 0x40/0x64/0x224 are literal).
void Tire::HandleContact(int a, int tag, CollisionObject* other)
{
    // Each case copies its own vector (VC6 merges the three copies into one tail and keeps
    // the source in edx); selecting a source pointer first gives a different register plan.
    switch (tag) {
    case 0x64:
    case 0x68:
        contactObjectVelocity = ((TireContactOwner*)other->ownerObject)->field_0x64;
        break;
    case 0x2711:
        contactObjectVelocity = ((TireContactOwner*)other->ownerObject)->field_0x224;
        break;
    case 0x69:
        contactObjectVelocity = ((TireContactOwner*)other->ownerObject)->field_0x40;
        break;
    case 0x65:
        hasContactObjectVelocity = 0;
        return;
    default:
        hasContactObjectVelocity = 0;
        return;
    }
    hasContactObjectVelocity = 1;
}

Tire::~Tire()
{
    if (field_0x2b0)
        delete field_0x2b0;
    else if (field_0x2ac)
        delete field_0x2ac;
}

// Slot 8 (base GameObject::slot 8 stores its argument in field_0x18 and returns this).
// Tier 3 semantics: sets up the collision shape as a half-circle polyline of 8 points in
// the wheel's y/z plane, radius field_0x274 (0x274 is the tire radius: the ctor stores the
// y extent of the wheel node there), running from angle pi/2 to 3*pi/2:
//   angle = i/7 * pi + pi/2,  point = (0, -cos(angle)*r, sin(angle)*r)
// Each element is 24 bytes: a zero vector followed by the point.  The shape is installed
// with CollisionObject 0x00432ab0 (type 2), the owner pointer (field_0x60) is this tire
// and field_0x88/field_0x8c get the contact callback and no second callback.
GameObject* Tire::SetRenderContext(int a)
{
    struct TirePoint {
        CollisionVec3 origin;
        CollisionVec3 rim;
    };
    TirePoint pts[8];

    Configure(a, 1, 1, 1);
    // The zero vector is built from a zero float (not three literals): VC6 then keeps the
    // zero in registers across the loop instead of reloading it from the stack.
    float zeroValue = 0.0f;
    int i = 0;
    CollisionVec3 zero;
    zero.x = zeroValue;
    zero.y = zeroValue;
    zero.z = zeroValue;
    for (; i < 8; i++) {
        float t = i * 0.142857149f;
        float angle = t * TIRE_PI + 1.57079637f;
        pts[i].origin = zero;
        pts[i].rim.x = 0.0f;
        pts[i].rim.y = -cos(angle) * wheelRadius;
        pts[i].rim.z = sin(angle) * wheelRadius;
    }
    SetMeshShape(8, pts);
    CollisionObject::onHitCallback = TireCollisionCallback;
    CollisionObject::onHitByCallback = 0;
    ownerObject = this;
    return this;
}

// v * s with all three products computed before the result is stored: retail
// stores the x component before the z product is spilled (0x00515b50, 0x005135f0),
// where the CollisionTypes.h operator* stores x and y after it.
static inline CollisionVec3 TireScaled(const CollisionVec3& v, float s)
{
    float x = v.x * s;
    float y = v.y * s;
    float z = v.z * s;
    CollisionVec3 r;
    r.x = x;
    r.y = y;
    r.z = z;
    return r;
}

// CollisionPoint slot 1 override (0x00515b50, compiled with this = the CollisionPoint
// subobject at +0xb8, hence the +0xb8 Tire offsets 0x230.. behind CollisionPoint-relative
// disassembly displacements).  The base implementation (0x0043b240) computes the contact
// force magnitude field_0x84 = -friction(field_0x88) * field_0x8c * field_0x74 and applies
// it along the tangent field_0x50.  The tire replaces the single friction coefficient by an
// anisotropic one (tier 3 reading):
//   cosA = |field_0x50 . field_0x230|      (tangent against the tire's rolling axis)
//   sinA = sqrt(1 - cosA^2)
//   force = -(mu_a*cosA + mu_b*sinA) * field_0x74 * field_0x8c,  mu_a = field_0x270*field_0x1d0,
//                                                               mu_b = field_0x1cc*field_0x1d4
// The force vector field_0x78 = field_0x50 * force and field_0x84 is stored as |force|.
// The cos/sin pair is kept in field_0x28c / field_0x288.
void Tire::CollisionPointVirtualSlot1()
{
    if (!(CollisionPoint::tangentSpeed > 0.001f) && !(CollisionPoint::spinSpeed > 0.001f)) {
        CollisionPoint::frictionMagnitude = 0.0f;
        CollisionPoint::frictionForce = kVec3Zero;
    } else {
        float c = TireDotInline(CollisionPoint::frictionDirection, rollDirection);
        if (c < 0.0f)
            c = -c;
        if (c >= 1.0f)
            c = 1.0f;
        tangentCos = c;
        tangentSin = sqrt(1.0f - c * c);
        float rollTerm = (-rollFriction) * rollFrictionScale * tangentCos * CollisionPoint::frictionCoefficient;
        CollisionPoint::frictionMagnitude = (rollTerm + (-sideFriction) * sideFrictionScale * tangentSin
                                                        * CollisionPoint::frictionCoefficient)
                                     * CollisionPoint::surfaceGrip;
        CollisionPoint::frictionForce = TireScaled(CollisionPoint::frictionDirection, CollisionPoint::frictionMagnitude);
        if (CollisionPoint::frictionMagnitude < 0.0f)
            CollisionPoint::frictionMagnitude = -CollisionPoint::frictionMagnitude;
    }
}

// Length from a squared length, as inlined in 0x005135f0 (1.0 is special-cased).
static inline float TireLengthFromSq(float lenSq)
{
    if (lenSq == 1.0f)
        return 1.0f;
    return FastSqrt(lenSq);
}

// v / len through the out-of-line constructor; the reciprocal is taken from the
// loaded length (`fld len; fdivr 1.0`).
static inline CollisionVec3 TireDivide(const CollisionVec3& v, float len)
{
    float inv = 1.0f / len;
    return CollisionVec3(v.x * inv, v.y * inv, v.z * inv);
}

// The part of v along the unit normal n.
static inline CollisionVec3 TireProjectOnto(const CollisionVec3& v, const CollisionVec3& n)
{
    float k = TireDotInline(v, n);
    CollisionVec3 p;
    p.x = k * n.x;
    p.y = k * n.y;
    p.z = k * n.z;
    return p;
}

// a + b, again with the sums computed before the stores.
static inline CollisionVec3 TireSum(const CollisionVec3& a, const CollisionVec3& b)
{
    float x = a.x + b.x;
    float y = a.y + b.y;
    float z = a.z + b.z;
    CollisionVec3 r;
    r.x = x;
    r.y = y;
    r.z = z;
    return r;
}

// v * s through local copies of v.x and v.y: the copies are newer leaves, so VC6
// loads each component first and multiplies by the held s (`fld mem; fmul st(1)`).
static inline CollisionVec3 TireScaleCopy(const CollisionVec3& v, float s)
{
    CollisionVec3 r;
    float x = v.x;
    r.x = x * s;
    float y = v.y;
    r.y = y * s;
    r.z = v.z * s;
    return r;
}

// 0x005135f0.  Semantics are tier 3.  `this` is the complete Tire; the CollisionPoint
// members are addressed through the subobject (+0xb8).
//   field_0x38 = contact position - *pos, field_0x5c = axis x field_0x38;
//   field_0x218 = (n (axis . n)) x field_0x38 with n = the contact normal field_0x2c.
// When minLength > 0.001, `ref` is rejected from the normal into `tangent` (0x0043b190)
// and renormalised, field_0x90 = tangent . ref and the slip vector field_0x1e8 =
// field_0x90 * tangent + field_0x218; otherwise (or when that dot is <= 0.001, which
// also zeroes field_0x90) field_0x1e8 = field_0x218.
// field_0x50 is field_0x1e8 normalised, field_0x94 its length, and the outputs report
// |field_0x50 . field_0x230| scaled by that length in field_0x27c plus its sign.
// Near miss, 1584 of 1664 bytes. `tangent` is a local so VC6 homes it (and the
// projection) in the dead `axis` argument slot like retail; `(-c) * x` keeps retail's
// fchs-then-fmul where `-c * x` lets VC6 hoist c * x above the branch. The slip
// vector is scaled through TireScaleCopy (retail's `fld mem; fmul st(1)`) and summed
// and normalised through TireSum/TireScaled (retail's copy-store schedule). Left:
// the lever's `lea edi` and store are scheduled later, the second TireDot push comes
// before the y product instead of after it (the d-path dot's x term is written
// `ref.x * tangent.x`, which loads retail's operand first).
// No gain from a products-first cross or projection for the lever
// (1571), a named lever or projection local (1575), a pointer or reference alias of
// the lever member (767), a TireScaleCopy that copies all three components or none (1548..1555),
// or the d-path dot written out in any order (1133..1137) or through a helper with
// copied or swapped x operands (1575..1582).
void Tire::UpdateContactPatch(const CollisionVec3* pos, CollisionVec3 axis, CollisionVec3 ref,
                              float minLength, float* outValue, int* outSign)
{
    CollisionVec3 rel;
    rel.x = this->CollisionPoint::worldPosition.x - pos->x;
    rel.y = this->CollisionPoint::worldPosition.y - pos->y;
    rel.z = this->CollisionPoint::worldPosition.z - pos->z;
    this->CollisionPoint::relativePosition = rel;

    this->CollisionPoint::field_0x5c = CollisionCross(axis, this->CollisionPoint::relativePosition);

    normalLeverCross = CollisionCross(TireProjectOnto(axis, this->CollisionPoint::surfaceNormal),
                                      this->CollisionPoint::relativePosition);

    if (minLength > 0.001f) {
        CollisionVec3 tangent;
        if (CollisionRejectFrom(&tangent, &ref, &this->CollisionPoint::surfaceNormal)) {
            CollisionVec3 n;
            tangent = *Vec3Normalize(&n, &tangent);
        }
        float d = tangent.y * ref.y + ref.x * tangent.x + tangent.z * ref.z;
        this->CollisionPoint::tangentSpeed = d;
        if (d > 0.001f) {
            if (reportContactOutputs)
                *outValue = d;
            slipVector = TireSum(TireScaleCopy(tangent, this->CollisionPoint::tangentSpeed), normalLeverCross);
            float lenSq = TireDot(&slipVector, &slipVector);
            float len;
            if (lenSq == 0.0f)
                len = 0.0f;
            else if (lenSq == 1.0f)
                len = 1.0f;
            else
                len = 1.0f / FastInvSqrt(lenSq);
            float inv = 1.0f / len;
            this->CollisionPoint::frictionDirection = TireScaled(slipVector, inv);
            lenSq = TireDot(&normalLeverCross, &normalLeverCross);
            this->CollisionPoint::spinSpeed = TireLengthFromSq(lenSq);
            float c = TireDotInline(this->CollisionPoint::frictionDirection, rollDirection);
            if (c >= 0.0f) {
                slipSpeed = TireLengthFromSq(TireDot(&slipVector, &slipVector)) * c;
                if (reportContactOutputs)
                    *outSign = 1;
            } else {
                slipSpeed = (-c) * TireLengthFromSq(TireDot(&slipVector, &slipVector));
                if (reportContactOutputs)
                    *outSign = 0;
            }
            return;
        }
        // d <= 0.001: same tail as the short-axis case below, with the dot product inlined.
        this->CollisionPoint::tangentSpeed = 0.0f;
        slipVector = normalLeverCross;
        float len = TireLengthFromSq(TireDot(&slipVector, &slipVector));
        this->CollisionPoint::spinSpeed = len;
        if (reportContactOutputs)
            *outValue = len;
        if (this->CollisionPoint::spinSpeed > 0.001f) {
            this->CollisionPoint::frictionDirection = TireDivide(slipVector, this->CollisionPoint::spinSpeed);
        } else {
            this->CollisionPoint::frictionDirection = kVec3Zero;
            this->CollisionPoint::spinSpeed = 0.0f;
        }
        float c = TireDotInline(this->CollisionPoint::frictionDirection, rollDirection);
        if (c >= 0.0f) {
            slipSpeed = c * this->CollisionPoint::spinSpeed;
            if (reportContactOutputs)
                *outSign = 1;
        } else {
            slipSpeed = (-c) * this->CollisionPoint::spinSpeed;
            if (reportContactOutputs)
                *outSign = 0;
        }
    } else {
        this->CollisionPoint::tangentSpeed = 0.0f;
        slipVector = normalLeverCross;
        float len = TireLengthFromSq(TireDot(&slipVector, &slipVector));
        this->CollisionPoint::spinSpeed = len;
        if (reportContactOutputs)
            *outValue = len;
        if (this->CollisionPoint::spinSpeed > 0.001f) {
            this->CollisionPoint::frictionDirection = TireDivide(slipVector, this->CollisionPoint::spinSpeed);
        } else {
            this->CollisionPoint::frictionDirection = kVec3Zero;
            this->CollisionPoint::spinSpeed = 0.0f;
        }
        float c = TireDot(&this->CollisionPoint::frictionDirection, &rollDirection);
        if (c >= 0.0f) {
            slipSpeed = c * this->CollisionPoint::spinSpeed;
            if (reportContactOutputs)
                *outSign = 1;
        } else {
            slipSpeed = (-c) * this->CollisionPoint::spinSpeed;
            if (reportContactOutputs)
                *outSign = 0;
        }
    }
}

// ---------------------------------------------------------------------------------------
// 0x00514550: Tire::UpdateSuspensionProbe (4258 bytes).  Everything here is tier 3.

// Out-of-line 0x00515600 (cdecl, hidden result): a x b.
CollisionVec3 TireCross(const CollisionVec3& a, const CollisionVec3& b);
// 0x0053304c (cdecl): out = function(a, b, t); used with the two global vectors and the angle.
void TireVectorBlend(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b, float t);
// 0x0042a510 / 0x0042a580 (cdecl): transform a point / a direction by a matrix.
void TireTransformPoint(CollisionVec3* out, CollisionVec3 v, const CollisionMatrix4* m);
void TireTransformDirection(CollisionVec3* out, const CollisionVec3* v, const CollisionMatrix4* m);
void TireBuildFrame(CollisionMatrix4* out, const CollisionVec3* pos, const CollisionVec3* zAxis,
                    const CollisionVec3* yAxis, int orthogonalize, int keepZ);

// Wheel/owner record reached through MovingPart::field_0x44 (first member is the node).
struct TireOwnerRef {
    TireNode* node;
};
// Record behind CollisionPoint::field_0xc0 (offset 0xa4 -> per-surface-kind tables).
struct TireSurfaceTables {
    char pad[0x3a0];
    float gripScale[16];       // 0x3a0
    float secondScale[16];     // 0x3e0
};
struct TireSurfaceRef {
    char pad[0xa4];
    TireSurfaceTables* tables;
};
// Bounds record behind CollisionObject::field_0x5c: scale at +0, box pointer at +4 (min[3], max[3]),
// and +8 a normal.
struct TireBox {
    float min[3];
    float max[3];
};
struct TireBoundsRef {
    float scale;
    TireBox* box;
    CollisionVec3 normal;
};

// Normalises v; an all-zero vector yields the fallback, a zero squared length the zero vector.
static inline CollisionVec3 TireNormalizeOr(const CollisionVec3& v, const CollisionVec3& fallback)
{
    if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f)
        return fallback;
    float lenSq = TireDot(&v, &v);
    if (lenSq == 0.0f)
        return kVec3Zero;
    float inv = FastInvSqrt(lenSq);
    CollisionVec3 r;
    r.x = v.x * inv;
    r.y = v.y * inv;
    r.z = v.z * inv;
    return r;
}

void Tire::UpdateSuspensionProbe(TireWorld* world, const CollisionVec3* velocity, float angle,
                                 float radiusScale, float a5, const CollisionVec3* a6,
                                 const CollisionVec3* a7, TireNode* a8)
{
    CollisionVec3& origin = wheelCenter;                 // wheel centre in world space
    CollisionVec3& prevPos = CollisionPoint::worldPosition;
    CollisionVec3& curPos = CollisionPoint::surfacePosition;
    CollisionVec3& normal = CollisionPoint::surfaceNormal;
    float& depth = CollisionPoint::penetration;
    unsigned char* surface = (unsigned char*)&this->CollisionPoint::surfaceType;
    CollisionVec3& up = wheelUpAxis;
    CollisionVec3& forward = rollDirection;
    CollisionVec3& side = sideAxis;

    MovingPart::sceneNode->GetPositionIn(0, &origin);
    curPos = origin;
    world->QueryGround(&curPos, &normal, 0, surface);

    if (MovingPart::ownerRef) {
        TireOwnerRef* owner = (TireOwnerRef*)MovingPart::ownerRef;
        CollisionVec3 t0;
        CollisionVec3 t1;
        up = *owner->node->LocalToWorldDirection(&t0, &kVec3ZAxis);
        CollisionVec3 b = *owner->node->LocalToWorldDirection(&t1, &kVec3YAxis);
        side = TireCross(up, b);
    } else {
        up = *a6;
        if (a7 == 0) {
            CollisionVec3 t0;
            CollisionVec3 b = *a8->LocalToWorldDirection(&t0, &kVec3YAxis);
            side = TireCross(up, b);
        } else {
            side = TireCross(up, *a7);
        }
    }
    float lenSq = side.x * side.x + side.y * side.y + side.z * side.z;
    if (lenSq == 0.0f) {
        side = kVec3Zero;
    } else {
        float inv = FastInvSqrt(lenSq);
        side.x = side.x * inv;
        side.y = side.y * inv;
        side.z = side.z * inv;
    }

    forward = TireNormalizeOr(TireCross(normal, side), up);

    CollisionVec3 lateral = CollisionCross(forward, side);
    if (lateral.x * normal.x + lateral.y * normal.y + lateral.z * normal.z > 0.0f) {
        lateral.x = lateral.x * -1.0f;
        lateral.y = lateral.y * -1.0f;
        lateral.z = lateral.z * -1.0f;
        side.x = side.x * -1.0f;
        side.y = side.y * -1.0f;
        side.z = side.z * -1.0f;
    }

    // First probe: the wheel centre pushed out along the lateral axis by field_0x274.
    CollisionVec3 p = lateral * wheelRadius + origin;
    prevPos = p;
    curPos = p;
    CollisionVec3 n1;
    world->QueryGround(&curPos, &n1, 0, surface);
    depth = (curPos.y - prevPos.y) * n1.y;

    // Second probe position pt: on the wheel rim, in the direction selected by `angle`.
    CollisionVec3 pt;
    float mag = angle;
    if (angle < 0.0f)
        mag = -mag;
    if (mag == 1.5707964f) {
        pt = *a7 * -wheelRadius + origin;
    } else if (MovingPart::ownerRef) {
        TireOwnerRef* owner = (TireOwnerRef*)MovingPart::ownerRef; (void)owner;
        CollisionVec3 base = up * wheelRadius + origin;
        CollisionVec3 c;
        c.x = kVec3YAxis.y * up.z - kVec3YAxis.z * up.y;
        c.y = kVec3YAxis.z * up.x - kVec3YAxis.x * up.z;
        c.z = kVec3YAxis.x * up.y - kVec3YAxis.y * up.x;
        float cLenSq = c.x * c.x + c.y * c.y + c.z * c.z;
        if (cLenSq == 0.0f) {
            c = kVec3Zero;
        } else {
            float inv = FastInvSqrt(cLenSq);
            c.x = c.x * inv;
            c.y = c.y * inv;
            c.z = c.z * inv;
        }
        float s = radiusScale * wheelRadius;
        pt = c * s + base;
    } else {
        CollisionVec3 v;
        TireVectorBlend(&v, &kVec3ZAxis, &kVec3YAxis, a5);
        CollisionVec3 base = v * -wheelRadius + origin;
        CollisionVec3 c;
        c.x = v.y * kVec3YAxis.z - v.z * kVec3YAxis.y;
        c.y = v.z * kVec3YAxis.x - v.x * kVec3YAxis.z;
        c.z = v.x * kVec3YAxis.y - v.y * kVec3YAxis.x;
        float cLenSq = TireDot(&c, &c);
        if (cLenSq == 0.0f) {
            c = kVec3Zero;
        } else {
            float inv = FastInvSqrt(cLenSq);
            c.x = c.x * inv;
            c.y = c.y * inv;
            c.z = c.z * inv;
        }
        float s = radiusScale * wheelRadius;
        pt = c * s + base;
    }

    // Second probe at pt, then a refinement using the axes found there.
    CollisionVec3 n2;
    unsigned char kind;
    world->QueryGround(&pt, &n2, 0, &kind);
    CollisionVec3 v = TireNormalizeOr(TireCross(n2, side), up);
    CollisionVec3 u = CollisionCross(v, side);
    CollisionVec3 lateral2 = u;
    if (u.x * n2.x + u.y * n2.y + u.z * n2.z > 0.0f) {
        lateral2.x = u.x * -1.0f;
        lateral2.y = u.y * -1.0f;
        lateral2.z = u.z * -1.0f;
    }

    CollisionVec3 scaled = CollisionVec3(lateral2.x * wheelRadius, lateral2.y * wheelRadius, lateral2.z * wheelRadius);
    CollisionVec3 r = CollisionVec3(scaled.x + origin.x, scaled.y + origin.y, scaled.z + origin.z);
    pt = r;
    CollisionVec3 n3;
    world->QueryGround(&pt, &n3, 0, &kind);
    float delta = pt.y - r.y;
    float score = delta * n3.y;
    if (score > depth) {
        prevPos = r;
        depth = score;
        curPos = pt;
        *surface = kind & 7;
        lateral = lateral2;
        if (n2.y != n3.y || n2.x != n3.x || n2.z != n3.z) {
            normal = n3;
            forward = TireNormalizeOr(TireCross(n3, side), up);
        } else {
            normal = n2;
            depth = delta * n2.y;
            forward = v;
        }
    } else {
        *surface = *surface & 7;
        if (normal.y != n1.y || normal.x != n1.x || normal.z != n1.z) {
            normal = n1;
            forward = TireNormalizeOr(TireCross(n1, side), up);
        } else {
            depth = (curPos.y - prevPos.y) * normal.y;
        }
    }

    CollisionMatrix4 frame;
    TireBuildFrame(&frame, &origin, &up, &lateral, 1, 1);
    SetTransform((const Matrix4*)&frame);   // CollisionObject 0x00435830
    QueryCollisions();

    if (CollisionObject::hasContact) {
        TireBoundsRef* bounds = (TireBoundsRef*)CollisionObject::contactRecord;
        float s = (1.0f - bounds->scale) * wheelRadius;
        if (s > depth) {
            depth = s;
            TireBox* box = bounds->box;
            CollisionVec3 ext = CollisionVec3(box->max[0] - box->min[0], box->max[1] - box->min[1], box->max[2] - box->min[2]);
            CollisionVec3 e2 = CollisionVec3(ext.x * bounds->scale, ext.y * bounds->scale, ext.z * bounds->scale);
            CollisionVec3 e3 = CollisionVec3(e2.x + box->min[0], e2.y + box->min[1], e2.z + box->min[2]);
            TireTransformPoint(&curPos, e3, &frame);
            normal = bounds->normal;
            *surface = 7;
            TireTransformDirection(&prevPos, (const CollisionVec3*)box->max, &frame);
            CollisionVec3 t;
            forward = *Fn_00515c90(&t, &normal, &side, &up);
        }
    }

    if (depth < 0.0f && depth > -0.01f)
        depth = 0.0f;
    inContact = (depth >= CollisionPoint::penetrationThreshold) ? 1 : 0;
    wheelVelocity = *velocity;
    if (CollisionPoint::surfaceOwner) {
        TireSurfaceRef* ref = (TireSurfaceRef*)CollisionPoint::surfaceOwner;
        CollisionPoint::surfaceGrip = ref->tables->gripScale[*surface];
        surfaceScaleB = ref->tables->secondScale[*surface];
    } else {
        CollisionPoint::surfaceGrip = 1.0f;
        surfaceScaleB = 1.0f;
    }
    reportContactOutputs = 0;
    field_0x268 = 0;
    CollisionPoint::inContact = 0;
    CollisionPoint::tangentSpeed = 0.0f;
    CollisionPoint::spinSpeed = 0.0f;
}

// ---------------------------------------------------------------------------------------
// Constructor 0x00512f10.  CollisionObject(1), MovingPart, then CollisionPoint whose constructor
// is inlined here (it is only declared in CollisionPoint.h).  Same body as the one in
// CollisionPoint.cpp but with this translation unit's zero/unit vector globals (tier 2:
// the stores read 0x0068a3d0 / 0x0068a3f0, not the collision globals).
inline CollisionPoint::CollisionPoint(float a, int b)
{
    normalForce = a;
    surfaceOwner = b;
    localPosition = kVec3Zero;
    ownerNode = 0;
    worldPosition = kVec3Zero;
    surfacePosition = kVec3Zero;
    surfaceNormal = kVec3YAxis;
    relativePosition = kVec3Zero;
    field_0x44 = kVec3Zero;
    frictionDirection = kVec3Zero;
    field_0x5c = kVec3Zero;
    tangentSpeed = 0;
    spinSpeed = 0;
    penetration = -999.0f;
    penetrationThreshold = 0;
    inContact = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0xb4 = 0;
    field_0xa0 = 0.5f;
    field_0xb8 = 1.0f;
    surfaceType = 0;
    surfaceGrip = 1.0f;
    frictionMagnitude = 0;
    frictionForce = kVec3Zero;
    frictionCoefficient = 0;
    field_0x68 = kVec3Zero;
}

Tire::Tire(void* a1, int a2, float a3, float a4, int a5, int a6, float a7, float a8,
           float a9, float a10, float a11, float a12, float a13, float a14, int a15)
    : CollisionObject(1), MovingPart(a1, a2, a6), CollisionPoint(a4, a15)
{
    CollisionVec3 extentA;
    CollisionVec3 extentB;
    MovingPart::sceneNode->GetLocalBounds(&extentA, &extentB);
    wheelRadius = extentB.y;
    invWheelRadius = 1.0f / extentB.y;
    rollAngle = 0.0f;
    MovingPart::sceneNode->GetPositionIn(0, &wheelCenter);
    slipVector = kVec3Zero;
    inContact = 0;
    wheelVelocity = kVec3Zero;
    rollDirection = kVec3Zero;
    sideAxis = kVec3Zero;
    field_0x248 = kVec3Zero;
    field_0x280 = 0;
    field_0x254 = kVec3Zero;
    tangentCos = 0;
    rollFriction = a3;
    sideFriction = a4;
    field_0x294 = a14;
    CollisionPoint::field_0xa0 = a14;
    field_0x298 = a5;
    field_0x2b4 = a7;
    rollFrictionScale = a8;
    sideFrictionScale = a9;
    field_0x2a0 = a10;
    field_0x1d8 = a11;
    tangentSin = 1.0f;
    field_0x2a8 = 0;
    field_0x2ac = 0;
    field_0x2b0 = 0;
    field_0x2b8 = 1.0f;
    field_0x2bc = 0;
    reportContactOutputs = 0;
    field_0x268 = 0;
    rampLevel = 0;
    field_0x1dc = a12;
    field_0x1e0 = a13;
    field_0x290 = 0;
    wheelUpAxis = kVec3ZAxis;
    surfaceScaleB = 1.0f;
    field_0x26c = 0;
    contactObjectVelocity = kVec3Zero;
    hasContactObjectVelocity = 0;
    field_0x2a4 = 1.0f;
    normalLeverCross = kVec3Zero;
}

// 0x00513560
void Tire::SetRollDistance(float distance)
{
    rollAngle = distance * invWheelRadius;
    if (_finite(rollAngle))
        MovingPart::sceneNode->RotateAbout(1.0f, 0.0f, 0.0f, rollAngle);
}

// 0x005135b0
void Tire::ApplyRollAngle()
{
    if (_finite(rollAngle))
        MovingPart::sceneNode->RotateAbout(1.0f, 0.0f, 0.0f, rollAngle);
}

// 0x00515660
void Tire::UpdateAttachment()
{
    if (TireAttachA* slider = field_0x2b0) {
        float offset = -slider->position;
        slider->node->SetPosition(TireVec3(offset * slider->direction.x,
                                           offset * slider->direction.y,
                                           offset * slider->direction.z));
    } else if (TireAttachB* hinge = field_0x2ac) {
        float angle = hinge->position / hinge->scale;
        if (_finite(angle)) {
            hinge->node->SetLocalMatrix(&hinge->restMatrix);
            hinge->node->Rotate(hinge->axis, angle);
        }
    }
}

// Inline form of the cross product (the inline dot is TireDotInline above).
static inline CollisionVec3 TireCross(const CollisionVec3& a, const CollisionVec3& b)
{
    CollisionVec3 r;
    r.x = b.z * a.y - b.y * a.z;
    r.y = b.x * a.z - b.z * a.x;
    r.z = b.y * a.x - b.x * a.y;
    return r;
}

// 0x00515c90 (exact, 292 bytes): the normalised cross product a x b; the
// fallback when it is exactly zero (this is unused). Source shapes that give
// retail's leaf ages (docs/VC6_OPERAND_ORDER.md section 1.2): the reference
// `av` for a.y in the first product puts the y and z products' b components
// first; assigning n after its declaration instead of initialising it gives
// retail's esi/ecx/edx result registers; testing n's components for zero
// makes y and z older than the FastInvSqrt result, so their scales load the
// scalar first and the last one consumes it. The x scale reads its component
// through a local stored after the call (`nx`), a leaf newer than the scalar,
// so it loads the component first (`fld c.x; fmul st(1)`) like retail; every
// spelling of `n.x * inv`, inline float helpers by value or reference and
// helper-based zero tests kept `fld st(0); fmul c.x` (287).
CollisionVec3* Tire::Fn_00515c90(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b,
                                 const CollisionVec3* fallback)
{
    const CollisionVec3& av = *a;
    CollisionVec3 c;
    c.x = (av.y * b->z) - a->z * b->y;
    c.y = (a->z * b->x) - a->x * b->z;
    c.z = (a->x * b->y) - a->y * b->x;
    CollisionVec3 n;
    n = c;
    if (n.x == 0.0f && n.y == 0.0f && n.z == 0.0f) {
        n = *fallback;
    } else {
        float lenSq = TireDotInline(n, n);
        if (lenSq == 0.0f) {
            n = kVec3Zero;
        } else {
            float inv = FastInvSqrt(lenSq);
            float nx = n.x;
            n.x = nx * inv;
            n.y *= inv;
            n.z *= inv;
        }
    }
    *out = n;
    return out;
}

// 0x005143d0
void Tire::UpdateRoll(float dt, float distanceScale, int locked, int driven, int a5, float driveScale)
{
    if (locked) {
        if (rampLevel != 0.0f) {
            rollAngle = (1.0f - rampLevel) * rollAngle;
            ApplyRollAngle();
            return;
        }
        if (field_0x2a8 && field_0x2a8->value < 1.0f && field_0x2a8->value != 0.0f)
            rollAngle = field_0x2a8->value * 6.0f;
        ApplyRollAngle();
    } else if (driven) {
        if (field_0x2a8) {
            float drive = field_0x2bc * field_0x280 * dt * driveScale;
            field_0x290 = drive;
            if (slipSpeed > 0.1f)
                SetRollDistance(distanceScale * slipSpeed + drive);
        } else if (slipSpeed > 0.1f) {
            SetRollDistance(distanceScale * slipSpeed);
        }
    } else if (inContact) {
        if (field_0x2a8) {
            float drive = field_0x2bc * field_0x280 * dt * driveScale;
            field_0x290 = drive;
            SetRollDistance(distanceScale * slipSpeed + drive);
        } else {
            SetRollDistance(distanceScale * slipSpeed);
        }
    } else {
        if (field_0x2a8 && field_0x2a8->value < 1.0f && field_0x2a8->value != 0.0f)
            rollAngle = field_0x2a8->value * 6.0f;
        ApplyRollAngle();
    }
}

// Length from a squared length (1.0 is special-cased), with the table square root.
static inline float TireLength(const CollisionVec3& v)
{
    float lenSq = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (lenSq == 1.0f)
        return 1.0f;
    return FastSqrt(lenSq);
}

// In-place scale through a reference.
static inline void TireScale(CollisionVec3& v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;
}

// 0x00513f90: exact. Retail keeps `boost` in the dead argument slot. With a
// single `&&` condition boost has one IR store fewer than `minimum` and loses the
// slot; the nested if (outer test on field_0xa4, inner test on the product, a
// `boost = 1.0f` store in each else) gives boost four references like minimum,
// the tie goes to the earlier last reference, and VC6 tail-merges the duplicate
// stores, so only the frame moves.
void Tire::UpdateDriveShare(TireVehicle* vehicle)
{
    if (!field_0x2a8)
        return;
    if (slipSpeed < field_0x2b4 && !vehicle->field_0x444) {
        float boost;
        if (vehicle->field_0xa4 > 0.0f) {
            if (vehicle->field_0x60 * 1.25f > 1.0f)
                boost = vehicle->field_0x60 * 1.25f;
            else
                boost = 1.0f;
        } else {
            boost = 1.0f;
        }
        float grip = vehicle->UnknownVirtualSlot47(this);
        float excess = field_0x280 * vehicle->field_0x24 - TireLength(vehicle->field_0x70);
        float share;
        if (excess < 1.01f) {
            share = 1.0f;
        } else {
            if (!(excess < 320.0f))
                excess = 320.0f;
            share = (400.0f - excess) * 0.0025f;
        }
        float minimum;
        if (slipSpeed > field_0x2b4)
            minimum = 1.0f;
        else
            minimum = 1.0f - (field_0x2b4 - slipSpeed) / field_0x2b4;
        share = share * grip * boost;
        if (!(share < 1.0f))
            share = 1.0f;
        if (!(share > minimum))
            share = minimum;
        field_0x2b8 = share * surfaceScaleB;
        TireScale(field_0x248, field_0x2b8);
        field_0x2bc = 1.0f - field_0x2b8;
    } else {
        field_0x2b8 = 1.0f;
        field_0x2bc = 0.0f;
    }
}

static inline CollisionVec3 TireTimes(float s, const CollisionVec3& v)
{
    CollisionVec3 r;
    r.x = s * v.x;
    r.y = s * v.y;
    r.z = s * v.z;
    return r;
}

static inline CollisionVec3 TireNegate(const CollisionVec3& v)
{
    CollisionVec3 r;
    r.x = -v.x;
    r.y = -v.y;
    r.z = -v.z;
    return r;
}

// Scales v by numerator / denominator for ApplyDrive.  The x product uses the
// quotient expression and y and z multiply copies of their components by a named
// quotient: VC6 then keeps the fresh quotient for x (`fld st(0); fmul mem`) and
// loads y and z first (`fld mem; fmul st(1)`), as retail does.
static inline CollisionVec3 TireRatioTimes(float numerator, float denominator,
                                           const CollisionVec3& v)
{
    CollisionVec3 r;
    r.x = numerator / denominator * v.x;
    float ratio = numerator / denominator;
    float y = v.y;
    float z = v.z;
    r.y = y * ratio;
    r.z = z * ratio;
    return r;
}

// 0x00514170 (near miss, 565 of 607 bytes): clamping as `*speed < amount` gives
// retail's `fcom`, and scaling the cross product component-wise through a local
// copy of +0x1d8 gives its moment. TireRatioTimes gives the scaled push's x87
// operand order (560 -> 565). Left: the copy of push.y out of the return temp and
// two cross-product fsubp/mov pairs are scheduled differently.
// Before TireRatioTimes (560): `dir * k`, a local k, k * dir.x per component, a
// pre-copied push, `1 / grip * needed`, `v.x * s` operand order (all 8 orders) in
// TireTimes, a helper copying v.x into a local or into r first, a `dx` copy before
// the grip test, the negation per component or as `forward ?:` / if-else (114..149),
// direct push.x stores (336), the quotient as a CSE expression in all three
// products (361) or named first (553..560).
void Tire::ApplyDrive(float share, float stepTime, int forward, float mass, float* speed,
                      CollisionVec3* torque, CollisionVec3* force)
{
    if (!inContact || field_0x2a0 == 0.0f || rampLevel == 0.0f || slipSpeed == 0.0f)
        return;
    float grip = CollisionPoint::surfaceGrip * field_0x2a0 * rampLevel;
    CollisionVec3 push = rollDirection * -grip;
    CollisionVec3 dir = push;
    float amount = share * slipSpeed;
    if (!forward)
        dir = TireNegate(push);
    amount = *speed < amount ? *speed : amount;
    amount = amount > 0.0f ? amount : 0.0f;
    float rate = mass / stepTime;
    float needed = rate * amount;
    if (grip > needed) {
        push = TireRatioTimes(needed, grip, dir);
        *speed -= amount;
    } else {
        *speed -= grip / rate;
        push = dir;
    }
    force->x += push.x;
    force->y += push.y;
    force->z += push.z;
    CollisionVec3 moment = CollisionCross(CollisionPoint::relativePosition, push);
    float s = field_0x1d8;
    moment.x *= s;
    moment.y *= s;
    moment.z *= s;
    torque->x += moment.x;
    torque->y += moment.y;
    torque->z += moment.z;
}

// 0x00513c70 (near miss, 769 of 797 bytes, same length): runs the attached
// shock against the contact. The inline (+0x2b0) and rotating (+0x2ac) shocks
// are solved with the same follow-up, written out twice as retail keeps two
// copies; the solve's load and active flag become the contact's penetration and
// inContact. The outputs live in the dead `dt` and `velocity` argument slots,
// as in retail. Each attachment clears field_0x26c and inContact on its own
// failure path (VC6 merges the two tails into retail's single block); with
// the clears written once after the if/else chain VC6 has too few zero stores
// to keep retail's zero register (ebx) across the solve call and compiles
// every null test as `test` (96 of 805 bytes). Left: the rotating path's
// first world-position add loads the attachment's x first where retail loads
// the contact's, and the surface position's z sum is staged in the x slot
// (retail reuses the y slot); TireAddTo-style `d + v` sums swap the inline
// path's first add as well.
void Tire::UpdateShock(float dt, int a2, int crashed, const CollisionVec3* offset,
                       const CollisionVec3* velocity, const CollisionVec3* axisB)
{
    float depth;
    int active;
    float load;
    if (TireAttachA* slider = field_0x2b0) {
        if (inContact) {
            depth = CollisionPoint::penetration;
            if (depth != 0.0f || slider->ratio != 0.0f) {
                slider->SolveContact(dt, velocity, &this->CollisionPoint::field_0x5c, &wheelCenter,
                                     &this->CollisionPoint::surfaceNormal, depth, &load, &active);
                field_0x26c = field_0x2b0->extending;
                if (active || crashed)
                    CollisionPoint::inContact = 1;
                CollisionPoint::penetration = load;
                CollisionPoint::worldPosition.x += field_0x2b0->field_0x80.x;
                CollisionPoint::worldPosition.y += field_0x2b0->field_0x80.y;
                CollisionPoint::worldPosition.z += field_0x2b0->field_0x80.z;
                if (active)
                    CollisionPoint::surfacePosition = TireTimes(load, CollisionPoint::surfaceNormal) + CollisionPoint::worldPosition;
                else
                    CollisionPoint::surfacePosition = CollisionPoint::worldPosition;
                field_0x268 = 1;
                return;
            }
        }
        field_0x26c = 0;
        CollisionPoint::inContact = 0;
        return;
    } else if (TireAttachB* hinge = field_0x2ac) {
        if (inContact) {
            depth = CollisionPoint::penetration;
            if (depth != 0.0f || hinge->ratio != 0.0f) {
                hinge->SolveContact(dt, &sideAxis, axisB, &wheelCenter, &this->CollisionPoint::surfaceNormal,
                                    velocity, &this->CollisionPoint::field_0x5c, depth, &load, &active);
                field_0x26c = field_0x2ac->extending;
                if (active || crashed)
                    CollisionPoint::inContact = 1;
                CollisionPoint::penetration = load;
                CollisionPoint::worldPosition.x += field_0x2ac->field_0x80.x;
                CollisionPoint::worldPosition.y += field_0x2ac->field_0x80.y;
                CollisionPoint::worldPosition.z += field_0x2ac->field_0x80.z;
                if (active)
                    CollisionPoint::surfacePosition = TireTimes(load, CollisionPoint::surfaceNormal) + CollisionPoint::worldPosition;
                else
                    CollisionPoint::surfacePosition = CollisionPoint::worldPosition;
                field_0x268 = 1;
                return;
            }
        }
        field_0x26c = 0;
        CollisionPoint::inContact = 0;
        return;
    } else {
        CollisionPoint::inContact = 1;
    }
}
