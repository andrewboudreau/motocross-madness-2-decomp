#pragma once

#include "Camera.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"

// RTTI: PCCamera : Camera. PCCamera introduces no primary slots; it overrides
// (among others) slots 13, 27 (pure in Camera) and 30-32.
//
// The overrides forward Camera's matrices to method 11 of the device at
// PCRenderTarget+0x50 (the camera's owner) with kind 1, 2 or 3; see
// RenderInterfaces.h.

class PCCamera : public Camera {
public:
    // No destructor is declared: retail's 0x004624d0 is the compiler-generated
    // one (a tail jump to ~Camera with no PCCamera vptr store).
    explicit PCCamera(int flags); // 0x004bed80

    virtual int UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot27();
    virtual int UnknownVirtualSlot30(const Matrix4* value);
    virtual int UnknownVirtualSlot31(const Matrix4* value);
    virtual int UnknownVirtualSlot32(const Matrix4* value);

    // The owner of a PCCamera is a PCRenderTarget.
    PCRenderTarget* PCOwner() const { return static_cast<PCRenderTarget*>(Owner()); }
};
