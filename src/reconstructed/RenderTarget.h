#pragma once

// RTTI: RenderTarget (root). Its vtable holds the destructor, _purecall in
// slots 1-17, and shared empty bodies in slots 18 and 19; PCRenderTarget
// implements it. This is the object Camera keeps at +0x18. The code sits just
// before ResourceManager.cpp references; RenderTarget.cpp is not attested.

#include "DisplayMode.h"

class Camera;

struct CameraRect { int left; int top; int right; int bottom; };

// Display-mode table reached through RenderTarget+0x04.
struct UnknownDisplay {
    unsigned char field_0x00[0x0c];
    int field_0x0c;                      // current mode index
    UnknownDisplayMode* field_0x10;
};

class RenderTarget {
public:
    RenderTarget();                      // 0x004e8c50
    virtual ~RenderTarget();             // 0x004e8cb0 (deleting wrapper 0x004e8c80)

    virtual int UnknownVirtualSlot1() = 0;
    virtual int UnknownVirtualSlot2() = 0;
    virtual int UnknownVirtualSlot3(void* a, void* b, void* c, int d) = 0;
    virtual int UnknownVirtualSlot4(int a, int b, int c) = 0;
    virtual int UnknownVirtualSlot5(void* rect) = 0;
    virtual long UnknownVirtualSlot6(int stage, int type, int* value) = 0;
    virtual long UnknownVirtualSlot7(int stage, int type, int value) = 0;
    virtual void UnknownVirtualSlot8(int state, int value, int force) = 0;
    virtual long UnknownVirtualSlot9(int state, int* value) = 0;
    virtual int UnknownVirtualSlot10(int a, int b) = 0;
    virtual long UnknownVirtualSlot11(int stage) = 0;
    virtual void UnknownVirtualSlot12(const CameraRect* rect, int flag) = 0;
    virtual int UnknownVirtualSlot13(int a) = 0;
    virtual int UnknownVirtualSlot14(void* viewport) = 0;
    virtual int UnknownVirtualSlot15(int a, int b, int c, int d, int e, int f, int g) = 0;
    virtual int UnknownVirtualSlot16(int a, int b, int c, int d, int e) = 0;
    virtual int UnknownVirtualSlot17(int a, int b, int c, int d, int e, int f, int g) = 0;
    virtual void UnknownVirtualSlot18(int value); // 0x004806f0 (shared empty body)
    virtual void UnknownVirtualSlot19();          // 0x0044d710 (shared empty body)

    void UnknownFunction4e8ca0(UnknownDisplay* display); // 0x004e8ca0: sets +0x04
    void UnknownFunction4e8cc0();                        // 0x004e8cc0: advances the frame counters
    void UnknownFunction4e8cf0(Camera* camera);          // 0x004e8cf0: sets the current camera

    UnknownDisplay* field_0x04;
    Camera* field_0x08;                  // current camera
    int field_0x0c;                      // width
    int field_0x10;                      // height
    int field_0x14;                      // frame modulus (Camera slots 5/18 read it)
    int field_0x18;                      // frame index, wraps at field_0x14
    int field_0x1c;                      // frame count
    int field_0x20;
    int field_0x24;
    int field_0x28;
    int field_0x2c;
    int field_0x30;
    int field_0x34;
    int field_0x38;
    int field_0x3c;
    int field_0x40;
    int field_0x44;
};
