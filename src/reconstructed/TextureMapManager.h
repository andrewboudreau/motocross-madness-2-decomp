#pragma once

#include "GameObject.h"

class TextureMap;

// RTTI: TextureMapManager : GameObject (0x7c bytes, the size Game's
// initialiser allocates). Its code sits among TextureMapManager.cpp's literals.
class TextureMapManager : public GameObject {
public:
    TextureMapManager();                      // 0x00510bd0
    void UnknownFunction5112f0(TextureMap* texture); // 0x005112f0: registers a texture
    void UnknownFunction511300(TextureMap* texture); // 0x00511300: unregisters it
    void UnknownFunction511580();             // 0x00511580 (PCTextureMap slot 12 and destructor)
    // 0x00511310 / 0x00511370: grow-only scratch buffers (+0x5c / +0x60,
    // sizes +0x64 / +0x68) of at least `bytes`; 0 when the allocation fails.
    void* UnknownFunction511310(int bytes);
    void* UnknownFunction511370(int bytes);

    unsigned char field_0x2c[0x7c - 0x2c];
};
