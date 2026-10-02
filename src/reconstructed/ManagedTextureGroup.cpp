#include "TextureMapManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugAlloc.h"
#include "DebugOverlay.h"
#include "ManagedTexture.h"
#include "TrackGame.h"

extern "C" __declspec(dllimport) int __stdcall DeleteObject(void* object);

// cdecl 0x00461d40: formatted output to `file`.
void UnknownFunction461d40(FILE* file, const char* format, ...);

// Texels of a square level-`level` texture (1 for level 0, none below).
static inline int LevelArea(int level) {
    if (level == 0)
        return 1;
    if (level < 0)
        return 0;
    int side = 2 << (level - 1);
    return side * side;
}

// 0x0050bed0
ManagedTextureGroup::ManagedTextureGroup(TextureMapManager* manager, int format, int addressU, int addressV)
    : field_0x1d4(5000), field_0x1e0(5000), field_0x1ec(5000) {
    field_0x10 = addressU;
    field_0x14 = addressV;
    field_0x40 = manager;
    field_0x0c = format;
    field_0x3c = 0;
    field_0x7c.Init(64, 64);
    field_0x144.Init(64, 64);
    field_0x158.Init(16, 16);
    field_0x16c.Init(16, 16);
    field_0x180.Init(16, 16);
    field_0x194.Init(16, 16);
    field_0x1a8.Init(16, 16);
    for (int i = 0; i < 9; i++)
        field_0x90[i].Init(64, 64);
    memset(field_0x18, 0, sizeof(field_0x18));
    field_0x1c4 = 0;
    field_0x1c8 = 0;
    field_0x1cc = 0;
    field_0x1d0 = 0;
    field_0x74 = 0x10000;
    field_0x78 = 6 / UnknownFunction511970(field_0x0c);
    field_0x1f8 = 0;
    field_0x1bc = 0;
    field_0x08 = 0;
    field_0x254 = -1;
    field_0x1c0 = 0;
}

// 0x0050c2e0: leaves the manager and releases the textures and pages.
ManagedTextureGroup::~ManagedTextureGroup() {
    field_0x40->field_0x44.Remove(this);
    TextureMap* texture;
    while ((texture = field_0x44.Last()) != 0) {
        field_0x44.Remove(texture);
        texture->Release();
    }
    while ((texture = field_0x54.Last()) != 0) {
        field_0x54.Remove(texture);
        texture->Release();
    }
    if (field_0x1f8) {
        DeleteObject(field_0x1f8);
        field_0x1f8 = 0;
    }
    if (field_0x1c0)
        field_0x1c0->Release();
}

// 0x0050c4a0: creates the shared 256x256 texture on first use, then adds
// pages sharing its system surface or drops pages from the end.
int ManagedTextureGroup::UnknownFunction50c4a0(int count) {
    int pages = field_0x54.m_count;
    int change = count - pages;
    if (count > 0 && !field_0x1c0) {
        field_0x1c0 = new(__FILE__, 107) PCTextureMap(field_0x40, 1);
        if (!field_0x1c0->UnknownVirtualSlot4(0, 256, 256, 0, 1, field_0x0c, field_0x0c, 0, 2, field_0x1bc, 1, 0,
                                              field_0x10, field_0x14, 0, 0x80, 0xff00ff)) {
            delete field_0x1c0;
            return 0;
        }
    }
    if (change > 0) {
        while (change-- > 0) {
            CacheTexture* page = new(__FILE__, 121) CacheTexture(field_0x40, this, 8);
            page->field_0x70 = field_0x1c0->field_0x70;
            page->field_0x70->UnknownMethod1();
            if (!page->UnknownVirtualSlot4(0, 256, 256, 0, 1, field_0x0c, field_0x0c, 0, 2, field_0x1bc, 1, 0,
                                           field_0x10, field_0x14, 0, 0x80, 0xff00ff)) {
                delete page;
                return 0;
            }
            if (!page->UnknownVirtualSlot8(1, 0, 1)) {
                page->Release();
                return 0;
            }
            field_0x3c += page->field_0x184;
            page->field_0x18 = 256;
            page->field_0x14 = 256;
            field_0x54.Append(page);
        }
    } else if (change < 0) {
        TextureMap* page = field_0x54.Last();
        while (change++ < 0) {
            field_0x3c -= static_cast<CacheTexture*>(page)->field_0x184;
            field_0x54.Remove(page);
            page = field_0x54.Last();
        }
    }
    return 1;
}

// 0x0050c6c0: the first texture of an 8-bit group supplies the pages'
// palette.
void ManagedTextureGroup::UnknownFunction50c6c0(ManagedTexture* texture) {
    if (field_0x0c == 8 && !field_0x1bc) {
        field_0x1bc = texture->field_0x78;
        for (TextureMap* page = field_0x54.First(); page; page = field_0x54.Next()) {
            CacheTexture* cache = static_cast<CacheTexture*>(page);
            cache->field_0x78 = field_0x1bc;
            cache->field_0x70->UnknownMethod31(field_0x1bc);
            cache->field_0x74->UnknownMethod31(field_0x1bc);
            cache->field_0x2c = texture->field_0x2c;
        }
    }
    field_0x44.Append(texture);
    texture->field_0xb8 = field_0x40->field_0x74;
    texture->field_0x90 = this;
}

// 0x0050c760
void ManagedTextureGroup::UnknownFunction50c760() {
    for (TextureMap* texture = field_0x44.First(); texture; texture = field_0x44.Next()) {
        static_cast<ManagedTexture*>(texture)->field_0xa0 = 0;
        static_cast<ManagedTexture*>(texture)->field_0xa4 = 0;
    }
}

