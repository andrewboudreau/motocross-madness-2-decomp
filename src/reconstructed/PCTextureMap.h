#pragma once

#include "MemTag.h"
#include "RenderInterfaces.h"
#include "TextureMap.h"

// Object at PCTextureMap+0x7c; its destructor 0x0052d050 sits among
// vfwdeco.cpp's literals.
class UnknownVideoDecoder {
public:
    ~UnknownVideoDecoder();                   // 0x0052d050
};

// Global at 0x00689964; when set, PCTextureMap tells the manager
// (0x00511580) about lost or released surfaces.
extern int g_UnknownGlobal689964;

// cdecl 0x004d1b90: downsamples `source` (stride in pixels) into the
// width x height `destination`; 0 on failure.
int UnknownFunction4d1b90(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a, int format, UnknownTexturePalette* palette, int filter);

// Lzw.cpp (cdecl 0x004a03d0): expands `source` into `size` bytes.
void UnknownFunction4a03d0(void* destination, void* source, int size);

// Pixtrans.cpp converters (cdecl): 0x004d1d20 copies `source` into
// `destination`; 0x004d24d0 inspects the converted bits.
void UnknownFunction4d1d20(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, int format, int sourceFormat, int a,
                           UnknownTexturePalette* palette, int alphaThreshold, unsigned int key);
int UnknownFunction4d24d0(void* bits, int width, int height, int stride, int format,
                          UnknownTexturePalette* palette);

// bmpfile.cpp (cdecl): 0x004245f0 describes an 8-bit width x height
// bitmap with a 256-entry RGB palette; 0x00424380 writes it to `name`.
struct UnknownBitmapFile {
    unsigned char field_0x000[0x438];
    void* bits;
    char name[0x80];
};

void UnknownFunction4245f0(UnknownBitmapFile* bitmap, void* bits, unsigned char (*palette)[3], int width, int height);
int UnknownFunction424380(UnknownBitmapFile* bitmap);

// Pixels as the Pixtrans.cpp replacers take them by value: 32-bit RGBA
// (format 0x22b8) and 24-bit RGB (0x378), red first in memory.
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

// Pixtrans.cpp replacers (cdecl): every `from` pixel in the width x height
// `bits` (stride in pixels) becomes `to`. 32-bit 0x004d1970, 24-bit
// 0x004d1a20, 16-bit 0x004d1ac0, 8-bit 0x004d1b40.
void UnknownFunction4d1970(void* bits, UnknownPixel32 from, UnknownPixel32 to, int width, int height, int stride);
void UnknownFunction4d1a20(void* bits, UnknownPixel24 from, UnknownPixel24 to, int width, int height, int stride);
void UnknownFunction4d1ac0(void* bits, unsigned short from, unsigned short to, int width, int height, int stride);
void UnknownFunction4d1b40(void* bits, unsigned char from, unsigned char to, int width, int height, int stride);

// Shared texture surfaces reused across textures: 0x0068a36c by mip level
// count, 0x0068a394 for single-level textures.
extern UnknownSurfaceInterface* g_UnknownSharedMipSurfaces68a36c[10];
extern UnknownSurfaceInterface* g_UnknownSharedSurfaces68a394[10];

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
    // 0x004c6080: reads the texture from `stream`, then creates it through slot 4.
    virtual int UnknownVirtualSlot5(UnknownTextureStream* stream, int width, int height, int minimumSize,
                                    int fileFormat, int dataSize, int format, UnknownTexturePalette* palette,
                                    int flags, void* surfacePalette, int addressU, int addressV,
                                    UnknownTextureFormatChoice* choice, int alphaThreshold, unsigned int key);
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
    virtual int UnknownVirtualSlot20();       // 0x004c8430: dumps every mip level

    int UnknownFunction4c7420();              // 0x004c7420: recreates a lost texture surface
    // 0x004c7b40: copies `rect` of +0x70 to (x, y) in `destination`,
    // blending through `table` when given.
    int UnknownFunction4c7b40(unsigned long x, unsigned long y, UnknownSurfaceInterface* destination,
                              struct UnknownRect* rect, int flags, unsigned char* table);
    // 0x004c7b00: blits +0x70 into `destination` unless `skip`.
    int UnknownFunction4c7b00(void* destinationRect, UnknownSurfaceInterface* destination,
                              void* sourceRect, int flags, int skip);
    // 0x004c83a0: the mip level whose width is `width`, or 0.
    UnknownSurfaceInterface* UnknownFunction4c83a0(int width);
    int UnknownFunction4c84e0(UnknownSurfaceInterface* surface, const char* name); // 0x004c84e0: dumps a level
    void UnknownFunction4c7e30(unsigned int color); // 0x004c7e30: sets the colour key
    int UnknownFunction4c7ef0(UnknownSurfaceInterface* surface, unsigned int color); // 0x004c7ef0: keys a level
    // 0x004c68e0: creates +0x70 with the first of `formats` (0-terminated)
    // the device accepts.
    int UnknownFunction4c68e0(UnknownSurfaceDesc* desc, int flags, int* formats);
    void UnknownFunction4c8550(UnknownSurfaceDesc* desc, const char* name); // 0x004c8550: writes a level

    UnknownSurfaceInterface* field_0x70;      // system-memory surface (counted in DirectX memory)
    UnknownSurfaceInterface* field_0x74;      // texture surface
    void* field_0x78;                         // palette for 8-bit textures
    UnknownVideoDecoder* field_0x7c;
};

