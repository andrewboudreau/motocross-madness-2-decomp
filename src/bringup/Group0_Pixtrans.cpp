// Group0_Pixtrans.cpp -- bring-up bodies for three Pixtrans.cpp pixel
// routines that other reconstructed code calls but nobody had written yet.
// Behavior follows the retail disassembly; byte matching was not attempted.
//
//   Halve555           0x004ce5f0
//   Halve565           0x004cea10
//   Convert555To8Fast  0x004d0d40 (hand-written assembly in retail)
//
// All strides are in pixels, as in Pixtrans.h.
// Re-checked against the retail disassembly on 2026-11-11: masks, shifts, rounding
// constants, loop bounds and the source/destination steps of every path agree.
// Audited again for the linked game (same day): no behavioural difference in any of
// the three paths of either halver. The keyed path is now written out per pixel as
// retail has it (the lower pair is read before the first key test, then the upper
// pair in one reused `pixel`), which moved the linked score of both halvers from
// 0.58 to 0.71. The rest is code shape this C cannot give: the packed (filter & 1) inner
// loop is inline assembly in retail (its accumulator lives in sourceStride's
// argument slot, `mov [ebp+0x1c], 0; add [ebp+0x1c], ebx`, and it counts down with
// `dec [ebp+0x24]`), which also gives the whole body an ebp frame, so every other
// path allocates registers without ebp. Convert555To8Fast is inline assembly too.
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
                int count = 0, red = 0, green = 0, blue = 0;
                unsigned short lowerLeft = from[sourceStride];
                unsigned short lowerRight = from[sourceStride + 1];
                unsigned short pixel = from[0];
                if (pixel != key) {
                    red += (pixel >> 7) & 0xf8;
                    green += (pixel >> 2) & 0xf8;
                    blue += (pixel & 0x1f) << 3;
                    count++;
                }
                pixel = from[1];
                if (pixel != key) {
                    red += (pixel >> 7) & 0xf8;
                    green += (pixel >> 2) & 0xf8;
                    blue += (pixel & 0x1f) << 3;
                    count++;
                }
                if (lowerLeft != key) {
                    red += (lowerLeft >> 7) & 0xf8;
                    green += (lowerLeft >> 2) & 0xf8;
                    blue += (lowerLeft & 0x1f) << 3;
                    count++;
                }
                if (lowerRight != key) {
                    red += (lowerRight >> 7) & 0xf8;
                    green += (lowerRight >> 2) & 0xf8;
                    blue += (lowerRight & 0x1f) << 3;
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
                int count = 0, red = 0, green = 0, blue = 0;
                unsigned short lowerLeft = from[sourceStride];
                unsigned short lowerRight = from[sourceStride + 1];
                unsigned short pixel = from[0];
                if (pixel != key) {
                    red += (pixel >> 8) & 0xf8;
                    green += (pixel >> 3) & 0xfc;
                    blue += (pixel & 0x1f) << 3;
                    count++;
                }
                pixel = from[1];
                if (pixel != key) {
                    red += (pixel >> 8) & 0xf8;
                    green += (pixel >> 3) & 0xfc;
                    blue += (pixel & 0x1f) << 3;
                    count++;
                }
                if (lowerLeft != key) {
                    red += (lowerLeft >> 8) & 0xf8;
                    green += (lowerLeft >> 3) & 0xfc;
                    blue += (lowerLeft & 0x1f) << 3;
                    count++;
                }
                if (lowerRight != key) {
                    red += (lowerRight >> 8) & 0xf8;
                    green += (lowerRight >> 3) & 0xfc;
                    blue += (lowerRight & 0x1f) << 3;
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
// Audited for the linked game 2026-11-11: no behavioural difference. Retail
// is unrolled inline assembly that reads a dword (two pixels) at a time,
// builds each four-index dword in ebx (bh/bl take the high/low pixel's index,
// `shl ebx, 16` moves the pair up) and stores two dwords a step, so the bytes
// land in source order as here. It preloads the next step's third and fourth
// pixels, so its last step reads bytes 4..7 past the end of the source; that
// read is not reproduced.
// The size gap (51 built vs 83 retail instructions) is that assembly. This
// version reads a dword (two pixels) at a time as retail does, which took the
// linked masked score from 0.369 to 0.418. Packing four indices per dword
// store, with or without retail's count-down do-while loops, was tried and
// scored lower (0.39). Fully unrolling the eight pixels scored 0.34.
void Convert555To8Fast(void* source, void* destination, int destinationStride, int width, int height,
                       unsigned char* table)
{
    const unsigned int* from = (const unsigned int*)source;
    unsigned char* row = (unsigned char*)destination;
    int groups = width / 8;
    for (int y = 0; y < height; y++) {
        unsigned char* to = row;
        for (int g = 0; g < groups; g++) {
            for (int i = 0; i < 4; i++) {
                unsigned int pair = from[i];
                to[0] = table[pair & 0x7fff];
                to[1] = table[(pair >> 16) & 0x7fff];
                to += 2;
            }
            from += 4;
        }
        row += destinationStride;
    }
}
