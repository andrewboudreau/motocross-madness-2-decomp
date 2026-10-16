#include <stdio.h>
#include <string.h>

#include <math.h>

#include "Camera.h"
#include "TrackGame.h"

// 0x0042e340. The two trailing copies (+0x214 from +0x17c, +0x208 from +0x170)
// are evidenced by retail reusing the registers that hold the first vectors'
// components; constructing fresh vectors there emits extra stack temporaries.
Camera::Camera(int flags) : GameObject(flags) {
    maxFieldOfView = 109.0f;
    minFieldOfView = 10.0f;
    fieldOfView = 77.0f;
    file = 0;
    field_0x1e8 = 0;
    field_0x1ec = 0;
    nearPlane = g_UnknownFloat550f6c;
    farPlane = g_UnknownFloat550f70;
    viewportClearFrames = 0;
    projectionChangeFrames = 0;
    memset(viewport, 0, sizeof(viewport));
    worldMatrix = IdentityMatrix();
    viewDirection = Vector3(0.0f, 0.0f, 1.0f);
    upDirection = Vector3(0.0f, 1.0f, 0.0f);
    eyePosition = Vector3(0.0f, 0.0f, 0.0f);
    roll = 0.0f;
    field_0x1f0 = 0;
    field_0x1fc = 0;
    field_0x1f4 = 0;
    field_0x1f8 = 0;
    lastMoveDistance = 0.0f;
    lastTurnAngle = 0.0f;
    fovKeysEnabled = 1;
    widthKeysEnabled = 1;
    previousViewDirection = viewDirection;
    previousEyePosition = eyePosition;
}

// Destructor core 0x0042f020: Camera vptr, close the +0x1e4 file if any, then
// the GameObject destructor. VC6 also emits the scalar deleting wrapper
// 0x0042e4e0 (Camera slot 0).
Camera::~Camera() {
    if (file)
        fclose(file);
}

// 0x00499ad0 (an identical forwarding body in another class shares the
// address).
void Camera::Disable() {
    GameObject::Disable();
}

// 0x0042e500: after the GameObject version, 0x0042e550 must succeed or the
// camera releases itself and returns 0; otherwise slot 28 runs and, when
// bit 0 is set, the owner is handed the camera.
GameObject* Camera::SetRenderContext(void* value) {
    GameObject::SetRenderContext(value);
    if (!UpdateViewport()) {
        Release();
        return 0;
    }
    UpdateMatrices();
    if (isEnabled)
        Owner()->SetCamera(this);
    return this;
}

// If +0x1cc is set, passes the rectangle at +0x1a0 (x, y, width, height) to
// the owner's slot 12, then calls the non-virtual 0x0042e8e0.
int Camera::RenderPre3D() {
    if (viewportClearFrames) {
        CameraRect rect;
        int x = viewport[0];
        int y = viewport[1];
        rect.left = x;
        rect.top = y;
        rect.right = x + viewport[2];
        rect.bottom = y + viewport[3];
        Owner()->Clear(&rect, 0);
    }
    ReapplyDefaultLimits();
    return 1;
}

// 0x0042e550: takes the owner's size, the current display mode's aspect
// ratio and a full-target viewport (+0x1a0: x, y, width, height and the
// minimum/maximum depth 0.0f/1.0f), submits the viewport through the
// owner's slot 14 and derives the height/width ratio from it.
// Retail keeps the display-mode table in a register but reads the current
// index again for the height: the height goes through owner->field_0x04,
// which VC6 does not merge with the `display` index read for the width
// (both reads through `display` share one index load).
int Camera::UpdateViewport() {
    RenderTarget* owner = Owner();
    targetWidth = owner->targetWidth;
    targetHeight = owner->targetHeight;
    UnknownDisplay* display = owner->targetDisplay;
    float width = (float)display->displayModes[display->currentDisplayMode].width;
    float height = (float)owner->targetDisplay->displayModes[owner->targetDisplay->currentDisplayMode].height;
    displayAspect = width / height;
    viewport[0] = 0;
    viewport[1] = 0;
    viewport[2] = targetWidth;
    viewport[3] = targetHeight;
    ((float*)viewport)[4] = 0.0f;
    ((float*)viewport)[5] = 1.0f;
    if (!owner->SetViewport(viewport))
        return 0;
    heightWidthRatio = (float)(unsigned int)viewport[3] / (float)(unsigned int)viewport[2];
    projectionChangeFrames = Owner()->frameModulus + 1;
    viewportClearFrames = Owner()->frameModulus + 1;
    return 1;
}

float FastSqrt(float value); // 0x00460b50 (FastSqrt)

// Retail adds the z term last and multiplies b's components into a's; both
// orders are visible in 0x0042e690.
static inline float Dot(const Vector3& a, const Vector3& b) {
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}

