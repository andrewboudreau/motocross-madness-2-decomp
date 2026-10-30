// Group0_Pixtrans.cpp -- bring-up bodies for three Pixtrans.cpp pixel
// routines that other reconstructed code calls but nobody had written yet.
// Behavior follows the retail disassembly; byte matching was not attempted.
//
//   Halve555           0x004ce5f0
//   Halve565           0x004cea10
//   Convert555To8Fast  0x004d0d40 (hand-written assembly in retail)
//
// All strides are in pixels, as in Pixtrans.h.
#include "Pixtrans.h"

// The two 16-bit halvers share one shape; only the channel layout differs.
// `filter` selects one of three paths:
//   bit 0: a packed path that adds two horizontally adjacent pixels of two
//          rows with the green channel moved above red/blue. As in retail
//          (and in Halve4444), the inner loop counts `height` pixels, not
//          `width`, and has no zero-count guard.
//   bit 1: a keyed average that skips pixels equal to `key`; a block that
//          is entirely `key` writes `key`.
//   else:  a plain rounded 2x2 average.
// Every path returns 1.

int Halve555(void* destination, void* source, int width, int height, int destinationStride,
             int sourceStride, int filter, unsigned short key)
{
    if (filter & 1) {
        unsigned short* sourceRow = (unsigned short*)source;
        unsigned short* row = (unsigned short*)destination;
        for (int y = 0; y < height; y++) {
            unsigned short* from = sourceRow;
            unsigned short* to = row;
            int count = height; // retail counts with height here
            do {
                unsigned int top = *(unsigned int*)from;
                unsigned int topLow = top & 0xffff, topHigh = top >> 16;
                unsigned int sum = ((topHigh << 15) & 0x1f00000) + ((topLow & 0x3e0) << 15)
                                 + (topHigh & 0x7c1f) + (topLow & 0x7c1f);
                unsigned int bottom = *(unsigned int*)(from + sourceStride);
                unsigned int bottomLow = bottom & 0xffff, bottomHigh = bottom >> 16;
                sum += ((bottomHigh << 15) & 0x1f00000) + ((bottomLow & 0x3e0) << 15)
                     + (bottomHigh & 0x7c1f) + (bottomLow & 0x7c1f);
                sum >>= 2;
                *to = (unsigned short)((sum & 0x7c1f) + ((sum >> 15) & 0x3e0));
                from += 2;
                to++;
            } while (--count);
            sourceRow += sourceStride * 2;
            row += destinationStride;
        }
        return 1;
    }

    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    if (filter & 2) {
        for (int y = 0; y < height; y++) {
            unsigned short* from = sourceRow;
            unsigned short* to = row;
            for (int x = 0; x < width; x++) {
                unsigned short pixels[4];
                pixels[0] = from[0];
                pixels[1] = from[1];
                pixels[2] = from[sourceStride];
                pixels[3] = from[sourceStride + 1];
                int count = 0, red = 0, green = 0, blue = 0;
                for (int i = 0; i < 4; i++) {
                    unsigned int p = pixels[i];
                    if (p == key)
                        continue;
                    red += (p >> 7) & 0xf8;
                    green += (p >> 2) & 0xf8;
                    blue += (p & 0x1f) << 3;
                    count++;
                }
                unsigned short out = key;
                if (count >= 1) {
                    int r = (red + count * 4) / count;
                    int g = (green + count * 4) / count;
                    int b = (blue + count * 4) / count;
                    out = (unsigned short)((((r & 0xfff8) << 5 | (g & 0xfff8)) << 2) | (b >> 3));
                }
                *to++ = out;
                from += 2;
            }
            sourceRow += sourceStride * 2;
            row += destinationStride;
        }
        return 1;
    }

    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++) {
            unsigned int a = from[0], b = from[1], c = from[sourceStride], d = from[sourceStride + 1];
            int red = ((a >> 7) & 0xf8) + ((b >> 7) & 0xf8) + ((c >> 7) & 0xf8) + ((d >> 7) & 0xf8) + 2;
            int green = ((a >> 2) & 0xf8) + ((b >> 2) & 0xf8) + ((c >> 2) & 0xf8) + ((d >> 2) & 0xf8) + 2;
            int blue = ((a & 0x1f) + (b & 0x1f) + (c & 0x1f) + (d & 0x1f)) * 8 + 2;
            *to++ = (unsigned short)(((((red >> 5) << 5) | (green >> 5)) << 5) | (blue >> 5));
            from += 2;
        }
        sourceRow += sourceStride * 2;
        row += destinationStride;
    }
    return 1;
}

