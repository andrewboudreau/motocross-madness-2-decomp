// Near-miss FollowCamera candidates, kept out of src/reconstructed until they
// match. See docs/FOLLOW_CAMERA.md. Bindings:
// FollowCameraNearMisses.bindings.json.
//
// FollowCamera::UnknownVirtualSlot10 (0x00465c20, 3672 bytes): the per-frame
// update. Slots 75 (bool) and 55 (unsigned char) return one byte, as retail
// uses their al. The body matches except 13 bytes: retail computes the second
// D3DRMVectorRotate call's `&direction` before pushing `&position`, and in
// state 5 reserves the 0x00460b50 argument slot (`push ecx`) after the two
// squares, not before. Unchanged by /G3-/G5, /Ob2, separate locals, inline
// wrappers (for the call, the sum or the rotation), term order, a distinct
// variable for the second rotation, or extern/non-static axis vectors.
// Source shapes it needs: one function-scope `position`/`target` pair shared
// with the first-frame block, `if/else` (not ?:) for +0x2b4, the state 5
// deltas as member arithmetic, Set(field_0x258, FLT_MAX) for every +0x294
// write, and the store orders below (found by permutation).
//
// FollowCamera::UnknownVirtualSlot46 (0x004654e0, 576 bytes): 563/576. Two
// `fsubp` instructions of the second cross product are scheduled one
// instruction later than retail (before/after the copy of the first result).
//
// FollowCamera::UnknownVirtualSlot47 (0x00465720, 1265 bytes): 1203/1267. The
// matrix product needs m[][] operands (operator() operands reorder every sum);
// left are retail's first a(0,3) load hoisted above the second rep movsd and
// the interleaved (scheduled) copies of rows 1-3 into +0x188/+0x17c/+0x170.
//
// FollowCamera::UnknownVirtualSlot45 (0x00463a30, 4055 bytes plus padding):
// the control flow, the stack frame (0x20, with ebp/edi holding the 1.55/7.0
// constants) and every FPU sequence agree, but the code drifts by a few bytes
// (aligned instruction ratio 0.937; 1088 instructions against retail's 1079).
// Each joystick block is guarded by `if (activeJoystick) { ... }`: retail's
// null test jumps to the shared epilogue at 0x004649fe. Left: at each joystick
// test retail tests a copy (mov edx, [ecx+0xc]) and reloads the pointer for
// the call (mov edx, ecx; mov ecx, [edx+0xc]) where VC6 reuses the tested
// register (inline free/member wrappers do not change this), and retail stores
// the x * speed scale before multiplying (fstp/fmul) where VC6 emits
// fst/fmulp. The clamp/wrap must come from inline helpers (they keep the
// sum on the FPU stack), and the key queries must be macros: with inline
// helpers VC6 runs out of inline budget and calls the Vector3 constructor.
//
// FollowCamera::UnknownFunction4650e0 (0x004650e0, 1012 bytes): about half the
// bytes match. Registers and the EH frame agree once the frame pointer is taken
// before the vector maths; retail keeps the two cross products in memory
// temporaries (12 more frame bytes) where VC6 keeps them on the FPU stack.
//
// FollowCamera::FollowCamera (0x00462ee0, 564 bytes): every store value,
// offset and the static slot-71 call are right, but VC6 schedules two zero
// stores (+0x274, +0x2dc) into the load-delay slot of the
// +0x29c global-vector copy, where retail placed +0x2e4/+0x276. About 141
// bytes differ, all instruction order; moving those two statements anywhere
// earlier does not help.
//
// FollowCamera::UnknownVirtualSlot36 (0x00465000, 209 bytes): 12 bytes differ.
// Retail's disabled exit is `xor al, al; mov [esi+0x277], al` (one zero for
// the store and the return); every source form tried so far emits an
// immediate store followed by `xor al, al`.
#include "../../src/reconstructed/FollowCamera.h"

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/UnknownResourceManager.h"

#include <stdio.h>
#include <string.h>

// FollowCam.cpp's per-file vectors (src/reconstructed/FollowCamera.cpp).
static Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

static inline Vector3& operator*=(Vector3& v, float scale) {
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
    return v;
}

static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3& operator-=(Vector3& v, const Vector3& other) {
    v.x -= other.x;
    v.y -= other.y;
    v.z -= other.z;
    return v;
}

