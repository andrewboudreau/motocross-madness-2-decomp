// Group1_WreckerContacts.cpp -- bring-up bodies for two Wrecker methods that
// Wrecker.cpp calls but src/ does not define yet.  Bodies come from
// samples/race/WreckerNearMisses.cpp (near misses: PushProbesOutOfBox 1200/1312,
// SprayContactParticles 1606/1624 positions; every block decoded against retail):
//   0x005329e0 Wrecker::PushProbesOutOfBox
//   0x00531740 Wrecker::SprayContactParticles
// The sample models retail's out-of-line COMDAT helper calls (Vector3 constructor
// 0x00404e60, dot 0x0040ae30, add 0x00421cb0, scale 0x005015b0) with declared-only
// "call views" that no source defines.  Here they are the equivalent inline
// expressions: same arithmetic and operand order, so the behavior is unchanged.

#include <math.h>

#include "Wrecker.h"
#include "../krusty2/core/SoultreeObject.h"
#include "../krusty2/constraint/ConstraintMethodCollisionModel.h"

// Wrecker.cpp's TU-local vector helpers.
static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}
static inline Vector3 operator/(const Vector3& v, float s) {
    return v * (1.0f / s);
}
static inline Vector3 Group1WreckerCross(const Vector3& a, const Vector3& b) {
    Vector3 r;
    r.x = (a.y * b.z) - (a.z * b.y);
    r.y = (a.z * b.x) - a.x * b.z;
    r.z = (a.x * b.y) - a.y * b.x;
    return r;
}

// Inline stand-ins for the sample's out-of-line call views.
static inline Vector3 Group1ScaleValue(const Vector3& v, float s) {     // 0x005015b0
    return Vector3(v.x * s, v.y * s, v.z * s);
}
static inline float Group1Dot(const Vector3* a, const Vector3* b) {     // 0x0040ae30
    return a->x * b->x + a->y * b->y + a->z * b->z;
}
static inline Vector3* Group1AddTo(Vector3* out, const Vector3* a, const Vector3* b) {  // 0x00421cb0
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    return out;
}

// Per-axis clip of a probe against the box (inlined three times in retail).
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

// |v|, exact for unit vectors.
static inline float WreckerLength(Vector3 v) {
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
}

// v / |v| as 0x005087b0 computes it (unchanged for a unit vector).
static inline Vector3 WreckerNormalize(const Vector3& v) {
    float squared = Group1Dot(&v, &v);
    if (squared == 1.0f)
        return v;
    return Group1ScaleValue(v, FastInvSqrt(squared));
}

// 0x005329e0: pushes probes back out of the box boxCenter/boxHalfExtents and turns
// the mean push into pushImpulse.  Retail uses the previous position's y for the x
// axis too (decoded at 0x00532be0).
void Wrecker::PushProbesOutOfBox(float frameTime) {
    Vector3 previousSum = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 sum = Vector3(0.0f, 0.0f, 0.0f);
    int hits = 0;
    for (int i = 0; i < collisionModel->probeCount; i++) {
        Vector3 point = reinterpret_cast<const Vector3&>(collisionModel->probes[i].node->LocalToWorldPoint(
            reinterpret_cast<const Vec3&>(collisionModel->probes[i].localPoint)));
        probePositions[i] = point;
        point = reinterpret_cast<const Vector3&>(
                    boxFrame->WorldToLocalPoint(reinterpret_cast<const Vec3&>(point))) - boxCenter;
        if (fabs(point.x) < boxHalfExtents.x && fabs(point.y) < boxHalfExtents.y
            && fabs(point.z) < boxHalfExtents.z) {
            Vector3 push = Vector3(0.0f, 0.0f, 0.0f);
            Vector3 previousPush = Vector3(0.0f, 0.0f, 0.0f);
            WreckerClipAxis(point.x, boxHalfExtents.x, probePositions[i].y, push.x, previousPush.x);
            WreckerClipAxis(point.y, boxHalfExtents.y, probePositions[i].y, push.y, previousPush.y);
            WreckerClipAxis(point.z, boxHalfExtents.z, probePositions[i].z, push.z, previousPush.z);
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
    pushImpulse = Group1ScaleValue(Group1ScaleValue(sum, pushScale), frameTime);
}

// The next free entry of the manager's particle table.
#define WRECKER_NEXT_PARTICLE() (particles->particles[particles->activeCount])
// The current entry of the random table, and the advance that wraps at 256.
#define WRECKER_RANDOM_TABLE() (randomTable[randomIndex])
#define WRECKER_ADVANCE_RANDOM() if (++randomIndex == 256) randomIndex = 0

// 0x00531740: sprays contact particles along a probe's slide from `from` to `to`.
// Their velocity is the wreck's (or the rigid body's) velocity plus a sideways
// jitter, plus the remaining gravity drift of the frame; frameTime * 200 particles
// are emitted per frame.  `normal` is unused, as in retail.
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
    Vector3 scaled = WreckerNormalize(velocity) * (speed + 2.0f);
    Vector3 side = WreckerNormalize(Group1WreckerCross(scaled, Vector3(0.0f, 1.0f, 0.0f)));
    Vector3 spread = side * 0.5f * speed;
    float count = frameTime * 200.0f;
    float inv = 1.0f / count;
    Vector3 step = delta * inv;                           // the step per particle
    Vector3 dirStep = Group1ScaleValue(Group1ScaleValue(particles->gravity, frameTime), inv);
    for (int i = 0; i < count; i++) {
        if (particles->activeCount != 1000) {
            WRECKER_NEXT_PARTICLE()->age = 0.0f;
            Vector3 along = Group1ScaleValue(step, (float)i);
            WRECKER_NEXT_PARTICLE()->position = along + from;
            WRECKER_NEXT_PARTICLE()->frame = sprayFrame;
            WRECKER_NEXT_PARTICLE()->size = WRECKER_RANDOM_TABLE() * 0.15f;
            WRECKER_NEXT_PARTICLE()->growth = WRECKER_RANDOM_TABLE() * 0.3f;
            WRECKER_ADVANCE_RANDOM();
            WRECKER_NEXT_PARTICLE()->lifetime = 375.0f;
            float jitter = WRECKER_RANDOM_TABLE() * 2.0f - 1.0f;
            WRECKER_ADVANCE_RANDOM();
            Vector3 drift = Group1ScaleValue(dirStep, count - (float)i);
            Vector3 sideways = Group1ScaleValue(spread, jitter);
            Vector3 sum = sideways + scaled;
            Vector3 result;
            WRECKER_NEXT_PARTICLE()->velocity = *Group1AddTo(&result, &sum, &drift);
            WRECKER_NEXT_PARTICLE()->flags = 0x14;
            particles->activeCount++;
        }
        if (++sprayFrame >= 0x2d)
            sprayFrame = 0x1d;
    }
}