// 0x0042e690: +0x200 is how far the position moved and +0x204 the angle the
// forward direction turned since the last frame (+0x208/+0x214 keep the
// previous values). Unless bit 2 of +0x25 is set it then counts down
// +0x1d0/+0x1cc, applies the field-of-view keys (0xc7/0xcf, when +0x1d4 is
// set) and the viewport width keys (0xd2/0xd3, when +0x1d8 is set) and
// rebuilds the projection through slot 28.
int Camera::Tick(float frameTime) {
    Vector3 delta(eyePosition.x - previousEyePosition.x, eyePosition.y - previousEyePosition.y, eyePosition.z - previousEyePosition.z);
    lastMoveDistance = FastSqrt(Dot(delta, delta));
    previousEyePosition = eyePosition;
    lastTurnAngle = (float)acos(Dot(viewDirection, previousViewDirection));
    if (lastTurnAngle < 0.0f)
        lastTurnAngle = -lastTurnAngle;
    previousViewDirection = viewDirection;
    if (!isPaused) {
        if (projectionChangeFrames)
            projectionChangeFrames--;
        if (viewportClearFrames)
            viewportClearFrames--;
        if (fovKeysEnabled) {
            if (g_TrackGame->controlInterface->IsDeviceButtonDown(0xc7, 0, 0x3f, 0)) {
                fieldOfView -= 1.0f;
                if (fieldOfView < minFieldOfView)
                    fieldOfView = minFieldOfView;
                SetFieldOfView(fieldOfView);
            }
            if (g_TrackGame->controlInterface->IsDeviceButtonDown(0xcf, 0, 0x3f, 0)) {
                fieldOfView += 1.0f;
                if (fieldOfView > maxFieldOfView)
                    fieldOfView = maxFieldOfView;
                SetFieldOfView(fieldOfView);
            }
        }
        if (widthKeysEnabled) {
            if (g_TrackGame->controlInterface->IsDeviceButtonDown(0xd2, 0, 0x3f, 0) &&
                (unsigned int)viewport[2] < (unsigned int)targetWidth)
                SetViewportWidth(-1, 2, 0);
            if (g_TrackGame->controlInterface->IsDeviceButtonDown(0xd3, 0, 0x3f, 0) &&
                (unsigned int)viewport[2] > 0x20)
                SetViewportWidth(1, -2, 0);
        }
        UpdateMatrices();
    }
    return 1;
}

// 0x0042e930: a zero value keeps the current +0x16c.
void Camera::SetFieldOfView(float value) {
    if (value != 0.0)
        fieldOfView = value;
    projectionChangeFrames = Owner()->frameModulus + 1;
}

float FastInvSqrt(float value); // 0x00460c00

// v scaled to unit length (unchanged when it already is).
static inline Vector3 Normalize(const Vector3& v) {
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return v;
    return v * FastInvSqrt(squared);
}

// 0x0042e9b0
void Camera::SetFrame(const Vector3* position, const Vector3* forward, const Vector3* up,
                                   const float* roll, const float* fov) {
    if (position)
        eyePosition = *position;
    if (forward)
        viewDirection = Normalize(*forward);
    if (up)
        upDirection = Normalize(*up);
    if (roll)
        this->roll = *roll;
    if (fov)
        SetFieldOfView(*fov);
}

// 0x0042e8e0
int Camera::ReapplyDefaultLimits() {
    if (farPlane != g_UnknownFloat550f70 || nearPlane != g_UnknownFloat550f6c) {
        ClampLimits(g_UnknownFloat550f6c, g_UnknownFloat550f70);
        UpdateMatrices();
    }
    return 1;
}

// 0x0042e960
void Camera::ClampLimits(float minimum, float maximum) {
    nearPlane = (g_UnknownFloat550f6c > minimum) ? g_UnknownFloat550f6c : minimum;
    if (g_UnknownFloat550f70 < maximum)
        farPlane = g_UnknownFloat550f70;
    else
        farPlane = maximum;
}

// 0x00467ae0 (shared return-1 body)
int Camera::UnknownVirtualSlot14() {
    return 1;
}

// 0x0040ae30 (cdecl): out-of-line dot product.
float Vec3Dot(const Vector3* a, const Vector3* b);

// Vector3 seen through the out-of-line (x, y, z) constructor copy 0x00404e60.
struct CameraVec3Call : Vector3 {
    CameraVec3Call(float x_, float y_, float z_);
};

// v scaled to unit length through the out-of-line dot product.
static inline Vector3 NormalizeCall(const Vector3& v) {
    float squared = Vec3Dot(&v, &v);
    if (squared == 1.0f)
        return v;
    float scale = FastInvSqrt(squared);
    return Vector3(v.x * scale, v.y * scale, v.z * scale);
}