// Slot 45's input queries go through the game's ControlInterface each time.
// Macros, not inline functions: with inline helpers VC6 runs out of inline
// budget in slot 45 and calls the Vector3 constructor out of line.
#define FOLLOWCAMERA_CONTROLS() (g_UnknownGlobal56e26c->field_0x14)
static inline UnknownJoystickBoolView* FollowCameraJoystick(ControlInterface* controls) {
    return (UnknownJoystickBoolView*)controls->activeJoystick;
}
#define FOLLOWCAMERA_KEY(control) (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3((control), 0, 0x3f, 0))

// Inlined by slot 38: the yaw turns and wraps to [-pi, pi]; the pitch tilts
// and is clamped to +-1.55. Retail negates the step before adding it, which
// an argument expression reproduces and `-=` does not.
inline void FollowCameraTurn(float& angle, float step) {
    angle += step;
    if (angle > 3.1415927f)
        angle -= 6.2831855f;
    else if (angle < -3.1415927f)
        angle += 6.2831855f;
}

inline void FollowCameraTilt(float& angle, float step) {
    angle += step;
    if (angle > 1.55f)
        angle = 1.55f;
    else if (angle < -1.55f)
        angle = -1.55f;
}

// Slot 45's reverse keys subtract the step instead (fsubr).
inline void FollowCameraTurnBack(float& angle, float step) {
    angle -= step;
    if (angle > 3.1415927f)
        angle -= 6.2831855f;
    else if (angle < -3.1415927f)
        angle += 6.2831855f;
}

inline void FollowCameraTiltBack(float& angle, float step) {
    angle -= step;
    if (angle > 1.55f)
        angle = 1.55f;
    else if (angle < -1.55f)
        angle = -1.55f;
}

// Out-of-line vector helpers that slot 46's inline normalisation calls (VC6
// did not expand them there). See src/krusty2/math/Math3D.h, whose call views
// describe the same addresses; names here are provisional.
float UnknownFunction460c00(float value);                          // 0x00460c00: 1/sqrt approximation
float UnknownFunction40ae30(const Vector3* a, const Vector3* b);   // 0x0040ae30: dot product
Vector3 UnknownFunction5015b0(const Vector3& v, float scale);      // 0x005015b0: v * scale
Vector3 UnknownFunction515600(const Vector3& a, const Vector3& b); // 0x00515600: cross product
Vector3 UnknownFunction5087b0(const Vector3& v);                    // 0x005087b0: normalised copy

