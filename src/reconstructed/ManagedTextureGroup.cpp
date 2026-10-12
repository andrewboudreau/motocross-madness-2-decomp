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
void FilePrintf(FILE* file, const char* format, ...);

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
    groupAddressU = addressU;
    groupAddressV = addressV;
    groupManager = manager;
    groupFormat = format;
    pageMemory = 0;
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
    pageCursor = 0;
    textureCursor = 0;
    field_0x1cc = 0;
    field_0x1d0 = 0;
    field_0x74 = 0x10000;
    field_0x78 = 6 / PixelFormatBytesPerPixel(groupFormat);
    gdiObject = 0;
    groupPalette = 0;
    noRepacking = 0;
    field_0x254 = -1;
    surfaceTexture = 0;
}

// 0x0050c2e0: leaves the manager and releases the textures and pages.
ManagedTextureGroup::~ManagedTextureGroup() {
    groupManager->field_0x44.Remove(this);
    TextureMap* texture;
    while ((texture = managedTextures.Last()) != 0) {
        managedTextures.Remove(texture);
        texture->Release();
    }
    while ((texture = cachePages.Last()) != 0) {
        cachePages.Remove(texture);
        texture->Release();
    }
    if (gdiObject) {
        DeleteObject(gdiObject);
        gdiObject = 0;
    }
    if (surfaceTexture)
        surfaceTexture->Release();
}

// 0x0050c4a0: creates the shared 256x256 texture on first use, then adds
// pages sharing its system surface or drops pages from the end.
int ManagedTextureGroup::SetPageCount(int count) {
    int pages = cachePages.m_count;
    int change = count - pages;
    if (count > 0 && !surfaceTexture) {
        surfaceTexture = new(__FILE__, 107) PCTextureMap(groupManager, 1);
        if (!surfaceTexture->UnknownVirtualSlot4(0, 256, 256, 0, 1, groupFormat, groupFormat, 0, 2, groupPalette, 1, 0,
                                              groupAddressU, groupAddressV, 0, 0x80, 0xff00ff)) {
            delete surfaceTexture;
            return 0;
        }
    }
    if (change > 0) {
        while (change-- > 0) {
            CacheTexture* page = new(__FILE__, 121) CacheTexture(groupManager, this, 8);
            page->systemSurface = surfaceTexture->systemSurface;
            page->systemSurface->AddRef();
            if (!page->UnknownVirtualSlot4(0, 256, 256, 0, 1, groupFormat, groupFormat, 0, 2, groupPalette, 1, 0,
                                           groupAddressU, groupAddressV, 0, 0x80, 0xff00ff)) {
                delete page;
                return 0;
            }
            if (!page->UnknownVirtualSlot8(1, 0, 1)) {
                page->Release();
                return 0;
            }
            pageMemory += page->texelCount;
            page->textureHeight = 256;
            page->textureWidth = 256;
            cachePages.Append(page);
        }
    } else if (change < 0) {
        TextureMap* page = cachePages.Last();
        while (change++ < 0) {
            pageMemory -= static_cast<CacheTexture*>(page)->texelCount;
            cachePages.Remove(page);
            page = cachePages.Last();
        }
    }
    return 1;
}

// 0x0050c6c0: the first texture of an 8-bit group supplies the pages'
// palette.
void ManagedTextureGroup::AddTexture(ManagedTexture* texture) {
    if (groupFormat == 8 && !groupPalette) {
        groupPalette = texture->texturePalette;
        for (TextureMap* page = cachePages.First(); page; page = cachePages.Next()) {
            CacheTexture* cache = static_cast<CacheTexture*>(page);
            cache->texturePalette = groupPalette;
            cache->systemSurface->SetPalette(groupPalette);
            cache->textureSurface->SetPalette(groupPalette);
            cache->field_0x2c = texture->field_0x2c;
        }
    }
    managedTextures.Append(texture);
    texture->lastUseFrame = groupManager->frameCount;
    texture->group = this;
}

// 0x0050c760
void ManagedTextureGroup::ClearUseRecords() {
    for (TextureMap* texture = managedTextures.First(); texture; texture = managedTextures.Next()) {
        static_cast<ManagedTexture*>(texture)->useCount = 0;
        static_cast<ManagedTexture*>(texture)->largestUse = 0;
    }
}

