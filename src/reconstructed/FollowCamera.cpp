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
    if (field_0x27c)
        delete field_0x27c;
    if (field_0x280)
        delete field_0x280;
    if (field_0x284)
        delete field_0x284;
    if (field_0x288)
        delete field_0x288;
    if (field_0x28c)
        delete field_0x28c;
    if (field_0x290)
        delete field_0x290;
    if (field_0x294)
        delete field_0x294;
    if (field_0x298)
        delete field_0x298;
}

// 0x00463620: only while the two points are within a tenth of Camera's
// +0x1c4/+0x1c8 in x and y, eases +0x220 toward a target that shrinks with
// the source height (capped at 335), keeping it at 20 or more.
void FollowCamera::UnknownVirtualSlot73(const Vector3& from, const Vector3& to, float dt) {
    if (to.x - from.x < field_0x1c4 * 0.1f && to.y - from.y < field_0x1c8 * 0.1f) {
        float height = FollowCameraMin(from.z, 335.0f);
        float target = field_0x220 - (height - 15.0f) * dt * 0.028f;
        field_0x220 = field_0x298->Update(target, dt);
        if (field_0x220 < 20.0f)
            field_0x220 = 20.0f;
    }
}

// 0x00463450
bool FollowCamera::UnknownFunction463450(int index, const Vector3* position, const float* a,
                                         const float* b, const float* c, const float* d,
                                         const float* e) {
    if (index < pointCount) {
        if (position)
            points[index].position = *position;
        if (a)
            points[index].field_0x0c = *a;
        if (b)
            points[index].field_0x10 = *b;
        if (c)
            points[index].field_0x14 = *c;
        if (d)
            points[index].field_0x18 = *d;
        if (e)
            points[index].field_0x1c = *e;
        return true;
    }
    return false;
}

// 0x00463520
bool FollowCamera::UnknownFunction463520(const Vector3& position, float a, float b, float c,
                                         float d, float e) {
    if (pointCount < field_0x2dc) {
        points[pointCount].position = position;
        points[pointCount].field_0x0c = a;
        points[pointCount].field_0x10 = b;
        points[pointCount].field_0x14 = c;
        points[pointCount].field_0x18 = d;
        points[pointCount].field_0x1c = e;
        pointCount++;
        return true;
    }
    return false;
}

// 0x00463140: Camera slot 8, then the +0x294/+0x298 values, presets, the
// point table and the state list (states 5-7 and values outside 0..7 are
// dropped); releases itself when no state is left.
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

// 0x00463600
void FollowCamera::UnknownFunction463600(UnknownFollowCameraSubject* subject) {
    field_0x240 = subject;
    field_0x2ec = subject->field_0x40;
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
        int code;
        if (!g_UnknownFollowCameraProjector575a98->UnknownFunction52f340(
                this, field_0xec, &points[i].position, &points[i].field_0x20, &code)) {
            field_0x220 += dt * points[i].field_0x0c;
            field_0x220 = field_0x298->Update(field_0x220, dt);
            switch (code) {
                case 1:
                    FollowCameraTurn(field_0x234, -(dt * points[i].field_0x1c));
                    break;
                case 2:
                    FollowCameraTurn(field_0x234, dt * points[i].field_0x1c);
                    break;
                case 4:
                    field_0x2d4 += dt * points[i].field_0x10;
                    if (pitch) {
                        FollowCameraTilt(field_0x22c, -(dt * points[i].field_0x18));
                    }
                    break;
                case 8:
                    field_0x2d4 -= dt * points[i].field_0x14;
                    break;
            }
            return;
        }
        if (points[i].field_0x20.z <= nearest.z) {
            second = nearest;
            nearest = points[i].field_0x20;
        } else if (points[i].field_0x20.z < second.z) {
            second = points[i].field_0x20;
        }
    }
    if (pointCount > 1 && blend)
        UnknownVirtualSlot73(nearest, second, dt);
}

