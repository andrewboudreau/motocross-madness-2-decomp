// Near-miss FollowCamera candidates, kept out of src/reconstructed until they
// match. See docs/FOLLOW_CAMERA.md. Bindings:
// FollowCameraNearMisses.bindings.json.
//
// FollowCamera::Tick (0x00465c20, 3672 bytes): the per-frame
// update. Slots 75 (bool) and 55 (unsigned char) return one byte, as retail
// uses their al. The body matches except 13 bytes: retail computes the second
// D3DRMVectorRotate call's `&direction` before pushing `&position`, and in
// state 5 reserves the 0x00460b50 argument slot (`push ecx`) after the two
// squares, not before. Unchanged by /G3-/G5, /Ob2, separate locals, inline
// wrappers (for the call, the sum or the rotation), term order, a distinct
// variable for the second rotation, or extern/non-static axis vectors.
// Source shapes it needs: one function-scope `position`/`target` pair shared
// with the first-frame block, `if/else` (not ?:) for +0x2b4, the state 5
// deltas as member arithmetic, Set(targetFieldOfView, FLT_MAX) for every +0x294
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
// FollowCamera::UnknownVirtualSlot45 (0x00463a30, 4058 complete bytes):
// the previous 4055-byte estimate omitted the final three-byte ret 8.
// the control flow, the stack frame (0x20, with ebp/edi holding the 1.55/7.0
// constants) and every FPU sequence agree, but the code drifts by a few bytes
// (4046 bytes). Each joystick block is guarded by `if (activeJoystick) { ... }`:
// retail's null test jumps to the shared epilogue at 0x004649fe. The speed
// blocks name the view's side axis and the `x * speed` scale (`float xs`), which
// gives retail's fstp/fmul; a scale local declared before the axis, or
// `(x * speed)` inline, is worse. Left: at each joystick test retail tests a
// copy (mov edx, [ecx+0xc]) and reloads the pointer for the call (mov edx, ecx;
// mov ecx, [edx+0xc]) where VC6 reuses the tested register (inline free/member
// wrappers, a const reference, `!= 0`, and type-punned or offset reads of
// activeJoystick do not change this); retail returns `moved` with mov al, bl
// where VC6 emits xor al, al. The clamp/wrap must come from inline helpers
// (they keep the sum on the FPU stack), and the key queries must be macros: with inline
// helpers VC6 runs out of inline budget and calls the Vector3 constructor.
//
// FollowCamera::LoadCameraFile (0x004650e0, 1014 bytes; 519 here, 952 bytes):
// registers and the EH frame agree once the frame pointer is taken before the
// vector maths.  Normalising the second cross product straight from the call
// (`up = Vec3Normalize(FollowCameraCross(direction, side))`) gives retail's frame
// 0x18c: the product becomes a memory temporary passed by reference.  Retail also
// stores each component of the first product to a temporary, copies it to `side`
// and reads the temporary (not `side`) in the second product.  A named result
// (`Vector3 r; r.x = ...`), float locals, by-value operands (either or both) and
// assignment instead of initialisation (41 forms) keep that product on the FPU
// stack (508..513 with the separate normalisation).
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
#define MCM2_FOLLOW_CAMERA_TICK_AXES_DEFINED

#include "FollowCameraCandidateOperators.h"

#include "FollowCameraControlSupport.h"

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
    isPlaced = 0;
    targetPoint = kVec3Zero;
    cachedTarget = kVec3Zero;
    worldYaw = 0;
    targetDistance = 0;
    followHeading = kVec3ZAxis;
    points = 0;
    isCrashView = 0;
    lift = 0;
    lookAheadDistance = 27.0f;
    targetFieldOfView = fieldOfView;
    state4FieldOfView = fieldOfView;
    fieldOfViewEase = 0;
    subject = 0;
    targetEaseX = 0;
    targetEaseY = 0;
    targetEaseZ = 0;
    positionEaseX = 0;
    positionEaseY = 0;
    positionEaseZ = 0;
    distanceEase = 0;
    fovKeysEnabled = 0;
    overrideActive = 0;
    isMovingTarget = 0;
    isSettled = 0;
    pointCapacity = 0;
    pointCount = 0;
    followHeight = 7.0f;
    subjectCellSize = 3.0f;
    cameraState = 0;
    UnknownVirtualSlot71(0);
    resumeEyePosition = eyePosition;
    savedCameraState = cameraState;
    resumeState = cameraState;
    savedFieldOfView = fieldOfView;
    initialDistance = followDistance;
    initialPitch = followPitch;
    initialYaw = followYaw;
    resumeFieldOfView = fieldOfView;
    settleChecked = 0;
    wasWide = 0;
    stateCount = 0;
    stateIndex = 0;
    blendClock = 0;
    turnYaw = 0;
    isBlending = 0;
    recordsPerSecond = 15.0f;
    frameTime = 0;
    distanceFovMinimum = 10.0f;
    distanceForMinimumFov = 280.0f;
    distanceFovMaximum = 70.0f;
    distanceForMaximumFov = 20.0f;
}

// 0x004650e0: each "CAMERA" line holds two skipped words, three skipped
// numbers, a direction, an up hint and a position (each stored x, z, y) and
// one more value. The frame matrix gets the side, the corrected up and the
// direction as rows 0-2 and the position as row 3.
void FollowCamera::LoadCameraFile(const char* path) {
    UnknownTextureStream* stream =
        new(__FILE__, 0x3c7) UnknownTextureStream(g_ResourceManager);
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
    stream->Seek(stream->sliceOffset, 0, 1);
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
            Vector3 up = Vec3Normalize(FollowCameraCross(direction, side));
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

#include "FollowCameraControls.cpp"

#include "FollowCameraTick.cpp"