// Inline cross product (expanded by slot 46).
inline Vector3 FollowCameraCross(const Vector3& a, const Vector3& b) {
    return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// Inline normalisation: unit-length vectors are returned unchanged.
inline Vector3 FollowCameraNormalize(const Vector3& v) {
    float lengthSquared = UnknownFunction40ae30(&v, &v);
    if (lengthSquared == 1.0f)
        return v;
    float scale = UnknownFunction460c00(lengthSquared);
    return UnknownFunction5015b0(v, scale);
}


// 0x00462ee0. Virtual calls in a constructor bind statically, so slot 71 is a
// direct call here.
FollowCamera::FollowCamera(int flags) : PCCamera(flags) {
    field_0x275 = 0;
    targetPoint = kVec3Zero;
    cachedTarget = kVec3Zero;
    field_0x23c = 0;
    field_0x2c0 = 0;
    field_0x29c = kVec3ZAxis;
    points = 0;
    field_0x276 = 0;
    field_0x2d8 = 0;
    field_0x228 = 27.0f;
    field_0x258 = field_0x16c;
    field_0x2f0 = field_0x16c;
    field_0x294 = 0;
    field_0x240 = 0;
    field_0x27c = 0;
    field_0x280 = 0;
    field_0x284 = 0;
    field_0x288 = 0;
    field_0x28c = 0;
    field_0x290 = 0;
    field_0x298 = 0;
    field_0x1d4 = 0;
    overrideActive = 0;
    field_0x26c = 0;
    field_0x274 = 0;
    field_0x2dc = 0;
    pointCount = 0;
    field_0x2d4 = 7.0f;
    field_0x2ec = 3.0f;
    cameraState = 0;
    UnknownVirtualSlot71(0);
    field_0x25c = field_0x170;
    field_0x224 = field_0x220;
    savedCameraState = cameraState;
    field_0x250 = cameraState;
    savedParameter = field_0x16c;
    field_0x230 = field_0x22c;
    field_0x238 = field_0x234;
    field_0x254 = field_0x16c;
    field_0x277 = 0;
    field_0x278 = 0;
    stateCount = 0;
    stateIndex = 0;
    field_0x304 = 0;
    field_0x308 = 0;
    field_0x279 = 0;
    field_0x270 = 15.0f;
    frameTime = 0;
    field_0x2f4 = 10.0f;
    field_0x2f8 = 280.0f;
    field_0x2fc = 70.0f;
    field_0x300 = 20.0f;
}

// 0x00465000: when enabled and not forced, a point the +0x288..+0x290 values
// already sit on (within 0.01) is fed through slot 44 and reported as
// settled; +0x277 records that the check ran.
bool FollowCamera::UnknownVirtualSlot36(const Vector3& point, bool enable, bool force) {
    if (enable) {
        if (!force && !field_0x274 &&
            FollowCameraAbs(field_0x288->value - point.x) < 0.01f &&
            FollowCameraAbs(field_0x290->value - point.z) < 0.01f &&
            FollowCameraAbs(field_0x28c->value - point.y) < 0.01f) {
            UnknownVirtualSlot44(point);
            field_0x277 = true;
            return true;
        }
        field_0x277 = true;
        return false;
    }
    field_0x277 = false;
    return false;
}

// 0x004654e0: an orthonormal basis from `forward` and `up` (up is made
// perpendicular to forward, both are normalised) stored with the side vector
// as rows 0-2 of the +0x344 matrix.
void FollowCamera::UnknownVirtualSlot46(Vector3* forward, Vector3* up) {
    Vector3 z = *forward;
    Vector3 y = *up;
    y = FollowCameraCross(z, FollowCameraCross(y, z));
    y = FollowCameraNormalize(y);
    z = FollowCameraNormalize(z);
    Vector3 x = UnknownFunction515600(y, z);
    field_0x344.m[0][0] = x.x;
    field_0x344.m[0][1] = x.y;
    field_0x344.m[0][2] = x.z;
    field_0x344.m[0][3] = 0.0f;
    field_0x344.m[1][0] = y.x;
    field_0x344.m[1][1] = y.y;
    field_0x344.m[1][2] = y.z;
    field_0x344.m[1][3] = 0.0f;
    field_0x344.m[2][0] = z.x;
    field_0x344.m[2][1] = z.y;
    field_0x344.m[2][2] = z.z;
    field_0x344.m[2][3] = 0.0f;
    field_0x344.m[3][0] = 0.0f;
    field_0x344.m[3][1] = 0.0f;
    field_0x344.m[3][2] = 0.0f;
    field_0x344.m[3][3] = 1.0f;
}


// 0x00465720: while this is the owner's active camera, advances the clock,
// wraps it to the loaded records and takes the record's matrix times +0x344 as
// the view; the position (row 3, moved by `offset`) is kept 3.5 above the
// subject's ground. Otherwise slot 54 runs. Always returns true.
bool FollowCamera::UnknownVirtualSlot47(float dt, const Vector3* offset) {
    if (Owner()->field_0x08 == this) {
        frameTime += dt;
        unsigned int count = frameCount;
        float period = count * field_0x270;
        while (frameTime > period)
            frameTime -= period;
        unsigned int index = (unsigned int)(frameTime * field_0x270);
        while (index >= count)
            index -= count;
        Matrix4 a = frames[index];
        Matrix4 b = field_0x344;
        viewMatrix(0, 0) = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] +
                           a.m[0][2] * b.m[2][0] + a.m[0][3] * b.m[3][0];
        viewMatrix(0, 1) = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] +
                           a.m[0][2] * b.m[2][1] + a.m[0][3] * b.m[3][1];
        viewMatrix(0, 2) = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] +
                           a.m[0][2] * b.m[2][2] + a.m[0][3] * b.m[3][2];
        viewMatrix(0, 3) = a.m[0][0] * b.m[0][3] + a.m[0][1] * b.m[1][3] +
                           a.m[0][2] * b.m[2][3] + a.m[0][3] * b.m[3][3];
        viewMatrix(1, 0) = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] +
                           a.m[1][2] * b.m[2][0] + a.m[1][3] * b.m[3][0];
        viewMatrix(1, 1) = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] +
                           a.m[1][2] * b.m[2][1] + a.m[1][3] * b.m[3][1];
        viewMatrix(1, 2) = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] +
                           a.m[1][2] * b.m[2][2] + a.m[1][3] * b.m[3][2];
        viewMatrix(1, 3) = a.m[1][0] * b.m[0][3] + a.m[1][1] * b.m[1][3] +
                           a.m[1][2] * b.m[2][3] + a.m[1][3] * b.m[3][3];
        viewMatrix(2, 0) = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] +
                           a.m[2][2] * b.m[2][0] + a.m[2][3] * b.m[3][0];
        viewMatrix(2, 1) = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] +
                           a.m[2][2] * b.m[2][1] + a.m[2][3] * b.m[3][1];
        viewMatrix(2, 2) = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] +
                           a.m[2][2] * b.m[2][2] + a.m[2][3] * b.m[3][2];
        viewMatrix(2, 3) = a.m[2][0] * b.m[0][3] + a.m[2][1] * b.m[1][3] +
                           a.m[2][2] * b.m[2][3] + a.m[2][3] * b.m[3][3];
        viewMatrix(3, 0) = a.m[3][0] * b.m[0][0] + a.m[3][1] * b.m[1][0] +
                           a.m[3][2] * b.m[2][0] + a.m[3][3] * b.m[3][0];
        viewMatrix(3, 1) = a.m[3][0] * b.m[0][1] + a.m[3][1] * b.m[1][1] +
                           a.m[3][2] * b.m[2][1] + a.m[3][3] * b.m[3][1];
        viewMatrix(3, 2) = a.m[3][0] * b.m[0][2] + a.m[3][1] * b.m[1][2] +
                           a.m[3][2] * b.m[2][2] + a.m[3][3] * b.m[3][2];
        viewMatrix(3, 3) = a.m[3][0] * b.m[0][3] + a.m[3][1] * b.m[1][3] +
                           a.m[3][2] * b.m[2][3] + a.m[3][3] * b.m[3][3];
        field_0x17c.z = viewMatrix(2, 2);
        field_0x17c.x = viewMatrix(2, 0);
        field_0x17c.y = viewMatrix(2, 1);
        field_0x188.z = viewMatrix(1, 2);
        field_0x188.x = viewMatrix(1, 0);
        field_0x188.y = viewMatrix(1, 1);
        field_0x170.z = viewMatrix(3, 2);
        field_0x170.x = viewMatrix(3, 0);
        field_0x170.y = viewMatrix(3, 1);
        field_0x170 += *offset;
        Vector3 ground = field_0x170;
        field_0x240->UnknownFunction507c10(&ground, 0, 0, 0);
        float height = ground.y + 3.5f;
        if (height > field_0x170.y)
            field_0x170.y = height;
        UnknownFunction42e930(66.0f);
        UnknownFunction42e690(dt);
        return true;
    }
    UnknownVirtualSlot54();
    return true;
}


