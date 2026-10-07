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
    virtual void UnknownVirtualSlot10(int mode, int textureAlpha);
    virtual long UnknownVirtualSlot11(int stage);
    virtual int UnknownVirtualSlot12(const CameraRect* rect, int flags); // 0x004c5510: near miss (samples/render)
    virtual int UnknownVirtualSlot13(int format);
    virtual int UnknownVirtualSlot14(void* viewport);
    virtual int UnknownVirtualSlot15(int type, int vertexFormat, int vertices, int vertexCount,
                                     int indices, int indexCount, int flags);
    virtual int UnknownVirtualSlot16(int type, int vertexFormat, int vertices, int count, int flags);
    virtual int UnknownVirtualSlot17(int type, int vertexBuffer, int start, int vertexCount,
                                     int indices, int indexCount, int flags);
    virtual void UnknownVirtualSlot18(int reference); // enables the alpha test
    virtual void UnknownVirtualSlot19();

    // 0x004c5d00 (PCCamera slot 27): saves the surface as the next free
    // "<computer name><5 digits>.TGA".
    void SaveScreenshot();
    // 0x004c4f80: attaches the display (+0x04), the device GUID (+0x54), the
    // surface (+0x48) and the frame modulus (+0x14); returns the target or 0.
    RenderTarget* InitializeRenderTarget(UnknownDisplay* display, const UnknownGuid* deviceId,
        UnknownSurfaceInterface* surface, int wantZBuffer, int frames);
    // 0x004c5950 (PCGame slot 31): measures texture memory by creating 256x256,
    // then 32x32 surfaces until creation fails; *value gets the bytes.
    int MeasureTextureMemory(int* value);
    // 0x004c5230 / 0x004c52a0: the device's texture-format and the Z-buffer
    // format enumeration callbacks; each appends the format to its list.
    static long __stdcall EnumTextureFormatCallback(UnknownPixelFormat* format, void* context);
    static long __stdcall EnumZBufferFormatCallback(UnknownPixelFormat* format, void* context);

    // The surface: Blt, Lock, GetSurfaceDesc and GetDC go to it (GameUi.cpp and
    // PCVideoCard.cpp use it under this provisional name).
    UnknownSurfaceInterface* renderSurface;    // +0x48
    UnknownSurfaceInterface* zbuffer;    // created with DDSCAPS_ZBUFFER and attached to the surface
    UnknownRenderInterface* device;      // created by the Direct3D object's method 4 (CreateDevice)
    UnknownGuid deviceGuid;             // Direct3D device GUID (PCGame +0x2f8); cleared by the constructor
    unsigned char field_0x64[0x164 - 0x64];
    // 0x164..0x250: the device description GetCaps fills (D3DDEVICEDESC7
    // layout: dwDevCaps, dpcLineCaps at +0x04, dpcTriCaps at +0x3c).
    // dwDevCaps: 0x4000 SEPARATETEXTUREMEMORIES; KrustyUI and PCGame read it.
    unsigned int deviceCaps;                   // +0x164
    unsigned char field_0x168[0x1a8 - 0x168];
    unsigned int triRasterCaps;          // dpcTriCaps.dwRasterCaps: dither, fog vertex/table/range, antialias
    unsigned char field_0x1ac[0x1b8 - 0x1ac];
    unsigned int triAlphaCmpCaps;        // dpcTriCaps.dwAlphaCmpCaps: GREATER, NOTEQUAL (slot 18)
    unsigned char field_0x1bc[0x1c0 - 0x1bc];
    unsigned int triTextureCaps;         // dpcTriCaps.dwTextureCaps (0x8 TRANSPARENCY before colour keying)
    unsigned int triTextureFilterCaps;   // dpcTriCaps.dwTextureFilterCaps (LINEAR, LINEARMIPLINEAR)
    unsigned char field_0x1c8[0x250 - 0x1c8];
    int fillMode;                        // D3DRENDERSTATE_FILLMODE value: 3 solid, 2 wireframe (PCGame key toggles)
    int textureFormatCount;
    UnknownPixelFormat* textureFormats;  // EnumTextureFormats results (DebugRealloc'd; freed by the destructor)
    int zbufferFormatCount;
    UnknownPixelFormat* zbufferFormats;  // EnumZBufferFormats results (DebugRealloc'd; freed by the destructor)
    UnknownRenderStateEntry renderStates[300]; // SetRenderState cache (slot 8)
};
