#pragma once

#include "GameObject.h"

// RTTI: TextureMapManager : GameObject (0x7c bytes, the size Game's
// initialiser allocates). Only the constructor is declared.
class TextureMapManager : public GameObject {
public:
    TextureMapManager();                      // 0x00510bd0
    void UnknownFunction511580();             // 0x00511580 (PCTextureMap slot 12 and destructor)
    // 0x00511310 / 0x00511370: grow-only scratch buffers (+0x5c / +0x60,
    // sizes +0x64 / +0x68) of at least `bytes`; 0 when the allocation fails.
    void* UnknownFunction511310(int bytes);
    void* UnknownFunction511370(int bytes);

    unsigned char field_0x2c[0x7c - 0x2c];
};
