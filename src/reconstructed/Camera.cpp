#include <stdio.h>
#include <string.h>

#include <math.h>

#include "Camera.h"
#include "TrackGame.h"

// 0x0042e340. The two trailing copies (+0x214 from +0x17c, +0x208 from +0x170)
// are evidenced by retail reusing the registers that hold the first vectors'
// components; constructing fresh vectors there emits extra stack temporaries.
Camera::Camera(int flags) : GameObject(flags) {
    field_0x1dc = 109.0f;
    field_0x1e0 = 10.0f;
    field_0x16c = 77.0f;
    field_0x1e4 = 0;
    field_0x1e8 = 0;
    field_0x1ec = 0;
    field_0x1bc = g_UnknownFloat550f6c;
    field_0x1c0 = g_UnknownFloat550f70;
    field_0x1cc = 0;
    field_0x1d0 = 0;
    memset(field_0x1a0, 0, sizeof(field_0x1a0));
    worldMatrix = IdentityMatrix();
    field_0x17c = Vector3(0.0f, 0.0f, 1.0f);
    field_0x188 = Vector3(0.0f, 1.0f, 0.0f);
    field_0x170 = Vector3(0.0f, 0.0f, 0.0f);
    field_0x194 = 0.0f;
    field_0x1f0 = 0;
    field_0x1fc = 0;
    field_0x1f4 = 0;
    field_0x1f8 = 0;
    field_0x200 = 0.0f;
    field_0x204 = 0.0f;
    field_0x1d4 = 1;
    field_0x1d8 = 1;
    field_0x214 = field_0x17c;
    field_0x208 = field_0x170;
}

// Destructor core 0x0042f020: Camera vptr, close the +0x1e4 file if any, then
// the GameObject destructor. VC6 also emits the scalar deleting wrapper
// 0x0042e4e0 (Camera slot 0).
Camera::~Camera() {
    if (field_0x1e4)
        fclose(field_0x1e4);
}

// 0x00499ad0 (an identical forwarding body in another class shares the
// address).
void Camera::UnknownVirtualSlot4() {
    GameObject::UnknownVirtualSlot4();
}

// 0x0042e500: after the GameObject version, 0x0042e550 must succeed or the
// camera releases itself and returns 0; otherwise slot 28 runs and, when
// bit 0 is set, the owner is handed the camera.
GameObject* Camera::UnknownVirtualSlot8(void* value) {
    GameObject::UnknownVirtualSlot8(value);
    if (!UnknownFunction42e550()) {
        Release();
        return 0;
    }
    UnknownVirtualSlot28();
    if (field_0x25_bit0)
        Owner()->UnknownFunction4e8cf0(this);
    return this;
}

// If +0x1cc is set, passes the rectangle at +0x1a0 (x, y, width, height) to
// the owner's slot 12, then calls the non-virtual 0x0042e8e0.
int Camera::UnknownVirtualSlot13() {
    if (field_0x1cc) {
        CameraRect rect;
        int x = field_0x1a0[0];
        int y = field_0x1a0[1];
        rect.left = x;
        rect.top = y;
        rect.right = x + field_0x1a0[2];
        rect.bottom = y + field_0x1a0[3];
        Owner()->UnknownVirtualSlot12(&rect, 0);
    }
    UnknownFunction42e8e0();
    return 1;
}

