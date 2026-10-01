#pragma once

#include "PCCamera.h"

// RTTI: FollowCamera : PCCamera. FollowCamera introduces primary slots 33-75;
// VehicleCamera, BikeCamera and KrustyBikeCamera inherit many of them.
// FollowCam.cpp is a source-file candidate (name overlap, nearby references),
// not a proven translation-unit assignment. Names are provisional.
//
// Observed fields:
//   Camera +0x170 (x, z) compared with the +0x2b4 triple (slot 68)
//   +0x220, +0x22c, +0x234  preset parameters (slots 63-67)
//   +0x244, +0x248  current/saved state
//   +0x24c  saved copy of +0x258
//   +0x258  float parameter, clamped to [10, 70] by slot 68
//   +0x268, +0x26c  slot 70 input byte and reset flag
//   +0x2a8  12-byte cached aggregate (slot 69)
//   +0x2b4  12-byte float triple (slot 68)
//   +0x2c4, +0x2c8, +0x2cc  snapshot of the preset parameters (slot 71)
//   +0x2f0  state-specific saved +0x258
//   +0x30c, +0x310, +0x314  cyclic index, count, inline dword table (slot 72)

struct CameraValue12 { unsigned int a; unsigned int b; unsigned int c; };

// cdecl float(float) at 0x00460b50: returns 0 for 0, otherwise an
// exponent-halving table approximation (square-root-like). Name provisional.
float UnknownFunction460b50(float value);

class FollowCamera : public PCCamera {
public:
    virtual void UnknownVirtualSlot33();
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35();
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot38();
    virtual void UnknownVirtualSlot39();
    virtual void UnknownVirtualSlot40();
    virtual void UnknownVirtualSlot41();
    virtual void UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot43(const CameraValue12& value);
    virtual void UnknownVirtualSlot44();
    virtual void UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot46();
    virtual void UnknownVirtualSlot47();
    virtual void UnknownVirtualSlot48();
    virtual void UnknownVirtualSlot49();
    virtual void UnknownVirtualSlot50();
    virtual void UnknownVirtualSlot51();
    virtual void UnknownVirtualSlot52();
    virtual void UnknownVirtualSlot53();
    virtual void UnknownVirtualSlot54();
    virtual void UnknownVirtualSlot55();
    virtual void UnknownVirtualSlot56();
    virtual CameraValue12 UnknownVirtualSlot57(int mode);
    virtual void UnknownVirtualSlot58();
    virtual void UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60();
    virtual void UnknownVirtualSlot61();
    virtual void UnknownVirtualSlot62();
    virtual void UnknownVirtualSlot63();
    virtual void UnknownVirtualSlot64();
    virtual void UnknownVirtualSlot65();
    virtual void UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual void UnknownVirtualSlot68();
    virtual void UnknownVirtualSlot69();
    virtual void UnknownVirtualSlot70(int value);
    virtual void UnknownVirtualSlot71(int value);
    virtual void UnknownVirtualSlot72();
    virtual void UnknownVirtualSlot73();
    virtual void UnknownVirtualSlot74();
    virtual void UnknownVirtualSlot75();
};
