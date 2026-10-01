#include "PCCamera.h"

inline UnknownRenderInterface* CameraRenderInterface(PCCamera* camera) {
    char* owner = *reinterpret_cast<char**>(reinterpret_cast<char*>(camera) + 0x18);
    return *reinterpret_cast<UnknownRenderInterface**>(owner + 0x50);
}

int PCCamera::UnknownVirtualSlot30(const CameraMatrix16* value) {
    Camera::UnknownVirtualSlot30(value);
    UnknownRenderInterface* render = CameraRenderInterface(this);
    if (render && render->UnknownMethod11(1, value))
        return 0;
    return 1;
}

int PCCamera::UnknownVirtualSlot31(const CameraMatrix16* value) {
    Camera::UnknownVirtualSlot31(value);
    UnknownRenderInterface* render = CameraRenderInterface(this);
    if (render && render->UnknownMethod11(2, value))
        return 0;
    return 1;
}

int PCCamera::UnknownVirtualSlot32(const CameraMatrix16* value) {
    Camera::UnknownVirtualSlot32(value);
    UnknownRenderInterface* render = CameraRenderInterface(this);
    if (render && render->UnknownMethod11(3, value))
        return 0;
    return 1;
}

// After the Camera version succeeds, re-sends the +0xac and +0x6c blocks (kinds
// 2 and 3) when the owner's +0x08 is this camera. No null check here.
int PCCamera::UnknownVirtualSlot13() {
    if (!Camera::UnknownVirtualSlot13())
        return 0;
    char* owner = *reinterpret_cast<char**>(reinterpret_cast<char*>(this) + 0x18);
    if (*reinterpret_cast<PCCamera**>(owner + 0x08) == this) {
        CameraRenderInterface(this)->UnknownMethod11(2, reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0xAC));
        CameraRenderInterface(this)->UnknownMethod11(3, reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0x6C));
    }
    return 1;
}
