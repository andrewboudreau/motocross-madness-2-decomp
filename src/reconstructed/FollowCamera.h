#pragma once

#include <float.h>

#include "PCCamera.h"

// RTTI: FollowCamera : PCCamera. FollowCamera introduces primary slots 33-75;
// VehicleCamera, BikeCamera and KrustyBikeCamera inherit many of them. Slots
// 33, 35, 39, 41, 42, 50, 51, 57 and 74 are _purecall (0x00534cfe) in
// FollowCamera's vtable, so they are pure here. FollowCam.cpp spans
// 0x00462ee0..0x004670fb: 0x00463140, 0x004650e0 and 0x00465c20 pass its
// __FILE__, and its per-file vector set (0x00466f10.., .CRT$XCU 113-116) closes
// it; Fog code precedes it and FontTexture.cpp's initializer follows. The
// slot 75 body at 0x00404fc0 is an inline copy emitted elsewhere. Names are
// provisional.


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
    // Inlined by 0x00463140: scale 1, open [-FLT_MAX, FLT_MAX] limits.
    UnknownFollowCameraValue(float initialValue, float initialRate) {
        maximum = FLT_MAX;
        value = initialValue;
        rate = initialRate;
        minimum = -FLT_MAX;
        scale = 1.0f;
    }

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

// 44-byte record in the table at FollowCamera+0x2e4 (count at +0x2e0,
// capacity at +0x2dc), written by 0x00463450 and 0x00463520. 0x00463140
// resets both vectors to the zero vector; slot 38 compares +0x28 (the z of
// field_0x20). The Vector3 members give the record an empty constructor, which
// leaves a dead loop counter in 0x00463140's `new[]`.
struct UnknownFollowCameraPoint {
    Vector3 position;
    float field_0x0c;
    float field_0x10;
    float field_0x14;
    float field_0x18;
    float field_0x1c;
    Vector3 field_0x20;
};

// Object at FollowCamera+0x240; slots 43/44 test its +0xbe8.
struct UnknownFollowCameraSubject {
    // 0x00507c10: adjusts *point (KrustyBikeCamera slot 52 reads its new y).
    void UnknownFunction507c10(Vector3* point, int a, int b, int c);

    unsigned char field_0x000[0x40];
    float field_0x40;           // copied to FollowCamera+0x2ec by 0x00463600
    unsigned char field_0x044[0xbe8 - 0x44];
    int field_0xbe8;
};

// FollowCamera's view of ControlInterface's active joystick (+0x0c): slot 45
// tests only the low byte of slot 4's result (JoystickDevice.h declares int),
// as with UnknownKeyboardBoolView. Slot 4 reads stick `index` into *x/*y.
class UnknownJoystickBoolView {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual bool UnknownVirtualSlot4(int index, float* x, float* y);
};

// The object at 0x00575a98 (TrackOverlay.h's UnknownProjector views the same
// pointer). 0x0052f340 projects `point` with the camera and its +0xec matrix
// into `screen`; slot 38 switches on the int it writes through `code` when it
// returns 0. Names and the meaning of `code` are provisional.
class UnknownFollowCameraProjector {
public:
    int UnknownFunction52f340(Camera* camera, const void* matrix, const Vector3* point,
                              Vector3* screen, int* code);
};
extern UnknownFollowCameraProjector* g_UnknownFollowCameraProjector575a98;

// d3drm.dll's D3DRMVectorRotate (import thunk 0x0053304c, the module's only
// d3drm import): rotates `vector` about `axis` by `theta`. The SDK types are
// LPD3DVECTOR; Vector3 has the same layout.
extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

class FollowCamera : public PCCamera {
public:
    explicit FollowCamera(int flags); // 0x00462ee0 (near miss: samples/camera)
    virtual ~FollowCamera();          // destructor core 0x00463350

    virtual int UnknownVirtualSlot10(float frameTime); // 0x00465c20, not reconstructed
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00466ad0, not reconstructed

