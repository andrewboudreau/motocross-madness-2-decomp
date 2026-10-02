#include "TextureMapManager.h"

#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "ManagedTexture.h"
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
        TextureMap* next = m_cursor->field_0x08;
        m_cursor->field_0x08 = 0;
        m_cursor->field_0x0c = 0;
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
        m_cursor = m_cursor->field_0x08;
    return m_cursor;
}

// 0x00510ad0
TextureMap* UnknownTextureMapList::Previous() {
    if (m_cursor)
        m_cursor = m_cursor->field_0x0c;
    return m_cursor;
}

// 0x00510af0
void UnknownTextureMapList::Append(TextureMap* texture) {
    if (!m_head) {
        m_tail = texture;
        m_head = texture;
        texture->field_0x0c = 0;
        texture->field_0x08 = 0;
        m_count++;
    } else {
        m_tail->field_0x08 = texture;
        texture->field_0x0c = m_tail;
        texture->field_0x08 = 0;
        m_tail = texture;
        m_count++;
    }
}

// 0x00510b30
void UnknownTextureMapList::Remove(TextureMap* texture) {
    if (texture->field_0x0c)
        texture->field_0x0c->field_0x08 = texture->field_0x08;
    if (texture->field_0x08)
        texture->field_0x08->field_0x0c = texture->field_0x0c;
    if (m_head == texture)
        m_head = texture->field_0x08;
    if (m_tail == texture)
        m_tail = texture->field_0x0c;
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
            m_tail->field_0x08 = head;
            head->field_0x0c = m_tail;
            m_tail = tail;
            m_count += count;
        }
    }
}

// 0x00510bd0
TextureMapManager::TextureMapManager() : GameObject(1) {
    field_0x58 = 0;
    field_0x5c = 0;
    field_0x64 = 0;
    field_0x60 = 0;
    field_0x68 = 0;
    field_0x78 = 0;
    field_0x74 = 0;
    field_0x44.Init(4, 4);
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x70 = new(__FILE__, 125) UnknownBlockAllocator(0x30, 0x1800);
    memset(g_UnknownSharedMipSurfaces68a36c, 0, sizeof(g_UnknownSharedMipSurfaces68a36c));
    memset(g_UnknownSharedSurfaces68a394, 0, sizeof(g_UnknownSharedSurfaces68a394));
}

// 0x00510cf0: frees the scratch buffers and the block pool and releases the
// shared texture surfaces.
TextureMapManager::~TextureMapManager() {
    if (field_0x5c)
        operator delete(field_0x5c, __FILE__, 143);
    if (field_0x60)
        operator delete(field_0x60, __FILE__, 144);
    if (field_0x70)
        delete field_0x70;
    for (int i = 0; i < 10; i++) {
        if (g_UnknownSharedSurfaces68a394[i])
            g_UnknownSharedSurfaces68a394[i]->UnknownMethod2();
        if (g_UnknownSharedMipSurfaces68a36c[i])
            g_UnknownSharedMipSurfaces68a36c[i]->UnknownMethod2();
    }
}

