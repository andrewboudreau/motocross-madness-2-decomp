#include "TextureMapManager.h"

#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "ManagedTexture.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"

// 0x00510a50
UnknownTextureMapList::UnknownTextureMapList() {
    m_cursor = 0;
    m_tail = 0;
    m_head = 0;
    m_count = 0;
}

// 0x00510a60
UnknownTextureMapList::~UnknownTextureMapList() {
    for (m_cursor = m_head; m_cursor;) {
        TextureMap* next = m_cursor->nextTexture;
        m_cursor->nextTexture = 0;
        m_cursor->previousTexture = 0;
        m_cursor = next;
    }
}

// 0x00510a90
TextureMap* UnknownTextureMapList::First() {
    return m_cursor = m_head;
}

// 0x00510aa0
TextureMap* UnknownTextureMapList::Last() {
    return m_cursor = m_tail;
}

// 0x00510ab0
TextureMap* UnknownTextureMapList::Next() {
    if (m_cursor)
        m_cursor = m_cursor->nextTexture;
    return m_cursor;
}

// 0x00510ad0
TextureMap* UnknownTextureMapList::Previous() {
    if (m_cursor)
        m_cursor = m_cursor->previousTexture;
    return m_cursor;
}

// 0x00510af0
void UnknownTextureMapList::Append(TextureMap* texture) {
    if (!m_head) {
        m_tail = texture;
        m_head = texture;
        texture->previousTexture = 0;
        texture->nextTexture = 0;
        m_count++;
    } else {
        m_tail->nextTexture = texture;
        texture->previousTexture = m_tail;
        texture->nextTexture = 0;
        m_tail = texture;
        m_count++;
    }
}

// 0x00510b30
void UnknownTextureMapList::Remove(TextureMap* texture) {
    if (texture->previousTexture)
        texture->previousTexture->nextTexture = texture->nextTexture;
    if (texture->nextTexture)
        texture->nextTexture->previousTexture = texture->previousTexture;
    if (m_head == texture)
        m_head = texture->nextTexture;
    if (m_tail == texture)
        m_tail = texture->previousTexture;
    m_count--;
}

// 0x00510b70
void UnknownTextureMapList::AppendList(UnknownTextureMapList* other) {
    TextureMap* head = other->First();
    if (head) {
        TextureMap* tail = other->Last();
        int count = other->m_count;
        other->m_tail = 0;
        other->m_head = 0;
        if (!m_head) {
            m_tail = tail;
            m_count += count;
            m_head = head;
        } else {
            m_tail->nextTexture = head;
            head->previousTexture = m_tail;
            m_tail = tail;
            m_count += count;
        }
    }
}

// 0x00510bd0
TextureMapManager::TextureMapManager() : GameObject(1) {
    debugKeyMode = 0;
    scratchBuffer = 0;
    scratchBufferSize = 0;
    secondScratchBuffer = 0;
    secondScratchBufferSize = 0;
    field_0x78 = 0;
    frameCount = 0;
    field_0x44.Init(4, 4);
    debugGroup = 0;
    debugGroupIndex = 0;
    regionAllocator = new(__FILE__, 125) BlockAllocator(0x30, 0x1800);
    memset(g_UnknownSharedMipSurfaces68a36c, 0, sizeof(g_UnknownSharedMipSurfaces68a36c));
    memset(g_UnknownSharedSurfaces68a394, 0, sizeof(g_UnknownSharedSurfaces68a394));
}

// 0x00510cf0: frees the scratch buffers and the block pool and releases the
// shared texture surfaces.
TextureMapManager::~TextureMapManager() {
    if (scratchBuffer)
        DebugFree(scratchBuffer, __FILE__, 143);
    if (secondScratchBuffer)
        DebugFree(secondScratchBuffer, __FILE__, 144);
    if (regionAllocator)
        delete regionAllocator;
    for (int i = 0; i < 10; i++) {
        if (g_UnknownSharedSurfaces68a394[i])
            g_UnknownSharedSurfaces68a394[i]->Release();
        if (g_UnknownSharedMipSurfaces68a36c[i])
            g_UnknownSharedMipSurfaces68a36c[i]->Release();
    }
}

// 0x00510dd0
int TextureMapManager::Tick(float frameTime) {
    frameCount++;
    return GameObject::Tick(frameTime);
}

// 0x00510df0
int TextureMapManager::PrepareGeometry() {
    for (int i = 0; i < field_0x44.m_count; i++) {
        ManagedTextureGroup* cache = field_0x44.Get(i);
        if (cache)
            cache->ClearUseRecords();
    }
    return GameObject::PrepareGeometry();
}

