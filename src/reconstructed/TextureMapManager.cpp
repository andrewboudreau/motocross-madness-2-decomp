#include "TextureMapManager.h"

#include <string.h>

#include "DebugAlloc.h"
#include "PCTextureMap.h"
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
        UnknownTextureCache* cache = field_0x44.Get(i);
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
        UnknownTextureCache* cache = field_0x44.Get(i);
        if (cache)
            cache->UnknownFunction50c8c0();
    }
    return GameObject::UnknownVirtualSlot13();
}

// 0x00510e90
int TextureMapManager::UnknownVirtualSlot15() {
    if (g_UnknownGlobal56e26c->field_0x2d4_bit2) {
        for (int i = 0; i < field_0x44.m_count; i++) {
            UnknownTextureCache* cache = field_0x44.Get(i);
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
            if (!field_0x3c->field_0x50 && field_0x58 == 1)
                field_0x58 = 2;
            if (!field_0x3c->field_0x60 && field_0x58 == 2)
                field_0x58 = 0;
        } else if (UnknownFunction43caa0(0x21, 0, event, 0x80)) {
            field_0x3c->field_0x08 ^= 1;
        } else if (UnknownFunction43caa0(0x1b, 0, event, 0x80)) {
            if (field_0x58 == 1) {
                if (++field_0x3c->field_0x1c8 >= field_0x3c->field_0x50)
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
                    field_0x3c->field_0x1c8 = field_0x3c->field_0x50 - 1;
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
UnknownTextureCache* TextureMapManager::UnknownFunction511180(int a, int b, int c, int d) {
    UnknownTextureCache* cache = new(__FILE__, 431) UnknownTextureCache(this, a, c, d);
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
        UnknownTextureCache* cache = field_0x44.Get(i);
        if (cache)
            cache->UnknownFunction50c790();
    }
    return GameObject::UnknownVirtualSlot18();
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
