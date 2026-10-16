#pragma once

#include "Camera.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"

// RTTI: PCCamera : Camera. PCCamera introduces no primary slots; it overrides
// (among others) slots 13, 27 (pure in Camera) and 30-32.
//
// Slots 30-32 forward Camera's world, view and projection matrices to the
// owner's device (method 11, SetTransform, with D3DTRANSFORMSTATE_WORLD, VIEW
// and PROJECTION); slot 27 saves a screenshot.

class PCCamera : public Camera {
public:
    // No destructor is declared: retail's 0x004624d0 is the compiler-generated
    // one (a tail jump to ~Camera with no PCCamera vptr store).
    explicit PCCamera(int flags); // 0x004bed80

    virtual int RenderPre3D();
    virtual void TakeScreenshot();
    virtual int SetWorldMatrix(const Matrix4* world);       // world matrix
    virtual int SetViewMatrix(const Matrix4* view);        // view matrix
    virtual int SetProjectionMatrix(const Matrix4* projection);  // projection matrix

    // The owner of a PCCamera is a PCRenderTarget.
    PCRenderTarget* PCOwner() const { return static_cast<PCRenderTarget*>(Owner()); }
};
