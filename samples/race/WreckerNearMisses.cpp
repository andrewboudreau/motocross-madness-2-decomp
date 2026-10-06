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
// The data flow is decoded, but VC6 here copies the old position directly
// into field_0xbc. Retail copies it through a stack temporary, holds the
// old up row in three FPU registers and copies the old forward row with
// integer moves. Named locals, copy-initialised and float-constructed
// locals, and by-value accessors on the frame all fail to give that mix.
// Positional score 45/552; the branch skeleton and the arithmetic match.
//
// UnknownFunction5329e0 (0x005329e0, 1312 bytes): pushes probes back out of
// the box field_0xe8/field_0xf4 and turns the mean push into the impulse
// field_0x10c. Every block matches in shape, including the per-axis clip
// (an inline helper with by-value parameters, which explains the local
// copies of the extent and the previous position) and the retail use of the
// previous position's y for the x axis. The stack frame is 0xa0 bytes here
// and 0x94 in retail, so every local slot differs (213/1314 positional).
// All 12 declaration orders of the accumulators were tried. Retail also
// builds the scaled sum with a call to the out-of-line Vector3 constructor
// (0x00404e60); VC6 here inlines it.

#include <math.h>
#include <stdlib.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Wrecker.h"

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

void Wrecker::UnknownFunction532580() {
    UnknownWreckerFrame pose;
    Vector3 last = field_0x6c.position;
    field_0xbc = last;
    field_0xb8->UnknownFunction4fca80(0, &pose);
    field_0xbc = Vector3(pose.position.x - field_0xbc.x, pose.position.y - field_0xbc.y,
                         pose.position.z - field_0xbc.z);
    Vector3 up = field_0x6c.up;
    Vector3 forward = field_0x6c.forward;
    if (up.x == pose.up.x && up.y == pose.up.y && up.z == pose.up.z) {
        if (forward.x == pose.forward.x && forward.y == pose.forward.y && forward.z == pose.forward.z)
            field_0xc8 = Vector3(0.0f, 0.0f, 0.0f);
        else
            field_0xc8 = WreckerCross(forward, pose.forward);
    } else {
        field_0xc8 = WreckerCross(up, pose.up);
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
    Vector3 force = sum * field_0x118;
    field_0x10c = UnknownFunction5015b0(force, frameTime);
}
