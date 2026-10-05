#include "PCTextureMap.h"

#include <string.h>

#include "DebugAlloc.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"

// 0x004c5f00
PCTextureMap::PCTextureMap(TextureMapManager* manager, int value) : TextureMap(manager, value) {
    field_0x70 = 0;
    field_0x74 = 0;
    field_0x78 = 0;
    field_0x7c = 0;
}

// 0x004c5f50: frees the decoder, takes the system-memory surface's size
// (bytes per pixel x height x width, 4/3 more with mip levels) off the
// DirectX memory count and releases it, then releases the texture surface.
PCTextureMap::~PCTextureMap() {
    if (field_0x7c) {
        delete field_0x7c;
        field_0x7c = 0;
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
    int sourceFormat = UnknownFunction5118a0(fileFormat);
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
    if (UnknownFunction511850(fileFormat)) {
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
            if (UnknownFunction511800(fileFormat) &&
                dataSize != UnknownFunction511740(fileFormat) * levelHeight * levelSide) {
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
                    UnknownFunction4a03d0(bits, buffer, UnknownFunction511740(fileFormat) * levelHeight * levelSide);
                    break;
                case 0x14:
                    bits = field_0x10->UnknownFunction511370(UnknownFunction511970(sourceFormat) * levelHeight *
                                                             levelSide + 4);
                    if (!bits)
                        return 0;
                    break;
                }
            } else if (!ReadRows(stream, bits, UnknownFunction511740(fileFormat) * levelSide, levelHeight)) {
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
                    if (result && result != (long)0x887600ff)
                        return 0;
                    surface = next;
                }
                if (surfaceSide == levelSide) {
                    memset(&desc, 0, sizeof(desc));
                    desc.size = sizeof(desc);
                    if (surface->UnknownMethod25(0, &desc, 0x801, 0))
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
                if (UnknownFunction511800(fileFormat) &&
                    dataSize != UnknownFunction511740(fileFormat) * levelHeight * levelSide) {
                    if (!dataSize && stream->UnknownFunction461640(&dataSize, 4, 1) != 1)
                        return 0;
                    if (stream->UnknownFunction461640(buffer, 1, dataSize) != dataSize)
                        return 0;
                } else if (!ReadRows(stream, bits, UnknownFunction511740(fileFormat) * levelSide, levelHeight)) {
                    return 0;
                }
                level++;
                levelSide <<= 1;
                levelHeight <<= 1;
            } while (levelSide <= width);
        }
        return 1;
    }

    if (UnknownFunction511800(fileFormat) && dataSize != UnknownFunction511740(fileFormat) * width * height) {
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
            UnknownFunction4a03d0(bits, buffer, UnknownFunction511740(fileFormat) * width * height);
            break;
        case 5:
            bits = field_0x10->UnknownFunction511370(UnknownFunction511970(sourceFormat) * width * height + 4);
            if (!bits)
                return 0;
            break;
        }
    } else {
        dataSize = UnknownFunction511740(fileFormat) * width * height;
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
        !((PCRenderTarget*)g_UnknownGlobal56e26c->field_0x10)->field_0x50->UnknownMethod35(0, field_0x74))
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
    UnknownSurfaceInterface* surface = UnknownFunction4c83a0(level);
    if (surface) {
        UnknownSurfaceDesc desc;
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (!surface->UnknownMethod25(0, &desc, 0x801, 0))
            return desc.surface;
    }
    return 0;
}

// 0x004c7ad0
int PCTextureMap::UnknownVirtualSlot17(int level) {
    UnknownSurfaceInterface* surface = UnknownFunction4c83a0(level);
    if (surface && !surface->UnknownMethod32(0))
        return 1;
    return 0;
}

