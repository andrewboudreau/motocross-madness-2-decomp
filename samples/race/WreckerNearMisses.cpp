// Near-miss wrecker.cpp candidates, kept out of src/reconstructed until they
// match. See docs/WRECKER.md. Check one with
//   tools/compile.py --compiler vc6 samples/race/WreckerNearMisses.cpp -o X.obj
// and compare the symbol against retail with
// samples/race/WreckerNearMisses.bindings.json.
//
// UnknownFunction5329e0 (0x005329e0, 1312 bytes; 763/1312): pushes probes
// back out of the box boxCenter/boxHalfExtents and turns the mean push into the
// impulse pushImpulse. Every block matches in shape, including the per-axis
// clip (an inline helper with by-value parameters, which explains the local
// copies of the extent and the previous position) and the retail use of the
// previous position's y for the x axis. Retail runs out of inline budget at
// the end (docs/VC6_INLINE_BUDGET.md): it expands `sum * pushScale` but
// calls the Vector3 constructor inside it (0x00404e60), then calls the
// out-of-line `operator*` (0x005015b0) for `* frameTime`; the scaled
// temporary is passed straight through, which gives retail's 0x94 frame.
// The clip sums written `out = out + (extent - p)` and the mean through the
// inline `operator/` give retail's instructions except one block placement:
// retail does not merge the two `outPrevious` updates of the z clip (the
// x and y clips are merged in both). Left: that block and the slot order
// (retail, nearest esp first: push, the y and x extent copies, p, the z
// extent copy, previousPush, sum, a zero temporary, hits, the three previous
// copies, previousSum; here p, previousPush, sum, push, a zero temporary,
// then the six copies and hits as one group). Tried without a change:
// every order of the clip's five parameters (120), `else if`, an early
// return, the clip returning `out` by value, pointer outputs, a macro with
// block locals, the clip written out with named locals, push/previousPush
// declared at function or loop scope, `sum = sum + push`.
//
// SprayContactParticles (0x00531740, 1624 bytes; 1606/1624): the contact
// particle spray. Frame, slot order and every instruction but two match
// with the normalisations written as nested expressions (their temporaries
// share slots as retail's do) and the per-particle gravity drift built
// from the nested out-of-line scale. Left: the argument push of two
// out-of-line constructor calls in the loop comes one load earlier here
// (`push ecx` before the particle-pointer load, and before the copy of
// `along`'s y), which also changes two registers. Tried without a change:
// a float copy of the index, a particle pointer local, `along` declared
// at loop or function scope or assigned, reusing `delta`/`step` for
// `along`, by-value or derived-type returning helpers, `from + along`;
// prepending 0..63 typedefs (tools/decl_shift_scan.py) never changes it.

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
// x products parenthesised gain 5 bytes in 0x00531740 (best of the 64 forms; slot 10
// scores the same with every form). 0x00532580 is exact in src/reconstructed/Wrecker.cpp.
inline Vector3 WreckerCross(const Vector3& a, const Vector3& b) {
    Vector3 r;
    r.x = (a.y * b.z) - (a.z * b.y);
    r.y = (a.z * b.x) - a.x * b.z;
    r.z = (a.x * b.y) - a.y * b.x;
    return r;
}

