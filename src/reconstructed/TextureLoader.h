#pragma once

class TextureMap;
class TextureMapManager;
class Palette8;
struct TextureContext;

// 0x0050a590: shared texture loader. The six-word choice layout is
// TextureContext in TextureMap.h; other local views explicitly
// convert at the call boundary instead of redeclaring this function.
TextureMap* TerrainLoadTexture(TextureMapManager* manager, const char* name, int format,
                              Palette8* palette, int flags, int addressU, int addressV,
                              TextureContext* choice, int alphaThreshold,
                              unsigned int key, int addRef, int fromArchive);
