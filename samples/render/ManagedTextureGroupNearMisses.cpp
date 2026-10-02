// Near-miss ManagedTextureGroup candidates (TextureCache.cpp), kept out of
// src/reconstructed until they match. See docs/TEXTUREMAPMANAGER.md.
//
// ManagedTextureGroup::UnknownFunction50c960 (0x0050c960, 3742 bytes): the
// repack. The control flow, every call and the inlining pattern line up:
// VC6 inlines ContainerList::Reserve only in the last two Add calls, as
// retail does, once the TexMem figures go through TexelMegabytes and
// PageMegabytes and the page search through LeastWantedPage (VC6's inline
// budget scales with the function's size, so without those helpers it
// inlines every Reserve). What remains is stack-slot packing: retail
// overlaps the 12-byte format buffer with three scalars and keeps "now" in
// ebp through the overlay block; VC6 here packs the slots in another order
// (a frame 8 bytes larger). About 250 of 1150 instructions differ, almost
// all in register or slot choice. Declaration order and block scopes do
// not change the packing.
//
// ManagedTextureGroup::UnknownFunction50ef70 (0x0050ef70, 1832 bytes): the
// debug display behind manager slot 15. Everything lines up except that
// retail saves ebx/esi only after the "not the selected group" return
// (VC6 here saves all four registers in the prologue, shifting the stack
// offsets of the entry block and the return-1 epilogues), two stack slots
// (the two device contexts, the page) and one block's scheduling. About 60
// of 550 instructions differ.

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

