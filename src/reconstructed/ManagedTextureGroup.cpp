#include "TextureMapManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugAlloc.h"
#include "DebugOverlay.h"
#include "ManagedTexture.h"
#include "PCRenderTarget.h"
// Only the offset-zero Game base is used (TrackGame RTTI COL 0x0055f8e0).
#include "Game.h"
#include "TrackGameGlobal.h"

extern "C" __declspec(dllimport) int __stdcall DeleteObject(void* object);

// cdecl 0x00461d40: formatted output to `file`.
#include "FilePrintf.h"

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
    : texelsBlittedHistory(5000), blitTimeHistory(5000), texturesBlittedHistory(5000) {
    groupAddressU = addressU;
    groupAddressV = addressV;
    groupManager = manager;
    groupFormat = format;
    pageMemory = 0;
    usedTextures.Init(64, 64);
    blitTextures.Init(64, 64);
    unplacedTextures.Init(16, 16);
    growingTextures.Init(16, 16);
    pageTextures.Init(16, 16);
    evictedTextures.Init(16, 16);
    dumpTextures.Init(16, 16);
    for (int i = 0; i < 9; i++)
        unusedByLevel[i].Init(64, 64);
    memset(pagesByLevel, 0, sizeof(pagesByLevel));
    pageCursor = 0;
    textureCursor = 0;
    placePasses = 0;
    texelsBlitted = 0;
    blitBudget = 0x10000;
    maxEmptiedPages = 6 / PixelFormatBytesPerPixel(groupFormat);
    gdiObject = 0;
    groupPalette = 0;
    noRepacking = 0;
    debugPage = -1;
    surfaceTexture = 0;
}