// 0x004650e0: each "CAMERA" line holds two skipped words, three skipped
// numbers, a direction, an up hint and a position (each stored x, z, y) and
// one more value. The frame matrix gets the side, the corrected up and the
// direction as rows 0-2 and the position as row 3.
void FollowCamera::UnknownFunction4650e0(const char* path) {
    UnknownTextureStream* stream =
        new(__FILE__, 0x3c7) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (!stream->UnknownFunction460f50(path, "rt", 0)) {
        delete stream;
        return;
    }
    char line[256];
    frameCount = 0;
    while (stream->UnknownFunction461aa0(line, 0xff)) {
        if (!_strnicmp(line, "CAMERA", 6))
            frameCount++;
    }
    frames = new(__FILE__, 0x3e1) Matrix4[frameCount];
    field_0x33c = new(__FILE__, 0x3e2) float[frameCount];
    stream->UnknownFunction461340(stream->field_0x130, 0, 1);
    int k = 0;
    while (stream->UnknownFunction461aa0(line, 0xff)) {
        if (!_strnicmp(line, "CAMERA", 6)) {
            Vector3 direction;
            Vector3 upHint;
            Vector3 position;
            sscanf(line, "%*s%*s%*f%*f%*f%f%f%f%f%f%f%f%f%f%f", &direction.x, &direction.z,
                   &direction.y, &upHint.x, &upHint.z, &upHint.y, &position.x, &position.z,
                   &position.y, &field_0x33c[k]);
            Matrix4* frame = &frames[k];
            Vector3 forward = direction;
            Vector3 side = FollowCameraCross(upHint, direction);
            Vector3 up = FollowCameraCross(direction, side);
            up = UnknownFunction5087b0(up);
            forward = UnknownFunction5087b0(forward);
            side = UnknownFunction515600(up, forward);
            frame->m[0][0] = side.x;
            frame->m[0][1] = side.y;
            frame->m[0][2] = side.z;
            frame->m[0][3] = 0.0f;
            frame->m[1][0] = up.x;
            frame->m[1][1] = up.y;
            frame->m[1][2] = up.z;
            frame->m[1][3] = 0.0f;
            frame->m[2][0] = forward.x;
            frame->m[2][1] = forward.y;
            frame->m[2][2] = forward.z;
            frame->m[2][3] = 0.0f;
            frame->m[3][0] = position.x;
            frame->m[3][1] = position.y;
            frame->m[3][2] = position.z;
            frame->m[3][3] = 1.0f;
            k++;
        }
    }
    delete stream;
}

