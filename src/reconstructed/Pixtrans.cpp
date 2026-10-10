// Pixtrans.cpp's pixel converters. Names are provisional; see Pixtrans.h.

#include <string.h>

#include "Pixtrans.h"

#include "DebugAlloc.h"
#include "TextureMap.h"

// 0x004d0700: converts 565 to 8888; the key colour becomes opaque magenta.
int Convert565To8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key) {
    unsigned short* sourceRow = (unsigned short*)source;
    UnknownPixel32* row = (UnknownPixel32*)destination;
    unsigned short transparent = Pack565(key);
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        UnknownPixel32* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            if (*from == transparent) {
                to->red = 0xff;
                to->green = 0;
                to->blue = 0xff;
                to->alpha = 0xff;
            } else {
                to->red = (*from >> 8) & 0xf8;
                to->green = (*from >> 3) & 0xfc;
                to->blue = *from << 3;
                to->alpha = 0xff;
            }
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
            if (*from & 0x8000)
                to->alpha = 0xff;
            else
                to->alpha = 0;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d0870: converts 4444 to 8888.
int Convert4444To8888(void* destination, void* source, int width, int height, int destinationStride,
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

// 0x004cde20: halves 24-bit pixels, averaging each 2x2 block.
int Halve24(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    UnknownPixel24* sourceRow = (UnknownPixel24*)source;
    UnknownPixel24* row = (UnknownPixel24*)destination;
    for (int y = 0; y < height; y++) {
        UnknownPixel24* top = sourceRow;
        UnknownPixel24* to = row;
        for (int x = 0; x < width; x++, to++, top += 2) {
            to->red = (top[0].red + top[1].red + top[sourceStride].red + top[sourceStride + 1].red) >> 2;
            to->green = (top[0].green + top[1].green + top[sourceStride].green + top[sourceStride + 1].green) >> 2;
            to->blue = (top[0].blue + top[1].blue + top[sourceStride].blue + top[sourceStride + 1].blue) >> 2;
        }
        sourceRow += sourceStride * 2;
        row += destinationStride;
    }
    return 1;
}

// 0x004ce190: halves 4444 pixels, rounding each channel's 2x2 average. Bit 0
// of `value` (always 0 from the downsamplers) selects a packed path that
// spreads the nibbles to 0x0f0f0f0f and sums them together; as in retail, that
// path counts pixels with `height` and never advances the source in a row.
int Halve4444(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int value) {
    if (value & 1) {
        unsigned short* sourceRow = (unsigned short*)source;
        unsigned short* row = (unsigned short*)destination;
        for (int y = 0; y < height; y++) {
            unsigned short* to = row;
            for (unsigned int x = 0; x != (unsigned int)height; x++) {
                unsigned int a = sourceRow[0], b = sourceRow[1], c = sourceRow[sourceStride],
                             d = sourceRow[sourceStride + 1];
                int sum = ((a & 0xf0f0) << 12 | (a & 0x0f0f)) + ((b & 0xf0f0) << 12 | (b & 0x0f0f))
                        + ((c & 0xf0f0) << 12 | (c & 0x0f0f)) + ((d & 0xf0f0) << 12 | (d & 0x0f0f));
                sum >>= 2;
                *to++ = (unsigned short)((sum & 0x0f0f) | ((sum >> 12) & ~0x0f0f));
            }
            sourceRow += sourceStride * 2;
            row += destinationStride;
        }
        return 1;
    }
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, to++, from += 2) {
            unsigned short a = from[0];
            unsigned short b = from[1];
            unsigned short c = from[sourceStride];
            unsigned short d = from[sourceStride + 1];
            int r = ((a >> 10) & 0x3c) + ((b >> 10) & 0x3c) + ((c >> 10) & 0x3c) + ((d >> 10) & 0x3c) + 2;
            unsigned short out = r >> 4;
            int g = ((a >> 6) & 0x3c) + ((b >> 6) & 0x3c) + ((c >> 6) & 0x3c) + ((d >> 6) & 0x3c) + 2;
            out = out << 4 | g >> 4;
            int bl = ((a >> 2) & 0x3c) + ((b >> 2) & 0x3c) + ((c >> 2) & 0x3c) + ((d >> 2) & 0x3c) + 2;
            out = out << 4 | bl >> 4;
            int al = (((a & 0xf) + (b & 0xf) + (c & 0xf) + (d & 0xf)) * 4 + 2) >> 4;
            *to = out << 4 | al;
        }
        sourceRow += sourceStride * 2;
        row += destinationStride;
    }
    return 1;
}