// 0x0042e550: takes the owner's size, the current display mode's aspect
// ratio and a full-target viewport (+0x1a0: x, y, width, height and the
// minimum/maximum depth 0.0f/1.0f), submits the viewport through the
// owner's slot 14 and derives the height/width ratio from it.
// Near miss (docs/NEAR_MISS_INDEX.md): retail keeps the display-mode array in
// a register, re-reads only the current index for the height and clears edi
// between the two fild loads; this source reloads the array as well.
int Camera::UnknownFunction42e550() {
    RenderTarget* owner = Owner();
    field_0x1c4 = owner->field_0x0c;
    field_0x1c8 = owner->field_0x10;
    UnknownDisplay* display = owner->field_0x04;
    float width = (float)display->displayModes[display->currentDisplayMode].width;
    float height = (float)display->displayModes[display->currentDisplayMode].height;
    field_0x19c = width / height;
    field_0x1a0[0] = 0;
    field_0x1a0[1] = 0;
    field_0x1a0[2] = field_0x1c4;
    field_0x1a0[3] = field_0x1c8;
    ((float*)field_0x1a0)[4] = 0.0f;
    ((float*)field_0x1a0)[5] = 1.0f;
    if (!owner->UnknownVirtualSlot14(field_0x1a0))
        return 0;
    field_0x1b8 = (float)(unsigned int)field_0x1a0[3] / (float)(unsigned int)field_0x1a0[2];
    field_0x1d0 = Owner()->field_0x14 + 1;
    field_0x1cc = Owner()->field_0x14 + 1;
    return 1;
}

float UnknownFunction460b50(float value); // 0x00460b50 (FastSqrt)

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
int Camera::UnknownVirtualSlot10(float frameTime) {
    Vector3 delta(field_0x170.x - field_0x208.x, field_0x170.y - field_0x208.y, field_0x170.z - field_0x208.z);
    field_0x200 = UnknownFunction460b50(Dot(delta, delta));
    field_0x208 = field_0x170;
    field_0x204 = (float)acos(Dot(field_0x17c, field_0x214));
    if (field_0x204 < 0.0f)
        field_0x204 = -field_0x204;
    field_0x214 = field_0x17c;
    if (!field_0x25_bit2) {
        if (field_0x1d0)
            field_0x1d0--;
        if (field_0x1cc)
            field_0x1cc--;
        if (field_0x1d4) {
            if (g_TrackGame->controlInterface->UnknownVirtualSlot3(0xc7, 0, 0x3f, 0)) {
                field_0x16c -= 1.0f;
                if (field_0x16c < field_0x1e0)
                    field_0x16c = field_0x1e0;
                UnknownFunction42e930(field_0x16c);
            }
            if (g_TrackGame->controlInterface->UnknownVirtualSlot3(0xcf, 0, 0x3f, 0)) {
                field_0x16c += 1.0f;
                if (field_0x16c > field_0x1dc)
                    field_0x16c = field_0x1dc;
                UnknownFunction42e930(field_0x16c);
            }
        }
        if (field_0x1d8) {
            if (g_TrackGame->controlInterface->UnknownVirtualSlot3(0xd2, 0, 0x3f, 0) &&
                (unsigned int)field_0x1a0[2] < (unsigned int)field_0x1c4)
                UnknownFunction42f0e0(-1, 2, 0);
            if (g_TrackGame->controlInterface->UnknownVirtualSlot3(0xd3, 0, 0x3f, 0) &&
                (unsigned int)field_0x1a0[2] > 0x20)
                UnknownFunction42f0e0(1, -2, 0);
        }
        UnknownVirtualSlot28();
    }
    return 1;
}

