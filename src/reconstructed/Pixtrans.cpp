// Pixtrans.cpp's pixel converters. Names are provisional; see Pixtrans.h.

#include "Pixtrans.h"

// 0x004d1970: replaces 32-bit `from` pixels with `to`; pixels already equal
// to `to` get their blue byte nudged (255 down, otherwise up) first.
int UnknownFunction4d1970(void* bits, UnknownPixel32 from, UnknownPixel32 to, int width, int height, int stride) {
    UnknownPixel32* row = (UnknownPixel32*)bits;
    for (int y = 0; y < height; y++) {
        UnknownPixel32* pixel = row;
        for (int x = 0; x < width; x++, pixel++) {
            if (pixel->red == to.red && pixel->green == to.green && pixel->blue == to.blue &&
                pixel->alpha == to.alpha) {
                if (pixel->blue == 0xff)
                    pixel->blue = 0xfe;
                else
                    pixel->blue++;
            }
            if (pixel->red == from.red && pixel->green == from.green && pixel->blue == from.blue &&
                pixel->alpha == from.alpha) {
                pixel->red = to.red;
                pixel->green = to.green;
                pixel->blue = to.blue;
                pixel->alpha = to.alpha;
            }
        }
        row += stride;
    }
    return 1;
}

// 0x004d1a20: the 24-bit replacer.
int UnknownFunction4d1a20(void* bits, UnknownPixel24 from, UnknownPixel24 to, int width, int height, int stride) {
    UnknownPixel24* row = (UnknownPixel24*)bits;
    for (int y = 0; y < height; y++) {
        UnknownPixel24* pixel = row;
        for (int x = 0; x < width; x++, pixel++) {
            if (pixel->red == to.red && pixel->green == to.green && pixel->blue == to.blue) {
                if (pixel->blue == 0xff)
                    pixel->blue = 0xfe;
                else
                    pixel->blue++;
            }
            if (pixel->red == from.red && pixel->green == from.green && pixel->blue == from.blue) {
                pixel->red = to.red;
                pixel->green = to.green;
                pixel->blue = to.blue;
            }
        }
        row += stride;
    }
    return 1;
}

// 0x004d1ac0: the 16-bit replacer; a pixel equal to `to` moves one step
// (down when its low five bits are all set, otherwise up).
int UnknownFunction4d1ac0(void* bits, unsigned short from, unsigned short to, int width, int height, int stride) {
    unsigned short* row = (unsigned short*)bits;
    for (int y = 0; y < height; y++) {
        unsigned short* pixel = row;
        for (int x = 0; x < width; x++, pixel++) {
            if (*pixel == to) {
                if ((*pixel & 0x1f) == 0x1f)
                    *pixel = to - 1;
                else
                    *pixel = to + 1;
            }
            if (*pixel == from)
                *pixel = to;
        }
        row += stride;
    }
    return 1;
}

// 0x004d1b40: the 8-bit (palette index) replacer.
int UnknownFunction4d1b40(void* bits, unsigned char from, unsigned char to, int width, int height, int stride) {
    unsigned char* row = (unsigned char*)bits;
    for (int y = 0; y < height; y++) {
        unsigned char* pixel = row;
        for (int x = 0; x < width; x++, pixel++) {
            if (*pixel == from)
                *pixel = to;
        }
        row += stride;
    }
    return 1;
}