// 0x004639f0: with a non-empty table, entry 0 takes the target point before
// slot 38 runs.
void FollowCamera::UnknownVirtualSlot40(float a) {
    if (pointCount > 0) {
        UnknownFunction463450(0, &targetPoint, 0, 0, 0, 0, 0);
        UnknownVirtualSlot38(a, 0, 0);
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
        result = field_0x170;
    } else if (orbit) {
        float yaw = field_0x308 + field_0x234;
        if (cameraState != 3) {
            Vector3 axis;
            axis.x = field_0x29c.z;
            axis.y = 0.0f;
            axis.z = -field_0x29c.x;
            float pitch;
            if (field_0x29c.y == 0.0f) {
                pitch = field_0x22c;
            } else {
                float scaled = field_0x22c * field_0x29c.y;
                if (scaled > 0.8f)
                    pitch = scaled;
                else
                    pitch = 0.8f;
            }
            D3DRMVectorRotate(&result, &field_0x29c, &axis, pitch);
            Vector3 turned;
            D3DRMVectorRotate(&turned, &result, &kVec3YAxis, yaw);
            turned *= -field_0x220;
            result = turned + targetPoint;
        } else {
            result = UnknownVirtualSlot50();
        }
        if (!field_0x276 && cameraState != 6) {
            field_0x2c4 = field_0x220;
            field_0x2c8 = field_0x22c;
            field_0x2cc = field_0x234;
            field_0x23c = yaw - 16.0f;
        }
    } else {
        if (field_0x23c < -8.0f) {
            field_0x2d4 = field_0x2d0;
            field_0x23c = field_0x23c + 16.0f;
            if (field_0x23c < 0.0f) {
                field_0x22c = 0.0f;
                field_0x234 = 0.0f;
                if (field_0x23c < -1.5707964f)
                    field_0x23c = field_0x2e8 - 3.1415927f;
                else
                    field_0x23c = -field_0x2e8;
            } else if (field_0x23c > 1.5707964f) {
                field_0x23c = 3.1415927f - field_0x2e8;
            } else {
                field_0x23c = field_0x2e8;
            }
            field_0x23c += UnknownVirtualSlot39();
            if (field_0x23c >= 6.2831855f)
                field_0x23c -= 6.2831855f;
            field_0x298->Set(field_0x220, FLT_MAX);
        }
        UnknownVirtualSlot40(dt);
        Vector3 direction;
        D3DRMVectorRotate(&direction, &kVec3ZAxis, &kVec3YAxis,
                          field_0x234 + field_0x23c);
        direction *= -field_0x220;
        Vector3 position = direction + base;
        result = position;
        result.y = position.y + field_0x2d4;
    }
    return result;
}

// 0x00464ee0 and 0x00464f70: feed a vector into the +0x27c..+0x284 and
// +0x288..+0x290 values with a rate chosen by the subject's +0xbe8.
void FollowCamera::UnknownVirtualSlot43(const Vector3& value) {
    float rate = (field_0x240->field_0xbe8 == 1) ? 0.25f : 0.3f;
    field_0x27c->Set(value.x, rate);
    field_0x280->Set(value.y, rate);
    field_0x284->Set(value.z, rate);
}

void FollowCamera::UnknownVirtualSlot44(const Vector3& value) {
    float rate = (field_0x240->field_0xbe8 == 1) ? 0.25f : 0.3f;
    field_0x288->Set(value.x, rate);
    field_0x28c->Set(value.y, rate);
    field_0x290->Set(value.z, rate);
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
    field_0x22c = 0.0f;
    field_0x234 = 0.0f;
    field_0x220 = 0.0f;
}

// Slots 64-67 load fixed presets (retail stores the float bit patterns as
// immediates).
void FollowCamera::UnknownVirtualSlot64() {
    field_0x22c = 0.9648179f;
    field_0x234 = -0.25621432f;
    field_0x220 = 75.98f;
}

void FollowCamera::UnknownVirtualSlot65() {
    field_0x22c = 0.6832963f;
    field_0x234 = -0.6098179f;
    field_0x220 = 22.68f;
}

void FollowCamera::UnknownVirtualSlot66() {
    field_0x22c = 1.0471975f;
    field_0x234 = 0.0f;
    field_0x220 = 11.72f;
    field_0x258 = 85.0f;
}

void FollowCamera::UnknownVirtualSlot67() {
    field_0x22c = 0.7f;
    field_0x234 = 3.1415927f;
    field_0x220 = 15.0f;
    field_0x258 = 85.0f;
}