// 0x0042e930: a zero value keeps the current +0x16c.
void Camera::UnknownFunction42e930(float value) {
    if (value != 0.0)
        field_0x16c = value;
    field_0x1d0 = Owner()->field_0x14 + 1;
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
void Camera::UnknownFunction42e9b0(const Vector3* position, const Vector3* forward, const Vector3* up,
                                   const float* roll, const float* fov) {
    if (position)
        field_0x170 = *position;
    if (forward)
        field_0x17c = Normalize(*forward);
    if (up)
        field_0x188 = Normalize(*up);
    if (roll)
        field_0x194 = *roll;
    if (fov)
        UnknownFunction42e930(*fov);
}

// 0x0042e8e0
int Camera::UnknownFunction42e8e0() {
    if (field_0x1c0 != g_UnknownFloat550f70 || field_0x1bc != g_UnknownFloat550f6c) {
        UnknownFunction42e960(g_UnknownFloat550f6c, g_UnknownFloat550f70);
        UnknownVirtualSlot28();
    }
    return 1;
}

// 0x0042e960
void Camera::UnknownFunction42e960(float minimum, float maximum) {
    field_0x1bc = (g_UnknownFloat550f6c > minimum) ? g_UnknownFloat550f6c : minimum;
    if (g_UnknownFloat550f70 < maximum)
        field_0x1c0 = g_UnknownFloat550f70;
    else
        field_0x1c0 = maximum;
}

// 0x00467ae0 (shared return-1 body)
int Camera::UnknownVirtualSlot14() {
    return 1;
}

int Camera::UnknownVirtualSlot30(const Matrix4* value) {
    worldMatrix = *value;
    return 1;
}

int Camera::UnknownVirtualSlot31(const Matrix4* value) {
    viewMatrix = *value;
    return 1;
}

int Camera::UnknownVirtualSlot32(const Matrix4* value) {
    projectionMatrix = *value;
    return 1;
}

// Both call the GameObject version, then store owner(+0x18)->+0x14 plus one at
// +0x1d0. Slot 5 first hands the camera to the owner's non-virtual 0x004e8cf0.
void Camera::UnknownVirtualSlot5() {
    GameObject::UnknownVirtualSlot5();
    Owner()->UnknownFunction4e8cf0(this);
    field_0x1d0 = Owner()->field_0x14 + 1;
}

int Camera::UnknownVirtualSlot18() {
    GameObject::UnknownVirtualSlot18();
    field_0x1d0 = Owner()->field_0x14 + 1;
    return 1;
}

// 0x0042f090: only when no child handled the GameObject search does it
// consult 0x0043caa0; a nonzero answer runs slot 27.
int Camera::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (!GameObject::UnknownVirtualSlot23(event, entry)) {
        if (UnknownFunction43caa0(0xB7, 0, event, 0x80000000)) {
            UnknownVirtualSlot27();
            return 1;
        }
    }
    return 0;
}

// 0x0042f0e0. The width is converted as unsigned and the height compared
// unsigned against +0x1c8.
int Camera::UnknownFunction42f0e0(int x, int width, int absolute) {
    if (absolute) {
        field_0x1a0[0] = x;
        field_0x1a0[2] = width;
    } else {
        field_0x1a0[0] += x;
        field_0x1a0[2] += width;
    }
    field_0x1a0[3] = (int)((float)(unsigned int)field_0x1a0[2] * field_0x1b8);
    if ((unsigned int)field_0x1a0[3] > (unsigned int)field_0x1c8)
        field_0x1a0[3] = field_0x1c8;
    field_0x1a0[1] = (unsigned int)(field_0x1c8 - field_0x1a0[3]) >> 1;
    field_0x1cc = field_0x1d0 = Owner()->field_0x14 + 1;
    return Owner()->UnknownVirtualSlot14(field_0x1a0) != 0;
}

// 0x0042f190. Retail stores the width before y.
int Camera::UnknownFunction42f190(int x, int y, int width, int height) {
    field_0x1a0[0] = x;
    field_0x1a0[2] = width;
    field_0x1a0[1] = y;
    field_0x1a0[3] = height;
    if ((unsigned int)field_0x1a0[2] > (unsigned int)field_0x1c4)
        field_0x1a0[2] = field_0x1c4;
    if ((unsigned int)field_0x1a0[3] > (unsigned int)field_0x1c8)
        field_0x1a0[3] = field_0x1c8;
    field_0x1cc = field_0x1d0 = Owner()->field_0x14 + 1;
    return Owner()->UnknownVirtualSlot14(field_0x1a0) != 0;
}

// 0x0042f210
void Camera::UnknownFunction42f210(CameraRect* rect) {
    rect->left = field_0x1a0[0];
    rect->right = field_0x1a0[0] + field_0x1a0[2];
    rect->top = field_0x1a0[1];
    rect->bottom = field_0x1a0[1] + field_0x1a0[3];
}

// Retail initialises the usual four vector constants (0x005796c0, 0x005796d0,
// 0x005796e0, 0x005796b0) with 0x0042f250..0x0042f38b, after every Camera
// function, so they are defined at the end of the file here.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);