// 0x0050c790
void ManagedTextureGroup::RestoreLostPages() {
    for (TextureMap* page = cachePages.First(); page; page = cachePages.Next()) {
        CacheTexture* cache = static_cast<CacheTexture*>(page);
        if (cache->textureSurface && cache->textureSurface->IsLost()) {
            cache->UnknownVirtualSlot10();
            cache->UnknownVirtualSlot8(1, 0, 0);
            cache->EmptyPage();
        }
    }
}

// 0x0050c7e0
void ManagedTextureGroup::WriteDebugReport() {
    FILE* file = fopen("C:\\temp\\TM_debug.txt", "a");
    if (file) {
        FilePrintf(file, "----------------------------------------------------------------------\n");
        FilePrintf(file, "Texture Format: %d  Cache Textures: %d  Mgd Textures: %d\n", groupFormat,
                              cachePages.m_count, managedTextures.m_count);
        TextureMap* texture = managedTextures.First();
        for (int index = 1; texture; texture = managedTextures.Next()) {
            ManagedTexture* managed = static_cast<ManagedTexture*>(texture);
            FilePrintf(file, "%03d: %03d x %03d, Max Req'd=%.2f", index++, managed->textureWidth,
                                  managed->textureHeight, managed->largestUse);
            if (managed->largestUse > 0.0f)
                FilePrintf(file, ", Allocd=%d unhappy=%.2f", managed->field_0xa8,
                                      managed->UnknownFunction5109b0());
            FilePrintf(file, "\n");
        }
        fclose(file);
    }
}

// 0x0050c8c0: unless frozen by the debug key, empties the size lists and
// repacks the pages (through 0x0050dad0 when partial texture blits are on).
void ManagedTextureGroup::RepackPages() {
    if (g_TrackGame->field_0x2d4_bit2 && noRepacking)
        return;
    field_0x7c.Clear();
    for (int i = 0; i < 9; i++)
        field_0x90[i].Clear();
    if (g_TrackGame->display->partialTextureUploadResult <= 0)
        RepackWholePages();
    else
        RepackWithPartialBlits();
}

// 0x0050ee70: qsort order of ManagedTexture pointers, by +0xac descending,
// then by address.
int UnknownCompare50ee70(const void* first, const void* second) {
    ManagedTexture* a = *(ManagedTexture**)first;
    ManagedTexture* b = *(ManagedTexture**)second;
    int difference = b->plannedLevel - a->plannedLevel;
    if (difference)
        return difference;
    return a - b;
}

// 0x0050eeb0: by (+0xa8 - +0xac) descending, then by address.
int UnknownCompare50eeb0(const void* first, const void* second) {
    ManagedTexture* a = *(ManagedTexture**)first;
    ManagedTexture* b = *(ManagedTexture**)second;
    int difference = b->field_0xa8 - b->plannedLevel - a->field_0xa8 + a->plannedLevel;
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
int FitTexturesToBudget(ContainerList<ManagedTexture*>* list, int budget) {
    qsort(list->m_data, list->m_count, sizeof(ManagedTexture*), UnknownCompare50ef00);
    int area = 0;
    for (int i = 0; i < list->m_count; i++)
        area += LevelArea(list->Get(i)->plannedLevel);
    while (list->m_count > 0 && area > budget) {
        int index = list->m_count - 1;
        ManagedTexture* texture = list->Get(index);
        while (list->m_count > 0 && texture->plannedLevel == 5) {
            if (--index == 0) {
                texture->plannedLevel = -1;
                area -= 0x400;
                list->RemoveOrdered(texture);
                index = list->m_count - 1;
            }
            if (index < 0)
                return area;
            texture = list->Get(index);
        }
        int level = texture->plannedLevel;
        if (texture->UnknownFunction5109b0() < 0.0f)
            texture->plannedLevel = texture->wantedLevel;
        else
            texture->plannedLevel--;
        int newLevel = texture->plannedLevel;
        area -= LevelArea(level);
        area += LevelArea(newLevel);
        if (list->m_count > 1) {
            list->RemoveOrdered(texture);
            list->Insert(texture, FindPosition(list, texture));
        }
    }
    return area;
}