// 0x00510e30: nothing to do with AGP textures.
int TextureMapManager::RenderPre3D() {
    if (g_TrackGame->display->isAGP)
        return 1;
    for (int i = 0; i < field_0x44.m_count; i++) {
        ManagedTextureGroup* cache = field_0x44.Get(i);
        if (cache)
            cache->RepackPages();
    }
    return GameObject::RenderPre3D();
}

// 0x00510e90
int TextureMapManager::RenderPost3D() {
    if (g_TrackGame->testKey) {
        for (int i = 0; i < field_0x44.m_count; i++) {
            ManagedTextureGroup* cache = field_0x44.Get(i);
            if (cache)
                cache->DrawSelection(static_cast<PCRenderTarget*>(renderContext));
        }
    }
    return GameObject::RenderPost3D();
}

// 0x00510ee0: with the "TestKey" debug input, key 0x14 cycles the mode,
// 0x21 toggles the selected cache's +0x08, 0x1b / 0x1a step forwards /
// backwards (through the cache's +0x1c8 or +0x1c4 entries, or through the
// caches) and 0x15 runs the cache's 0x0050c7e0.
int TextureMapManager::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int handled = 0;
    if (GameObject::OnControlPress(event, entry))
        return 1;
    if (g_TrackGame->testKey && debugGroup) {
        handled = 1;
        if (TestInputEvent(0x14, 0, event, 0x80)) {
            if (++debugKeyMode > 2)
                debugKeyMode = 0;
            if (!debugGroup->managedTextures.m_count && debugKeyMode == 1)
                debugKeyMode = 2;
            if (!debugGroup->cachePages.m_count && debugKeyMode == 2)
                debugKeyMode = 0;
        } else if (TestInputEvent(0x21, 0, event, 0x80)) {
            debugGroup->noRepacking ^= 1;
        } else if (TestInputEvent(0x1b, 0, event, 0x80)) {
            if (debugKeyMode == 1) {
                if (++debugGroup->textureCursor >= debugGroup->managedTextures.m_count)
                    debugGroup->textureCursor = 0;
            } else if (debugKeyMode == 2) {
                if (++debugGroup->pageCursor >= debugGroup->cachePages.m_count)
                    debugGroup->pageCursor = 0;
            } else if (debugKeyMode == 0) {
                if (++debugGroupIndex >= field_0x44.m_count)
                    debugGroupIndex = 0;
                debugGroup = field_0x44.Get(debugGroupIndex);
            }
        } else if (TestInputEvent(0x1a, 0, event, 0x80)) {
            if (debugKeyMode == 1) {
                if (--debugGroup->textureCursor < 0)
                    debugGroup->textureCursor = debugGroup->managedTextures.m_count - 1;
            } else if (debugKeyMode == 2) {
                if (--debugGroup->pageCursor < 0)
                    debugGroup->pageCursor = debugGroup->cachePages.m_count - 1;
            } else if (debugKeyMode == 0) {
                if (--debugGroupIndex < 0)
                    debugGroupIndex = field_0x44.m_count;
                debugGroup = field_0x44.Get(debugGroupIndex);
            }
        } else if (TestInputEvent(0x15, 0, event, 0x80)) {
            if (debugGroup)
                debugGroup->WriteDebugReport();
        } else {
            handled = 0;
        }
    }
    return handled;
}

// 0x00511180
ManagedTextureGroup* TextureMapManager::CreateGroup(int format, int pages, int addressU, int addressV) {
    ManagedTextureGroup* cache = new(__FILE__, 431) ManagedTextureGroup(this, format, addressU, addressV);
    cache->SetPageCount(pages);
    field_0x44.Add(cache);
    if (!debugGroup)
        debugGroup = cache;
    return cache;
}

// 0x00511290
int TextureMapManager::OnActivate() {
    for (TextureMap* texture = allTextures.First(); texture; texture = allTextures.Next())
        texture->RecreateIfLost();
    for (int i = 0; i < field_0x44.m_count; i++) {
        ManagedTextureGroup* cache = field_0x44.Get(i);
        if (cache)
            cache->RestoreLostPages();
    }
    return GameObject::OnActivate();
}

