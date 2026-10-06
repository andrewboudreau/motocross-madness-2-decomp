// Near-miss FollowCamera candidates, kept out of src/reconstructed until they
// match. See docs/FOLLOW_CAMERA.md. Bindings:
// FollowCameraNearMisses.bindings.json.
//
// FollowCamera::UnknownFunction463140 (0x00463140, 520 bytes): only the
// point-table reset loop differs (about 45 bytes). Retail forms each entry
// address as `offset + points` (mov ebx, points; mov edi, eax; add edi, ebx)
// and loads the zero vector after it; VC6 emits `points + offset` for every
// indexing form tried (points[i], (points + i)->, i[points], a pointer local,
// unsigned index).
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
// constants) and every FPU sequence agree, but the code drifts by a few bytes:
// at each joystick test retail reloads ControlInterface+0x0c for the call
// (mov edx, ecx; mov ecx, [edx+0xc]) where VC6 reuses the tested register, and
// retail stores the x * speed scale before multiplying (fstp/fmul) where VC6
// emits fst/fmulp. The clamp/wrap must come from inline helpers (they keep the
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

// 0x00463140
FollowCamera* FollowCamera::UnknownFunction463140(void* value, float rate294, float rate298,
                                                  float value228, float value2d0, float value2e8,
                                                  int capacity, int count, const int* list) {
    if (!Camera::UnknownVirtualSlot8(value))
        return 0;
    field_0x294 = new(__FILE__, 0x78) UnknownFollowCameraValue(field_0x16c, rate294);
    field_0x298 = new(__FILE__, 0x79) UnknownFollowCameraValue(field_0x220, rate298);
    field_0x228 = value228;
    field_0x2d0 = value2d0;
    field_0x2d4 = value2d0;
    field_0x2dc = capacity;
    if (capacity > 0) {
        points = new(__FILE__, 0x88) UnknownFollowCameraPoint[capacity];
        for (int i = 0; i < field_0x2dc; i++) {
            points[i].position = kVec3Zero;
            points[i].field_0x0c = 0.0f;
            points[i].field_0x10 = 0.0f;
            points[i].field_0x14 = 0.0f;
            points[i].field_0x18 = 0.0f;
            points[i].field_0x1c = 0.0f;
            points[i].field_0x20 = kVec3Zero;
        }
    }
    field_0x2e8 = value2e8;
    stateCount = count;
    int kept = 0;
    for (int j = 0; j < count; j++) {
        int entry = list[j];
        if (entry < 0 || entry >= 8 || entry == 6 || entry == 7 || entry == 5)
            stateCount--;
        else
            states[kept++] = entry;
    }
    if (stateCount <= 0) {
        Release();
        return 0;
    }
    return this;
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
        if (!FOLLOWCAMERA_CONTROLS()->activeJoystick)
            return moved;
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
        if (!FOLLOWCAMERA_CONTROLS()->activeJoystick)
            return moved;
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
    if (!FOLLOWCAMERA_CONTROLS()->activeJoystick)
        return moved;
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
    return moved;
    } else {
        field_0x274 = 1;
    }
    } else {
        field_0x274 = 0;
    }
    return moved;
}
