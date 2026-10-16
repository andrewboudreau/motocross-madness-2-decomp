// Near-miss PCTextureMap candidates, kept out of src/reconstructed until
// they match. See docs/PCTEXTUREMAP.md.
//
// PCTextureMap::CopyRectTo (0x004c7b40, 750 bytes): the table
// blit (callers around 0x00404750 pass 0x005777c8 or a per-object table).
// The early BltFast path, both locks, the clipping arithmetic and the
// shared failure return line up; about half the instructions differ in the
// loop nest. Retail keeps left in ebp (with a spilled copy), top in ebx,
// bottom in edi and right on the stack, and the 8-bit source row in esi;
// VC6 here swaps left/bottom and the row pointers. Declaration order of the
// rect locals, row pointers, operand order and indexed loops do not help.
//
// PCTextureMap::Clone (0x004c71c0, 600 bytes): the copy.
// Everything lines up except that VC6 here keeps the constant 1 in ebp and
// the copy in ebx (retail: ebx and ebp), which also moves the 8-bit key
// expansion's scratch registers. The shape that gets this close: the
// cached flag normalised to 0/1 (`(field_0x68 & 1) != 0`, stored in the
// slot the `new` temporary reuses), the manager in a local before each
// `new`, the copy assigned directly (a separate ManagedTexture local
// assigns ebp before the registration call; retail after), the two
// caps-derived flags as separate `if`s, and the ManagedTexture `new`
// first (its EH state is 0). `int cached = field_0x68 & 1` keeps the
// store but not the constant register; `if (field_0x68 & 1)` keeps the
// register but tests the byte in place.
//
// PCTextureMap::CreateSurface (0x004c69c0, 2031 bytes): the setup.
// Everything but the non-mip format fallback chain lines up, including
// the mip chain (same source shape). VC6 cross-jumps the identical
// CreateSystemSurface call tails of that chain into the first case
// (4444); retail merges them into the last (565) in mirror order, which
// also changes which blocks interleave their array stores with the pushes.
// if/else, nested negated ifs, switch, goto-to-label, return/goto mixes,
// aggregate initializers and an inline helper all keep VC6's order.

#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/TrackGame.h"

#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/D3DConstants.h"

