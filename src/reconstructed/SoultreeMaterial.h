#pragma once

// SoultreeMaterial.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\SoultreeMaterial.cpp",
// 0x005740d8): a scene material that loads its texture and sets the render
// target's blend and texture-stage states.
//
// RTTI: SoultreeMaterial : GameObject (single base, mdisp 0), vtable 0x00557cb0
// with GameObject's 27 slots; only slot 0 (scalar deleting destructor
// 0x004ff090) is its own. TU code 0x004fefe0..0x00500214: after soultree.cpp's
// $E initializers and its out-of-line 3x3 transpose (0x004fefb0), and before
// SoulTreePhysics.cpp's first function (0x00500220). The material keys come
// from the parser's literals ("TextureMap", "MappingType", "SourceBlend", ...).
// Field and member names are provisional; offsets are decoded.

#include "GameObject.h"
#include "ManagedTexture.h"
#include "Parameterblocks.h"
#include "PCRenderTarget.h"
#include "TextureMapManager.h"

// The 0x70-byte texture holder at +0x70 (RTTI SurfaceMap : BaseObject, vtable
// 0x00558248; code 0x005051f0..0x00505350, reconstructed in
// samples/physics/motion/SurfaceMap.cpp). Only what the material calls.
class SurfaceMap : public BaseObject {
public:
    SurfaceMap(TextureMap* texture, const void* material); // 0x005051f0
    void SetTexture(TextureMap* texture);                   // 0x005052c0
    void Select();                                          // 0x00505310

    TextureMap* texture;                                    // +0x08
    unsigned char field_0x0c[0x70 - 0x0c];
};

// The six dwords at +0x74, handed to the texture loader as its format
// choice (+0x80 is the format UnknownTextureFormatChoice calls field_0x0c).
// The two ManagedTextureGroups at +0x04/+0x08 receive the procedural managed
// textures (0x0050c6c0), by whether the format has alpha (0x00511ad0).
struct SoultreeTextureOptions {
    TextureMapManager* textureManager; // creates procedural textures
    ManagedTextureGroup* opaqueTextures; // textures without alpha
    ManagedTextureGroup* alphaTextures; // textures with alpha
    int format;                               // format; 1555 when colour keyed
    int field_0x10;
    int field_0x14;
};

// 0x0050a590 (Texmap.cpp, cdecl): loads the texture file `name`; 0 on failure.
// TrackOverlay.h declares the same entry with an int in place of `options`.
TextureMap* UnknownFunction50a590(TextureMapManager* manager, const char* name, int format,
                                  int a4, int flags, int sourceBlend, int destBlend,
                                  SoultreeTextureOptions* options, int alphaThreshold,
                                  unsigned int key, int a11, int a12);

class SoultreeMaterial : public GameObject {
public:
    explicit SoultreeMaterial(int flags);     // 0x004fefe0
    virtual ~SoultreeMaterial();               // 0x004ff120 (deleting wrapper 0x004ff090)

    // 0x004ff0b0: attaches the render target and the texture options.
    SoultreeMaterial* Attach(RenderTarget* target, const SoultreeTextureOptions* options,
                               TextureMapManager* manager, int format);
    void ApplyRenderStates();                 // 0x004ff180: sets the render states
    void RestoreRenderStates();               // 0x004ff410: restores them
    void ReadSaved(UnknownParameterStream* stream);             // 0x004ff450: reads a saved material
    void LoadTexture();                       // 0x004ff620: (re)loads the texture
    // 0x004ff9e0: reads the material keys; loads the texture when `load`.
    void ReadKeys(UnknownParameterBlock* block, int load);
    void MakeUntextured();                    // 0x005000b0: an untextured "NONE" material
    void CopyFrom(const SoultreeMaterial* other);              // 0x005000f0: copies `other`

    char textureName[0x40];                   // texture name ("TextureMap")
    TextureMapManager* textureManager;        // loads named textures
    SurfaceMap* field_0x70;
    SoultreeTextureOptions field_0x74;
    int textureFormat;                        // "TextureFormat"
    unsigned int colorKey;                    // colour key, 0x00rrggbb ("ColorKey")
    int sourceBlend;                          // "SourceBlend" (1 ZERO .. 13 BOTHINVSRCALPHA)
    int destBlend;                            // "DestBlend"
    int hasTextureName;                       // has a texture name
    int useVertexColor;                       // "UseVertexColor"
    int mipMapped;                            // "MipMapped" (default 1)
    int hasColorKey;                          // colour key given
    int hasSourceBlend;                       // source blend given
    int hasDestBlend;                         // dest blend given
    int clampTexture;                         // "ClampTexture"
    int field_0xb8;                           // 1 after construction
    int hasAlpha;                             // alpha given
    float materialAlpha;                      // "Alpha" (default 1.0)
    float textureSpeed;                       // "TextureSpeed"
    unsigned short mappingType;                      // "MappingType" (0 STANDARD .. 9 NONE)
    int field_0xcc;
};