// Per-axis clip of a probe against the box (inlined three times). The sums are
// written `out = out + (extent - p)`: with `+=` VC6 computes the probe-array
// address base first (`mov ecx, [esi+0x104]; add ecx, edi`) where retail adds
// the base to the index register, and scores 240 instead of 763.
static inline void WreckerClipAxis(float p, float extent, float previous, float& out, float& outPrevious) {
    if (p >= 0.0f) {
        if (p < extent) {
            if (p < previous)
                outPrevious = outPrevious + (previous - p);
            out = out + (extent - p);
        }
    } else {
        if (-extent > p) {
            if (p > previous)
                outPrevious = outPrevious + (previous - p);
            out = out + (extent - p);
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
    return FastSqrt(squared);
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
    return WreckerScaleCtorCall(v, FastInvSqrt(squared));
}
static inline Vector3 WreckerNormalizeTail(const Vector3& v) {
    float squared = WreckerDotCall(&v, &v);
    if (squared == 1.0f)
        return v;
    float inv = FastInvSqrt(squared);
    return Vec3ScaleCall(v, inv);
}

static inline Vector3 WreckerAddCtorCall(const Vector3& a, const Vector3& b) {
    return WreckerVec3Call(a.x + b.x, a.y + b.y, a.z + b.z);
}

// The mean push divides through the inline `operator/` that Wrecker.cpp uses in
// 0x00531da0 (`v * (1.0f / s)`): retail keeps the reciprocal on the stack for
// the first component (`fld st(0); fmul [sum.x]`), and the later scale by
// pushScale then loads sum.x as a new leaf, as retail does.
inline Vector3 operator/(const Vector3& v, float s) {
    return v * (1.0f / s);
}

void Wrecker::UnknownFunction5329e0(float frameTime) {
    Vector3 previousSum = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 sum = Vector3(0.0f, 0.0f, 0.0f);
    int hits = 0;
    for (int i = 0; i < collisionModel->probeCount; i++) {
        Vector3 p = collisionModel->probes[i].node->LocalToWorldPoint(collisionModel->probes[i].localPoint);
        probePositions[i] = p;
        p = boxFrame->UnknownFunction4fd7f0(p) - boxCenter;
        if (fabs(p.x) < boxHalfExtents.x && fabs(p.y) < boxHalfExtents.y && fabs(p.z) < boxHalfExtents.z) {
            Vector3 push = Vector3(0.0f, 0.0f, 0.0f);
            Vector3 previousPush = Vector3(0.0f, 0.0f, 0.0f);
            WreckerClipAxis(p.x, boxHalfExtents.x, probePositions[i].y, push.x, previousPush.x);
            WreckerClipAxis(p.y, boxHalfExtents.y, probePositions[i].y, push.y, previousPush.y);
            WreckerClipAxis(p.z, boxHalfExtents.z, probePositions[i].z, push.z, previousPush.z);
            if (push.x != 0.0f || push.y != boxHalfExtents.y || push.z != boxHalfExtents.z) {
                sum += push;
                hits++;
            }
            if (previousPush.x != 0.0f || previousPush.y != boxHalfExtents.y || previousPush.z != boxHalfExtents.z)
                previousSum += previousPush;
        }
    }
    if (hits)
        sum = sum / hits;
    pushImpulse = Vec3ScaleCall(WreckerScaleCtorCall(sum, pushScale), frameTime);
}

// The particle the next free entry of the manager's table.
#define WRECKER_NEXT_PARTICLE() (particles->particles[particles->activeCount])
// The current entry of the random table, and the advance that wraps at 256.
#define WRECKER_RANDOM_TABLE() (randomTable[randomIndex])
#define WRECKER_ADVANCE_RANDOM() if (++randomIndex == 256) randomIndex = 0

// 0x00531740 (1624 bytes): sprays contact particles along a probe's slide
// from `from` to `to`. Their velocity is the wreck's (or the rigid body's)
// velocity plus a sideways jitter, plus the remaining gravity drift of the
// frame; frameTime * 200 particles are emitted per frame.
void Wrecker::SprayContactParticles(Vector3 from, Vector3 to, Vector3 normal, float frameTime) {
    Vector3 delta = to - from;
    Vector3 velocity;
    if (wreckState == 0)
        velocity = linearStep;
    else
        velocity = rigidBody->velocity;
    float speed = WreckerLength(velocity);
    if (speed < 1.0f)
        return;
    Vector3 scaled = WreckerNormalizeFirst(velocity) * (speed + 2.0f);
    Vector3 side = WreckerNormalizeTail(WreckerCross(scaled, Vector3(0.0f, 1.0f, 0.0f)));
    Vector3 spread = side * 0.5f * speed;
    float count = frameTime * 200.0f;
    float inv = 1.0f / count;
    Vector3 step = delta * inv;                           // the step per particle
    // The gravity drift per particle; nested, the inner temporary shares the
    // particle drift's slot as retail's does.
    Vector3 dirStep = WreckerScaleCtorCall(WreckerScaleCtorCall(particles->gravity, frameTime), inv);
    for (int i = 0; i < count; i++) {
        if (particles->activeCount != 1000) {
            WRECKER_NEXT_PARTICLE()->age = 0.0f;
            Vector3 along = WreckerScaleCtorCall(step, (float)i);
            WRECKER_NEXT_PARTICLE()->position = WreckerAddCtorCall(along, from);
            WRECKER_NEXT_PARTICLE()->frame = sprayFrame;
            WRECKER_NEXT_PARTICLE()->size = WRECKER_RANDOM_TABLE() * 0.15f;
            WRECKER_NEXT_PARTICLE()->growth = WRECKER_RANDOM_TABLE() * 0.3f;
            WRECKER_ADVANCE_RANDOM();
            WRECKER_NEXT_PARTICLE()->lifetime = 375.0f;
            float jitter = WRECKER_RANDOM_TABLE() * 2.0f - 1.0f;
            WRECKER_ADVANCE_RANDOM();
            Vector3 drift = WreckerScaleCtorCall(dirStep, count - (float)i);
            Vector3 sideways = WreckerScaleCtorCall(spread, jitter);
            Vector3 sum = WreckerAddCtorCall(sideways, scaled);
            Vector3 result;
            WRECKER_NEXT_PARTICLE()->velocity = *WreckerAddCall(&result, &sum, &drift);
            WRECKER_NEXT_PARTICLE()->flags = 0x14;
            particles->activeCount++;
        }
        if (++sprayFrame >= 0x2d)
            sprayFrame = 0x1d;
    }
}