// 0x004c7b40: copies `rect` of +0x70 to (x, y) in `destination`. Without a
// table this is BltFast; with one, both surfaces are locked and each
// destination pixel becomes table[source << 8 | destination], clipped to
// the destination. (16-bit pixels read their destination byte through the
// pixel value, as retail does.)
int PCTextureMap::CopyRectTo(unsigned long x, unsigned long y, UnknownSurfaceInterface* destination,
                                        UnknownRect* rect, int flags, unsigned char* table) {
    UnknownSurfaceDesc source;
    UnknownSurfaceDesc target;
    long left, top, right, bottom;
    if (!table) {
        if (destination->BltFast(x, y, systemSurface, rect, flags))
            goto failed;
        return 1;
    }
    memset(&source, 0, sizeof(source));
    source.size = sizeof(source);
    if (systemSurface->Lock(0, &source, 0x811, 0))
        goto failed;
    memset(&target, 0, sizeof(target));
    target.size = sizeof(target);
    if (destination->Lock(0, &target, 0x811, 0))
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
    if (destination->Unlock(0) || systemSurface->Unlock(0))
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004c71c0: a copy of the texture (a ManagedTexture registered with this
// one's cache, or a PCTextureMap of the game's manager), filled from +0x70
// through its slot 4 and given the same colour key; 0 on failure.
TextureMap* PCTextureMap::Clone() {
    TextureMap* copy;
    TextureMapManager* manager;
    int cached = (mapFlags & 1) != 0;
    if (cached) {
        manager = ownerManager;
        copy = new(__FILE__, 976) ManagedTexture(manager);
        copy->mapFlags |= 1;
        ((ManagedTexture*)this)->group->AddTexture((ManagedTexture*)copy);
    } else {
        manager = g_TrackGame->textureManager;
        copy = new(__FILE__, 986) PCTextureMap(manager, 1);
    }
    if (systemSurface) {
        UnknownSurfaceDesc desc;
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (!systemSurface->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_READONLY | DDLOCK_NOSYSLOCK, 0)) {
            int systemMemory = 0;
            int noAlpha = 0;
            if (!(desc.caps[0] & 0x30000000))
                systemMemory = 8;
            if (!(desc.caps[0] & 0x1000))
                noAlpha = 4;
            int created = copy->CreateSurface(desc.surface, textureWidth, textureHeight,
                                              desc.pitch / PixelFormatBytesPerPixel(mapFormat), field_0x1c,
                                              mapFormat, mapFormat, field_0x2c, noAlpha | systemMemory,
                                              texturePalette, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
            if (!systemSurface->Unlock(0) && created) {
                if (usesColorKey) {
                    if (mapFormat == 8) {
                        copy->ApplyColorKey((field_0x2c->field_0x010[colorKeyValue][0] << 8 |
                                             field_0x2c->field_0x010[colorKeyValue][1]) << 8 |
                                                   field_0x2c->field_0x010[colorKeyValue][2]);
                    } else {
                        unsigned int key = colorKeyValue;
                        unsigned int color;
                        if (mapFormat == 555)
                            color = (key & 0x7c00) << 9 | (key & 0x3e0) << 6 | (key & 0x1f) << 3;
                        else
                            color = (key & 0xf800) << 8 | (key & 0x7e0) << 5 | (key & 0x1f) << 3;
                        copy->ApplyColorKey(color);
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
    map->statePairs[map->statePairCount].state = state;
    map->statePairs[map->statePairCount].value = value;
    map->statePairCount++;
}

// 0x004c69c0: sets the texture up from `bits` (or empty): picks the format,
// counts the mip levels down to `minimumSize`, reuses a shared surface or
// creates one (trying formats in order), converts the bits into it, builds
// the mip levels and records the alpha or colour-key render states.
int PCTextureMap::CreateSurface(void* bits, int width, int height, int stride, int minimumSize,
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
    textureWidth = width;
    textureHeight = height;
    field_0x1c = minimumSize;
    field_0x2c = palette;
    texturePalette = surfacePalette;
    if (choice)
        format = IsFormat4444Or8888(format) ? choice->field_0x10 : choice->field_0x0c;
    mapFormat = format;
    FillPixelFormat(format, pixelFormat);
    mipmapped = flags & 2;
    mipCount = 1;
    if (!mipmapped) {
        field_0x1c = width;
        if (!systemSurface) {
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
                if (!(flags & 8) && !g_TrackGame->field_0x2d0 &&
                    !(g_TrackGame->field_0x2d5_bit2) &&
                    (mapFormat == 555 || mapFormat == 565)) {
                    shared = 1;
                    if (g_UnknownSharedSurfaces68a394[mipCount]) {
                        systemSurface = g_UnknownSharedSurfaces68a394[mipCount];
                        systemSurface->AddRef();
                    } else {
                        desc.caps[0] = 0x1800;
                    }
                } else {
                    desc.caps[0] = 0x1800;
                }
            }
            if (!systemSurface) {
                if (mapFormat == 4444) {
                    formats[0] = mapFormat;
                    formats[1] = 1555;
                    formats[2] = 555;
                    formats[3] = 565;
                    formats[4] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                } else if (mapFormat == 1555) {
                    formats[0] = mapFormat;
                    formats[1] = 555;
                    formats[2] = 565;
                    formats[3] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                } else if (mapFormat == 8) {
                    formats[0] = mapFormat;
                    formats[1] = 555;
                    formats[2] = 565;
                    formats[3] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                } else if (mapFormat == 555) {
                    formats[0] = mapFormat;
                    formats[1] = 565;
                    formats[2] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                } else if (mapFormat == 565) {
                    formats[0] = mapFormat;
                    formats[1] = 555;
                    formats[2] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                } else if (g_TrackGame->display->directDraw->CreateSurface(&desc, &systemSurface, 0)) {
                    goto failed;
                }
            }
            if (shared) {
                g_UnknownSharedSurfaces68a394[mipCount] = systemSurface;
                systemSurface->AddRef();
            }
            if (texturePalette && mapFormat == 8 && systemSurface->SetPalette(texturePalette))
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
            mipCount++;
        }
        if (bits && sourceFormat != mapFormat &&
            (!checkMemory || PixelFormatBytesPerPixel(565) > PixelFormatBytesPerPixel(sourceFormat)) &&
            !ownerManager->GrowSecondScratchBuffer(PixelFormatBytesPerPixel(565) * width * height))
            return 0;
        if (!systemSurface) {
            memset(&desc, 0, sizeof(desc));
            desc.size = sizeof(desc);
            desc.height = height;
            desc.width = width;
            desc.mipMapCount = mipCount;
            memcpy(&desc.pixelFormat, pixelFormat, sizeof(pixelFormat));
            shared = 0;
            desc.flags = 0x21007;
            if (flags & 4) {
                desc.caps[0] = 0x400808;
            } else {
                desc.caps[0] = 0x401008;
                if (!(flags & 8) && !g_TrackGame->field_0x2d0 &&
                    !(g_TrackGame->field_0x2d5_bit2) &&
                    (mapFormat == 555 || mapFormat == 565)) {
                    shared = 1;
                    if (g_UnknownSharedMipSurfaces68a36c[mipCount]) {
                        systemSurface = g_UnknownSharedMipSurfaces68a36c[mipCount];
                        systemSurface->AddRef();
                    } else {
                        desc.caps[0] = 0x401808;
                    }
                } else {
                    desc.caps[0] = 0x401808;
                }
            }
            if (!systemSurface) {
                if (mapFormat == 4444) {
                    formats[0] = mapFormat;
                    formats[1] = 1555;
                    formats[2] = 555;
                    formats[3] = 565;
                    formats[4] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                } else if (mapFormat == 1555) {
                    formats[0] = mapFormat;
                    formats[1] = 555;
                    formats[2] = 565;
                    formats[3] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                } else if (mapFormat == 8) {
                    formats[0] = mapFormat;
                    formats[1] = 555;
                    formats[2] = 565;
                    formats[3] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                } else if (mapFormat == 555) {
                    formats[0] = mapFormat;
                    formats[1] = 565;
                    formats[2] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                } else if (mapFormat == 565) {
                    formats[0] = mapFormat;
                    formats[1] = 555;
                    formats[2] = 0;
                    if (!CreateSystemSurface(&desc, flags, formats))
                        goto failed;
                }
            }
            if (shared) {
                g_UnknownSharedMipSurfaces68a36c[mipCount] = systemSurface;
                systemSurface->AddRef();
            }
            if (texturePalette && mapFormat == 8 && systemSurface->SetPalette(texturePalette))
                goto failed;
        }
    }
    if (systemSurface->GetCaps(&caps))
        goto failed;
    if (bits) {
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (systemSurface->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
            goto failed;
        void* destination = desc.surface;
        int bytesPerPixel = PixelFormatBytesPerPixel(mapFormat);
        ConvertPixels(destination, bits, width, height, desc.pitch / bytesPerPixel, stride, mapFormat,
                              sourceFormat, 0, palette, alphaThreshold, key);
        if (!(flags & 4))
            field_0x3c = AverageColor(destination, desc.width, desc.height, desc.pitch / bytesPerPixel,
                                               mapFormat, palette);
        if (systemSurface->Unlock(0))
            goto failed;
        if (mipmapped && !BuildMipLevels(2))
            goto failed;
    }
    if (g_UnknownGlobal689968)
        DumpLevel(systemSurface, 0);
    if (IsFormat4444Or8888(format) & !IsFormat4444Or8888(mapFormat)) {
        ApplyColorKey(0xff00ff);
        return 1;
    }
    if (IsFormat4444Or8888(mapFormat)) {
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
