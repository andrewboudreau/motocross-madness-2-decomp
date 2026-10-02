#pragma once

#include "BaseObject.h"
#include "MemTag.h"
#include "RenderInterfaces.h"
#include "TextureMapManager.h"

// Palette object at TextureMap+0x2c: +0x710 maps 555 colours to palette
// indices.
struct UnknownTexturePalette {
    unsigned char field_0x000[0x10];
    unsigned char field_0x010[256][3];             // RGB entries
    unsigned char field_0x310[0x710 - 0x310];
    unsigned char field_0x710[0x8000];
};

// Formats slot 4 picks between: +0x0c without alpha, +0x10 with alpha.
struct UnknownTextureFormatChoice {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
};

// A render state and its value (RenderTarget slot 8).
struct UnknownRenderStatePair {
    int state;
    int value;
};

// RTTI: TextureMap : BaseObject (vtable 0x00558354; its code sits among
// Texmap.cpp's literals). Slots 4-20 are pure (`_purecall`). Only what
// PCTextureMap uses is declared; names are provisional.
class TextureMap : public BaseObject {
public:
    TextureMap(TextureMapManager* manager, int value); // 0x0050a4e0
    virtual ~TextureMap();                            // 0x0050ab40 (deleting wrapper 0x0050a570)
    virtual int UnknownVirtualSlot4(void* bits, int width, int height, int stride, int minimumSize,
                                    int sourceFormat, int format, UnknownTexturePalette* palette,
                                    int flags, void* surfacePalette, int checkMemory, int unused,
                                    int addressU, int addressV, UnknownTextureFormatChoice* choice,
                                    int alphaThreshold, unsigned int key) = 0;
    virtual void UnknownVirtualSlot5() = 0;
    virtual TextureMap* UnknownVirtualSlot6() = 0;
    virtual int UnknownVirtualSlot7() = 0;
    virtual int UnknownVirtualSlot8(int a, int b, int c) = 0;
    virtual int UnknownVirtualSlot9(struct UnknownRect* rect, int mode) = 0;
    virtual void UnknownVirtualSlot10() = 0;
    virtual int UnknownVirtualSlot11() = 0;
    virtual void UnknownVirtualSlot12() = 0;
    virtual void* UnknownVirtualSlot13(void* rect, long* pitch, int flags) = 0;
    virtual int UnknownVirtualSlot14(void* rect) = 0;
    virtual int UnknownVirtualSlot15(int filter) = 0;
    virtual void* UnknownVirtualSlot16(int level) = 0;
    virtual int UnknownVirtualSlot17(int level) = 0;
    virtual int UnknownVirtualSlot18(unsigned int color) = 0;
    virtual void UnknownVirtualSlot19() = 0;
    virtual int UnknownVirtualSlot20() = 0;

    int field_0x08;
    int field_0x0c;
    TextureMapManager* field_0x10;            // manager (constructor argument)
    int field_0x14;                           // width
    int field_0x18;                           // height
    int field_0x1c;
    int field_0x20;                           // pixel format
    int field_0x24;                           // 1: no mip levels
    unsigned char field_0x28[0x2c - 0x28];
    UnknownTexturePalette* field_0x2c;
    int field_0x30;                           // has a colour key
    int field_0x34;                           // colour key (also +0x38)
    int field_0x38;
    int field_0x3c;                           // from Pixtrans 0x004d24d0 (slot 4)
    unsigned char field_0x40[0x44 - 0x40];
    int field_0x44;                           // render-state pair count
    UnknownRenderStatePair field_0x48[4];     // applied by slot 19 (length not established)
    int field_0x68;                           // bit 0: a CacheTexture
    unsigned char field_0x6c[0x70 - 0x6c];
};

// Object at PCTextureMap+0x7c; its destructor 0x0052d050 sits among
// vfwdeco.cpp's literals.
class UnknownVideoDecoder {
public:
    ~UnknownVideoDecoder();                   // 0x0052d050
};

// cdecl 0x00511970 (Tgafile.cpp): bytes per pixel of a pixel format.
int UnknownFunction511970(int format);

// Global at 0x00689964; when set, PCTextureMap tells the manager
// (0x00511580) about lost or released surfaces.
extern int g_UnknownGlobal689964;

// cdecl 0x004d1b90: downsamples `source` (stride in pixels) into the
// width x height `destination`.
void UnknownFunction4d1b90(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, int a, int format, UnknownTexturePalette* palette, int filter);

// Tgafile.cpp helpers (cdecl): whether a format has alpha, and its
// DirectDraw pixel format.
int UnknownFunction511ad0(int format);
void UnknownFunction5119c0(int format, void* pixelFormat);

// Pixtrans.cpp converters (cdecl): 0x004d1d20 copies `source` into
// `destination`; 0x004d24d0 inspects the converted bits.
void UnknownFunction4d1d20(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, int format, int sourceFormat, int a,
                           UnknownTexturePalette* palette, int alphaThreshold, unsigned int key);
int UnknownFunction4d24d0(void* bits, int width, int height, int stride, int format,
                          UnknownTexturePalette* palette);

