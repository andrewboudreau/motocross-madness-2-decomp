// Complete nonexact menu texture color scan; original TU ownership is unproven.
// Retail 0x0047b490 spans 220 bytes. See docs/TEXTURE_HAS_COLOR.md.
#include "../../src/reconstructed/TextureMap.h"

int TextureHasColor(TextureMap* texture, int key) {
    int found = 0;
    unsigned short* bits = (unsigned short*)texture->LockSurface(0, 0, 0x11);
    if (bits) {
        int pixel;
        int green;
        if (texture->mapFormat == 0x235) {
            pixel = (key >> 8) & 0xf800;
            green = (key >> 5) & 0x7e0;
        } else if (texture->mapFormat == 0x22b) {
            pixel = (key >> 9) & 0x7c00;
            green = (key >> 6) & 0x3e0;
        } else {
            goto unlock;
        }
        pixel |= green | ((key >> 3) & 0x1f);
        if (pixel) {
            for (int rows = texture->textureHeight; rows > 0; rows--) {
                int x;
                for (x = 0; x < texture->textureWidth; x++) {
                    if (bits[x] == pixel)
                        break;
                }
                if (x < texture->textureWidth)
                    found = 1;
                bits += texture->textureWidth;
            }
        }
unlock:
        if (!texture->UnlockSurface(0))
            return 0;
        return found;
    }
    return 0;
}