// 0x0050c2e0: leaves the manager and releases the textures and pages.
ManagedTextureGroup::~ManagedTextureGroup() {
    groupManager->groups.Remove(this);
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
        if (!surfaceTexture->CreateSurface(0, 256, 256, 0, 1, groupFormat, groupFormat, 0, 2, groupPalette, 1, 0,
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
            if (!page->CreateSurface(0, 256, 256, 0, 1, groupFormat, groupFormat, 0, 2, groupPalette, 1, 0,
                                     groupAddressU, groupAddressV, 0, 0x80, 0xff00ff)) {
                delete page;
                return 0;
            }
            if (!page->CreateTextureSurface(1, 0, 1)) {
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
            cache->colorMapper = texture->colorMapper;
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
            cache->ReleaseTextureSurface();
            cache->CreateTextureSurface(1, 0, 0);
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
                FilePrintf(file, ", Allocd=%d unhappy=%.2f", managed->allocatedLevel,
                                      managed->Unhappiness());
            FilePrintf(file, "\n");
        }
        fclose(file);
    }
}

// 0x0050c8c0: unless frozen by the debug key, empties the size lists and
// repacks the pages (through 0x0050dad0 when partial texture blits are on).
void ManagedTextureGroup::RepackPages() {
    if (reinterpret_cast<Game*>(g_TrackGame)->testKey && noRepacking)
        return;
    usedTextures.Clear();
    for (int i = 0; i < 9; i++)
        unusedByLevel[i].Clear();
    if (reinterpret_cast<Game*>(g_TrackGame)->display->partialTextureUploadResult <= 0)
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
    int difference = b->allocatedLevel - b->plannedLevel - a->allocatedLevel + a->plannedLevel;
    if (difference)
        return difference;
    return a - b;
}

// 0x0050ef00: by 0x005109b0 descending, then by address (descending).
int UnknownCompare50ef00(const void* first, const void* second) {
    ManagedTexture* a = *(ManagedTexture**)first;
    ManagedTexture* b = *(ManagedTexture**)second;
    float difference = -(a->Unhappiness() - b->Unhappiness());
    if (difference == 0.0f)
        return b - a;
    if (difference > 0.0f)
        return 1;
    return -1;
}

// GDI (gdi32.dll imports).
extern "C" __declspec(dllimport) void* __stdcall CreatePen(int style, int width, unsigned long color);
extern "C" __declspec(dllimport) void* __stdcall SelectObject(void* dc, void* object);
extern "C" __declspec(dllimport) int __stdcall MoveToEx(void* dc, int x, int y, void* previous);
extern "C" __declspec(dllimport) int __stdcall LineTo(void* dc, int x, int y);
extern "C" __declspec(dllimport) int __stdcall TextOutA(void* dc, int x, int y, const char* text, int length);

// The rectangle and fill effects of DirectDraw's Blt.
struct UnknownBltRect {
    long left;
    long top;
    long right;
    long bottom;
};

struct UnknownBltEffects {
    unsigned long size;
    unsigned char field_0x04[0x50 - 0x04];
    unsigned long fillColor;
    unsigned char field_0x54[0x64 - 0x54];
};

// 0x0050ef70: with the "TestKey" debug input, shows the debug-key selection
// below the top 20 rows of `target`: in mode 1 the selected texture (with
// its use, planned level and Un/Hi/Lo state) and an outline on its page,
// in mode 2 the selected page with every texture copied into its region.
// The page-drawing tail sits in its own block so its locals reuse the slots
// of dead earlier locals, and the single `return 1` (the other exits folded
// into the page and GetDC tests) lets VC6 save ebx/esi after the "not the
// selected group" return, as retail does (docs/VC6_SHRINK_WRAP.md). The
// outline copies the offsets and each axis's scale into locals: a fresh
// local is the newer x87 leaf, so `width * u` loads `u` and multiplies by
// st(1), and `uScale + pageOffsetU` loads the scale first.
int ManagedTextureGroup::DrawSelection(PCRenderTarget* target) {
    UnknownSurfaceInterface* surface = target->renderSurface;
    if (groupManager->debugGroup != this)
        return 0;
    if (reinterpret_cast<Game*>(g_TrackGame)->testKey) {
        char text[0x50];
        ManagedTexture* selected = 0;
        TextureMap* page = 0;
        int mode = groupManager->debugKeyMode;
        if (mode == 1 && textureCursor >= 0 && textureCursor < managedTextures.m_count) {
            int index = textureCursor;
            selected = static_cast<ManagedTexture*>(managedTextures.First());
            while (selected && index--)
                selected = static_cast<ManagedTexture*>(managedTextures.Next());
            if (selected) {
                const char* state = "";
                if (!selected->useCount && selected->page)
                    state = "Un";
                else if (selected->largestUse > 0.0f) {
                    if (selected->allocatedLevel > selected->wantedLevel)
                        state = "Hi";
                    else if (selected->allocatedLevel < selected->wantedLevel && selected->allocatedLevel != 8)
                        state = "Lo";
                }
                int x = target->targetWidth - selected->textureWidth;
                long pitch;
                unsigned char* bits = (unsigned char*)target->LockSurface(0, &pitch, 0x821);
                if (bits) {
                    long sourcePitch;
                    void* source = selected->LockSurface(0, &sourcePitch, 0x811);
                    if (source) {
                        int sourceSize = PixelFormatBytesPerPixel(selected->mapFormat);
                        int size = PixelFormatBytesPerPixel(target->surfacePixelFormat);
                        ConvertPixels(bits + x * size + 20 * pitch, source, selected->textureWidth,
                                              selected->textureHeight, pitch / size, sourcePitch / sourceSize,
                                              target->surfacePixelFormat, selected->mapFormat, 0, selected->colorMapper, 0, 0);
                        selected->systemSurface->Unlock(0);
                    }
                    target->UnlockSurface(0);
                }
                _snprintf(text, sizeof(text), "%d %0.2f->%d,%s", textureCursor + 1, selected->largestUse,
                          selected->allocatedLevel, state);
                void* textDc;
                if (!target->renderSurface->GetDC(&textDc)) {
                    TextOutA(textDc, x, 20, text, strlen(text));
                    target->renderSurface->ReleaseDC(textDc);
                }
            }
        }
        if (groupManager->debugKeyMode == 1 && selected) {
            int index = 0;
            for (page = cachePages.First(); page; page = cachePages.Next(), index++) {
                if (page == selected->page)
                    break;
            }
            if (page)
                pageCursor = index;
        } else if (groupManager->debugKeyMode == 2) {
            page = cachePages.First();
            for (int i = 0; i < pageCursor; i++)
                page = cachePages.Next();
        }
        if (page) {
            UnknownBltEffects effects;
            memset(&effects, 0, sizeof(effects));
            effects.size = sizeof(effects);
            effects.fillColor = 0;
            UnknownBltRect rect;
            rect.left = 0;
            rect.top = 20;
            rect.right = page->textureWidth;
            rect.bottom = page->textureHeight + 20;
            surface->Blt(&rect, 0, 0, 0x400, &effects);
            static_cast<CacheTexture*>(page)->ListTextures(&dumpTextures);
            long pitch;
            unsigned char* bits = (unsigned char*)target->LockSurface(0, &pitch, 0x821);
            if (bits) {
                for (int j = 0; j < dumpTextures.m_count; j++) {
                    ManagedTexture* managed = dumpTextures.Get(j);
                    UnknownTextureRegion* region = managed->region;
                    int left = (int)(page->textureWidth * region->left);
                    int top = (int)(page->textureHeight * region->top) + 20;
                    int side = (int)(managed->pageScale * 256.0f);
                    int levelSide = managed->textureWidth;
                    UnknownSurfaceInterface* level = managed->systemSurface;
                    UnknownSurfaceDesc desc;
                    memset(&desc, 0, sizeof(desc));
                    desc.size = sizeof(desc);
                    level->GetSurfaceDesc(&desc);
                    while (side < levelSide) {
                        levelSide >>= 1;
                        level->GetAttachedSurface((UnknownSurfaceCaps*)desc.caps, &level);
                        level->GetSurfaceDesc(&desc);
                    }
                    if (desc.height == side && desc.width == side) {
                        UnknownSurfaceDesc locked;
                        memset(&locked, 0, sizeof(locked));
                        locked.size = sizeof(locked);
                        if (!level->Lock(0, &locked, 0x811, 0)) {
                            int sourceSize = PixelFormatBytesPerPixel(managed->mapFormat);
                            int size = PixelFormatBytesPerPixel(target->surfacePixelFormat);
                            ConvertPixels(bits + top * pitch + left * size, locked.surface, levelSide, levelSide,
                                                  pitch / size, locked.pitch / sourceSize, target->surfacePixelFormat,
                                                  managed->mapFormat, 0, managed->colorMapper, 0, 0);
                            level->Unlock(0);
                        }
                    }
                }
                surface->Unlock(0);
            }
            dumpTextures.Clear();

            void* dc;
            int failed = surface->GetDC(&dc);
            if (!failed) {
                if (!gdiObject)
                    gdiObject = CreatePen(0, 1, 0xff00);
                if (gdiObject)
                    SelectObject(dc, gdiObject);
            }
            if (selected && !failed) {
                float width = (float)page->textureWidth;
                float u = selected->pageOffsetU;
                int left = (int)(width * u);
                float uScale = selected->pageScale;
                int right = (int)((uScale + selected->pageOffsetU) * width);
                float height = (float)page->textureHeight;
                float v = selected->pageOffsetV;
                int top = (int)(height * v + 20.0f);
                float vScale = selected->pageScale;
                int bottom = (int)((vScale + selected->pageOffsetV) * height + 20.0f);
                MoveToEx(dc, left, top, 0);
                LineTo(dc, right, top);
                LineTo(dc, right, bottom);
                LineTo(dc, left, bottom);
                LineTo(dc, left, top);
            }
            if (!failed) {
                sprintf(text, "#%d", pageCursor + 1);
                TextOutA(dc, 0, 20, text, strlen(text));
                surface->ReleaseDC(dc);
            }
        }
    }
    return 1;
}

// Where `texture` goes in `list`, kept in descending 0x005109b0 order.
static inline int FindPosition(ContainerList<ManagedTexture*>* list, ManagedTexture* texture) {
    int position = 0;
    for (int j = list->m_count - 1; j >= 0; j--) {
        if (texture->Unhappiness() < list->Get(j)->Unhappiness()) {
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
        if (texture->Unhappiness() < 0.0f)
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
