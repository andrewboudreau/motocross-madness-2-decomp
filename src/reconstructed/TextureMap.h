#pragma once

#include <stdio.h>

#include "BaseObject.h"
#include "TextureMapManager.h"
#include "Tgafile.h"

// Palette object at TextureMap+0x2c: +0x710 maps 555 colours to palette
// indices.
struct UnknownTexturePalette {
    // ColorMapper's table getters (Quantize.h): Pixtrans.cpp calls them on
    // its palette argument.
    unsigned char* GetColors();        // 0x004de270: &entries
    unsigned char* GetIndexFrom555();        // 0x004de280: &indexFrom555
    unsigned char* GetIndexFrom565();        // 0x004de290: &field_0x8710 (565 to index)

    unsigned char field_0x000[0x10];
    // Named like ColorMapper's members at the same offsets (Quantize.h).
    unsigned char entries[256][3];                 // +0x010 RGB entries
    unsigned short entries555[256];                // +0x310 entries as 555 (Pixtrans 0x004d11c0, 8 to 555)
    unsigned short entries565[256];                // +0x510 entries as 565 (Pixtrans 0x004d1150, 8 to 565)
    unsigned char indexFrom555[0x8000];            // +0x710 555 colour -> palette index
};

// Formats slots 4 and 5 pick between: +0x0c without alpha, +0x10 with
// alpha. +0x14 is how many times slot 5 halves textures wider than 32.
// 0x0050a590 also takes the manager (+0x00) and the ManagedTextureGroups
// for textures without (+0x04) and with (+0x08) alpha from it.
struct UnknownTextureFormatChoice {
    TextureMapManager* manager;               // +0x0  the TextureMapManager 0x0050a590 loads with
    ManagedTextureGroup* opaqueGroup;         // +0x4  group taking ManagedTextures without alpha (0x0050a590)
    ManagedTextureGroup* alphaGroup;          // +0x8  group taking 4444/8888 ManagedTextures (0x0050a590)
    int opaqueFormat;                         // +0xC  format picked for a non-alpha request (PCTextureMap slots 4/5)
    int alphaFormat;                          // +0x10  format picked for a 4444/8888 request (PCTextureMap slots 4/5)
    int halvings;                             // +0x14  times a texture wider than 32 is halved; the loader sets 0..2 from memory
};

#include "FileStream.h"

// A render state and its value (RenderTarget slot 8).
struct UnknownRenderStatePair {
    int state;
    int value;
};

// RTTI: TextureMap : BaseObject (vtable 0x00558354; its code sits among
// Texmap.cpp's literals). Slots 4-20 are pure (`_purecall`); they are named after
// PCTextureMap's overrides (DirectDraw/Direct3D calls in PCTexMap.cpp), and
// slots 4/5/8/19 agree with the krusty2 TerrainSurfaceBase and shadow views.
class TextureMap : public BaseObject {
public:
    // 0x0050a4e0: registers with `manager` (0x005112f0) when `registered`.
    TextureMap(TextureMapManager* manager, int registered);
    virtual ~TextureMap();                            // 0x0050ab40 (deleting wrapper 0x0050a570)
    virtual int CreateSurface(void* bits, int width, int height, int stride, int minimumSize,
                              int sourceFormat, int format, UnknownTexturePalette* palette,
                              int flags, void* surfacePalette, int checkMemory, int unused,
                              int addressU, int addressV, UnknownTextureFormatChoice* choice,
                              int alphaThreshold, unsigned int key) = 0;
    virtual int LoadSurface(UnknownTextureStream* stream, int width, int height, int minimumSize,
                            int fileFormat, int dataSize, int format, UnknownTexturePalette* palette,
                            int flags, void* surfacePalette, int addressU, int addressV,
                            UnknownTextureFormatChoice* choice, int alphaThreshold, unsigned int key) = 0;
    virtual TextureMap* Clone() = 0;
    virtual int IsResident() = 0;
    virtual int CreateTextureSurface(int upload, int asRenderTarget, int localVideo) = 0;
    virtual int Upload(struct UnknownRect* rect, int mode) = 0;
    virtual void ReleaseTextureSurface() = 0;
    virtual int SetAsTexture() = 0;
    virtual void RecreateIfLost() = 0;
    virtual void* LockSurface(void* rect, long* pitch, int flags) = 0;
    virtual int UnlockSurface(void* rect) = 0;
    virtual int BuildMipLevels(int filter) = 0;
    virtual void* LockLevel(int level) = 0;
    virtual int UnlockLevel(int level) = 0;
    virtual int ApplyColorKey(unsigned int color) = 0;
    virtual void Bind() = 0;
    virtual int DumpAllLevels() = 0;

    // 0x0050abd0: sets the address-mode render states (0x13, 0x14) of an
    // alpha texture; 0 for other formats.
    int SetAddressModes(int addressU, int addressV);

    TextureMap* nextTexture;                  // +0x08 next in the manager's list
    TextureMap* previousTexture;              // +0x0c previous in the manager's list
    TextureMapManager* ownerManager;          // +0x10 manager (constructor argument)
    int textureWidth;                         // +0x14 width
    int textureHeight;                        // +0x18 height
    int minimumMipSize;                       // +0x1C  CreateSurface's minimum size: the load argument, or the width without mips
    int mapFormat;                            // +0x20 pixel format
    int mipCount;                             // +0x24 mip level count
    BaseObject* sourcePalette;                // +0x28 released by the destructor
    UnknownTexturePalette* colorMapper;       // +0x2c CreateSurface's palette; handed to the Pixtrans converters
    int usesColorKey;                         // +0x30 has a colour key
    int colorKeyValue;                        // +0x34 colour key (also +0x38)
    int field_0x38;
    int averageColor;                         // +0x3C  Pixtrans AverageColor 0x004d24d0 (slot 4); Terrain.h's name
    int field_0x40;
    int statePairCount;                       // +0x44 render-state pair count
    UnknownRenderStatePair statePairs[4];     // +0x48 applied by slot 19 (length not established)
    int mapFlags;                             // +0x68 bit 0: a ManagedTexture
    int halvings;                             // +0x6C  the format choice's halvings (slot 5)
};

class Palette8;

// 0x0050a590 (cdecl): the texture `name` (format `format`), shared through
// the resource manager (0x00572b44; AddRef on reuse when `addRef`). With an
// archive entry and `fromArchive` it reads the file format, data size,
// width, height and a palette name from the entry's stream and loads it
// through slot 5; otherwise it loads the TGA file (0x005125c0), halves it
// while wider than 32 up to choice+0x14 times and loads it through slot 4.
// A ManagedTexture when `choice` has a group for the format, else a
// PCTextureMap; 0 on failure.
#include "TextureLoader.h"
