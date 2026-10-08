// Near-miss wrecker.cpp candidates, kept out of src/reconstructed until they
// match. See docs/WRECKER.md. Check one with
//   tools/compile.py --compiler vc6 samples/race/WreckerNearMisses.cpp -o X.obj
// and compare the symbol against retail with
// samples/race/WreckerNearMisses.bindings.json.
//
// UnknownFunction531da0 (0x00531da0, 634 bytes): emits a trail of particles
// along a probe's movement. 629 of 634 compared bytes match. The difference
// is the x component of `step`: retail computes `scale * delta.x` with the
// reciprocal still on the stack (`fld st(0); fmul [delta.x]`). VC6 here
// loads delta.x first (`fld [delta.x]; fmul st(1)`). Member operator*,
// free operator* with either order, a reciprocal folded into the operand,
// an explicit per-component constructor, `*=` on a copy and separate
// component statements all leave that instruction pair. Retail also needs
// the length to be written `(y*y + x*x) + z*z` (or `z*z + (x*x + y*y)`);
// `x*x + y*y + z*z` reuses delta.z from the FPU stack.
//
// UnknownFunction532580 (0x00532580, 562 bytes): rebuilds the wreck pose's
// linear (field_0xbc) and angular (field_0xc8) step from the node's frame.
// 490/562: the temporaries (a member-by-member copy helper), the FPU-held
// up row (float locals) and the branch layout match. Left: each cross
// product's loads are swapped (retail `fld pose.z; fmul old.y`, VC6 here
// `fld old.y; fmul pose.z` whatever the operand order, argument order,
// sign convention, by-value or matrix form) and its `fsubp` is scheduled
// after the destination pointer copy.
//
// UnknownFunction5329e0 (0x005329e0, 1312 bytes): pushes probes back out of
// the box field_0xe8/field_0xf4 and turns the mean push into the impulse
// field_0x10c. Every block matches in shape, including the per-axis clip
// (an inline helper with by-value parameters, which explains the local
// copies of the extent and the previous position) and the retail use of the
// previous position's y for the x axis. Retail runs out of inline budget at
// the end (docs/VC6_INLINE_BUDGET.md): it expands `sum * field_0x118` but
// calls the Vector3 constructor inside it (0x00404e60), then calls the
// out-of-line `operator*` (0x005015b0) for `* frameTime`. Written naturally
// (`field_0x10c = sum * field_0x118 * frameTime;`) VC6 expands every site
// and gets retail's 0x94-byte frame; one more trivial inline expansion
// anywhere in the body makes it call the constructor as retail does, so
// the original had about one small inline helper more (an accessor, for
// example). The version below passes the scaled temporary straight to the
// out-of-line scale, which gives retail's 0x94 frame; only the slot order
// differs (215/1306; retail keeps `push` nearest esp). A probe-count
// accessor does not change the budget.
//
// UnknownFunction531740 (0x00531740, 1624 bytes; 768/1617): the contact
// particle spray. Frame 0xc4 and every call, operator and constant in
// retail's order; three scratch vectors are reused across the body as
// retail's slot sharing shows. Left: the slot order and `0.0f - scaled.z`
// of the cross product with the Y axis (retail loads scaled.z first).
//
// UnknownVirtualSlot10 (0x005306e0, 4176 bytes; 1817/4183): the first
// 0x62c bytes match retail. The rider carry-over (rigid inverse of the
// wreck pose, two 4x4 products, axis rebuild) keeps retail's shape and
// calls but not its term/operand order; the second product expands only
// with __forceinline (docs/VC6_INLINE_BUDGET.md).

#include <math.h>
#include <stdlib.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Wrecker.h"

// The unit's zero vector (Wrecker.cpp's per-TU constant at 0x0068ad00) and
// its random factor macro.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
#define RANDOM_UNIT() (rand() * (1.0f / 32768))

