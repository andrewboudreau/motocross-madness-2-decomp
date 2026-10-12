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
// The state 5 `push ecx` is placed in the first x87 stall slot of its block
// (after the third `fsub`, before `fld st(0)`); retail's slot (after the
// two squares) is where VC6 puts it when the block starts at `fld st(0)`,
// as in state 7 after its `je` and at every other retail site of the
// pattern. No source form moves it: dx/dy/dz as locals (any order, const,
// register, function scope), a Vector3 delta, macro or inline squares,
// inline subtractions, every association/parenthesisation, a named sum,
// an inline call wrapper, double, a switch, a block; /G6 moves it the
// other way (before the subtractions). For the rotation, a scoped or
// separate result variable, the call's return value, D3DVECTOR casts and
// `position = direction * k + target` are all worse.
// 0..15 dead initialised int locals at the top of the function change nothing.
//
// FollowCamera::UnknownVirtualSlot46 (0x004654e0) is exact in src/reconstructed/FollowCamera.cpp.
//
// FollowCamera::UnknownVirtualSlot47 (0x00465720) is exact in src/reconstructed/FollowCamera.cpp.
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
// FollowCamera::LoadCameraFile (0x004650e0, 1012 bytes): about half the
// bytes match. Registers and the EH frame agree once the frame pointer is taken
// before the vector maths; retail keeps the two cross products in memory
// temporaries (12 more frame bytes) where VC6 keeps them on the FPU stack.
// Retail stores each component of the first product to a temporary, copies
// it to `side` and reads the temporary (not `side`) in the second product.
// A named result (`Vector3 r; r.x = ...`), float locals, by-value operands
// (either or both) and assignment instead of initialisation (41 forms) keep
// frame 0x180 (508..513).
//
// FollowCamera::FollowCamera (0x00462ee0, 564 bytes; 511 of 564): every
// store value, offset and the static slot-71 call are right. Storing the
// saved state and parameter before the +0x224 copy (120 orders of the five
// post-call copies) gives retail's second +0x16c load for +0x254 (418 ->
// 511). Left: VC6 schedules two zero stores (+0x274, +0x2dc) into the
// load-delay slot of the +0x29c global-vector copy, where retail placed
// +0x2e4/+0x276. Moving the pair to any of 28 positions (either order)
// fills the slot with whichever two stores precede `mov ecx, esi` (515 at
// best, after +0x298, which is not retail's store order); moving the
// +0x29c copy (29 positions) is worse. All orders of the six statements
// after the copy (720) or of the six before the slot-71 call (720) keep a
// non-retail fill (526 at best, with more diffs); 0..33 dead int locals
// change nothing.
//
// FollowCamera::UnknownVirtualSlot36 (0x00465000) is exact in src/reconstructed/FollowCamera.cpp.
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
#define FOLLOWCAMERA_CONTROLS() (g_TrackGame->controlInterface)
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
float FastInvSqrt(float value);                          // 0x00460c00: 1/sqrt approximation
float Vec3Dot(const Vector3* a, const Vector3* b);   // 0x0040ae30: dot product
Vector3 Vec3ScaleCall(const Vector3& v, float scale);      // 0x005015b0: v * scale
Vector3 CrossProduct(const Vector3& a, const Vector3& b); // 0x00515600: cross product
Vector3 Vec3Normalize(const Vector3& v);                    // 0x005087b0: normalised copy

// Inline cross product (expanded by slot 46 and 0x004650e0). The parenthesised first
// products put each fsubp between the load and the store of the previous component's copy,
// as retail does (docs/VC6_OPERAND_ORDER.md section 3).
inline Vector3 FollowCameraCross(const Vector3& a, const Vector3& b) {
    return Vector3((a.y * b.z) - a.z * b.y, (a.z * b.x) - a.x * b.z, (a.x * b.y) - a.y * b.x);
}

