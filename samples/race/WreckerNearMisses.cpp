// Near-miss wrecker.cpp candidates, kept out of src/reconstructed until they
// match. See docs/WRECKER.md. Check one with
//   tools/compile.py --compiler vc6 samples/race/WreckerNearMisses.cpp -o X.obj
// and compare the symbol against retail with
// samples/race/WreckerNearMisses.bindings.json.
//
// PushProbesOutOfBox (0x005329e0, 1312 bytes; 1200/1312 inside
// Wrecker.cpp): pushes probes back out of the box boxCenter/boxHalfExtents
// and turns the mean push into the impulse pushImpulse. Every block matches
// in shape, including the per-axis clip (an inline helper with by-value
// parameters, which explains the local copies of the extent and the
// previous position) and the retail use of the previous position's y for
// the x axis. Retail runs out of inline budget at the end
// (docs/VC6_INLINE_BUDGET.md): it expands `sum * pushScale` but calls the
// Vector3 constructor inside it (0x00404e60), then calls the out-of-line
// `operator*` (0x005015b0) for `* frameTime`; the scaled temporary is
// passed straight through, which gives retail's 0x94 frame. The clip sums
// written `out = out + (extent - p)` and the mean through the inline
// `operator/` give retail's instructions. Compiled inside Wrecker.cpp every
// instruction is equal (compiled on its own with copies of the unit's
// helpers, VC6 merged the z clip's two `outPrevious` updates and scored
// 763). Left: the slot order (retail, nearest esp first: push, the y and x
// extent copies, p, the z extent copy, previousPush, sum, a zero temporary,
// hits, the three previous copies, previousSum; here p, previousPush, sum,
// push, a zero temporary, then the six copies and hits as one group).
// Tried without a change: every order of the clip's five parameters (120),
// `else if`, an early return, the clip returning `out` by value, pointer
// outputs, a macro with block locals, the clip written out with named
// locals, push/previousPush declared at function or loop scope (also inside
// Wrecker.cpp), `sum = sum + push`, an empty block before p; previousPush
// declared before push keeps the slots (1203). Worse inside Wrecker.cpp:
// direct-initialised push (370), push and previousPush (113), sum (124)
// or previousSum (117), a component-wise sum (216), `sum = sum + push`
// (259) and `hits++` before the sum (929); a direct-initialised p is
// unchanged. The extent and previous copies have four accesses each in both
// builds (the z extent five in retail), so the per-byte use count does not
// explain retail ranking the extent copies among the vectors.
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

// Compiled inside Wrecker.cpp, whose vector operators, WreckerCross and the
// mean push's inline `operator/` (`v * (1.0f / s)`) these candidates use.
#include <math.h>

#include "../../src/reconstructed/Wrecker.cpp"

// This call-only view is still needed by the unfinished helpers below.
// The canonical Wrecker playback body uses the shared Vec3 constructor.
struct WreckerVec3Call : Vector3 {
    WreckerVec3Call(float x_, float y_, float z_);
};

inline float WreckerSquareMagnitude(const Vector3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
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

void Wrecker::PushProbesOutOfBox(float frameTime) {
    Vector3 previousSum = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 sum = Vector3(0.0f, 0.0f, 0.0f);
    int hits = 0;
    for (int i = 0; i < collisionModel->probeCount; i++) {
        Vector3 p = reinterpret_cast<const Vector3&>(collisionModel->probes[i].node->LocalToWorldPoint(reinterpret_cast<const Vec3&>(collisionModel->probes[i].localPoint)));
        probePositions[i] = p;
        p = reinterpret_cast<const Vector3&>(boxFrame->WorldToLocalPoint(reinterpret_cast<const Vec3&>(p))) - boxCenter;
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
