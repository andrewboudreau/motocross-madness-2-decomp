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

// 0x00464e90 (one empty body the linker folds with others): called when a
// texture finds no page space.
void ManagedTextureGroup::OnOutOfPageSpace() {
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

static inline void SortTextures(ContainerList<ManagedTexture*>* list, int (__cdecl* compare)(const void*, const void*)) {
    qsort(list->m_data, list->m_count, sizeof(ManagedTexture*), compare);
}

static inline float TexelMegabytes(ManagedTextureGroup* group, const int& texels) {
    return (float)PixelFormatBytesPerPixel(group->groupFormat) * texels * 1.2715658e-06f;
}

static inline CacheTexture* LeastWantedPage(TextureMapList* pages, int* lowest) {
    CacheTexture* best = 0;
    *lowest = 0x7fffffff;
    for (TextureMap* texture = pages->First(); texture; texture = pages->Next()) {
        CacheTexture* page = static_cast<CacheTexture*>(texture);
        if (page->wantedTexels < *lowest) {
            best = page;
            *lowest = page->wantedTexels;
        }
    }
    return best;
}

static inline float PageMegabytes(ManagedTextureGroup* group, const int& pages) {
    return (PixelFormatBytesPerPixel(group->groupFormat) * pages << 18) * 3.1789145e-07f;
}

// 0x0050c960: repacks the pages. Each texture used since the last repack
// is planned at the level its use asks for (at least 5, at most its own);
// unused ones give up their place. The pages least wanted by the planned
// textures are emptied until the textures that need a place fit, and are
// refilled largest level first; the figures go to the debug overlay.
// Not exact (136 of 3742 bytes in place, 3754 compiled: retail's frame is 8
// bytes smaller and the scalars sit in other slots; docs/TEXTUREMAPMANAGER.md).
void ManagedTextureGroup::RepackWholePages() {
    texelsBlitted = 0;
    ReadClock();
    placePasses = 0;
    memset(requestedBySize, 0, sizeof(requestedBySize));
    memset(grantedBySize, 0, sizeof(grantedBySize));
    texelsRequested = 0;
    texelsGranted = 0;
    texelsManaged = 0;
    TextureMapList emptied;
    int unused[9];
    memset(unused, 0, sizeof(unused));
    int texels = 0; // requested, then needed, texels
    TextureMap* texture;
    texture = managedTextures.First();
    while (texture) {
        ManagedTexture* managed = static_cast<ManagedTexture*>(texture);
        texelsManaged += LevelArea(managed->SizeLevel());
        if (!managed->useCount) {
            managed->plannedLevel = -1;
            int placed = managed->allocatedLevel;
            if (placed >= 0) {
                unused[placed]++;
                unusedByLevel[placed].Add(managed);
            }
            texture = managedTextures.Next();
            continue;
        }
        int level = (int)managed->largestUse;
        if (level > managed->SizeLevel())
            level = managed->SizeLevel();
        if (level < 5)
            level = 5;
        managed->plannedLevel = level;
        managed->wantedLevel = level;
        requestedBySize[level]++;
        texelsRequested += LevelArea(level);
        if (managed->allocatedLevel > level)
            level = managed->allocatedLevel;
        managed->plannedLevel = level;
        managed->lastUseFrame = groupManager->frameCount;
        usedTextures.Add(managed);
        texture = managedTextures.Next();
        texels += LevelArea(level);
    }

    FitTexturesToBudget(&usedTextures, pageMemory);
    int i;
    for (i = usedTextures.m_count - 1; i >= 0; i--) {
        ManagedTexture* managed = usedTextures.Get(i);
        if (managed->allocatedLevel <= 0) {
            usedTextures.RemoveOrdered(managed);
            unplacedTextures.Add(managed);
        } else if (managed->allocatedLevel < managed->plannedLevel) {
            usedTextures.RemoveOrdered(managed);
            growingTextures.Add(managed);
        }
    }

    texels = 0;
    for (i = 0; i < unplacedTextures.m_count; i++)
        texels += unplacedTextures.Get(i)->textureWidth * unplacedTextures.Get(i)->textureHeight;

    for (texture = cachePages.First(); texture; texture = cachePages.Next()) {
        CacheTexture* page = static_cast<CacheTexture*>(texture);
        page->wantedTexels = -(page->textureWidth * page->textureHeight);
        page->ListTextures(&pageTextures);
        for (int j = 0; j < pageTextures.m_count; j++) {
            ManagedTexture* managed = pageTextures.Get(j);
            if (managed->plannedLevel > 0)
                page->wantedTexels += LevelArea(managed->plannedLevel);
        }
    }

    qsort(growingTextures.m_data, growingTextures.m_count, sizeof(ManagedTexture*), CompareTexturesByLevelSurplus);
    int smallest = 0x7fffffff;
    if (growingTextures.m_count > 0)
        smallest = LevelArea(growingTextures.Get(0)->plannedLevel);
    int freed = 0;
    while (cachePages.m_count > 0) {
        int lowest;
        CacheTexture* best = LeastWantedPage(&cachePages, &lowest);
        int enough = freed >= texels;
        int full = freed >= unplacedTextures.m_count << 10;
        int limit = emptied.m_count >= maxEmptiedPages;
        int worthless = lowest <= -smallest;
        if (full && (limit || (enough && !worthless)))
            break;
        cachePages.Remove(best);
        emptied.Append(best);
        best->ListTextures(&evictedTextures);
        freed += best->textureWidth * best->textureHeight;
        for (int j = 0; j < evictedTextures.m_count; j++) {
            ManagedTexture* managed = evictedTextures.Get(j);
            if (managed->largestUse > 0.0f && !unplacedTextures.Contains(managed) && unplacedTextures.Add(managed))
                texels += managed->textureWidth * managed->textureHeight;
            usedTextures.Remove(managed);
            growingTextures.Remove(managed);
            managed->ReleasePage();
        }
        best->EmptyPage();
    }

    freed -= FitTexturesToBudget(&unplacedTextures, freed); // now the spare texels
    while (freed > 0 && growingTextures.m_count > 0) {
        ManagedTexture* managed = growingTextures.Get(0);
        if (LevelArea(managed->plannedLevel) > freed)
            break;
        freed -= LevelArea(managed->plannedLevel);
        unplacedTextures.Add(managed);
        growingTextures.Remove(managed);
        managed->ReleasePage();
    }

    qsort(unplacedTextures.m_data, unplacedTextures.m_count, sizeof(ManagedTexture*), CompareTexturesByPlannedLevel);
    int levels[9];
    memset(levels, 0, sizeof(levels));
    for (i = 0; i < unplacedTextures.m_count; i++)
        levels[unplacedTextures.Get(i)->plannedLevel]++;
    blitStartTime = ReadClock();
    int filled = 0;
    for (texture = emptied.First(); texture; texture = emptied.Next()) {
        if (unplacedTextures.m_count <= 0)
            break;
        CacheTexture* page = static_cast<CacheTexture*>(texture);
        filled++;
        page->ReserveRegions(levels, 8);
        page->PlaceTextures(&unplacedTextures, 0);
        page->Upload(0, -1);
    }
    cachePages.AppendList(&emptied);

    if (reinterpret_cast<Game*>(g_TrackGame)->debugOverlay) {
        int now = ReadClock();
        texelsBlittedHistory.RecordSample(texelsBlitted);
        blitTimeHistory.RecordSample(now - blitStartTime);
        char format[12];
        strcpy(format, "Unknown");
        texture = managedTextures.First();
        if (texture) {
            sprintf(format, "%d", texture->mapFormat);
            do {
                ManagedTexture* managed = static_cast<ManagedTexture*>(texture);
                if (managed->plannedLevel > 0) {
                    grantedBySize[managed->plannedLevel]++;
                    texelsGranted += LevelArea(managed->plannedLevel);
                }
            } while ((texture = managedTextures.Next()) != 0);
        }
        if (debugPage < 0)
            debugPage = reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->NewPage();
        texturesBlittedHistory.RecordSample(filled);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Title(debugPage, "TextureManager no partial");
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "PixelFormat:%s", format);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "CacheTextures:%d %.2fM", cachePages.m_count,
            PageMegabytes(this, cachePages.m_count));
        int managedCount = managedTextures.m_count;
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "ManagedTextures:%d", managedCount);
        float peak = (float)texelsBlittedHistory.LargestRecentSample();
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "TexMem Blted: %.2fM, Peak: %.2fM",
            (float)PixelFormatBytesPerPixel(groupFormat) * texelsBlitted * 9.536743e-07f,
            PixelFormatBytesPerPixel(groupFormat) * peak * 9.536743e-07f);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "BltTime %d (%d)", now - blitStartTime,
                                                                blitTimeHistory.LargestRecentSample());
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "Size Requested Granted");
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "256:    %02d        %02d",
                                                                requestedBySize[8], grantedBySize[8]);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "128:    %02d        %02d",
                                                                requestedBySize[7], grantedBySize[7]);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, " 64:    %02d        %02d",
                                                                requestedBySize[6], grantedBySize[6]);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, " 32:    %02d        %02d",
                                                                requestedBySize[5], grantedBySize[5]);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "TexMemRequested:%.2fM", TexelMegabytes(this, texelsRequested));
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "TexMemGranted:%.2fM", TexelMegabytes(this, texelsGranted));
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "TexMemManaged:%.2fM", TexelMegabytes(this, texelsManaged));
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "Textures Blted %d (%d)", filled,
                                                                texturesBlittedHistory.LargestRecentSample());
    }
    unplacedTextures.Clear();
    growingTextures.Clear();
    pageTextures.Clear();
    evictedTextures.Clear();
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
    qsort(list->m_data, list->m_count, sizeof(ManagedTexture*), CompareTexturesByUnhappiness);
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