// Inline normalisation: unit-length vectors are returned unchanged.
inline Vector3 FollowCameraNormalize(const Vector3& v) {
    float lengthSquared = Vec3Dot(&v, &v);
    if (lengthSquared == 1.0f)
        return v;
    float scale = FastInvSqrt(lengthSquared);
    return Vec3ScaleCall(v, scale);
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
    lift = 0;
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
    pointCapacity = 0;
    pointCount = 0;
    field_0x2d4 = 7.0f;
    field_0x2ec = 3.0f;
    cameraState = 0;
    UnknownVirtualSlot71(0);
    field_0x25c = eyePosition;
    savedCameraState = cameraState;
    field_0x250 = cameraState;
    savedParameter = field_0x16c;
    initialDistance = followDistance;
    initialPitch = followPitch;
    initialYaw = followYaw;
    field_0x254 = field_0x16c;
    field_0x277 = 0;
    field_0x278 = 0;
    stateCount = 0;
    stateIndex = 0;
    blendClock = 0;
    field_0x308 = 0;
    field_0x279 = 0;
    field_0x270 = 15.0f;
    frameTime = 0;
    field_0x2f4 = 10.0f;
    field_0x2f8 = 280.0f;
    field_0x2fc = 70.0f;
    field_0x300 = 20.0f;
}