int Halve565(void* destination, void* source, int width, int height, int destinationStride,
             int sourceStride, int filter, unsigned short key)
{
    if (filter & 1) {
        unsigned short* sourceRow = (unsigned short*)source;
        unsigned short* row = (unsigned short*)destination;
        for (int y = 0; y < height; y++) {
            unsigned short* from = sourceRow;
            unsigned short* to = row;
            int count = height; // retail counts with height here
            do {
                unsigned int top = *(unsigned int*)from;
                unsigned int topLow = top & 0xffff, topHigh = top >> 16;
                unsigned int sum = ((topHigh << 15) & 0x3f00000) + ((topLow & 0x7e0) << 15)
                                 + (topHigh & 0xf81f) + (topLow & 0xf81f);
                unsigned int bottom = *(unsigned int*)(from + sourceStride);
                unsigned int bottomLow = bottom & 0xffff, bottomHigh = bottom >> 16;
                sum += ((bottomHigh << 15) & 0x3f00000) + ((bottomLow & 0x7e0) << 15)
                     + (bottomHigh & 0xf81f) + (bottomLow & 0xf81f);
                sum >>= 2;
                *to = (unsigned short)((sum & 0xf81f) + ((sum >> 15) & 0x7e0));
                from += 2;
                to++;
            } while (--count);
            sourceRow += sourceStride * 2;
            row += destinationStride;
        }
        return 1;
    }

    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    if (filter & 2) {
        for (int y = 0; y < height; y++) {
            unsigned short* from = sourceRow;
            unsigned short* to = row;
            for (int x = 0; x < width; x++) {
                unsigned short pixels[4];
                pixels[0] = from[0];
                pixels[1] = from[1];
                pixels[2] = from[sourceStride];
                pixels[3] = from[sourceStride + 1];
                int count = 0, red = 0, green = 0, blue = 0;
                for (int i = 0; i < 4; i++) {
                    unsigned int p = pixels[i];
                    if (p == key)
                        continue;
                    red += (p >> 8) & 0xf8;
                    green += (p >> 3) & 0xfc;
                    blue += (p & 0x1f) << 3;
                    count++;
                }
                unsigned short out = key;
                if (count >= 1) {
                    int r = (red + count * 4) / count;
                    int g = (green + count * 2) / count;
                    int b = (blue + count * 4) / count;
                    out = (unsigned short)((((r & 0xfff8) << 5 | (g & 0xfffc)) << 3) | (b >> 3));
                }
                *to++ = out;
                from += 2;
            }
            sourceRow += sourceStride * 2;
            row += destinationStride;
        }
        return 1;
    }

    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++) {
            unsigned int a = from[0], b = from[1], c = from[sourceStride], d = from[sourceStride + 1];
            int red = ((a >> 8) & 0xf8) + ((b >> 8) & 0xf8) + ((c >> 8) & 0xf8) + ((d >> 8) & 0xf8) + 8;
            int green = ((a >> 3) & 0xfc) + ((b >> 3) & 0xfc) + ((c >> 3) & 0xfc) + ((d >> 3) & 0xfc) + 4;
            int blue = ((a & 0x1f) + (b & 0x1f) + (c & 0x1f) + (d & 0x1f)) * 8 + 8;
            *to++ = (unsigned short)(((((red >> 5) << 6) | (green >> 4)) << 5) | (blue >> 5));
            from += 2;
        }
        sourceRow += sourceStride * 2;
        row += destinationStride;
    }
    return 1;
}

// 0x004d0d40: 555 pixels to palette indices through `table` (indexed by the
// low 15 bits of each pixel), eight pixels per step. The source rows are
// contiguous (0x004d0c40 only calls it when the source stride equals the
// width and the width is a multiple of eight); only the destination has a
// stride. best guess: retail's counted loops never test for a zero count
// (width < 8 or height 0 would run away); this version simply does nothing.
void Convert555To8Fast(void* source, void* destination, int destinationStride, int width, int height,
                       unsigned char* table)
{
    unsigned short* from = (unsigned short*)source;
    unsigned char* row = (unsigned char*)destination;
    int groups = width / 8;
    for (int y = 0; y < height; y++) {
        unsigned char* to = row;
        for (int g = 0; g < groups; g++) {
            for (int i = 0; i < 8; i++)
                to[i] = table[from[i] & 0x7fff];
            to += 8;
            from += 8;
        }
        row += destinationStride;
    }
}
