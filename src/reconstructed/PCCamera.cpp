#include "PCCamera.h"

#include "D3DConstants.h"

// Forwards its argument to Camera and installs the PCCamera vptr. Emitting the
// vtable here also emits the compiler-generated destructor (0x004624d0, a tail
// jump to ~Camera) and the scalar deleting wrapper 0x004beda0, which retail
// shares between PCCamera and ShadowCamera slot 0.
PCCamera::PCCamera(int flags) : Camera(flags) {}


int PCCamera::UnknownVirtualSlot30(const Matrix4* value) {
    Camera::UnknownVirtualSlot30(value);
    UnknownRenderInterface* render = PCOwner()->device;
    if (render && render->SetTransform(D3DTRANSFORMSTATE_WORLD, value))
        return 0;
    return 1;
}

int PCCamera::UnknownVirtualSlot31(const Matrix4* value) {
    Camera::UnknownVirtualSlot31(value);
    UnknownRenderInterface* render = PCOwner()->device;
    if (render && render->SetTransform(D3DTRANSFORMSTATE_VIEW, value))
        return 0;
    return 1;
}

int PCCamera::UnknownVirtualSlot32(const Matrix4* value) {
    Camera::UnknownVirtualSlot32(value);
    UnknownRenderInterface* render = PCOwner()->device;
    if (render && render->SetTransform(D3DTRANSFORMSTATE_PROJECTION, value))
        return 0;
    return 1;
}

// After the Camera version succeeds, re-sends the view and projection
// matrices when the owner's +0x08 is this camera. No null check here.
int PCCamera::UnknownVirtualSlot13() {
    if (!Camera::UnknownVirtualSlot13())
        return 0;
    if (Owner()->field_0x08 == this) {
        PCOwner()->device->SetTransform(D3DTRANSFORMSTATE_VIEW, &viewMatrix);
        PCOwner()->device->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &projectionMatrix);
    }
    return 1;
}

// 0x004beed0: saves a screenshot of the owner (0x004c5d00), then a tail call
// to 0x00468880 on the object at 0x0056e26c.
void PCCamera::UnknownVirtualSlot27() {
    PCOwner()->SaveScreenshot();
    g_TrackGame->UnknownFunction468880();
}
