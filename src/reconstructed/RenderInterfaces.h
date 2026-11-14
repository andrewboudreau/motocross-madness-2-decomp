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
struct SurfaceMaterial;
struct Guid;
struct DirectDrawClipper;
struct DDSurfaceDesc2;
struct DDSCaps2;
struct DDPixelFormat;

// Device at PCRenderTarget+0x50 (IDirect3DDevice7, created by IDirect3D7's
// CreateDevice).
struct Direct3DDevice {
    virtual long __stdcall QueryInterface();
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();                                           // 2
    virtual long __stdcall GetCaps(void* caps);                                   // 3
    virtual long __stdcall EnumTextureFormats(long(__stdcall* callback)(DDPixelFormat*, void*),
                                              void* context);                     // 4
    virtual long __stdcall BeginScene();                                          // 5
    virtual long __stdcall EndScene();                                            // 6
    virtual long __stdcall GetDirect3D();
    virtual long __stdcall SetRenderTarget();
    virtual long __stdcall GetRenderTarget();
    virtual long __stdcall Clear(unsigned long count, void* rects, unsigned long flags,
                                 unsigned long color, float z, unsigned long stencil); // 10
    virtual long __stdcall SetTransform(int kind, const Matrix4* value);           // 11
    virtual long __stdcall GetTransform();
    virtual long __stdcall SetViewport(void* viewport);                           // 13
    virtual long __stdcall MultiplyTransform();
    virtual long __stdcall GetViewport();
    virtual long __stdcall SetMaterial(const SurfaceMaterial* material);          // 16
    virtual long __stdcall GetMaterial();
    virtual long __stdcall SetLight();
    virtual long __stdcall GetLight();
    virtual long __stdcall SetRenderState(int state, int value);                  // 20
    virtual long __stdcall GetRenderState(int state, int* value);                 // 21
    virtual long __stdcall BeginStateBlock();
    virtual long __stdcall EndStateBlock();
    virtual long __stdcall PreLoad();
    virtual long __stdcall DrawPrimitive(int type, int vertexFormat, void* vertices, int count,
                                         int flags);                              // 25
    virtual long __stdcall DrawIndexedPrimitive(int type, int vertexFormat, void* vertices,
                                                int vertexCount, void* indices, int indexCount,
                                                int flags);                       // 26
    virtual long __stdcall SetClipStatus();
    virtual long __stdcall GetClipStatus();
    virtual long __stdcall DrawPrimitiveStrided();
    virtual long __stdcall DrawIndexedPrimitiveStrided();
    virtual long __stdcall DrawPrimitiveVB();
    virtual long __stdcall DrawIndexedPrimitiveVB(int type, void* vertexBuffer, int start,
                                                  int vertexCount, void* indices, int indexCount,
                                                  int flags);                     // 32
    virtual long __stdcall ComputeSphereVisibility();
    virtual long __stdcall GetTexture();
    virtual long __stdcall SetTexture(int stage, void* texture);                  // 35
    virtual long __stdcall GetTextureStageState(int stage, int type, int* value);  // 36
    virtual long __stdcall SetTextureStageState(int stage, int type, int value);   // 37
    virtual long __stdcall ValidateDevice();
    virtual long __stdcall ApplyStateBlock();
    virtual long __stdcall CaptureStateBlock();
    virtual long __stdcall DeleteStateBlock();
    virtual long __stdcall CreateStateBlock();
    virtual long __stdcall Load(struct DirectDrawSurface* destination, void* point,
                                struct DirectDrawSurface* source, void* rect,
                                int flags);                                       // 43
};

