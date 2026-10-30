// Group2_ManagedTextureGroup.cpp -- bring-up (link) definitions for the two
// TextureCache.cpp repack bodies still missing from src/:
// ManagedTextureGroup::RepackWholePages (0x0050c960) and
// ManagedTextureGroup::RepackWithPartialBlits (0x0050dad0), both chosen by
// ManagedTextureGroup::RepackPages (src/reconstructed/ManagedTextureGroup.cpp).
//
// Status: best-guess-unreviewed. The bodies are copied unchanged from the
// near-miss file samples/render/ManagedTextureGroupNearMisses.cpp, whose
// header documents the remaining (stack-slot / register / scheduling only)
// differences against retail; see docs/TEXTUREMAPMANAGER.md. Not
// differentially tested.
//
// Also defined here because nothing in src/ provides them and only these
// bodies call them: UnknownTimer::LargestRecentSample (0x004cb690) and
// UnknownTimer::RecordSample (0x004cb6b0), written from the retail
// disassembly, and ManagedTextureGroup::UnknownFunction464e90 (the shared
// empty body 0x00464e90).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugAlloc.h"
#include "DebugOverlay.h"
#include "ManagedTexture.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"

unsigned int ReadClock();   // 0x004bfa80: the millisecond clock

// 0x004cb690: the peak sample while the interval since it has not run out,
// else 0.
int UnknownTimer::LargestRecentSample() {
    unsigned int now = ReadClock();
    if (now - (unsigned int)timerStart > (unsigned int)timerInterval)
        return 0;
    return field_0x04;
}

// 0x004cb6b0: keeps `sample` as the new peak (restarting the interval) when
// it is larger than the current peak or the current peak has expired.
void UnknownTimer::RecordSample(int sample) {
    unsigned int now = ReadClock();
    if (sample > field_0x04 || now - (unsigned int)timerStart > (unsigned int)timerInterval) {
        field_0x04 = sample;
        timerStart = (int)now;
    }
}

// 0x00464e90: the shared empty body (called when a texture finds no page
// space).
void ManagedTextureGroup::UnknownFunction464e90() {
}

static inline void SortTextures(ContainerList<ManagedTexture*>* list, int (__cdecl* compare)(const void*, const void*)) {
    qsort(list->m_data, list->m_count, sizeof(ManagedTexture*), compare);
}

static inline int LevelArea(int level) {
    if (level == 0)
        return 1;
    if (level < 0)
        return 0;
    int side = 2 << (level - 1);
    return side * side;
}

static inline float TexelMegabytes(ManagedTextureGroup* group, const int& texels) {
    return (float)PixelFormatBytesPerPixel(group->groupFormat) * texels * 1.2715658e-06f;
}