// 0x004d0900: converts 24-bit to 565, through the ditherer when asked.
int Convert24To565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither) {
    if (dither) {
        DitherConvert(source, 888, width, height, sourceStride, destinationStride, 0, destination,
                              0, 0, 0);
    } else {
        UnknownPixel24* sourceRow = (UnknownPixel24*)source;
        unsigned short* row = (unsigned short*)destination;
        for (int y = 0; y < height; y++) {
            UnknownPixel24* from = sourceRow;
            unsigned short* to = row;
            for (int x = 0; x < width; x++, to++, from++)
                *to = (from->red >> 3) << 11 | (from->green >> 2) << 5 | from->blue >> 3;
            sourceRow += sourceStride;
            row += destinationStride;
        }
    }
    return 1;
}

// 0x004d09d0: converts 24-bit to 555, through the ditherer when asked.
int Convert24To555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither) {
    if (dither) {
        DitherConvert(source, 888, width, height, sourceStride, destinationStride, 0, 0,
                              destination, 0, 0);
    } else {
        UnknownPixel24* sourceRow = (UnknownPixel24*)source;
        unsigned short* row = (unsigned short*)destination;
        for (int y = 0; y < height; y++) {
            UnknownPixel24* from = sourceRow;
            unsigned short* to = row;
            for (int x = 0; x < width; x++, to++, from++)
                *to = (from->red >> 3) << 10 | (from->green >> 3) << 5 | from->blue >> 3;
            sourceRow += sourceStride;
            row += destinationStride;
        }
    }
    return 1;
}

