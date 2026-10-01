#pragma once

#include <float.h>

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

// Inline minimum; slot 73 and UnknownFollowCameraValue::Update need the
// function form (a ternary reloads the chosen operand).
inline float FollowCameraMin(float a, float b) {
    return (a < b) ? a : b;
}

// Inline absolute value (fcom 0 / fchs, not the fabs intrinsic).
inline float FollowCameraAbs(float value) {
    if (value < 0.0f)
        value = -value;
    return value;
}

// 20-byte object allocated by 0x00463140 (FollowCam.cpp lines 0x78 onward).
// Set() is inlined by slots 43 and 44: the value always changes, the rate (and
// a 1.0 scale) only when one is given.
struct UnknownFollowCameraValue {
    void Set(float newValue, float newRate) {
        value = newValue;
        if (newRate != FLT_MAX) {
            rate = newRate;
            scale = 1.0f;
        }
    }

    // Inlined by slot 73: eases value toward target by min(dt, rate) / rate.
    float Update(float target, float dt) {
        scale = FollowCameraMin(dt, rate) / rate;
        value = (target - value) * scale + value;
        return value;
    }

    float value;
    float rate;
    float scale;    // 1.0f
    float maximum;  // FLT_MAX
    float minimum;  // -FLT_MAX
};

// 44-byte record in the table at FollowCamera+0x2e4 (count at +0x2e0), written
// by 0x00463450. The five 4-byte values' types are not established.
struct UnknownFollowCameraPoint {
    Vector3 position;
    float field_0x0c;
    float field_0x10;
    float field_0x14;
    float field_0x18;
    float field_0x1c;
    int field_0x20;
    int field_0x24;
    int field_0x28;
};

// Object at FollowCamera+0x240; slots 43/44 test its +0xbe8.
struct UnknownFollowCameraSubject {
    unsigned char field_0x000[0xbe8];
    int field_0xbe8;
};

// .bss vectors copied by the constructor.
extern Vector3 g_UnknownVector65b438;
extern Vector3 g_UnknownVector65b448;

class FollowCamera : public PCCamera {
public:
    explicit FollowCamera(int flags); // 0x00462ee0 (near miss: samples/camera)
    virtual ~FollowCamera();          // destructor core 0x00463350

    virtual void UnknownVirtualSlot33() = 0;
    virtual Vector3 UnknownVirtualSlot34(int unused);
    virtual Vector3 UnknownVirtualSlot35(int a, int b) = 0;
    // 0x00465000 (near miss: samples/camera)
    virtual bool UnknownVirtualSlot36(const Vector3& point, bool enable, bool force);
    virtual Vector3 UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot38(int a, int b, int c);
    virtual float UnknownVirtualSlot39() = 0;
    virtual void UnknownVirtualSlot40(int a);
    virtual void UnknownVirtualSlot41() = 0;
    virtual void UnknownVirtualSlot42() = 0;
    virtual void UnknownVirtualSlot43(const Vector3& value);
    virtual void UnknownVirtualSlot44(const Vector3& value);
    virtual void UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot46();
    virtual void UnknownVirtualSlot47();
    virtual Vector3 UnknownVirtualSlot48(int a, bool flag, int b);
    virtual void UnknownVirtualSlot49();
    virtual Vector3 UnknownVirtualSlot50() = 0;
    virtual float UnknownVirtualSlot51() = 0;
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
    virtual void UnknownVirtualSlot73(const Vector3& from, const Vector3& to, float dt);
    virtual bool UnknownVirtualSlot74() = 0;
    // Inline: retail's copy sits at 0x00404fc0, far from FollowCam.cpp, and
    // VehicleCamera slot 75 inlines the same test.
    virtual int UnknownVirtualSlot75() { return field_0x244 == 5 || field_0x244 == 2; }

    // 0x00463450: writes entry `index` of the +0x2e4 table; each non-null
    // pointer supplies one field. Returns false when index >= +0x2e0.
    bool UnknownFunction463450(int index, const Vector3* position, const float* a,
                               const float* b, const float* c, const float* d,
                               const float* e);

protected:
    // Layout from the constructor (0x00462ee0) and destructor (0x00463350).
    float field_0x220;          // preset distance-like value (slots 63-67)
    float field_0x224;          // copy of field_0x220 (constructor)
    float field_0x228;          // 27.0f
    float field_0x22c;          // preset angle-like value (slots 63-67)
    float field_0x230;          // copy of field_0x22c
    float field_0x234;          // preset angle-like value (slots 63-67)
    float field_0x238;          // copy of field_0x234
    float field_0x23c;          // advanced by pi in VehicleCamera slot 39
    UnknownFollowCameraSubject* field_0x240;
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
    bool field_0x277;           // set when slot 36 ran with enable
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
    int field_0x2e0;            // number of entries in field_0x2e4
    UnknownFollowCameraPoint* field_0x2e4; // owned table; deleted by the destructor
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
