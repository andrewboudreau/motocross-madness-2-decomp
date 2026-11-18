#include "PCTextureMap.h"

#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"
#include "ManagedTexture.h"
#include "D3DConstants.h"

// 0x004c5f00
PCTextureMap::PCTextureMap(TextureMapManager* manager, int value) : TextureMap(manager, value) {
    systemSurface = 0;
    textureSurface = 0;
    texturePalette = 0;
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
    if (systemSurface) {
        float scale = 1.0f;
        if (mipCount != 1)
            scale = 4.0f / 3.0f;
        g_MemTagStack->SubtractDirectXBytes(
            (int)(PixelFormatBytesPerPixel(mapFormat) * textureHeight * textureWidth * scale));
        systemSurface->Release();
        systemSurface = 0;
    }
    if (textureSurface) {
        textureSurface->Release();
        textureSurface = 0;
        if (g_DebugCountTextureMemory)
            ownerManager->CountTextureMemory();
    }
}

// 0x004c6040
void PCTextureMap::RecreateIfLost() {
    if (textureSurface && textureSurface->IsLost()) {
        ReleaseTextureSurface();
        CreateTextureSurface(1, 0, 0);
        if (g_DebugCountTextureMemory)
            ownerManager->CountTextureMemory();
    }
}

// The larger of two formats' pixel sizes (re-evaluated, max-macro style).
static inline int LargerPixelSize(int first, int second) {
    return PixelFormatBytesPerPixel(first) > PixelFormatBytesPerPixel(second) ? PixelFormatBytesPerPixel(first)
                                                                         : PixelFormatBytesPerPixel(second);
}

// Reads `rows` rows of `rowSize` bytes; whether they were all read.
static inline int ReadRows(FileStream* stream, void* bits, int rowSize, int rows) {
    return stream->Read(bits, rowSize, rows) == rows;
}

