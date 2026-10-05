// Near-miss Pixtrans.cpp candidates, kept out of src/reconstructed until they
// match. See docs/PIXTRANS.md.
//
// UnknownFunction4cdf10 (0x004cdf10, 636 bytes with its jump table): the
// 8888 halver. Control flow, the count switch and the retail alpha quirk
// line up, but retail keeps the upper row pointer in ecx and the lower one
// on the stack, VC6 here the other way round, so most registers differ.
// Accumulator declaration order (all 120) and an indexed lower row do not
// change it.
//
// UnknownFunction4d0aa0 (0x004d0aa0, 232 bytes) and UnknownFunction4d0b90
// (0x004d0b90, 171 bytes): 24-bit and 565 to palette indices. Retail loads
// both `dither` and `palette` before testing either; VC6 here loads
// `palette` only after the first test. Comparison and local forms of the
// condition do not change it; everything else matches.
//
// UnknownFunction4d0700 (0x004d0700, 208 bytes): 565 to 8888 with a key.
// The row pointers and the 0xff constant land in different registers and
// slots (retail keeps 0xff in dl and both rows in argument slots); about 60
// lines differ in every pointer-order and key-placement variant.
//
// UnknownFunction4d07d0 (0x004d07d0, 146 bytes): 1555 to 8888. Only the
// pixel pointer's base offset differs (retail addresses the pixel from its
// alpha byte, VC6 here from blue); 7 lines.
//
// UnknownFunction4cfaf0 (0x004cfaf0, 327 bytes): the 24-bit downsampler
// behind 0x004d1b90. Everything but the plain-copy path (levels == 0)
// matches: retail hoists width * 3, destinationStride * 3 and
// sourceStride * 3 in that order into the height, destination-stride and
// width argument slots; VC6 here computes the source step first and puts
// it in the source-stride slot. Advancing the parameters themselves is
// required (pointer locals cost 50 lines); reordering the advances, typed
// strides, a down-counting loop and for-increment advances do not fix the
// slots. Its 32-bit sibling 0x004cfc40 has the same copy path.

#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Pixtrans.h"

// 0x004cfaf0: shrinks 24-bit `source` by `levels` halvings into the width x
// height `destination` (0: a plain copy). Intermediate levels go through a
// buffer of the first level's size.
int UnknownFunction4cfaf0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels) {
    if (levels == 0) {
        for (int y = 0; y < height; y++) {
            memcpy(destination, source, width * 3);
            source = (unsigned char*)source + sourceStride * 3;
            destination = (unsigned char*)destination + destinationStride * 3;
        }
        return 1;
    }
    if (levels == 1) {
        UnknownFunction4cde20(destination, source, width, height, destinationStride, sourceStride);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth * 3, __FILE__, 1329);
    UnknownFunction4cde20(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        UnknownFunction4cde20(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2);
    }
    UnknownFunction4cde20(destination, buffer, width, height, destinationStride, levelWidth);
    operator delete(buffer, __FILE__, 1350);
    return 1;
}

