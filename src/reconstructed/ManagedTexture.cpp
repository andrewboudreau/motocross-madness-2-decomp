#include "ManagedTexture.h"

#include "PCRenderTarget.h"
#include "TrackGame.h"

// 0x00510500
ManagedTexture::ManagedTexture(TextureMapManager* manager) : PCTextureMap(manager, 0) {
    page = 0;
    plannedLevel = -1;
    allocatedLevel = -1;
    region = 0;
    pageOffsetV = 0;
    pageOffsetU = 0;
    useCount = 0;
    largestUse = 0;
    lastUseFrame = 0;
    group = 0;
    copiedSkipLevels = 0;
    blitSkipLevels = 0;
    mapFlags |= 1;
    pageScale = 1.0f;
}

// 0x005105a0
ManagedTexture::~ManagedTexture() {
    if (group)
        group->managedTextures.Remove(this);
    if (page)
        ReleasePage();
}

// 0x00510610
void ManagedTexture::UpdatePageCoords(int level) {
    UnknownTextureRegion* placed = region;
    if (placed && page) {
        pageScale = (placed->right - placed->left) / (1 << level);
        pageOffsetU = placed->left;
        pageOffsetV = placed->top;
        allocatedLevel = placed->sizeLevel;
    }
}

// 0x00510670
void ManagedTexture::Reregister() {
    if (group)
        group->managedTextures.Remove(this);
    if (page)
        ReleasePage();
    page = 0;
    region = 0;
    pageOffsetV = 0;
    pageOffsetU = 0;
    pageScale = 1.0f;
    useCount = 0;
    largestUse = 0;
    lastUseFrame = 0;
    plannedLevel = -1;
    allocatedLevel = -1;
    copiedSkipLevels = 0;
    blitSkipLevels = 0;
    group->AddTexture(this);
}

// 0x00510700
void ManagedTexture::ReleasePage() {
    if (page) {
        page->RemoveTexture(this);
        page = 0;
        region = 0;
        pageOffsetV = 0;
        pageOffsetU = 0;
        pageScale = 0;
        allocatedLevel = -1;
    }
}

// 0x00510750
int ManagedTexture::CreateTextureSurface(int a, int b, int c) {
    return 0;
}

// 0x00510760
int ManagedTexture::CreateOwnSurface(int a, int b, int c) {
    return PCTextureMap::CreateTextureSurface(a, b, c);
}

// 0x00510780
void ManagedTexture::RemapTexCoords(float* scale, float* offsetU, float* offsetV, float* u, float* v,
                                           int count, unsigned int stride) {
    unsigned int step = stride / sizeof(float);
    float ratio = pageScale / *scale;
    float du = pageOffsetU - ratio * *offsetU;
    float dv = pageOffsetV - ratio * *offsetV;
    while (count--) {
        *u = ratio * *u + du;
        *v = ratio * *v + dv;
        u += step;
        v += step;
    }
    *scale = pageScale;
    *offsetU = pageOffsetU;
    *offsetV = pageOffsetV;
}

// 0x00510820
void ManagedTexture::RecordUse(float value) {
    if (value > 9.0f)
        value = 9.0f;
    if (value > largestUse)
        largestUse = value;
    useCount++;
    lastUseFrame = ownerManager->frameCount;
}

// 0x00510870: with a page, applies the render states, the colour key and
// the page; otherwise the PCTextureMap behaviour.
void ManagedTexture::Bind() {
    if (page) {
        for (int i = 0; i < statePairCount; i++)
            g_TrackGame->renderTarget->SetRenderState(statePairs[i].state, statePairs[i].value, 0);
        if (usesColorKey && page->textureSurface && page->textureSurface->SetColorKey(8, &colorKeyValue))
            return;
        page->SetAsTexture();
    } else if (textureSurface) {
        PCTextureMap::Bind();
    }
}

// 0x005108f0
int ManagedTexture::SetAsTexture() {
    if (page)
        return page->SetAsTexture();
    if (textureSurface)
        return PCTextureMap::SetAsTexture();
    return 0;
}

// 0x00510910
int ManagedTexture::RemapTexCoordsIfMoved(float* scale, float* offsetU, float* offsetV, float* u, float* v,
                                          int count, unsigned int stride) {
    if (*scale == pageScale && *offsetU == pageOffsetU && *offsetV == pageOffsetV)
        return 0;
    RemapTexCoords(scale, offsetU, offsetV, u, v, count, stride);
    return 1;
}

// 0x00510980
int ManagedTexture::IsResident() {
    return region != 0;
}

// 0x00510990
int ManagedTexture::SizeLevel() {
    int size = textureWidth;
    int level = 0;
    if (size == 1)
        return 0;
    size >>= 1;
    while (size) {
        size >>= 1;
        level++;
    }
    return level;
}

// 0x005109b0
float ManagedTexture::Unhappiness() {
    return largestUse - plannedLevel;
}
