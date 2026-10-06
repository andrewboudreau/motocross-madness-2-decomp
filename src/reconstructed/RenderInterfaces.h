#pragma once

// COM-style interfaces used through PCRenderTarget (`this` is passed on the
// stack, __stdcall). Method indices are decoded from retail call sites. Their
// layout is consistent with IDirect3DDevice7 and IDirectDrawSurface7 (DirectX
// 7; the VC98 headers only go to DirectX 5), but that identity is inference,
// so the names stay neutral. Indices noted on the known methods.

struct Matrix4;
struct UnknownGuid;
struct UnknownClipperInterface;
struct UnknownSurfaceDesc;
struct UnknownSurfaceCaps;

// Device at PCRenderTarget+0x50 (IDirect3DDevice7-shaped).
struct UnknownRenderInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();                              // BeginScene
    virtual long __stdcall UnknownMethod6();                              // EndScene
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10(unsigned long count, void* rects, unsigned long flags,
                                           unsigned long color, int z, unsigned long stencil); // Clear
    virtual long __stdcall UnknownMethod11(int kind, const Matrix4* value); // SetTransform
    virtual long __stdcall UnknownMethod12();
    virtual long __stdcall UnknownMethod13(void* viewport);               // SetViewport
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall UnknownMethod15();
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall UnknownMethod17();
    virtual long __stdcall UnknownMethod18();
    virtual long __stdcall UnknownMethod19();
    virtual long __stdcall UnknownMethod20(int state, int value);         // SetRenderState
    virtual long __stdcall UnknownMethod21(int state, int* value);        // GetRenderState
    virtual long __stdcall UnknownMethod22();
    virtual long __stdcall UnknownMethod23();
    virtual long __stdcall UnknownMethod24();
    virtual long __stdcall UnknownMethod25();
    virtual long __stdcall UnknownMethod26();
    virtual long __stdcall UnknownMethod27();
    virtual long __stdcall UnknownMethod28();
    virtual long __stdcall UnknownMethod29();
    virtual long __stdcall UnknownMethod30();
    virtual long __stdcall UnknownMethod31();
    virtual long __stdcall UnknownMethod32();
    virtual long __stdcall UnknownMethod33();
    virtual long __stdcall UnknownMethod34();
    virtual long __stdcall UnknownMethod35(int stage, void* texture);     // SetTexture
    virtual long __stdcall UnknownMethod36(int stage, int type, int* value); // GetTextureStageState
    virtual long __stdcall UnknownMethod37(int stage, int type, int value);  // SetTextureStageState
    virtual long __stdcall UnknownMethod38();
    virtual long __stdcall UnknownMethod39();
    virtual long __stdcall UnknownMethod40();
    virtual long __stdcall UnknownMethod41();
    virtual long __stdcall UnknownMethod42();
    virtual long __stdcall UnknownMethod43(struct UnknownSurfaceInterface* destination, void* point,
                                           struct UnknownSurfaceInterface* source, void* rect,
                                           int flags);                       // Load
};

// Surface at PCRenderTarget+0x48 (IDirectDrawSurface7-shaped).
struct UnknownSurfaceInterface {
    virtual long __stdcall UnknownMethod0(const UnknownGuid* iid, void** object); // QueryInterface
    virtual long __stdcall UnknownMethod1();                              // AddRef
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5(void* destination, UnknownSurfaceInterface* source,
                                          void* sourceRect, int flags, void* effects); // Blt
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7(long x, long y, UnknownSurfaceInterface* source, void* rect,
                                          int flags);                       // BltFast
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11(UnknownSurfaceInterface* target, unsigned long flags); // Flip
    virtual long __stdcall UnknownMethod12(UnknownSurfaceCaps* caps, UnknownSurfaceInterface** surface); // GetAttachedSurface
    virtual long __stdcall UnknownMethod13(unsigned long flags);                 // GetBltStatus
    virtual long __stdcall UnknownMethod14(UnknownSurfaceCaps* caps);      // GetCaps
    virtual long __stdcall UnknownMethod15();
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall UnknownMethod17(void** dc);                     // GetDC
    virtual long __stdcall UnknownMethod18();
    virtual long __stdcall UnknownMethod19();
    virtual long __stdcall UnknownMethod20();
    virtual long __stdcall UnknownMethod21();
    virtual long __stdcall UnknownMethod22(UnknownSurfaceDesc* desc);      // GetSurfaceDesc
    virtual long __stdcall UnknownMethod23();
    virtual long __stdcall UnknownMethod24();                             // IsLost
    virtual long __stdcall UnknownMethod25(void* rect, UnknownSurfaceDesc* desc, int flags, void* event); // Lock
    virtual long __stdcall UnknownMethod26(void* dc);                      // ReleaseDC
    virtual long __stdcall UnknownMethod27();                             // Restore
    virtual long __stdcall UnknownMethod28(UnknownClipperInterface* clipper); // SetClipper
    virtual long __stdcall UnknownMethod29(int flags, void* key);         // SetColorKey
    virtual long __stdcall UnknownMethod30();
    virtual long __stdcall UnknownMethod31(void* palette);                 // SetPalette
    virtual long __stdcall UnknownMethod32(void* rect);                   // Unlock
};