// 0x00510dd0
int TextureMapManager::UnknownVirtualSlot10(float frameTime) {
    field_0x74++;
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x00510df0
int TextureMapManager::UnknownVirtualSlot12() {
    for (int i = 0; i < field_0x44.m_count; i++) {
        ManagedTextureGroup* cache = field_0x44.Get(i);
        if (cache)
            cache->UnknownFunction50c760();
    }
    return GameObject::UnknownVirtualSlot12();
}

// 0x00510e30: nothing to do with AGP textures.
int TextureMapManager::UnknownVirtualSlot13() {
    if (g_UnknownGlobal56e26c->field_0x0c->field_0x9f0)
        return 1;
    for (int i = 0; i < field_0x44.m_count; i++) {
        ManagedTextureGroup* cache = field_0x44.Get(i);
        if (cache)
            cache->UnknownFunction50c8c0();
    }
    return GameObject::UnknownVirtualSlot13();
}

// 0x00510e90
int TextureMapManager::UnknownVirtualSlot15() {
    if (g_UnknownGlobal56e26c->field_0x2d4_bit2) {
        for (int i = 0; i < field_0x44.m_count; i++) {
            ManagedTextureGroup* cache = field_0x44.Get(i);
            if (cache)
                cache->UnknownFunction50ef70(field_0x18);
        }
    }
    return GameObject::UnknownVirtualSlot15();
}

// 0x00510ee0: with the "TestKey" debug input, key 0x14 cycles the mode,
// 0x21 toggles the selected cache's +0x08, 0x1b / 0x1a step forwards /
// backwards (through the cache's +0x1c8 or +0x1c4 entries, or through the
// caches) and 0x15 runs the cache's 0x0050c7e0.
int TextureMapManager::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int handled = 0;
    if (GameObject::UnknownVirtualSlot23(event, entry))
        return 1;
    if (g_UnknownGlobal56e26c->field_0x2d4_bit2 && field_0x3c) {
        handled = 1;
        if (UnknownFunction43caa0(0x14, 0, event, 0x80)) {
            if (++field_0x58 > 2)
                field_0x58 = 0;
            if (!field_0x3c->field_0x44.m_count && field_0x58 == 1)
                field_0x58 = 2;
            if (!field_0x3c->field_0x60 && field_0x58 == 2)
                field_0x58 = 0;
        } else if (UnknownFunction43caa0(0x21, 0, event, 0x80)) {
            field_0x3c->field_0x08 ^= 1;
        } else if (UnknownFunction43caa0(0x1b, 0, event, 0x80)) {
            if (field_0x58 == 1) {
                if (++field_0x3c->field_0x1c8 >= field_0x3c->field_0x44.m_count)
                    field_0x3c->field_0x1c8 = 0;
            } else if (field_0x58 == 2) {
                if (++field_0x3c->field_0x1c4 >= field_0x3c->field_0x60)
                    field_0x3c->field_0x1c4 = 0;
            } else if (field_0x58 == 0) {
                if (++field_0x40 >= field_0x44.m_count)
                    field_0x40 = 0;
                field_0x3c = field_0x44.Get(field_0x40);
            }
        } else if (UnknownFunction43caa0(0x1a, 0, event, 0x80)) {
            if (field_0x58 == 1) {
                if (--field_0x3c->field_0x1c8 < 0)
                    field_0x3c->field_0x1c8 = field_0x3c->field_0x44.m_count - 1;
            } else if (field_0x58 == 2) {
                if (--field_0x3c->field_0x1c4 < 0)
                    field_0x3c->field_0x1c4 = field_0x3c->field_0x60 - 1;
            } else if (field_0x58 == 0) {
                if (--field_0x40 < 0)
                    field_0x40 = field_0x44.m_count;
                field_0x3c = field_0x44.Get(field_0x40);
            }
        } else if (UnknownFunction43caa0(0x15, 0, event, 0x80)) {
            if (field_0x3c)
                field_0x3c->UnknownFunction50c7e0();
        } else {
            handled = 0;
        }
    }
    return handled;
}

// 0x00511180
ManagedTextureGroup* TextureMapManager::UnknownFunction511180(int a, int b, int c, int d) {
    ManagedTextureGroup* cache = new(__FILE__, 431) ManagedTextureGroup(this, a, c, d);
    cache->UnknownFunction50c4a0(b);
    field_0x44.Add(cache);
    if (!field_0x3c)
        field_0x3c = cache;
    return cache;
}

// 0x00511290
int TextureMapManager::UnknownVirtualSlot18() {
    for (TextureMap* texture = field_0x2c.First(); texture; texture = field_0x2c.Next())
        texture->UnknownVirtualSlot12();
    for (int i = 0; i < field_0x44.m_count; i++) {
        ManagedTextureGroup* cache = field_0x44.Get(i);
        if (cache)
            cache->UnknownFunction50c790();
    }
    return GameObject::UnknownVirtualSlot18();
}