// The last element of `list`, or 0 when empty.
static inline ManagedTexture* LastOf(ContainerList<ManagedTexture*>* list) {
    if (list->m_count == 0)
        return 0;
    return list->Get(list->m_count - 1);
}

// 0x0050dad0: the repack with partial blits. Textures used since the last
// repack are planned at the level their use asks for and lowered, least
// needy first (0x005109b0), until they fit the group's texels; the planned
// level counts, topped up from the unused textures' places, become the
// page plan (+0x18). Pages whose whole region is one texture are set aside
// for level-8 textures, further pages are re-planned, and the textures
// still needing a place are placed largest first, collecting those to blit
// in +0x144. Their blits are spread over up to two passes within +0x74
// texels; the figures go to the debug overlay.
// Not exact (4913 of 5020 bytes in place: two of the 9-entry arrays are
// swapped and a few overlay rows load in another order; docs/TEXTUREMAPMANAGER.md).
void ManagedTextureGroup::RepackWithPartialBlits() {
    texelsBlitted = 0;
    ReadClock();
    placePasses = 0;
    memset(requestedBySize, 0, sizeof(requestedBySize));
    memset(grantedBySize, 0, sizeof(grantedBySize));
    texelsRequested = 0;
    texelsGranted = 0;
    texelsManaged = 0;
    blitTextures.Clear();
    int unused[9];
    int pages[9];
    int levels[9];
    int spare[9];
    int wanted[9];
    memset(unused, 0, sizeof(unused));
    int dropped = 0;
    ManagedTexture* managed = static_cast<ManagedTexture*>(managedTextures.First());
    if (!managed)
        return;
    while (managed) {
        texelsManaged += LevelArea(managed->SizeLevel());
        if (!managed->useCount) {
            managed->plannedLevel = -1;
            int placed = managed->allocatedLevel;
            if (placed >= 0) {
                unused[placed]++;
                unusedByLevel[placed].Add(managed);
            }
            managed = static_cast<ManagedTexture*>(managedTextures.Next());
            continue;
        }
        int level = (int)managed->largestUse;
        if (level > managed->SizeLevel())
            level = managed->SizeLevel();
        if (level < 5)
            level = 5;
        managed->wantedLevel = level;
        requestedBySize[level]++;
        texelsRequested += LevelArea(level);
        if (managed->allocatedLevel > level)
            level = managed->allocatedLevel;
        managed->lastUseFrame = groupManager->frameCount;
        managed->plannedLevel = level;
        usedTextures.Add(managed);
        managed = static_cast<ManagedTexture*>(managedTextures.Next());
    }

    SortTextures(&usedTextures, CompareTexturesByUnhappiness);
    int texels = 0;
    int i;
    for (i = 0; i < usedTextures.m_count; i++) {
        int level = usedTextures.Get(i)->plannedLevel;
        if (level >= 0)
            texels += LevelArea(level);
    }
    if (texels > pageMemory) {
        dropped = 1;
        memset(unused, 0, sizeof(unused));
        for (int k = 0; k < 9; k++) {
            for (int j = 0; j < unusedByLevel[k].m_count; j++)
                unusedByLevel[k].Get(j)->ReleasePage();
            unusedByLevel[k].Clear();
        }
    }
    while (texels > pageMemory) {
        int index = usedTextures.m_count - 1;
        managed = usedTextures.Get(index);
        while (managed->plannedLevel == 5)
            managed = usedTextures.Get(--index);
        int level = managed->plannedLevel;
        if (managed->Unhappiness() < 0.0f) {
            managed->ReleasePage();
            managed->plannedLevel = managed->wantedLevel;
        } else {
            managed->plannedLevel--;
        }
        int newLevel = managed->plannedLevel;
        texels -= LevelArea(level);
        texels += LevelArea(newLevel);
        if (usedTextures.m_count > 1) {
            usedTextures.RemoveOrdered(managed);
            usedTextures.Insert(managed, FindPosition(&usedTextures, managed));
        }
    }

    for (i = 0; i < usedTextures.m_count; i++) {
        managed = usedTextures.Get(i);
        if (managed->IsResident() && managed->plannedLevel != managed->allocatedLevel)
            managed->ReleasePage();
    }
    memset(levels, 0, sizeof(levels));
    int count = usedTextures.m_count;
    for (i = 0; i < count; i++)
        levels[usedTextures.Get(i)->plannedLevel]++;

    memset(pages, 0, sizeof(pages));
    int total = 0;
    memset(spare, 0, sizeof(spare));
    for (i = 0; i <= 8; i++) {
        int n;
        if (dropped) {
            n = levels[i];
        } else {
            n = levels[i] + unused[i];
            if (n < pagesByLevel[i]) {
                spare[i] = pagesByLevel[i] - n;
                n = pagesByLevel[i];
            }
        }
        total += LevelArea(i) * n;
        pages[i] = n;
    }
    int excess = total - pageMemory;
    int level = 5;
    while (excess > 0 && level <= 8) {
        int count = excess / LevelArea(level);
        if (excess % LevelArea(level) > 0)
            count++;
        if (count > spare[level])
            count = spare[level];
        pages[level] -= count;
        spare[level] -= count;
        excess -= LevelArea(level) * count;
        total -= LevelArea(level) * count;
        level++;
    }
    level = 5;
    while (excess > 0 && level <= 8) {
        int count = excess / LevelArea(level);
        if (excess % LevelArea(level) > 0)
            count++;
        if (count > unused[level])
            count = unused[level];
        pages[level] -= count;
        unused[level] -= count;
        excess -= LevelArea(level) * count;
        total -= LevelArea(level) * count;
        ContainerList<ManagedTexture*>* list = &unusedByLevel[level];
        ManagedTexture* last = LastOf(list);
        for (int j = 0; j < count; j++) {
            last->ReleasePage();
            list->RemoveOrdered(last);
            last = LastOf(list);
        }
        level++;
    }
    memcpy(pagesByLevel, pages, sizeof(pages));

    TextureMapList emptied;
    TextureMap* texture;
    texture = cachePages.First();
    int whole = pages[8];
    pages[8] = 0;
    while (whole && texture) {
        TextureMap* next = texture->nextTexture;
        CacheTextureRegion* region = static_cast<CacheTexture*>(texture)->rootRegion;
        if (region && region->reserved && !region->quarters[0]) {
            cachePages.Remove(texture);
            emptied.Append(texture);
            whole--;
        }
        texture = next;
    }
    memset(wanted, 0, sizeof(wanted));
    wanted[8] = whole;
    texture = cachePages.Last();
    while (whole) {
        TextureMap* previous = texture->previousTexture;
        cachePages.Remove(texture);
        emptied.Append(texture);
        static_cast<CacheTexture*>(texture)->ReserveRegions(wanted, 8);
        whole--;
        texture = previous;
    }
    texture = cachePages.First();
    while (texture) {
        int full = static_cast<CacheTexture*>(texture)->ReserveRegions(pages, 8);
        texture = cachePages.Next();
        if (full)
            break;
    }
    for (; texture; texture = cachePages.Next())
        static_cast<CacheTexture*>(texture)->EmptyPage();

    i = 0;
    while (i < usedTextures.m_count) {
        managed = usedTextures.Get(i);
        if (managed->IsResident()) {
            if (managed->copiedSkipLevels)
                blitTextures.Add(managed);
            usedTextures.Remove(managed);
        } else {
            i++;
        }
    }
    cachePages.AppendList(&emptied);
    SortTextures(&usedTextures, CompareTexturesByPlannedLevel);
    if (usedTextures.m_count > 0) {
        texture = cachePages.Last();
        while (usedTextures.m_count > 0 && usedTextures.Get(0)->plannedLevel == 8) {
            static_cast<CacheTexture*>(texture)->PlaceTextures(&usedTextures, &blitTextures);
            placePasses++;
            texture = cachePages.Previous();
        }
        texture = cachePages.First();
        while (texture) {
            int done = static_cast<CacheTexture*>(texture)->PlaceTextures(&usedTextures, &blitTextures);
            placePasses++;
            if (done)
                break;
            texture = cachePages.Next();
            if (!texture)
                OnOutOfPageSpace();
        }
    }

    blitStartTime = ReadClock();
    if (blitTextures.m_count > 0) {
        int blitted = 0;
        for (i = 0; i < blitTextures.m_count; i++) {
            managed = blitTextures.Get(i);
            managed->blitSkipLevels = 0;
            blitted += LevelArea(managed->plannedLevel);
        }
        int j = 0;
        int pass = 1;
        while (pass < 3 && blitTextures.m_count > 0 && blitted > blitBudget) {
            managed = blitTextures.Get(j);
            if (managed->allocatedLevel - pass >= 5) {
                blitted -= LevelArea(managed->plannedLevel - managed->blitSkipLevels);
                managed->blitSkipLevels = pass;
                if (pass == managed->copiedSkipLevels && managed->IsResident()) {
                    blitTextures.RemoveOrdered(managed);
                } else {
                    blitted += LevelArea(managed->plannedLevel - managed->blitSkipLevels);
                    j++;
                }
            } else {
                j++;
            }
            if (j >= blitTextures.m_count) {
                j = 0;
                pass++;
            }
        }
        for (i = 0; i < blitTextures.m_count; i++) {
            managed = blitTextures.Get(i);
            managed->copiedSkipLevels = managed->blitSkipLevels;
            managed->page->CopyMipLevels(managed->region, managed->blitSkipLevels);
        }
    }

    if (reinterpret_cast<Game*>(g_TrackGame)->debugOverlay) {
        int now = ReadClock();
        texelsBlittedHistory.RecordSample(texelsBlitted);
        blitTimeHistory.RecordSample(now - blitStartTime);
        int managedTexels = managedTextures.m_count << 16;
        char format[8];
        int wantedTexels = 0;
        float managedF;
        float wantedF;
        strcpy(format, "Unknown");
        texture = managedTextures.First();
        if (texture) {
            sprintf(format, "%d", texture->mapFormat);
            do {
                managed = static_cast<ManagedTexture*>(texture);
                if (managed->plannedLevel > 0) {
                    grantedBySize[managed->plannedLevel]++;
                    texelsGranted += LevelArea(managed->plannedLevel);
                }
                if (managed->wantedLevel > 0)
                    wantedTexels += LevelArea(managed->wantedLevel);
            } while ((texture = managedTextures.Next()) != 0);
        }
        if (debugPage < 0)
            debugPage = reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->NewPage();
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Title(debugPage, "TextureManager partial blts");
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "PixelFormat:%s", format);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "CacheTextures:%d %.2fM", cachePages.m_count,
            (PixelFormatBytesPerPixel(groupFormat) * cachePages.m_count << 18) * 3.1789145e-07f);
        int managedCount = managedTextures.m_count;
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "ManagedTextures:%d", managedCount);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "Managed %0.2fM",
                                                                (managedF = (float)managedTexels) * 9.536743e-07f);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "Ideal   %0.2fM",
                                                                (wantedF = (float)wantedTexels) * 9.536743e-07f);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "  ratio %0.2fM", wantedF / managedF);
        float peak = (float)texelsBlittedHistory.LargestRecentSample();
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "TexMem Blted: %.2fM, (%.2fM)",
            (float)PixelFormatBytesPerPixel(groupFormat) * texelsBlitted * 9.536743e-07f,
            PixelFormatBytesPerPixel(groupFormat) * peak * 9.536743e-07f);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "BltTime %d (%d)", now - blitStartTime,
                                                                blitTimeHistory.LargestRecentSample());
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "256:    %02d        %02d",
                                                                requestedBySize[8], grantedBySize[8]);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "128:    %02d        %02d",
                                                                requestedBySize[7], grantedBySize[7]);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, " 64:    %02d        %02d",
                                                                requestedBySize[6], grantedBySize[6]);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, " 32:    %02d        %02d",
                                                                requestedBySize[5], grantedBySize[5]);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "TexMemRequested:%.2fM", (float)PixelFormatBytesPerPixel(groupFormat) * texelsRequested * 1.2715658e-06f);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "TexMemGranted:%.2fM", (float)PixelFormatBytesPerPixel(groupFormat) * texelsGranted * 1.2715658e-06f);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(
            debugPage, "TexMemManaged:%.2fM", (float)PixelFormatBytesPerPixel(groupFormat) * texelsManaged * 1.2715658e-06f);
        reinterpret_cast<Game*>(g_TrackGame)->debugOverlay->Print(debugPage, "Textures Blted %d", blitTextures.m_count);
    }
}

