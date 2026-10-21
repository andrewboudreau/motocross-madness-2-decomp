// Complete texture surface setup candidate.
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/TrackGame.h"

#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/D3DConstants.h"

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