static inline Vector3 Diff(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 Cross(const Vector3& a, const Vector3& b) {
    Vector3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

// v scaled to unit length through the constructor copy.
static inline Vector3 NormalizeInline(const Vector3& v) {
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return v;
    float scale = FastInvSqrt(squared);
    return CameraVec3Call(scale * v.x, scale * v.y, scale * v.z);
}

// 0x0042eb10: aims the camera at `target`. Returns 0 when the target is the
// camera position; otherwise the forward direction becomes the unit vector to
// the target, the up vector is rebuilt from (0, 1, 0) through the right
// vector (the x axis when looking straight up or down) and the projection is
// marked for refresh. The `up` reference used for x, y and the first cross
// (but not the z store) is what puts the cross product's fld/fmul operands
// in retail order; with plain member access four pairs swap (659/690), with
// the reference for all three stores 675/690.
int Camera::LookAt(Vector3 target) {
    if (target.x == eyePosition.x && target.y == eyePosition.y && target.z == eyePosition.z)
        return 0;
    Vector3& up = upDirection;
    up.x = 0.0f;
    up.y = 1.0f;
    upDirection.z = 0.0f;
    viewDirection = NormalizeCall(Diff(target, eyePosition));
    float y = viewDirection.y;
    if (y < 0.0f)
        y = -y;
    Vector3 right;
    if (y != 1.0f)
        right = NormalizeInline(Cross(up, viewDirection));
    else
        right = Vector3(1.0f, 0.0f, 0.0f);
    upDirection = Cross(viewDirection, right);
    projectionChangeFrames = Owner()->frameModulus + 1;
    return 1;
}

int Camera::SetWorldMatrix(const Matrix4* value) {
    worldMatrix = *value;
    return 1;
}

int Camera::SetViewMatrix(const Matrix4* value) {
    viewMatrix = *value;
    return 1;
}

int Camera::SetProjectionMatrix(const Matrix4* value) {
    projectionMatrix = *value;
    return 1;
}

// Both call the GameObject version, then store owner(+0x18)->+0x14 plus one at
// +0x1d0. Slot 5 first hands the camera to the owner's non-virtual 0x004e8cf0.
void Camera::Enable() {
    GameObject::Enable();
    Owner()->SetCamera(this);
    projectionChangeFrames = Owner()->frameModulus + 1;
}

int Camera::OnActivate() {
    GameObject::OnActivate();
    projectionChangeFrames = Owner()->frameModulus + 1;
    return 1;
}

// 0x0042f090: only when no child handled the GameObject search does it
// consult 0x0043caa0; a nonzero answer runs slot 27.
int Camera::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (!GameObject::OnControlPress(event, entry)) {
        if (TestInputEvent(0xB7, 0, event, 0x80000000)) {
            TakeScreenshot();
            return 1;
        }
    }
    return 0;
}

// 0x0042f0e0. The width is converted as unsigned and the height compared
// unsigned against +0x1c8.
int Camera::SetViewportWidth(int x, int width, int absolute) {
    if (absolute) {
        viewport[0] = x;
        viewport[2] = width;
    } else {
        viewport[0] += x;
        viewport[2] += width;
    }
    viewport[3] = (int)((float)(unsigned int)viewport[2] * heightWidthRatio);
    if ((unsigned int)viewport[3] > (unsigned int)targetHeight)
        viewport[3] = targetHeight;
    viewport[1] = (unsigned int)(targetHeight - viewport[3]) >> 1;
    viewportClearFrames = projectionChangeFrames = Owner()->frameModulus + 1;
    return Owner()->SetViewport(viewport) != 0;
}

// 0x0042f190. Retail stores the width before y.
int Camera::SetViewport(int x, int y, int width, int height) {
    viewport[0] = x;
    viewport[2] = width;
    viewport[1] = y;
    viewport[3] = height;
    if ((unsigned int)viewport[2] > (unsigned int)targetWidth)
        viewport[2] = targetWidth;
    if ((unsigned int)viewport[3] > (unsigned int)targetHeight)
        viewport[3] = targetHeight;
    viewportClearFrames = projectionChangeFrames = Owner()->frameModulus + 1;
    return Owner()->SetViewport(viewport) != 0;
}

// 0x0042f210
void Camera::GetViewportRect(CameraRect* rect) {
    rect->left = viewport[0];
    rect->right = viewport[0] + viewport[2];
    rect->top = viewport[1];
    rect->bottom = viewport[1] + viewport[3];
}

// Retail initialises the usual four vector constants (0x005796c0, 0x005796d0,
// 0x005796e0, 0x005796b0) with 0x0042f250..0x0042f38b, after every Camera
// function, so they are defined at the end of the file here.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);
