#pragma once

#include "PCCamera.h"

// RTTI: FollowCamera : PCCamera. FollowCamera introduces primary slots 33-75;
// VehicleCamera, BikeCamera and KrustyBikeCamera inherit many of them. Slots
// 33, 35, 39, 41, 42, 50, 51, 57 and 74 are _purecall (0x00534cfe) in
// FollowCamera's vtable, so they are pure here. FollowCam.cpp is a
// source-file candidate (name overlap, nearby references), not a proven
// translation-unit assignment. Names are provisional.

struct CameraValue12 { unsigned int a; unsigned int b; unsigned int c; };

// cdecl float(float) at 0x00460b50: returns 0 for 0, otherwise an
// exponent-halving table approximation (square-root-like). Name provisional.
float UnknownFunction460b50(float value);

class FollowCamera : public PCCamera {
public:
    virtual void UnknownVirtualSlot33() = 0;
    virtual Vector3 UnknownVirtualSlot34(int unused);
    virtual void UnknownVirtualSlot35() = 0;
    virtual void UnknownVirtualSlot36();
    virtual Vector3 UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot38();
    virtual void UnknownVirtualSlot39() = 0;
    virtual void UnknownVirtualSlot40();
    virtual void UnknownVirtualSlot41() = 0;
    virtual void UnknownVirtualSlot42() = 0;
    virtual void UnknownVirtualSlot43(const CameraValue12& value);
    virtual void UnknownVirtualSlot44();
    virtual void UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot46();
    virtual void UnknownVirtualSlot47();
    virtual void UnknownVirtualSlot48();
    virtual void UnknownVirtualSlot49();
    virtual void UnknownVirtualSlot50() = 0;
    virtual void UnknownVirtualSlot51() = 0;
    virtual void UnknownVirtualSlot52(int value);
    virtual void UnknownVirtualSlot53();
    virtual void UnknownVirtualSlot54();
    virtual void UnknownVirtualSlot55();
    virtual void UnknownVirtualSlot56();
    virtual CameraValue12 UnknownVirtualSlot57(int mode) = 0;
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
    virtual void UnknownVirtualSlot74() = 0;
    virtual int UnknownVirtualSlot75();

protected:
    float field_0x220;          // preset distance-like value (slots 63-67)
    int field_0x224;
    int field_0x228;
    float field_0x22c;          // preset angle-like value (slots 63-67)
    int field_0x230;
    float field_0x234;          // preset angle-like value (slots 63-67)
    int field_0x238;
    int field_0x23c;
    int field_0x240;
    int field_0x244;            // current state (slots 70-72, 75)
    int field_0x248;            // saved state (slot 70)
    float field_0x24c;          // saved field_0x258 (slot 70)
    int field_0x250;
    int field_0x254;
    float field_0x258;          // clamped to [10, 70] by slot 68
    int field_0x25c;
    int field_0x260;
    int field_0x264;
    int field_0x268;            // slot 70 input byte
    int field_0x26c;            // reset by slot 70
    int field_0x270[14];
    CameraValue12 field_0x2a8;  // cached slot 57 result (slot 69)
    Vector3 field_0x2b4;        // target point (slots 34, 37, 68)
    int field_0x2c0;
    float field_0x2c4;          // snapshot of field_0x220 (slot 71)
    float field_0x2c8;          // snapshot of field_0x22c
    float field_0x2cc;          // snapshot of field_0x234
    int field_0x2d0[8];
    float field_0x2f0;          // state-4 saved field_0x258 (slot 71)
    int field_0x2f4[6];
    int field_0x30c;            // cyclic index into field_0x314 (slot 72)
    int field_0x310;            // number of entries in field_0x314
    int field_0x314[1];         // state table; its length is not established
};
