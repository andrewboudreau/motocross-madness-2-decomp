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
// Halve1555 (0x004ce420, 456 bytes; candidate 448): the 1555
// halver behind 0x004d0170. The arithmetic, the >= 2 test and the packing
// match. Retail loads the two lower-row pixels before the first test and
// keeps them in frame slots (a 0x10-byte frame); VC6 here loads each where
// it is first tested (0x8 frame). Locals in every declaration order, a
// lower-row pointer, function-scope pixels, a 1555 bitfield struct and
// direct indexing do not keep them.
//
// Halve8 (0x004cee30, 1133 bytes; candidate 1093): the palette-index
// halver. The search for the magenta entry, both block loops, the key test
// and the >= 2 rule match, and nested ifs reproduce retail's single
// `return 0` block. Retail pushes ebp only after the three checks,
// holds 0xff in bl for the search and keeps the found index in the dead
// `source` slot with both row pointers in locals. VC6 here pushes ebp in
// the prologue and homes `sourceRow` in `source`'s slot, which shifts every
// slot. `key` as the loop variable (with or without `== 256` -> -1) and a
// combined `key < 256 && key != -1` test are worse. The fast loop's red/blue/green
// sum order also differs (retail sums blue before green).
//
// UnknownFunction4d24d0 (0x004d24d0, 987 bytes): a bitmap's average colour
// (0xAARRGGBB) by format. Every case body matches; the shared palette and
// 24-bit return lands after the 24-bit loop here but after the palette
// loop in retail. Explicit identical returns move it there, but then VC6
// also merges the 565 and 555 tails; a goto or reordered cases do not
// change it.

// DitherConvert (0x004cf2a0, 1759 bytes; candidate 1735, 242 match): the
// Floyd-Steinberg ditherer behind the dithered converters. Two 16.16 error
// rows (grown on demand, 0x00689a6c/0x00689a70/0x00689a74), each pixel
// rounded to the nearest 5-bit level per channel and, with a palette, to
// the palette entry when it is exactly that colour; errors spread 7/16
// right, 5/16 below, 3/16 below left and the remainder below right. Draft:
// the flow, calls and arithmetic follow retail, but VC6 here assigns the
// format, the pixel size and the palette/width to ebx/esi/edi where retail
// uses edi/ebx/esi, and the frame is 0x50 against retail's 0x54.
//
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Pixtrans.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Tgafile.h"

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

// 0x004cee30: halves palette indices. Each 2x2 block's RGB entries are
// averaged and mapped back through the palette's 555-to-index table. If the
// palette holds magenta (255, 0, 255) that index is the key: key pixels are
// left out of the average and a block with fewer than two others stays the key.
int Halve8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette) {
    unsigned char* sourceRow = (unsigned char*)source;
    unsigned char* row = (unsigned char*)destination;
    if (palette) {
        unsigned char* indices = palette->UnknownFunction4de280();
        if (indices) {
            UnknownPixel24* entries = (UnknownPixel24*)palette->UnknownFunction4de270();
            if (entries) {
                int key = -1;
                for (int i = 0; i < 256; i++) {
                    if (entries[i].red == 0xff && entries[i].green == 0 && entries[i].blue == 0xff) {
                        key = i;
                        break;
                    }
                }
                if (key != -1) {
                    for (int y = 0; y < height; y++) {
                        unsigned char* from = sourceRow;
                        unsigned char* to = row;
                        for (int x = 0; x < width; x++, from += 2) {
                            int ia = from[0];
                            UnknownPixel24 a = entries[ia];
                            int ib = from[1];
                            UnknownPixel24 b = entries[ib];
                            int ic = from[sourceStride];
                            UnknownPixel24 c = entries[ic];
                            int id = from[sourceStride + 1];
                            UnknownPixel24 d = entries[id];
                            int count = 0, red = 0, green = 0, blue = 0;
                            if (ia != key) {
                                red = a.red;
                                green = a.green;
                                blue = a.blue;
                                count = 1;
                            }
                            if (ib != key) {
                                red += b.red;
                                green += b.green;
                                blue += b.blue;
                                count++;
                            }
                            if (ic != key) {
                                red += c.red;
                                green += c.green;
                                blue += c.blue;
                                count++;
                            }
                            if (id != key) {
                                red += d.red;
                                green += d.green;
                                blue += d.blue;
                                count++;
                            }
                            if (count >= 2) {
                                red = (red + count * 4) / count;
                                green = (green + count * 4) / count;
                                blue = (blue + count * 4) / count;
                                *to++ = indices[(red >> 3) << 10 | (green >> 3) << 5 | blue >> 3];
                            } else {
                                *to++ = (unsigned char)key;
                            }
                        }
                        sourceRow += sourceStride * 2;
                        row += destinationStride;
                    }
                    return 1;
                }
                for (int y = 0; y < height; y++) {
                    unsigned char* from = sourceRow;
                    unsigned char* to = row;
                    for (int x = 0; x < width; x++, from += 2) {
                        UnknownPixel24 a = entries[from[0]];
                        UnknownPixel24 b = entries[from[1]];
                        UnknownPixel24 c = entries[from[sourceStride]];
                        UnknownPixel24 d = entries[from[sourceStride + 1]];
                        *to++ = indices[((a.red + b.red + c.red + d.red + 16) >> 5) << 10
                                        | ((a.green + b.green + c.green + d.green + 16) >> 5) << 5
                                        | (a.blue + b.blue + c.blue + d.blue + 16) >> 5];
                    }
                    sourceRow += sourceStride * 2;
                    row += destinationStride;
                }
                return 1;
            }
        }
    }
    return 0;
}

// The ditherer's two error rows (16.16 channel triples), grown on demand.
int g_ditherCapacity;               // 0x00689a6c
int* g_ditherRows[2];               // 0x00689a70, 0x00689a74

