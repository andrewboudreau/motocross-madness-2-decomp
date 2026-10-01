#pragma once

#include "Camera.h"
#include "UnknownObject56e26c.h"

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
