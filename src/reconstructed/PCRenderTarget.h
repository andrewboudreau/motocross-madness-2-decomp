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
    virtual void* UnknownVirtualSlot4(void* rect, long* pitch, int flags);
    virtual int UnknownVirtualSlot5(void* rect);
    virtual long UnknownVirtualSlot6(int stage, int type, int* value);
    virtual long UnknownVirtualSlot7(int stage, int type, int value);
    virtual void UnknownVirtualSlot8(int state, int value, int force);
    virtual long UnknownVirtualSlot9(int state, int* value);
    virtual void UnknownVirtualSlot10(int mode, int flag);
    virtual long UnknownVirtualSlot11(int stage);
    virtual int UnknownVirtualSlot12(const CameraRect* rect, int flags); // 0x004c5510: near miss (samples/render)
    virtual int UnknownVirtualSlot13(int format);
    virtual int UnknownVirtualSlot14(void* viewport);
    virtual int UnknownVirtualSlot15(int type, int vertexFormat, int vertices, int vertexCount,
                                     int indices, int indexCount, int flags);
    virtual int UnknownVirtualSlot16(int type, int vertexFormat, int vertices, int count, int flags);
    virtual int UnknownVirtualSlot17(int type, int vertexBuffer, int start, int vertexCount,
                                     int indices, int indexCount, int flags);
    virtual void UnknownVirtualSlot18(int value);
    virtual void UnknownVirtualSlot19();

    // 0x004c5d00 (PCCamera slot 27): saves the surface as the next free
    // "<computer name><5 digits>.TGA".
    void UnknownFunction4c5d00();
    // 0x004c4f80: attaches the display (+0x04), the device GUID (+0x54), the
    // surface (+0x48) and the frame modulus (+0x14); returns the target or 0.
    RenderTarget* UnknownFunction4c4f80(UnknownDisplay* display, const UnknownGuid* device,
                                        UnknownSurfaceInterface* surface, int zbuffer, int frames);
    // 0x004c5950 (PCGame slot 31): measures texture memory by creating 256x256,
    // then 32x32 surfaces until creation fails; *value gets the bytes.
    int UnknownFunction4c5950(int* value);
    // 0x004c5230 / 0x004c52a0: the device's texture-format and the Z-buffer
    // format enumeration callbacks; each appends the format to its list.
    static long __stdcall UnknownFunction4c5230(UnknownPixelFormat* format, void* context);
    static long __stdcall UnknownFunction4c52a0(UnknownPixelFormat* format, void* context);

    UnknownSurfaceInterface* field_0x48;
    UnknownSurfaceInterface* field_0x4c; // released by the destructor; PCGame slot 5 restores it
    UnknownRenderInterface* field_0x50; // the device
    UnknownGuid field_0x54;             // Direct3D device GUID (PCGame +0x2f8); cleared by the constructor
    unsigned char field_0x64[0x164 - 0x64];
    unsigned int field_0x164;            // capability bits (0x4000: 800x600, PCGame 0x004c0d10)
    unsigned char field_0x168[0x1a8 - 0x168];
    unsigned int field_0x1a8;            // capability bits (PCGame slot 7 tests 0x1, 0x800)
    unsigned char field_0x1ac[0x1b8 - 0x1ac];
    unsigned int field_0x1b8;            // capability bits (slot 18: 0x10, 0x20)
    unsigned char field_0x1bc[0x1c0 - 0x1bc];
    unsigned int field_0x1c0;            // capability bits (SoultreeMaterial tests 0x8 before colour keying)
    unsigned int field_0x1c4;            // texture filter capability bits (0x2, 0x20)
    unsigned char field_0x1c8[0x250 - 0x1c8];
    int field_0x250;                     // 3
    int field_0x254;                     // texture format count
    UnknownPixelFormat* field_0x258;     // texture formats (DebugRealloc'd; freed by the destructor)
    int field_0x25c;                     // Z-buffer format count
    UnknownPixelFormat* field_0x260;     // Z-buffer formats (DebugRealloc'd; freed by the destructor)
    UnknownRenderStateEntry field_0x264[300];
};