// 0x005113d0: on AGP displays, restores every grouped texture; otherwise
// hands out the "TextureCacheLimit" in 0x2aaaa-byte steps, each to the group
// with the fewest steps for its texture count.
int TextureMapManager::UnknownFunction5113d0() {
    int count = field_0x44.m_count;
    int* steps = new(__FILE__, 531) int[count];
    int* sizes = new(__FILE__, 532) int[count];
    int i;
    for (i = 0; i < count; i++) {
        ManagedTextureGroup* group = field_0x44.Get(i);
        steps[i] = 0;
        sizes[i] = group->field_0x44.m_count;
        if (!sizes[i])
            sizes[i] = 1;
    }
    if (g_UnknownGlobal56e26c->field_0x0c->field_0x9f0) {
        for (i = 0; i < count; i++) {
            UnknownTextureMapList* textures = &field_0x44.Get(i)->field_0x44;
            for (TextureMap* texture = textures->First(); texture; texture = textures->Next())
                static_cast<ManagedTexture*>(texture)->UnknownFunction510760(1, 0, 0);
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
            if (total > g_UnknownGlobal56e26c->field_0x0c->field_0x60)
                break;
            if (!field_0x44.Get(best)->UnknownFunction50c4a0(steps[best] + 1))
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
void TextureMapManager::UnknownFunction511580() {
    char text[128];
    UnknownSurfaceCaps caps;
    unsigned long totalMemory;
    unsigned long freeMemory;
    TextureMap* texture = field_0x2c.First();
    memset(&caps, 0, sizeof(caps));
    caps.caps = 0x1000;
    int count32 = 0;
    int count64 = 0;
    int count128 = 0;
    int count256 = 0;
    int cost = 0;
    g_UnknownGlobal56e26c->field_0x0c->field_0x190->UnknownMethod23(&caps, &totalMemory, &freeMemory);
    int bytes = 0;
    for (; texture; texture = field_0x2c.Next()) {
        int kind = 0;
        if (static_cast<PCTextureMap*>(texture)->field_0x74) {
            if (texture->field_0x14 == 32) {
                count32++;
                kind |= 6;
            }
            if (texture->field_0x14 == 64) {
                count64++;
                kind |= 4;
            }
            if (texture->field_0x14 == 128) {
                count128++;
                kind |= 2;
            }
            if (texture->field_0x14 == 256)
                count256++;
            kind |= texture->field_0x14 > texture->field_0x1c;
            kind |= UnknownFunction511970(texture->field_0x20) == 1 ? 8 : 0;
            int pixels = 0;
            for (int width = texture->field_0x14, height = texture->field_0x18; width >= texture->field_0x1c;
                 width /= 2, height /= 2)
                pixels += height * width;
            bytes += UnknownFunction511970(texture->field_0x20) * pixels;
            cost += g_UnknownGlobal56e26c->field_0x0c->field_0x14[kind];
        }
    }
    sprintf(text, "\nTexture Memory 32x32 = %d, 64x64 = %d, 128x128 = %d, 256x256 = %d, total = %d \n", count32,
            count64, count128, count256, bytes);
    sprintf(text, "Card reports - totalTextureMemory = %d, freeTextureMemory = %d \n ", totalMemory, freeMemory);
}

// 0x005112f0
void TextureMapManager::UnknownFunction5112f0(TextureMap* texture) {
    field_0x2c.Append(texture);
}

// 0x00511300
void TextureMapManager::UnknownFunction511300(TextureMap* texture) {
    field_0x2c.Remove(texture);
}

// 0x00511310
void* TextureMapManager::UnknownFunction511310(unsigned int bytes) {
    if (field_0x64 < bytes) {
        if (field_0x5c) {
            operator delete(field_0x5c, __FILE__, 497);
            field_0x5c = 0;
        }
        field_0x5c = DebugMalloc(bytes, __FILE__, 500);
        if (!field_0x5c)
            return 0;
        field_0x64 = bytes;
    }
    return field_0x5c;
}

// 0x00511370
void* TextureMapManager::UnknownFunction511370(unsigned int bytes) {
    if (field_0x68 < bytes) {
        if (field_0x60) {
            operator delete(field_0x60, __FILE__, 512);
            field_0x60 = 0;
        }
        field_0x60 = DebugMalloc(bytes, __FILE__, 515);
        if (!field_0x60)
            return 0;
        field_0x68 = bytes;
    }
    return field_0x60;
}
