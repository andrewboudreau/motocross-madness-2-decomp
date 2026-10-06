#pragma once

#include "PCTextureMap.h"

class ManagedTexture;

// A square part of a CacheTexture page, a node of the page's quadtree
// (0x30-byte blocks from the manager's pool, TextureMapManager+0x70). A
// leaf holds at most one ManagedTexture (ManagedTexture+0x94).
struct UnknownTextureRegion {
    // Inlined wherever a block is taken from the pool.
    void Init(UnknownTextureRegion* parent, int level) {
        field_0x10 = parent;
        field_0x14 = 0;
        field_0x18 = level;
        field_0x1c = field_0x20 = field_0x24 = field_0x28 = 0;
        field_0x2c = 0;
        for (int i = 0; i < 4; i++)
            field_0x00[i] = 0;
    }

    UnknownTextureRegion* field_0x00[4];      // quarters, all 0 for a leaf
    UnknownTextureRegion* field_0x10;         // parent
    ManagedTexture* field_0x14;               // occupant
    int field_0x18;                           // level (side 2 << (level - 1))
    float field_0x1c;                         // left (u)
    float field_0x20;                         // top (v)
    float field_0x24;                         // right (u)
    float field_0x28;                         // bottom (v)
    int field_0x2c;                           // reserved (listed in +0xd0)
};

// RTTI: CacheTexture : PCTextureMap (vtable 0x005583d8, overriding only the
// destructor), 0x190 bytes. The destructor (0x0050f830, deleting wrapper
// 0x0050f810) is the implicit one: it does not reset the vtable pointer. A page that a ManagedTextureGroup packs its
// ManagedTextures onto, split as a quadtree of regions. Its code sits
// between ManagedTextureGroup's and ManagedTexture's. Names are provisional.
class CacheTexture : public PCTextureMap {
public:
    // 0x0050f6a0: a page of one free level-`level` region.
    CacheTexture(TextureMapManager* manager, ManagedTextureGroup* group, int level);

    // 0x0050f890: lists the textures placed on the page in `textures` and
    // returns how many.
    int UnknownFunction50f890(ContainerList<ManagedTexture*>* textures);
    // 0x0050f8e0: adds the occupants below `region` to `textures`.
    int UnknownFunction50f8e0(ContainerList<ManagedTexture*>* textures, UnknownTextureRegion* region);
    // 0x0050f9b0: reserves regions for `levels[0..count]` textures, taking
    // what fits off `levels`; whether everything fitted.
    int UnknownFunction50f9b0(int* levels, int count);
    void UnknownFunction50fc40();             // 0x0050fc40: empties the page
    // 0x0050fc60: the first quarter of `region`, splitting it if needed.
    UnknownTextureRegion* UnknownFunction50fc60(UnknownTextureRegion* region);
    // 0x0050fc90: evicts the occupant and drops the reservation.
    void UnknownFunction50fc90(UnknownTextureRegion* region);
    // 0x0050fd60: empties and frees the quarters below `region`.
    void UnknownFunction50fd60(UnknownTextureRegion* region);
    // 0x0050fdb0: places textures from `textures` on reserved regions,
    // adding each one placed to `placed` when given (and blitting it when
    // not); 0 once a level has no reserved region left.
    int UnknownFunction50fdb0(ContainerList<ManagedTexture*>* textures, ContainerList<ManagedTexture*>* placed);
    // 0x0050ffa0: splits `region` into quarters; returns the first.
    UnknownTextureRegion* UnknownFunction50ffa0(UnknownTextureRegion* region);
    // 0x00510120: evicts and unreserves `region`, then with `merge` joins
    // its parents back while all their quarters are empty leaves.
    void UnknownFunction510120(UnknownTextureRegion* region, int merge);
    // 0x00510250: puts `texture` in `region`.
    int UnknownFunction510250(ManagedTexture* texture, UnknownTextureRegion* region);
    void UnknownFunction5102b0(ManagedTexture* texture); // 0x005102b0: takes `texture` off the page
    // 0x005102d0: copies the occupant's mip levels from `level` down into
    // `region`'s part of the page surface.
    int UnknownFunction5102d0(UnknownTextureRegion* region, int level);

    ManagedTextureGroup* field_0x80;
    UnknownTextureRegion* field_0x84;         // the whole page
    int field_0x88[9];                        // free leaves per level
    int field_0xac[9];                        // reserved leaves per level
    ContainerList<UnknownTextureRegion*> field_0xd0[9]; // reserved leaves per level
    int field_0x184;                          // texels
    int field_0x188;                          // a blit happened without the display's +0x5bc
    int field_0x18c;                          // texels wanted on the page less its size (repacking)
};