// 0x0050c790
void ManagedTextureGroup::UnknownFunction50c790() {
    for (TextureMap* page = field_0x54.First(); page; page = field_0x54.Next()) {
        CacheTexture* cache = static_cast<CacheTexture*>(page);
        if (cache->field_0x74 && cache->field_0x74->UnknownMethod24()) {
            cache->UnknownVirtualSlot10();
            cache->UnknownVirtualSlot8(1, 0, 0);
            cache->UnknownFunction50fc40();
        }
    }
}

// 0x0050c7e0
void ManagedTextureGroup::UnknownFunction50c7e0() {
    FILE* file = fopen("C:\\temp\\TM_debug.txt", "a");
    if (file) {
        UnknownFunction461d40(file, "----------------------------------------------------------------------\n");
        UnknownFunction461d40(file, "Texture Format: %d  Cache Textures: %d  Mgd Textures: %d\n", field_0x0c,
                              field_0x54.m_count, field_0x44.m_count);
        TextureMap* texture = field_0x44.First();
        for (int index = 1; texture; texture = field_0x44.Next()) {
            ManagedTexture* managed = static_cast<ManagedTexture*>(texture);
            UnknownFunction461d40(file, "%03d: %03d x %03d, Max Req'd=%.2f", index++, managed->field_0x14,
                                  managed->field_0x18, managed->field_0xa4);
            if (managed->field_0xa4 > 0.0f)
                UnknownFunction461d40(file, ", Allocd=%d unhappy=%.2f", managed->field_0xa8,
                                      managed->UnknownFunction5109b0());
            UnknownFunction461d40(file, "\n");
        }
        fclose(file);
    }
}

// 0x0050c8c0: unless frozen by the debug key, empties the size lists and
// repacks the pages (through 0x0050dad0 when partial texture blits are on).
void ManagedTextureGroup::UnknownFunction50c8c0() {
    if (g_UnknownGlobal56e26c->field_0x2d4_bit2 && field_0x08)
        return;
    field_0x7c.Clear();
    for (int i = 0; i < 9; i++)
        field_0x90[i].Clear();
    if (g_UnknownGlobal56e26c->field_0x0c->field_0x5bc <= 0)
        UnknownFunction50c960();
    else
        UnknownFunction50dad0();
}

// 0x0050ee70: qsort order of ManagedTexture pointers, by +0xac descending,
// then by address.
int UnknownCompare50ee70(const void* first, const void* second) {
    ManagedTexture* a = *(ManagedTexture**)first;
    ManagedTexture* b = *(ManagedTexture**)second;
    int difference = b->field_0xac - a->field_0xac;
    if (difference)
        return difference;
    return a - b;
}

// 0x0050eeb0: by (+0xa8 - +0xac) descending, then by address.
int UnknownCompare50eeb0(const void* first, const void* second) {
    ManagedTexture* a = *(ManagedTexture**)first;
    ManagedTexture* b = *(ManagedTexture**)second;
    int difference = b->field_0xa8 - b->field_0xac - a->field_0xa8 + a->field_0xac;
    if (difference)
        return difference;
    return a - b;
}

// 0x0050ef00: by 0x005109b0 descending, then by address (descending).
int UnknownCompare50ef00(const void* first, const void* second) {
    ManagedTexture* a = *(ManagedTexture**)first;
    ManagedTexture* b = *(ManagedTexture**)second;
    float difference = -(a->UnknownFunction5109b0() - b->UnknownFunction5109b0());
    if (difference == 0.0f)
        return b - a;
    if (difference > 0.0f)
        return 1;
    return -1;
}


// Where `texture` goes in `list`, kept in descending 0x005109b0 order.
static inline int FindPosition(ContainerList<ManagedTexture*>* list, ManagedTexture* texture) {
    int position = 0;
    for (int j = list->m_count - 1; j >= 0; j--) {
        if (texture->UnknownFunction5109b0() < list->Get(j)->UnknownFunction5109b0()) {
            position = j + 1;
            break;
        }
    }
    return position;
}

// 0x0050d800: sorts `list` by 0x005109b0 and lowers the planned level (+0xac)
// of the least needy texture until the planned texels fit `budget`; a
// texture already at level 5 is dropped (level -1). Returns the texels.
int UnknownFunction50d800(ContainerList<ManagedTexture*>* list, int budget) {
    qsort(list->m_data, list->m_count, sizeof(ManagedTexture*), UnknownCompare50ef00);
    int area = 0;
    for (int i = 0; i < list->m_count; i++)
        area += LevelArea(list->Get(i)->field_0xac);
    while (list->m_count > 0 && area > budget) {
        int index = list->m_count - 1;
        ManagedTexture* texture = list->Get(index);
        while (list->m_count > 0 && texture->field_0xac == 5) {
            if (--index == 0) {
                texture->field_0xac = -1;
                area -= 0x400;
                list->RemoveOrdered(texture);
                index = list->m_count - 1;
            }
            if (index < 0)
                return area;
            texture = list->Get(index);
        }
        int level = texture->field_0xac;
        if (texture->UnknownFunction5109b0() < 0.0f)
            texture->field_0xac = texture->field_0xb0;
        else
            texture->field_0xac--;
        int newLevel = texture->field_0xac;
        area -= LevelArea(level);
        area += LevelArea(newLevel);
        if (list->m_count > 1) {
            list->RemoveOrdered(texture);
            list->Insert(texture, FindPosition(list, texture));
        }
    }
    return area;
}
