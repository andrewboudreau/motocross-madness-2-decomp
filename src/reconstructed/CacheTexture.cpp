#include "CacheTexture.h"

#include <string.h>

#include "ManagedTexture.h"
#include "PCGame.h"
#include "TrackGame.h"

// Texels of a square level-`level` region (1 for level 0, none below).
static inline int LevelArea(int level) {
    if (level == 0)
        return 1;
    if (level < 0)
        return 0;
    int side = 2 << (level - 1);
    return side * side;
}

// 0x0050f6a0
CacheTexture::CacheTexture(TextureMapManager* manager, ManagedTextureGroup* group, int level)
    : PCTextureMap(manager, 0) {
    field_0x80 = group;
    field_0x184 = LevelArea(level);
    for (int i = 0; i < 9; i++) {
        field_0x88[i] = field_0xac[i] = 0;
        field_0xd0[i].Init(32, 32);
    }
    field_0x84 = (UnknownTextureRegion*)field_0x10->field_0x70->Alloc();
    field_0x84->Init(0, level);
    field_0x84->field_0x1c = field_0x84->field_0x20 = 0.0f;
    field_0x84->field_0x24 = field_0x84->field_0x28 = 1.0f;
    field_0x88[level] = 1;
    field_0x188 = 0;
}

// 0x0050f890
int CacheTexture::UnknownFunction50f890(ContainerList<ManagedTexture*>* textures) {
    textures->Clear();
    if (field_0x84)
        return UnknownFunction50f8e0(textures, field_0x84);
    return 0;
}

// 0x0050f8e0
int CacheTexture::UnknownFunction50f8e0(ContainerList<ManagedTexture*>* textures, UnknownTextureRegion* region) {
    if (region->field_0x00[0]) {
        int count = 0;
        for (int i = 0; i < 4; i++)
            count += UnknownFunction50f8e0(textures, region->field_0x00[i]);
        return count;
    }
    ManagedTexture* texture = region->field_0x14;
    if (texture) {
        textures->Add(texture);
        return 1;
    }
    return 0;
}

// 0x0050fc40
void CacheTexture::UnknownFunction50fc40() {
    UnknownTextureRegion* root = field_0x84;
    if (root->field_0x00[0])
        UnknownFunction50fd60(root);
    else
        UnknownFunction50fc90(root);
}

// 0x0050fc60
UnknownTextureRegion* CacheTexture::UnknownFunction50fc60(UnknownTextureRegion* region) {
    UnknownTextureRegion* first = region->field_0x00[0];
    if (!first) {
        UnknownFunction50fc90(region);
        first = UnknownFunction50ffa0(region);
    }
    return first;
}

// 0x0050fc90
void CacheTexture::UnknownFunction50fc90(UnknownTextureRegion* region) {
    if (region->field_0x14) {
        region->field_0x14->field_0x80 = 0;
        region->field_0x14->field_0x94 = 0;
        region->field_0x14->field_0xa8 = -1;
        region->field_0x14 = 0;
    }
    if (region->field_0x2c) {
        field_0xac[region->field_0x18]--;
        field_0xd0[region->field_0x18].RemoveOrdered(region);
        region->field_0x2c = 0;
        field_0x88[region->field_0x18]++;
    }
}

// 0x0050fd60
void CacheTexture::UnknownFunction50fd60(UnknownTextureRegion* region) {
    if (!region->field_0x00[0])
        return;
    for (int i = 0; i < 4; i++) {
        UnknownTextureRegion* quarter = region->field_0x00[i];
        if (quarter) {
            if (quarter->field_0x00[0])
                UnknownFunction50fd60(quarter);
            else
                UnknownFunction50fc90(quarter);
            field_0x10->field_0x70->Free(region->field_0x00[i]);
            region->field_0x00[i] = 0;
        }
    }
}