// 0x00463a30. Inactive: clears +0x274. With the override on and +0x26c set
// (slot 23), the keys and the joystick move the cached target (+0x2a8);
// otherwise, outside states 3, 6 and 7, state 5 moves the camera position
// (+0x170) and the other states turn the pitch (+0x22c, +-1.55), the yaw
// (+0x234, wrapped to +-pi) and the distance (+0x220, at least 7). Steps are
// 300 or 30 units per second; below distance 50 the turn rate grows by
// 3 - 0.04 * distance. Returns whether the camera moved.
bool FollowCamera::UnknownVirtualSlot45(bool active, float dt) {
    bool moved = false;
    if (active) {
    if (overrideActive && field_0x26c) {
        float x;
        float y;
        float speed = dt * 300.0f;
        field_0x274 = 0;
        if (FOLLOWCAMERA_KEY(0x48))
            cachedTarget.y += speed;
        if (FOLLOWCAMERA_KEY(0x50) && field_0x2c0 > 0.0f)
            cachedTarget.y -= speed;
        if (FOLLOWCAMERA_KEY(0x4b))
            cachedTarget -= Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
        if (FOLLOWCAMERA_KEY(0x4d))
            cachedTarget += Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
        if (FOLLOWCAMERA_KEY(0x4a) || FOLLOWCAMERA_KEY(0x0c))
            cachedTarget += speed * field_0x17c;
        if ((FOLLOWCAMERA_KEY(0x4e) || FOLLOWCAMERA_KEY(0x0d)) && field_0x2c0 > 0.0)
            cachedTarget -= speed * field_0x17c;
        if (FOLLOWCAMERA_CONTROLS()->activeJoystick) {
            if (FollowCameraJoystick(FOLLOWCAMERA_CONTROLS())
                    ->UnknownVirtualSlot4(0, &x, &y)) {
                if (field_0x2c0 > 0.0f)
                    cachedTarget.y += y * speed;
                cachedTarget += Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * (x * speed);
                if (x != 0.0f || y != 0.0f)
                    return moved;
            }
            if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(2, 2, 0xc, 0))
                cachedTarget += speed * field_0x17c;
            if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(3, 2, 0xc, 0) && field_0x2c0 > 0.0)
                cachedTarget -= speed * field_0x17c;
        }
    } else if (cameraState != 3 && cameraState != 6 && cameraState != 7) {
    float x;
    float y;
    float turn = dt * 30.0f;
    float speed = dt * 300.0f;
    if (cameraState == 5) {
        if (FOLLOWCAMERA_KEY(0x48)) {
            field_0x170.y += speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x50) && field_0x2c0 > 6.5f) {
            field_0x170.y -= speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x4b)) {
            field_0x170 -= Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x4d)) {
            field_0x170 += Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x4a) || FOLLOWCAMERA_KEY(0x0c)) {
            field_0x170 -= speed * field_0x17c;
            moved = true;
        }
        if ((FOLLOWCAMERA_KEY(0x4e) || FOLLOWCAMERA_KEY(0x0d)) && field_0x2c0 > 6.5f) {
            field_0x170 += speed * field_0x17c;
            moved = true;
        }
        if (FOLLOWCAMERA_CONTROLS()->activeJoystick) {
            if (FollowCameraJoystick(FOLLOWCAMERA_CONTROLS())
                    ->UnknownVirtualSlot4(0, &x, &y)) {
                if (field_0x2c0 > 6.5f)
                    field_0x170.y += y * speed;
                field_0x170 += Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * (x * speed);
                if (x != 0.0 || y != 0.0f)
                    moved = true;
                if (x != 0.0f || y != 0.0f)
                    return moved;
            }
            if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(2, 2, 0xc, 0)) {
                field_0x170 -= speed * field_0x17c;
                moved = true;
            }
            if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(3, 2, 0xc, 0) && field_0x2c0 > 6.5f) {
                field_0x170 += speed * field_0x17c;
                moved = true;
            }
        }
        return moved;
    }
    if (FOLLOWCAMERA_KEY(0x48)) {
        if (field_0x220 < 50.0)
            FollowCameraTilt(field_0x22c, (3.0f - field_0x220 * 0.04f) * turn * 0.01745329f);
        else
            FollowCameraTilt(field_0x22c, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x50)) {
        if (field_0x220 < 50.0)
            FollowCameraTilt(field_0x22c, (3.0f - field_0x220 * 0.04f) * turn * -0.01745329f);
        else
            FollowCameraTiltBack(field_0x22c, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4b)) {
        if (field_0x220 < 50.0)
            FollowCameraTurn(field_0x234, (3.0f - field_0x220 * 0.04f) * turn * 0.01745329f);
        else
            FollowCameraTurn(field_0x234, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4d)) {
        if (field_0x220 < 50.0)
            FollowCameraTurn(field_0x234, (3.0f - field_0x220 * 0.04f) * turn * -0.01745329f);
        else
            FollowCameraTurnBack(field_0x234, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4a) || FOLLOWCAMERA_KEY(0x0c)) {
        field_0x220 += turn;
        if (field_0x220 < 7.0f)
            field_0x220 = 7.0f;
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4e) || FOLLOWCAMERA_KEY(0x0d)) {
        field_0x220 -= turn;
        if (field_0x220 < 7.0f)
            field_0x220 = 7.0f;
        moved = true;
    }
    if (FOLLOWCAMERA_CONTROLS()->activeJoystick) {
        if (FollowCameraJoystick(FOLLOWCAMERA_CONTROLS())
                ->UnknownVirtualSlot4(0, &x, &y)) {
            if (field_0x220 < 50.0)
                FollowCameraTilt(field_0x22c, (3.0f - field_0x220 * 0.04f) * y * turn * 0.01745329f);
            else
                FollowCameraTilt(field_0x22c, y * turn * 0.01745329f);
            if (field_0x220 < 50.0)
                FollowCameraTurn(field_0x234, (3.0f - field_0x220 * 0.04f) * x * turn * -0.01745329f);
            else
                FollowCameraTurn(field_0x234, x * turn * -0.01745329f);
            if (x != 0.0 || y != 0.0f)
                moved = true;
            if (x != 0.0f || y != 0.0f)
                return moved;
        }
        if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(2, 2, 0xc, 0)) {
            field_0x220 += turn;
            if (field_0x220 < 7.0f)
                field_0x220 = 7.0f;
            moved = true;
        }
        if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(3, 2, 0xc, 0)) {
            field_0x220 -= turn;
            if (field_0x220 < 7.0f)
                field_0x220 = 7.0f;
            moved = true;
        }
    }
    return moved;
    } else {
        field_0x274 = 1;
    }
    } else {
        field_0x274 = 0;
    }
    return moved;
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// 0x00465c20 (3672 bytes; near miss, see the header comment).
int FollowCamera::UnknownVirtualSlot10(float dt) {
    Vector3 position;
    Vector3 target;
    Vector3 smoothed;
    bool current = ((RenderTarget*)field_0x18)->field_0x08 == this;
    field_0x29c = UnknownVirtualSlot41();
    if (!field_0x275) {
        field_0x275 = 1;
        targetPoint = UnknownVirtualSlot57(0);
        position = field_0x29c * -field_0x220;
        position += targetPoint;
        field_0x240->UnknownFunction507c10(&position, 0, 0, 0);
        Vector3 direction = position - targetPoint;
        D3DRMVectorRotate(&position, &direction, &kVec3XAxis, field_0x22c);
        D3DRMVectorRotate(&direction, &position, &kVec3YAxis, field_0x234);
        direction *= field_0x220;
        position = direction + targetPoint;
        UnknownFunction42e9b0(&position, 0, 0, 0, 0);
        target = field_0x29c * field_0x228;
        target += targetPoint;
        field_0x240->UnknownFunction507c10(&target, 0, 0, 0);
        UnknownVirtualSlot29(target);
        field_0x278 = FollowCameraAbs(field_0x234) > 2.0f;
    }
    if (cameraState != 6 && field_0x276 && !UnknownVirtualSlot74()) {
        field_0x276 = 0;
        field_0x220 = field_0x2c4;
        field_0x22c = field_0x2c8;
        field_0x234 = field_0x2cc;
    }
    bool wide = FollowCameraAbs(field_0x234) > 2.0f;
    if (cameraState != 6 && !field_0x278 && wide)
        field_0x279 = 1;
    if (overrideActive)
        targetPoint = cachedTarget;
    else
        targetPoint = UnknownVirtualSlot57(*(int*)&dt);
    if (cameraState == 5) {
        float dx = field_0x170.x - targetPoint.x;
        float dy = field_0x170.y - targetPoint.y;
        float dz = field_0x170.z - targetPoint.z;
        field_0x2c0 = UnknownFunction460b50(dz * dz + dx * dx + dy);
    }
    bool moved = UnknownVirtualSlot45(current, dt);
    UnknownVirtualSlot42(wide);
    bool following = UnknownVirtualSlot75();
    target = UnknownVirtualSlot48(wide || field_0x274, following, *(int*)&dt);
    position = UnknownVirtualSlot49(following, target, dt);
    if (!field_0x288) {
        field_0x284 = new(__FILE__, 0x4b0) UnknownFollowCameraValue(target.z, 0.3f);
        field_0x27c = new(__FILE__, 0x4b1) UnknownFollowCameraValue(target.x, 0.3f);
        field_0x280 = new(__FILE__, 0x4b2) UnknownFollowCameraValue(target.y, 0.3f);
        UnknownVirtualSlot43(target);
        field_0x288 = new(__FILE__, 0x4b4) UnknownFollowCameraValue(position.x, 0.3f);
        field_0x28c = new(__FILE__, 0x4b5) UnknownFollowCameraValue(position.y, 0.3f);
        field_0x290 = new(__FILE__, 0x4b6) UnknownFollowCameraValue(position.z, 0.3f);
        UnknownVirtualSlot44(position);
    }
    if (cameraState == 5) {
        if (!overrideActive) {
            field_0x258 = (field_0x2f8 - field_0x2c0) / (field_0x2f8 - field_0x300) *
                              (field_0x2fc - field_0x2f4) + field_0x2f4;
            if (field_0x258 < field_0x2f4)
                field_0x258 = field_0x2f4;
            if (field_0x258 > field_0x2fc)
                field_0x258 = field_0x2fc;
            if (moved)
                field_0x258 = (field_0x258 - field_0x16c) * 0.75f + field_0x16c;
        }
    } else if (cameraState == 7) {
        float dx = field_0x170.x - target.x;
        if (dx < 0.0f) dx = -dx;
        float dz = field_0x170.z - target.z;
        if (dz < 0.0f) dz = -dz;
        field_0x258 = ((200.0f - UnknownFunction460b50(dz * dz + dx * dx)) / 180.0f) * 60.0f + 10.0f;
        if (field_0x258 < 10.0f) field_0x258 = 10.0f;
        if (field_0x258 > 70.0f) field_0x258 = 70.0f;
        field_0x294->Set(field_0x258, FLT_MAX);
    } else if (following) {
        if (cameraState != 3) {
            if (cameraState == 4)
                UnknownVirtualSlot59();
            field_0x274 = UnknownVirtualSlot36(position, wide, moved);
        }
    } else if (UnknownVirtualSlot74()) {
        field_0x276 = 1;
    }
    cachedTarget.x = field_0x27c->Update(target.x, dt);
    cachedTarget.y = field_0x280->Update(target.y, dt);
    cachedTarget.z = field_0x284->Update(target.z, dt);
    smoothed.x = field_0x288->Update(position.x, dt);
    smoothed.y = field_0x28c->Update(position.y, dt);
    smoothed.z = field_0x290->Update(position.z, dt);
    if (field_0x279) {
        UnknownVirtualSlot33();
        field_0x304 += dt;
        field_0x304 = FollowCameraMin(field_0x304, 1.0f);
        float blend = field_0x304;
        field_0x274 = 0;
        smoothed.x = (position.x - smoothed.x) * blend + smoothed.x;
        smoothed.y = (position.y - smoothed.y) * blend + smoothed.y;
        smoothed.z = (position.z - smoothed.z) * blend + smoothed.z;
        if (!moved && blend >= 1.0f) {
            UnknownVirtualSlot44(position);
            UnknownVirtualSlot43(cachedTarget);
            field_0x304 = 0.0f;
            field_0x279 = moved;
        }
    }
    if (savedCameraState == 6 || field_0x250 == 7) {
        field_0x274 = 1;
        if (field_0x250 == 7)
            field_0x250 = cameraState;
        else
            savedCameraState = cameraState;
        UnknownVirtualSlot44(position);
        UnknownVirtualSlot43(target);
        smoothed = position;
        cachedTarget = target;
    } else if (field_0x274) {
        smoothed = position;
        if (!overrideActive)
            cachedTarget = target;
    }
    UnknownVirtualSlot52(&smoothed);
    if (current) {
        float roll;
        if (cameraState == 3 && following) {
            roll = UnknownVirtualSlot51();
            UnknownVirtualSlot53();
        } else {
            roll = 0.0f;
            UnknownVirtualSlot54();
        }
        UnknownFunction42e9b0(&smoothed, 0, 0, (int)&roll, 0);
        UnknownVirtualSlot29(cachedTarget);
        if (UnknownVirtualSlot56()) {
            if (cameraState != 7 && !field_0x276) {
                field_0x250 = cameraState;
                field_0x254 = field_0x258;
                field_0x230 = field_0x22c;
                field_0x238 = field_0x234;
                field_0x224 = field_0x220;
                cameraState = 7;
                field_0x25c = field_0x170;
                UnknownVirtualSlot68();
                field_0x294->Set(field_0x258, FLT_MAX);
            }
        } else if (cameraState == 7) {
            cameraState = field_0x250;
            field_0x22c = field_0x230;
            field_0x234 = field_0x238;
            field_0x220 = field_0x224;
            field_0x258 = field_0x254;
            field_0x170 = field_0x25c;
            field_0x250 = 7;
            field_0x294->Set(field_0x258, FLT_MAX);
        }
        if (cameraState != 5 && cameraState != 7) {
            if (UnknownVirtualSlot55()) {
                if (cameraState != 6 && !field_0x276) {
                    savedCameraState = cameraState;
                    field_0x230 = field_0x22c;
                    field_0x238 = field_0x234;
                    field_0x224 = field_0x220;
                    cameraState = 6;
                    savedParameter = field_0x258;
                    UnknownVirtualSlot67();
                    field_0x294->Set(field_0x258, FLT_MAX);
                }
            } else if (cameraState == 6) {
                cameraState = savedCameraState;
                field_0x258 = savedParameter;
                savedCameraState = 6;
                field_0x22c = field_0x230;
                field_0x234 = field_0x238;
                field_0x220 = field_0x224;
                field_0x294->Set(field_0x258 = savedParameter, FLT_MAX);

            }
        }
        if (g_UnknownGlobal56e26c->field_0x1c4 || (cameraState != 5 && cameraState != 7)) {
            if (g_UnknownGlobal56e26c->field_0x14->UnknownVirtualSlot3(0xc7, 0, 0x80000000, 0)) {
                field_0x258 -= 3.0f;
                if (field_0x258 < field_0x1e0)
                    field_0x258 = field_0x1e0;
            }
            if (g_UnknownGlobal56e26c->field_0x14->UnknownVirtualSlot3(0xcf, 0, 0x80000000, 0)) {
                field_0x258 += 3.0f;
                if (field_0x258 > field_0x1dc)
                    field_0x258 = field_0x1dc;
            }
        }
        UnknownFunction42e930(field_0x294->Update(field_0x258, dt));
        UnknownFunction42e690(dt);
        field_0x278 = wide;
        return 1;
    }
    UnknownVirtualSlot54();
    field_0x278 = wide;
    return 1;
}
