// Complete nonexact channel comparison; retail 0x004cb330..0x004cb5a4 (629 bytes).
#include <math.h>
#include "../../src/reconstructed/TextureMap.h"

// Expands 16-bit 5:5:5 and 5:6:5 pixels to 0xRRGGBB (low bits zero).
#define UNKNOWN_EXPAND_555(c) (((((c) & 0x7c00) << 3 | ((c) & 0x3e0)) << 3 | ((c) & 0x1f)) << 3)
#define UNKNOWN_EXPAND_565(c) (((((c) & 0xf800) << 3 | ((c) & 0x7e0)) << 2 | ((c) & 0x1f)) << 3)

// 0x004cb330: the mean absolute channel difference (1/256 units) of two
// 16-bit textures of `first`'s size, or -1 when one cannot be locked.
float MeanTextureChannelDifference(TextureMap* first, TextureMap* second) {
    long firstPitch;
    long secondPitch;
    unsigned short* a = (unsigned short*)first->LockSurface(0, &firstPitch, 0x811);
    if (!a)
        return -1.0f;
    unsigned short* b = (unsigned short*)second->LockSurface(0, &secondPitch, 0x811);
    if (!b) {
        second->UnlockSurface(0);
        return -1.0f;
    }
    int width = first->textureWidth;
    int is555 = first->mapFormat == 0x22b;
    float total = 0.0f;
    int firstSkip = firstPitch / sizeof(unsigned short) - width;
    int secondSkip = secondPitch / sizeof(unsigned short) - width;
    int count = 0;
    for (int rows = first->textureHeight; rows--;) {
        for (int columns = width; columns--;) {
            int pixelA;
            int pixelB;
            count++;
            if (is555) {
                pixelA = UNKNOWN_EXPAND_555(*a);
                pixelB = UNKNOWN_EXPAND_555(*b);
            } else {
                pixelA = UNKNOWN_EXPAND_565(*a);
                pixelB = UNKNOWN_EXPAND_565(*b);
            }
            a++;
            b++;
            float difference = (float)fabs(((pixelB >> 16) & 0xff) * 0.00390625f - ((pixelA >> 16) & 0xff) * 0.00390625f);
            total += difference;
            difference = (float)fabs(((pixelB >> 8) & 0xff) * 0.00390625f - ((pixelA >> 8) & 0xff) * 0.00390625f);
            total += difference;
            difference = (float)fabs((pixelB & 0xff) * 0.00390625f - (pixelA & 0xff) * 0.00390625f);
            total += difference;
        }
        a += firstSkip;
        b += secondSkip;
    }
    first->UnlockSurface(0);
    second->UnlockSurface(0);
    return total / (count * 3);
}
