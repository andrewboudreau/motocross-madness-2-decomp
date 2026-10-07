// Near-miss Pixtrans.cpp candidates, kept out of src/reconstructed until they
// match. See docs/PIXTRANS.md.
//
// Halve8888 (0x004cdf10, 636 bytes with its jump table): the
// 8888 halver. Control flow, the count switch and the retail alpha quirk
// line up, but retail keeps the upper row pointer in ecx and the lower one
// on the stack, VC6 here the other way round, so most registers differ.
// Accumulator declaration order (all 120) and an indexed lower row do not
// change it.
//
// Convert565To8888 (0x004d0700, 208 bytes): 565 to 8888 with a key.
// The row pointers and the 0xff constant land in different registers and
// slots (retail keeps 0xff in dl and both rows in argument slots); about 60
// lines differ in every pointer-order and key-placement variant.
//
// Convert1555To8888 (0x004d07d0, 146 bytes): 1555 to 8888. Only the
// pixel pointer's base offset differs (retail addresses the pixel from its
// alpha byte, VC6 here from blue); 7 lines.
//
// Halve1555 (0x004ce420, 456 bytes; candidate 448): the 1555
// halver behind 0x004d0170. The arithmetic, the >= 2 test and the packing
// match. Retail loads the two lower-row pixels before the first test and
// keeps them in frame slots (a 0x10-byte frame); VC6 here loads each where
// it is first tested (0x8 frame). Locals in every declaration order, a
// lower-row pointer, function-scope pixels, a 1555 bitfield struct and
// direct indexing do not keep them.
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

// 0x004d0700: converts 565 to 8888; the key colour becomes opaque magenta.
int Convert565To8888(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert1555To8888(void* destination, void* source, int width, int height, int destinationStride,
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
int Halve8888(void* destination, void* source, int width, int height, int destinationStride,
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
    case 565: {
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
    case 555: {
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
    case 888: {
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
    case 8888: {
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
    case 4444: {
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

// 0x004ce420: halves 1555 pixels; colour averages the opaque pixels of
// each 2x2 block, which stays opaque when at least two of them are.
int Halve1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* top = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, to++, top += 2) {
            unsigned short pixel0 = top[0];
            unsigned short pixel1 = top[1];
            unsigned short pixel2 = top[sourceStride];
            unsigned short pixel3 = top[sourceStride + 1];
            int count = 0;
            int red = 0;
            int green = 0;
            int blue = 0;
            if (pixel0 & 0x8000) {
                red += (pixel0 >> 7) & 0xf8;
                green += (pixel0 >> 2) & 0xf8;
                blue += (pixel0 & 0x1f) << 3;
                count++;
            }

            if (pixel1 & 0x8000) {
                red += (pixel1 >> 7) & 0xf8;
                green += (pixel1 >> 2) & 0xf8;
                blue += (pixel1 & 0x1f) << 3;
                count++;
            }
            if (pixel2 & 0x8000) {
                red += (pixel2 >> 7) & 0xf8;
                green += (pixel2 >> 2) & 0xf8;
                blue += (pixel2 & 0x1f) << 3;
                count++;
            }
            if (pixel3 & 0x8000) {
                red += (pixel3 >> 7) & 0xf8;
                green += (pixel3 >> 2) & 0xf8;
                blue += (pixel3 & 0x1f) << 3;
                count++;
            }
            if (count >= 2) {
                red = (red + count * 4) / count;
                green = (green + count * 4) / count;
                blue = (blue + count * 4) / count;
                *to = 0x8000 | ((red >> 3) << 10) | ((green >> 3) << 5) | (blue >> 3);
            } else {
                *to = 0;
            }
        }
        sourceRow += sourceStride * 2;
        row += destinationStride;
    }
    return 1;
}
