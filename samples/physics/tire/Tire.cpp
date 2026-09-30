// Tire.cpp -- Tire (CollisionObject + MovingPart + CollisionPoint), see Tire.h.
#include "Tire.h"
#include <math.h>

#define TIRE_PI 3.14159265358979f

// Callback stored in CollisionObject::field_0x88 by slot 8 (0x00512e30).  a is the
// tire's own CollisionObject (field_0x60 = owner, the Tire), b is the object that was hit.
static void TireCollisionCallback(CollisionObject* a, CollisionObject* b)
{
    Tire* tire = (Tire*)a->field_0x60;
    int tag = b->field_0x64;
    if (tag == 0x64) {
        // virtual call, vtable offset 0xcc, on b's owner
        if (!(*(int (__fastcall**)(void*))(**(int**)&b->field_0x60 + 0xcc))(b->field_0x60))
            tire->HandleContact(1, tag, b);
    } else if (tag != 0x65) {
        tire->HandleContact(1, tag, b);
    }
}

// 0x0040ae30 (cdecl, out of line): dot product of two vectors, result in st(0).
float TireDot(const CollisionVec3* a, const CollisionVec3* b);

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

    float lenSq = y.y * y.y + y.z * y.z + y.x * y.x;
    if (lenSq != 1.0f) {
        float s = FastInvSqrt(lenSq);
        y = CollisionVec3(y.x * s, y.y * s, y.z * s);
    }

    CollisionVec3 n;
    lenSq = TireDot(&z, &z);
    if (lenSq == 1.0f) {
        n = z;
    } else {
        float s = FastInvSqrt(lenSq);
        n.x = z.x * s;
        n.y = z.y * s;
        n.z = z.z * s;
    }

    float tx = y.y * n.z - y.z * n.y;
    float ty = n.x * y.z - y.x * n.z;
    float tz = y.x * n.y - n.x * y.y;
    out->m[0] = tx;
    out->m[1] = ty;
    out->m[2] = tz;
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
    const CollisionVec3* src;
    switch (tag) {
    case 0x64:
    case 0x68:
        src = &((TireContactOwner*)other->field_0x60)->field_0x64;
        break;
    case 0x2711:
        src = &((TireContactOwner*)other->field_0x60)->field_0x224;
        break;
    case 0x69:
        src = &((TireContactOwner*)other->field_0x60)->field_0x40;
        break;
    case 0x65:
        field_0x1e4 = 0;
        return;
    default:
        field_0x1e4 = 0;
        return;
    }
    field_0x1f4 = *src;
    field_0x1e4 = 1;
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
GameObject* Tire::GameObjectVirtualSlot8(int a)
{
    struct TirePoint {
        CollisionVec3 origin;
        CollisionVec3 rim;
    };
    TirePoint pts[8];

    Fn_004320f0(a, 1, 1, 1);
    int i;
    CollisionVec3 zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    for (i = 0; i < 8; i++) {
        float t = i * 0.142857149f;
        float angle = t * TIRE_PI + 1.57079637f;
        pts[i].origin = zero;
        pts[i].rim.x = 0.0f;
        pts[i].rim.y = -cos(angle) * field_0x274;
        pts[i].rim.z = sin(angle) * field_0x274;
    }
    Fn_00432ab0(8, pts);
    CollisionObject::field_0x88 = TireCollisionCallback;
    CollisionObject::field_0x8c = 0;
    field_0x60 = this;
    return this;
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
    if (!(CollisionPoint::field_0x90 > 0.001f) && !(CollisionPoint::field_0x94 > 0.001f)) {
        CollisionPoint::field_0x84 = 0.0f;
        CollisionPoint::field_0x78 = g_TireZeroVec3;
    } else {
        float c = CollisionPoint::field_0x50.y * field_0x230.y + CollisionPoint::field_0x50.x * field_0x230.x
                + CollisionPoint::field_0x50.z * field_0x230.z;
        if (c < 0.0f)
            c = -c;
        if (c >= 1.0f)
            c = 1.0f;
        field_0x28c = c;
        field_0x288 = sqrt(1.0f - c * c);
        CollisionPoint::field_0x84 = ((-field_0x1cc) * field_0x1d4 * field_0x288 * CollisionPoint::field_0x74
                                      + (-field_0x270) * field_0x1d0 * field_0x28c * CollisionPoint::field_0x74)
                                     * CollisionPoint::field_0x8c;
        CollisionPoint::field_0x78 = CollisionPoint::field_0x50 * CollisionPoint::field_0x84;
        if (CollisionPoint::field_0x84 < 0.0f)
            CollisionPoint::field_0x84 = -CollisionPoint::field_0x84;
    }
}

