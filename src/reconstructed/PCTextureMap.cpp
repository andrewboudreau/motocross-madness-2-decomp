#include "PCTextureMap.h"

#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"
#include "D3DConstants.h"

// 0x004c5f00
PCTextureMap::PCTextureMap(TextureMapManager* manager, int value) : TextureMap(manager, value) {
    field_0x70 = 0;
    field_0x74 = 0;
    field_0x78 = 0;
    videoDecoder = 0;
}

// 0x004c5f50: frees the decoder, takes the system-memory surface's size
// (bytes per pixel x height x width, 4/3 more with mip levels) off the
// DirectX memory count and releases it, then releases the texture surface.
PCTextureMap::~PCTextureMap() {
    if (videoDecoder) {
        delete videoDecoder;
        videoDecoder = 0;
    }
    if (field_0x70) {
        float scale = 1.0f;
        if (field_0x24 != 1)
            scale = 4.0f / 3.0f;
        g_MemTagStack->UnknownFunction4a2e00(
            (int)(UnknownFunction511970(field_0x20) * field_0x18 * field_0x14 * scale));
        field_0x70->UnknownMethod2();
        field_0x70 = 0;
    }
    if (field_0x74) {
        field_0x74->UnknownMethod2();
        field_0x74 = 0;
        if (g_UnknownGlobal689964)
            field_0x10->UnknownFunction511580();
    }
}

// The larger of two formats' pixel sizes (re-evaluated, max-macro style).
static inline int LargerPixelSize(int first, int second) {
    return UnknownFunction511970(first) > UnknownFunction511970(second) ? UnknownFunction511970(first)
                                                                         : UnknownFunction511970(second);
}

// Reads `rows` rows of `rowSize` bytes; whether they were all read.
static inline int ReadRows(UnknownTextureStream* stream, void* bits, int rowSize, int rows) {
    return stream->UnknownFunction461640(bits, rowSize, rows) == rows;
}