static inline float TexelMegabytes(ManagedTextureGroup* group, int texels) {
    return (float)UnknownFunction511970(group->field_0x0c) * texels * 1.2715658e-06f;
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

static inline float PageMegabytes(ManagedTextureGroup* group, int pages) {
    return (UnknownFunction511970(group->field_0x0c) * pages << 18) * 3.1789145e-07f;
}

// 0x0050c960: repacks the pages. Each texture used since the last repack
// is planned at the level its use asks for (at least 5, at most its own);
// unused ones give up their place. The pages least wanted by the planned
// textures are emptied until the textures that need a place fit, and are
// refilled largest level first; the figures go to the debug overlay.
void ManagedTextureGroup::UnknownFunction50c960() {
    field_0x1d0 = 0;
    UnknownFunction4bfa80();
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
    texture = field_0x44.First();
    while (texture) {
        ManagedTexture* managed = static_cast<ManagedTexture*>(texture);
        field_0x24c += LevelArea(managed->UnknownFunction510990());
        if (!managed->field_0xa0) {
            managed->field_0xac = -1;
            int placed = managed->field_0xa8;
            if (placed >= 0) {
                unused[placed]++;
                field_0x90[placed].Add(managed);
            }
            texture = field_0x44.Next();
            continue;
        }
        int level = (int)managed->field_0xa4;
        if (level > managed->UnknownFunction510990())
            level = managed->UnknownFunction510990();
        if (level < 5)
            level = 5;
        managed->field_0xac = level;
        managed->field_0xb0 = level;
        field_0x1fc[level]++;
        field_0x244 += LevelArea(level);
        if (managed->field_0xa8 > level)
            level = managed->field_0xa8;
        managed->field_0xac = level;
        managed->field_0xb8 = field_0x40->field_0x74;
        field_0x7c.Add(managed);
        texture = field_0x44.Next();
        texels += LevelArea(level);
    }

    UnknownFunction50d800(&field_0x7c, field_0x3c);
    int i;
    for (i = field_0x7c.m_count - 1; i >= 0; i--) {
        ManagedTexture* managed = field_0x7c.Get(i);
        if (managed->field_0xa8 <= 0) {
            field_0x7c.RemoveOrdered(managed);
            field_0x158.Add(managed);
        } else if (managed->field_0xa8 < managed->field_0xac) {
            field_0x7c.RemoveOrdered(managed);
            field_0x16c.Add(managed);
        }
    }

    texels = 0;
    for (i = 0; i < field_0x158.m_count; i++)
        texels += field_0x158.Get(i)->field_0x14 * field_0x158.Get(i)->field_0x18;

    for (texture = field_0x54.First(); texture; texture = field_0x54.Next()) {
        CacheTexture* page = static_cast<CacheTexture*>(texture);
        page->field_0x18c = -(page->field_0x14 * page->field_0x18);
        page->UnknownFunction50f890(&field_0x180);
        for (int j = 0; j < field_0x180.m_count; j++) {
            ManagedTexture* managed = field_0x180.Get(j);
            if (managed->field_0xac > 0)
                page->field_0x18c += LevelArea(managed->field_0xac);
        }
    }

    qsort(field_0x16c.m_data, field_0x16c.m_count, sizeof(ManagedTexture*), UnknownCompare50eeb0);
    int smallest = 0x7fffffff;
    if (field_0x16c.m_count > 0)
        smallest = LevelArea(field_0x16c.Get(0)->field_0xac);
    int freed = 0;
    while (field_0x54.m_count > 0) {
        int lowest;
        CacheTexture* best = LeastWantedPage(&field_0x54, &lowest);
        int enough = freed >= texels;
        int full = freed >= field_0x158.m_count << 10;
        int limit = emptied.m_count >= field_0x78;
        int worthless = lowest <= -smallest;
        if (full && (limit || (enough && !worthless)))
            break;
        field_0x54.Remove(best);
        emptied.Append(best);
        best->UnknownFunction50f890(&field_0x194);
        freed += best->field_0x14 * best->field_0x18;
        for (int j = 0; j < field_0x194.m_count; j++) {
            ManagedTexture* managed = field_0x194.Get(j);
            if (managed->field_0xa4 > 0.0f && !field_0x158.Contains(managed) && field_0x158.Add(managed))
                texels += managed->field_0x14 * managed->field_0x18;
            field_0x7c.Remove(managed);
            field_0x16c.Remove(managed);
            managed->UnknownFunction510700();
        }
        best->UnknownFunction50fc40();
    }

    freed -= UnknownFunction50d800(&field_0x158, freed); // now the spare texels
    while (freed > 0 && field_0x16c.m_count > 0) {
        ManagedTexture* managed = field_0x16c.Get(0);
        if (LevelArea(managed->field_0xac) > freed)
            break;
        freed -= LevelArea(managed->field_0xac);
        field_0x158.Add(managed);
        field_0x16c.Remove(managed);
        managed->UnknownFunction510700();
    }

    qsort(field_0x158.m_data, field_0x158.m_count, sizeof(ManagedTexture*), UnknownCompare50ee70);
    int levels[9];
    memset(levels, 0, sizeof(levels));
    for (i = 0; i < field_0x158.m_count; i++)
        levels[field_0x158.Get(i)->field_0xac]++;
    field_0x250 = UnknownFunction4bfa80();
    int filled = 0;
    for (texture = emptied.First(); texture; texture = emptied.Next()) {
        if (field_0x158.m_count <= 0)
            break;
        CacheTexture* page = static_cast<CacheTexture*>(texture);
        filled++;
        page->UnknownFunction50f9b0(levels, 8);
        page->UnknownFunction50fdb0(&field_0x158, 0);
        page->UnknownVirtualSlot9(0, -1);
    }
    field_0x54.AppendList(&emptied);

    if (g_UnknownGlobal56e26c->field_0x38) {
        int now = UnknownFunction4bfa80();
        field_0x1d4.UnknownFunction4cb6b0(field_0x1d0);
        field_0x1e0.UnknownFunction4cb6b0(now - field_0x250);
        char format[12];
        strcpy(format, "Unknown");
        texture = field_0x44.First();
        if (texture) {
            sprintf(format, "%d", texture->field_0x20);
            do {
                ManagedTexture* managed = static_cast<ManagedTexture*>(texture);
                if (managed->field_0xac > 0) {
                    field_0x220[managed->field_0xac]++;
                    field_0x248 += LevelArea(managed->field_0xac);
                }
            } while ((texture = field_0x44.Next()) != 0);
        }
        if (field_0x254 < 0)
            field_0x254 = g_UnknownGlobal56e26c->field_0x38->NewPage();
        field_0x1ec.UnknownFunction4cb6b0(filled);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447fa0(field_0x254, "TextureManager no partial");
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x254, "PixelFormat:%s", format);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(
            field_0x254, "CacheTextures:%d %.2fM", field_0x54.m_count,
            PageMegabytes(this, field_0x54.m_count));
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x254, "ManagedTextures:%d", field_0x44.m_count);
        float peak = (float)field_0x1d4.UnknownFunction4cb690();
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(
            field_0x254, "TexMem Blted: %.2fM, Peak: %.2fM",
            (float)UnknownFunction511970(field_0x0c) * field_0x1d0 * 9.536743e-07f,
            UnknownFunction511970(field_0x0c) * peak * 9.536743e-07f);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x254, "BltTime %d (%d)", now - field_0x250,
                                                                field_0x1e0.UnknownFunction4cb690());
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x254, "Size Requested Granted");
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x254, "256:    %02d        %02d",
                                                                field_0x1fc[8], field_0x220[8]);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x254, "128:    %02d        %02d",
                                                                field_0x1fc[7], field_0x220[7]);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x254, " 64:    %02d        %02d",
                                                                field_0x1fc[6], field_0x220[6]);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x254, " 32:    %02d        %02d",
                                                                field_0x1fc[5], field_0x220[5]);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(
            field_0x254, "TexMemRequested:%.2fM", TexelMegabytes(this, field_0x244));
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(
            field_0x254, "TexMemGranted:%.2fM", TexelMegabytes(this, field_0x248));
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(
            field_0x254, "TexMemManaged:%.2fM", TexelMegabytes(this, field_0x24c));
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x254, "Textures Blted %d (%d)", filled,
                                                                field_0x1ec.UnknownFunction4cb690());
    }
    field_0x158.Clear();
    field_0x16c.Clear();
    field_0x180.Clear();
    field_0x194.Clear();
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
int ManagedTextureGroup::UnknownFunction50ef70(PCRenderTarget* target) {
    UnknownSurfaceInterface* surface = target->field_0x48;
    if (field_0x40->field_0x3c != this)
        return 0;
    if (!g_UnknownGlobal56e26c->field_0x2d4_bit2)
        return 1;
    char text[0x50];
    void* dc;
    ManagedTexture* selected = 0;
    TextureMap* page;
    int mode = field_0x40->field_0x58;
    if (mode == 1 && field_0x1c8 >= 0 && field_0x1c8 < field_0x44.m_count) {
        int index = field_0x1c8;
        selected = static_cast<ManagedTexture*>(field_0x44.First());
        while (selected && index--)
            selected = static_cast<ManagedTexture*>(field_0x44.Next());
        if (selected) {
            const char* state = "";
            if (!selected->field_0xa0 && selected->field_0x80)
                state = "Un";
            else if (selected->field_0xa4 > 0.0f) {
                if (selected->field_0xa8 > selected->field_0xb0)
                    state = "Hi";
                else if (selected->field_0xa8 < selected->field_0xb0 && selected->field_0xa8 != 8)
                    state = "Lo";
            }
            int x = target->field_0x0c - selected->field_0x14;
            long pitch;
            unsigned char* bits = (unsigned char*)target->UnknownVirtualSlot4(0, &pitch, 0x821);
            if (bits) {
                long sourcePitch;
                void* source = selected->UnknownVirtualSlot13(0, &sourcePitch, 0x811);
                if (source) {
                    int sourceSize = UnknownFunction511970(selected->field_0x20);
                    int size = UnknownFunction511970(target->field_0x28);
                    UnknownFunction4d1d20(bits + x * size + 20 * pitch, source, selected->field_0x14,
                                          selected->field_0x18, pitch / size, sourcePitch / sourceSize,
                                          target->field_0x28, selected->field_0x20, 0, selected->field_0x2c, 0, 0);
                    selected->field_0x70->UnknownMethod32(0);
                }
                target->UnknownVirtualSlot5(0);
            }
            _snprintf(text, sizeof(text), "%d %0.2f->%d,%s", field_0x1c8 + 1, selected->field_0xa4,
                      selected->field_0xa8, state);
            if (!target->field_0x48->UnknownMethod17(&dc)) {
                TextOutA(dc, x, 20, text, strlen(text));
                target->field_0x48->UnknownMethod26(dc);
            }
        }
    }
    mode = field_0x40->field_0x58;
    if (mode == 1 && selected) {
        int index = 0;
        for (page = field_0x54.First(); page; page = field_0x54.Next(), index++) {
            if (page == selected->field_0x80)
                break;
        }
        if (!page)
            return 1;
        field_0x1c4 = index;
    } else if (mode == 2) {
        page = field_0x54.First();
        for (int i = 0; i < field_0x1c4; i++)
            page = field_0x54.Next();
    } else {
        return 1;
    }
    if (!page)
        return 1;

    UnknownBltEffects effects;
    memset(&effects, 0, sizeof(effects));
    effects.size = sizeof(effects);
    effects.fillColor = 0;
    UnknownBltRect rect;
    rect.left = 0;
    rect.top = 20;
    rect.right = page->field_0x14;
    rect.bottom = page->field_0x18 + 20;
    surface->UnknownMethod5(&rect, 0, 0, 0x400, &effects);
    static_cast<CacheTexture*>(page)->UnknownFunction50f890(&field_0x1a8);
    long pitch;
    unsigned char* bits = (unsigned char*)target->UnknownVirtualSlot4(0, &pitch, 0x821);
    if (bits) {
        for (int j = 0; j < field_0x1a8.m_count; j++) {
            ManagedTexture* managed = field_0x1a8.Get(j);
            UnknownTextureRegion* region = managed->field_0x94;
            int left = (int)(page->field_0x14 * region->field_0x1c);
            int top = (int)(page->field_0x18 * region->field_0x20) + 20;
            int side = (int)(managed->field_0x84 * 256.0f);
            int levelSide = managed->field_0x14;
            UnknownSurfaceInterface* level = managed->field_0x70;
            UnknownSurfaceDesc desc;
            memset(&desc, 0, sizeof(desc));
            desc.size = sizeof(desc);
            level->UnknownMethod22(&desc);
            while (side < levelSide) {
                levelSide >>= 1;
                level->UnknownMethod12((UnknownSurfaceCaps*)desc.caps, &level);
                level->UnknownMethod22(&desc);
            }
            if (desc.height == side && desc.width == side) {
                UnknownSurfaceDesc locked;
                memset(&locked, 0, sizeof(locked));
                locked.size = sizeof(locked);
                if (!level->UnknownMethod25(0, &locked, 0x811, 0)) {
                    int sourceSize = UnknownFunction511970(managed->field_0x20);
                    int size = UnknownFunction511970(target->field_0x28);
                    UnknownFunction4d1d20(bits + top * pitch + left * size, locked.surface, levelSide, levelSide,
                                          pitch / size, locked.pitch / sourceSize, target->field_0x28,
                                          managed->field_0x20, 0, managed->field_0x2c, 0, 0);
                    level->UnknownMethod32(0);
                }
            }
        }
        surface->UnknownMethod32(0);
    }
    field_0x1a8.Clear();

    int failed = surface->UnknownMethod17(&dc);
    if (!failed) {
        if (!field_0x1f8)
            field_0x1f8 = CreatePen(0, 1, 0xff00);
        if (field_0x1f8)
            SelectObject(dc, field_0x1f8);
    }
    if (selected) {
        if (failed)
            return 1;
        float width = (float)page->field_0x14;
        int left = (int)(selected->field_0x88 * width);
        int right = (int)((selected->field_0x84 + selected->field_0x88) * width);
        float height = (float)page->field_0x18;
        int top = (int)(selected->field_0x8c * height + 20.0f);
        int bottom = (int)((selected->field_0x84 + selected->field_0x8c) * height + 20.0f);
        MoveToEx(dc, left, top, 0);
        LineTo(dc, right, top);
        LineTo(dc, right, bottom);
        LineTo(dc, left, bottom);
        LineTo(dc, left, top);
    } else if (failed) {
        return 1;
    }
    sprintf(text, "#%d", field_0x1c4 + 1);
    TextOutA(dc, 0, 20, text, strlen(text));
    surface->UnknownMethod26(dc);
    return 1;
}