    virtual Vector3 UnknownVirtualSlot33() = 0;
    virtual Vector3 UnknownVirtualSlot34(int unused);
    virtual Vector3 UnknownVirtualSlot35(int a, int b) = 0;
    // 0x00465000 (near miss: samples/camera)
    virtual bool UnknownVirtualSlot36(const Vector3& point, bool enable, bool force);
    virtual Vector3 UnknownVirtualSlot37();
    // 0x004636e0
    virtual void UnknownVirtualSlot38(float dt, bool blend, bool pitch);
    virtual float UnknownVirtualSlot39() = 0;
    virtual void UnknownVirtualSlot40(float a);
    virtual void UnknownVirtualSlot41() = 0;
    virtual void UnknownVirtualSlot42(bool flag) = 0;
    virtual void UnknownVirtualSlot43(const Vector3& value);
    virtual void UnknownVirtualSlot44(const Vector3& value);
    // 0x00463a30: keyboard/joystick camera controls; returns whether it moved.
    virtual bool UnknownVirtualSlot45(bool active, float dt);
    // 0x004654e0: builds the +0x344 orientation from a forward and an up vector.
    virtual void UnknownVirtualSlot46(Vector3* forward, Vector3* up);
    // 0x00465720: plays back the loaded CAMERA records.
    virtual bool UnknownVirtualSlot47(float dt, const Vector3* offset);
    virtual Vector3 UnknownVirtualSlot48(int a, bool flag, int b);
    // 0x00464b30: the camera position for this frame.
    virtual Vector3 UnknownVirtualSlot49(bool orbit, const Vector3& base, float dt);
    virtual Vector3 UnknownVirtualSlot50() = 0;
    virtual float UnknownVirtualSlot51() = 0;
    virtual void UnknownVirtualSlot52(Vector3* point);
    virtual void UnknownVirtualSlot53();
    virtual void UnknownVirtualSlot54();
    virtual void UnknownVirtualSlot55();
    virtual bool UnknownVirtualSlot56();
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
    // The flag is a byte: slot 23 passes the low byte of the game's +0x1c4.
    virtual void UnknownVirtualSlot70(unsigned char value);
    virtual void UnknownVirtualSlot71(int value);
    virtual void UnknownVirtualSlot72();
    virtual void UnknownVirtualSlot73(const Vector3& from, const Vector3& to, float dt);
    virtual bool UnknownVirtualSlot74() = 0;
    // Inline: retail's copy sits at 0x00404fc0, far from FollowCam.cpp, and
    // VehicleCamera slot 75 inlines the same test.
    virtual int UnknownVirtualSlot75() { return cameraState == 5 || cameraState == 2; }

    // 0x00463450: writes entry `index` of the +0x2e4 table; each non-null
    // pointer supplies one field. Returns false when index >= +0x2e0.
    bool UnknownFunction463450(int index, const Vector3* position, const float* a,
                               const float* b, const float* c, const float* d,
                               const float* e);

    // 0x00463140 (VehicleCamera's constructor caller 0x0052ba12): Camera slot 8,
    // then the +0x294/+0x298 values (FollowCam.cpp lines 0x78/0x79), presets,
    // the point table (line 0x88) and the state list; states 5-7 and values
    // outside 0..7 are dropped. Releases itself and returns 0 when no state
    // is left.
    FollowCamera* UnknownFunction463140(void* value, float rate294, float rate298,
                                        float value228, float value2d0, float value2e8,
                                        int capacity, int count, const int* list);
    // 0x004650e0 (KrustyBikeCamera's constructor): loads the "CAMERA" records
    // of the text file `path` into frames/field_0x33c (FollowCam.cpp lines
    // 0x3c7, 0x3e1, 0x3e2).
    void UnknownFunction4650e0(const char* path);
    // 0x00463520: appends a table entry while below capacity.
    bool UnknownFunction463520(const Vector3& position, float a, float b, float c,
                               float d, float e);
    // 0x00463600: stores the subject and copies its +0x40.
    void UnknownFunction463600(UnknownFollowCameraSubject* subject);

protected:
    friend class VCRDlg;        // InGameProcs.cpp: ButCamera restores the saved state
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
    int cameraState;                 // +0x244, current state (slots 70-72, 75)
    int savedCameraState;            // +0x248, saved state (slot 70)
    float savedParameter;            // +0x24c, saved field_0x258 (slot 70)
    int field_0x250;
    float field_0x254;
    float field_0x258;          // clamped to [10, 70] by slot 68
    Vector3 field_0x25c;        // copy of Camera field_0x170
    int overrideActive;              // +0x268, slot 70 input byte
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
    Vector3 cachedTarget;             // +0x2a8, cached slot 57 result (slot 69)
    Vector3 targetPoint;              // +0x2b4, target point (slots 34, 37, 68)
    float field_0x2c0;          // height-like value: slot 45 needs it above 0 or 6.5 to descend
    float field_0x2c4;          // snapshot of field_0x220 (slot 71)
    float field_0x2c8;          // snapshot of field_0x22c
    float field_0x2cc;          // snapshot of field_0x234
    float field_0x2d0;          // set with field_0x2d4 by 0x00463140
    float field_0x2d4;          // 7.0f
    int field_0x2d8;
    int field_0x2dc;                 // capacity of points (0x00463140)
    int pointCount;                  // +0x2e0, number of entries in points
    UnknownFollowCameraPoint* points; // +0x2e4, owned table
    float field_0x2e8;          // yaw offset (0x00463140, slot 49)
    float field_0x2ec;          // 3.0f
    float field_0x2f0;          // state-4 saved field_0x258 (slot 71)
    float field_0x2f4;          // 10.0f
    float field_0x2f8;          // 280.0f
    float field_0x2fc;          // 70.0f
    float field_0x300;          // 20.0f
    int field_0x304;
    float field_0x308;          // set by KrustyBikeCamera slot 42
    int stateIndex;                  // +0x30c, cyclic index into states (slot 72)
    int stateCount;                  // +0x310, number of entries in states
    int states[8];                   // +0x314, state table (slot 72); only 0-4 are kept
    unsigned int frameCount;         // +0x334, CAMERA records loaded by 0x004650e0
    Matrix4* frames;                 // +0x338, one matrix per record (FollowCam.cpp line 0x3e1)
    float* field_0x33c;              // +0x33c, one value per record (line 0x3e2)
    float frameTime;                 // +0x340, playback clock (slot 47)
    Matrix4 field_0x344;             // orientation built by slot 46
};
