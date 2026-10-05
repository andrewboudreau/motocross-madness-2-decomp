// Near-miss PCTextureMap candidates, kept out of src/reconstructed until
// they match. See docs/PCTEXTUREMAP.md.
//
// PCTextureMap::UnknownFunction4c7b40 (0x004c7b40, 750 bytes): the table
// blit (callers around 0x00404750 pass 0x005777c8 or a per-object table).
// The early BltFast path, both locks, the clipping arithmetic and the
// shared failure return line up; about half the instructions differ in the
// loop nest. Retail keeps left in ebp (with a spilled copy), top in ebx,
// bottom in edi and right on the stack, and the 8-bit source row in esi;
// VC6 here swaps left/bottom and the row pointers. Declaration order of the
// rect locals, row pointers, operand order and indexed loops do not help.
//
// PCTextureMap::UnknownFunction4c8550 (0x004c8550, 393 bytes): the level
// dump. Everything but the 32-bit buffer size matches (391 of 393 bytes):
// retail loads the height and multiplies by the width, VC6 here the other
// way round in every operand order, cast, `<< 2` and sizeof form tried; a
// separate size local or swapped branches are much worse.
//
// PCTextureMap::UnknownVirtualSlot9 (0x004c7640, 385 bytes): the upload.
// With separate `next` surfaces VC6 packs them into the dead parameter
// slots as retail does (the frame matches), but retail keeps `this` in ebp
// from the start, the area in registers across the level loop and tests
// the loop at the top on every pass; VC6 here rotates the loop and pushes
// ebp late. while, for(;;) and goto loops compile alike. 117 of 385 bytes.
//
// PCTextureMap::UnknownVirtualSlot6 (0x004c71c0, 600 bytes): the copy.
// Control flow, both debug `new`s (lines 976/986), the slot 4 and slot 18
// calls and the folded 555/565 key expansion line up. Retail keeps the
// constant 1 in ebx and the copy in ebp, and also stores `field_0x68 & 1`
// to a frame slot. With a `cached` local VC6 here stores it but loses the
// ebx constant; without it, it keeps the constant but drops the store.
//
// PCTextureMap::UnknownVirtualSlot4 (0x004c69c0, 2031 bytes): the setup.
// Everything but the non-mip format fallback chain lines up, including
// the mip chain (same source shape). VC6 cross-jumps the identical
// UnknownFunction4c68e0 call tails of that chain into the first case
// (0x115c); retail merges them into the last (0x235) in mirror order, which
// also changes which blocks interleave their array stores with the pushes.
// if/else, nested negated ifs, switch, goto-to-label, return/goto mixes,
// aggregate initializers and an inline helper all keep VC6's order.

#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/TrackGame.h"

#include "../../src/reconstructed/ManagedTexture.h"

