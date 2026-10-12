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
    ownerGroup = group;
    texelCount = LevelArea(level);
    for (int i = 0; i < 9; i++) {
        freeLeafCounts[i] = reservedLeafCounts[i] = 0;
        reservedLeaves[i].Init(32, 32);
    }
    rootRegion = (UnknownTextureRegion*)ownerManager->field_0x70->Alloc();
    rootRegion->Init(0, level);
    rootRegion->field_0x1c = rootRegion->field_0x20 = 0.0f;
    rootRegion->field_0x24 = rootRegion->field_0x28 = 1.0f;
    freeLeafCounts[level] = 1;
    field_0x188 = 0;
}

// 0x0050f890
int CacheTexture::ListTextures(ContainerList<ManagedTexture*>* textures) {
    textures->Clear();
    if (rootRegion)
        return CollectOccupants(textures, rootRegion);
    return 0;
}

// 0x0050f8e0
int CacheTexture::CollectOccupants(ContainerList<ManagedTexture*>* textures, UnknownTextureRegion* region) {
    if (region->field_0x00[0]) {
        int count = 0;
        for (int i = 0; i < 4; i++)
            count += CollectOccupants(textures, region->field_0x00[i]);
        return count;
    }
    ManagedTexture* texture = region->field_0x14;
    if (texture) {
        textures->Add(texture);
        return 1;
    }
    return 0;
}

// A step of the depth-first walk in 0x0050f9b0.
struct UnknownRegionStep {
    UnknownTextureRegion* region;
    int index;
};

// The largest level still wanted, taken off `wanted`; -1 when none is.
static inline int TakeLargest(int* wanted) {
    for (int level = 8; level >= 0; level--) {
        if (wanted[level]) {
            wanted[level]--;
            return level;
        }
    }
    return -1;
}

// 0x0050f9b0
int CacheTexture::ReserveRegions(int* levels, int count) {
    int wanted[9];
    memset(wanted, 0, sizeof(wanted));
    int space = texelCount;
    int fitted = 1;
    int level;
    for (level = 8; level >= 0; level--) {
        int n = levels[level];
        while (n) {
            int area = LevelArea(level);
            if (space < area) {
                fitted = 0;
                break;
            }
            space -= area;
            wanted[level]++;
            levels[level]--;
            n--;
        }
    }
    int same = 1;
    for (level = 0; level <= 8; level++) {
        if (reservedLeafCounts[level] != wanted[level]) {
            same = 0;
            break;
        }
    }
    if (same)
        return fitted;

    UnknownRegionStep steps[10];
    int depth = -1;
    int first = 1;
    UnknownTextureRegion* region = rootRegion;
    for (;;) {
        level = TakeLargest(wanted);
        if (level == -1)
            break;
        if (!first) {
            while (steps[depth].index++ == 3)
                depth--;
            region = steps[depth].region->field_0x00[steps[depth].index];
        } else {
            first = 0;
        }
        while (region->field_0x18 > level) {
            depth++;
            steps[depth].region = region;
            steps[depth].index = 0;
            region = FirstQuarter(region);
        }
        FreeQuarters(region);
        if (region->field_0x2c)
            continue;
        region->field_0x2c = 1;
        reservedLeafCounts[level]++;
        freeLeafCounts[level]--;
        reservedLeaves[level].Add(region);
    }
    for (; depth >= 0; depth--) {
        UnknownTextureRegion* parent = steps[depth].region;
        int i = steps[depth].index + 1;
        for (; i < 4; i++) {
            UnknownTextureRegion* quarter = parent->field_0x00[i];
            if (quarter->field_0x00[0])
                FreeQuarters(quarter);
            else
                EvictRegion(quarter);
        }
    }
    return fitted;
}

// 0x0050fc40
void CacheTexture::EmptyPage() {
    UnknownTextureRegion* root = rootRegion;
    if (root->field_0x00[0])
        FreeQuarters(root);
    else
        EvictRegion(root);
}

// 0x0050fc60
UnknownTextureRegion* CacheTexture::FirstQuarter(UnknownTextureRegion* region) {
    UnknownTextureRegion* first = region->field_0x00[0];
    if (!first) {
        EvictRegion(region);
        first = SplitRegion(region);
    }
    return first;
}

