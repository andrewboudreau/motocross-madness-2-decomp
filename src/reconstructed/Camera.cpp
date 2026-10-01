#include <stdio.h>

#include "Camera.h"

// Destructor core 0x0042f020: Camera vptr, close the +0x1e4 file if any, then
// the GameObject destructor. VC6 also emits the scalar deleting wrapper
// 0x0042e4e0 (Camera slot 0).
Camera::~Camera() {
    FILE* file = *reinterpret_cast<FILE**>(reinterpret_cast<char*>(this) + 0x1E4);
    if (file)
        fclose(file);
}

// If +0x1cc is set, passes the rectangle at +0x1a0 (x, y, width, height as
// ints) to the owner's slot 12, then calls the non-virtual 0x0042e8e0.
int Camera::UnknownVirtualSlot13() {
    char* p = reinterpret_cast<char*>(this);
    if (*reinterpret_cast<int*>(p + 0x1CC)) {
        CameraRect rect;
        int x = *reinterpret_cast<int*>(p + 0x1A0);
        int y = *reinterpret_cast<int*>(p + 0x1A4);
        rect.left = x;
        rect.top = y;
        rect.right = x + *reinterpret_cast<int*>(p + 0x1A8);
        rect.bottom = y + *reinterpret_cast<int*>(p + 0x1AC);
        (*reinterpret_cast<UnknownCameraOwner**>(p + 0x18))->UnknownVirtualSlot12(&rect, 0);
    }
    UnknownFunction42e8e0();
    return 1;
}

int Camera::UnknownVirtualSlot30(const CameraMatrix16* value) {
    *reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0x2C) = *value;
    return 1;
}

int Camera::UnknownVirtualSlot31(const CameraMatrix16* value) {
    *reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0xAC) = *value;
    return 1;
}

int Camera::UnknownVirtualSlot32(const CameraMatrix16* value) {
    *reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0x6C) = *value;
    return 1;
}

// Both call the GameObject version, then store owner(+0x18)->+0x14 plus one at
// +0x1d0. Slot 5 first hands the camera to the owner's non-virtual 0x004e8cf0.
void Camera::UnknownVirtualSlot5() {
    GameObject::UnknownVirtualSlot5();
    char* p = reinterpret_cast<char*>(this);
    (*reinterpret_cast<UnknownCameraOwner**>(p + 0x18))->UnknownFunction4e8cf0(this);
    *reinterpret_cast<int*>(p + 0x1D0) = *reinterpret_cast<int*>(*reinterpret_cast<char**>(p + 0x18) + 0x14) + 1;
}

int Camera::UnknownVirtualSlot18() {
    GameObject::UnknownVirtualSlot18();
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<int*>(p + 0x1D0) = *reinterpret_cast<int*>(*reinterpret_cast<char**>(p + 0x18) + 0x14) + 1;
    return 1;
}
