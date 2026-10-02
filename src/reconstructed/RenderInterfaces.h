#pragma once

// COM-style interfaces used through PCRenderTarget (`this` is passed on the
// stack, __stdcall). Method indices are decoded from retail call sites. Their
// layout is consistent with IDirect3DDevice7 and IDirectDrawSurface7 (DirectX
// 7; the VC98 headers only go to DirectX 5), but that identity is inference,
// so the names stay neutral. Indices noted on the known methods.

struct Matrix4;
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
    virtual long __stdcall UnknownMethod10();
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
};

// Surface at PCRenderTarget+0x48 (IDirectDrawSurface7-shaped).
struct UnknownSurfaceInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5(void* destination, UnknownSurfaceInterface* source,
                                          void* sourceRect, int flags, void* effects); // Blt
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11();
    virtual long __stdcall UnknownMethod12(UnknownSurfaceCaps* caps, UnknownSurfaceInterface** surface); // GetAttachedSurface
    virtual long __stdcall UnknownMethod13();
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall UnknownMethod15();
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall UnknownMethod17();
    virtual long __stdcall UnknownMethod18();
    virtual long __stdcall UnknownMethod19();
    virtual long __stdcall UnknownMethod20();
    virtual long __stdcall UnknownMethod21();
    virtual long __stdcall UnknownMethod22();
    virtual long __stdcall UnknownMethod23();
    virtual long __stdcall UnknownMethod24();                             // IsLost
    virtual long __stdcall UnknownMethod25(void* rect, UnknownSurfaceDesc* desc, int flags, void* event); // Lock
    virtual long __stdcall UnknownMethod26();
    virtual long __stdcall UnknownMethod27();                             // Restore
    virtual long __stdcall UnknownMethod28();
    virtual long __stdcall UnknownMethod29();
    virtual long __stdcall UnknownMethod30();
    virtual long __stdcall UnknownMethod31();
    virtual long __stdcall UnknownMethod32(void* rect);                   // Unlock
};

// 16-byte surface capabilities (the DDSCAPS2 layout).
struct UnknownSurfaceCaps {
    unsigned long caps;
    unsigned long caps2;
    unsigned long caps3;
    unsigned long caps4;
};

// 0x7c-byte surface description (the DDSURFACEDESC2 layout).
struct UnknownSurfaceDesc {
    unsigned long size;
    unsigned long flags;
    unsigned long height;
    unsigned long width;
    long pitch;
    unsigned char field_0x14[0x24 - 0x14];
    void* surface;                                // the locked bits
    unsigned char field_0x28[0x48 - 0x28];
    unsigned char pixelFormat[0x20];
    unsigned long caps[4];
    unsigned long textureStage;
};

// Interface at Display+0x190 (IDirectDraw7-shaped).
struct UnknownDirectDrawInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6(UnknownSurfaceDesc* desc, UnknownSurfaceInterface** surface,
                                          void* outer);                    // CreateSurface
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11();
    virtual long __stdcall UnknownMethod12();
    virtual long __stdcall UnknownMethod13();
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall UnknownMethod15();
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall UnknownMethod17();
    virtual long __stdcall UnknownMethod18();
    virtual long __stdcall UnknownMethod19();
    virtual long __stdcall UnknownMethod20();
    virtual long __stdcall UnknownMethod21(unsigned long width, unsigned long height,
                                           unsigned long bitDepth, unsigned long refreshRate,
                                           unsigned long flags);              // SetDisplayMode
    virtual long __stdcall UnknownMethod22();
    virtual long __stdcall UnknownMethod23(UnknownSurfaceCaps* caps, unsigned long* total,
                                           unsigned long* free);            // GetAvailableVidMem
};