// Surface at PCRenderTarget+0x48 (IDirectDrawSurface7).
struct DirectDrawSurface {
    virtual long __stdcall QueryInterface(const Guid* iid, void** object);  // 0
    virtual long __stdcall AddRef();                                              // 1
    virtual long __stdcall Release();                                           // 2
    virtual long __stdcall AddAttachedSurface(DirectDrawSurface* surface);   // 3
    virtual long __stdcall AddOverlayDirtyRect();
    virtual long __stdcall Blt(void* destination, DirectDrawSurface* source,
                               void* sourceRect, int flags, void* effects);       // 5
    virtual long __stdcall BltBatch();
    virtual long __stdcall BltFast(long x, long y, DirectDrawSurface* source, void* rect,
                                   int flags);                                    // 7
    virtual long __stdcall DeleteAttachedSurface();
    virtual long __stdcall EnumAttachedSurfaces();
    virtual long __stdcall EnumOverlayZOrders();
    virtual long __stdcall Flip(DirectDrawSurface* target, unsigned long flags); // 11
    virtual long __stdcall GetAttachedSurface(DDSCaps2* caps,
                                              DirectDrawSurface** surface); // 12
    virtual long __stdcall GetBltStatus(unsigned long flags);                     // 13
    virtual long __stdcall GetCaps(DDSCaps2* caps);                     // 14
    virtual long __stdcall GetClipper();
    virtual long __stdcall GetColorKey();
    virtual long __stdcall GetDC(void** dc);                                      // 17
    virtual long __stdcall GetFlipStatus();
    virtual long __stdcall GetOverlayPosition();
    virtual long __stdcall GetPalette();
    virtual long __stdcall GetPixelFormat(DDPixelFormat* format);            // 21
    virtual long __stdcall GetSurfaceDesc(DDSurfaceDesc2* desc);              // 22
    virtual long __stdcall Initialize();
    virtual long __stdcall IsLost();                                              // 24
    virtual long __stdcall Lock(void* rect, DDSurfaceDesc2* desc, int flags, void* event); // 25
    virtual long __stdcall ReleaseDC(void* dc);                                   // 26
    virtual long __stdcall Restore();                                             // 27
    virtual long __stdcall SetClipper(DirectDrawClipper* clipper);          // 28
    virtual long __stdcall SetColorKey(int flags, void* key);                     // 29
    virtual long __stdcall SetOverlayPosition();
    virtual long __stdcall SetPalette(void* palette);                             // 31
    virtual long __stdcall Unlock(void* rect);                                    // 32
};

// 16-byte surface capabilities (the DDSCAPS2 layout).
struct DDSCaps2 {
    unsigned long caps;
    unsigned long caps2;
    unsigned long caps3;
    unsigned long caps4;
};

// 0x20-byte pixel format (the DDPIXELFORMAT layout).
struct DDPixelFormat {
    unsigned long size;
    unsigned long flags;
    unsigned long fourCC;
    unsigned long bitCount;
    unsigned long masks[4];
};

// 32-bit RGBA (format 0x22b8) and 24-bit RGB (0x378) pixels, red first in
// memory (Pixtrans.cpp takes them by value; Tgafile.cpp swaps red and blue).
struct RgbaPixel {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
};

