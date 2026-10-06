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
//
// UnknownFunction4cfc40 (0x004cfc40, 351 bytes): the 8888 downsampler; all
// but the copy path matches (7 lines). Retail computes width * 4,
// destinationStride * 4 and sourceStride * 4 in that order into the height,
// destination-stride and width slots. Advancing `destination` first gives
// that order but puts the destination step in the width slot;
// `destinationStride *= 4` before the loop gives the slots but computes it
// before the height test. Operand order, typed advances and a helper do not
// help.
//
// UnknownFunction4cfda0 (0x004cfda0, 208 bytes): 24-bit to 8888, keyed
// (behind 0x004cfe70, which matches). Retail walks plain byte pointers from
// the row start and hoists both row steps into argument slots; VC6 here
// biases the pixel pointers (+2/+3) and recomputes the steps (about 50
// lines in every struct, byte-pointer and parameter-advancing form).
//
// UnknownFunction4d24d0 (0x004d24d0, 987 bytes): a bitmap's average colour
// (0xAARRGGBB) by format. Every case body matches; the shared palette and
// 24-bit return lands after the 24-bit loop here but after the palette
// loop in retail. Explicit identical returns move it there, but then VC6
// also merges the 565 and 555 tails; a goto or reordered cases do not
// change it.

#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Pixtrans.h"
#include "../../src/reconstructed/TextureMap.h"

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

// 0x004cfc40: shrinks 8888 `source` by `levels` halvings (0: a plain
// copy), like 0x004cfaf0.
int UnknownFunction4cfc40(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels) {
    if (levels == 0) {
        for (int y = 0; y < height; y++) {
            memcpy(destination, source, width * 4);
            source = (unsigned char*)source + sourceStride * 4;
            destination = (unsigned char*)destination + destinationStride * 4;
        }
        return 1;
    }
    if (levels == 1) {
        UnknownFunction4cdf10(destination, source, width, height, destinationStride, sourceStride);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth * 4, __FILE__, 1390);
    if (!buffer)
        return 0;
    UnknownFunction4cdf10(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        UnknownFunction4cdf10(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2);
    }
    UnknownFunction4cdf10(destination, buffer, width, height, destinationStride, levelWidth);
    operator delete(buffer, __FILE__, 1412);
    return 1;
}

// 0x004cfda0: converts 24-bit to 8888; the 0xRRGGBB `key` becomes
// transparent.
void UnknownFunction4cfda0(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, unsigned int key) {
    int keyColor[3];
    keyColor[0] = (key >> 16) & 0xff;
    keyColor[1] = (key >> 8) & 0xff;
    keyColor[2] = key & 0xff;
    for (int y = 0; y < height; y++) {
        unsigned char* from = (unsigned char*)source;
        unsigned char* to = (unsigned char*)destination;
        for (int x = 0; x < width; x++) {
            to[0] = from[0];
            to[1] = from[1];
            to[2] = from[2];
            if (from[0] == keyColor[0] && from[1] == keyColor[1] && from[2] == keyColor[2])
                to[3] = 0;
            else
                to[3] = 0xff;
            from += 3;
            to += 4;
        }
        source = (unsigned char*)source + sourceStride * 3;
        destination = (unsigned char*)destination + destinationStride * 4;
    }
}

#define UNKNOWN_ARGB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

// 0x004d24d0: the average colour of a width x height bitmap of `format`
// (rows of `width` pixels; `stride` is unused), as 0xAARRGGBB with opaque
// alpha for formats without one; 0 for other formats.
int UnknownFunction4d24d0(void* bits, int width, int height, int stride, int format,
                          UnknownTexturePalette* palette) {
    unsigned int alpha = 0;
    unsigned int red = 0;
    unsigned int green = 0;
    unsigned int blue = 0;
    unsigned int count = width * height;
    int x;
    int y;

    switch (format) {
    case 0x235: {
        unsigned short* row = (unsigned short*)bits;
        for (y = 0; y < height; y++) {
            unsigned short* pixel = row;
            for (x = 0; x < width; x++, pixel++) {
                red += *pixel >> 11;
                green += (*pixel >> 5) & 0x3f;
                blue += *pixel & 0x1f;
            }
            row += width;
        }
        return UNKNOWN_ARGB(0xff, red * 8 / count, green * 4 / count, blue * 8 / count);
    }
    case 0x22b: {
        unsigned short* row = (unsigned short*)bits;
        for (y = 0; y < height; y++) {
            unsigned short* pixel = row;
            for (x = 0; x < width; x++, pixel++) {
                red += (*pixel >> 10) & 0x1f;
                green += (*pixel >> 5) & 0x1f;
                blue += *pixel & 0x1f;
            }
            row += width;
        }
        return UNKNOWN_ARGB(0xff, red * 8 / count, green * 8 / count, blue * 8 / count);
    }
    case 8: {
        unsigned char* row = (unsigned char*)bits;
        for (y = 0; y < height; y++) {
            unsigned char* pixel = row;
            for (x = 0; x < width; x++, pixel++) {
                red += palette->field_0x010[*pixel][0];
                green += palette->field_0x010[*pixel][1];
                blue += palette->field_0x010[*pixel][2];
            }
            row += width;
        }
        break;
    }
    case 0x378: {
        UnknownPixel24* row = (UnknownPixel24*)bits;
        for (y = 0; y < height; y++) {
            UnknownPixel24* pixel = row;
            for (x = 0; x < width; x++, pixel++) {
                red += pixel->red;
                green += pixel->green;
                blue += pixel->blue;
            }
            row += width;
        }
        break;
    }
    case 0x22b8: {
        UnknownPixel32* row = (UnknownPixel32*)bits;
        for (y = 0; y < height; y++) {
            UnknownPixel32* pixel = row;
            for (x = 0; x < width; x++, pixel++) {
                alpha += pixel->alpha;
                red += pixel->red;
                green += pixel->green;
                blue += pixel->blue;
            }
            row += width;
        }
        return UNKNOWN_ARGB(alpha / count, red / count, green / count, blue / count);
    }
    case 0x115c: {
        unsigned short* row = (unsigned short*)bits;
        for (y = 0; y < height; y++) {
            unsigned short* pixel = row;
            for (x = 0; x < width; x++, pixel++) {
                alpha += *pixel >> 12;
                red += (*pixel >> 8) & 0xf;
                green += (*pixel >> 4) & 0xf;
                blue += *pixel & 0xf;
            }
            row += width;
        }
        return UNKNOWN_ARGB(alpha * 16 / count, red * 16 / count, green * 16 / count, blue * 16 / count);
    }
    default:
        return 0;
    }
    return UNKNOWN_ARGB(0xff, red / count, green / count, blue / count);
}
