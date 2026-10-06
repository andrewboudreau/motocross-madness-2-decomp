#pragma once

// Grid1.h -- the reconstructed part of D:\aardvark\VC\krusty2\Grid1.cpp
// (code 0x0047c880..0x0047db5f). See docs/GRIDDRAW.md for the neighbouring
// Gridbase.cpp and Griddraw.cpp.
//
// Confirmed (tier 1): RTTI .?AVDrawableGridNodeSharedTextures@@ (type
// descriptor 0x0056c078, in Grid1.cpp's .data just before its __FILE__
// string 0x0056c0a0), vtable 0x00553e54 (9 slots), single base
// DrawableGridNode. The constructor 0x0047c880 writes the vptr at 0x0047c90e.
// It overrides slots 0 (0x0047c930), 2 (0x0047c960), 3 (0x0047caa0),
// 4 (0x0047d160), 6 (0x0047d310), 7 (0x0047d370) and 8 (0x0047d420); slot 5
// is the shared stub 0x00464e80. Slot 2 allocates 0x5c bytes, so the class
// adds no fields. Member, method and type names are provisional (tier 3).

#include "Griddraw.h"
#include "ManagedTexture.h"

class BaseObject;

// Grid1.cpp's view of the per-block records at GridNodeExtra::field_0x38
// (0x2c bytes, the same records Griddraw.h calls GridBlockRecord): +0 packs
// the texture reference, +4 the texture, +8 its AgeManager entry (0x0047d370
// passes record+8 and tests record+0x18, the entry's size).
struct Grid1BlockTexture {
    unsigned int own : 1;                      // +0x00 bit 0: texture at +4, else Terrain's table
    int index : 15;                            // +0x00 bits 1..15: table index (-1: none)
    ManagedTexture* texture;                   // +0x04
    GridAgeEntry ageEntry;                     // +0x08
    float field_0x1c;
    float field_0x20;
    float field_0x24;
    float detail;                              // +0x28 (0x0047d160)
};

// Grid1.cpp's view of GridNodeExtra (DrawableGridNode+0x3c): the node-wide
// texture is at +0x08 with its packed reference at +0x0c, a second object at
// +0x10, its AgeManager entry at +0x14 and texture flags at +0x3c (bit 16:
// whole node, bit n: block n).
struct Grid1NodeExtra {
    int field_0x00;
    void* field_0x04;
    ManagedTexture* texture;                   // +0x08
    unsigned int own : 1;                      // +0x0c packed like Grid1BlockTexture
    int index : 15;
    BaseObject* field_0x10;                    // +0x10 handed back to Terrain by 0x0047ca00
    GridAgeEntry ageEntry;                     // +0x14
    float field_0x28;
    float field_0x2c;
    float field_0x30;
    float detail;                              // +0x34
    Grid1BlockTexture* blocks;                 // +0x38 sixteen records or null
    int textureFlags;                          // +0x3c
};

// Grid1.cpp's view of Terrain (src/krusty2/broadphase/Terrain.h). The
// texture table at +0x544 is Terrain's ownedObjects[] and 0x005057d0 is the
// method Terrain.h calls RetireOwnedObject.
struct Grid1Terrain {
    void RetireOwnedObject(BaseObject* object);  // 0x005057d0 (ret 4)
    ManagedTexture* AcquireOwnedObject();        // 0x00505600 (Terrain.h's name)

    unsigned char field_0x000[0x18];
    GridRenderDevice* field_0x18;              // +0x18
    unsigned char field_0x01c[0x70 - 0x1c];
    int field_0x70;                            // +0x70
    unsigned char field_0x074[0x544 - 0x74];
    ManagedTexture* textures[(0xbec - 0x544) / 4];  // +0x544
    unsigned char field_0xbec[0xc24 - 0xbec];
    int field_0xc24;                           // +0xc24 pixel format of the node textures
    unsigned char field_0xc28[0xc84 - 0xc28];
    GridAgeManager* field_0xc84;               // +0xc84 texture AgeManager
};

// Grid1.cpp's view of the host object at Terrain+0x18 (Terrain.h: its +8
// is a PCCamera); slot 3 tests the camera's float at +0x200.
struct Grid1Camera {
    unsigned char field_0x000[0x200];
    float field_0x200;
};
struct Grid1RenderHost {
    int field_0x00;
    int field_0x04;
    Grid1Camera* camera;                       // +0x08
};

class DrawableGridNodeSharedTextures : public DrawableGridNode {
public:
    DrawableGridNodeSharedTextures(UnknownTextureStream* stream, int a1, GridNode* parent,
                                   GridTerrain* terrain, int level, int shift, int x, int z,
                                   int index, float* boundsMin, float* boundsMax);  // 0x0047c880 (ret 0x2c)

    virtual GridNode* UnknownVirtualSlot2(UnknownTextureStream* stream, int a1, DrawableGridNode* parent,
                                          int level, int shift, int x, int z, int index,
                                          void* a8, void* a9);  // 0x0047c960
    virtual void UnknownVirtualSlot3(float a);                  // 0x0047caa0
    virtual void UnknownVirtualSlot4(int block);                // 0x0047d160
    virtual void UnknownVirtualSlot6();                         // 0x0047d310
    virtual void UnknownVirtualSlot7(int block);                // 0x0047d370
    virtual int UnknownVirtualSlot8(int block);                 // 0x0047d420

    // 0x0047d470 / 0x0047d780: build the node-wide texture (or one block's)
    // from the node's run-length texture indices (tier 3 names).
    int UnknownFunction47d470(ManagedTexture* texture);         // 0x0047d470 (ret 4)
    int UnknownFunction47d780(int block, ManagedTexture* texture);  // 0x0047d780 (ret 8)

    Grid1NodeExtra* Extra() { return (Grid1NodeExtra*)extra; }
    Grid1Terrain* Terrain() { return (Grid1Terrain*)terrain; }
};

// AgeManager eviction callbacks registered for the node-wide texture and for
// one block's texture (tier 3).
int UnknownFunction47ca00(void* owner);
int UnknownFunction47ca40(void* owner, int block);

// Texture use table (defined with Terrain's data; tier 3 names).
extern int g_gridTextureCount;                 // 0x0068a2ec
extern ManagedTexture* g_gridTextures[];       // 0x0066047c
extern int g_gridTextureSizes[];               // 0x0065b65c