// 0x004c7b40: copies `rect` of +0x70 to (x, y) in `destination`. Without a
// table this is BltFast; with one, both surfaces are locked and each
// destination pixel becomes table[source << 8 | destination], clipped to
// the destination. (16-bit pixels read their destination byte through the
// pixel value, as retail does.)
int PCTextureMap::UnknownFunction4c7b40(unsigned long x, unsigned long y, UnknownSurfaceInterface* destination,
                                        UnknownRect* rect, int flags, unsigned char* table) {
    UnknownSurfaceDesc source;
    UnknownSurfaceDesc target;
    long left, top, right, bottom;
    if (!table) {
        if (destination->UnknownMethod7(x, y, field_0x70, rect, flags))
            goto failed;
        return 1;
    }
    memset(&source, 0, sizeof(source));
    source.size = sizeof(source);
    if (field_0x70->UnknownMethod25(0, &source, 0x811, 0))
        goto failed;
    memset(&target, 0, sizeof(target));
    target.size = sizeof(target);
    if (destination->UnknownMethod25(0, &target, 0x811, 0))
        goto failed;
    left = rect->left;
    top = rect->top;
    right = rect->right;
    bottom = rect->bottom;
    if (x < target.width && y < target.height) {
        if (right - left + x + 1 >= target.width)
            right = target.width - x + left - 2;
        if (bottom - top + y + 1 >= target.height)
            bottom = target.height - y + top - 2;
        if (target.pixelFormat.bitCount == 8) {
            unsigned char* from = (unsigned char*)source.surface + source.pitch * top + left;
            unsigned char* to = (unsigned char*)target.surface + target.pitch * y + x;
            for (long row = top; row <= bottom; row++) {
                unsigned char* pixel = to;
                for (long column = left; column <= right; column++, pixel++)
                    *pixel = table[(from[column - left] << 8) + *pixel];
                from += source.pitch;
                to += target.pitch;
            }
        } else {
            unsigned short* from = (unsigned short*)source.surface + source.pitch * top / 2 + left;
            unsigned short* to = (unsigned short*)target.surface + target.pitch * y / 2 + x;
            for (long row = top; row <= bottom; row++) {
                unsigned short* pixel = to;
                for (long column = left; column <= right; column++, pixel++)
                    *pixel = table[(from[column - left] << 8) + *(unsigned char*)*pixel];
                from += source.pitch / 2;
                to += target.pitch / 2;
            }
        }
    }
    if (destination->UnknownMethod32(0) || field_0x70->UnknownMethod32(0))
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004c8550: writes a locked level to C:\temp\<name><nnn>.bmp (8-bit) or
// .tga (anything else, converted to 32-bit first), taking the first number
// with no existing file. `name` defaults to "tex".
void PCTextureMap::UnknownFunction4c8550(UnknownSurfaceDesc* desc, const char* name) {
    char path[260];
    UnknownBitmapFile bitmap;
    const char* extension = desc->pixelFormat.bitCount == 8 ? ".bmp" : ".tga";
    int number = 0;
    if (!name)
        name = "tex";
    FILE* file = 0;
    do {
        if (file)
            fclose(file);
        sprintf(path, "C:\\temp\\%s%03d%s", name, number, extension);
        file = fopen(path, "r");
        number++;
    } while (file);
    if (desc->pixelFormat.bitCount == 8) {
        UnknownFunction4245f0(&bitmap, desc->surface, field_0x2c->field_0x010, desc->width, desc->height);
        int length = strlen(path);
        int count = length > 0x7f ? 0x7f : length;
        strncpy(bitmap.name, path, count);
        bitmap.name[count] = 0;
        UnknownFunction424380(&bitmap);
    } else {
        void* pixels = DebugMalloc(desc->width * desc->height * sizeof(unsigned int), __FILE__, 2149);
        UnknownFunction4d1d20(pixels, desc->surface, desc->width, desc->height, desc->width,
                              desc->pitch / UnknownFunction511970(field_0x20), 0x22b8, field_0x20, 0, 0, 0x80,
                              0xff00ff);
        UnknownFunction5127f0(pixels, desc->width, desc->height, 0, path, 32);
        operator delete(pixels, __FILE__, 2154);
    }
}

// 0x004c7640: with partial texture blits (Display+0x5bc) or a positive
// `mode`, copies `rect` (or the whole texture) down the mip chain with
// BltFast, halving it per level; otherwise lets the device Load it. Then
// applies the colour key.
int PCTextureMap::UnknownVirtualSlot9(UnknownRect* rect, int mode) {
    UnknownSurfaceInterface* source = field_0x70;
    UnknownSurfaceInterface* destination = field_0x74;
    if (!source || !destination)
        return 0;
    if (mode > 0 || mode == -1 && g_UnknownGlobal56e26c->field_0x0c->field_0x5bc > 0 ||
        g_UnknownGlobal56e26c->field_0x0c->field_0x5bc <= 0) {
        UnknownRect area;
        if (rect && g_UnknownGlobal56e26c->field_0x0c->field_0x5bc > 0) {
            area = *rect;
        } else {
            area.left = 0;
            area.top = 0;
            area.right = field_0x14;
            area.bottom = field_0x18;
        }
        UnknownSurfaceCaps caps;
        memset(&caps, 0, sizeof(caps));
        caps.caps = 0x401000;
        while (area.right - area.left > 0 && area.bottom - area.top > 0) {
            if (destination->UnknownMethod7(area.left, area.top, source, &area, 0x10))
                return 0;
            UnknownSurfaceInterface* nextSource;
            UnknownSurfaceInterface* nextDestination;
            long sourceResult = source->UnknownMethod12(&caps, &nextSource);
            long destinationResult = destination->UnknownMethod12(&caps, &nextDestination);
            if (sourceResult || destinationResult)
                break;
            source = nextSource;
            destination = nextDestination;
            area.top >>= 1;
            area.left >>= 1;
            area.bottom >>= 1;
            area.right >>= 1;
        }
        if (field_0x30 && field_0x74->UnknownMethod29(8, &field_0x34))
            return 0;
        return 1;
    }
    if (destination != source)
        ((PCRenderTarget*)g_UnknownGlobal56e26c->field_0x10)
            ->field_0x50->UnknownMethod43(destination, 0, source, 0, 0);
    return 0;
}

// 0x004c71c0: a copy of the texture (a ManagedTexture registered with this
// one's cache, or a PCTextureMap of the game's manager), filled from +0x70
// through its slot 4 and given the same colour key; 0 on failure.
TextureMap* PCTextureMap::UnknownVirtualSlot6() {
    TextureMap* copy;
    TextureMapManager* manager;
    int cached = field_0x68 & 1;
    if (cached) {
        manager = field_0x10;
        ManagedTexture* cache = new(__FILE__, 976) ManagedTexture(manager);
        cache->field_0x68 |= 1;
        ((ManagedTexture*)this)->field_0x90->UnknownFunction50c6c0(cache);
        copy = cache;
    } else {
        manager = g_UnknownGlobal56e26c->field_0x3c;
        copy = new(__FILE__, 986) PCTextureMap(manager, 1);
    }
    if (field_0x70) {
        UnknownSurfaceDesc desc;
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (!field_0x70->UnknownMethod25(0, &desc, 0x811, 0)) {
            int flags = (desc.caps[0] & 0x1000 ? 0 : 4) | (desc.caps[0] & 0x30000000 ? 0 : 8);
            int created = copy->UnknownVirtualSlot4(desc.surface, field_0x14, field_0x18,
                                                    desc.pitch / UnknownFunction511970(field_0x20), field_0x1c,
                                                    field_0x20, field_0x20, field_0x2c, flags, field_0x78, 0, 0,
                                                    2, 1, 0, 0x80, 0xff00ff);
            if (!field_0x70->UnknownMethod32(0) && created) {
                if (field_0x30) {
                    if (field_0x20 == 8) {
                        copy->UnknownVirtualSlot18((field_0x2c->field_0x010[field_0x34][0] << 8 |
                                                    field_0x2c->field_0x010[field_0x34][1]) << 8 |
                                                   field_0x2c->field_0x010[field_0x34][2]);
                    } else {
                        unsigned int key = field_0x34;
                        unsigned int color;
                        if (field_0x20 == 0x22b)
                            color = (key & 0x7c00) << 9 | (key & 0x3e0) << 6 | (key & 0x1f) << 3;
                        else
                            color = (key & 0xf800) << 8 | (key & 0x7e0) << 5 | (key & 0x1f) << 3;
                        copy->UnknownVirtualSlot18(color);
                    }
                }
                return copy;
            }
        }
    }
    delete copy;
    return 0;
}

// Appends a render-state pair for slot 19 to apply.
static inline void AppendRenderStatePair(TextureMap* map, int state, int value) {
    map->field_0x48[map->field_0x44].state = state;
    map->field_0x48[map->field_0x44].value = value;
    map->field_0x44++;
}

// 0x004c69c0: sets the texture up from `bits` (or empty): picks the format,
// counts the mip levels down to `minimumSize`, reuses a shared surface or
// creates one (trying formats in order), converts the bits into it, builds
// the mip levels and records the alpha or colour-key render states.
int PCTextureMap::UnknownVirtualSlot4(void* bits, int width, int height, int stride, int minimumSize,
                                      int sourceFormat, int format, UnknownTexturePalette* palette,
                                      int flags, void* surfacePalette, int checkMemory, int unused,
                                      int addressU, int addressV, UnknownTextureFormatChoice* choice,
                                      int alphaThreshold, unsigned int key) {
    int formats[5];
    int mipmapped;
    UnknownSurfaceDesc desc;
    unsigned char pixelFormat[0x20];
    UnknownSurfaceCaps caps;
    int shared;
    field_0x14 = width;
    field_0x18 = height;
    field_0x1c = minimumSize;
    field_0x2c = palette;
    field_0x78 = surfacePalette;
    if (choice)
        format = UnknownFunction511ad0(format) ? choice->field_0x10 : choice->field_0x0c;
    field_0x20 = format;
    UnknownFunction5119c0(format, pixelFormat);
    mipmapped = flags & 2;
    field_0x24 = 1;
    if (!mipmapped) {
        field_0x1c = width;
        if (!field_0x70) {
            memset(&desc, 0, sizeof(desc));
            desc.size = sizeof(desc);
            desc.height = height;
            desc.width = width;
            memcpy(&desc.pixelFormat, pixelFormat, sizeof(pixelFormat));
            desc.flags = (~flags & 4) << 10 | 7;
            shared = 0;
            if (flags & 4) {
                desc.caps[0] = 0x800;
            } else {
                desc.caps[0] = 0x1000;
                if (!(flags & 8) && !g_UnknownGlobal56e26c->field_0x2d0 &&
                    !(g_UnknownGlobal56e26c->field_0x2d5_bit2) &&
                    (field_0x20 == 0x22b || field_0x20 == 0x235)) {
                    shared = 1;
                    if (g_UnknownSharedSurfaces68a394[field_0x24]) {
                        field_0x70 = g_UnknownSharedSurfaces68a394[field_0x24];
                        field_0x70->UnknownMethod1();
                    } else {
                        desc.caps[0] = 0x1800;
                    }
                } else {
                    desc.caps[0] = 0x1800;
                }
            }
            if (!field_0x70) {
                if (field_0x20 == 0x115c) {
                    formats[0] = field_0x20;
                    formats[1] = 0x613;
                    formats[2] = 0x22b;
                    formats[3] = 0x235;
                    formats[4] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                } else if (field_0x20 == 0x613) {
                    formats[0] = field_0x20;
                    formats[1] = 0x22b;
                    formats[2] = 0x235;
                    formats[3] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                } else if (field_0x20 == 8) {
                    formats[0] = field_0x20;
                    formats[1] = 0x22b;
                    formats[2] = 0x235;
                    formats[3] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                } else if (field_0x20 == 0x22b) {
                    formats[0] = field_0x20;
                    formats[1] = 0x235;
                    formats[2] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                } else if (field_0x20 == 0x235) {
                    formats[0] = field_0x20;
                    formats[1] = 0x22b;
                    formats[2] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                } else if (g_UnknownGlobal56e26c->field_0x0c->field_0x190->UnknownMethod6(&desc, &field_0x70, 0)) {
                    goto failed;
                }
            }
            if (shared) {
                g_UnknownSharedSurfaces68a394[field_0x24] = field_0x70;
                field_0x70->UnknownMethod1();
            }
            if (field_0x78 && field_0x20 == 8 && field_0x70->UnknownMethod31(field_0x78))
                goto failed;
        }
    } else {
        int levelWidth = width;
        int levelHeight = height;
        for (;;) {
            levelWidth >>= 1;
            levelHeight >>= 1;
            if (levelWidth < minimumSize && levelHeight < minimumSize)
                break;
            field_0x24++;
        }
        if (bits && sourceFormat != field_0x20 &&
            (!checkMemory || UnknownFunction511970(0x235) > UnknownFunction511970(sourceFormat)) &&
            !field_0x10->UnknownFunction511370(UnknownFunction511970(0x235) * width * height))
            return 0;
        if (!field_0x70) {
            memset(&desc, 0, sizeof(desc));
            desc.size = sizeof(desc);
            desc.height = height;
            desc.width = width;
            desc.mipMapCount = field_0x24;
            memcpy(&desc.pixelFormat, pixelFormat, sizeof(pixelFormat));
            shared = 0;
            desc.flags = 0x21007;
            if (flags & 4) {
                desc.caps[0] = 0x400808;
            } else {
                desc.caps[0] = 0x401008;
                if (!(flags & 8) && !g_UnknownGlobal56e26c->field_0x2d0 &&
                    !(g_UnknownGlobal56e26c->field_0x2d5_bit2) &&
                    (field_0x20 == 0x22b || field_0x20 == 0x235)) {
                    shared = 1;
                    if (g_UnknownSharedMipSurfaces68a36c[field_0x24]) {
                        field_0x70 = g_UnknownSharedMipSurfaces68a36c[field_0x24];
                        field_0x70->UnknownMethod1();
                    } else {
                        desc.caps[0] = 0x401808;
                    }
                } else {
                    desc.caps[0] = 0x401808;
                }
            }
            if (!field_0x70) {
                if (field_0x20 == 0x115c) {
                    formats[0] = field_0x20;
                    formats[1] = 0x613;
                    formats[2] = 0x22b;
                    formats[3] = 0x235;
                    formats[4] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                } else if (field_0x20 == 0x613) {
                    formats[0] = field_0x20;
                    formats[1] = 0x22b;
                    formats[2] = 0x235;
                    formats[3] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                } else if (field_0x20 == 8) {
                    formats[0] = field_0x20;
                    formats[1] = 0x22b;
                    formats[2] = 0x235;
                    formats[3] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                } else if (field_0x20 == 0x22b) {
                    formats[0] = field_0x20;
                    formats[1] = 0x235;
                    formats[2] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                } else if (field_0x20 == 0x235) {
                    formats[0] = field_0x20;
                    formats[1] = 0x22b;
                    formats[2] = 0;
                    if (!UnknownFunction4c68e0(&desc, flags, formats))
                        goto failed;
                }
            }
            if (shared) {
                g_UnknownSharedMipSurfaces68a36c[field_0x24] = field_0x70;
                field_0x70->UnknownMethod1();
            }
            if (field_0x78 && field_0x20 == 8 && field_0x70->UnknownMethod31(field_0x78))
                goto failed;
        }
    }
    if (field_0x70->UnknownMethod14(&caps))
        goto failed;
    if (bits) {
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (field_0x70->UnknownMethod25(0, &desc, 0x801, 0))
            goto failed;
        void* destination = desc.surface;
        int bytesPerPixel = UnknownFunction511970(field_0x20);
        UnknownFunction4d1d20(destination, bits, width, height, desc.pitch / bytesPerPixel, stride, field_0x20,
                              sourceFormat, 0, palette, alphaThreshold, key);
        if (!(flags & 4))
            field_0x3c = UnknownFunction4d24d0(destination, desc.width, desc.height, desc.pitch / bytesPerPixel,
                                               field_0x20, palette);
        if (field_0x70->UnknownMethod32(0))
            goto failed;
        if (mipmapped && !UnknownVirtualSlot15(2))
            goto failed;
    }
    if (g_UnknownGlobal689968)
        UnknownFunction4c84e0(field_0x70, 0);
    if (UnknownFunction511ad0(format) & !UnknownFunction511ad0(field_0x20)) {
        UnknownVirtualSlot18(0xff00ff);
        return 1;
    }
    if (UnknownFunction511ad0(field_0x20)) {
        AppendRenderStatePair(this, 0x29, 0);
        AppendRenderStatePair(this, 0x1b, 1);
        AppendRenderStatePair(this, 0x13, addressU);
        AppendRenderStatePair(this, 0x14, addressV);
    } else {
        AppendRenderStatePair(this, 0x29, 0);
        AppendRenderStatePair(this, 0x1b, 0);
    }
    return 1;
failed:
    return 0;
}