// 0x004c6080: reads the texture from `stream`. Files with mip levels hold a
// table of level offsets (relative to where each entry is read) and the
// levels from 1x1 up, each either raw or compressed (a zero size means the
// size follows); levels at least `minimumSize` wide are converted into the
// surfaces slot 4 creates, larger levels than the choice allows are
// skipped. Other files hold one image, which is halved as the choice asks
// and handed to slot 4.
int PCTextureMap::LoadSurface(FileStream* stream, int width, int height, int minimumSize,
                              int fileFormat, int dataSize, int format, TextureColorMapperView* palette,
                              int flags, void* surfacePalette, int addressU, int addressV,
                              TextureContext* choice, int alphaThreshold, unsigned int key) {
    int offsets[12];
    DDSCaps2 caps;
    DDSurfaceDesc2 desc;
    int sourceFormat = DecodedFormat(fileFormat);
    if (choice) {
        format = IsFormat4444Or8888(format) ? choice->alphaFormat : choice->opaqueFormat;
        halvings = choice->halvings;
    } else {
        halvings = 0;
    }
    void* buffer = ownerManager->GrowScratchBuffer(LargerPixelSize(sourceFormat, format) * width * height);
    if (!buffer)
        return 0;
    void* bits = ownerManager->GrowSecondScratchBuffer(LargerPixelSize(sourceFormat, format) * width * height + 4);
    if (!bits)
        return 0;
    if (HasMipLevels(fileFormat)) {
        int levelWidth = width;
        if (choice) {
            for (int i = 0; i < choice->halvings && levelWidth > 32; i++) {
                levelWidth >>= 1;
                height >>= 1;
            }
        }
        if (!CreateSurface(0, levelWidth, height, levelWidth, minimumSize, sourceFormat, format, palette,
                           flags, surfacePalette, 1, 0, addressU, addressV, 0, alphaThreshold, key))
            return 0;

        int side = 1;
        int level = 0;
        int position;
        do {
            position = stream->Tell();
            if (stream->Read(&offsets[level], 4, 1) != 1)
                return 0;
            offsets[level] += position;
            side <<= 1;
            level++;
        } while (side <= width);
        position = stream->Tell();
        if (stream->Read(&offsets[level], 4, 1) != 1)
            return 0;
        offsets[level] += position;

        int levelSide = 1;
        int levelHeight = 1;
        level = 0;
        do {
            dataSize = offsets[level + 1] - offsets[level];
            if (IsCompressedFormat(fileFormat) &&
                dataSize != BytesPerPixel(fileFormat) * levelHeight * levelSide) {
                if (!dataSize && stream->Read(&dataSize, 4, 1) != 1)
                    return 0;
                if (stream->Read(buffer, 1, dataSize) != dataSize)
                    return 0;
                switch (fileFormat) {
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x18:
                case 0x19:
                case 0x1a:
                case 0x1b:
                    ExpandLzw(static_cast<unsigned char*>(bits), buffer, BytesPerPixel(fileFormat) * levelHeight * levelSide);
                    break;
                case 0x14:
                    bits = ownerManager->GrowSecondScratchBuffer(PixelFormatBytesPerPixel(sourceFormat) * levelHeight *
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
                DirectDrawSurface* surface = systemSurface;
                for (; surfaceSide > levelSide; surfaceSide /= 2) {
                    DirectDrawSurface* next;
                    long result = surface->GetAttachedSurface(&caps, &next);
                    if (result && result != (long)DDERR_NOTFOUND)
                        return 0;
                    surface = next;
                }
                if (surfaceSide == levelSide) {
                    memset(&desc, 0, sizeof(desc));
                    desc.size = sizeof(desc);
                    if (surface->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
                        return 0;
                    int stride = desc.pitch / PixelFormatBytesPerPixel(format);
                    ConvertPixels(desc.surface, bits, levelSide, levelHeight, stride, levelSide, mapFormat,
                                          sourceFormat, 0, palette, alphaThreshold, key);
                    if (surface->Unlock(0))
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
                    if (!dataSize && stream->Read(&dataSize, 4, 1) != 1)
                        return 0;
                    if (stream->Read(buffer, 1, dataSize) != dataSize)
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
        if (!dataSize && stream->Read(&dataSize, 4, 1) != 1)
            return 0;
        if (stream->Read(buffer, 1, dataSize) != dataSize)
            return 0;
        switch (fileFormat) {
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            ExpandLzw(static_cast<unsigned char*>(bits), buffer, BytesPerPixel(fileFormat) * width * height);
            break;
        case 5:
            bits = ownerManager->GrowSecondScratchBuffer(PixelFormatBytesPerPixel(sourceFormat) * width * height + 4);
            if (!bits)
                return 0;
            break;
        }
    } else {
        dataSize = BytesPerPixel(fileFormat) * width * height;
        if (stream->Read(bits, 1, dataSize) != dataSize)
            return 0;
    }
    if (choice) {
        int count = choice->halvings;
        while (width > 32 && count--) {
            if (Downsample(buffer, bits, width / 2, height / 2, width / 2, width, 1, sourceFormat,
                                      palette, 2)) {
                width /= 2;
                height /= 2;
                memcpy(bits, buffer, PixelFormatBytesPerPixel(sourceFormat) * width * height);
            }
        }
    }
    return CreateSurface(bits, width, height, width, minimumSize, sourceFormat, format, palette, flags,
                         surfacePalette, 1, 0, addressU, addressV, 0, alphaThreshold, key);
}

// 0x004c68e0: tries `formats` in turn (0-terminated) until the display
// creates +0x70 from `desc` in one the render target accepts (flag 4 skips
// that check), then counts the surface (4/3 more with mip levels) in DirectX
// memory.
int PCTextureMap::CreateSystemSurface(DDSurfaceDesc2* desc, int flags, int* formats) {
    for (; *formats; formats++) {
        mapFormat = *formats;
        if (desc->flags & 0x1000)
            FillPixelFormat(mapFormat, &desc->pixelFormat);
        if ((flags & 4 || g_TrackGame->renderTarget->SupportsTextureFormat(mapFormat)) &&
            !g_TrackGame->display->directDraw->CreateSurface(desc, &systemSurface, 0)) {
            float scale = 1.0f;
            if (mipCount != 1)
                scale = 4.0f / 3.0f;
            g_MemTagStack->AddDirectXBytes(
                (int)(PixelFormatBytesPerPixel(mapFormat) * textureHeight * textureWidth * scale));
            return 1;
        }
    }
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
// Not exact (518 of 2033 bytes, 2028 compiled: VC6 cross-jumps the identical
// CreateSystemSurface call tails of the non-mip format fallback chain into
// its first case, retail into its last; docs/PCTEXTUREMAP.md lists the forms
// tried and docs/TEXTURE_SURFACE_CREATE.md the behaviour review).
int PCTextureMap::CreateSurface(void* bits, int width, int height, int stride, int minimumSize,
                                int sourceFormat, int format, TextureColorMapperView* palette,
                                int flags, void* surfacePalette, int checkMemory, int unused,
                                int addressU, int addressV, TextureContext* choice,
                                int alphaThreshold, unsigned int key) {
    int formats[5];
    int mipmapped;
    DDSurfaceDesc2 desc;
    unsigned char pixelFormat[0x20];
    DDSCaps2 caps;
    int shared;
    textureWidth = width;
    textureHeight = height;
    minimumMipSize = minimumSize;
    colorMapper = palette;
    texturePalette = surfacePalette;
    if (choice)
        format = IsFormat4444Or8888(format) ? choice->alphaFormat : choice->opaqueFormat;
    mapFormat = format;
    FillPixelFormat(format, pixelFormat);
    mipmapped = flags & 2;
    mipCount = 1;
    if (!mipmapped) {
        minimumMipSize = width;
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
                if (!(flags & 8) && !g_TrackGame->bladeRenderer &&
                    !(g_TrackGame->texturesCached) &&
                    (mapFormat == 555 || mapFormat == 565)) {
                    shared = 1;
                    if (g_SharedTextureSurfaces[mipCount]) {
                        systemSurface = g_SharedTextureSurfaces[mipCount];
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
                g_SharedTextureSurfaces[mipCount] = systemSurface;
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
                // DDSCAPS flags; tools/reference_diff.py reads 0x401008 and 0x401808 as
                // .text addresses; they are DDSCAPS bits, not pointers (a false positive).
                desc.caps[0] = 0x401008;
                if (!(flags & 8) && !g_TrackGame->bladeRenderer &&
                    !(g_TrackGame->texturesCached) &&
                    (mapFormat == 555 || mapFormat == 565)) {
                    shared = 1;
                    if (g_SharedMipTextureSurfaces[mipCount]) {
                        systemSurface = g_SharedMipTextureSurfaces[mipCount];
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
                g_SharedMipTextureSurfaces[mipCount] = systemSurface;
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
            averageColor = AverageColor(destination, desc.width, desc.height, desc.pitch / bytesPerPixel,
                                               mapFormat, palette);
        if (systemSurface->Unlock(0))
            goto failed;
        if (mipmapped && !BuildMipLevels(2))
            goto failed;
    }
    if (g_DebugDumpTextureLevels)
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

// 0x004c71c0: a copy of the texture (a ManagedTexture registered with this
// one's cache, or a PCTextureMap of the game's manager), filled from +0x70
// through its slot 4 and given the same colour key; 0 on failure.
// Not exact (333 of 600 bytes, 590 compiled: VC6 keeps the constant 1 in ebp
// and the copy in ebx, retail the other way round; docs/PCTEXTUREMAP.md lists
// the forms tried and docs/TEXTURE_CLONE.md the behaviour review).
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
        DDSurfaceDesc2 desc;
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
                                              desc.pitch / PixelFormatBytesPerPixel(mapFormat), minimumMipSize,
                                              mapFormat, mapFormat, colorMapper, noAlpha | systemMemory,
                                              texturePalette, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
            if (!systemSurface->Unlock(0) && created) {
                if (usesColorKey) {
                    if (mapFormat == 8) {
                        copy->ApplyColorKey((colorMapper->entries[colorKeyValue][0] << 8 |
                                             colorMapper->entries[colorKeyValue][1]) << 8 |
                                                   colorMapper->entries[colorKeyValue][2]);
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

// 0x004c7420
int PCTextureMap::RestoreTextureSurface() {
    if (textureSurface && textureSurface->IsLost()) {
        textureSurface->Release();
        if (systemSurface == textureSurface)
            systemSurface = 0;
        textureSurface = 0;
        return CreateTextureSurface(1, 0, 0);
    }
    return 1;
}

// 0x004c7470
int PCTextureMap::IsResident() {
    return textureSurface != 0;
}

// 0x004c7480: creates the texture surface from +0x70 (or shares +0x70 when
// it is already in non-local video memory and `localVideo` is clear, or
// when Game+0x2d0 is set); `asRenderTarget` adds DDSCAPS_3DDEVICE and
// `upload` then asks slot 9 to upload it.
int PCTextureMap::CreateTextureSurface(int upload, int asRenderTarget, int localVideo) {
    if (textureSurface)
        goto done;
    if (systemSurface) {
        DDSCaps2 caps;
        if (systemSurface->GetCaps(&caps))
            goto failed;
        if (caps.caps & DDSCAPS_NONLOCALVIDMEM && !localVideo || g_TrackGame->bladeRenderer) {
            textureSurface = systemSurface;
            systemSurface->AddRef();
        } else {
            DDSurfaceDesc2 desc;
            memset(&desc, 0, sizeof(desc));
            desc.size = sizeof(desc);
            if (systemSurface->GetSurfaceDesc(&desc))
                goto failed;
            desc.flags &= 0x3f087;
            desc.caps[0] = desc.caps[0] & 0xcffff7ff | (asRenderTarget ? 0x2000 : 0) | DDSCAPS_VIDEOMEMORY;
            if (g_TrackGame->display->isAGP && localVideo)
                desc.caps[0] |= DDSCAPS_LOCALVIDMEM;
            desc.pitch = 0;
            if (!textureSurface &&
                g_TrackGame->display->directDraw->CreateSurface(&desc, &textureSurface, 0))
                goto failed;
            if (texturePalette && mapFormat == 8 && textureSurface->SetPalette(texturePalette))
                goto failed;
            if (g_DebugCountTextureMemory)
                ownerManager->CountTextureMemory();
        }
    }
    if (upload && textureSurface != systemSurface)
        Upload(0, -1);
    if (!textureSurface)
        goto failed;
done:
    return 1;
failed:
    return 0;
}

// 0x004c7610
void PCTextureMap::ReleaseTextureSurface() {
    if (textureSurface) {
        textureSurface->Release();
        textureSurface = 0;
        if (g_DebugCountTextureMemory)
            ownerManager->CountTextureMemory();
    }
}

// 0x004c7640: with partial texture blits (Display+0x5bc) or a positive
// `mode`, copies `rect` (or the whole texture) down the mip chain with
// BltFast, halving it per level; otherwise lets the device Load it. Then
// applies the colour key. The level loop tests its two sizes in separate
// `break`s: a `while (a && b)` condition is rotated to the bottom, retail
// re-runs the test at the top of every pass.
int PCTextureMap::Upload(Win32Rect* rect, int mode) {
    if (!systemSurface || !textureSurface)
        return 0;
    DirectDrawSurface* source = systemSurface;
    DirectDrawSurface* destination = textureSurface;
    if (mode > 0 || mode == -1 && g_TrackGame->display->partialTextureUploadResult > 0 ||
        g_TrackGame->display->partialTextureUploadResult <= 0) {
        Win32Rect area;
        if (rect && g_TrackGame->display->partialTextureUploadResult > 0) {
            area = *rect;
        } else {
            area.left = 0;
            area.top = 0;
            area.right = textureWidth;
            area.bottom = textureHeight;
        }
        DDSCaps2 caps;
        memset(&caps, 0, sizeof(caps));
        caps.caps = 0x401000;
        while (1) {
            if (area.right - area.left <= 0)
                break;
            if (area.bottom - area.top <= 0)
                break;
            if (destination->BltFast(area.left, area.top, source, &area, 0x10))
                return 0;
            DirectDrawSurface* nextSource;
            DirectDrawSurface* nextDestination;
            long sourceResult = source->GetAttachedSurface(&caps, &nextSource);
            long destinationResult = destination->GetAttachedSurface(&caps, &nextDestination);
            if (sourceResult || destinationResult)
                break;
            source = nextSource;
            destination = nextDestination;
            area.top >>= 1;
            area.left >>= 1;
            area.bottom >>= 1;
            area.right >>= 1;
        }
        if (usesColorKey && textureSurface->SetColorKey(8, &colorKeyValue))
            return 0;
        return 1;
    }
    if (destination != source)
        ((PCRenderTarget*)g_TrackGame->renderTarget)
            ->device->Load(destination, 0, source, 0, 0);
    return 0;
}

// 0x004c77d0: fills each mip level by downsampling the level above it.
int PCTextureMap::BuildMipLevels(int filter) {
    DirectDrawSurface* level;
    void* sourceBits;
    int bytesPerPixel;
    int stride;
    long result;
    DDSCaps2 caps;
    DDSurfaceDesc2 desc;
    memset(&caps, 0, sizeof(caps));
    caps.caps = 0x401000;
    bytesPerPixel = PixelFormatBytesPerPixel(mapFormat);
    DirectDrawSurface* parent = systemSurface;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (parent->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
        goto failed;
    stride = desc.pitch / bytesPerPixel;
    sourceBits = desc.surface;
    result = parent->GetAttachedSurface(&caps, &level);
    while (!result) {
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (level->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
            goto failed;
        int sourceStride = stride;
        stride = desc.pitch / bytesPerPixel;
        Downsample(desc.surface, sourceBits, desc.width, desc.height, stride, sourceStride, 1,
                              mapFormat, colorMapper, filter);
        if (parent->Unlock(0))
            goto failed;
        sourceBits = desc.surface;
        parent = level;
        result = parent->GetAttachedSurface(&caps, &level);
    }
    if (result != (long)DDERR_NOTFOUND) {
        ReportDirectDrawError(result, __FILE__, 1421);
        return 0;
    }
    if (parent->Unlock(0))
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004c7970
int PCTextureMap::SetAsTexture() {
    if (textureSurface &&
        !((PCRenderTarget*)g_TrackGame->renderTarget)->device->SetTexture(0, textureSurface))
        return 1;
    return 0;
}

// 0x004c79a0
void PCTextureMap::Bind() {
    SetAsTexture();
    for (int i = 0; i < statePairCount; i++)
        g_TrackGame->renderTarget->SetRenderState(statePairs[i].state, statePairs[i].value, 0);
}

// 0x004c79e0
void* PCTextureMap::LockSurface(void* rect, long* pitch, int flags) {
    DDSurfaceDesc2 desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (systemSurface->Lock(rect, &desc, flags, 0))
        return 0;
    if (pitch)
        *pitch = desc.pitch;
    return desc.surface;
}

// 0x004c7a50
int PCTextureMap::UnlockSurface(void* rect) {
    return !systemSurface->Unlock(rect);
}

// 0x004c7a70
void* PCTextureMap::LockLevel(int level) {
    DirectDrawSurface* surface = FindMipLevel(level);
    if (surface) {
        DDSurfaceDesc2 desc;
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (!surface->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
            return desc.surface;
    }
    return 0;
}

// 0x004c7ad0
int PCTextureMap::UnlockLevel(int level) {
    DirectDrawSurface* surface = FindMipLevel(level);
    if (surface && !surface->Unlock(0))
        return 1;
    return 0;
}

// 0x004c7b00
int PCTextureMap::BlitTo(void* destinationRect, DirectDrawSurface* destination,
                                        void* sourceRect, int flags, int skip) {
    if (!skip)
        return !destination->Blt(destinationRect, systemSurface, sourceRect, flags, 0);
    return 0;
}

// 0x004c7b40: copies `rect` of +0x70 to (x, y) in `destination`. Without a
// table this is BltFast; with one, both surfaces are locked and each
// destination pixel becomes table[source << 8 | destination], clipped to
// the destination. Every non-8-bit format follows the WORD path and reads
// a byte through the destination WORD value, including its low-address fault.
// Not exact (364 of 750 bytes, 775 compiled: the destination WORD is read
// before the source one, as retail faults; register choice and loop entry
// differ; docs/TEXTURE_RECT_COPY.md).
int PCTextureMap::CopyRectTo(unsigned long x, unsigned long y, DirectDrawSurface* destination,
                                        Win32Rect* rect, int flags, unsigned char* table) {
    DDSurfaceDesc2 source;
    DDSurfaceDesc2 target;
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
    top = rect->top;
    bottom = rect->bottom;
    left = rect->left;
    right = rect->right;
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
                for (long column = left; column <= right; column++, pixel++) {
                    // Preserve the observed destination-before-source access/fault order.
                    unsigned short destinationPixel = *(volatile unsigned short*)pixel;
                    unsigned short sourcePixel = *(volatile unsigned short*)(from + column - left);
                    *pixel = table[(sourcePixel << 8) + *(unsigned char*)destinationPixel];
                }
                from += source.pitch / 2;
                to += target.pitch / 2;
            }
        }
    }
    if (destination->Unlock(0))
        goto failed;
    if (systemSurface->Unlock(0))
        goto failed;
    return 1;
failed:
    return 0;
}

// Sets render state `state` to `value` in the texture's list, appending it
// when absent.
static inline void SetRenderStatePair(TextureMap* map, int state, int value) {
    int i;
    for (i = 0; i < map->statePairCount; i++) {
        if (map->statePairs[i].state == state)
            break;
    }
    if (i < map->statePairCount) {
        map->statePairs[i].value = value;
    } else {
        map->statePairs[map->statePairCount].state = state;
        map->statePairs[map->statePairCount].value = value;
        map->statePairCount++;
    }
}

// 0x004c7e30: converts a 24-bit colour to the texture's format (555, 565 or
// a palette index) and stores it as the colour key.
void PCTextureMap::SetColorKey(unsigned int color) {
    int key;
    if (mapFormat == 555)
        key = Pack555(color);
    else if (mapFormat == 565)
        key = Pack565(color);
    else if (mapFormat == 8)
        key = colorMapper->indexFrom555[Pack555(color)];
    else
        key = color;
    colorKeyValue = colorKeyHighValue = key;
}

// 0x004c7ef0: replaces magenta in `surface` with `color` and makes `color`
// (converted to the texture's format) the colour key. Magenta itself only
// sets the key, except in format 8888, where the key pixels also lose
// their alpha.
int PCTextureMap::ColorKeyLevel(DirectDrawSurface* surface, unsigned int color) {
    if (color == 0xff00ff && mapFormat != 8888) {
        SetColorKey(color);
        return 1;
    }
    DDSurfaceDesc2 desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (surface->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0))
        goto failed;
    if (mapFormat == 8888) {
        RgbaPixel from;
        RgbaPixel to;
        colorKeyValue = colorKeyHighValue = color;
        from.red = 0xff;
        from.green = 0;
        from.blue = 0xff;
        from.alpha = 0xff;
        to.red = (unsigned char)(color >> 16);
        to.green = (unsigned char)(color >> 8);
        to.blue = (unsigned char)color;
        to.alpha = 0;
        ReplaceColor32(desc.surface, from, to, desc.width, desc.height,
                              desc.pitch / PixelFormatBytesPerPixel(8888));
    }
    if (mapFormat == 888) {
        RgbPixel from;
        RgbPixel to;
        colorKeyValue = colorKeyHighValue = color;
        from.red = 0xff;
        from.green = 0;
        from.blue = 0xff;
        to.red = (unsigned char)(color >> 16);
        to.green = (unsigned char)(color >> 8);
        to.blue = (unsigned char)color;
        ReplaceColor24(desc.surface, from, to, desc.width, desc.height,
                              desc.pitch / PixelFormatBytesPerPixel(888));
    }
    if (mapFormat == 555 || mapFormat == 1555) {
        colorKeyValue = colorKeyHighValue = Pack555(color);
        ReplaceColor16(desc.surface, 0x7c1f, Pack555(color), desc.width, desc.height,
                              desc.pitch / PixelFormatBytesPerPixel(mapFormat));
    } else if (mapFormat == 565) {
        colorKeyValue = colorKeyHighValue = Pack565(color);
        ReplaceColor16(desc.surface, 0xf81f, Pack565(color), desc.width, desc.height,
                              desc.pitch / PixelFormatBytesPerPixel(565));
    } else if (mapFormat == 8) {
        colorKeyValue = colorKeyHighValue = colorMapper->indexFrom555[Pack555(color)];
        ReplaceColor8(desc.surface, colorMapper->indexFrom555[0x7c1f],
                              colorMapper->indexFrom555[Pack555(color)], desc.width, desc.height, desc.pitch);
    }
    if (surface->Unlock(0))
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004c81d0: for 555, 565, 24-bit and 1555 formats, applies colour `color` as the
// key on every level (0x004c7ef0), re-uploads, sets the surfaces' colour key
// and records render states 0x29 = 1 and 0x1b = 0.
int PCTextureMap::ApplyColorKey(unsigned int color) {
    if (mapFormat != 555 && mapFormat != 565 && mapFormat != 888 && mapFormat != 1555)
        return 0;
    if (systemSurface) {
        ColorKeyLevel(systemSurface, color);
        if (mipCount > 1) {
            DirectDrawSurface* surface;
            DDSCaps2 caps;
            DirectDrawSurface* top = systemSurface;
            memset(&caps, 0, sizeof(caps));
            caps.caps = 0x401000;
            long result = top->GetAttachedSurface(&caps, &surface);
            while (!result) {
                ColorKeyLevel(surface, color);
                result = surface->GetAttachedSurface(&caps, &surface);
            }
            if (result != (long)DDERR_NOTFOUND) {
                ReportDirectDrawError(result, __FILE__, 1811);
                return 0;
            }
        }
    }
    if ((color != 0xff00ff || mapFormat == 8888) && textureSurface)
        Upload(0, -1);
    if (mapFormat == 1555)
        return 1;
    if (systemSurface && systemSurface->SetColorKey(8, &colorKeyValue) ||
        textureSurface && textureSurface->SetColorKey(8, &colorKeyValue))
        return 0;
    usesColorKey = 1;
    SetRenderStatePair(this, 0x29, 1);
    SetRenderStatePair(this, 0x1b, 0);
    return 1;
}

// 0x004c83a0: follows the attached mip surfaces, halving the width, until it
// reaches `width`. (`next` shares the dead parameter's stack slot, as in
// retail.)
DirectDrawSurface* PCTextureMap::FindMipLevel(int width) {
    DDSCaps2 caps;
    memset(&caps, 0, sizeof(caps));
    int size = textureWidth;
    DirectDrawSurface* surface = systemSurface;
    caps.caps = 0x401000;
    long result = 0;
    while (size != width) {
        DirectDrawSurface* next;
        result = surface->GetAttachedSurface(&caps, &next);
        if (result)
            break;
        surface = next;
        size /= 2;
    }
    if (result && result != (long)DDERR_NOTFOUND)
        ReportDirectDrawError(result, __FILE__, 2034);
    else if (size == width)
        return surface;
    return 0;
}

// 0x004c8430: dumps every mip level (0x004c84e0).
int PCTextureMap::DumpAllLevels() {
    if (systemSurface) {
        DumpLevel(systemSurface, 0);
        if (mipCount > 1) {
            DirectDrawSurface* surface;
            DDSCaps2 caps;
            DirectDrawSurface* top = systemSurface;
            memset(&caps, 0, sizeof(caps));
            caps.caps = 0x401000;
            long result = top->GetAttachedSurface(&caps, &surface);
            while (!result) {
                DumpLevel(surface, 0);
                result = surface->GetAttachedSurface(&caps, &surface);
            }
            if (result != (long)DDERR_NOTFOUND) {
                ReportDirectDrawError(result, __FILE__, 2084);
                return 0;
            }
        }
    }
    return 1;
}

// 0x004c84e0: locks a level and writes it to a file (0x004c8550).
int PCTextureMap::DumpLevel(DirectDrawSurface* surface, const char* name) {
    DDSurfaceDesc2 desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    surface->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_READONLY | DDLOCK_NOSYSLOCK, 0);
    WriteLevel(&desc, name);
    surface->Unlock(0);
    return 1;
}

// 0x004c8550: writes a locked level to C:\temp\<name><nnn>.bmp (8-bit) or
// .tga (anything else, converted to 32-bit first), taking the first number
// with no existing file. `name` defaults to "tex".
void PCTextureMap::WriteLevel(DDSurfaceDesc2* desc, const char* name) {
    char path[260];
    BitmapFile bitmap;
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
        // Reading the description through a second name here makes VC6 load the height before
        // the width in the .tga branch's buffer size, as retail does.
        const DDSurfaceDesc2& level = *desc;
        DescribeBitmap8(&bitmap, level.surface, colorMapper->entries, level.width, level.height);
        int length = strlen(path);
        int count = length > 0x7f ? 0x7f : length;
        strncpy(bitmap.name, path, count);
        bitmap.name[count] = 0;
        WriteBitmap(&bitmap);
    } else {
        void* pixels = DebugMalloc(desc->width * desc->height * sizeof(unsigned int), __FILE__, 2149);
        ConvertPixels(pixels, desc->surface, desc->width, desc->height, desc->width,
                              desc->pitch / PixelFormatBytesPerPixel(mapFormat), 8888, mapFormat, 0, 0, 0x80,
                              0xff00ff);
        WriteTga32(pixels, desc->width, desc->height, 0, path, 32);
        DebugFree(pixels, __FILE__, 2154);
    }
}

// 0x004c86e0: formats the name of a DirectDraw result into a local buffer.
// Nothing reads the text, `file` or `line` in retail (the output was
// presumably compiled out). Cases are in retail's body order.
void ReportDirectDrawError(long result, const char* file, int line) {
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