struct RgbPixel {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

// 0x7c-byte surface description (the DDSURFACEDESC2 layout).
struct DDSurfaceDesc2 {
    unsigned long size;
    unsigned long flags;
    unsigned long height;
    unsigned long width;
    long pitch;
    union {
        unsigned long depth;
        unsigned long backBufferCount;
    };
    union {
        unsigned long mipMapCount;
        unsigned long refreshRate;
    };
    unsigned char field_0x1c[0x24 - 0x1c];
    void* surface;                                // the locked bits
    unsigned char field_0x28[0x48 - 0x28];
    DDPixelFormat pixelFormat;
    unsigned long caps[4];
    unsigned long textureStage;
};

// A palette Display+0x190's slot 5 creates (IDirectDrawPalette-shaped;
// Palette8 releases it through slot 2).
struct DirectDrawPalette {
    virtual long __stdcall QueryInterface();
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();                                           // 2
};

// Interface at Display+0x190 (IDirectDraw7-shaped).
struct DirectDraw {
    virtual long __stdcall QueryInterface(const Guid* iid, void** object); // 0
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();                                           // 2
    virtual long __stdcall Compact();
    virtual long __stdcall CreateClipper(unsigned long flags, DirectDrawClipper** clipper,
                                          void* outer);                         // 4
    virtual long __stdcall CreatePalette(unsigned long flags, void* entries, DirectDrawPalette** palette,
                                          void* outer);                         // 5
    virtual long __stdcall CreateSurface(DDSurfaceDesc2* desc, DirectDrawSurface** surface,
                                          void* outer);                         // 6
    virtual long __stdcall DuplicateSurface();
    virtual long __stdcall EnumDisplayModes(unsigned long flags, DDSurfaceDesc2* desc, void* context,
                                          long(__stdcall* callback)(DDSurfaceDesc2*, void*)); // 8
    virtual long __stdcall EnumSurfaces();
    virtual long __stdcall FlipToGDISurface();
    virtual long __stdcall GetCaps(void* driverCaps, void* emulationCaps);      // 11
    virtual long __stdcall GetDisplayMode(DDSurfaceDesc2* desc);            // 12
    virtual long __stdcall GetFourCCCodes();
    virtual long __stdcall GetGDISurface(DirectDrawSurface** surface);    // 14
    virtual long __stdcall GetMonitorFrequency();
    virtual long __stdcall GetScanLine();
    virtual long __stdcall GetVerticalBlankStatus();
    virtual long __stdcall Initialize();
    virtual long __stdcall RestoreDisplayMode();
    virtual long __stdcall SetCooperativeLevel(void* window, unsigned long flags); // 20
    virtual long __stdcall SetDisplayMode(unsigned long width, unsigned long height,
                                           unsigned long bitDepth, unsigned long refreshRate,
                                           unsigned long flags);                // 21
    virtual long __stdcall WaitForVerticalBlank(unsigned long flags, void* event); // 22
    virtual long __stdcall GetAvailableVidMem(DDSCaps2* caps, unsigned long* total,
                                           unsigned long* free);                // 23
    virtual long __stdcall GetSurfaceFromDC();
    virtual long __stdcall RestoreAllSurfaces();
    virtual long __stdcall TestCooperativeLevel();
    virtual long __stdcall GetDeviceIdentifier(void* identifier, unsigned long flags); // 27
};

// Interface at Display+0x194 (IDirect3D7, from QueryInterface).
struct Direct3D {
    virtual long __stdcall QueryInterface();
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();                                           // 2
    virtual long __stdcall EnumDevices();
    virtual long __stdcall CreateDevice(const Guid* device, DirectDrawSurface* surface,
                                          Direct3DDevice** result);     // 4
    virtual long __stdcall CreateVertexBuffer();
    virtual long __stdcall EnumZBufferFormats(const Guid* device,
                                          long(__stdcall* callback)(DDPixelFormat*, void*),
                                          void* context);                       // 6
};

// Clipper at Display+0x1ac (IDirectDrawClipper-shaped).
struct DirectDrawClipper {
    virtual long __stdcall QueryInterface();
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();                                           // 2
    virtual long __stdcall GetClipList();
    virtual long __stdcall GetHWnd();
    virtual long __stdcall Initialize();
    virtual long __stdcall IsClipListChanged();
    virtual long __stdcall SetClipList();
    virtual long __stdcall SetHWnd(unsigned long flags, void* window);          // 8
};

// Gamma control at Display+0x1a4 (IDirectDrawGammaControl-shaped, queried
// from the primary surface).
struct DirectDrawGammaControl {
    virtual long __stdcall QueryInterface();
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();                                           // 2
    virtual long __stdcall GetGammaRamp();
    virtual long __stdcall SetGammaRamp(unsigned long flags, void* ramp);       // 4
};
