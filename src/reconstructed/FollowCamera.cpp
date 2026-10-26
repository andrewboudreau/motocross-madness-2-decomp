#include "../krusty2/visibility/VisibilityClipper.h"
#include "../krusty2/broadphase/Terrain.h"
#include "FollowCamera.h"

#include "DebugAlloc.h"

// FollowCam.cpp's per-file vector constants: 0x0065b448, 0x0065b458,
// 0x0065b468 and 0x0065b438, initialised by 0x00466f10..0x004670fb
// (.CRT$XCU entries 113-116). Only FollowCam.cpp code reads them. Not const:
// slot 49 passes them to D3DRMVectorRotate, which takes LPD3DVECTOR.
static Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// D3D_OVERLOADS-style D3DVECTOR operators that MatrixUtil.h's Vector3 lacks
// (file-local, as in BikeRace.cpp and RaceStatus.cpp).
// Slot 49 needs both: scaling in place (fmul into the same vector) and a sum
// built through the constructor.
static inline Vector3& operator*=(Vector3& v, float scale) {
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
    return v;
}

static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

// 0x00463350 (scalar deleting wrapper 0x00463120): frees the owned values,
// then the Camera destructor (PCCamera's is implicit).
FollowCamera::~FollowCamera() {
    if (points)
        delete[] points;
    if (targetEaseX)
        delete targetEaseX;
    if (targetEaseY)
        delete targetEaseY;
    if (targetEaseZ)
        delete targetEaseZ;
    if (positionEaseX)
        delete positionEaseX;
    if (positionEaseY)
        delete positionEaseY;
    if (positionEaseZ)
        delete positionEaseZ;
    if (fieldOfViewEase)
        delete fieldOfViewEase;
    if (distanceEase)
        delete distanceEase;
}

// 0x00463620: only while the two points are within a tenth of Camera's
// +0x1c4/+0x1c8 in x and y, eases +0x220 toward a target that shrinks with
// the source height (capped at 335), keeping it at 20 or more.
void FollowCamera::UnknownVirtualSlot73(const Vector3& from, const Vector3& to, float dt) {
    if (to.x - from.x < targetWidth * 0.1f && to.y - from.y < targetHeight * 0.1f) {
        float height = FollowCameraMin(from.z, 335.0f);
        float target = followDistance - (height - 15.0f) * dt * 0.028f;
        followDistance = distanceEase->Update(target, dt);
        if (followDistance < 20.0f)
            followDistance = 20.0f;
    }
}

// 0x00463450
bool FollowCamera::SetTableEntry(int index, const Vector3* position, const float* distanceRate,
                                 const float* raiseRate, const float* lowerRate, const float* tiltRate,
                                 const float* yawRate) {
    if (index < pointCount) {
        if (position)
            points[index].position = *position;
        if (distanceRate)
            points[index].distanceRate = *distanceRate;
        if (raiseRate)
            points[index].raiseRate = *raiseRate;
        if (lowerRate)
            points[index].lowerRate = *lowerRate;
        if (tiltRate)
            points[index].tiltRate = *tiltRate;
        if (yawRate)
            points[index].yawRate = *yawRate;
        return true;
    }
    return false;
}

// 0x00463520
bool FollowCamera::AppendTableEntry(const Vector3& position, float distanceRate, float raiseRate,
                                    float lowerRate, float tiltRate, float yawRate) {
    if (pointCount < pointCapacity) {
        points[pointCount].position = position;
        points[pointCount].distanceRate = distanceRate;
        points[pointCount].raiseRate = raiseRate;
        points[pointCount].lowerRate = lowerRate;
        points[pointCount].tiltRate = tiltRate;
        points[pointCount].yawRate = yawRate;
        pointCount++;
        return true;
    }
    return false;
}