// A 16.16 channel rounded to the nearest of the 32 5-bit levels.
static inline int QuantizeChannel(int value)
{
    if (value < 0)
        return 0;
    if (value >= 0x1000000)
        return 0xffffff;
    int level = value / 0x80000 * 0x80000;
    if (value - level > 0x40000)
        level += 0x80000;
    return level;
}

static inline int ClampHigh(int value)
{
    return value >= 0x1000000 ? 0xffffff : value;
}

static inline int Absolute(int value)
{
    return value < 0 ? -value : value;
}

static inline void ReadRow(int format, void* source, int* channels, int width)
{
    if (format == 555)
        ReadRow555((unsigned short*)source, channels, width);
    else if (format == 565)
        ReadRow565((unsigned short*)source, channels, width);
    else
        ReadRow24((UnknownPixel24*)source, channels, width);
}

int DitherConvert(void* source, int format, int width, int height, int sourceStride,
                  int destinationStride, void* output24, void* output565, void* output555,
                  void* output8, UnknownTexturePalette* palette)
{
    int bytesPerPixel = UnknownFunction511970(format);
    unsigned char* indices;
    unsigned char* colors;
    if (palette) {
        indices = palette->UnknownFunction4de280();
        colors = palette->UnknownFunction4de270();
    }
    if (g_ditherCapacity < width) {
        if (g_ditherRows[0])
            DebugFree(g_ditherRows[0], __FILE__, 1065);
        g_ditherCapacity = 0;
        g_ditherRows[0] = (int*)DebugMalloc(width * 24, __FILE__, 1068);
        if (!g_ditherRows[0])
            return 0;
        g_ditherCapacity = width;
        g_ditherRows[1] = g_ditherRows[0] + width * 3;
    }
    ReadRow(format, source, g_ditherRows[0], width);
    int rowBytes = bytesPerPixel * sourceStride;
    unsigned char* nextSource = (unsigned char*)source + rowBytes;
    unsigned char* row24 = (unsigned char*)output24;
    unsigned short* row565 = (unsigned short*)output565;
    unsigned short* row555 = (unsigned short*)output555;
    unsigned char* row8 = (unsigned char*)output8;
    for (int y = 0; y < height; y++) {
        int* current = g_ditherRows[y % 2];
        int* next = g_ditherRows[1 - y % 2];
        unsigned char* to24 = row24;
        unsigned short* to565 = row565;
        unsigned short* to555 = row555;
        unsigned char* to8 = row8;
        if (y != height - 1)
            ReadRow(format, nextSource, next, width);
        for (int x = 0; x < width; x++) {
            int red = current[0];
            int qr = QuantizeChannel(red);
            int green = current[1];
            int qg = QuantizeChannel(green);
            int blue = current[2];
            int qb = QuantizeChannel(blue);
            unsigned char index;
            if (palette) {
                index = indices[(unsigned short)((ClampHigh(qr) >> 9 & 0xfc1f | ClampHigh(qg) >> 14) & 0xffe0
                                                 | ClampHigh(qb) >> 19)];
                unsigned char* color = colors + index * 3;
                int pr = color[0] << 16;
                int pg = color[1] << 16;
                int pb = color[2] << 16;
                if (Absolute(red - pr) <= 0 && Absolute(green - pg) <= 0 && Absolute(blue - pb) <= 0) {
                    qr = pr;
                    qg = pg;
                    qb = pb;
                }
            }
            int errorRed = red - qr;
            int errorGreen = green - qg;
            int errorBlue = blue - qb;
            current[0] = ClampHigh(qr);
            current[1] = ClampHigh(qg);
            current[2] = ClampHigh(qb);
            current[0] = current[0] < 0 ? 0 : current[0];
            current[1] = current[1] < 0 ? 0 : current[1];
            current[2] = current[2] < 0 ? 0 : current[2];
            if (output24) {
                to24[0] = (unsigned char)(current[0] >> 16);
                to24[1] = (unsigned char)(current[1] >> 16);
                to24[2] = (unsigned char)(current[2] >> 16);
                to24 += 3;
            }
            if (output565) {
                *to565 = (unsigned short)((current[0] >> 8 & 0xf81f | current[1] >> 13) & 0xffe0 | current[2] >> 19);
                to565++;
            }
            if (output555) {
                *to555 = (unsigned short)((current[0] >> 9 & 0xfc1f | current[1] >> 14) & 0xffe0 | current[2] >> 19);
                to555++;
            }
            if (output8) {
                *to8 = index;
                to8++;
            }
            if (x < width - 1) {
                int e7 = errorRed * 7 / 16;
                current[3] += e7;
                next[3] += errorRed - e7 - errorRed * 5 / 16 - errorRed * 3 / 16;
                e7 = errorGreen * 7 / 16;
                current[4] += e7;
                next[4] += errorGreen - e7 - errorGreen * 5 / 16 - errorGreen * 3 / 16;
                e7 = errorBlue * 7 / 16;
                current[5] += e7;
                next[5] += errorBlue - e7 - errorBlue * 5 / 16 - errorBlue * 3 / 16;
            }
            next[0] += errorRed * 5 / 16;
            next[1] += errorGreen * 5 / 16;
            next[2] += errorBlue * 5 / 16;
            if (x > 0) {
                next[-3] += errorRed * 3 / 16;
                next[-2] += errorGreen * 3 / 16;
                next[-1] += errorBlue * 3 / 16;
            }
            current += 3;
            next += 3;
        }
        nextSource += rowBytes;
        row24 += destinationStride * 3;
        row565 += destinationStride;
        row555 += destinationStride;
        row8 += destinationStride;
    }
    return 1;
}
