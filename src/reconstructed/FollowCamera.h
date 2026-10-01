#pragma once

#include "PCCamera.h"

// RTTI: FollowCamera : PCCamera. FollowCamera introduces primary slots 33-75;
// VehicleCamera, BikeCamera and KrustyBikeCamera inherit many of them. Slots
// 33, 35, 39, 41, 42, 50, 51, 57 and 74 are _purecall (0x00534cfe) in
// FollowCamera's vtable, so they are pure here. FollowCam.cpp is a
// source-file candidate (name overlap, nearby references), not a proven
// translation-unit assignment. Names are provisional.


// cdecl float(float) at 0x00460b50: returns 0 for 0, otherwise an
// exponent-halving table approximation (square-root-like). Name provisional.
float UnknownFunction460b50(float value);

// 20-byte object allocated by 0x00463140 (FollowCam.cpp lines 0x78 onward):
// a value, an integer argument, a 1.0 scale and FLT_MAX / -FLT_MAX bounds.
struct UnknownFollowCameraValue {
    float value;
    int argument;
    float scale;    // 1.0f
    float maximum;  // FLT_MAX
    float minimum;  // -FLT_MAX
};

// .bss vectors copied by the constructor.
extern Vector3 g_UnknownVector65b438;
extern Vector3 g_UnknownVector65b448;

class FollowCamera : public PCCamera {
public:
    explicit FollowCamera(int flags); // 0x00462ee0 (near-miss in samples/camera)
    virtual ~FollowCamera();          // destructor core 0x00463350

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
    virtual void UnknownVirtualSlot43(const Vector3& value);
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
    virtual Vector3 UnknownVirtualSlot57(int mode) = 0;
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
    // Layout from the constructor (0x00462ee0) and destructor (0x00463350).
    float field_0x220;          // preset distance-like value (slots 63-67)
    float field_0x224;          // copy of field_0x220 (constructor)
    float field_0x228;          // 27.0f
    float field_0x22c;          // preset angle-like value (slots 63-67)
    float field_0x230;          // copy of field_0x22c
    float field_0x234;          // preset angle-like value (slots 63-67)
    float field_0x238;          // copy of field_0x234
    int field_0x23c;
    int field_0x240;
    int field_0x244;            // current state (slots 70-72, 75)
    int field_0x248;            // saved state (slot 70)
    float field_0x24c;          // saved field_0x258 (slot 70)
    int field_0x250;
    float field_0x254;
    float field_0x258;          // clamped to [10, 70] by slot 68
    Vector3 field_0x25c;        // copy of Camera field_0x170
    int field_0x268;            // slot 70 input byte
    int field_0x26c;            // reset by slot 70
    float field_0x270;          // 15.0f
    unsigned char field_0x274;
    unsigned char field_0x275;
    unsigned char field_0x276;
    unsigned char field_0x277;
    unsigned char field_0x278;
    unsigned char field_0x279;
    UnknownFollowCameraValue* field_0x27c; // owned values (allocated by 0x00463140, deleted by the destructor)
    UnknownFollowCameraValue* field_0x280;
    UnknownFollowCameraValue* field_0x284;
    UnknownFollowCameraValue* field_0x288;
    UnknownFollowCameraValue* field_0x28c;
    UnknownFollowCameraValue* field_0x290;
    UnknownFollowCameraValue* field_0x294;
    UnknownFollowCameraValue* field_0x298;
    Vector3 field_0x29c;        // g_UnknownVector65b438
    Vector3 field_0x2a8;        // cached slot 57 result (slot 69)
    Vector3 field_0x2b4;        // target point (slots 34, 37, 68)
    int field_0x2c0;
    float field_0x2c4;          // snapshot of field_0x220 (slot 71)
    float field_0x2c8;          // snapshot of field_0x22c
    float field_0x2cc;          // snapshot of field_0x234
    int field_0x2d0;
    float field_0x2d4;          // 7.0f
    int field_0x2d8;
    int field_0x2dc;
    int field_0x2e0;
    UnknownFollowCameraValue* field_0x2e4; // owned; deleted by the destructor
    int field_0x2e8;
    float field_0x2ec;          // 3.0f
    float field_0x2f0;          // state-4 saved field_0x258 (slot 71)
    float field_0x2f4;          // 10.0f
    float field_0x2f8;          // 280.0f
    float field_0x2fc;          // 70.0f
    float field_0x300;          // 20.0f
    int field_0x304;
    int field_0x308;
    int field_0x30c;            // cyclic index into field_0x314 (slot 72)
    int field_0x310;            // number of entries in field_0x314
    int field_0x314[11];        // state table (slot 72)
    int field_0x340;
};
