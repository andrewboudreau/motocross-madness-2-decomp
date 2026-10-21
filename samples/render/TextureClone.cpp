// Complete PCTextureMap::Clone candidate; original TU ownership is unproven.
// Retail 0x004c71c0, 600 bytes. See docs/TEXTURE_CLONE.md.
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/TrackGame.h"

#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/D3DConstants.h"

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
