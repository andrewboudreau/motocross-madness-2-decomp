// Near-miss ManagedTextureGroup candidates (TextureCache.cpp), kept out of
// src/reconstructed until they match. See docs/TEXTUREMAPMANAGER.md.
//
// ManagedTextureGroup::RepackWholePages (0x0050c960, 3742 bytes): the
// repack. The control flow, every call and the inlining pattern line up:
// VC6 inlines ContainerList::Reserve only in the last two Add calls, as
// retail does, once the TexMem figures go through TexelMegabytes and
// PageMegabytes and the page search through LeastWantedPage (VC6's inline
// budget scales with the function's size, so without those helpers it
// inlines every Reserve). The helpers take their counts by const reference:
// retail multiplies straight from the fields (`fimul [ebx+0x244]`), which
// a by-value parameter turns into a stack copy (304 -> 158 differing
// instructions). What remains is stack-slot packing: retail has two scalar
// slots fewer (frame 0x78 against 0x80), puts the 12-byte format buffer
// over the dead `i`/`smallest` slots and `filled` over `texels`, keeps
// `now` in ebp through the overlay block and hoists the level-count loop's
// bound. About 158 of 1150 instructions differ, almost all in slot or
// register choice. Tried without effect: block scopes around the emptying
// section, the first pass (`requested`), the levels loop and the
// refill+overlay section; a hoisted count (frame shrinks by 4 but the loop
// registers rotate); an `elapsed` local for `now - field_0x250` (worse).
//
// ManagedTextureGroup::RepackWithPartialBlits (0x0050dad0, 5020 bytes): the
// repack with partial texture blits, chosen by 0x0050c8c0. Control flow,
// calls, inlining and the scalar stack slots line up (the slots only once
// one `managed` variable serves every loop, and the blit-pass loop only as
// `while (pass < 3 && ...)`). Left: retail places `unused`'s neighbours
// `levels` and `spare` the other way round (0x8c/0xb0), the 9-entry loop's
// registers follow from that, retail stores `dropped = 0` after the
// empty-list return (moving the declaration or the store there costs 130
// more differences), and from the "ManagedTextures" row on the overlay
// calls rotate eax/ecx/edx. About 216 of 1600 instructions differ; the
// size is 5022 against 5020. Declaration order, block scopes around the
// arrays and the memset order do not move the arrays; computing
// `managedTexels` at its row is worse.
//
// ManagedTextureGroup::DrawSelection (0x0050ef70, 1834 bytes): the
// debug display behind manager slot 15. Every stack slot matches once the
// page-drawing tail sits in its own block (`if (page) { ... }`): VC6 lets
// block-scoped locals reuse the slots of dead earlier locals, but gives a
// function-scope local its own slot wherever it is declared (101 -> 35
// differing instructions; declaration order and names change nothing).
// Reading the outline's fields through `const ManagedTexture& t` fixes the
// fadd load order (35 -> 31). Left: retail saves ebx/esi only after the
// "not the selected group" return (VC6 here saves all four registers in
// the prologue, shifting the entry block and the return-1 epilogues; not
// changed by a manager local, the comparison order or nesting the body
// under either test), retail reloads the page into ebx at the bottom of
// the texture loop (VC6 at its top and after it; a while loop is worse),
// and retail multiplies `left`/`top` as `fld [field]; fmul st(1)` where
// VC6 emits `fld st(0); fmul [field]` (not moved by float/double temps,
// pointer locals, by-value or by-reference inline helpers). About 31 of
// 575 instructions differ; the size is 1840 against 1834.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/DebugOverlay.h"
#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/TrackGame.h"

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
        if (page->field_0x18c < *lowest) {
            best = page;
            *lowest = page->field_0x18c;
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
    field_0x1d0 = 0;
    ReadClock();
    field_0x1cc = 0;
    memset(field_0x1fc, 0, sizeof(field_0x1fc));
    memset(field_0x220, 0, sizeof(field_0x220));
    field_0x244 = 0;
    field_0x248 = 0;
    field_0x24c = 0;
    UnknownTextureMapList emptied;
    int unused[9];
    memset(unused, 0, sizeof(unused));
    int texels = 0; // requested, then needed, texels
    TextureMap* texture;
    texture = managedTextures.First();
    while (texture) {
        ManagedTexture* managed = static_cast<ManagedTexture*>(texture);
        field_0x24c += LevelArea(managed->SizeLevel());
        if (!managed->useCount) {
            managed->plannedLevel = -1;
            int placed = managed->field_0xa8;
            if (placed >= 0) {
                unused[placed]++;
                field_0x90[placed].Add(managed);
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
        field_0x1fc[level]++;
        field_0x244 += LevelArea(level);
        if (managed->field_0xa8 > level)
            level = managed->field_0xa8;
        managed->plannedLevel = level;
        managed->lastUseFrame = groupManager->frameCount;
        field_0x7c.Add(managed);
        texture = managedTextures.Next();
        texels += LevelArea(level);
    }

    FitTexturesToBudget(&field_0x7c, pageMemory);
    int i;
    for (i = field_0x7c.m_count - 1; i >= 0; i--) {
        ManagedTexture* managed = field_0x7c.Get(i);
        if (managed->field_0xa8 <= 0) {
            field_0x7c.RemoveOrdered(managed);
            field_0x158.Add(managed);
        } else if (managed->field_0xa8 < managed->plannedLevel) {
            field_0x7c.RemoveOrdered(managed);
            field_0x16c.Add(managed);
        }
    }

    texels = 0;
    for (i = 0; i < field_0x158.m_count; i++)
        texels += field_0x158.Get(i)->textureWidth * field_0x158.Get(i)->textureHeight;

    for (texture = cachePages.First(); texture; texture = cachePages.Next()) {
        CacheTexture* page = static_cast<CacheTexture*>(texture);
        page->field_0x18c = -(page->textureWidth * page->textureHeight);
        page->ListTextures(&field_0x180);
        for (int j = 0; j < field_0x180.m_count; j++) {
            ManagedTexture* managed = field_0x180.Get(j);
            if (managed->plannedLevel > 0)
                page->field_0x18c += LevelArea(managed->plannedLevel);
        }
    }

    qsort(field_0x16c.m_data, field_0x16c.m_count, sizeof(ManagedTexture*), UnknownCompare50eeb0);
    int smallest = 0x7fffffff;
    if (field_0x16c.m_count > 0)
        smallest = LevelArea(field_0x16c.Get(0)->plannedLevel);
    int freed = 0;
    while (cachePages.m_count > 0) {
        int lowest;
        CacheTexture* best = LeastWantedPage(&cachePages, &lowest);
        int enough = freed >= texels;
        int full = freed >= field_0x158.m_count << 10;
        int limit = emptied.m_count >= field_0x78;
        int worthless = lowest <= -smallest;
        if (full && (limit || (enough && !worthless)))
            break;
        cachePages.Remove(best);
        emptied.Append(best);
        best->ListTextures(&field_0x194);
        freed += best->textureWidth * best->textureHeight;
        for (int j = 0; j < field_0x194.m_count; j++) {
            ManagedTexture* managed = field_0x194.Get(j);
            if (managed->largestUse > 0.0f && !field_0x158.Contains(managed) && field_0x158.Add(managed))
                texels += managed->textureWidth * managed->textureHeight;
            field_0x7c.Remove(managed);
            field_0x16c.Remove(managed);
            managed->ReleasePage();
        }
        best->EmptyPage();
    }

    freed -= FitTexturesToBudget(&field_0x158, freed); // now the spare texels
    while (freed > 0 && field_0x16c.m_count > 0) {
        ManagedTexture* managed = field_0x16c.Get(0);
        if (LevelArea(managed->plannedLevel) > freed)
            break;
        freed -= LevelArea(managed->plannedLevel);
        field_0x158.Add(managed);
        field_0x16c.Remove(managed);
        managed->ReleasePage();
    }

    qsort(field_0x158.m_data, field_0x158.m_count, sizeof(ManagedTexture*), UnknownCompare50ee70);
    int levels[9];
    memset(levels, 0, sizeof(levels));
    for (i = 0; i < field_0x158.m_count; i++)
        levels[field_0x158.Get(i)->plannedLevel]++;
    field_0x250 = ReadClock();
    int filled = 0;
    for (texture = emptied.First(); texture; texture = emptied.Next()) {
        if (field_0x158.m_count <= 0)
            break;
        CacheTexture* page = static_cast<CacheTexture*>(texture);
        filled++;
        page->ReserveRegions(levels, 8);
        page->PlaceTextures(&field_0x158, 0);
        page->Upload(0, -1);
    }
    cachePages.AppendList(&emptied);

    if (g_TrackGame->debugOverlay) {
        int now = ReadClock();
        field_0x1d4.RecordSample(field_0x1d0);
        field_0x1e0.RecordSample(now - field_0x250);
        char format[12];
        strcpy(format, "Unknown");
        texture = managedTextures.First();
        if (texture) {
            sprintf(format, "%d", texture->mapFormat);
            do {
                ManagedTexture* managed = static_cast<ManagedTexture*>(texture);
                if (managed->plannedLevel > 0) {
                    field_0x220[managed->plannedLevel]++;
                    field_0x248 += LevelArea(managed->plannedLevel);
                }
            } while ((texture = managedTextures.Next()) != 0);
        }
        if (field_0x254 < 0)
            field_0x254 = g_TrackGame->debugOverlay->NewPage();
        field_0x1ec.RecordSample(filled);
        g_TrackGame->debugOverlay->Title(field_0x254, "TextureManager no partial");
        g_TrackGame->debugOverlay->Print(field_0x254, "PixelFormat:%s", format);
        g_TrackGame->debugOverlay->Print(
            field_0x254, "CacheTextures:%d %.2fM", cachePages.m_count,
            PageMegabytes(this, cachePages.m_count));
        g_TrackGame->debugOverlay->Print(field_0x254, "ManagedTextures:%d", managedTextures.m_count);
        float peak = (float)field_0x1d4.LargestRecentSample();
        g_TrackGame->debugOverlay->Print(
            field_0x254, "TexMem Blted: %.2fM, Peak: %.2fM",
            (float)PixelFormatBytesPerPixel(groupFormat) * field_0x1d0 * 9.536743e-07f,
            PixelFormatBytesPerPixel(groupFormat) * peak * 9.536743e-07f);
        g_TrackGame->debugOverlay->Print(field_0x254, "BltTime %d (%d)", now - field_0x250,
                                                                field_0x1e0.LargestRecentSample());
        g_TrackGame->debugOverlay->Print(field_0x254, "Size Requested Granted");
        g_TrackGame->debugOverlay->Print(field_0x254, "256:    %02d        %02d",
                                                                field_0x1fc[8], field_0x220[8]);
        g_TrackGame->debugOverlay->Print(field_0x254, "128:    %02d        %02d",
                                                                field_0x1fc[7], field_0x220[7]);
        g_TrackGame->debugOverlay->Print(field_0x254, " 64:    %02d        %02d",
                                                                field_0x1fc[6], field_0x220[6]);
        g_TrackGame->debugOverlay->Print(field_0x254, " 32:    %02d        %02d",
                                                                field_0x1fc[5], field_0x220[5]);
        g_TrackGame->debugOverlay->Print(
            field_0x254, "TexMemRequested:%.2fM", TexelMegabytes(this, field_0x244));
        g_TrackGame->debugOverlay->Print(
            field_0x254, "TexMemGranted:%.2fM", TexelMegabytes(this, field_0x248));
        g_TrackGame->debugOverlay->Print(
            field_0x254, "TexMemManaged:%.2fM", TexelMegabytes(this, field_0x24c));
        g_TrackGame->debugOverlay->Print(field_0x254, "Textures Blted %d (%d)", filled,
                                                                field_0x1ec.LargestRecentSample());
    }
    field_0x158.Clear();
    field_0x16c.Clear();
    field_0x180.Clear();
    field_0x194.Clear();
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
        if (texture->UnknownFunction5109b0() < list->Get(j)->UnknownFunction5109b0()) {
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
    field_0x1d0 = 0;
    ReadClock();
    field_0x1cc = 0;
    memset(field_0x1fc, 0, sizeof(field_0x1fc));
    memset(field_0x220, 0, sizeof(field_0x220));
    field_0x244 = 0;
    field_0x248 = 0;
    field_0x24c = 0;
    field_0x144.Clear();
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
        field_0x24c += LevelArea(managed->SizeLevel());
        if (!managed->useCount) {
            managed->plannedLevel = -1;
            int placed = managed->field_0xa8;
            if (placed >= 0) {
                unused[placed]++;
                field_0x90[placed].Add(managed);
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
        field_0x1fc[level]++;
        field_0x244 += LevelArea(level);
        if (managed->field_0xa8 > level)
            level = managed->field_0xa8;
        managed->lastUseFrame = groupManager->frameCount;
        managed->plannedLevel = level;
        field_0x7c.Add(managed);
        managed = static_cast<ManagedTexture*>(managedTextures.Next());
    }

    qsort(field_0x7c.m_data, field_0x7c.m_count, sizeof(ManagedTexture*), UnknownCompare50ef00);
    int texels = 0;
    int i;
    for (i = 0; i < field_0x7c.m_count; i++) {
        int level = field_0x7c.Get(i)->plannedLevel;
        if (level >= 0)
            texels += LevelArea(level);
    }
    if (texels > pageMemory) {
        dropped = 1;
        memset(unused, 0, sizeof(unused));
        for (int k = 0; k < 9; k++) {
            for (int j = 0; j < field_0x90[k].m_count; j++)
                field_0x90[k].Get(j)->ReleasePage();
            field_0x90[k].Clear();
        }
    }
    while (texels > pageMemory) {
        int index = field_0x7c.m_count - 1;
        managed = field_0x7c.Get(index);
        while (managed->plannedLevel == 5)
            managed = field_0x7c.Get(--index);
        int level = managed->plannedLevel;
        if (managed->UnknownFunction5109b0() < 0.0f) {
            managed->ReleasePage();
            managed->plannedLevel = managed->wantedLevel;
        } else {
            managed->plannedLevel--;
        }
        int newLevel = managed->plannedLevel;
        texels -= LevelArea(level);
        texels += LevelArea(newLevel);
        if (field_0x7c.m_count > 1) {
            field_0x7c.RemoveOrdered(managed);
            field_0x7c.Insert(managed, FindPosition(&field_0x7c, managed));
        }
    }

    for (i = 0; i < field_0x7c.m_count; i++) {
        managed = field_0x7c.Get(i);
        if (managed->IsResident() && managed->plannedLevel != managed->field_0xa8)
            managed->ReleasePage();
    }
    memset(levels, 0, sizeof(levels));
    int count = field_0x7c.m_count;
    for (i = 0; i < count; i++)
        levels[field_0x7c.Get(i)->plannedLevel]++;

    memset(pages, 0, sizeof(pages));
    int total = 0;
    memset(spare, 0, sizeof(spare));
    for (i = 0; i <= 8; i++) {
        int n = levels[i];
        if (!dropped) {
            n += unused[i];
            if (n < field_0x18[i]) {
                spare[i] = field_0x18[i] - n;
                n = field_0x18[i];
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
        spare[level] -= count;
        pages[level] -= count;
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
        unused[level] -= count;
        pages[level] -= count;
        excess -= LevelArea(level) * count;
        total -= LevelArea(level) * count;
        ContainerList<ManagedTexture*>* list = &field_0x90[level];
        ManagedTexture* last = LastOf(list);
        for (int j = 0; j < count; j++) {
            last->ReleasePage();
            list->RemoveOrdered(last);
            last = LastOf(list);
        }
        level++;
    }
    memcpy(field_0x18, pages, sizeof(pages));

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
    while (i < field_0x7c.m_count) {
        managed = field_0x7c.Get(i);
        if (managed->IsResident()) {
            if (managed->field_0x98)
                field_0x144.Add(managed);
            field_0x7c.Remove(managed);
        } else {
            i++;
        }
    }
    cachePages.AppendList(&emptied);
    qsort(field_0x7c.m_data, field_0x7c.m_count, sizeof(ManagedTexture*), UnknownCompare50ee70);
    if (field_0x7c.m_count > 0) {
        texture = cachePages.Last();
        while (field_0x7c.m_count > 0 && field_0x7c.Get(0)->plannedLevel == 8) {
            static_cast<CacheTexture*>(texture)->PlaceTextures(&field_0x7c, &field_0x144);
            field_0x1cc++;
            texture = cachePages.Previous();
        }
        texture = cachePages.First();
        while (texture) {
            int done = static_cast<CacheTexture*>(texture)->PlaceTextures(&field_0x7c, &field_0x144);
            field_0x1cc++;
            if (done)
                break;
            texture = cachePages.Next();
            if (!texture)
                UnknownFunction464e90();
        }
    }

    field_0x250 = ReadClock();
    if (field_0x144.m_count > 0) {
        int blitted = 0;
        for (i = 0; i < field_0x144.m_count; i++) {
            managed = field_0x144.Get(i);
            managed->field_0x9c = 0;
            blitted += LevelArea(managed->plannedLevel);
        }
        int j = 0;
        int pass = 1;
        while (pass < 3 && field_0x144.m_count > 0 && blitted > field_0x74) {
            managed = field_0x144.Get(j);
            if (managed->field_0xa8 - pass >= 5) {
                blitted -= LevelArea(managed->plannedLevel - managed->field_0x9c);
                managed->field_0x9c = pass;
                if (pass == managed->field_0x98 && managed->IsResident()) {
                    field_0x144.RemoveOrdered(managed);
                } else {
                    blitted += LevelArea(managed->plannedLevel - managed->field_0x9c);
                    j++;
                }
            } else {
                j++;
            }
            if (j >= field_0x144.m_count) {
                j = 0;
                pass++;
            }
        }
        for (i = 0; i < field_0x144.m_count; i++) {
            managed = field_0x144.Get(i);
            managed->field_0x98 = managed->field_0x9c;
            managed->page->CopyMipLevels(managed->region, managed->field_0x9c);
        }
    }

    if (g_TrackGame->debugOverlay) {
        int now = ReadClock();
        field_0x1d4.RecordSample(field_0x1d0);
        field_0x1e0.RecordSample(now - field_0x250);
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
                    field_0x220[managed->plannedLevel]++;
                    field_0x248 += LevelArea(managed->plannedLevel);
                }
                if (managed->wantedLevel > 0)
                    wantedTexels += LevelArea(managed->wantedLevel);
            } while ((texture = managedTextures.Next()) != 0);
        }
        if (field_0x254 < 0)
            field_0x254 = g_TrackGame->debugOverlay->NewPage();
        g_TrackGame->debugOverlay->Title(field_0x254, "TextureManager partial blts");
        g_TrackGame->debugOverlay->Print(field_0x254, "PixelFormat:%s", format);
        g_TrackGame->debugOverlay->Print(
            field_0x254, "CacheTextures:%d %.2fM", cachePages.m_count,
            (PixelFormatBytesPerPixel(groupFormat) * cachePages.m_count << 18) * 3.1789145e-07f);
        g_TrackGame->debugOverlay->Print(field_0x254, "ManagedTextures:%d", managedTextures.m_count);
        g_TrackGame->debugOverlay->Print(field_0x254, "Managed %0.2fM",
                                                                (managedF = (float)managedTexels) * 9.536743e-07f);
        g_TrackGame->debugOverlay->Print(field_0x254, "Ideal   %0.2fM",
                                                                (wantedF = (float)wantedTexels) * 9.536743e-07f);
        g_TrackGame->debugOverlay->Print(field_0x254, "  ratio %0.2fM", wantedF / managedF);
        float peak = (float)field_0x1d4.LargestRecentSample();
        g_TrackGame->debugOverlay->Print(
            field_0x254, "TexMem Blted: %.2fM, (%.2fM)",
            (float)PixelFormatBytesPerPixel(groupFormat) * field_0x1d0 * 9.536743e-07f,
            PixelFormatBytesPerPixel(groupFormat) * peak * 9.536743e-07f);
        g_TrackGame->debugOverlay->Print(field_0x254, "BltTime %d (%d)", now - field_0x250,
                                                                field_0x1e0.LargestRecentSample());
        g_TrackGame->debugOverlay->Print(field_0x254, "256:    %02d        %02d",
                                                                field_0x1fc[8], field_0x220[8]);
        g_TrackGame->debugOverlay->Print(field_0x254, "128:    %02d        %02d",
                                                                field_0x1fc[7], field_0x220[7]);
        g_TrackGame->debugOverlay->Print(field_0x254, " 64:    %02d        %02d",
                                                                field_0x1fc[6], field_0x220[6]);
        g_TrackGame->debugOverlay->Print(field_0x254, " 32:    %02d        %02d",
                                                                field_0x1fc[5], field_0x220[5]);
        g_TrackGame->debugOverlay->Print(
            field_0x254, "TexMemRequested:%.2fM", (float)PixelFormatBytesPerPixel(groupFormat) * field_0x244 * 1.2715658e-06f);
        g_TrackGame->debugOverlay->Print(
            field_0x254, "TexMemGranted:%.2fM", (float)PixelFormatBytesPerPixel(groupFormat) * field_0x248 * 1.2715658e-06f);
        g_TrackGame->debugOverlay->Print(
            field_0x254, "TexMemManaged:%.2fM", (float)PixelFormatBytesPerPixel(groupFormat) * field_0x24c * 1.2715658e-06f);
        g_TrackGame->debugOverlay->Print(field_0x254, "Textures Blted %d", field_0x144.m_count);
    }
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
int ManagedTextureGroup::DrawSelection(PCRenderTarget* target) {
    UnknownSurfaceInterface* surface = target->renderSurface;
    if (groupManager->debugGroup != this)
        return 0;
    if (!g_TrackGame->testKey)
        return 1;
    char text[0x50];
    ManagedTexture* selected = 0;
    TextureMap* page;
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
                if (selected->field_0xa8 > selected->wantedLevel)
                    state = "Hi";
                else if (selected->field_0xa8 < selected->wantedLevel && selected->field_0xa8 != 8)
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
                                          target->surfacePixelFormat, selected->mapFormat, 0, selected->field_0x2c, 0, 0);
                    selected->systemSurface->Unlock(0);
                }
                target->UnlockSurface(0);
            }
            _snprintf(text, sizeof(text), "%d %0.2f->%d,%s", textureCursor + 1, selected->largestUse,
                      selected->field_0xa8, state);
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
        if (!page)
            return 1;
        pageCursor = index;
    } else if (groupManager->debugKeyMode == 2) {
        page = cachePages.First();
        for (int i = 0; i < pageCursor; i++)
            page = cachePages.Next();
    } else {
        return 1;
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
        static_cast<CacheTexture*>(page)->ListTextures(&field_0x1a8);
        long pitch;
        unsigned char* bits = (unsigned char*)target->LockSurface(0, &pitch, 0x821);
        if (bits) {
            for (int j = 0; j < field_0x1a8.m_count; j++) {
                ManagedTexture* managed = field_0x1a8.Get(j);
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
                                              managed->mapFormat, 0, managed->field_0x2c, 0, 0);
                        level->Unlock(0);
                    }
                }
            }
            surface->Unlock(0);
        }
        field_0x1a8.Clear();

        void* dc;
        int failed = surface->GetDC(&dc);
        if (!failed) {
            if (!gdiObject)
                gdiObject = CreatePen(0, 1, 0xff00);
            if (gdiObject)
                SelectObject(dc, gdiObject);
        }
        if (selected) {
            if (failed)
                return 1;
            const ManagedTexture& t = *selected; // reads the fields in retail's order
            float width = (float)page->textureWidth;
            int left = (int)(width * t.pageOffsetU);
            int right = (int)((t.pageScale + t.pageOffsetU) * width);
            float height = (float)page->textureHeight;
            int top = (int)(height * t.pageOffsetV + 20.0f);
            int bottom = (int)((t.pageScale + t.pageOffsetV) * height + 20.0f);
            MoveToEx(dc, left, top, 0);
            LineTo(dc, right, top);
            LineTo(dc, right, bottom);
            LineTo(dc, left, bottom);
            LineTo(dc, left, top);
        } else if (failed) {
            return 1;
        }
        sprintf(text, "#%d", pageCursor + 1);
        TextOutA(dc, 0, 20, text, strlen(text));
        surface->ReleaseDC(dc);
    }
    return 1;
}