// 0x004d0700: converts 565 to 8888; the key colour becomes opaque magenta.
int UnknownFunction4d0700(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key) {
    unsigned short transparent = Pack565(key);
    unsigned short* sourceRow = (unsigned short*)source;
    UnknownPixel32* row = (UnknownPixel32*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        UnknownPixel32* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            if (*from == transparent) {
                to->red = 0xff;
                to->green = 0;
                to->blue = 0xff;
            } else {
                to->red = (*from >> 8) & 0xf8;
                to->green = (*from >> 3) & 0xfc;
                to->blue = *from << 3;
            }
            to->alpha = 0xff;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d07d0: converts 1555 to 8888.
int UnknownFunction4d07d0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    unsigned short* sourceRow = (unsigned short*)source;
    UnknownPixel32* row = (UnknownPixel32*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        UnknownPixel32* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            to->red = (*from >> 7) & 0xf8;
            to->green = (*from >> 2) & 0xf8;
            to->blue = *from << 3;
            to->alpha = (*from & 0x8000) ? 0xff : 0;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d0aa0: converts 24-bit to palette indices through the 555 table.
int UnknownFunction4d0aa0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, UnknownTexturePalette* palette) {
    if (dither && palette) {
        UnknownFunction4cf2a0(source, 0x378, width, height, sourceStride, destinationStride, 0, 0, 0,
                              destination, palette);
    } else {
        UnknownPixel24* sourceRow = (UnknownPixel24*)source;
        unsigned char* row = (unsigned char*)destination;
        unsigned char* indices = UnknownFunction4de280();
        for (int y = 0; y < height; y++) {
            UnknownPixel24* from = sourceRow;
            unsigned char* to = row;
            for (int x = 0; x < width; x++, to++, from++)
                *to = indices[(unsigned short)((from->red >> 3) << 10 | (from->green >> 3) << 5) | from->blue >> 3];
            sourceRow += sourceStride;
            row += destinationStride;
        }
    }
    return 1;
}

// 0x004d0b90: converts 565 to palette indices through the 565 table.
int UnknownFunction4d0b90(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, UnknownTexturePalette* palette) {
    if (dither && palette) {
        UnknownFunction4cf2a0(source, 0x235, width, height, sourceStride, destinationStride, 0, 0, 0,
                              destination, palette);
    } else {
        unsigned short* sourceRow = (unsigned short*)source;
        unsigned char* row = (unsigned char*)destination;
        unsigned char* indices = UnknownFunction4de290();
        for (int y = 0; y < height; y++) {
            unsigned short* from = sourceRow;
            unsigned char* to = row;
            for (int x = 0; x < width; x++, to++, from++)
                *to = indices[*from];
            sourceRow += sourceStride;
            row += destinationStride;
        }
    }
    return 1;
}

// 0x004cdf10: halves 8888 pixels. Colour averages the 2x2 pixels whose
// alpha is set; alpha is the block's sum / 4. (The fourth pixel adds the
// third one's alpha, as in retail.)
int UnknownFunction4cdf10(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    UnknownPixel32* sourceRow = (UnknownPixel32*)source;
    UnknownPixel32* row = (UnknownPixel32*)destination;
    for (int y = 0; y < height; y++) {
        UnknownPixel32* top = sourceRow;
        UnknownPixel32* bottom = sourceRow + sourceStride;
        UnknownPixel32* to = row;
        for (int x = 0; x < width; x++, to++, top += 2, bottom += 2) {
            int red = 0;
            int green = 0;
            int blue = 0;
            int alpha = 0;
            int count = 0;
            unsigned char alpha0 = top[0].alpha;
            if (alpha0) {
                red += top[0].red;
                green += top[0].green;
                blue += top[0].blue;
                alpha += alpha0;
                count++;
            }
            unsigned char alpha1 = top[1].alpha;
            if (alpha1) {
                red += top[1].red;
                green += top[1].green;
                blue += top[1].blue;
                alpha += alpha1;
                count++;
            }
            unsigned char alpha2 = bottom[0].alpha;
            if (alpha2) {
                red += bottom[0].red;
                green += bottom[0].green;
                blue += bottom[0].blue;
                alpha += alpha2;
                count++;
            }
            if (bottom[1].alpha) {
                red += bottom[1].red;
                green += bottom[1].green;
                blue += bottom[1].blue;
                alpha += alpha2;
                count++;
            }
            switch (count) {
            case 0:
                to->red = 0;
                to->green = 0;
                to->blue = 0;
                to->alpha = 0;
                break;
            case 1:
                to->red = red;
                to->green = green;
                to->blue = blue;
                to->alpha = alpha >> 2;
                break;
            case 2:
                to->red = red >> 1;
                to->green = green >> 1;
                to->blue = blue >> 1;
                to->alpha = alpha >> 2;
                break;
            case 3:
                to->red = red / 3;
                to->green = green / 3;
                to->blue = blue / 3;
                to->alpha = alpha >> 2;
                break;
            case 4:
                to->red = red >> 2;
                to->green = green >> 2;
                to->blue = blue >> 2;
                to->alpha = alpha >> 2;
                break;
            }
        }
        sourceRow += sourceStride * 2;
        row += destinationStride;
    }
    return 1;
}