// Length from a squared length, as inlined in 0x005135f0 (1.0 is special-cased).
static inline float TireLengthFromSq(float lenSq)
{
    if (lenSq == 1.0f)
        return 1.0f;
    return FastSqrt(lenSq);
}

// 0x005135f0.  Semantics are tier 3.  `this` is the complete Tire; the CollisionPoint
// members are addressed through the subobject (+0xb8).
//   field_0x38 = contact position - *pos, field_0x5c = axis x field_0x38;
//   field_0x218 = (n (axis . n)) x field_0x38 with n = the contact normal field_0x2c.
// When minLength > 0.001 the axis is first projected off the normal (0x0043b190) and
// renormalised and the tangent vector field_0x1e8 = field_0x90 * axis + field_0x218;
// otherwise (or when the projected dot is <= 0.001) field_0x1e8 = field_0x218.
// field_0x50 is field_0x1e8 normalised, field_0x94 its length, and the outputs report
// |field_0x50 . field_0x230| scaled by that length in field_0x27c plus its sign.
void Tire::UpdateContactPatch(const CollisionVec3* pos, CollisionVec3 axis, CollisionVec3 ref,
                              float minLength, float* outValue, int* outSign)
{
    CollisionVec3 rel;
    rel.x = this->CollisionPoint::field_0x14.x - pos->x;
    rel.y = this->CollisionPoint::field_0x14.y - pos->y;
    rel.z = this->CollisionPoint::field_0x14.z - pos->z;
    this->CollisionPoint::field_0x38 = rel;

    this->CollisionPoint::field_0x5c = CollisionCross(axis, this->CollisionPoint::field_0x38);

    float k = axis.y * this->CollisionPoint::field_0x2c.y + axis.x * this->CollisionPoint::field_0x2c.x + axis.z * this->CollisionPoint::field_0x2c.z;
    axis.x = k * this->CollisionPoint::field_0x2c.x;
    axis.y = k * this->CollisionPoint::field_0x2c.y;
    axis.z = k * this->CollisionPoint::field_0x2c.z;
    field_0x218 = CollisionCross(axis, this->CollisionPoint::field_0x38);

    if (minLength > 0.001f) {
        if (CollisionRejectFrom(&axis, &ref, &this->CollisionPoint::field_0x2c)) {
            CollisionVec3 n;
            axis = *Fn_005087b0(&n, &axis);
        }
        float d = axis.y * ref.y + axis.x * ref.x + axis.z * ref.z;
        this->CollisionPoint::field_0x90 = d;
        if (d > 0.001f) {
            if (field_0x264)
                *outValue = d;
            field_0x1e8.x = this->CollisionPoint::field_0x90 * axis.x + field_0x218.x;
            field_0x1e8.y = this->CollisionPoint::field_0x90 * axis.y + field_0x218.y;
            field_0x1e8.z = this->CollisionPoint::field_0x90 * axis.z + field_0x218.z;
            float lenSq = TireDot(&field_0x1e8, &field_0x1e8);
            float len;
            if (lenSq == 0.0f)
                len = 0.0f;
            else if (lenSq == 1.0f)
                len = 1.0f;
            else
                len = 1.0f / FastInvSqrt(lenSq);
            float inv = 1.0f / len;
            this->CollisionPoint::field_0x50.x = field_0x1e8.x * inv;
            this->CollisionPoint::field_0x50.y = field_0x1e8.y * inv;
            this->CollisionPoint::field_0x50.z = field_0x1e8.z * inv;
            lenSq = TireDot(&field_0x218, &field_0x218);
            this->CollisionPoint::field_0x94 = TireLengthFromSq(lenSq);
            float c = this->CollisionPoint::field_0x50.y * field_0x230.y + this->CollisionPoint::field_0x50.x * field_0x230.x +
                      this->CollisionPoint::field_0x50.z * field_0x230.z;
            lenSq = TireDot(&field_0x1e8, &field_0x1e8);
            if (c >= 0.0f) {
                field_0x27c = TireLengthFromSq(lenSq) * c;
                if (field_0x264)
                    *outSign = 1;
            } else {
                field_0x27c = -c * TireLengthFromSq(lenSq);
                if (field_0x264)
                    *outSign = 0;
            }
            return;
        }
        // d <= 0.001: same tail as the short-axis case below, with the dot product inlined.
        field_0x1e8 = field_0x218;
        float len = TireLengthFromSq(TireDot(&field_0x1e8, &field_0x1e8));
        this->CollisionPoint::field_0x94 = len;
        if (field_0x264)
            *outValue = len;
        if (this->CollisionPoint::field_0x94 > 0.001f) {
            float inv = 1.0f / this->CollisionPoint::field_0x94;
            this->CollisionPoint::field_0x50 = CollisionVec3(field_0x1e8.x * inv, field_0x1e8.y * inv, field_0x1e8.z * inv);
        } else {
            this->CollisionPoint::field_0x50 = g_TireZeroVec3;
            this->CollisionPoint::field_0x94 = 0.0f;
        }
        float c = this->CollisionPoint::field_0x50.y * field_0x230.y + this->CollisionPoint::field_0x50.x * field_0x230.x +
                  this->CollisionPoint::field_0x50.z * field_0x230.z;
        if (c >= 0.0f) {
            field_0x27c = c * this->CollisionPoint::field_0x94;
            if (field_0x264)
                *outSign = 1;
        } else {
            field_0x27c = -c * this->CollisionPoint::field_0x94;
            if (field_0x264)
                *outSign = 0;
        }
    } else {
        this->CollisionPoint::field_0x90 = 0.0f;
        field_0x1e8 = field_0x218;
        float len = TireLengthFromSq(TireDot(&field_0x1e8, &field_0x1e8));
        this->CollisionPoint::field_0x94 = len;
        if (field_0x264)
            *outValue = len;
        if (this->CollisionPoint::field_0x94 > 0.001f) {
            float inv = 1.0f / this->CollisionPoint::field_0x94;
            this->CollisionPoint::field_0x50 = CollisionVec3(field_0x1e8.x * inv, field_0x1e8.y * inv, field_0x1e8.z * inv);
        } else {
            this->CollisionPoint::field_0x50 = g_TireZeroVec3;
            this->CollisionPoint::field_0x94 = 0.0f;
        }
        float c = TireDot(&this->CollisionPoint::field_0x50, &field_0x230);
        if (c >= 0.0f) {
            field_0x27c = c * this->CollisionPoint::field_0x94;
            if (field_0x264)
                *outSign = 1;
        } else {
            field_0x27c = -c * this->CollisionPoint::field_0x94;
            if (field_0x264)
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
        return g_TireZeroVec3;
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
    CollisionVec3& origin = field_0x200;                 // wheel centre in world space
    CollisionVec3& prevPos = CollisionPoint::field_0x14;
    CollisionVec3& curPos = CollisionPoint::field_0x20;
    CollisionVec3& normal = CollisionPoint::field_0x2c;
    float& depth = CollisionPoint::field_0x98;
    unsigned char* surface = (unsigned char*)&this->CollisionPoint::field_0xbc;
    CollisionVec3& up = field_0x20c;
    CollisionVec3& forward = field_0x230;
    CollisionVec3& side = field_0x23c;

    ((TireNode*)MovingPart::field_0x40)->Fn_004fc9a0(0, &origin);
    curPos = origin;
    world->Fn_00507c10(&curPos, &normal, 0, surface);

    if (MovingPart::field_0x44) {
        TireOwnerRef* owner = (TireOwnerRef*)MovingPart::field_0x44;
        CollisionVec3 t0;
        CollisionVec3 t1;
        up = *owner->node->Fn_004fd5c0(&t0, &g_TireVec3_68a3c0);
        CollisionVec3 b = *owner->node->Fn_004fd5c0(&t1, &g_TireVec3_68a3f0);
        side = TireCross(up, b);
    } else {
        up = *a6;
        if (a7 == 0) {
            CollisionVec3 t0;
            CollisionVec3 b = *a8->Fn_004fd5c0(&t0, &g_TireVec3_68a3f0);
            side = TireCross(up, b);
        } else {
            side = TireCross(up, *a7);
        }
    }
    float lenSq = side.x * side.x + side.y * side.y + side.z * side.z;
    if (lenSq == 0.0f) {
        side = g_TireZeroVec3;
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
    CollisionVec3 p = lateral * field_0x274 + origin;
    prevPos = p;
    curPos = p;
    CollisionVec3 n1;
    world->Fn_00507c10(&curPos, &n1, 0, surface);
    depth = (curPos.y - prevPos.y) * n1.y;

    // Second probe position pt: on the wheel rim, in the direction selected by `angle`.
    CollisionVec3 pt;
    float mag = angle;
    if (angle < 0.0f)
        mag = -mag;
    if (mag == 1.5707964f) {
        pt = *a7 * -field_0x274 + origin;
    } else if (MovingPart::field_0x44) {
        TireOwnerRef* owner = (TireOwnerRef*)MovingPart::field_0x44; (void)owner;
        CollisionVec3 base = up * field_0x274 + origin;
        CollisionVec3 c;
        c.x = g_TireVec3_68a3f0.y * up.z - g_TireVec3_68a3f0.z * up.y;
        c.y = g_TireVec3_68a3f0.z * up.x - g_TireVec3_68a3f0.x * up.z;
        c.z = g_TireVec3_68a3f0.x * up.y - g_TireVec3_68a3f0.y * up.x;
        float cLenSq = c.x * c.x + c.y * c.y + c.z * c.z;
        if (cLenSq == 0.0f) {
            c = g_TireZeroVec3;
        } else {
            float inv = FastInvSqrt(cLenSq);
            c.x = c.x * inv;
            c.y = c.y * inv;
            c.z = c.z * inv;
        }
        float s = radiusScale * field_0x274;
        pt = c * s + base;
    } else {
        CollisionVec3 v;
        TireVectorBlend(&v, &g_TireVec3_68a3c0, &g_TireVec3_68a3f0, a5);
        CollisionVec3 base = v * -field_0x274 + origin;
        CollisionVec3 c;
        c.x = v.y * g_TireVec3_68a3f0.z - v.z * g_TireVec3_68a3f0.y;
        c.y = v.z * g_TireVec3_68a3f0.x - v.x * g_TireVec3_68a3f0.z;
        c.z = v.x * g_TireVec3_68a3f0.y - v.y * g_TireVec3_68a3f0.x;
        float cLenSq = TireDot(&c, &c);
        if (cLenSq == 0.0f) {
            c = g_TireZeroVec3;
        } else {
            float inv = FastInvSqrt(cLenSq);
            c.x = c.x * inv;
            c.y = c.y * inv;
            c.z = c.z * inv;
        }
        float s = radiusScale * field_0x274;
        pt = c * s + base;
    }

    // Second probe at pt, then a refinement using the axes found there.
    CollisionVec3 n2;
    unsigned char kind;
    world->Fn_00507c10(&pt, &n2, 0, &kind);
    CollisionVec3 v = TireNormalizeOr(TireCross(n2, side), up);
    CollisionVec3 u = CollisionCross(v, side);
    CollisionVec3 lateral2 = u;
    if (u.x * n2.x + u.y * n2.y + u.z * n2.z > 0.0f) {
        lateral2.x = u.x * -1.0f;
        lateral2.y = u.y * -1.0f;
        lateral2.z = u.z * -1.0f;
    }

    CollisionVec3 scaled = CollisionVec3(lateral2.x * field_0x274, lateral2.y * field_0x274, lateral2.z * field_0x274);
    CollisionVec3 r = CollisionVec3(scaled.x + origin.x, scaled.y + origin.y, scaled.z + origin.z);
    pt = r;
    CollisionVec3 n3;
    world->Fn_00507c10(&pt, &n3, 0, &kind);
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
    Fn_00438e70();

    if (CollisionObject::field_0x58) {
        TireBoundsRef* bounds = (TireBoundsRef*)CollisionObject::field_0x5c;
        float s = (1.0f - bounds->scale) * field_0x274;
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
    field_0x260 = (depth >= CollisionPoint::field_0x9c) ? 1 : 0;
    field_0x224 = *velocity;
    if (CollisionPoint::field_0xc0) {
        TireSurfaceRef* ref = (TireSurfaceRef*)CollisionPoint::field_0xc0;
        CollisionPoint::field_0x8c = ref->tables->gripScale[*surface];
        field_0x284 = ref->tables->secondScale[*surface];
    } else {
        CollisionPoint::field_0x8c = 1.0f;
        field_0x284 = 1.0f;
    }
    field_0x264 = 0;
    field_0x268 = 0;
    CollisionPoint::field_0xa4 = 0.0f;
    CollisionPoint::field_0x90 = 0.0f;
    CollisionPoint::field_0x94 = 0.0f;
}

// ---------------------------------------------------------------------------------------
// Constructor 0x00512f10.  CollisionObject(1), MovingPart, then CollisionPoint whose constructor
// is inlined here (it is only declared in CollisionPoint.h).  Same body as the one in
// CollisionPoint.cpp but with this translation unit's zero/unit vector globals (tier 2:
// the stores read 0x0068a3d0 / 0x0068a3f0, not the collision globals).
inline CollisionPoint::CollisionPoint(float a, int b)
    : field_0x88(a), field_0xc0(b) {
    field_0x08 = g_TireZeroVec3;
    field_0x04 = 0;
    field_0x14 = g_TireZeroVec3;
    field_0x20 = g_TireZeroVec3;
    field_0x2c = g_TireVec3_68a3f0;
    field_0x38 = g_TireZeroVec3;
    field_0x44 = g_TireZeroVec3;
    field_0x50 = g_TireZeroVec3;
    field_0x5c = g_TireZeroVec3;
    field_0x90 = 0;
    field_0x94 = 0;
    field_0x98 = -999.0f;
    field_0x9c = 0;
    field_0xa4 = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0xb4 = 0;
    field_0xa0 = 0.5f;
    field_0xb8 = 1.0f;
    field_0xbc = 0;
    field_0x8c = 1.0f;
    field_0x84 = 0;
    field_0x78 = g_TireZeroVec3;
    field_0x74 = 0;
    field_0x68 = g_TireZeroVec3;
}

Tire::Tire(void* a1, int a2, float a3, float a4, int a5, int a6, int a7, float a8,
           float a9, void* a10, float a11, float a12, float a13, float a14, int a15)
    : CollisionObject(1), MovingPart(a1, a2, a6), CollisionPoint(a4, a15)
{
    CollisionVec3 extentA;
    CollisionVec3 extentB;
    ((TireNode*)MovingPart::field_0x40)->Fn_004fe0a0(&extentA, &extentB);
    field_0x274 = extentB.y;
    field_0x1c8 = 1.0f / extentB.y;
    field_0x278 = 0;
    ((TireNode*)MovingPart::field_0x40)->Fn_004fc9a0(0, &field_0x200);
    field_0x1e8 = g_TireZeroVec3;
    field_0x260 = 0;
    field_0x224 = g_TireZeroVec3;
    field_0x230 = g_TireZeroVec3;
    field_0x23c = g_TireZeroVec3;
    field_0x248 = g_TireZeroVec3;
    field_0x280 = 0;
    field_0x254 = g_TireZeroVec3;
    field_0x28c = 0;
    field_0x270 = a3;
    field_0x1cc = a4;
    field_0x294 = a14;
    CollisionPoint::field_0xa0 = a14;
    field_0x298 = a5;
    field_0x2b4 = a7;
    field_0x1d0 = a8;
    field_0x1d4 = a9;
    field_0x2a0 = a10;
    field_0x1d8 = a11;
    field_0x288 = 1.0f;
    field_0x2a8 = 0;
    field_0x2ac = 0;
    field_0x2b0 = 0;
    field_0x2b8 = 1.0f;
    field_0x2bc = 0;
    field_0x264 = 0;
    field_0x268 = 0;
    field_0x29c = 0;
    field_0x1dc = a12;
    field_0x1e0 = a13;
    field_0x290 = 0;
    field_0x20c = g_TireVec3_68a3c0;
    field_0x284 = 1.0f;
    field_0x26c = 0;
    field_0x1f4 = g_TireZeroVec3;
    field_0x1e4 = 0;
    field_0x2a4 = 1.0f;
    field_0x218 = g_TireZeroVec3;
}