// 0x00463140: Camera slot 8, then the +0x294/+0x298 values, presets, the
// point table and the state list (states 5-7 and values outside 0..7 are
// dropped); releases itself when no state is left.
FollowCamera* FollowCamera::SetupFollowCamera(void* context, float rate294, float rate298,
                                                  float value228, float value2d0, float value2e8,
                                                  int capacity, int count, const int* list) {
    if (!Camera::SetRenderContext(context))
        return 0;
    fieldOfViewEase = new(__FILE__, 0x78) UnknownFollowCameraValue(fieldOfView, rate294);
    distanceEase = new(__FILE__, 0x79) UnknownFollowCameraValue(followDistance, rate298);
    lookAheadDistance = value228;
    defaultFollowHeight = value2d0;
    followHeight = value2d0;
    pointCapacity = capacity;
    if (capacity > 0) {
        points = new(__FILE__, 0x88) UnknownFollowCameraPoint[capacity];
        for (int i = 0; i < pointCapacity; i++) {
            points[i].position = kVec3Zero;
            points[i].distanceRate = 0.0f;
            points[i].raiseRate = 0.0f;
            points[i].lowerRate = 0.0f;
            points[i].tiltRate = 0.0f;
            points[i].yawRate = 0.0f;
            points[i].screenPoint = kVec3Zero;
        }
    }
    yawOffset = value2e8;
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

// 0x00463600
void FollowCamera::SetSubject(UnknownFollowCameraSubject* newSubject) {
    subject = newSubject;
    subjectCellSize = newSubject->gridCellSize;
}

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

// 0x004636e0: projects every table entry. The first one off screen (0x0052f340
// returns 0) steers the camera: +0x220 grows by its +0x0c rate and eases
// through the +0x298 value, then the returned code turns the yaw at +0x234
// (codes 1/2, wrapped to [-pi, pi]), changes +0x2d4 (codes 4/8) and, when
// `pitch` is set, the pitch at +0x22c (code 4, clamped to +-1.55). With every
// entry on screen, the two nearest screen points (by depth) go to slot 73.
void FollowCamera::UnknownVirtualSlot38(float dt, bool blend, bool pitch) {
    if (pointCount == 0)
        return;
    Vector3 nearest = Vector3(0.0f, 0.0f, FLT_MAX);
    Vector3 second;
    for (int i = 0; i < pointCount; i++) {
        unsigned int code;
        if (!reinterpret_cast<VisibilityClipper*>(g_visibilityClipper)->ProjectPoint(
                reinterpret_cast<const VisibilityCamera*>(this),
                reinterpret_cast<const VisibilityMatrix*>(screenMatrix),
                reinterpret_cast<const VisibilityBoxVec*>(&points[i].position),
                reinterpret_cast<VisibilityBoxVec*>(&points[i].screenPoint),
                &code)) {
            followDistance += dt * points[i].distanceRate;
            followDistance = distanceEase->Update(followDistance, dt);
            switch (code) {
                case 1:
                    FollowCameraTurn(followYaw, -(dt * points[i].yawRate));
                    break;
                case 2:
                    FollowCameraTurn(followYaw, dt * points[i].yawRate);
                    break;
                case 4:
                    followHeight += dt * points[i].raiseRate;
                    if (pitch) {
                        FollowCameraTilt(followPitch, -(dt * points[i].tiltRate));
                    }
                    break;
                case 8:
                    followHeight -= dt * points[i].lowerRate;
                    break;
            }
            return;
        }
        if (points[i].screenPoint.z <= nearest.z) {
            second = nearest;
            nearest = points[i].screenPoint;
        } else if (points[i].screenPoint.z < second.z) {
            second = points[i].screenPoint;
        }
    }
    if (pointCount > 1 && blend)
        UnknownVirtualSlot73(nearest, second, dt);
}

// 0x004639f0: with a non-empty table, entry 0 takes the target point before
// slot 38 runs.
void FollowCamera::UnknownVirtualSlot40(float dt) {
    if (pointCount > 0) {
        SetTableEntry(0, &targetPoint, 0, 0, 0, 0, 0);
        UnknownVirtualSlot38(dt, 0, 0);
    }
}

// 0x00464a80: the point the camera follows, by state.
Vector3 FollowCamera::UnknownVirtualSlot48(int a, bool flag, float dt) {
    Vector3 result;
    if (cameraState == 5) {
        if (overrideActive)
            result = cachedTarget;
        else
            result = UnknownVirtualSlot34(dt);
    } else if (cameraState == 7) {
        result = targetPoint;
        result.y += 3.0f;
    } else if (flag) {
        result = UnknownVirtualSlot35(a, dt);
    } else {
        result = UnknownVirtualSlot37();
    }
    return result;
}

// 0x00464b30: in states 5 and 7 the camera keeps its position. Orbiting, the
// +0x29c direction is pitched about its horizontal normal (by +0x22c, scaled
// by its y and at least 0.8) and turned about y by +0x234 + +0x308, then
// pushed +0x220 back from the target point (state 3 takes slot 50 instead);
// outside state 6 that also snapshots the presets and +0x23c. Otherwise
// +0x23c is re-based when it dropped below -8, then slot 40 runs and the camera
// sits +0x220 behind `base` at yaw +0x234 + +0x23c, raised by +0x2d4.
Vector3 FollowCamera::UnknownVirtualSlot49(bool orbit, const Vector3& base, float dt) {
    Vector3 result;
    if (cameraState == 5 || cameraState == 7) {
        result = eyePosition;
    } else if (orbit) {
        float yaw = turnYaw + followYaw;
        if (cameraState != 3) {
            Vector3 axis;
            axis.x = followHeading.z;
            axis.y = 0.0f;
            axis.z = -followHeading.x;
            float pitch;
            if (followHeading.y == 0.0f) {
                pitch = followPitch;
            } else {
                float scaled = followPitch * followHeading.y;
                if (scaled > 0.8f)
                    pitch = scaled;
                else
                    pitch = 0.8f;
            }
            D3DRMVectorRotate(&result, &followHeading, &axis, pitch);
            Vector3 turned;
            D3DRMVectorRotate(&turned, &result, &kVec3YAxis, yaw);
            turned *= -followDistance;
            result = turned + targetPoint;
        } else {
            result = UnknownVirtualSlot50();
        }
        if (!isCrashView && cameraState != 6) {
            savedDistance = followDistance;
            savedPitch = followPitch;
            savedYaw = followYaw;
            worldYaw = yaw - 16.0f;
        }
    } else {
        if (worldYaw < -8.0f) {
            followHeight = defaultFollowHeight;
            worldYaw = worldYaw + 16.0f;
            if (worldYaw < 0.0f) {
                followPitch = 0.0f;
                followYaw = 0.0f;
                if (worldYaw < -1.5707964f)
                    worldYaw = yawOffset - 3.1415927f;
                else
                    worldYaw = -yawOffset;
            } else if (worldYaw > 1.5707964f) {
                worldYaw = 3.1415927f - yawOffset;
            } else {
                worldYaw = yawOffset;
            }
            worldYaw += UnknownVirtualSlot39();
            if (worldYaw >= 6.2831855f)
                worldYaw -= 6.2831855f;
            distanceEase->Set(followDistance, FLT_MAX);
        }
        UnknownVirtualSlot40(dt);
        Vector3 direction;
        D3DRMVectorRotate(&direction, &kVec3ZAxis, &kVec3YAxis,
                          followYaw + worldYaw);
        direction *= -followDistance;
        Vector3 position = direction + base;
        result = position;
        result.y = position.y + followHeight;
    }
    return result;
}

// 0x00464ee0 and 0x00464f70: feed a vector into the +0x27c..+0x284 and
// +0x288..+0x290 values with a rate chosen by the subject's +0xbe8.
void FollowCamera::UnknownVirtualSlot43(const Vector3& value) {
    float rate = (subject->field_0xbe8 == 1) ? 0.25f : 0.3f;
    targetEaseX->Set(value.x, rate);
    targetEaseY->Set(value.y, rate);
    targetEaseZ->Set(value.z, rate);
}

void FollowCamera::UnknownVirtualSlot44(const Vector3& value) {
    float rate = (subject->field_0xbe8 == 1) ? 0.25f : 0.3f;
    positionEaseX->Set(value.x, rate);
    positionEaseY->Set(value.y, rate);
    positionEaseZ->Set(value.z, rate);
}

// Strong ABI reconstruction: slot 57 returns a 12-byte aggregate by value.
// MSVC uses a hidden return buffer and returns that buffer in EAX; retail then
// copies three dwords into this+0x2A8 and passes the cache to virtual slot 43.
// Retail forms the cache address before the call and copies straight from the
// returned buffer, i.e. the call result is assigned directly to the cache.
void FollowCamera::UnknownVirtualSlot69() {
    cachedTarget = UnknownVirtualSlot57(0);
    UnknownVirtualSlot43(cachedTarget);
}

// Semantically strong but compiler-sensitive: retail VC6 zeros EAX once and
// reuses it. Modern clang emits immediate-zero stores instead.
void FollowCamera::UnknownVirtualSlot63() {
    followPitch = 0.0f;
    followYaw = 0.0f;
    followDistance = 0.0f;
}

// Slots 64-67 load fixed presets (retail stores the float bit patterns as
// immediates).
void FollowCamera::UnknownVirtualSlot64() {
    followPitch = 0.9648179f;
    followYaw = -0.25621432f;
    followDistance = 75.98f;
}

void FollowCamera::UnknownVirtualSlot65() {
    followPitch = 0.6832963f;
    followYaw = -0.6098179f;
    followDistance = 22.68f;
}

void FollowCamera::UnknownVirtualSlot66() {
    followPitch = 1.0471975f;
    followYaw = 0.0f;
    followDistance = 11.72f;
    targetFieldOfView = 85.0f;
}

void FollowCamera::UnknownVirtualSlot67() {
    followPitch = 0.7f;
    followYaw = 3.1415927f;
    followDistance = 15.0f;
    targetFieldOfView = 85.0f;
}

// Copies the +0x2B4 triple, raises its second component by 3, derives +0x258
// from the horizontal (x/z) distance between Camera's +0x170 and that triple,
// clamps it to [10, 70] and passes the triple by value to slot 29. The
// parenthesized division matters: VC6 folds `/ 180.0f * 60.0f` into one
// multiply, while retail multiplies by 1/180 and then by 60.
void FollowCamera::UnknownVirtualSlot68() {
    Vector3 target = targetPoint;
    target.y += 3.0f;
    float dx = eyePosition.x - target.x;
    if (dx < 0.0f) dx = -dx;
    float dz = eyePosition.z - target.z;
    if (dz < 0.0f) dz = -dz;
    targetFieldOfView = ((200.0f - FastSqrt(dx * dx + dz * dz)) / 180.0f) * 60.0f + 10.0f;
    if (targetFieldOfView < 10.0f) targetFieldOfView = 10.0f;
    if (targetFieldOfView > 70.0f) targetFieldOfView = 70.0f;
    LookAt(target);
}

// State dispatcher reconstructed from retail 0x00466E50.
// States 0-4 dispatch through FollowCamera virtuals, then snapshot the exact
// preset triplet before a final update virtual.
void FollowCamera::UnknownVirtualSlot71(int value) {
    cameraState = value;
    UnknownVirtualSlot58();

    switch (static_cast<unsigned int>(cameraState)) {
        case 0:
            UnknownVirtualSlot66();
            break;
        case 1:
            UnknownVirtualSlot65();
            break;
        case 2:
            UnknownVirtualSlot64();
            break;
        case 3:
            targetFieldOfView = state4FieldOfView;
            UnknownVirtualSlot63();
            break;
        case 4:
            state4FieldOfView = targetFieldOfView;
            UnknownVirtualSlot60();
            break;
    }

    savedDistance = followDistance;
    savedPitch = followPitch;
    savedYaw = followYaw;

    UnknownVirtualSlot61();
}

// Retail behavior strongly matches a simple cyclic state-list advance.
// VC6 keeps the increment/store/compare/wrap sequence explicit; modern clang
// folds the wrap through CMOV on P6+ CPU targets, making this another useful
// historical compiler/profile calibration target.
void FollowCamera::UnknownVirtualSlot72() {
    stateIndex = stateIndex + 1;
    if (stateIndex >= stateCount)
        stateIndex = 0;

    int value = states[stateIndex];
    cameraState = value;
    UnknownVirtualSlot71(value);
}

// Strong semantic reconstruction, but not a clang smoke target.
void FollowCamera::UnknownVirtualSlot70(unsigned char value) {
    overrideActive = value;

    if (value) {
        int current = cameraState;

        if (current != 5) {
            savedCameraState = current;
            savedFieldOfView = targetFieldOfView;
            cameraState = 5;
        }

        isMovingTarget = 0;
    } else {
        cameraState = savedCameraState;
        targetFieldOfView = savedFieldOfView;
    }
}

// 0x00464a10: the target point.
Vector3 FollowCamera::UnknownVirtualSlot37() {
    Vector3 result = targetPoint;
    return result;
}

// 0x00464a40: the target point raised by 3 (as slot 68 uses it).
Vector3 FollowCamera::UnknownVirtualSlot34(float) {
    Vector3 result = targetPoint;
    result.y += 3.0f;
    return result;
}

// 0x00464e80 and 0x00464e90 (slots 53, 54 and 58-62 share the empty body).
void FollowCamera::UnknownVirtualSlot52(Vector3*) {}
void FollowCamera::UnknownVirtualSlot53() {}
void FollowCamera::UnknownVirtualSlot54() {}
void FollowCamera::UnknownVirtualSlot58() {}
void FollowCamera::UnknownVirtualSlot59() {}
void FollowCamera::UnknownVirtualSlot60() {}
void FollowCamera::UnknownVirtualSlot61() {}
void FollowCamera::UnknownVirtualSlot62() {}

// 0x00464ea0 and 0x00464ec0: slot 5 of the interface behind the 0x0056e26c
// object, with codes 0x38 and 0x2a.
unsigned char FollowCamera::UnknownVirtualSlot55() {
    return ((UnknownKeyboardBoolView*)g_TrackGame->controlInterface->keyboard)
        ->UnknownVirtualSlot5(0x38, 0x3F, 0);
}

bool FollowCamera::UnknownVirtualSlot56() {
    return ((UnknownKeyboardBoolView*)g_TrackGame->controlInterface->keyboard)
        ->UnknownVirtualSlot5(0x2A, 0x3F, 0);
}

// 0x00465000: when enabled and not forced, a point the +0x288..+0x290 values
// already sit on (within 0.01) is fed through slot 44 and reported as
// settled; +0x277 records that the check ran. The result is one variable
// joined at a single return: retail's disabled exit stores the zero from al
// (`xor al, al; mov [esi+0x277], al`), which `field_0x277 = false;
// return false;` does not give (VC6 stores the immediate).
bool FollowCamera::UnknownVirtualSlot36(const Vector3& point, bool enable, bool force) {
    bool result;
    if (enable) {
        if (!force && !isSettled &&
            FollowCameraAbs(positionEaseX->value - point.x) < 0.01f &&
            FollowCameraAbs(positionEaseZ->value - point.z) < 0.01f &&
            FollowCameraAbs(positionEaseY->value - point.y) < 0.01f) {
            UnknownVirtualSlot44(point);
            result = true;
            settleChecked = result;
        } else {
            settleChecked = true;
            result = false;
        }
    } else {
        result = false;
        settleChecked = result;
    }
    return result;
}

// Out-of-line vector helpers that slot 46's inline normalisation calls (VC6
// did not expand them there). See src/krusty2/math/Math3D.h, whose call views
// describe the same addresses; names here are provisional.
float FastInvSqrt(float value);                          // 0x00460c00: 1/sqrt approximation
struct Vec3;
float Vec3DotCall(const Vec3* a, const Vec3* b);   // 0x0040ae30: dot product
Vec3* Vec3ScaleCall(Vec3* out, const Vec3* v, float scale);      // 0x005015b0: v * scale
#include "../krusty2/math/Vec3.h"
Vec3 CrossProductCall(const Vec3& a, const Vec3& b); // 0x00515600: shared cross product

// Inline cross product (expanded by slot 46). The parenthesised first
// products put each fsubp between the load and the store of the previous component's copy,
// as retail does (docs/VC6_OPERAND_ORDER.md section 3).
inline Vector3 FollowCameraCross(const Vector3& a, const Vector3& b) {
    return Vector3((a.y * b.z) - a.z * b.y, (a.z * b.x) - a.x * b.z, (a.x * b.y) - a.y * b.x);
}

// Inline normalisation: unit-length vectors are returned unchanged.
inline Vector3 FollowCameraNormalize(const Vector3& v) {
    float lengthSquared = Vec3DotCall(reinterpret_cast<const Vec3*>(&v), reinterpret_cast<const Vec3*>(&v));
    if (lengthSquared == 1.0f)
        return v;
    float scale = FastInvSqrt(lengthSquared);
    Vector3 result;
    return *reinterpret_cast<Vector3*>(Vec3ScaleCall(reinterpret_cast<Vec3*>(&result),
        reinterpret_cast<const Vec3*>(&v), scale));
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
    Vec3 x = CrossProductCall(reinterpret_cast<const Vec3&>(y), reinterpret_cast<const Vec3&>(z));
    orientation.m[0][0] = x.x;
    orientation.m[0][1] = x.y;
    orientation.m[0][2] = x.z;
    orientation.m[0][3] = 0.0f;
    orientation.m[1][0] = y.x;
    orientation.m[1][1] = y.y;
    orientation.m[1][2] = y.z;
    orientation.m[1][3] = 0.0f;
    orientation.m[2][0] = z.x;
    orientation.m[2][1] = z.y;
    orientation.m[2][2] = z.z;
    orientation.m[2][3] = 0.0f;
    orientation.m[3][0] = 0.0f;
    orientation.m[3][1] = 0.0f;
    orientation.m[3][2] = 0.0f;
    orientation.m[3][3] = 1.0f;
}

// 0x00465720: while this is the owner's active camera, advances the clock,
// wraps it to the loaded records and takes the record's matrix times +0x344 as
// the view; the position (row 3, moved by `offset`) is kept 3.5 above the
// subject's ground. Otherwise slot 54 runs. Always returns true. The products
// are written b * a and the rows read through m[][]: with a * b VC6 loads
// orientation's element first in the (0, 0) sum (retail hoists a(0, 3) above
// the second copy), and operator() or other copy orders lose the interleaved
// row copies.
bool FollowCamera::UnknownVirtualSlot47(float dt, const Vector3* offset) {
    if (Owner()->currentCamera == this) {
        frameTime += dt;
        unsigned int count = frameCount;
        float period = count * recordsPerSecond;
        while (frameTime > period)
            frameTime -= period;
        unsigned int index = (unsigned int)(frameTime * recordsPerSecond);
        while (index >= count)
            index -= count;
        Matrix4 a = frames[index];
        Matrix4 b = orientation;
        viewMatrix(0, 0) = b.m[0][0] * a.m[0][0] + b.m[1][0] * a.m[0][1] +
                           b.m[2][0] * a.m[0][2] + b.m[3][0] * a.m[0][3];
        viewMatrix(0, 1) = b.m[0][1] * a.m[0][0] + b.m[1][1] * a.m[0][1] +
                           b.m[2][1] * a.m[0][2] + b.m[3][1] * a.m[0][3];
        viewMatrix(0, 2) = b.m[0][2] * a.m[0][0] + b.m[1][2] * a.m[0][1] +
                           b.m[2][2] * a.m[0][2] + b.m[3][2] * a.m[0][3];
        viewMatrix(0, 3) = b.m[0][3] * a.m[0][0] + b.m[1][3] * a.m[0][1] +
                           b.m[2][3] * a.m[0][2] + b.m[3][3] * a.m[0][3];
        viewMatrix(1, 0) = b.m[0][0] * a.m[1][0] + b.m[1][0] * a.m[1][1] +
                           b.m[2][0] * a.m[1][2] + b.m[3][0] * a.m[1][3];
        viewMatrix(1, 1) = b.m[0][1] * a.m[1][0] + b.m[1][1] * a.m[1][1] +
                           b.m[2][1] * a.m[1][2] + b.m[3][1] * a.m[1][3];
        viewMatrix(1, 2) = b.m[0][2] * a.m[1][0] + b.m[1][2] * a.m[1][1] +
                           b.m[2][2] * a.m[1][2] + b.m[3][2] * a.m[1][3];
        viewMatrix(1, 3) = b.m[0][3] * a.m[1][0] + b.m[1][3] * a.m[1][1] +
                           b.m[2][3] * a.m[1][2] + b.m[3][3] * a.m[1][3];
        viewMatrix(2, 0) = b.m[0][0] * a.m[2][0] + b.m[1][0] * a.m[2][1] +
                           b.m[2][0] * a.m[2][2] + b.m[3][0] * a.m[2][3];
        viewMatrix(2, 1) = b.m[0][1] * a.m[2][0] + b.m[1][1] * a.m[2][1] +
                           b.m[2][1] * a.m[2][2] + b.m[3][1] * a.m[2][3];
        viewMatrix(2, 2) = b.m[0][2] * a.m[2][0] + b.m[1][2] * a.m[2][1] +
                           b.m[2][2] * a.m[2][2] + b.m[3][2] * a.m[2][3];
        viewMatrix(2, 3) = b.m[0][3] * a.m[2][0] + b.m[1][3] * a.m[2][1] +
                           b.m[2][3] * a.m[2][2] + b.m[3][3] * a.m[2][3];
        viewMatrix(3, 0) = b.m[0][0] * a.m[3][0] + b.m[1][0] * a.m[3][1] +
                           b.m[2][0] * a.m[3][2] + b.m[3][0] * a.m[3][3];
        viewMatrix(3, 1) = b.m[0][1] * a.m[3][0] + b.m[1][1] * a.m[3][1] +
                           b.m[2][1] * a.m[3][2] + b.m[3][1] * a.m[3][3];
        viewMatrix(3, 2) = b.m[0][2] * a.m[3][0] + b.m[1][2] * a.m[3][1] +
                           b.m[2][2] * a.m[3][2] + b.m[3][2] * a.m[3][3];
        viewMatrix(3, 3) = b.m[0][3] * a.m[3][0] + b.m[1][3] * a.m[3][1] +
                           b.m[2][3] * a.m[3][2] + b.m[3][3] * a.m[3][3];
        viewDirection.x = viewMatrix.m[2][0];
        viewDirection.y = viewMatrix.m[2][1];
        viewDirection.z = viewMatrix.m[2][2];
        upDirection.x = viewMatrix.m[1][0];
        upDirection.y = viewMatrix.m[1][1];
        upDirection.z = viewMatrix.m[1][2];
        eyePosition.x = viewMatrix.m[3][0];
        eyePosition.y = viewMatrix.m[3][1];
        eyePosition.z = viewMatrix.m[3][2];
        eyePosition += *offset;
        Vector3 ground = eyePosition;
        reinterpret_cast<Terrain*>(subject)->QueryGround(reinterpret_cast<TerrainVec3*>(&ground), 0, 0, 0);
        float height = ground.y + 3.5f;
        if (height > eyePosition.y)
            eyePosition.y = height;
        SetFieldOfView(66.0f);
        Camera::Tick(dt);
        return true;
    }
    UnknownVirtualSlot54();
    return true;
}

// 0x00466ad0: Camera's controls first; then, with the game's +0x2d4 bit 0,
// control 0x21 toggles the override (slot 70) and, while it is on, control
// 0x10 flips +0x26c and calls slot 69. Outside states 6 and 7 the raw keys
// 0x52 and 0x4c leave or enter state 5 (0x4c otherwise advances, slot 72) and
// 0x37 calls slot 59 below state 5.
int FollowCamera::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (Camera::OnControlPress(event, entry) || event->kind)
        return 0;
    if (g_TrackGame->allowFreezeCamera) {
        if (TestInputEvent(0x21, 0, event, 0xc) && cameraState != 6) {
            UnknownVirtualSlot70(g_TrackGame->paused);
            return 1;
        }
        if (overrideActive) {
            if (!TestInputEvent(0x10, 0, event, 0xc))
                return 0;
            isMovingTarget = 1 - isMovingTarget;
            if (!isMovingTarget)
                return 0;
            UnknownVirtualSlot69();
            return 1;
        }
    }
    if (cameraState == 6 || cameraState == 7)
        return 0;
    int control = event->control;
    if (control == 0x52) {
        if (cameraState == 5) {
            cameraState = savedCameraState;
            targetFieldOfView = savedFieldOfView;
        } else {
            savedCameraState = cameraState;
            cameraState = 5;
            savedFieldOfView = targetFieldOfView;
        }
        UnknownVirtualSlot58();
        return 1;
    }
    if (control == 0x4c) {
        if (cameraState == 5) {
            cameraState = savedCameraState;
            targetFieldOfView = savedFieldOfView;
            UnknownVirtualSlot58();
            return 1;
        }
        UnknownVirtualSlot72();
        return 1;
    }
    if (control == 0x37 && cameraState < 5) {
        UnknownVirtualSlot59();
        return 1;
    }
    return 0;
}
