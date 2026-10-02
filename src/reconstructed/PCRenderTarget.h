#pragma once

#include "Guid.h"
#include "RenderInterfaces.h"
#include "RenderTarget.h"

// Object passed as the Blt source to PCRenderTarget slot 3; its surface is at
// +0x70.
struct UnknownBlitSource {
    unsigned char field_0x00[0x70];
    UnknownSurfaceInterface* field_0x70;
};

// One cached render state (PCRenderTarget+0x264, 300 entries).
struct UnknownRenderStateEntry {
    int state;
    int value;
};

// RTTI: PCRenderTarget : RenderTarget. TU PCRenderTarget.cpp (literal
// __FILE__ in its destructor). It wraps a DirectX 7-style device (+0x50) and
// surface (+0x48); most slots forward to them and report success as
// `result == 0`. Names are provisional.
class PCRenderTarget : public RenderTarget {
public:
    PCRenderTarget();                    // 0x004c4ee0
    virtual ~PCRenderTarget();           // 0x004c5320 (deleting wrapper 0x004c4f60)

    virtual int UnknownVirtualSlot1();
    virtual int UnknownVirtualSlot2();
    virtual int UnknownVirtualSlot3(void* destination, void* source, void* sourceRect, int flags);
    virtual int UnknownVirtualSlot4(int a, int b, int c);              // 0x004c5420, not reconstructed
    virtual int UnknownVirtualSlot5(void* rect);
    virtual long UnknownVirtualSlot6(int stage, int type, int* value);
    virtual long UnknownVirtualSlot7(int stage, int type, int value);
    virtual void UnknownVirtualSlot8(int state, int value, int force);
    virtual long UnknownVirtualSlot9(int state, int* value);
    virtual int UnknownVirtualSlot10(int a, int b);                    // 0x004c5740, not reconstructed
    virtual long UnknownVirtualSlot11(int stage);
    virtual void UnknownVirtualSlot12(const CameraRect* rect, int flag); // 0x004c5510, not reconstructed
    virtual int UnknownVirtualSlot13(int a);                           // 0x004c5640, not reconstructed
    virtual int UnknownVirtualSlot14(void* viewport);
    virtual int UnknownVirtualSlot15(int a, int b, int c, int d, int e, int f, int g); // 0x004c5b20
    virtual int UnknownVirtualSlot16(int a, int b, int c, int d, int e);               // 0x004c5bd0
    virtual int UnknownVirtualSlot17(int a, int b, int c, int d, int e, int f, int g); // 0x004c5c70
    virtual void UnknownVirtualSlot18(int value);                      // 0x004c5e60, not reconstructed
    virtual void UnknownVirtualSlot19();

    void UnknownFunction4c5d00();        // 0x004c5d00 (PCCamera slot 27)
    // 0x004c4f80: attaches the display (+0x04), the device GUID (+0x54), the
    // surface (+0x48) and the frame modulus (+0x14); returns the target or 0.
    RenderTarget* UnknownFunction4c4f80(UnknownDisplay* display, const UnknownGuid* device,
                                        UnknownSurfaceInterface* surface, int flag, int frames);
    void UnknownFunction4c5950(int* value);   // 0x004c5950 (PCGame slot 31)

    UnknownSurfaceInterface* field_0x48;
    UnknownSurfaceInterface* field_0x4c; // released by the destructor; PCGame slot 5 restores it
    UnknownRenderInterface* field_0x50; // the device
    UnknownGuid field_0x54;             // Direct3D device GUID (PCGame +0x2f8); cleared by the constructor
    unsigned char field_0x64[0x1a8 - 0x64];
    unsigned int field_0x1a8;            // capability bits (PCGame slot 7 tests 0x1, 0x800)
    unsigned char field_0x1ac[0x1c4 - 0x1ac];
    unsigned int field_0x1c4;            // texture filter capability bits (0x2, 0x20)
    unsigned char field_0x1c8[0x250 - 0x1c8];
    int field_0x250;                     // 3
    int field_0x254;
    void* field_0x258;                   // DebugMalloc'd; freed by the destructor
    int field_0x25c;
    void* field_0x260;                   // DebugMalloc'd; freed by the destructor
    UnknownRenderStateEntry field_0x264[300];
};
