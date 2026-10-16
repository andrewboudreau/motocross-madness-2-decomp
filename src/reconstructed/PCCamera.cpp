#include "PCCamera.h"

#include "D3DConstants.h"

// Forwards its argument to Camera and installs the PCCamera vptr. Emitting the
// vtable here also emits the compiler-generated destructor (0x004624d0, a tail
// jump to ~Camera) and the scalar deleting wrapper 0x004beda0, which retail
// shares between PCCamera and ShadowCamera slot 0.
PCCamera::PCCamera(int flags) : Camera(flags) {}


// 0x004bedc0: stores and submits the world transform.
int PCCamera::SetWorldMatrix(const Matrix4* world) {
    Camera::SetWorldMatrix(world);
    UnknownRenderInterface* device = PCOwner()->device;
    if (device && device->SetTransform(D3DTRANSFORMSTATE_WORLD, world))
        return 0;
    return 1;
}

// 0x004bee00: stores and submits the view transform.
int PCCamera::SetViewMatrix(const Matrix4* view) {
    Camera::SetViewMatrix(view);
    UnknownRenderInterface* device = PCOwner()->device;
    if (device && device->SetTransform(D3DTRANSFORMSTATE_VIEW, view))
        return 0;
    return 1;
}

// 0x004bee40: stores and submits the projection transform.
int PCCamera::SetProjectionMatrix(const Matrix4* projection) {
    Camera::SetProjectionMatrix(projection);
    UnknownRenderInterface* device = PCOwner()->device;
    if (device && device->SetTransform(D3DTRANSFORMSTATE_PROJECTION, projection))
        return 0;
    return 1;
}

// After the Camera version succeeds, re-sends the view and projection
// matrices when the owner's +0x08 is this camera. No null check here.
int PCCamera::RenderPre3D() {
    if (!Camera::RenderPre3D())
        return 0;
    if (Owner()->currentCamera == this) {
        PCOwner()->device->SetTransform(D3DTRANSFORMSTATE_VIEW, &viewMatrix);
        PCOwner()->device->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &projectionMatrix);
    }
    return 1;
}

// 0x004beed0: saves a screenshot of the owner (0x004c5d00), then a tail call
// to 0x00468880 on the object at 0x0056e26c.
void PCCamera::TakeScreenshot() {
    PCOwner()->SaveScreenshot();
    g_TrackGame->ResetClock();
}
