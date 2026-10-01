#include <stdio.h>
#include <string.h>

#include "Camera.h"

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
    field_0x2c = UnknownFunction4a1410();
    field_0x17c = CameraFloat3(0.0f, 0.0f, 1.0f);
    field_0x188 = CameraFloat3(0.0f, 1.0f, 0.0f);
    field_0x170 = CameraFloat3(0.0f, 0.0f, 0.0f);
    field_0x194 = 0;
    field_0x1f0 = 0;
    field_0x1fc = 0;
    field_0x1f4 = 0;
    field_0x1f8 = 0;
    field_0x200 = 0;
    field_0x204 = 0;
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

int Camera::UnknownVirtualSlot30(const CameraMatrix16* value) {
    field_0x2c = *value;
    return 1;
}

int Camera::UnknownVirtualSlot31(const CameraMatrix16* value) {
    field_0xac = *value;
    return 1;
}

int Camera::UnknownVirtualSlot32(const CameraMatrix16* value) {
    field_0x6c = *value;
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