// 0x004c6080: reads the texture from `stream`. Files with mip levels hold a
// table of level offsets (relative to where each entry is read) and the
// levels from 1x1 up, each either raw or compressed (a zero size means the
// size follows); levels at least `minimumSize` wide are converted into the
// surfaces slot 4 creates, larger levels than the choice allows are
// skipped. Other files hold one image, which is halved as the choice asks
// and handed to slot 4.
int PCTextureMap::UnknownVirtualSlot5(UnknownTextureStream* stream, int width, int height, int minimumSize,
                                      int fileFormat, int dataSize, int format, UnknownTexturePalette* palette,
                                      int flags, void* surfacePalette, int addressU, int addressV,
                                      UnknownTextureFormatChoice* choice, int alphaThreshold, unsigned int key) {
    int offsets[12];
    UnknownSurfaceCaps caps;
    UnknownSurfaceDesc desc;
    int sourceFormat = DecodedFormat(fileFormat);
    if (choice) {
        format = UnknownFunction511ad0(format) ? choice->field_0x10 : choice->field_0x0c;
        field_0x6c = choice->field_0x14;
    } else {
        field_0x6c = 0;
    }
    void* buffer = field_0x10->UnknownFunction511310(LargerPixelSize(sourceFormat, format) * width * height);
    if (!buffer)
        return 0;
    void* bits = field_0x10->UnknownFunction511370(LargerPixelSize(sourceFormat, format) * width * height + 4);
    if (!bits)
        return 0;
    if (HasMipLevels(fileFormat)) {
        int levelWidth = width;
        if (choice) {
            for (int i = 0; i < choice->field_0x14 && levelWidth > 32; i++) {
                levelWidth >>= 1;
                height >>= 1;
            }
        }
        if (!UnknownVirtualSlot4(0, levelWidth, height, levelWidth, minimumSize, sourceFormat, format, palette,
                                 flags, surfacePalette, 1, 0, addressU, addressV, 0, alphaThreshold, key))
            return 0;

        int side = 1;
        int level = 0;
        int position;
        do {
            position = stream->UnknownFunction461600();
            if (stream->UnknownFunction461640(&offsets[level], 4, 1) != 1)
                return 0;
            offsets[level] += position;
            side <<= 1;
            level++;
        } while (side <= width);
        position = stream->UnknownFunction461600();
        if (stream->UnknownFunction461640(&offsets[level], 4, 1) != 1)
            return 0;
        offsets[level] += position;

        int levelSide = 1;
        int levelHeight = 1;
        level = 0;
        do {
            dataSize = offsets[level + 1] - offsets[level];
            if (IsCompressedFormat(fileFormat) &&
                dataSize != BytesPerPixel(fileFormat) * levelHeight * levelSide) {
                if (!dataSize && stream->UnknownFunction461640(&dataSize, 4, 1) != 1)
                    return 0;
                if (stream->UnknownFunction461640(buffer, 1, dataSize) != dataSize)
                    return 0;
                switch (fileFormat) {
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x18:
                case 0x19:
                case 0x1a:
                case 0x1b:
                    UnknownFunction4a03d0(bits, buffer, BytesPerPixel(fileFormat) * levelHeight * levelSide);
                    break;
                case 0x14:
                    bits = field_0x10->UnknownFunction511370(UnknownFunction511970(sourceFormat) * levelHeight *
                                                             levelSide + 4);
                    if (!bits)
                        return 0;
                    break;
                }
            } else if (!ReadRows(stream, bits, BytesPerPixel(fileFormat) * levelSide, levelHeight)) {
                return 0;
            }
            if (levelSide >= minimumSize) {
                memset(&caps, 0, sizeof(caps));
                caps.caps = 0x401000;
                int surfaceSide = levelWidth;
                UnknownSurfaceInterface* surface = field_0x70;
                for (; surfaceSide > levelSide; surfaceSide /= 2) {
                    UnknownSurfaceInterface* next;
                    long result = surface->UnknownMethod12(&caps, &next);
                    if (result && result != (long)DDERR_NOTFOUND)
                        return 0;
                    surface = next;
                }
                if (surfaceSide == levelSide) {
                    memset(&desc, 0, sizeof(desc));
                    desc.size = sizeof(desc);
                    if (surface->UnknownMethod25(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
                        return 0;
                    int stride = desc.pitch / UnknownFunction511970(format);
                    UnknownFunction4d1d20(desc.surface, bits, levelSide, levelHeight, stride, levelSide, field_0x20,
                                          sourceFormat, 0, palette, alphaThreshold, key);
                    if (surface->UnknownMethod32(0))
                        return 0;
                }
            }
            levelSide <<= 1;
            levelHeight <<= 1;
            level++;
        } while (levelSide <= levelWidth);

        if (levelWidth < width) {
            do {
                dataSize = offsets[level + 1] - offsets[level];
                if (IsCompressedFormat(fileFormat) &&
                    dataSize != BytesPerPixel(fileFormat) * levelHeight * levelSide) {
                    if (!dataSize && stream->UnknownFunction461640(&dataSize, 4, 1) != 1)
                        return 0;
                    if (stream->UnknownFunction461640(buffer, 1, dataSize) != dataSize)
                        return 0;
                } else if (!ReadRows(stream, bits, BytesPerPixel(fileFormat) * levelSide, levelHeight)) {
                    return 0;
                }
                level++;
                levelSide <<= 1;
                levelHeight <<= 1;
            } while (levelSide <= width);
        }
        return 1;
    }

    if (IsCompressedFormat(fileFormat) && dataSize != BytesPerPixel(fileFormat) * width * height) {
        if (!dataSize && stream->UnknownFunction461640(&dataSize, 4, 1) != 1)
            return 0;
        if (stream->UnknownFunction461640(buffer, 1, dataSize) != dataSize)
            return 0;
        switch (fileFormat) {
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            UnknownFunction4a03d0(bits, buffer, BytesPerPixel(fileFormat) * width * height);
            break;
        case 5:
            bits = field_0x10->UnknownFunction511370(UnknownFunction511970(sourceFormat) * width * height + 4);
            if (!bits)
                return 0;
            break;
        }
    } else {
        dataSize = BytesPerPixel(fileFormat) * width * height;
        if (stream->UnknownFunction461640(bits, 1, dataSize) != dataSize)
            return 0;
    }
    if (choice) {
        int count = choice->field_0x14;
        while (width > 32 && count--) {
            if (UnknownFunction4d1b90(buffer, bits, width / 2, height / 2, width / 2, width, 1, sourceFormat,
                                      palette, 2)) {
                width /= 2;
                height /= 2;
                memcpy(bits, buffer, UnknownFunction511970(sourceFormat) * width * height);
            }
        }
    }
    return UnknownVirtualSlot4(bits, width, height, width, minimumSize, sourceFormat, format, palette, flags,
                               surfacePalette, 1, 0, addressU, addressV, 0, alphaThreshold, key);
}

// 0x004c68e0: tries `formats` in turn (0-terminated) until the display
// creates +0x70 from `desc` in one the render target accepts (flag 4 skips
// that check), then counts the surface (4/3 more with mip levels) in DirectX
// memory.
int PCTextureMap::CreateSystemSurface(UnknownSurfaceDesc* desc, int flags, int* formats) {
    for (; *formats; formats++) {
        field_0x20 = *formats;
        if (desc->flags & 0x1000)
            UnknownFunction5119c0(field_0x20, &desc->pixelFormat);
        if ((flags & 4 || g_UnknownGlobal56e26c->field_0x10->UnknownVirtualSlot13(field_0x20)) &&
            !g_UnknownGlobal56e26c->field_0x0c->field_0x190->UnknownMethod6(desc, &field_0x70, 0)) {
            float scale = 1.0f;
            if (field_0x24 != 1)
                scale = 4.0f / 3.0f;
            g_MemTagStack->UnknownFunction4a2de0(
                (int)(UnknownFunction511970(field_0x20) * field_0x18 * field_0x14 * scale));
            return 1;
        }
    }
    return 0;
}

// 0x004c6040
void PCTextureMap::UnknownVirtualSlot12() {
    if (field_0x74 && field_0x74->UnknownMethod24()) {
        UnknownVirtualSlot10();
        UnknownVirtualSlot8(1, 0, 0);
        if (g_UnknownGlobal689964)
            field_0x10->UnknownFunction511580();
    }
}

// 0x004c7470
int PCTextureMap::UnknownVirtualSlot7() {
    return field_0x74 != 0;
}

// 0x004c7610
void PCTextureMap::UnknownVirtualSlot10() {
    if (field_0x74) {
        field_0x74->UnknownMethod2();
        field_0x74 = 0;
        if (g_UnknownGlobal689964)
            field_0x10->UnknownFunction511580();
    }
}

// 0x004c7970
int PCTextureMap::UnknownVirtualSlot11() {
    if (field_0x74 &&
        !((PCRenderTarget*)g_UnknownGlobal56e26c->field_0x10)->device->UnknownMethod35(0, field_0x74))
        return 1;
    return 0;
}

// 0x004c79a0
void PCTextureMap::UnknownVirtualSlot19() {
    UnknownVirtualSlot11();
    for (int i = 0; i < field_0x44; i++)
        g_UnknownGlobal56e26c->field_0x10->UnknownVirtualSlot8(field_0x48[i].state, field_0x48[i].value, 0);
}

// 0x004c79e0
void* PCTextureMap::UnknownVirtualSlot13(void* rect, long* pitch, int flags) {
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (field_0x70->UnknownMethod25(rect, &desc, flags, 0))
        return 0;
    if (pitch)
        *pitch = desc.pitch;
    return desc.surface;
}

// 0x004c7a50
int PCTextureMap::UnknownVirtualSlot14(void* rect) {
    return !field_0x70->UnknownMethod32(rect);
}

// 0x004c7a70
void* PCTextureMap::UnknownVirtualSlot16(int level) {
    UnknownSurfaceInterface* surface = FindMipLevel(level);
    if (surface) {
        UnknownSurfaceDesc desc;
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (!surface->UnknownMethod25(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
            return desc.surface;
    }
    return 0;
}

// 0x004c7ad0
int PCTextureMap::UnknownVirtualSlot17(int level) {
    UnknownSurfaceInterface* surface = FindMipLevel(level);
    if (surface && !surface->UnknownMethod32(0))
        return 1;
    return 0;
}

// 0x004c7420
int PCTextureMap::RestoreTextureSurface() {
    if (field_0x74 && field_0x74->UnknownMethod24()) {
        field_0x74->UnknownMethod2();
        if (field_0x70 == field_0x74)
            field_0x70 = 0;
        field_0x74 = 0;
        return UnknownVirtualSlot8(1, 0, 0);
    }
    return 1;
}

// 0x004c7b00
int PCTextureMap::BlitTo(void* destinationRect, UnknownSurfaceInterface* destination,
                                        void* sourceRect, int flags, int skip) {
    if (!skip)
        return !destination->UnknownMethod5(destinationRect, field_0x70, sourceRect, flags, 0);
    return 0;
}

// 0x004c83a0: follows the attached mip surfaces, halving the width, until it
// reaches `width`. (`next` shares the dead parameter's stack slot, as in
// retail.)
UnknownSurfaceInterface* PCTextureMap::FindMipLevel(int width) {
    UnknownSurfaceCaps caps;
    memset(&caps, 0, sizeof(caps));
    int size = field_0x14;
    UnknownSurfaceInterface* surface = field_0x70;
    caps.caps = 0x401000;
    long result = 0;
    while (size != width) {
        UnknownSurfaceInterface* next;
        result = surface->UnknownMethod12(&caps, &next);
        if (result)
            break;
        surface = next;
        size /= 2;
    }
    if (result && result != (long)DDERR_NOTFOUND)
        UnknownReportDirectDrawError(result, __FILE__, 2034);
    else if (size == width)
        return surface;
    return 0;
}

// 0x004c8430: dumps every mip level (0x004c84e0).
int PCTextureMap::UnknownVirtualSlot20() {
    if (field_0x70) {
        DumpLevel(field_0x70, 0);
        if (field_0x24 > 1) {
            UnknownSurfaceInterface* surface;
            UnknownSurfaceCaps caps;
            UnknownSurfaceInterface* top = field_0x70;
            memset(&caps, 0, sizeof(caps));
            caps.caps = 0x401000;
            long result = top->UnknownMethod12(&caps, &surface);
            while (!result) {
                DumpLevel(surface, 0);
                result = surface->UnknownMethod12(&caps, &surface);
            }
            if (result != (long)DDERR_NOTFOUND) {
                UnknownReportDirectDrawError(result, __FILE__, 2084);
                return 0;
            }
        }
    }
    return 1;
}

// 0x004c84e0: locks a level and writes it to a file (0x004c8550).
int PCTextureMap::DumpLevel(UnknownSurfaceInterface* surface, const char* name) {
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    surface->UnknownMethod25(0, &desc, DDLOCK_WAIT | DDLOCK_READONLY | DDLOCK_NOSYSLOCK, 0);
    WriteLevel(&desc, name);
    surface->UnknownMethod32(0);
    return 1;
}

// 0x004c7480: creates the texture surface from +0x70 (or shares +0x70 when
// it is already in video memory and `c` is clear, or when Game+0x2d0 is
// set); `a` then asks slot 9 to upload it.
int PCTextureMap::UnknownVirtualSlot8(int a, int b, int c) {
    if (field_0x74)
        goto done;
    if (field_0x70) {
        UnknownSurfaceCaps caps;
        if (field_0x70->UnknownMethod14(&caps))
            goto failed;
        if (caps.caps & DDSCAPS_NONLOCALVIDMEM && !c || g_UnknownGlobal56e26c->field_0x2d0) {
            field_0x74 = field_0x70;
            field_0x70->UnknownMethod1();
        } else {
            UnknownSurfaceDesc desc;
            memset(&desc, 0, sizeof(desc));
            desc.size = sizeof(desc);
            if (field_0x70->UnknownMethod22(&desc))
                goto failed;
            desc.flags &= 0x3f087;
            desc.caps[0] = desc.caps[0] & 0xcffff7ff | (b ? 0x2000 : 0) | DDSCAPS_VIDEOMEMORY;
            if (g_UnknownGlobal56e26c->field_0x0c->field_0x9f0 && c)
                desc.caps[0] |= DDSCAPS_LOCALVIDMEM;
            desc.pitch = 0;
            if (!field_0x74 &&
                g_UnknownGlobal56e26c->field_0x0c->field_0x190->UnknownMethod6(&desc, &field_0x74, 0))
                goto failed;
            if (field_0x78 && field_0x20 == 8 && field_0x74->UnknownMethod31(field_0x78))
                goto failed;
            if (g_UnknownGlobal689964)
                field_0x10->UnknownFunction511580();
        }
    }
    if (a && field_0x74 != field_0x70)
        UnknownVirtualSlot9(0, -1);
    if (!field_0x74)
        goto failed;
done:
    return 1;
failed:
    return 0;
}

// 0x004c7640: with partial texture blits (Display+0x5bc) or a positive
// `mode`, copies `rect` (or the whole texture) down the mip chain with
// BltFast, halving it per level; otherwise lets the device Load it. Then
// applies the colour key. The level loop tests its two sizes in separate
// `break`s: a `while (a && b)` condition is rotated to the bottom, retail
// re-runs the test at the top of every pass.
int PCTextureMap::UnknownVirtualSlot9(UnknownRect* rect, int mode) {
    if (!field_0x70 || !field_0x74)
        return 0;
    UnknownSurfaceInterface* source = field_0x70;
    UnknownSurfaceInterface* destination = field_0x74;
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
        while (1) {
            if (area.right - area.left <= 0)
                break;
            if (area.bottom - area.top <= 0)
                break;
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
            ->device->UnknownMethod43(destination, 0, source, 0, 0);
    return 0;
}

// 0x004c77d0: fills each mip level by downsampling the level above it.
int PCTextureMap::UnknownVirtualSlot15(int filter) {
    UnknownSurfaceInterface* level;
    void* sourceBits;
    int bytesPerPixel;
    int stride;
    long result;
    UnknownSurfaceCaps caps;
    UnknownSurfaceDesc desc;
    memset(&caps, 0, sizeof(caps));
    caps.caps = 0x401000;
    bytesPerPixel = UnknownFunction511970(field_0x20);
    UnknownSurfaceInterface* parent = field_0x70;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (parent->UnknownMethod25(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
        goto failed;
    stride = desc.pitch / bytesPerPixel;
    sourceBits = desc.surface;
    result = parent->UnknownMethod12(&caps, &level);
    while (!result) {
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (level->UnknownMethod25(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
            goto failed;
        int sourceStride = stride;
        stride = desc.pitch / bytesPerPixel;
        UnknownFunction4d1b90(desc.surface, sourceBits, desc.width, desc.height, stride, sourceStride, 1,
                              field_0x20, field_0x2c, filter);
        if (parent->UnknownMethod32(0))
            goto failed;
        sourceBits = desc.surface;
        parent = level;
        result = parent->UnknownMethod12(&caps, &level);
    }
    if (result != (long)DDERR_NOTFOUND) {
        UnknownReportDirectDrawError(result, __FILE__, 1421);
        return 0;
    }
    if (parent->UnknownMethod32(0))
        goto failed;
    return 1;
failed:
    return 0;
}

// Sets render state `state` to `value` in the texture's list, appending it
// when absent.
static inline void SetRenderStatePair(TextureMap* map, int state, int value) {
    int i;
    for (i = 0; i < map->field_0x44; i++) {
        if (map->field_0x48[i].state == state)
            break;
    }
    if (i < map->field_0x44) {
        map->field_0x48[i].value = value;
    } else {
        map->field_0x48[map->field_0x44].state = state;
        map->field_0x48[map->field_0x44].value = value;
        map->field_0x44++;
    }
}

// 0x004c7e30: converts a 24-bit colour to the texture's format (555, 565 or
// a palette index) and stores it as the colour key.
void PCTextureMap::SetColorKey(unsigned int color) {
    int key;
    if (field_0x20 == 555)
        key = Pack555(color);
    else if (field_0x20 == 565)
        key = Pack565(color);
    else if (field_0x20 == 8)
        key = field_0x2c->field_0x710[Pack555(color)];
    else
        key = color;
    field_0x34 = field_0x38 = key;
}

// 0x004c7ef0: replaces magenta in `surface` with `color` and makes `color`
// (converted to the texture's format) the colour key. Magenta itself only
// sets the key, except in format 8888, where the key pixels also lose
// their alpha.
int PCTextureMap::ColorKeyLevel(UnknownSurfaceInterface* surface, unsigned int color) {
    if (color == 0xff00ff && field_0x20 != 8888) {
        SetColorKey(color);
        return 1;
    }
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (surface->UnknownMethod25(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
        goto failed;
    if (field_0x20 == 8888) {
        UnknownPixel32 from;
        UnknownPixel32 to;
        field_0x34 = field_0x38 = color;
        from.red = 0xff;
        from.green = 0;
        from.blue = 0xff;
        from.alpha = 0xff;
        to.red = (unsigned char)(color >> 16);
        to.green = (unsigned char)(color >> 8);
        to.blue = (unsigned char)color;
        to.alpha = 0;
        ReplaceColor32(desc.surface, from, to, desc.width, desc.height,
                              desc.pitch / UnknownFunction511970(8888));
    }
    if (field_0x20 == 888) {
        UnknownPixel24 from;
        UnknownPixel24 to;
        field_0x34 = field_0x38 = color;
        from.red = 0xff;
        from.green = 0;
        from.blue = 0xff;
        to.red = (unsigned char)(color >> 16);
        to.green = (unsigned char)(color >> 8);
        to.blue = (unsigned char)color;
        ReplaceColor24(desc.surface, from, to, desc.width, desc.height,
                              desc.pitch / UnknownFunction511970(888));
    }
    if (field_0x20 == 555 || field_0x20 == 1555) {
        field_0x34 = field_0x38 = Pack555(color);
        ReplaceColor16(desc.surface, 0x7c1f, Pack555(color), desc.width, desc.height,
                              desc.pitch / UnknownFunction511970(field_0x20));
    } else if (field_0x20 == 565) {
        field_0x34 = field_0x38 = Pack565(color);
        ReplaceColor16(desc.surface, 0xf81f, Pack565(color), desc.width, desc.height,
                              desc.pitch / UnknownFunction511970(565));
    } else if (field_0x20 == 8) {
        field_0x34 = field_0x38 = field_0x2c->field_0x710[Pack555(color)];
        ReplaceColor8(desc.surface, field_0x2c->field_0x710[0x7c1f],
                              field_0x2c->field_0x710[Pack555(color)], desc.width, desc.height, desc.pitch);
    }
    if (surface->UnknownMethod32(0))
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004c81d0: for 555, 565, 24-bit and 1555 formats, applies colour `color` as the
// key on every level (0x004c7ef0), re-uploads, sets the surfaces' colour key
// and records render states 0x29 = 1 and 0x1b = 0.
int PCTextureMap::UnknownVirtualSlot18(unsigned int color) {
    if (field_0x20 != 555 && field_0x20 != 565 && field_0x20 != 888 && field_0x20 != 1555)
        return 0;
    if (field_0x70) {
        ColorKeyLevel(field_0x70, color);
        if (field_0x24 > 1) {
            UnknownSurfaceInterface* surface;
            UnknownSurfaceCaps caps;
            UnknownSurfaceInterface* top = field_0x70;
            memset(&caps, 0, sizeof(caps));
            caps.caps = 0x401000;
            long result = top->UnknownMethod12(&caps, &surface);
            while (!result) {
                ColorKeyLevel(surface, color);
                result = surface->UnknownMethod12(&caps, &surface);
            }
            if (result != (long)DDERR_NOTFOUND) {
                UnknownReportDirectDrawError(result, __FILE__, 1811);
                return 0;
            }
        }
    }
    if ((color != 0xff00ff || field_0x20 == 8888) && field_0x74)
        UnknownVirtualSlot9(0, -1);
    if (field_0x20 == 1555)
        return 1;
    if (field_0x70 && field_0x70->UnknownMethod29(8, &field_0x34) ||
        field_0x74 && field_0x74->UnknownMethod29(8, &field_0x34))
        return 0;
    field_0x30 = 1;
    SetRenderStatePair(this, 0x29, 1);
    SetRenderStatePair(this, 0x1b, 0);
    return 1;
}

// 0x004c86e0: formats the name of a DirectDraw result into a local buffer.
// Nothing reads the text, `file` or `line` in retail (the output was
// presumably compiled out). Cases are in retail's body order.
void UnknownReportDirectDrawError(long result, const char* file, int line) {
    char text[256];
    switch (result) {
    case 0x800401f0:
        sprintf(text, "DDERR_NOTINITIALIZED");
        break;
    case 0x80004005:
        sprintf(text, "DDERR_GENERIC");
        break;
    case 0x80004001:
        sprintf(text, "DDERR_UNSUPPORTED");
        break;
    case 0x8007000e:
        sprintf(text, "DDERR_OUTOFMEMORY");
        break;
    case 0x88760005:
        sprintf(text, "DDERR_ALREADYINITIALIZED");
        break;
    case 0x80070057:
        sprintf(text, "DDERR_INVALIDPARAMS");
        break;
    case 0x8876000a:
        sprintf(text, "DDERR_CANNOTATTACHSURFACE");
        break;
    case 0x88760014:
        sprintf(text, "DDERR_CANNOTDETACHSURFACE");
        break;
    case 0x88760028:
        sprintf(text, "DDERR_CURRENTLYNOTAVAIL");
        break;
    case 0x88760037:
        sprintf(text, "DDERR_EXCEPTION");
        break;
    case 0x8876005a:
        sprintf(text, "DDERR_HEIGHTALIGN");
        break;
    case 0x8876005f:
        sprintf(text, "DDERR_INCOMPATIBLEPRIMARY");
        break;
    case 0x88760064:
        sprintf(text, "DDERR_INVALIDCAPS");
        break;
    case 0x8876006e:
        sprintf(text, "DDERR_INVALIDCLIPLIST");
        break;
    case 0x88760078:
        sprintf(text, "DDERR_INVALIDMODE");
        break;
    case 0x88760082:
        sprintf(text, "DDERR_INVALIDOBJECT");
        break;
    case 0x88760091:
        sprintf(text, "DDERR_INVALIDPIXELFORMAT");
        break;
    case 0x88760096:
        sprintf(text, "DDERR_INVALIDRECT");
        break;
    case 0x887600a0:
        sprintf(text, "DDERR_LOCKEDSURFACES");
        break;
    case 0x887600aa:
        sprintf(text, "DDERR_NO3D");
        break;
    case 0x887600b4:
        sprintf(text, "DDERR_NOALPHAHW");
        break;
    case 0x887600cd:
        sprintf(text, "DDERR_NOCLIPLIST");
        break;
    case 0x887600d2:
        sprintf(text, "DDERR_NOCOLORCONVHW");
        break;
    case 0x887600d4:
        sprintf(text, "DDERR_NOCOOPERATIVELEVELSET");
        break;
    case 0x887600d7:
        sprintf(text, "DDERR_NOCOLORKEY");
        break;
    case 0x887600dc:
        sprintf(text, "DDERR_NOCOLORKEYHW");
        break;
    case 0x887600de:
        sprintf(text, "DDERR_NODIRECTDRAWSUPPORT");
        break;
    case 0x887600e1:
        sprintf(text, "DDERR_NOEXCLUSIVEMODE");
        break;
    case 0x887600e6:
        sprintf(text, "DDERR_NOFLIPHW");
        break;
    case 0x887600f0:
        sprintf(text, "DDERR_NOGDI");
        break;
    case 0x887600fa:
        sprintf(text, "DDERR_NOMIRRORHW");
        break;
    case 0x887600ff:
        sprintf(text, "DDERR_NOTFOUND");
        break;
    case 0x88760104:
        sprintf(text, "DDERR_NOOVERLAYHW");
        break;
    case 0x8876010e:
        sprintf(text, "DDERR_OVERLAPPINGRECTS");
        break;
    case 0x88760118:
        sprintf(text, "DDERR_NORASTEROPHW");
        break;
    case 0x88760122:
        sprintf(text, "DDERR_NOROTATIONHW");
        break;
    case 0x88760136:
        sprintf(text, "DDERR_NOSTRETCHHW");
        break;
    case 0x8876013c:
        sprintf(text, "DDERR_NOT4BITCOLOR");
        break;
    case 0x8876013d:
        sprintf(text, "DDERR_NOT4BITCOLORINDEX");
        break;
    case 0x88760140:
        sprintf(text, "DDERR_NOT8BITCOLOR");
        break;
    case 0x8876014a:
        sprintf(text, "DDERR_NOTEXTUREHW");
        break;
    case 0x8876014f:
        sprintf(text, "DDERR_NOVSYNCHW");
        break;
    case 0x88760154:
        sprintf(text, "DDERR_NOZBUFFERHW");
        break;
    case 0x8876015e:
        sprintf(text, "DDERR_NOZOVERLAYHW");
        break;
    case 0x88760168:
        sprintf(text, "DDERR_OUTOFCAPS");
        break;
    case 0x8876017c:
        sprintf(text, "DDERR_OUTOFVIDEOMEMORY");
        break;
    case 0x8876017e:
        sprintf(text, "DDERR_OVERLAYCANTCLIP");
        break;
    case 0x88760180:
        sprintf(text, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE");
        break;
    case 0x88760183:
        sprintf(text, "DDERR_PALETTEBUSY");
        break;
    case 0x88760190:
        sprintf(text, "DDERR_COLORKEYNOTSET");
        break;
    case 0x8876019a:
        sprintf(text, "DDERR_SURFACEALREADYATTACHED");
        break;
    case 0x887601a4:
        sprintf(text, "DDERR_SURFACEALREADYDEPENDENT");
        break;
    case 0x887601ae:
        sprintf(text, "DDERR_SURFACEBUSY");
        break;
    case 0x887601b3:
        sprintf(text, "DDERR_CANTLOCKSURFACE");
        break;
    case 0x887601b8:
        sprintf(text, "DDERR_SURFACEISOBSCURED");
        break;
    case 0x887601c2:
        sprintf(text, "DDERR_SURFACELOST");
        break;
    case 0x887601cc:
        sprintf(text, "DDERR_SURFACENOTATTACHED");
        break;
    case 0x887601d6:
        sprintf(text, "DDERR_TOOBIGHEIGHT");
        break;
    case 0x887601e0:
        sprintf(text, "DDERR_TOOBIGSIZE");
        break;
    case 0x887601ea:
        sprintf(text, "DDERR_TOOBIGWIDTH");
        break;
    case 0x887601fe:
        sprintf(text, "DDERR_UNSUPPORTEDFORMAT");
        break;
    case 0x88760208:
        sprintf(text, "DDERR_UNSUPPORTEDMASK");
        break;
    case 0x88760209:
        sprintf(text, "DDERR_INVALIDSTREAM");
        break;
    case 0x88760219:
        sprintf(text, "DDERR_VERTICALBLANKINPROGRESS");
        break;
    case 0x8876021c:
        sprintf(text, "DDERR_WASSTILLDRAWING");
        break;
    case 0x88760230:
        sprintf(text, "DDERR_XALIGN");
        break;
    case 0x88760231:
        sprintf(text, "DDERR_INVALIDDIRECTDRAWGUID");
        break;
    case 0x88760232:
        sprintf(text, "DDERR_DIRECTDRAWALREADYCREATED");
        break;
    case 0x88760233:
        sprintf(text, "DDERR_NODIRECTDRAWHW");
        break;
    case 0x88760234:
        sprintf(text, "DDERR_PRIMARYSURFACEALREADYEXISTS");
        break;
    case 0x88760235:
        sprintf(text, "DDERR_NOEMULATION");
        break;
    case 0x88760236:
        sprintf(text, "DDERR_REGIONTOOSMALL");
        break;
    case 0x88760237:
        sprintf(text, "DDERR_CLIPPERISUSINGHWND");
        break;
    case 0x88760238:
        sprintf(text, "DDERR_NOCLIPPERATTACHED");
        break;
    case 0x88760239:
        sprintf(text, "DDERR_NOHWND");
        break;
    case 0x8876023a:
        sprintf(text, "DDERR_HWNDSUBCLASSED");
        break;
    case 0x8876023b:
        sprintf(text, "DDERR_HWNDALREADYSET");
        break;
    case 0x8876023c:
        sprintf(text, "DDERR_NOPALETTEATTACHED");
        break;
    case 0x8876023d:
        sprintf(text, "DDERR_NOPALETTEHW");
        break;
    case 0x8876023e:
        sprintf(text, "DDERR_BLTFASTCANTCLIP");
        break;
    case 0x8876023f:
        sprintf(text, "DDERR_NOBLTHW");
        break;
    case 0x88760240:
        sprintf(text, "DDERR_NODDROPSHW");
        break;
    case 0x88760241:
        sprintf(text, "DDERR_OVERLAYNOTVISIBLE");
        break;
    case 0x88760242:
        sprintf(text, "DDERR_NOOVERLAYDEST");
        break;
    case 0x88760243:
        sprintf(text, "DDERR_INVALIDPOSITION");
        break;
    case 0x88760244:
        sprintf(text, "DDERR_NOTAOVERLAYSURFACE");
        break;
    case 0x88760245:
        sprintf(text, "DDERR_EXCLUSIVEMODEALREADYSET");
        break;
    case 0x88760246:
        sprintf(text, "DDERR_NOTFLIPPABLE");
        break;
    case 0x88760247:
        sprintf(text, "DDERR_CANTDUPLICATE");
        break;
    case 0x88760248:
        sprintf(text, "DDERR_NOTLOCKED");
        break;
    case 0x88760249:
        sprintf(text, "DDERR_CANTCREATEDC");
        break;
    case 0x8876024a:
        sprintf(text, "DDERR_NODC");
        break;
    case 0x8876024b:
        sprintf(text, "DDERR_WRONGMODE");
        break;
    case 0x8876024c:
        sprintf(text, "DDERR_IMPLICITLYCREATED");
        break;
    case 0x8876024d:
        sprintf(text, "DDERR_NOTPALETTIZED");
        break;
    case 0x8876024e:
        sprintf(text, "DDERR_UNSUPPORTEDMODE");
        break;
    case 0x8876024f:
        sprintf(text, "DDERR_NOMIPMAPHW");
        break;
    case 0x88760250:
        sprintf(text, "DDERR_INVALIDSURFACETYPE");
        break;
    case 0x88760258:
        sprintf(text, "DDERR_NOOPTIMIZEHW");
        break;
    case 0x88760259:
        sprintf(text, "DDERR_NOTLOADED");
        break;
    case 0x8876025a:
        sprintf(text, "DDERR_NOFOCUSWINDOW");
        break;
    case 0x8876026c:
        sprintf(text, "DDERR_DCALREADYCREATED");
        break;
    case 0x88760276:
        sprintf(text, "DDERR_NONONLOCALVIDMEM");
        break;
    case 0x88760280:
        sprintf(text, "DDERR_CANTPAGELOCK");
        break;
    case 0x88760294:
        sprintf(text, "DDERR_CANTPAGEUNLOCK");
        break;
    case 0x887602a8:
        sprintf(text, "DDERR_NOTPAGELOCKED");
        break;
    case 0x887602b2:
        sprintf(text, "DDERR_MOREDATA");
        break;
    case 0x887602b3:
        sprintf(text, "DDERR_EXPIRED");
        break;
    case 0x887602b7:
        sprintf(text, "DDERR_VIDEONOTACTIVE");
        break;
    case 0x887602bb:
        sprintf(text, "DDERR_DEVICEDOESNTOWNSURFACE");
        break;
    default:
        sprintf(text, "Unknown Error");
        break;
    }
}
