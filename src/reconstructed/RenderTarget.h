#pragma once

// RTTI: RenderTarget (root). Its vtable holds the destructor, _purecall in
// slots 1-17, and shared empty bodies in slots 18 and 19; PCRenderTarget
// implements it. This is the object Camera keeps at +0x18. The code sits just
// before ResourceManager.cpp references; RenderTarget.cpp is not attested.

#include "CameraRect.h"
#include "Display.h"

class Camera;

class RenderTarget {
public:
    RenderTarget();                      // 0x004e8c50
    virtual ~RenderTarget();             // 0x004e8cb0 (deleting wrapper 0x004e8c80)

    virtual int UnknownVirtualSlot1() = 0;
    virtual int UnknownVirtualSlot2() = 0;
    virtual int UnknownVirtualSlot3(void* a, void* b, void* c, int d) = 0;
    virtual void* UnknownVirtualSlot4(void* rect, long* pitch, int flags) = 0; // locks the surface
    virtual int UnknownVirtualSlot5(void* rect) = 0;
    virtual long UnknownVirtualSlot6(int stage, int type, int* value) = 0;
    virtual long UnknownVirtualSlot7(int stage, int type, int value) = 0;
    virtual void UnknownVirtualSlot8(int state, int value, int force) = 0;
    virtual long UnknownVirtualSlot9(int state, int* value) = 0;
    virtual void UnknownVirtualSlot10(int mode, int flag) = 0;
    virtual long UnknownVirtualSlot11(int stage) = 0;
    virtual int UnknownVirtualSlot12(const CameraRect* rect, int flags) = 0;
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
    int field_0x20;                      // Z depth (PCRenderTarget: 16 or 32)
    int field_0x24;                      // surface memory caps (0x4000 video, 0x800 system)
    int field_0x28;                      // pixel format of the surface
    float field_0x2c;                    // Z clear value (1.0f)
    int field_0x30;                      // clear colour
    int field_0x34;                      // nonzero: slot 12 also clears the target (D3DCLEAR_TARGET, 0x004c5549)
    int field_0x38;                      // vertices drawn in formats 0x112/0x1e2 (reset by slot 12)
    int field_0x3c;                      // points drawn
    int field_0x40;                      // lines drawn
    int field_0x44;                      // triangles drawn
};
