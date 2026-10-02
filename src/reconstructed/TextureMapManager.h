#pragma once

#include "GameObject.h"

// RTTI: TextureMapManager : GameObject (0x7c bytes, the size Game's
// initialiser allocates). Only the constructor is declared.
class TextureMapManager : public GameObject {
public:
    TextureMapManager();                      // 0x00510bd0
    void UnknownFunction511580();             // 0x00511580 (PCTextureMap slot 12 and destructor)

    unsigned char field_0x2c[0x7c - 0x2c];
};