// 0x004650e0: each "CAMERA" line holds two skipped words, three skipped
// numbers, a direction, an up hint and a position (each stored x, z, y) and
// one more value. The frame matrix gets the side, the corrected up and the
// direction as rows 0-2 and the position as row 3.
void FollowCamera::LoadCameraFile(const char* path) {
    UnknownTextureStream* stream =
        new(__FILE__, 0x3c7) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (!stream->Open(path, "rt", 0)) {
        delete stream;
        return;
    }
    char line[256];
    frameCount = 0;
    while (stream->ReadLine(line, 0xff)) {
        if (!_strnicmp(line, "CAMERA", 6))
            frameCount++;
    }
    frames = new(__FILE__, 0x3e1) Matrix4[frameCount];
    field_0x33c = new(__FILE__, 0x3e2) float[frameCount];
    stream->Seek(stream->field_0x130, 0, 1);
    int k = 0;
    while (stream->ReadLine(line, 0xff)) {
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
            up = Vec3Normalize(up);
            forward = Vec3Normalize(forward);
            side = CrossProduct(up, forward);
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
            cachedTarget += speed * viewDirection;
        if ((FOLLOWCAMERA_KEY(0x4e) || FOLLOWCAMERA_KEY(0x0d)) && field_0x2c0 > 0.0)
            cachedTarget -= speed * viewDirection;
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
                cachedTarget += speed * viewDirection;
            if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(3, 2, 0xc, 0) && field_0x2c0 > 0.0)
                cachedTarget -= speed * viewDirection;
        }
    } else if (cameraState != 3 && cameraState != 6 && cameraState != 7) {
    float x;
    float y;
    float turn = dt * 30.0f;
    float speed = dt * 300.0f;
    if (cameraState == 5) {
        if (FOLLOWCAMERA_KEY(0x48)) {
            eyePosition.y += speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x50) && field_0x2c0 > 6.5f) {
            eyePosition.y -= speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x4b)) {
            eyePosition -= Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x4d)) {
            eyePosition += Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x4a) || FOLLOWCAMERA_KEY(0x0c)) {
            eyePosition -= speed * viewDirection;
            moved = true;
        }
        if ((FOLLOWCAMERA_KEY(0x4e) || FOLLOWCAMERA_KEY(0x0d)) && field_0x2c0 > 6.5f) {
            eyePosition += speed * viewDirection;
            moved = true;
        }
        if (FOLLOWCAMERA_CONTROLS()->activeJoystick) {
            if (FollowCameraJoystick(FOLLOWCAMERA_CONTROLS())
                    ->UnknownVirtualSlot4(0, &x, &y)) {
                if (field_0x2c0 > 6.5f)
                    eyePosition.y += y * speed;
                eyePosition += Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * (x * speed);
                if (x != 0.0 || y != 0.0f)
                    moved = true;
                if (x != 0.0f || y != 0.0f)
                    return moved;
            }
            if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(2, 2, 0xc, 0)) {
                eyePosition -= speed * viewDirection;
                moved = true;
            }
            if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(3, 2, 0xc, 0) && field_0x2c0 > 6.5f) {
                eyePosition += speed * viewDirection;
                moved = true;
            }
        }
        return moved;
    }
    if (FOLLOWCAMERA_KEY(0x48)) {
        if (followDistance < 50.0)
            FollowCameraTilt(followPitch, (3.0f - followDistance * 0.04f) * turn * 0.01745329f);
        else
            FollowCameraTilt(followPitch, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x50)) {
        if (followDistance < 50.0)
            FollowCameraTilt(followPitch, (3.0f - followDistance * 0.04f) * turn * -0.01745329f);
        else
            FollowCameraTiltBack(followPitch, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4b)) {
        if (followDistance < 50.0)
            FollowCameraTurn(followYaw, (3.0f - followDistance * 0.04f) * turn * 0.01745329f);
        else
            FollowCameraTurn(followYaw, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4d)) {
        if (followDistance < 50.0)
            FollowCameraTurn(followYaw, (3.0f - followDistance * 0.04f) * turn * -0.01745329f);
        else
            FollowCameraTurnBack(followYaw, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4a) || FOLLOWCAMERA_KEY(0x0c)) {
        followDistance += turn;
        if (followDistance < 7.0f)
            followDistance = 7.0f;
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4e) || FOLLOWCAMERA_KEY(0x0d)) {
        followDistance -= turn;
        if (followDistance < 7.0f)
            followDistance = 7.0f;
        moved = true;
    }
    if (FOLLOWCAMERA_CONTROLS()->activeJoystick) {
        if (FollowCameraJoystick(FOLLOWCAMERA_CONTROLS())
                ->UnknownVirtualSlot4(0, &x, &y)) {
            if (followDistance < 50.0)
                FollowCameraTilt(followPitch, (3.0f - followDistance * 0.04f) * y * turn * 0.01745329f);
            else
                FollowCameraTilt(followPitch, y * turn * 0.01745329f);
            if (followDistance < 50.0)
                FollowCameraTurn(followYaw, (3.0f - followDistance * 0.04f) * x * turn * -0.01745329f);
            else
                FollowCameraTurn(followYaw, x * turn * -0.01745329f);
            if (x != 0.0 || y != 0.0f)
                moved = true;
            if (x != 0.0f || y != 0.0f)
                return moved;
        }
        if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(2, 2, 0xc, 0)) {
            followDistance += turn;
            if (followDistance < 7.0f)
                followDistance = 7.0f;
            moved = true;
        }
        if (FOLLOWCAMERA_CONTROLS()->UnknownVirtualSlot3(3, 2, 0xc, 0)) {
            followDistance -= turn;
            if (followDistance < 7.0f)
                followDistance = 7.0f;
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
    bool current = ((RenderTarget*)renderContext)->currentCamera == this;
    field_0x29c = UnknownVirtualSlot41();
    if (!field_0x275) {
        field_0x275 = 1;
        targetPoint = UnknownVirtualSlot57(0);
        position = field_0x29c * -followDistance;
        position += targetPoint;
        field_0x240->QueryGround(&position, 0, 0, 0);
        Vector3 direction = position - targetPoint;
        D3DRMVectorRotate(&position, &direction, &kVec3XAxis, followPitch);
        D3DRMVectorRotate(&direction, &position, &kVec3YAxis, followYaw);
        direction *= followDistance;
        position = direction + targetPoint;
        SetFrame(&position, 0, 0, 0, 0);
        target = field_0x29c * field_0x228;
        target += targetPoint;
        field_0x240->QueryGround(&target, 0, 0, 0);
        UnknownVirtualSlot29(target);
        field_0x278 = FollowCameraAbs(followYaw) > 2.0f;
    }
    if (cameraState != 6 && field_0x276 && !UnknownVirtualSlot74()) {
        field_0x276 = 0;
        followDistance = savedDistance;
        followPitch = savedPitch;
        followYaw = savedYaw;
    }
    bool wide = FollowCameraAbs(followYaw) > 2.0f;
    if (cameraState != 6 && !field_0x278 && wide)
        field_0x279 = 1;
    if (overrideActive)
        targetPoint = cachedTarget;
    else
        targetPoint = UnknownVirtualSlot57(dt);
    if (cameraState == 5) {
        float dx = eyePosition.x - targetPoint.x;
        float dy = eyePosition.y - targetPoint.y;
        float dz = eyePosition.z - targetPoint.z;
        field_0x2c0 = FastSqrt(dz * dz + dx * dx + dy);
    }
    bool moved = UnknownVirtualSlot45(current, dt);
    UnknownVirtualSlot42(wide);
    bool following = UnknownVirtualSlot75();
    target = UnknownVirtualSlot48(wide || field_0x274, following, dt);
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
        float dx = eyePosition.x - target.x;
        if (dx < 0.0f) dx = -dx;
        float dz = eyePosition.z - target.z;
        if (dz < 0.0f) dz = -dz;
        field_0x258 = ((200.0f - FastSqrt(dz * dz + dx * dx)) / 180.0f) * 60.0f + 10.0f;
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
        blendClock += dt;
        blendClock = FollowCameraMin(blendClock, 1.0f);
        float blend = blendClock;
        field_0x274 = 0;
        smoothed.x = (position.x - smoothed.x) * blend + smoothed.x;
        smoothed.y = (position.y - smoothed.y) * blend + smoothed.y;
        smoothed.z = (position.z - smoothed.z) * blend + smoothed.z;
        if (!moved && blend >= 1.0f) {
            UnknownVirtualSlot44(position);
            UnknownVirtualSlot43(cachedTarget);
            blendClock = 0.0f;
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
        SetFrame(&smoothed, 0, 0, &roll, 0);
        UnknownVirtualSlot29(cachedTarget);
        if (UnknownVirtualSlot56()) {
            if (cameraState != 7 && !field_0x276) {
                field_0x250 = cameraState;
                field_0x254 = field_0x258;
                initialPitch = followPitch;
                initialYaw = followYaw;
                initialDistance = followDistance;
                cameraState = 7;
                field_0x25c = eyePosition;
                UnknownVirtualSlot68();
                field_0x294->Set(field_0x258, FLT_MAX);
            }
        } else if (cameraState == 7) {
            cameraState = field_0x250;
            followPitch = initialPitch;
            followYaw = initialYaw;
            followDistance = initialDistance;
            field_0x258 = field_0x254;
            eyePosition = field_0x25c;
            field_0x250 = 7;
            field_0x294->Set(field_0x258, FLT_MAX);
        }
        if (cameraState != 5 && cameraState != 7) {
            if (UnknownVirtualSlot55()) {
                if (cameraState != 6 && !field_0x276) {
                    savedCameraState = cameraState;
                    initialPitch = followPitch;
                    initialYaw = followYaw;
                    initialDistance = followDistance;
                    cameraState = 6;
                    savedParameter = field_0x258;
                    UnknownVirtualSlot67();
                    field_0x294->Set(field_0x258, FLT_MAX);
                }
            } else if (cameraState == 6) {
                cameraState = savedCameraState;
                field_0x258 = savedParameter;
                savedCameraState = 6;
                followPitch = initialPitch;
                followYaw = initialYaw;
                followDistance = initialDistance;
                field_0x294->Set(field_0x258 = savedParameter, FLT_MAX);

            }
        }
        if (g_TrackGame->paused || (cameraState != 5 && cameraState != 7)) {
            if (g_TrackGame->controlInterface->UnknownVirtualSlot3(0xc7, 0, 0x80000000, 0)) {
                field_0x258 -= 3.0f;
                if (field_0x258 < field_0x1e0)
                    field_0x258 = field_0x1e0;
            }
            if (g_TrackGame->controlInterface->UnknownVirtualSlot3(0xcf, 0, 0x80000000, 0)) {
                field_0x258 += 3.0f;
                if (field_0x258 > field_0x1dc)
                    field_0x258 = field_0x1dc;
            }
        }
        SetFieldOfView(field_0x294->Update(field_0x258, dt));
        Camera::UnknownVirtualSlot10(dt);
        field_0x278 = wide;
        return 1;
    }
    UnknownVirtualSlot54();
    field_0x278 = wide;
    return 1;
}