// 0x004d0e40: converts 555 to 8888; the key colour becomes opaque magenta.
int Convert555To8888(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert555To24(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert555To565(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert565To555(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert8To24(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert8888To4444(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert8888To1555(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert24To1555(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert4444To555(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert4444To1555(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert4444To565(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert555To1555(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert1555To555(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert565To1555(void* destination, void* source, int width, int height, int destinationStride,
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
int Convert1555To565(void* destination, void* source, int width, int height, int destinationStride,
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
                          int sourceStride, int levels, int format, UnknownTexturePalette* palette, int filter) {
    switch (format) {
    case 8:
        return Downsample8(destination, source, width, height, destinationStride, sourceStride, levels,
                                     palette);
    case 555:
        return Downsample555(destination, source, width, height, destinationStride, sourceStride, levels,
                                     filter);
    case 565:
        return Downsample565(destination, source, width, height, destinationStride, sourceStride, levels,
                                     filter);
    case 888:
        return Downsample24(destination, source, width, height, destinationStride, sourceStride, levels);
    case 1555:
        return Downsample1555(destination, source, width, height, destinationStride, sourceStride, levels);
    case 4444:
        return Downsample4444(destination, source, width, height, destinationStride, sourceStride, levels);
    case 8888:
        return Downsample8888(destination, source, width, height, destinationStride, sourceStride, levels);
    }
    return 0;
}

// 0x004d1970: replaces 32-bit `from` pixels with `to`; pixels already equal
// to `to` get their blue byte nudged (255 down, otherwise up) first.
int ReplaceColor32(void* bits, UnknownPixel32 from, UnknownPixel32 to, int width, int height, int stride) {
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
int ReplaceColor24(void* bits, UnknownPixel24 from, UnknownPixel24 to, int width, int height, int stride) {
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
int ReplaceColor16(void* bits, unsigned short from, unsigned short to, int width, int height, int stride) {
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
int ReplaceColor8(void* bits, unsigned char from, unsigned char to, int width, int height, int stride) {
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

// 0x004d0aa0: converts 24-bit to palette indices through the 555 table.
int Convert24To8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, UnknownTexturePalette* palette) {
    if (dither && palette) {
        DitherConvert(source, 888, width, height, sourceStride, destinationStride, 0, 0, 0,
                              destination, palette);
    } else {
        UnknownPixel24* sourceRow = (UnknownPixel24*)source;
        unsigned char* row = (unsigned char*)destination;
        unsigned char* indices = palette->UnknownFunction4de280();
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
int Convert565To8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, UnknownTexturePalette* palette) {
    if (dither && palette) {
        DitherConvert(source, 565, width, height, sourceStride, destinationStride, 0, 0, 0,
                              destination, palette);
    } else {
        unsigned short* sourceRow = (unsigned short*)source;
        unsigned char* row = (unsigned char*)destination;
        unsigned char* indices = palette->UnknownFunction4de290();
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

// 0x004cf980: spreads 24-bit pixels into 16.16 channel triples (the
// ditherer's row reader).
void ReadRow24(UnknownPixel24* source, int* channels, int count) {
    do {
        channels[0] = source->red << 16;
        channels[1] = source->green << 16;
        channels[2] = source->blue << 16;
        channels += 3;
        source++;
    } while (--count);
}

// 0x004cf9c0: the 565 row reader.
void ReadRow565(unsigned short* source, int* channels, int count) {
    do {
        channels[0] = (*source & 0xf800) << 8;
        channels[1] = (*source & 0x7e0) << 13;
        channels[2] = (*(unsigned char*)source & 0x1f) << 19;
        channels += 3;
        source++;
    } while (--count);
}

// 0x004cfa10: the 555 row reader.
void ReadRow555(unsigned short* source, int* channels, int count) {
    do {
        channels[0] = (*source & 0x7c00) << 9;
        channels[1] = (*source & 0x3e0) << 14;
        channels[2] = (*source & 0x1f) << 19;
        channels += 3;
        source++;
    } while (--count);
}

// 0x004cfa60: converts 565 to 24-bit.
int Convert565To24(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    unsigned short* sourceRow = (unsigned short*)source;
    UnknownPixel24* row = (UnknownPixel24*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        UnknownPixel24* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            to->red = (*from >> 8) & 0xf8;
            to->green = (*from >> 3) & 0xfc;
            to->blue = *from << 3;
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004cfaf0: shrinks 24-bit `source` by `levels` halvings into the width x
// height `destination` (0: a plain copy). Intermediate levels go through a
// buffer of the first level's size.
int Downsample24(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels) {
    if (levels == 0) {
        for (int y = 0; y < height; y++)
            memcpy((UnknownPixel24*)destination + y * destinationStride, (UnknownPixel24*)source + y * sourceStride,
                   width * 3);
        return 1;
    }
    if (levels == 1) {
        Halve24(destination, source, width, height, destinationStride, sourceStride);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth * 3, __FILE__, 1329);
    Halve24(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        Halve24(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2);
    }
    Halve24(destination, buffer, width, height, destinationStride, levelWidth);
    DebugFree(buffer, __FILE__, 1350);
    return 1;
}

// 0x004cfc40: shrinks 8888 `source` by `levels` halvings (0: a plain
// copy), like 0x004cfaf0.
int Downsample8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels) {
    if (levels == 0) {
        for (int y = 0; y < height; y++)
            memcpy((UnknownPixel32*)destination + y * destinationStride, (UnknownPixel32*)source + y * sourceStride,
                   width * 4);
        return 1;
    }
    if (levels == 1) {
        Halve8888(destination, source, width, height, destinationStride, sourceStride);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth * 4, __FILE__, 1390);
    if (!buffer)
        return 0;
    Halve8888(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        Halve8888(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2);
    }
    Halve8888(destination, buffer, width, height, destinationStride, levelWidth);
    DebugFree(buffer, __FILE__, 1412);
    return 1;
}

// 0x004d0020: shrinks 4444 `source` by `levels` halvings through 0x004ce190.
int Downsample4444(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels) {
    if (levels == 0) {
        for (int y = 0; y < height; y++)
            memcpy((unsigned short*)destination + y * destinationStride, (unsigned short*)source + y * sourceStride,
                   width * 2);
        return 1;
    }
    if (levels == 1) {
        Halve4444(destination, source, width, height, destinationStride, sourceStride, 0);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth * 2, __FILE__, 1572);
    Halve4444(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride, 0);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        Halve4444(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2, 0);
    }
    Halve4444(destination, buffer, width, height, destinationStride, levelWidth, 0);
    DebugFree(buffer, __FILE__, 1593);
    return 1;
}

// 0x004d0170: shrinks 1555 `source` by `levels` halvings through 0x004ce420.
int Downsample1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels) {
    if (levels == 0) {
        for (int y = 0; y < height; y++)
            memcpy((unsigned short*)destination + y * destinationStride, (unsigned short*)source + y * sourceStride,
                   width * 2);
        return 1;
    }
    if (levels == 1) {
        Halve1555(destination, source, width, height, destinationStride, sourceStride);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth * 2, __FILE__, 1632);
    Halve1555(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        Halve1555(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2);
    }
    Halve1555(destination, buffer, width, height, destinationStride, levelWidth);
    DebugFree(buffer, __FILE__, 1653);
    return 1;
}

// 0x004d02c0: shrinks 565 `source` by `levels` halvings through 0x004cea10
// (magenta 0xf81f is the key).
int Downsample565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, int filter) {
    if (levels == 0) {
        for (int y = 0; y < height; y++)
            memcpy((unsigned short*)destination + y * destinationStride, (unsigned short*)source + y * sourceStride,
                   width * 2);
        return 1;
    }
    if (levels == 1) {
        Halve565(destination, source, width, height, destinationStride, sourceStride, filter, 0xf81f);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth * 2, __FILE__, 1693);
    Halve565(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride, filter, 0xf81f);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        Halve565(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2, filter, 0xf81f);
    }
    Halve565(destination, buffer, width, height, destinationStride, levelWidth, filter, 0xf81f);
    DebugFree(buffer, __FILE__, 1714);
    return 1;
}

// 0x004d0440: shrinks 555 `source` by `levels` halvings through 0x004ce5f0
// (magenta 0x7c1f is the key).
int Downsample555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, int filter) {
    if (levels == 0) {
        for (int y = 0; y < height; y++)
            memcpy((unsigned short*)destination + y * destinationStride, (unsigned short*)source + y * sourceStride,
                   width * 2);
        return 1;
    }
    if (levels == 1) {
        Halve555(destination, source, width, height, destinationStride, sourceStride, filter, 0x7c1f);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth * 2, __FILE__, 1754);
    Halve555(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride, filter, 0x7c1f);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        Halve555(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2, filter, 0x7c1f);
    }
    Halve555(destination, buffer, width, height, destinationStride, levelWidth, filter, 0x7c1f);
    DebugFree(buffer, __FILE__, 1775);
    return 1;
}

// 0x004d05c0: shrinks palette-index `source` by `levels` halvings through
// 0x004cee30.
int Downsample8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, UnknownTexturePalette* palette) {
    if (levels == 0) {
        for (int y = 0; y < height; y++)
            memcpy((unsigned char*)destination + y * destinationStride, (unsigned char*)source + y * sourceStride,
                   width);
        return 1;
    }
    if (levels == 1) {
        Halve8(destination, source, width, height, destinationStride, sourceStride, palette);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth, __FILE__, 1816);
    Halve8(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride, palette);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        Halve8(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2, palette);
    }
    Halve8(destination, buffer, width, height, destinationStride, levelWidth, palette);
    DebugFree(buffer, __FILE__, 1840);
    return 1;
}

// 0x004cfda0: converts 24-bit to 8888; the 0xRRGGBB `key` becomes
// transparent. Rows are addressed from the row index.
void Convert24To8888(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, unsigned int key) {
    int keyColor[3];
    keyColor[0] = (key >> 16) & 0xff;
    keyColor[1] = (key >> 8) & 0xff;
    keyColor[2] = key & 0xff;
    for (int y = 0; y < height; y++) {
        unsigned char* from = (unsigned char*)source + y * sourceStride * 3;
        unsigned char* to = (unsigned char*)destination + y * destinationStride * 4;
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
    }
}

// 0x004cfe70: converts 24-bit to 8888 (0x004cfda0, keyed) at `levels`
// times the size, then halves it down through 0x004cdf10.
int UnknownFunction4cfe70(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, unsigned int key) {
    if (levels == 0) {
        Convert24To8888(destination, source, width, height, destinationStride, sourceStride, key);
        return 1;
    }
    int fullWidth = width << levels;
    int fullHeight = height << levels;
    void* full = DebugMalloc(fullWidth * fullHeight * 4, __FILE__, 1484);
    if (!full)
        return 0;
    Convert24To8888(full, source, fullWidth, fullHeight, sourceStride, sourceStride, key);
    if (levels == 1) {
        Halve8888(destination, full, width, height, destinationStride, sourceStride);
    } else {
        int levelWidth = width << (levels - 1);
        int levelHeight = height << (levels - 1);
        void* buffer = DebugMalloc(levelWidth * levelHeight * 4, __FILE__, 1505);
        if (!buffer) {
            DebugFree(full, __FILE__, 1534);
            return 0;
        }
        Halve8888(buffer, full, levelWidth, levelHeight, levelWidth, sourceStride);
        for (int i = 2; i < levels; i++) {
            levelWidth /= 2;
            levelHeight /= 2;
            Halve8888(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2);
        }
        Halve8888(destination, buffer, width, height, destinationStride, levelWidth);
        DebugFree(buffer, __FILE__, 1527);
    }
    DebugFree(full, __FILE__, 1529);
    return 1;
}

// 0x004d0c40: converts 555 to palette indices: dithered when asked, through
// the assembly 0x004d0d40 for unpadded rows of a multiple of eight pixels,
// otherwise through the 555 table.
int Convert555To8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, UnknownTexturePalette* palette) {
    if (dither && palette) {
        DitherConvert(source, 555, width, height, sourceStride, destinationStride, 0, 0, 0,
                              destination, palette);
    } else if (sourceStride == width && sourceStride % 8 == 0) {
        Convert555To8Fast(source, destination, destinationStride, width, height,
                              palette->UnknownFunction4de280());
    } else {
        unsigned short* sourceRow = (unsigned short*)source;
        unsigned char* row = (unsigned char*)destination;
        unsigned char* indices = palette->UnknownFunction4de280();
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

// 0x004d1d20: converts `source` (pixel format `sourceFormat`) into
// `destination` (`format`) with the matching converter; same-format copies
// go through the downsamplers with no halving. 0 for unsupported pairs.
int UnknownFunction4d1d20(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, int format, int sourceFormat, int dither,
                           UnknownTexturePalette* palette, int alphaThreshold, unsigned int key) {
    switch (sourceFormat) {
    case 565:
        switch (format) {
        case 565:
            return Downsample565(destination, source, width, height, destinationStride, sourceStride, 0, 2);
        case 555:
            return Convert565To555(destination, source, width, height, destinationStride, sourceStride);
        case 8:
            return Convert565To8(destination, source, width, height, destinationStride, sourceStride, dither,
                                         palette);
        case 888:
            return Convert565To24(destination, source, width, height, destinationStride, sourceStride);
        case 8888:
            return Convert565To8888(destination, source, width, height, destinationStride, sourceStride, key);
        case 1555:
            return Convert565To1555(destination, source, width, height, destinationStride, sourceStride, key);
        }
        break;
    case 555:
        switch (format) {
        case 565:
            return Convert555To565(destination, source, width, height, destinationStride, sourceStride);
        case 555:
            return Downsample555(destination, source, width, height, destinationStride, sourceStride, 0, 2);
        case 8:
            return Convert555To8(destination, source, width, height, destinationStride, sourceStride, dither,
                                         palette);
        case 888:
            return Convert555To24(destination, source, width, height, destinationStride, sourceStride);
        case 8888:
            return Convert555To8888(destination, source, width, height, destinationStride, sourceStride, key);
        case 1555:
            return Convert555To1555(destination, source, width, height, destinationStride, sourceStride, key);
        }
        break;
    case 8:
        switch (format) {
        case 555:
            return UnknownFunction4d11c0(destination, source, width, height, destinationStride, sourceStride, palette);
        case 8:
            return Downsample8(destination, source, width, height, destinationStride, sourceStride, 0,
                                         palette);
        case 565:
            return UnknownFunction4d1150(destination, source, width, height, destinationStride, sourceStride, palette);
        case 888:
            return Convert8To24(destination, source, width, height, destinationStride, sourceStride, palette);
        }
        break;
    case 888:
        switch (format) {
        case 565:
            return Convert24To565(destination, source, width, height, destinationStride, sourceStride, dither);
        case 555:
            return Convert24To555(destination, source, width, height, destinationStride, sourceStride, dither);
        case 8:
            return Convert24To8(destination, source, width, height, destinationStride, sourceStride, dither,
                                         palette);
        case 888:
            if (dither)
                return DitherConvert(source, 888, width, height, sourceStride, destinationStride, destination,
                                             0, 0, 0, 0);
            else
                return Downsample24(destination, source, width, height, destinationStride, sourceStride, 0);
        case 8888:
            return UnknownFunction4cfe70(destination, source, width, height, destinationStride, sourceStride, 0, key);
        case 1555:
            return Convert24To1555(destination, source, width, height, destinationStride, sourceStride, key);
        }
        break;
    case 8888:
        switch (format) {
        case 8888:
            return Downsample8888(destination, source, width, height, destinationStride, sourceStride, 0);
        case 4444:
            return Convert8888To4444(destination, source, width, height, destinationStride, sourceStride);
        case 1555:
            return Convert8888To1555(destination, source, width, height, destinationStride, sourceStride,
                                         alphaThreshold);
        }
        break;
    case 4444:
        switch (format) {
        case 565:
            return Convert4444To565(destination, source, width, height, destinationStride, sourceStride,
                                         alphaThreshold, key);
        case 555:
            return Convert4444To555(destination, source, width, height, destinationStride, sourceStride,
                                         alphaThreshold, key);
        case 1555:
            return Convert4444To1555(destination, source, width, height, destinationStride, sourceStride,
                                         alphaThreshold);
        case 8888:
            return Convert4444To8888(destination, source, width, height, destinationStride, sourceStride);
        case 4444:
            return Downsample4444(destination, source, width, height, destinationStride, sourceStride, 0);
        }
        break;
    case 1555:
        switch (format) {
        case 565:
            return Convert1555To565(destination, source, width, height, destinationStride, sourceStride, key);
        case 555:
            return Convert1555To555(destination, source, width, height, destinationStride, sourceStride, key);
        case 1555:
            return Downsample1555(destination, source, width, height, destinationStride, sourceStride, 0);
        case 8888:
            return Convert1555To8888(destination, source, width, height, destinationStride, sourceStride);
        }
        break;
    }
    return 0;
}

#define UNKNOWN_ARGB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

// 0x004d24d0: the average colour of a width x height bitmap of `format`
// (rows of `width` pixels; `stride` is unused), as 0xAARRGGBB with opaque
// alpha for formats without one; 0 for other formats. The palette and 24-bit
// cases return the same expression each: VC6 cross-jumps the 24-bit copy
// into the palette case's, which is retail's layout; a shared return after
// the switch lands after the 24-bit loop instead.
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
        return UNKNOWN_ARGB(0xff, red / count, green / count, blue / count);
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
        return UNKNOWN_ARGB(0xff, red / count, green / count, blue / count);
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
}