// Vector helpers in the d3dvec.inl style. MatrixUtil.h's Vector3 has only
// the scale and += operators.
inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline float WreckerSquareMagnitude(const Vector3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline Vector3 WreckerCross(const Vector3& a, const Vector3& b) {
    Vector3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

void Wrecker::UnknownFunction531da0(Vector3 from, Vector3 to, float frameTime) {
    Vector3 delta = to - from;
    float lengthSq = (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z;
    float length = lengthSq == 1.0f ? 1.0f : UnknownFunction460b50(lengthSq);
    float speed = length / frameTime;
    if (speed < 0.5f)
        return;
    speed *= 0.013333f;
    if (field_0x44) {
        if (speed < 1.5f)
            speed = 1.5f;
        else if (speed > 2.0f)
            speed = 2.0f;
    } else {
        if (speed < 0.25f)
            speed = 0.25f;
        else if (speed > 2.0f)
            speed = 2.0f;
    }
    float lifetime = speed * 2000.0f;
    if (lifetime < 500.0f)
        lifetime = 500.0f;
    else if (lifetime > 1000.0f)
        lifetime = 1000.0f;
    int count = (int)(length / (speed * 0.6f));
    if (!count)
        count = 1;
    float age = lifetime * 0.25f;
    float scale = 1.0f / count;
    Vector3 step = delta * scale;
    for (int i = 0; i < count; i++) {
        Vector3 position = from + step * (float)i;
        field_0x11c->UnknownFunction4baa50(age, &position, field_0x120, speed, lifetime, speed + speed, 10, 0xffffff);
        if (++field_0x120 >= 13)
            field_0x120 = 0;
    }
}

// Copies a vector member by member into a returned local: retail copies
// the old position (and forward row) through a stack temporary this way.
static inline Vector3 WreckerCopy(const Vector3& v) {
    Vector3 r;
    r.x = v.x;
    r.y = v.y;
    r.z = v.z;
    return r;
}

void Wrecker::UnknownFunction532580() {
    UnknownWreckerFrame pose;
    field_0xbc = WreckerCopy(field_0x6c.position);
    field_0xb8->UnknownFunction4fca80(0, &pose);
    field_0xbc = pose.position - field_0xbc;
    Vector3 forward = WreckerCopy(field_0x6c.forward);
    // The old up row stays on the FPU stack: float locals.
    float upX = field_0x6c.up.x;
    float upY = field_0x6c.up.y;
    float upZ = field_0x6c.up.z;
    if (upX == pose.up.x && upY == pose.up.y && upZ == pose.up.z) {
        if (forward.x == pose.forward.x && forward.y == pose.forward.y && forward.z == pose.forward.z)
            field_0xc8 = Vector3(0.0f, 0.0f, 0.0f);
        else
            field_0xc8 = WreckerCross(forward, pose.forward);
    } else {
        field_0xc8 = WreckerCross(Vector3(upX, upY, upZ), pose.up);
    }
    field_0xc8 = UnknownFunction5015b0(field_0xc8, 1.0f);
}

// Per-axis clip of a probe against the box (inlined three times).
static inline void WreckerClipAxis(float p, float extent, float previous, float& out, float& outPrevious) {
    if (p >= 0.0f) {
        if (p < extent) {
            if (p < previous)
                outPrevious += previous - p;
            out += extent - p;
        }
    } else {
        if (-extent > p) {
            if (p > previous)
                outPrevious += previous - p;
            out += extent - p;
        }
    }
}


// ---------------------------------------------------------------------------
// Out-of-line call views (docs/VC6_INLINE_BUDGET.md): the sites where retail
// calls the COMDAT copy of a header inline instead of expanding it. Declared
// only; the bindings map them to the retail copies.
// ---------------------------------------------------------------------------
struct WreckerVec3Call : Vector3 {
    WreckerVec3Call(float x_, float y_, float z_);                 // 0x00404e60
};
float WreckerDotCall(const Vector3* a, const Vector3* b);          // 0x0040ae30
Vector3* WreckerAddCall(Vector3* out, const Vector3* a, const Vector3* b);      // 0x00421cb0
Vector3* WreckerSubtractCall(Vector3* out, const Vector3* a, const Vector3* b); // 0x00421d00

// |v|, exact for unit vectors (the same helper CarProcedural.cpp expands).
static inline float WreckerLength(Vector3 v) {
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return 1.0f;
    return UnknownFunction460b50(squared);
}

// The scale with the constructor called out of line.
static inline Vector3 WreckerScaleCtorCall(const Vector3& v, float s) {
    return WreckerVec3Call(v.x * s, v.y * s, v.z * s);
}

// v / |v| as 0x005087b0 computes it (unchanged for a unit vector), with the
// dot product called out of line. The first site of 0x00531740 expands the
// scale around the out-of-line constructor; the second calls the scale.
static inline Vector3 WreckerNormalizeFirst(const Vector3& v) {
    float squared = WreckerDotCall(&v, &v);
    if (squared == 1.0f)
        return v;
    return WreckerScaleCtorCall(v, UnknownFunction460c00(squared));
}
static inline Vector3 WreckerNormalizeTail(const Vector3& v) {
    float squared = WreckerDotCall(&v, &v);
    if (squared == 1.0f)
        return v;
    float inv = UnknownFunction460c00(squared);
    return UnknownFunction5015b0(v, inv);
}

static inline Vector3 WreckerAddCtorCall(const Vector3& a, const Vector3& b) {
    return WreckerVec3Call(a.x + b.x, a.y + b.y, a.z + b.z);
}

void Wrecker::UnknownFunction5329e0(float frameTime) {
    Vector3 previousSum = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 sum = Vector3(0.0f, 0.0f, 0.0f);
    int hits = 0;
    for (int i = 0; i < field_0x38->field_0x100; i++) {
        Vector3 p = field_0x38->field_0x104[i].field_0x0c->UnknownFunction4fd660(field_0x38->field_0x104[i].field_0x00);
        field_0x104[i] = p;
        p = field_0x100->UnknownFunction4fd7f0(p) - field_0xe8;
        if (fabs(p.x) < field_0xf4.x && fabs(p.y) < field_0xf4.y && fabs(p.z) < field_0xf4.z) {
            Vector3 push = Vector3(0.0f, 0.0f, 0.0f);
            Vector3 previousPush = Vector3(0.0f, 0.0f, 0.0f);
            WreckerClipAxis(p.x, field_0xf4.x, field_0x104[i].y, push.x, previousPush.x);
            WreckerClipAxis(p.y, field_0xf4.y, field_0x104[i].y, push.y, previousPush.y);
            WreckerClipAxis(p.z, field_0xf4.z, field_0x104[i].z, push.z, previousPush.z);
            if (push.x != 0.0f || push.y != field_0xf4.y || push.z != field_0xf4.z) {
                sum += push;
                hits++;
            }
            if (previousPush.x != 0.0f || previousPush.y != field_0xf4.y || previousPush.z != field_0xf4.z)
                previousSum += previousPush;
        }
    }
    if (hits)
        sum = sum * (1.0f / hits);
    field_0x10c = UnknownFunction5015b0(WreckerScaleCtorCall(sum, field_0x118), frameTime);
}

// The particle the next free entry of the manager's table.
#define WRECKER_NEXT_PARTICLE() (field_0x11c->field_0x40[field_0x11c->field_0x2c])
// The current entry of the random table, and the advance that wraps at 256.
#define WRECKER_RANDOM_TABLE() (field_0x128[field_0x528])
#define WRECKER_ADVANCE_RANDOM() if (++field_0x528 == 256) field_0x528 = 0

// 0x00531740 (1624 bytes): sprays contact particles along a probe's slide
// from `from` to `to`. Their velocity is the wreck's (or the rigid body's)
// velocity plus a sideways jitter, plus the remaining gravity drift of the
// frame; frameTime * 200 particles are emitted per frame.
void Wrecker::UnknownFunction531740(Vector3 from, Vector3 to, Vector3 normal, float frameTime) {
    Vector3 delta = to - from;
    Vector3 velocity;
    if (field_0x44 == 0)
        velocity = field_0xbc;
    else
        velocity = field_0x40->field_0x154;
    float speed = WreckerLength(velocity);
    if (speed < 1.0f)
        return;
    Vector3 dir = WreckerNormalizeFirst(velocity);
    Vector3 scaled = dir * (speed + 2.0f);
    dir = WreckerCross(scaled, Vector3(0.0f, 1.0f, 0.0f));
    Vector3 side = WreckerNormalizeTail(dir);
    Vector3 spread = side * 0.5f * speed;
    float count = frameTime * 200.0f;
    float inv = 1.0f / count;
    side = delta * inv;                                   // the step per particle
    delta = WreckerScaleCtorCall(field_0x11c->field_0x1f98, frameTime);
    dir = WreckerScaleCtorCall(delta, inv);               // the gravity drift per particle
    for (int i = 0; i < count; i++) {
        if (field_0x11c->field_0x2c != 1000) {
            WRECKER_NEXT_PARTICLE()->field_0x14 = 0.0f;
            delta = WreckerScaleCtorCall(side, (float)i);
            WRECKER_NEXT_PARTICLE()->field_0x00 = WreckerAddCtorCall(delta, from);
            WRECKER_NEXT_PARTICLE()->field_0x10 = field_0x124;
            WRECKER_NEXT_PARTICLE()->field_0x1c = WRECKER_RANDOM_TABLE() * 0.15f;
            WRECKER_NEXT_PARTICLE()->field_0x20 = WRECKER_RANDOM_TABLE() * 0.3f;
            WRECKER_ADVANCE_RANDOM();
            WRECKER_NEXT_PARTICLE()->field_0x18 = 375.0f;
            float jitter = WRECKER_RANDOM_TABLE() * 2.0f - 1.0f;
            WRECKER_ADVANCE_RANDOM();
            Vector3 drift = WreckerScaleCtorCall(dir, count - (float)i);
            Vector3 sideways = WreckerScaleCtorCall(spread, jitter);
            Vector3 sum = WreckerAddCtorCall(sideways, scaled);
            Vector3 result;
            WRECKER_NEXT_PARTICLE()->field_0x38 = *WreckerAddCall(&result, &sum, &drift);
            WRECKER_NEXT_PARTICLE()->field_0x34 = 0x14;
            field_0x11c->field_0x2c++;
        }
        if (++field_0x124 >= 0x2d)
            field_0x124 = 0x1d;
    }
}

// ---------------------------------------------------------------------------
// Slot 10, 0x005306e0 (4176 bytes): the per-frame update.
// ---------------------------------------------------------------------------

Vector3 WreckerNormalizeCall(const Vector3& v);                           // 0x005087b0
Vector3 WreckerCrossCall(const Vector3& a, const Vector3& b);             // 0x00515600

static inline Vector3& operator*=(Vector3& v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

// Per-step update of a GameObject (slot 11) through the float view.
static inline void WreckerStep(GameObject* object, float step) {
    ((UnknownWreckerSteppable*)object)->UnknownVirtualSlot11(step);
}

// In-place inverse of a rigid transform (the InvertRigid helper of
// src/krusty2/bvh/BoundingBoxTreeQuery.h on a Matrix4).
static inline void WreckerInvertRigid(UnknownWreckerMatrix* m) {
    float swap;
    swap = m->_12; m->_12 = m->_21; m->_21 = swap;
    swap = m->_13; m->_13 = m->_31; m->_31 = swap;
    swap = m->_23; m->_23 = m->_32; m->_32 = swap;
    Vector3 t;
    t.x = -(m->_41 * m->_11 + m->_42 * m->_21 + m->_43 * m->_31);
    t.y = -(m->_41 * m->_12 + m->_42 * m->_22 + m->_43 * m->_32);
    t.z = -(m->_41 * m->_13 + m->_42 * m->_23 + m->_43 * m->_33);
    m->_41 = t.x;
    m->_42 = t.y;
    m->_43 = t.z;
}

// The 4x4 product expanded inline (the MatrixProduct text of
// src/krusty2/soultree/soultree.cpp): *out = b * a in the row-vector
// convention.
static __forceinline void WreckerMatrixProduct(UnknownWreckerMatrix* out, const UnknownWreckerMatrix& a, const UnknownWreckerMatrix& b) {
    out->_11 = a._11 * b._11 + a._21 * b._12 + a._31 * b._13 + a._41 * b._14;
    out->_12 = a._12 * b._11 + a._22 * b._12 + a._32 * b._13 + a._42 * b._14;
    out->_13 = a._13 * b._11 + a._23 * b._12 + a._33 * b._13 + a._43 * b._14;
    out->_14 = a._14 * b._11 + a._24 * b._12 + a._34 * b._13 + a._44 * b._14;
    out->_21 = a._11 * b._21 + a._21 * b._22 + a._31 * b._23 + a._41 * b._24;
    out->_22 = a._12 * b._21 + a._22 * b._22 + a._32 * b._23 + a._42 * b._24;
    out->_23 = a._13 * b._21 + a._23 * b._22 + a._33 * b._23 + a._43 * b._24;
    out->_24 = a._14 * b._21 + a._24 * b._22 + a._34 * b._23 + a._44 * b._24;
    out->_31 = a._11 * b._31 + a._21 * b._32 + a._31 * b._33 + a._41 * b._34;
    out->_32 = a._12 * b._31 + a._22 * b._32 + a._32 * b._33 + a._42 * b._34;
    out->_33 = a._13 * b._31 + a._23 * b._32 + a._33 * b._33 + a._43 * b._34;
    out->_34 = a._14 * b._31 + a._24 * b._32 + a._34 * b._33 + a._44 * b._34;
    out->_41 = a._11 * b._41 + a._21 * b._42 + a._31 * b._43 + a._41 * b._44;
    out->_42 = a._12 * b._41 + a._22 * b._42 + a._32 * b._43 + a._42 * b._44;
    out->_43 = a._13 * b._41 + a._23 * b._42 + a._33 * b._43 + a._43 * b._44;
    out->_44 = a._14 * b._41 + a._24 * b._42 + a._34 * b._43 + a._44 * b._44;
}

int Wrecker::UnknownVirtualSlot10(float frameTime) {
    GameObject::UnknownVirtualSlot10(frameTime);
    if (field_0x44 == 0) {
        field_0x38->UnknownFunctionStep(frameTime);
        if (field_0x52c)
            field_0x52c = 0;
        else
            UnknownFunction532020(frameTime);
        field_0xd8 = 0;
        UnknownFunction5329e0(frameTime);
        field_0xdc = kVec3Zero;
        float touching = 0.0f;
        for (int i = 0; i < field_0x38->field_0x100; i++) {
            if (field_0x38->field_0x104[i].field_0x10) {
                field_0x108[i] = 1;
                field_0xd8 = 1;
                field_0xdc += field_0x104[i];
                touching += 1.0f;
            } else {
                field_0x108[i] = 0;
            }
        }
        if (field_0xd8)
            field_0xdc *= 1.0f / touching;
        UnknownFunction532580();
        field_0xbc = field_0xbc * (1.0f / frameTime);
        field_0xc8 = field_0xc8 * (1.0f / frameTime);
        field_0xb8->UnknownFunction4fca80(0, &field_0x6c);
    } else {
        if (field_0xd4 && !field_0x34->field_0x34) {
            field_0x34->UnknownFunction4a8b80(field_0xd4, 0.5f);
            field_0xd4 = 0;
            if (field_0x538 == 0 && field_0x530 == 0) {
                field_0x530 = 1;
            } else {
                field_0xb4 = 1;
                field_0x538 = 0;
                field_0x530 = 0;
            }
        }
        float remaining = frameTime;
        field_0xb0 = 0;
        float step = 0.02f;
        if (frameTime > 0.06f)
            step = frameTime * (1.0f / 3);
        while (remaining > 0.0f) {
            if (remaining < step)
                step = remaining;
            if (field_0x44 != 1) {
                WreckerStep(field_0x3c, step);
                WreckerStep(field_0x40, step);
            }
            if (field_0x52c)
                field_0x52c = 0;
            else
                UnknownFunction532020(frameTime);
            for (int i = 0; i < field_0x38->field_0x100; i++)
                field_0x108[i] = field_0x38->field_0x104[i].field_0x10;
            UnknownFunction5329e0(step);
            if (field_0x44 != 1) {
                Vector3 position;
                field_0x40->field_0x234->UnknownFunction4fc9a0(0, &position);
                position += field_0x10c;
                field_0x40->field_0x234->UnknownFunction4fc740(0, &position);
            }
            field_0x38->UnknownFunctionStep(step);
            remaining -= step;
        }
        if (field_0x38->field_0x58) {
            field_0xb0 = 1;
            field_0xac = 1;
            if (field_0x44 == 2) {
                field_0x44 = 3;
                field_0xd4 = field_0x58[(int)(RANDOM_UNIT() * field_0x5c)];
            }
        }
        if (field_0x44) {
            if (field_0x44 == 1) {
                UnknownWreckerRaceState* state = field_0x34->field_0x18;
                if ((float)(state->field_0xa8 - *field_0x34->field_0x1c) / state->field_0xb4 < 0.5f) {
                    UnknownFunction532490();
                    UnknownFunction532580();
                    field_0xbc = field_0xbc * (1.0f / frameTime);
                    field_0xc8 = field_0xc8 * (1.0f / frameTime);
                    field_0x40->UnknownVirtualSlot44();
                    field_0x40->field_0x154 = field_0xbc;
                    field_0x40->field_0x160 = field_0xbc;
                    field_0x40->field_0x16c = field_0x40->field_0x234->UnknownFunction4fd710(field_0xc8);
                }
            }
            if (field_0x530 && !field_0x34->field_0x34) {
                field_0xb4 = 1;
                field_0x530 = 0;
            }
            if (field_0xb4 && field_0x534) {
                UnknownFunction532580();
                field_0xbc = field_0xbc * (1.0f / frameTime);
                field_0xc8 = field_0xc8 * (1.0f / frameTime);
                field_0x534 = 0;
            }
            field_0xb8->UnknownFunction4fca80(0, &field_0x6c);
            field_0x34->UnknownVirtualSlot7(frameTime, 0, 0);
        }
    }
    if (field_0xb4) {
        // Carry the rider's root over by the motion of the wreck node since
        // the pose field_0x6c was taken, then re-orthonormalise its axes.
        UnknownWreckerMatrix pose;
        UnknownWreckerMatrix scratch;
        UnknownWreckerMatrix relative;
        field_0xb8->UnknownFunction4fca80(0, &pose);
        Vector3 oldPosition = field_0x6c.position;
        WreckerInvertRigid(&pose);
        scratch = *(UnknownWreckerMatrix*)&field_0x6c;
        WreckerMatrixProduct(&relative, scratch, pose);
        field_0x34->field_0x1a0->UnknownFunction4fca80(0, &pose);
        scratch = pose;
        WreckerMatrixProduct(&pose, relative, scratch);
        Vector3 up = WreckerVec3Call(pose._21, pose._22, pose._23);
        Vector3 forward = WreckerVec3Call(pose._31, pose._32, pose._33);
        forward = WreckerNormalizeCall(forward);
        Vector3 right = WreckerNormalizeCall(WreckerCrossCall(up, forward));
        up = WreckerCrossCall(right, forward);
        pose._11 = right.x;
        pose._12 = right.y;
        pose._13 = right.z;
        pose._14 = 0.0f;
        pose._21 = up.x;
        pose._22 = up.y;
        pose._23 = up.z;
        pose._24 = 0.0f;
        pose._31 = forward.x;
        pose._32 = forward.y;
        pose._33 = forward.z;
        pose._34 = 0.0f;
        field_0x34->field_0x1a0->UnknownFunction4fb8c0(0, &pose);
        Vector3 position;
        field_0xb8->UnknownFunction4fc9a0(0, &position);
        Vector3 result;
        Vector3 delta = *WreckerSubtractCall(&result, &oldPosition, &position);
        field_0x34->field_0x1a0->UnknownFunction4fc890(0, &delta);
        field_0xb4 = 0;
    }
    UnknownFunction532150();
    return 1;
}
