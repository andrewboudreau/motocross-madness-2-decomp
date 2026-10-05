// Pixtrans.cpp's pixel converters. Names are provisional; see Pixtrans.h.

#include "Pixtrans.h"

#include "TextureMap.h"

// 0x004d0870: converts 4444 to 8888.
int UnknownFunction4d0870(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    unsigned short* sourceRow = (unsigned short*)source;
    UnknownPixel32* row = (UnknownPixel32*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        UnknownPixel32* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            to->alpha = (*from >> 8) & 0xf0;
            to->red = (*from >> 4) & 0xf0;
            to->green = *from & 0xf0;
            to->blue = *from << 4;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d0e40: converts 555 to 8888; the key colour becomes opaque magenta.
int UnknownFunction4d0e40(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key) {
    unsigned short* sourceRow = (unsigned short*)source;
    UnknownPixel32* row = (UnknownPixel32*)destination;
    unsigned short transparent = Pack555(key);
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        UnknownPixel32* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            if (*from == transparent) {
                to->red = 0xff;
                to->green = 0;
                to->blue = 0xff;
            } else {
                to->red = (*from >> 7) & 0xf8;
                to->green = (*from >> 2) & 0xf8;
                to->blue = *from << 3;
            }
            to->alpha = 0xff;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d0f10: converts 555 to 24-bit; 555 magenta stays exactly magenta.
int UnknownFunction4d0f10(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    unsigned short* sourceRow = (unsigned short*)source;
    UnknownPixel24* row = (UnknownPixel24*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        UnknownPixel24* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            if (*from == 0x7c1f) {
                to->red = 0xff;
                to->green = 0;
                to->blue = 0xff;
            } else {
                to->red = (*from >> 7) & 0xf8;
                to->green = (*from >> 2) & 0xf8;
                to->blue = *from << 3;
            }
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d0fb0: converts 555 to 565.
int UnknownFunction4d0fb0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            *to = (*from & 0xffe0) << 1 | *from & 0x1f;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1030: converts 565 pixels to 555.
int UnknownFunction4d1030(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, to++, from++)
            *to = (*from >> 1) & 0x7fe0 | *from & 0x1f;
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d10b0: converts palette indices to 24-bit RGB.
int UnknownFunction4d10b0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette) {
    unsigned char* sourceRow = (unsigned char*)source;
    UnknownPixel24* row = (UnknownPixel24*)destination;
    for (int y = 0; y < height; y++) {
        unsigned char* from = sourceRow;
        UnknownPixel24* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            to->red = palette->field_0x010[*from][0];
            to->green = palette->field_0x010[*from][1];
            to->blue = palette->field_0x010[*from][2];
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1150: converts palette indices through the +0x510 table.
int UnknownFunction4d1150(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette) {
    unsigned char* sourceRow = (unsigned char*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned char* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++)
            *to++ = palette->field_0x510[*from++];
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d11c0: converts palette indices through the +0x310 table.
int UnknownFunction4d11c0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette) {
    unsigned char* sourceRow = (unsigned char*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned char* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++)
            *to++ = palette->field_0x310[*from++];
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1230: converts 8888 to 4444.
int UnknownFunction4d1230(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    UnknownPixel32* sourceRow = (UnknownPixel32*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* to = row;
        UnknownPixel32* from = sourceRow;
        for (int x = 0; x < width; x++, from++, to++) {
            *to = (from->alpha >> 4) << 12 | (from->red >> 4) << 8 | (from->green >> 4) << 4 | from->blue >> 4;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d12d0: converts 8888 to 1555, opaque where alpha reaches the threshold.
int UnknownFunction4d12d0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold) {
    UnknownPixel32* sourceRow = (UnknownPixel32*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        UnknownPixel32* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            unsigned short pixel = (from->red >> 3) << 10 | (from->green >> 3) << 5 | from->blue >> 3;
            *to = pixel;
            if (from->alpha >= alphaThreshold)
                *to = pixel | 0x8000;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1370: converts 24-bit to 1555; the key colour becomes transparent 0.
int UnknownFunction4d1370(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int key) {
    UnknownPixel24* sourceRow = (UnknownPixel24*)source;
    unsigned short* row = (unsigned short*)destination;
    unsigned char keyRed = key >> 16;
    unsigned char keyGreen = key >> 8;
    for (int y = 0; y < height; y++) {
        unsigned short* to = row;
        UnknownPixel24* from = sourceRow;
        for (int x = 0; x < width; x++, to++, from++) {
            if (from->red == keyRed && from->green == keyGreen && from->blue == (unsigned char)key)
                *to = 0;
            else
                *to = 0x8000 | (from->red >> 3) << 10 | (from->green >> 3) << 5 | from->blue >> 3;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1440: converts 4444 to 555; pixels below the alpha threshold become `color`.
int UnknownFunction4d1440(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold, unsigned int color) {
    alphaThreshold = (alphaThreshold >> 4) << 12;
    unsigned short background = Pack555(color);
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, from++) {
            if ((*from & 0xf000) < alphaThreshold)
                *to++ = background;
            else
                *to++ = (*from & 0xf00) << 3 | (*from & 0xf0) << 2 | (*from & 0xf) << 1;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1530: converts 4444 to 1555; pixels below the alpha threshold become 0.
int UnknownFunction4d1530(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold) {
    alphaThreshold = (alphaThreshold >> 4) << 12;
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, from++) {
            if ((*from & 0xf000) < alphaThreshold)
                *to++ = 0;
            else
                *to++ = 0x8000 | (*from & 0xf00) << 3 | (*from & 0xf0) << 2 | (*from & 0xf) << 1;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d15f0: converts 4444 to 565; pixels below the alpha threshold become `color`.
int UnknownFunction4d15f0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold, unsigned int color) {
    alphaThreshold = (alphaThreshold >> 4) << 12;
    unsigned short background = Pack565(color);
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, from++) {
            if ((*from & 0xf000) < alphaThreshold)
                *to++ = background;
            else
                *to++ = (*from & 0xf00) << 4 | (*from & 0xf0) << 3 | (*from & 0xf) << 1;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d16e0: converts 555 to 1555, transparent where the pixel is the key.
int UnknownFunction4d16e0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key) {
    unsigned short transparent = Pack555(key);
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, from++, to++) {
            *to = *from;
            if (*from == transparent)
                *to &= 0x7fff;
            else
                *to |= 0x8000;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1780: converts 1555 to 555: transparent pixels become the key (opaque ones only lose
// the alpha bit of what is already in `destination`, as in retail).
int UnknownFunction4d1780(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key) {
    unsigned short transparent = Pack555(key);
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, from++) {
            if (*from & 0x8000)
                *to++ &= 0x7fff;
            else
                *to++ = transparent;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1810: converts 565 to 1555, transparent where the pixel is the key.
int UnknownFunction4d1810(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key) {
    unsigned short transparent = Pack565(key);
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, from++, to++) {
            unsigned short pixel = (*from >> 1) & 0x7fe0 | *from & 0x1f;
            *to = pixel;
            if (*from != transparent)
                *to = pixel | 0x8000;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d18c0: converts 1555 to 565; transparent pixels become the key.
int UnknownFunction4d18c0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key) {
    unsigned short transparent = Pack565(key);
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, from++) {
            if (*from & 0x8000)
                *to++ = (*from & 0xffe0) << 1 | *from & 0x1f;
            else
                *to++ = transparent;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1b90: hands the downsampling to the helper for `format`.
int UnknownFunction4d1b90(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a, int format, UnknownTexturePalette* palette, int filter) {
    switch (format) {
    case 8:
        return UnknownFunction4d05c0(destination, source, width, height, destinationStride, sourceStride, a,
                                     palette);
    case 0x22b:
        return UnknownFunction4d0440(destination, source, width, height, destinationStride, sourceStride, a,
                                     filter);
    case 0x235:
        return UnknownFunction4d02c0(destination, source, width, height, destinationStride, sourceStride, a,
                                     filter);
    case 0x378:
        return UnknownFunction4cfaf0(destination, source, width, height, destinationStride, sourceStride, a);
    case 0x613:
        return UnknownFunction4d0170(destination, source, width, height, destinationStride, sourceStride, a);
    case 0x115c:
        return UnknownFunction4d0020(destination, source, width, height, destinationStride, sourceStride, a);
    case 0x22b8:
        return UnknownFunction4cfc40(destination, source, width, height, destinationStride, sourceStride, a);
    }
    return 0;
}

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
