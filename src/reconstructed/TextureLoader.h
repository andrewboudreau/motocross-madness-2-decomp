#pragma once

class TextureMap;
class TextureMapManager;
class Palette8;
class ManagedTextureGroup;

// Formats slots 4 and 5 pick between: +0x0c without alpha, +0x10 with
// alpha. +0x14 is how many times slot 5 halves textures wider than 32.
// 0x0050a590 also takes the manager (+0x00) and the ManagedTextureGroups
// for textures without (+0x04) and with (+0x08) alpha from it.
struct TextureContext {
    TextureMapManager* manager;               // +0x0  the TextureMapManager 0x0050a590 loads with
    ManagedTextureGroup* opaqueGroup;         // +0x4  group taking ManagedTextures without alpha (0x0050a590)
    ManagedTextureGroup* alphaGroup;          // +0x8  group taking 4444/8888 ManagedTextures (0x0050a590)
    int opaqueFormat;                         // +0xC  format picked for a non-alpha request (PCTextureMap slots 4/5)
    int alphaFormat;                          // +0x10  format picked for a 4444/8888 request (PCTextureMap slots 4/5)
    int halvings;                             // +0x14  times a texture wider than 32 is halved; the loader sets 0..2 from memory
};

// 0x0050a590 (cdecl): the texture `name` (format `format`), shared through
// the resource manager (0x00572b44; AddRef on reuse when `addRef`). With an
// archive entry and `fromArchive` it reads the file format, data size,
// width, height and a palette name from the entry's stream and loads it
// through slot 5; otherwise it loads the TGA file (0x005125c0), halves it
// while wider than 32 up to choice+0x14 times and loads it through slot 4.
// A ManagedTexture when `choice` has a group for the format, else a
// PCTextureMap; 0 on failure.
TextureMap* TerrainLoadTexture(TextureMapManager* manager, const char* name, int format,
                              Palette8* palette, int flags, int srcBlend, int destBlend,
                              TextureContext* choice, int alphaThreshold,
                              unsigned int key, int addRef, int fromArchive);
