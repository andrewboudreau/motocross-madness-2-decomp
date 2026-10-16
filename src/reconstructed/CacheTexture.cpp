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
    rootRegion = (UnknownTextureRegion*)ownerManager->regionAllocator->Alloc();
    rootRegion->Init(0, level);
    rootRegion->left = rootRegion->top = 0.0f;
    rootRegion->right = rootRegion->bottom = 1.0f;
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
    if (region->quarters[0]) {
        int count = 0;
        for (int i = 0; i < 4; i++)
            count += CollectOccupants(textures, region->quarters[i]);
        return count;
    }
    ManagedTexture* texture = region->occupant;
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
            region = steps[depth].region->quarters[steps[depth].index];
        } else {
            first = 0;
        }
        while (region->sizeLevel > level) {
            depth++;
            steps[depth].region = region;
            steps[depth].index = 0;
            region = FirstQuarter(region);
        }
        FreeQuarters(region);
        if (region->reserved)
            continue;
        region->reserved = 1;
        reservedLeafCounts[level]++;
        freeLeafCounts[level]--;
        reservedLeaves[level].Add(region);
    }
    for (; depth >= 0; depth--) {
        UnknownTextureRegion* parent = steps[depth].region;
        int i = steps[depth].index + 1;
        for (; i < 4; i++) {
            UnknownTextureRegion* quarter = parent->quarters[i];
            if (quarter->quarters[0])
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
    if (root->quarters[0])
        FreeQuarters(root);
    else
        EvictRegion(root);
}

// 0x0050fc60
UnknownTextureRegion* CacheTexture::FirstQuarter(UnknownTextureRegion* region) {
    UnknownTextureRegion* first = region->quarters[0];
    if (!first) {
        EvictRegion(region);
        first = SplitRegion(region);
    }
    return first;
}

// 0x0050fc90
void CacheTexture::EvictRegion(UnknownTextureRegion* region) {
    if (region->occupant) {
        region->occupant->page = 0;
        region->occupant->region = 0;
        region->occupant->allocatedLevel = -1;
        region->occupant = 0;
    }
    if (region->reserved) {
        reservedLeafCounts[region->sizeLevel]--;
        reservedLeaves[region->sizeLevel].RemoveOrdered(region);
        region->reserved = 0;
        freeLeafCounts[region->sizeLevel]++;
    }
}

// 0x0050fd60
void CacheTexture::FreeQuarters(UnknownTextureRegion* region) {
    if (!region->quarters[0])
        return;
    for (int i = 0; i < 4; i++) {
        UnknownTextureRegion* quarter = region->quarters[i];
        if (quarter) {
            if (quarter->quarters[0])
                FreeQuarters(quarter);
            else
                EvictRegion(quarter);
            ownerManager->regionAllocator->Free(region->quarters[i]);
            region->quarters[i] = 0;
        }
    }
}

// 0x0050ffa0
UnknownTextureRegion* CacheTexture::SplitRegion(UnknownTextureRegion* region) {
    float left = region->left;
    float right = region->right;
    float top = region->top;
    float bottom = region->bottom;
    float middleU = (region->right - region->left) * 0.5f + left;
    float middleV = (region->bottom - region->top) * 0.5f + top;
    for (int i = 0; i < 4; i++) {
        region->quarters[i] = (UnknownTextureRegion*)ownerManager->regionAllocator->Alloc();
        region->quarters[i]->Init(region, region->sizeLevel - 1);
    }
    region->quarters[0]->left = left;
    region->quarters[0]->top = top;
    region->quarters[0]->right = middleU;
    region->quarters[0]->bottom = middleV;
    region->quarters[1]->left = middleU;
    region->quarters[1]->top = top;
    region->quarters[1]->right = right;
    region->quarters[1]->bottom = middleV;
    region->quarters[2]->left = left;
    region->quarters[2]->top = middleV;
    region->quarters[2]->right = middleU;
    region->quarters[2]->bottom = bottom;
    region->quarters[3]->left = middleU;
    region->quarters[3]->top = middleV;
    region->quarters[3]->right = right;
    region->quarters[3]->bottom = bottom;
    freeLeafCounts[region->sizeLevel]--;
    freeLeafCounts[region->sizeLevel - 1] += 4;
    return region->quarters[0];
}

// 0x00510120
void CacheTexture::ReleaseRegion(UnknownTextureRegion* region, int merge) {
    if (region->occupant) {
        region->occupant->page = 0;
        region->occupant->region = 0;
        region->occupant = 0;
    }
    if (region->reserved) {
        reservedLeafCounts[region->sizeLevel]--;
        reservedLeaves[region->sizeLevel].RemoveOrdered(region);
        region->reserved = 0;
        freeLeafCounts[region->sizeLevel]++;
    }
    if (!merge)
        return;
    for (UnknownTextureRegion* parent = region->parentRegion; parent; parent = parent->parentRegion) {
        int i;
        for (i = 0; i < 4; i++) {
            if (parent->quarters[i]->quarters[0] || parent->quarters[i]->reserved)
                return;
        }
        for (i = 0; i < 4; i++) {
            ownerManager->regionAllocator->Free(parent->quarters[i]);
            parent->quarters[i] = 0;
        }
        freeLeafCounts[parent->sizeLevel - 1] -= 4;
        freeLeafCounts[parent->sizeLevel]++;
    }
}

// 0x00510250
int CacheTexture::PlaceTexture(ManagedTexture* texture, UnknownTextureRegion* region) {
    region->occupant = texture;
    texture->region = region;
    region->occupant->page = this;
    region->reserved = 1;
    freeLeafCounts[texture->plannedLevel]--;
    texture->allocatedLevel = region->sizeLevel;
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
