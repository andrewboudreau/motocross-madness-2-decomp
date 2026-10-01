#pragma once

#include "GameObject.h"

// RTTI: Camera : GameObject : BaseObject. Camera introduces primary slots 27-32
// and overrides (among others) slots 5, 13 and 18. Names are provisional; the
// member offsets are confirmed by the strict-exact constructor (0x0042e340)
// and methods, and the class ends at +0x220 where FollowCamera's fields begin.

// 4x4 float matrix (D3DMATRIX layout). The empty user-declared constructor
// matters: with it VC6 copies a by-value result straight from the returned
// pointer, as the Camera constructor does (see samples/physics/common/Math3D.h).
struct CameraMatrix16 {
    CameraMatrix16() {}
    float m[4][4];
};
struct CameraRect { int left; int top; int right; int bottom; };
// 12-byte float triple with D3DVECTOR-style constructors (D3D_OVERLOADS);
// passed by value to slot 29.
struct CameraFloat3 {
    CameraFloat3() {}
    CameraFloat3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    float x;
    float y;
    float z;
};

// cdecl 0x004a1410: returns the identity matrix. TU not attributed.
CameraMatrix16 UnknownFunction4a1410();

// .rdata floats just before Camera's vtable, loaded by the constructor.
extern const float g_UnknownFloat550f6c; // 1.0f
extern const float g_UnknownFloat550f70; // 100000.0f

struct _iobuf; // FILE

class Camera;
struct UnknownRenderInterface; // PCCamera.h

// Object referenced from Camera+0x18. Its +0x08 is compared with the camera,
// +0x14 is read by slots 5/18, +0x50 holds the render interface used by
// PCCamera, and virtual slot 12 receives a rectangle.
class UnknownCameraOwner {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12(const CameraRect* rect, int flag);
    void UnknownFunction4e8cf0(Camera* camera); // retail 0x004e8cf0

    void* field_0x04;
    Camera* field_0x08;                // compared with the camera by PCCamera slot 13
    int field_0x0c;
    int field_0x10;
    int field_0x14;                    // read by Camera slots 5 and 18
    unsigned char field_0x18[0x38];
    UnknownRenderInterface* field_0x50;
};

class Camera : public GameObject {
public:
    explicit Camera(int flags); // 0x0042e340
    virtual ~Camera();

    virtual void UnknownVirtualSlot5();
    virtual int UnknownVirtualSlot13();
    virtual int UnknownVirtualSlot18();

    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29(CameraFloat3 value);
    virtual int UnknownVirtualSlot30(const CameraMatrix16* value);
    virtual int UnknownVirtualSlot31(const CameraMatrix16* value);
    virtual int UnknownVirtualSlot32(const CameraMatrix16* value);

    void UnknownFunction42e8e0(); // retail 0x0042e8e0, called by slot 13

protected:
    UnknownCameraOwner* Owner() const { return static_cast<UnknownCameraOwner*>(field_0x18); }

    CameraMatrix16 field_0x2c;      // set by slot 30 (PCCamera: kind 1)
    CameraMatrix16 field_0x6c;      // set by slot 32 (kind 3)
    CameraMatrix16 field_0xac;      // set by slot 31 (kind 2)
    unsigned char field_0xec[0x80]; // not touched by the constructor
    float field_0x16c;              // 77.0f
    CameraFloat3 field_0x170;       // (0, 0, 0); copied to field_0x208
    CameraFloat3 field_0x17c;       // (0, 0, 1); copied to field_0x214
    CameraFloat3 field_0x188;       // (0, 1, 0)
    int field_0x194;
    int field_0x198;
    int field_0x19c;
    int field_0x1a0[6];             // [0..3]: x, y, width, height for slot 13
    int field_0x1b8;
    float field_0x1bc;              // g_UnknownFloat550f6c (1.0f)
    float field_0x1c0;              // g_UnknownFloat550f70 (100000.0f)
    int field_0x1c4;
    int field_0x1c8;
    int field_0x1cc;                // enables the slot 13 rectangle
    int field_0x1d0;                // owner+0x14 plus one (slots 5 and 18)
    int field_0x1d4;
    int field_0x1d8;
    float field_0x1dc;              // 109.0f
    float field_0x1e0;              // 10.0f
    _iobuf* field_0x1e4;            // FILE*, closed by the destructor
    int field_0x1e8;
    int field_0x1ec;
    int field_0x1f0;
    int field_0x1f4;
    int field_0x1f8;
    int field_0x1fc;
    int field_0x200;
    int field_0x204;
    CameraFloat3 field_0x208;
    CameraFloat3 field_0x214;
};
