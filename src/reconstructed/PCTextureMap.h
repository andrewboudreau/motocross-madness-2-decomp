#pragma once

#include "MemTag.h"
#include "RenderInterfaces.h"
#include "Pixtrans.h"
#include "TextureMap.h"
#include "VfwDeco.h"   // UnknownVideoDecoder at PCTextureMap+0x7c

// Global at 0x00689964; when set, PCTextureMap tells the manager
// (0x00511580) about lost or released surfaces.
extern int g_UnknownGlobal689964;

// Lzw.cpp (cdecl 0x004a03d0): expands `source` into `size` bytes.
void ExpandLzw(void* destination, void* source, int size);

// bmpfile.cpp (cdecl): 0x004245f0 describes an 8-bit bitmap and 0x00424380
// writes it (the level dump).
#include "bmpfile.h"

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
    virtual int CreateSurface(void* bits, int width, int height, int stride, int minimumSize,
                              int sourceFormat, int format, UnknownTexturePalette* palette,
                              int flags, void* surfacePalette, int checkMemory, int unused,
                              int addressU, int addressV, UnknownTextureFormatChoice* choice,
                              int alphaThreshold, unsigned int key);
    // 0x004c6080: reads the texture from `stream`, then creates it through slot 4.
    virtual int LoadSurface(UnknownTextureStream* stream, int width, int height, int minimumSize,
                            int fileFormat, int dataSize, int format, UnknownTexturePalette* palette,
                            int flags, void* surfacePalette, int addressU, int addressV,
                            UnknownTextureFormatChoice* choice, int alphaThreshold, unsigned int key);
    virtual TextureMap* Clone();               // 0x004c71c0: a copy of the texture
    virtual int IsResident();                 // 0x004c7470: whether +0x74 exists
    // 0x004c7480: creates the texture surface from +0x70 (or shares it).
    virtual int CreateTextureSurface(int a, int b, int c);
    // 0x004c7640: uploads +0x70 into +0x74, level by level.
    virtual int Upload(struct UnknownRect* rect, int mode);
    virtual void ReleaseTextureSurface();     // 0x004c7610: releases the texture surface
    virtual int SetAsTexture();               // 0x004c7970: sets it as texture stage 0
    virtual void RecreateIfLost();            // 0x004c6040: restores a lost surface
    virtual void* LockSurface(void* rect, long* pitch, int flags);          // 0x004c79e0: locks +0x70
    virtual int UnlockSurface(void* rect);        // 0x004c7a50: unlocks +0x70
    virtual int BuildMipLevels(int filter);       // 0x004c77d0: builds the mip levels
    virtual void* LockLevel(int level);            // 0x004c7a70: locks a mip level
    virtual int UnlockLevel(int level);          // 0x004c7ad0: unlocks it
    virtual int ApplyColorKey(unsigned int color);        // 0x004c81d0: colour-keys every level
    virtual void Bind();                      // 0x004c79a0: binds and applies the render states
    virtual int DumpAllLevels();              // 0x004c8430: dumps every mip level

    int RestoreTextureSurface();              // 0x004c7420: recreates a lost texture surface
    // 0x004c7b40: copies `rect` of +0x70 to (x, y) in `destination`,
    // blending through `table` when given.
    int CopyRectTo(unsigned long x, unsigned long y, UnknownSurfaceInterface* destination,
                              struct UnknownRect* rect, int flags, unsigned char* table);
    // 0x004c7b00: blits +0x70 into `destination` unless `skip`.
    int BlitTo(void* destinationRect, UnknownSurfaceInterface* destination,
                              void* sourceRect, int flags, int skip);
    // 0x004c83a0: the mip level whose width is `width`, or 0.
    UnknownSurfaceInterface* FindMipLevel(int width);
    int DumpLevel(UnknownSurfaceInterface* surface, const char* name);             // 0x004c84e0: dumps a level
    void SetColorKey(unsigned int color);           // 0x004c7e30: sets the colour key
    int ColorKeyLevel(UnknownSurfaceInterface* surface, unsigned int color);         // 0x004c7ef0: keys a level
    // 0x004c68e0: creates +0x70 with the first of `formats` (0-terminated)
    // the device accepts.
    int CreateSystemSurface(UnknownSurfaceDesc* desc, int flags, int* formats);
    void WriteLevel(UnknownSurfaceDesc* desc, const char* name);            // 0x004c8550: writes a level

    UnknownSurfaceInterface* systemSurface;    // +0x70: system-memory surface (counted in DirectX memory)
    UnknownSurfaceInterface* textureSurface;   // +0x74: texture surface
    void* texturePalette;                      // +0x78: palette for 8-bit textures
    UnknownVideoDecoder* videoDecoder;
};
