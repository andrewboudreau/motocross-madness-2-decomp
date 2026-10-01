#pragma once

#include "Camera.h"

// RTTI: PCCamera : Camera. PCCamera introduces no primary slots; it overrides
// (among others) slots 13, 27 (pure in Camera) and 30-32.
//
// The overrides forward Camera's 64-byte blocks to method 11 of the COM-style
// interface at owner(+0x18)->+0x50 with kind 1, 2 or 3. That call shape is
// consistent with IDirect3DDevice7::SetTransform(world/view/projection), but
// the interface identity is inference, so neutral names are kept.

// COM-style interface: `this` is passed on the stack (__stdcall virtuals).
struct UnknownRenderInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11(int kind, const Matrix4* value);
};

// Interface reached as (0x0056e26c object)->+0x14->+0x34; FollowCamera
// slots 55 and 56 call its slot 5 with three small constants.
class UnknownInterface56e26c {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5(int a, int b, int c);
};

struct UnknownObject56e26cPart {
    unsigned char field_0x00[0x34];
    UnknownInterface56e26c* field_0x34;
};

// Object behind the global pointer at 0x0056e26c; PCCamera slot 27 calls its
// non-virtual 0x00468880.
class UnknownObject56e26c {
public:
    void UnknownFunction468880();

    unsigned char field_0x00[0x14];
    UnknownObject56e26cPart* field_0x14;
};
extern UnknownObject56e26c* g_UnknownGlobal56e26c;

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
};