static inline CacheTexture* LeastWantedPage(UnknownTextureMapList* pages, int* lowest) {
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
void ManagedTextureGroup::RepackWholePages() {
    texelsBlitted = 0;
    ReadClock();
    placePasses = 0;
    memset(requestedBySize, 0, sizeof(requestedBySize));
    memset(grantedBySize, 0, sizeof(grantedBySize));
    texelsRequested = 0;
    texelsGranted = 0;
    texelsManaged = 0;
    UnknownTextureMapList emptied;
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

    qsort(growingTextures.m_data, growingTextures.m_count, sizeof(ManagedTexture*), UnknownCompare50eeb0);
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

    qsort(unplacedTextures.m_data, unplacedTextures.m_count, sizeof(ManagedTexture*), UnknownCompare50ee70);
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

    if (g_TrackGame->debugOverlay) {
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
            debugPage = g_TrackGame->debugOverlay->NewPage();
        texturesBlittedHistory.RecordSample(filled);
        g_TrackGame->debugOverlay->Title(debugPage, "TextureManager no partial");
        g_TrackGame->debugOverlay->Print(debugPage, "PixelFormat:%s", format);
        g_TrackGame->debugOverlay->Print(
            debugPage, "CacheTextures:%d %.2fM", cachePages.m_count,
            PageMegabytes(this, cachePages.m_count));
        int managedCount = managedTextures.m_count;
        g_TrackGame->debugOverlay->Print(debugPage, "ManagedTextures:%d", managedCount);
        float peak = (float)texelsBlittedHistory.LargestRecentSample();
        g_TrackGame->debugOverlay->Print(
            debugPage, "TexMem Blted: %.2fM, Peak: %.2fM",
            (float)PixelFormatBytesPerPixel(groupFormat) * texelsBlitted * 9.536743e-07f,
            PixelFormatBytesPerPixel(groupFormat) * peak * 9.536743e-07f);
        g_TrackGame->debugOverlay->Print(debugPage, "BltTime %d (%d)", now - blitStartTime,
                                                                blitTimeHistory.LargestRecentSample());
        g_TrackGame->debugOverlay->Print(debugPage, "Size Requested Granted");
        g_TrackGame->debugOverlay->Print(debugPage, "256:    %02d        %02d",
                                                                requestedBySize[8], grantedBySize[8]);
        g_TrackGame->debugOverlay->Print(debugPage, "128:    %02d        %02d",
                                                                requestedBySize[7], grantedBySize[7]);
        g_TrackGame->debugOverlay->Print(debugPage, " 64:    %02d        %02d",
                                                                requestedBySize[6], grantedBySize[6]);
        g_TrackGame->debugOverlay->Print(debugPage, " 32:    %02d        %02d",
                                                                requestedBySize[5], grantedBySize[5]);
        g_TrackGame->debugOverlay->Print(
            debugPage, "TexMemRequested:%.2fM", TexelMegabytes(this, texelsRequested));
        g_TrackGame->debugOverlay->Print(
            debugPage, "TexMemGranted:%.2fM", TexelMegabytes(this, texelsGranted));
        g_TrackGame->debugOverlay->Print(
            debugPage, "TexMemManaged:%.2fM", TexelMegabytes(this, texelsManaged));
        g_TrackGame->debugOverlay->Print(debugPage, "Textures Blted %d (%d)", filled,
                                                                texturesBlittedHistory.LargestRecentSample());
    }
    unplacedTextures.Clear();
    growingTextures.Clear();
    pageTextures.Clear();
    evictedTextures.Clear();
}

// The last element of `list`, or 0 when empty.
static inline ManagedTexture* LastOf(ContainerList<ManagedTexture*>* list) {
    if (list->m_count == 0)
        return 0;
    return list->Get(list->m_count - 1);
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

// 0x0050dad0: the repack with partial blits. Textures used since the last
// repack are planned at the level their use asks for and lowered, least
// needy first (0x005109b0), until they fit the group's texels; the planned
// level counts, topped up from the unused textures' places, become the
// page plan (+0x18). Pages whose whole region is one texture are set aside
// for level-8 textures, further pages are re-planned, and the textures
// still needing a place are placed largest first, collecting those to blit
// in +0x144. Their blits are spread over up to two passes within +0x74
// texels; the figures go to the debug overlay.
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

    SortTextures(&usedTextures, UnknownCompare50ef00);
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

    UnknownTextureMapList emptied;
    TextureMap* texture;
    texture = cachePages.First();
    int whole = pages[8];
    pages[8] = 0;
    while (whole && texture) {
        TextureMap* next = texture->nextTexture;
        UnknownTextureRegion* region = static_cast<CacheTexture*>(texture)->rootRegion;
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
    SortTextures(&usedTextures, UnknownCompare50ee70);
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
                UnknownFunction464e90();
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

    if (g_TrackGame->debugOverlay) {
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
            debugPage = g_TrackGame->debugOverlay->NewPage();
        g_TrackGame->debugOverlay->Title(debugPage, "TextureManager partial blts");
        g_TrackGame->debugOverlay->Print(debugPage, "PixelFormat:%s", format);
        g_TrackGame->debugOverlay->Print(
            debugPage, "CacheTextures:%d %.2fM", cachePages.m_count,
            (PixelFormatBytesPerPixel(groupFormat) * cachePages.m_count << 18) * 3.1789145e-07f);
        int managedCount = managedTextures.m_count;
        g_TrackGame->debugOverlay->Print(debugPage, "ManagedTextures:%d", managedCount);
        g_TrackGame->debugOverlay->Print(debugPage, "Managed %0.2fM",
                                                                (managedF = (float)managedTexels) * 9.536743e-07f);
        g_TrackGame->debugOverlay->Print(debugPage, "Ideal   %0.2fM",
                                                                (wantedF = (float)wantedTexels) * 9.536743e-07f);
        g_TrackGame->debugOverlay->Print(debugPage, "  ratio %0.2fM", wantedF / managedF);
        float peak = (float)texelsBlittedHistory.LargestRecentSample();
        g_TrackGame->debugOverlay->Print(
            debugPage, "TexMem Blted: %.2fM, (%.2fM)",
            (float)PixelFormatBytesPerPixel(groupFormat) * texelsBlitted * 9.536743e-07f,
            PixelFormatBytesPerPixel(groupFormat) * peak * 9.536743e-07f);
        g_TrackGame->debugOverlay->Print(debugPage, "BltTime %d (%d)", now - blitStartTime,
                                                                blitTimeHistory.LargestRecentSample());
        g_TrackGame->debugOverlay->Print(debugPage, "256:    %02d        %02d",
                                                                requestedBySize[8], grantedBySize[8]);
        g_TrackGame->debugOverlay->Print(debugPage, "128:    %02d        %02d",
                                                                requestedBySize[7], grantedBySize[7]);
        g_TrackGame->debugOverlay->Print(debugPage, " 64:    %02d        %02d",
                                                                requestedBySize[6], grantedBySize[6]);
        g_TrackGame->debugOverlay->Print(debugPage, " 32:    %02d        %02d",
                                                                requestedBySize[5], grantedBySize[5]);
        g_TrackGame->debugOverlay->Print(
            debugPage, "TexMemRequested:%.2fM", (float)PixelFormatBytesPerPixel(groupFormat) * texelsRequested * 1.2715658e-06f);
        g_TrackGame->debugOverlay->Print(
            debugPage, "TexMemGranted:%.2fM", (float)PixelFormatBytesPerPixel(groupFormat) * texelsGranted * 1.2715658e-06f);
        g_TrackGame->debugOverlay->Print(
            debugPage, "TexMemManaged:%.2fM", (float)PixelFormatBytesPerPixel(groupFormat) * texelsManaged * 1.2715658e-06f);
        g_TrackGame->debugOverlay->Print(debugPage, "Textures Blted %d", blitTextures.m_count);
    }
}