// Copies the +0x2B4 triple, raises its second component by 3, derives +0x258
// from the horizontal (x/z) distance between Camera's +0x170 and that triple,
// clamps it to [10, 70] and passes the triple by value to slot 29. The
// parenthesized division matters: VC6 folds `/ 180.0f * 60.0f` into one
// multiply, while retail multiplies by 1/180 and then by 60.
void FollowCamera::UnknownVirtualSlot68() {
    Vector3 target = targetPoint;
    target.y += 3.0f;
    float dx = field_0x170.x - target.x;
    if (dx < 0.0f) dx = -dx;
    float dz = field_0x170.z - target.z;
    if (dz < 0.0f) dz = -dz;
    field_0x258 = ((200.0f - UnknownFunction460b50(dx * dx + dz * dz)) / 180.0f) * 60.0f + 10.0f;
    if (field_0x258 < 10.0f) field_0x258 = 10.0f;
    if (field_0x258 > 70.0f) field_0x258 = 70.0f;
    UnknownVirtualSlot29(target);
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
            field_0x258 = field_0x2f0;
            UnknownVirtualSlot63();
            break;
        case 4:
            field_0x2f0 = field_0x258;
            UnknownVirtualSlot60();
            break;
    }

    field_0x2c4 = field_0x220;
    field_0x2c8 = field_0x22c;
    field_0x2cc = field_0x234;

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
            savedParameter = field_0x258;
            cameraState = 5;
        }

        field_0x26c = 0;
    } else {
        cameraState = savedCameraState;
        field_0x258 = savedParameter;
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

// Out-of-line vector helpers that slot 46's inline normalisation calls (VC6
// did not expand them there). See src/krusty2/math/Math3D.h, whose call views
// describe the same addresses; names here are provisional.
float UnknownFunction460c00(float value);                          // 0x00460c00: 1/sqrt approximation
float UnknownFunction40ae30(const Vector3* a, const Vector3* b);   // 0x0040ae30: dot product
Vector3 UnknownFunction5015b0(const Vector3& v, float scale);      // 0x005015b0: v * scale
Vector3 UnknownFunction515600(const Vector3& a, const Vector3& b); // 0x00515600: cross product

// Inline cross product (expanded by slot 46). The parenthesised first
// products put each fsubp between the load and the store of the previous component's copy,
// as retail does (docs/VC6_OPERAND_ORDER.md section 3).
inline Vector3 FollowCameraCross(const Vector3& a, const Vector3& b) {
    return Vector3((a.y * b.z) - a.z * b.y, (a.z * b.x) - a.x * b.z, (a.x * b.y) - a.y * b.x);
}

// Inline normalisation: unit-length vectors are returned unchanged.
inline Vector3 FollowCameraNormalize(const Vector3& v) {
    float lengthSquared = UnknownFunction40ae30(&v, &v);
    if (lengthSquared == 1.0f)
        return v;
    float scale = UnknownFunction460c00(lengthSquared);
    return UnknownFunction5015b0(v, scale);
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

// 0x00466ad0: Camera's controls first; then, with the game's +0x2d4 bit 0,
// control 0x21 toggles the override (slot 70) and, while it is on, control
// 0x10 flips +0x26c and calls slot 69. Outside states 6 and 7 the raw keys
// 0x52 and 0x4c leave or enter state 5 (0x4c otherwise advances, slot 72) and
// 0x37 calls slot 59 below state 5.
int FollowCamera::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (Camera::UnknownVirtualSlot23(event, entry) || event->kind)
        return 0;
    if (g_TrackGame->field_0x2d4_bit0) {
        if (UnknownFunction43caa0(0x21, 0, event, 0xc) && cameraState != 6) {
            UnknownVirtualSlot70(g_TrackGame->field_0x1c4);
            return 1;
        }
        if (overrideActive) {
            if (!UnknownFunction43caa0(0x10, 0, event, 0xc))
                return 0;
            field_0x26c = 1 - field_0x26c;
            if (!field_0x26c)
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
            field_0x258 = savedParameter;
        } else {
            savedCameraState = cameraState;
            cameraState = 5;
            savedParameter = field_0x258;
        }
        UnknownVirtualSlot58();
        return 1;
    }
    if (control == 0x4c) {
        if (cameraState == 5) {
            cameraState = savedCameraState;
            field_0x258 = savedParameter;
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