// 0x004c7420
int PCTextureMap::UnknownFunction4c7420() {
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
int PCTextureMap::UnknownFunction4c7b00(void* destinationRect, UnknownSurfaceInterface* destination,
                                        void* sourceRect, int flags, int skip) {
    if (!skip)
        return !destination->UnknownMethod5(destinationRect, field_0x70, sourceRect, flags, 0);
    return 0;
}

// 0x004c83a0: follows the attached mip surfaces, halving the width, until it
// reaches `width`. (`next` shares the dead parameter's stack slot, as in
// retail.)
UnknownSurfaceInterface* PCTextureMap::UnknownFunction4c83a0(int width) {
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
    if (result && result != (long)0x887600ff)
        UnknownReportDirectDrawError(result, __FILE__, 2034);
    else if (size == width)
        return surface;
    return 0;
}

// 0x004c8430: fills every mip level.
int PCTextureMap::UnknownVirtualSlot20() {
    if (field_0x70) {
        UnknownFunction4c84e0(field_0x70, 0);
        if (field_0x24 > 1) {
            UnknownSurfaceInterface* surface;
            UnknownSurfaceCaps caps;
            UnknownSurfaceInterface* top = field_0x70;
            memset(&caps, 0, sizeof(caps));
            caps.caps = 0x401000;
            long result = top->UnknownMethod12(&caps, &surface);
            while (!result) {
                UnknownFunction4c84e0(surface, 0);
                result = surface->UnknownMethod12(&caps, &surface);
            }
            if (result != (long)0x887600ff) {
                UnknownReportDirectDrawError(result, __FILE__, 2084);
                return 0;
            }
        }
    }
    return 1;
}

// 0x004c84e0
int PCTextureMap::UnknownFunction4c84e0(UnknownSurfaceInterface* surface, int value) {
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    surface->UnknownMethod25(0, &desc, 0x811, 0);
    UnknownFunction4c8550(&desc, value);
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
        if (caps.caps & 0x20000000 && !c || g_UnknownGlobal56e26c->field_0x2d0) {
            field_0x74 = field_0x70;
            field_0x70->UnknownMethod1();
        } else {
            UnknownSurfaceDesc desc;
            memset(&desc, 0, sizeof(desc));
            desc.size = sizeof(desc);
            if (field_0x70->UnknownMethod22(&desc))
                goto failed;
            desc.flags &= 0x3f087;
            desc.caps[0] = desc.caps[0] & 0xcffff7ff | (b ? 0x2000 : 0) | 0x4000;
            if (g_UnknownGlobal56e26c->field_0x0c->field_0x9f0 && c)
                desc.caps[0] |= 0x10000000;
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
    if (parent->UnknownMethod25(0, &desc, 0x801, 0))
        goto failed;
    stride = desc.pitch / bytesPerPixel;
    sourceBits = desc.surface;
    result = parent->UnknownMethod12(&caps, &level);
    while (!result) {
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (level->UnknownMethod25(0, &desc, 0x801, 0))
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
    if (result != (long)0x887600ff) {
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

// A 24-bit 0xRRGGBB colour as a 555 or 565 pixel.
static inline unsigned short Pack555(unsigned int color) {
    return (unsigned short)(((color >> 3) & 0x1f) | ((color >> 6) & 0x3e0) | ((color >> 9) & 0x7c00));
}

static inline unsigned short Pack565(unsigned int color) {
    return (unsigned short)(((color >> 3) & 0x1f) | ((color >> 5) & 0x7e0) | ((color >> 8) & 0xf800));
}

// 0x004c7e30: converts a 24-bit colour to the texture's format (555, 565 or
// a palette index) and stores it as the colour key.
void PCTextureMap::UnknownFunction4c7e30(unsigned int color) {
    int key;
    if (field_0x20 == 0x22b)
        key = Pack555(color);
    else if (field_0x20 == 0x235)
        key = Pack565(color);
    else if (field_0x20 == 8)
        key = field_0x2c->field_0x710[Pack555(color)];
    else
        key = color;
    field_0x34 = field_0x38 = key;
}

// 0x004c7ef0: replaces magenta in `surface` with `color` and makes `color`
// (converted to the texture's format) the colour key. Magenta itself only
// sets the key, except in format 0x22b8, where the key pixels also lose
// their alpha.
int PCTextureMap::UnknownFunction4c7ef0(UnknownSurfaceInterface* surface, unsigned int color) {
    if (color == 0xff00ff && field_0x20 != 0x22b8) {
        UnknownFunction4c7e30(color);
        return 1;
    }
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (surface->UnknownMethod25(0, &desc, 0x801, 0))
        goto failed;
    if (field_0x20 == 0x22b8) {
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
        UnknownFunction4d1970(desc.surface, from, to, desc.width, desc.height,
                              desc.pitch / UnknownFunction511970(0x22b8));
    }
    if (field_0x20 == 0x378) {
        UnknownPixel24 from;
        UnknownPixel24 to;
        field_0x34 = field_0x38 = color;
        from.red = 0xff;
        from.green = 0;
        from.blue = 0xff;
        to.red = (unsigned char)(color >> 16);
        to.green = (unsigned char)(color >> 8);
        to.blue = (unsigned char)color;
        UnknownFunction4d1a20(desc.surface, from, to, desc.width, desc.height,
                              desc.pitch / UnknownFunction511970(0x378));
    }
    if (field_0x20 == 0x22b || field_0x20 == 0x613) {
        field_0x34 = field_0x38 = Pack555(color);
        UnknownFunction4d1ac0(desc.surface, 0x7c1f, Pack555(color), desc.width, desc.height,
                              desc.pitch / UnknownFunction511970(field_0x20));
    } else if (field_0x20 == 0x235) {
        field_0x34 = field_0x38 = Pack565(color);
        UnknownFunction4d1ac0(desc.surface, 0xf81f, Pack565(color), desc.width, desc.height,
                              desc.pitch / UnknownFunction511970(0x235));
    } else if (field_0x20 == 8) {
        field_0x34 = field_0x38 = field_0x2c->field_0x710[Pack555(color)];
        UnknownFunction4d1b40(desc.surface, field_0x2c->field_0x710[0x7c1f],
                              field_0x2c->field_0x710[Pack555(color)], desc.width, desc.height, desc.pitch);
    }
    if (surface->UnknownMethod32(0))
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004c81d0: for 16-bit and 0x613 formats, applies colour `color` as the
// key on every level (0x004c7ef0), re-uploads, sets the surfaces' colour key
// and records render states 0x29 = 1 and 0x1b = 0.
int PCTextureMap::UnknownVirtualSlot18(unsigned int color) {
    if (field_0x20 != 0x22b && field_0x20 != 0x235 && field_0x20 != 0x378 && field_0x20 != 0x613)
        return 0;
    if (field_0x70) {
        UnknownFunction4c7ef0(field_0x70, color);
        if (field_0x24 > 1) {
            UnknownSurfaceInterface* surface;
            UnknownSurfaceCaps caps;
            UnknownSurfaceInterface* top = field_0x70;
            memset(&caps, 0, sizeof(caps));
            caps.caps = 0x401000;
            long result = top->UnknownMethod12(&caps, &surface);
            while (!result) {
                UnknownFunction4c7ef0(surface, color);
                result = surface->UnknownMethod12(&caps, &surface);
            }
            if (result != (long)0x887600ff) {
                UnknownReportDirectDrawError(result, __FILE__, 1811);
                return 0;
            }
        }
    }
    if ((color != 0xff00ff || field_0x20 == 0x22b8) && field_0x74)
        UnknownVirtualSlot9(0, -1);
    if (field_0x20 == 0x613)
        return 1;
    if (field_0x70 && field_0x70->UnknownMethod29(8, &field_0x34) ||
        field_0x74 && field_0x74->UnknownMethod29(8, &field_0x34))
        return 0;
    field_0x30 = 1;
    SetRenderStatePair(this, 0x29, 1);
    SetRenderStatePair(this, 0x1b, 0);
    return 1;
}
