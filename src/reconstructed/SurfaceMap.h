#pragma once

#include "BaseObject.h"

class TextureMap;

// Seventeen floats copied by retail 0x005051f0, matching D3DMATERIAL7's
// diffuse, ambient, specular, emissive and power layout (strong inference).
struct SurfaceMaterial {
    float diffuse[4];
    float ambient[4];
    float specular[4];
    float emissive[4];
    float power;
};

// RTTI SurfaceMap : BaseObject, primary table 0x00558248, object offset 0.
// Reconstruction grouping only: no original source filename is established.
class SurfaceMap : public BaseObject {
public:
    SurfaceMap(TextureMap* texture, const SurfaceMaterial* material); // 0x005051f0
    virtual ~SurfaceMap();                                         // 0x00505350
    void SetTexture(TextureMap* texture);                           // 0x005052c0
    int Apply();                                                   // 0x005052f0
    void Select();                                                 // 0x00505310

    TextureMap* texture;                  // +0x08, reference-counted
    SurfaceMaterial material;             // +0x0c, through +0x4f
    // SoultreeMaterial::LoadTexture allocates 0x70 bytes at 0x004ff650.
    // The observed SurfaceMap methods do not read or initialize this tail.
    unsigned char field_0x50[0x70 - 0x50];
};
