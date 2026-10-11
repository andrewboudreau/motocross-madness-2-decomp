// Near-miss wrecker.cpp candidates, kept out of src/reconstructed until they
// match. See docs/WRECKER.md. Check one with
//   tools/compile.py --compiler vc6 samples/race/WreckerNearMisses.cpp -o X.obj
// and compare the symbol against retail with
// samples/race/WreckerNearMisses.bindings.json.
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
// differs (241/1312; retail keeps `push` nearest esp). A probe-count
// accessor does not change the budget.
//
// UnknownFunction531740 (0x00531740, 1624 bytes; 772/1617): the contact
// particle spray. Frame 0xc4 and every call, operator and constant in
// retail's order; three scratch vectors are reused across the body as
// retail's slot sharing shows. Left: the slot order and `0.0f - scaled.z`
// of the cross product with the Y axis (retail loads scaled.z first).

#include <math.h>
#include <stdlib.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Wrecker.h"

// The unit's random factor macro (as in Wrecker.cpp).
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

// The parenthesised first product puts each fsubp between the load and the store of the
// previous component's copy, as retail does (docs/VC6_OPERAND_ORDER.md section 3); both
// x products parenthesised gain 4 bytes in 0x00531740 (best of the 64 forms; slot 10
// scores the same with every form). 0x00532580 is exact in src/reconstructed/Wrecker.cpp.
inline Vector3 WreckerCross(const Vector3& a, const Vector3& b) {
    Vector3 r;
    r.x = (a.y * b.z) - (a.z * b.y);
    r.y = (a.z * b.x) - a.x * b.z;
    r.z = (a.x * b.y) - a.y * b.x;
    return r;
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