// 0x005113d0: on AGP displays, restores every grouped texture; otherwise
// hands out the "TextureCacheLimit" in 0x2aaaa-byte steps, each to the group
// with the fewest steps for its texture count.
int TextureMapManager::ShareCacheLimit() {
    int count = field_0x44.m_count;
    int* steps = new(__FILE__, 531) int[count];
    int* sizes = new(__FILE__, 532) int[count];
    int i;
    for (i = 0; i < count; i++) {
        ManagedTextureGroup* group = field_0x44.Get(i);
        steps[i] = 0;
        sizes[i] = group->managedTextures.m_count;
        if (!sizes[i])
            sizes[i] = 1;
    }
    if (g_TrackGame->display->isAGP) {
        for (i = 0; i < count; i++) {
            UnknownTextureMapList* textures = &field_0x44.Get(i)->managedTextures;
            for (TextureMap* texture = textures->First(); texture; texture = textures->Next())
                static_cast<ManagedTexture*>(texture)->CreateOwnSurface(1, 0, 0);
        }
    } else {
        int total = 0;
        for (;;) {
            int best = -1;
            float bestShare = 1.0f;
            for (i = 0; i < count; i++) {
                float share = (float)steps[i] / sizes[i];
                if (share < bestShare) {
                    bestShare = share;
                    best = i;
                }
            }
            if (best == -1)
                break;
            total += 0x2aaaa;
            if (total > g_TrackGame->display->textureCacheLimit)
                break;
            if (!field_0x44.Get(best)->SetPageCount(steps[best] + 1))
                break;
            steps[best]++;
        }
    }
    delete steps;
    delete sizes;
    return 1;
}

// 0x00511580: counts the 32-256 pixel textures and their bytes (with mip
// levels), and the display's figure for each kind of texture, and formats
// them and the card's reported texture memory (the text is not output).
void TextureMapManager::CountTextureMemory() {
    char text[128];
    UnknownSurfaceCaps caps;
    unsigned long totalMemory;
    unsigned long freeMemory;
    TextureMap* texture = allTextures.First();
    memset(&caps, 0, sizeof(caps));
    caps.caps = 0x1000;
    int count32 = 0;
    int count64 = 0;
    int count128 = 0;
    int count256 = 0;
    int cost = 0;
    g_TrackGame->display->directDraw->GetAvailableVidMem(&caps, &totalMemory, &freeMemory);
    int bytes = 0;
    for (; texture; texture = allTextures.Next()) {
        int kind = 0;
        if (static_cast<PCTextureMap*>(texture)->textureSurface) {
            if (texture->textureWidth == 32) {
                count32++;
                kind |= 6;
            }
            if (texture->textureWidth == 64) {
                count64++;
                kind |= 4;
            }
            if (texture->textureWidth == 128) {
                count128++;
                kind |= 2;
            }
            if (texture->textureWidth == 256)
                count256++;
            kind |= texture->textureWidth > texture->minimumMipSize;
            kind |= PixelFormatBytesPerPixel(texture->mapFormat) == 1 ? 8 : 0;
            int pixels = 0;
            for (int width = texture->textureWidth, height = texture->textureHeight; width >= texture->minimumMipSize;
                 width /= 2, height /= 2)
                pixels += height * width;
            bytes += PixelFormatBytesPerPixel(texture->mapFormat) * pixels;
            cost += g_TrackGame->display->field_0x14[kind];
        }
    }
    sprintf(text, "\nTexture Memory 32x32 = %d, 64x64 = %d, 128x128 = %d, 256x256 = %d, total = %d \n", count32,
            count64, count128, count256, bytes);
    sprintf(text, "Card reports - totalTextureMemory = %d, freeTextureMemory = %d \n ", totalMemory, freeMemory);
}

// 0x005112f0
void TextureMapManager::RegisterTexture(TextureMap* texture) {
    allTextures.Append(texture);
}

// 0x00511300
void TextureMapManager::UnregisterTexture(TextureMap* texture) {
    allTextures.Remove(texture);
}

// 0x00511310
void* TextureMapManager::GrowScratchBuffer(unsigned int bytes) {
    if (scratchBufferSize < bytes) {
        if (scratchBuffer) {
            DebugFree(scratchBuffer, __FILE__, 497);
            scratchBuffer = 0;
        }
        scratchBuffer = DebugMalloc(bytes, __FILE__, 500);
        if (!scratchBuffer)
            return 0;
        scratchBufferSize = bytes;
    }
    return scratchBuffer;
}

// 0x00511370
void* TextureMapManager::GrowSecondScratchBuffer(unsigned int bytes) {
    if (secondScratchBufferSize < bytes) {
        if (secondScratchBuffer) {
            DebugFree(secondScratchBuffer, __FILE__, 512);
            secondScratchBuffer = 0;
        }
        secondScratchBuffer = DebugMalloc(bytes, __FILE__, 515);
        if (!secondScratchBuffer)
            return 0;
        secondScratchBufferSize = bytes;
    }
    return secondScratchBuffer;
}