// Shared texture surfaces reused across textures: 0x0068a36c by mip level
// count, 0x0068a394 for single-level textures.
extern UnknownSurfaceInterface* g_UnknownSharedMipSurfaces68a36c[10];
extern UnknownSurfaceInterface* g_UnknownSharedSurfaces68a394[2];

// Global at 0x00689968: when set, slot 4 refills the first level.
extern int g_UnknownGlobal689968;

// cdecl 0x004c86e0: formats a DirectDraw result with the caller's __FILE__
// and __LINE__.
void UnknownReportDirectDrawError(long result, const char* file, int line);

// RTTI: PCTextureMap : TextureMap (vtable 0x00555fec; its code sits among
// PCTexMap.cpp's literals). Names are provisional.
class PCTextureMap : public TextureMap {
public:
    PCTextureMap(TextureMapManager* manager, int value); // 0x004c5f00
    virtual ~PCTextureMap();                  // 0x004c5f50 (deleting wrapper 0x004c5f30)
    // 0x004c69c0: creates the surfaces from `bits`.
    virtual int UnknownVirtualSlot4(void* bits, int width, int height, int stride, int minimumSize,
                                    int sourceFormat, int format, UnknownTexturePalette* palette,
                                    int flags, void* surfacePalette, int checkMemory, int unused,
                                    int addressU, int addressV, UnknownTextureFormatChoice* choice,
                                    int alphaThreshold, unsigned int key);
    virtual void UnknownVirtualSlot5();       // 0x004c6080
    virtual TextureMap* UnknownVirtualSlot6(); // 0x004c71c0: a copy of the texture
    virtual int UnknownVirtualSlot7();        // 0x004c7470: whether +0x74 exists
    // 0x004c7480: creates the texture surface from +0x70 (or shares it).
    virtual int UnknownVirtualSlot8(int a, int b, int c);
    // 0x004c7640: uploads +0x70 into +0x74, level by level.
    virtual int UnknownVirtualSlot9(struct UnknownRect* rect, int mode);
    virtual void UnknownVirtualSlot10();      // 0x004c7610: releases the texture surface
    virtual int UnknownVirtualSlot11();       // 0x004c7970: sets it as texture stage 0
    virtual void UnknownVirtualSlot12();      // 0x004c6040: restores a lost surface
    virtual void* UnknownVirtualSlot13(void* rect, long* pitch, int flags); // 0x004c79e0: locks +0x70
    virtual int UnknownVirtualSlot14(void* rect); // 0x004c7a50: unlocks +0x70
    virtual int UnknownVirtualSlot15(int filter); // 0x004c77d0: builds the mip levels
    virtual void* UnknownVirtualSlot16(int level); // 0x004c7a70: locks a mip level
    virtual int UnknownVirtualSlot17(int level); // 0x004c7ad0: unlocks it
    virtual int UnknownVirtualSlot18(unsigned int color); // 0x004c81d0: colour-keys every level
    virtual void UnknownVirtualSlot19();      // 0x004c79a0: binds and applies the render states
    virtual int UnknownVirtualSlot20();       // 0x004c8430: fills every mip level

    int UnknownFunction4c7420();              // 0x004c7420: recreates a lost texture surface
    // 0x004c7b00: blits +0x70 into `destination` unless `skip`.
    int UnknownFunction4c7b00(void* destinationRect, UnknownSurfaceInterface* destination,
                              void* sourceRect, int flags, int skip);
    // 0x004c83a0: the mip level whose width is `width`, or 0.
    UnknownSurfaceInterface* UnknownFunction4c83a0(int width);
    int UnknownFunction4c84e0(UnknownSurfaceInterface* surface, int value); // 0x004c84e0: fills a level
    void UnknownFunction4c7e30(unsigned int color); // 0x004c7e30: sets the colour key
    void UnknownFunction4c7ef0(UnknownSurfaceInterface* surface, unsigned int color); // 0x004c7ef0
    // 0x004c68e0: creates +0x70 with the first of `formats` (0-terminated)
    // the device accepts.
    int UnknownFunction4c68e0(UnknownSurfaceDesc* desc, int flags, int* formats);
    void UnknownFunction4c8550(UnknownSurfaceDesc* desc, int value);        // 0x004c8550

    UnknownSurfaceInterface* field_0x70;      // system-memory surface (counted in DirectX memory)
    UnknownSurfaceInterface* field_0x74;      // texture surface
    void* field_0x78;                         // palette for 8-bit textures
    UnknownVideoDecoder* field_0x7c;
};

class CacheTexture;

// Texture cache at CacheTexture+0x90 (TextureCache.cpp).
class UnknownTextureCache {
public:
    void UnknownFunction50c6c0(CacheTexture* texture); // 0x0050c6c0: adds a texture
};

// RTTI: CacheTexture : PCTextureMap (vtable 0x00558430; 0xbc bytes). Its
// constructor 0x00510500 sets TextureMap+0x68 bit 0.
class CacheTexture : public PCTextureMap {
public:
    explicit CacheTexture(TextureMapManager* manager); // 0x00510500

    unsigned char field_0x80[0x90 - 0x80];
    UnknownTextureCache* field_0x90;
    unsigned char field_0x94[0xbc - 0x94];
};

