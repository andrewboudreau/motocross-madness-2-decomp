#pragma once

// COM-style interfaces used through PCRenderTarget (`this` is passed on the
// stack, __stdcall). Method indices are decoded from retail call sites. The
// game imports DirectDrawCreateEx and carries IID_IDirectDraw7 and
// IID_IDirect3D7, and every decoded index and argument count lines up with
// the DirectX 7 SDK order of IDirect3DDevice7 and IDirectDrawSurface7
// (IUnknown's 0-2, then the methods in declaration order), so the methods
// carry those names (strong inference; the VC98 headers only go to DirectX
// 5). Slot indices are noted on each method.

struct Matrix4;
struct UnknownGuid;
struct UnknownClipperInterface;
struct UnknownSurfaceDesc;
struct UnknownSurfaceCaps;
struct UnknownPixelFormat;

// Device at PCRenderTarget+0x50 (IDirect3DDevice7, created by IDirect3D7's
// CreateDevice).
struct UnknownRenderInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall GetCaps(void* caps);                                   // 3
    virtual long __stdcall EnumTextureFormats(long(__stdcall* callback)(UnknownPixelFormat*, void*),
                                              void* context);                     // 4
    virtual long __stdcall BeginScene();                                          // 5
    virtual long __stdcall EndScene();                                            // 6
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall Clear(unsigned long count, void* rects, unsigned long flags,
                                 unsigned long color, float z, unsigned long stencil); // 10
    virtual long __stdcall SetTransform(int kind, const Matrix4* value);           // 11
    virtual long __stdcall UnknownMethod12();
    virtual long __stdcall SetViewport(void* viewport);                           // 13
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall UnknownMethod15();
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall UnknownMethod17();
    virtual long __stdcall UnknownMethod18();
    virtual long __stdcall UnknownMethod19();
    virtual long __stdcall SetRenderState(int state, int value);                  // 20
    virtual long __stdcall GetRenderState(int state, int* value);                 // 21
    virtual long __stdcall UnknownMethod22();
    virtual long __stdcall UnknownMethod23();
    virtual long __stdcall UnknownMethod24();
    virtual long __stdcall DrawPrimitive(int type, int vertexFormat, void* vertices, int count,
                                         int flags);                              // 25
    virtual long __stdcall DrawIndexedPrimitive(int type, int vertexFormat, void* vertices,
                                                int vertexCount, void* indices, int indexCount,
                                                int flags);                       // 26
    virtual long __stdcall UnknownMethod27();
    virtual long __stdcall UnknownMethod28();
    virtual long __stdcall UnknownMethod29();
    virtual long __stdcall UnknownMethod30();
    virtual long __stdcall UnknownMethod31();
    virtual long __stdcall DrawIndexedPrimitiveVB(int type, void* vertexBuffer, int start,
                                                  int vertexCount, void* indices, int indexCount,
                                                  int flags);                     // 32
    virtual long __stdcall UnknownMethod33();
    virtual long __stdcall UnknownMethod34();
    virtual long __stdcall SetTexture(int stage, void* texture);                  // 35
    virtual long __stdcall GetTextureStageState(int stage, int type, int* value);  // 36
    virtual long __stdcall SetTextureStageState(int stage, int type, int value);   // 37
    virtual long __stdcall UnknownMethod38();
    virtual long __stdcall UnknownMethod39();
    virtual long __stdcall UnknownMethod40();
    virtual long __stdcall UnknownMethod41();
    virtual long __stdcall UnknownMethod42();
    virtual long __stdcall Load(struct UnknownSurfaceInterface* destination, void* point,
                                struct UnknownSurfaceInterface* source, void* rect,
                                int flags);                                       // 43
};

// Surface at PCRenderTarget+0x48 (IDirectDrawSurface7).
struct UnknownSurfaceInterface {
    virtual long __stdcall QueryInterface(const UnknownGuid* iid, void** object);  // 0
    virtual long __stdcall AddRef();                                              // 1
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall AddAttachedSurface(UnknownSurfaceInterface* surface);   // 3
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall Blt(void* destination, UnknownSurfaceInterface* source,
                               void* sourceRect, int flags, void* effects);       // 5
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall BltFast(long x, long y, UnknownSurfaceInterface* source, void* rect,
                                   int flags);                                    // 7
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall Flip(UnknownSurfaceInterface* target, unsigned long flags); // 11
    virtual long __stdcall GetAttachedSurface(UnknownSurfaceCaps* caps,
                                              UnknownSurfaceInterface** surface); // 12
    virtual long __stdcall GetBltStatus(unsigned long flags);                     // 13
    virtual long __stdcall GetCaps(UnknownSurfaceCaps* caps);                     // 14
    virtual long __stdcall UnknownMethod15();
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall GetDC(void** dc);                                      // 17
    virtual long __stdcall UnknownMethod18();
    virtual long __stdcall UnknownMethod19();
    virtual long __stdcall UnknownMethod20();
    virtual long __stdcall GetPixelFormat(UnknownPixelFormat* format);            // 21
    virtual long __stdcall GetSurfaceDesc(UnknownSurfaceDesc* desc);              // 22
    virtual long __stdcall UnknownMethod23();
    virtual long __stdcall IsLost();                                              // 24
    virtual long __stdcall Lock(void* rect, UnknownSurfaceDesc* desc, int flags, void* event); // 25
    virtual long __stdcall ReleaseDC(void* dc);                                   // 26
    virtual long __stdcall Restore();                                             // 27
    virtual long __stdcall SetClipper(UnknownClipperInterface* clipper);          // 28
    virtual long __stdcall SetColorKey(int flags, void* key);                     // 29
    virtual long __stdcall UnknownMethod30();
    virtual long __stdcall SetPalette(void* palette);                             // 31
    virtual long __stdcall Unlock(void* rect);                                    // 32
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
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4(const UnknownGuid* device, UnknownSurfaceInterface* surface,
                                          UnknownRenderInterface** result); // CreateDevice
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6(const UnknownGuid* device,
                                          long(__stdcall* callback)(UnknownPixelFormat*, void*),
                                          void* context);                   // EnumZBufferFormats
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