// 0x0050ffa0
UnknownTextureRegion* CacheTexture::UnknownFunction50ffa0(UnknownTextureRegion* region) {
    float left = region->field_0x1c;
    float right = region->field_0x24;
    float top = region->field_0x20;
    float bottom = region->field_0x28;
    float middleU = (region->field_0x24 - region->field_0x1c) * 0.5f + left;
    float middleV = (region->field_0x28 - region->field_0x20) * 0.5f + top;
    for (int i = 0; i < 4; i++) {
        region->field_0x00[i] = (UnknownTextureRegion*)field_0x10->field_0x70->Alloc();
        region->field_0x00[i]->Init(region, region->field_0x18 - 1);
    }
    region->field_0x00[0]->field_0x1c = left;
    region->field_0x00[0]->field_0x20 = top;
    region->field_0x00[0]->field_0x24 = middleU;
    region->field_0x00[0]->field_0x28 = middleV;
    region->field_0x00[1]->field_0x1c = middleU;
    region->field_0x00[1]->field_0x20 = top;
    region->field_0x00[1]->field_0x24 = right;
    region->field_0x00[1]->field_0x28 = middleV;
    region->field_0x00[2]->field_0x1c = left;
    region->field_0x00[2]->field_0x20 = middleV;
    region->field_0x00[2]->field_0x24 = middleU;
    region->field_0x00[2]->field_0x28 = bottom;
    region->field_0x00[3]->field_0x1c = middleU;
    region->field_0x00[3]->field_0x20 = middleV;
    region->field_0x00[3]->field_0x24 = right;
    region->field_0x00[3]->field_0x28 = bottom;
    field_0x88[region->field_0x18]--;
    field_0x88[region->field_0x18 - 1] += 4;
    return region->field_0x00[0];
}

// 0x00510120
void CacheTexture::UnknownFunction510120(UnknownTextureRegion* region, int merge) {
    if (region->field_0x14) {
        region->field_0x14->field_0x80 = 0;
        region->field_0x14->field_0x94 = 0;
        region->field_0x14 = 0;
    }
    if (region->field_0x2c) {
        field_0xac[region->field_0x18]--;
        field_0xd0[region->field_0x18].RemoveOrdered(region);
        region->field_0x2c = 0;
        field_0x88[region->field_0x18]++;
    }
    if (!merge)
        return;
    for (UnknownTextureRegion* parent = region->field_0x10; parent; parent = parent->field_0x10) {
        int i;
        for (i = 0; i < 4; i++) {
            if (parent->field_0x00[i]->field_0x00[0] || parent->field_0x00[i]->field_0x2c)
                return;
        }
        for (i = 0; i < 4; i++) {
            field_0x10->field_0x70->Free(parent->field_0x00[i]);
            parent->field_0x00[i] = 0;
        }
        field_0x88[parent->field_0x18 - 1] -= 4;
        field_0x88[parent->field_0x18]++;
    }
}

// 0x00510250
int CacheTexture::UnknownFunction510250(ManagedTexture* texture, UnknownTextureRegion* region) {
    region->field_0x14 = texture;
    texture->field_0x94 = region;
    region->field_0x14->field_0x80 = this;
    region->field_0x2c = 1;
    field_0x88[texture->field_0xac]--;
    texture->field_0xa8 = region->field_0x18;
    texture->field_0x94 = region;
    texture->field_0x80 = this;
    return 1;
}

// 0x005102b0
void CacheTexture::UnknownFunction5102b0(ManagedTexture* texture) {
    UnknownFunction510120(texture->field_0x94, 1);
}

// 0x0050fdb0
int CacheTexture::UnknownFunction50fdb0(ContainerList<ManagedTexture*>* textures,
                                        ContainerList<ManagedTexture*>* placed) {
    while (textures->m_count > 0) {
        ManagedTexture* texture = textures->Get(0);
        if (field_0xd0[texture->field_0xac].m_count == 0)
            return 0;
        UnknownTextureRegion* region = field_0xd0[texture->field_0xac].Get(0);
        field_0xd0[texture->field_0xac].RemoveOrdered(region);
        UnknownFunction510250(texture, region);
        textures->RemoveOrdered(texture);
        if (placed) {
            placed->Add(texture);
            texture->field_0x98 = 0;
        } else {
            UnknownFunction5102d0(region, 0);
        }
    }
    return 1;
}