// 0x0050fc90
void CacheTexture::EvictRegion(UnknownTextureRegion* region) {
    if (region->field_0x14) {
        region->field_0x14->page = 0;
        region->field_0x14->region = 0;
        region->field_0x14->field_0xa8 = -1;
        region->field_0x14 = 0;
    }
    if (region->field_0x2c) {
        reservedLeafCounts[region->field_0x18]--;
        reservedLeaves[region->field_0x18].RemoveOrdered(region);
        region->field_0x2c = 0;
        freeLeafCounts[region->field_0x18]++;
    }
}

// 0x0050fd60
void CacheTexture::FreeQuarters(UnknownTextureRegion* region) {
    if (!region->field_0x00[0])
        return;
    for (int i = 0; i < 4; i++) {
        UnknownTextureRegion* quarter = region->field_0x00[i];
        if (quarter) {
            if (quarter->field_0x00[0])
                FreeQuarters(quarter);
            else
                EvictRegion(quarter);
            ownerManager->field_0x70->Free(region->field_0x00[i]);
            region->field_0x00[i] = 0;
        }
    }
}

// 0x0050ffa0
UnknownTextureRegion* CacheTexture::SplitRegion(UnknownTextureRegion* region) {
    float left = region->field_0x1c;
    float right = region->field_0x24;
    float top = region->field_0x20;
    float bottom = region->field_0x28;
    float middleU = (region->field_0x24 - region->field_0x1c) * 0.5f + left;
    float middleV = (region->field_0x28 - region->field_0x20) * 0.5f + top;
    for (int i = 0; i < 4; i++) {
        region->field_0x00[i] = (UnknownTextureRegion*)ownerManager->field_0x70->Alloc();
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
    freeLeafCounts[region->field_0x18]--;
    freeLeafCounts[region->field_0x18 - 1] += 4;
    return region->field_0x00[0];
}

// 0x00510120
void CacheTexture::ReleaseRegion(UnknownTextureRegion* region, int merge) {
    if (region->field_0x14) {
        region->field_0x14->page = 0;
        region->field_0x14->region = 0;
        region->field_0x14 = 0;
    }
    if (region->field_0x2c) {
        reservedLeafCounts[region->field_0x18]--;
        reservedLeaves[region->field_0x18].RemoveOrdered(region);
        region->field_0x2c = 0;
        freeLeafCounts[region->field_0x18]++;
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
            ownerManager->field_0x70->Free(parent->field_0x00[i]);
            parent->field_0x00[i] = 0;
        }
        freeLeafCounts[parent->field_0x18 - 1] -= 4;
        freeLeafCounts[parent->field_0x18]++;
    }
}

// 0x00510250
int CacheTexture::PlaceTexture(ManagedTexture* texture, UnknownTextureRegion* region) {
    region->field_0x14 = texture;
    texture->region = region;
    region->field_0x14->page = this;
    region->field_0x2c = 1;
    freeLeafCounts[texture->plannedLevel]--;
    texture->field_0xa8 = region->field_0x18;
    texture->region = region;
    texture->page = this;
    return 1;
}

// 0x005102b0
void CacheTexture::RemoveTexture(ManagedTexture* texture) {
    ReleaseRegion(texture->region, 1);
}

// 0x0050fdb0
int CacheTexture::PlaceTextures(ContainerList<ManagedTexture*>* textures,
                                        ContainerList<ManagedTexture*>* placed) {
    while (textures->m_count > 0) {
        ManagedTexture* texture = textures->Get(0);
        if (reservedLeaves[texture->plannedLevel].m_count == 0)
            return 0;
        UnknownTextureRegion* region = reservedLeaves[texture->plannedLevel].Get(0);
        reservedLeaves[texture->plannedLevel].RemoveOrdered(region);
        PlaceTexture(texture, region);
        textures->RemoveOrdered(texture);
        if (placed) {
            placed->Add(texture);
            texture->field_0x98 = 0;
        } else {
            CopyMipLevels(region, 0);
        }
    }
    return 1;
}