// 0x0050ee70: qsort order of ManagedTexture pointers, by +0xac descending,
// then by address.
int CompareTexturesByPlannedLevel(const void* first, const void* second) {
    ManagedTexture* a = *(ManagedTexture**)first;
    ManagedTexture* b = *(ManagedTexture**)second;
    int difference = b->plannedLevel - a->plannedLevel;
    if (difference)
        return difference;
    return a - b;
}

// 0x0050eeb0: by (+0xa8 - +0xac) descending, then by address.
int CompareTexturesByLevelSurplus(const void* first, const void* second) {
    ManagedTexture* a = *(ManagedTexture**)first;
    ManagedTexture* b = *(ManagedTexture**)second;
    int difference = b->allocatedLevel - b->plannedLevel - a->allocatedLevel + a->plannedLevel;
    if (difference)
        return difference;
    return a - b;
}

// 0x0050ef00: by 0x005109b0 descending, then by address (descending).
int CompareTexturesByUnhappiness(const void* first, const void* second) {
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
struct BltDestinationRect {
    long left;
    long top;
    long right;
    long bottom;
};

struct BltFillEffects {
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
    DirectDrawSurface* surface = target->renderSurface;
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
            BltFillEffects effects;
            memset(&effects, 0, sizeof(effects));
            effects.size = sizeof(effects);
            effects.fillColor = 0;
            BltDestinationRect rect;
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
                    CacheTextureRegion* region = managed->region;
                    int left = (int)(page->textureWidth * region->left);
                    int top = (int)(page->textureHeight * region->top) + 20;
                    int side = (int)(managed->pageScale * 256.0f);
                    int levelSide = managed->textureWidth;
                    DirectDrawSurface* level = managed->systemSurface;
                    DDSurfaceDesc2 desc;
                    memset(&desc, 0, sizeof(desc));
                    desc.size = sizeof(desc);
                    level->GetSurfaceDesc(&desc);
                    while (side < levelSide) {
                        levelSide >>= 1;
                        level->GetAttachedSurface((DDSCaps2*)desc.caps, &level);
                        level->GetSurfaceDesc(&desc);
                    }
                    if (desc.height == side && desc.width == side) {
                        DDSurfaceDesc2 locked;
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
