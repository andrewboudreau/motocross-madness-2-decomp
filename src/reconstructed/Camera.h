#pragma once

#include "ControlInterface.h"
#include "GameObject.h"
#include "MatrixUtil.h"
#include "RenderTarget.h"

// RTTI: Camera : GameObject : BaseObject. Camera introduces primary slots 27-32
// and overrides (among others) slots 5, 13 and 18. Names are provisional; the
// member offsets are confirmed by the strict-exact constructor (0x0042e340)
// and methods, and the class ends at +0x220 where FollowCamera's fields begin.


// .rdata floats just before Camera's vtable, loaded by the constructor.
extern const float g_UnknownFloat550f6c; // 1.0f
extern const float g_UnknownFloat550f70; // 100000.0f

struct _iobuf; // FILE

class Camera : public GameObject {
public:
    explicit Camera(int flags); // 0x0042e340
    virtual ~Camera();

    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual GameObject* UnknownVirtualSlot8(void* value);
    virtual int UnknownVirtualSlot13();
    virtual int UnknownVirtualSlot14();
    virtual int UnknownVirtualSlot18();
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry);

    // Pure: Camera's vtable holds LIBCMT _purecall (0x00534cfe) in slot 27.
    virtual void UnknownVirtualSlot27() = 0;
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29(Vector3 value);
    virtual int UnknownVirtualSlot30(const Matrix4* value);
    virtual int UnknownVirtualSlot31(const Matrix4* value);
    virtual int UnknownVirtualSlot32(const Matrix4* value);

    // 0x0042e8e0, called by slot 13: re-applies the default limits through
    // 0x0042e960 and slot 28 when +0x1bc/+0x1c0 differ from them.
    int UnknownFunction42e8e0();
    // 0x0042e960: +0x1bc = max(minimum, g_UnknownFloat550f6c),
    // +0x1c0 = min(maximum, g_UnknownFloat550f70).
    void UnknownFunction42e960(float minimum, float maximum);
    int UnknownFunction42e550(); // 0x0042e550, called by slot 8

protected:
    // field_0x18 (GameObject's owner slot) holds the camera's RenderTarget.
    RenderTarget* Owner() const { return static_cast<RenderTarget*>(field_0x18); }

    friend class RenderTarget;
    friend class PCGame;           // slot 19 reattaches the camera (+0x18, +0x1a0)

    Matrix4 field_0x2c;      // set by slot 30 (PCCamera: kind 1)
    Matrix4 field_0x6c;      // set by slot 32 (kind 3)
    Matrix4 field_0xac;      // set by slot 31 (kind 2)
    unsigned char field_0xec[0x80]; // not touched by the constructor
    float field_0x16c;              // 77.0f
    Vector3 field_0x170;       // (0, 0, 0); copied to field_0x208
    Vector3 field_0x17c;       // (0, 0, 1); copied to field_0x214
    Vector3 field_0x188;       // (0, 1, 0)
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
    Vector3 field_0x208;
    Vector3 field_0x214;
};