// 16-byte surface capabilities (the DDSCAPS2 layout).
struct UnknownSurfaceCaps {
    unsigned long caps;
    unsigned long caps2;
    unsigned long caps3;
    unsigned long caps4;
};

// 0x20-byte pixel format (the DDPIXELFORMAT layout).
struct UnknownPixelFormat {
    unsigned long size;
    unsigned long flags;
    unsigned long fourCC;
    unsigned long bitCount;
    unsigned long masks[4];
};

// 32-bit RGBA (format 0x22b8) and 24-bit RGB (0x378) pixels, red first in
// memory (Pixtrans.cpp takes them by value; Tgafile.cpp swaps red and blue).
struct UnknownPixel32 {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
};

struct UnknownPixel24 {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

// 0x7c-byte surface description (the DDSURFACEDESC2 layout).
struct UnknownSurfaceDesc {
    unsigned long size;
    unsigned long flags;
    unsigned long height;
    unsigned long width;
    long pitch;
    union {
        unsigned long field_0x14;
        unsigned long backBufferCount;
    };
    union {
        unsigned long mipMapCount;
        unsigned long refreshRate;
    };
    unsigned char field_0x1c[0x24 - 0x1c];
    void* surface;                                // the locked bits
    unsigned char field_0x28[0x48 - 0x28];
    UnknownPixelFormat pixelFormat;
    unsigned long caps[4];
    unsigned long textureStage;
};

// A palette Display+0x190's slot 5 creates (IDirectDrawPalette-shaped;
// Palette8 releases it through slot 2).
struct UnknownPaletteInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();                              // Release
};

// Interface at Display+0x190 (IDirectDraw7-shaped).
struct UnknownDirectDrawInterface {
    virtual long __stdcall UnknownMethod0(const UnknownGuid* iid, void** object);  // QueryInterface
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4(unsigned long flags, UnknownClipperInterface** clipper,
                                          void* outer);                          // CreateClipper
    virtual long __stdcall UnknownMethod5(unsigned long flags, void* entries, UnknownPaletteInterface** palette,
                                          void* outer);                    // CreatePalette (Palette8.cpp)
    virtual long __stdcall UnknownMethod6(UnknownSurfaceDesc* desc, UnknownSurfaceInterface** surface,
                                          void* outer);                    // CreateSurface
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8(unsigned long flags, UnknownSurfaceDesc* desc, void* context,
                                          long(__stdcall* callback)(UnknownSurfaceDesc*, void*)); // EnumDisplayModes
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11(void* driverCaps, void* emulationCaps);    // GetCaps
    virtual long __stdcall UnknownMethod12(UnknownSurfaceDesc* desc);                 // GetDisplayMode
    virtual long __stdcall UnknownMethod13();
    virtual long __stdcall UnknownMethod14(UnknownSurfaceInterface** surface);        // GetGDISurface
    virtual long __stdcall UnknownMethod15();
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall UnknownMethod17();
    virtual long __stdcall UnknownMethod18();
    virtual long __stdcall UnknownMethod19();
    virtual long __stdcall UnknownMethod20(void* window, unsigned long flags);        // SetCooperativeLevel
    virtual long __stdcall UnknownMethod21(unsigned long width, unsigned long height,
                                           unsigned long bitDepth, unsigned long refreshRate,
                                           unsigned long flags);              // SetDisplayMode
    virtual long __stdcall UnknownMethod22(unsigned long flags, void* event);         // WaitForVerticalBlank
    virtual long __stdcall UnknownMethod23(UnknownSurfaceCaps* caps, unsigned long* total,
                                           unsigned long* free);            // GetAvailableVidMem
    virtual long __stdcall UnknownMethod24();
    virtual long __stdcall UnknownMethod25();
    virtual long __stdcall UnknownMethod26();
    virtual long __stdcall UnknownMethod27(void* identifier, unsigned long flags); // GetDeviceIdentifier
};

// Interface at Display+0x194 (IDirect3D7, from QueryInterface).
struct UnknownDirect3DInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();                              // Release
};

// Clipper at Display+0x1ac (IDirectDrawClipper-shaped).
struct UnknownClipperInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();                              // Release
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8(unsigned long flags, void* window); // SetHWnd
};

// Gamma control at Display+0x1a4 (IDirectDrawGammaControl-shaped, queried
// from the primary surface).
struct UnknownGammaControlInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();                              // Release
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4(unsigned long flags, void* ramp);  // SetGammaRamp
};
