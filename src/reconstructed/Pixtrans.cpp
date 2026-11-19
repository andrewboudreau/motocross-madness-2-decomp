// Pixtrans.cpp's pixel converters. Names are provisional; see Pixtrans.h.

#include <stdlib.h>
#include <string.h>

#include "Pixtrans.h"

#include "DebugAlloc.h"
#include "TextureMap.h"
#include "Quantize.h"
#include "Tgafile.h"

// 0x004cde20: halves 24-bit pixels, averaging each 2x2 block.
int Halve24(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    RgbPixel* sourceRow = (RgbPixel*)source;
    RgbPixel* row = (RgbPixel*)destination;
    for (int y = 0; y < height; y++) {
        RgbPixel* top = sourceRow;
        RgbPixel* to = row;
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

// 0x004cdf10. Not exact (110 of 636 bytes, 628 compiled: VC6 strength-reduces
// the lower row to `top + sourceStride` where retail biases it to the alpha
// byte and spills it; docs/PIXTRANS.md forms tried).
// Halves 8888 pixels. Colour averages the 2x2 pixels whose
// alpha is set; alpha is the block's sum / 4. (The fourth pixel adds the
// third one's alpha, as in retail.)
int Halve8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    RgbaPixel* sourceRow = (RgbaPixel*)source;
    RgbaPixel* row = (RgbaPixel*)destination;
    for (int y = 0; y < height; y++) {
        RgbaPixel* to = row;
        for (int x = 0; x < width; x++, to++) {
            RgbaPixel* top = sourceRow + x * 2;
            int red = 0;
            int green = 0;
            int blue = 0;
            int alpha = 0;
            int count = 0;
            if (top[0].alpha) {
                red += top[0].red;
                green += top[0].green;
                blue += top[0].blue;
                alpha += top[0].alpha;
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
            unsigned char alpha2 = top[sourceStride].alpha;
            if (alpha2) {
                red += top[sourceStride].red;
                green += top[sourceStride].green;
                blue += top[sourceStride].blue;
                alpha += alpha2;
                count++;
            }
            if (top[sourceStride + 1].alpha) {
                red += top[sourceStride + 1].red;
                green += top[sourceStride + 1].green;
                blue += top[sourceStride + 1].blue;
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

// 0x004ce420. Not exact (280 of 456 bytes, 458 compiled: VC6 loads the source
// row into ebp at entry where retail copies it from eax at each row head;
// docs/PIXTRANS.md forms tried).
// Halves 1555 pixels; colour averages the opaque pixels of
// each 2x2 block, which stays opaque when at least two of them are.
int Halve1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    unsigned short* sourceRow = (unsigned short*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* top = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++, top += 2) {
            int count = 0;
            int red = 0;
            int green = 0;
            int blue = 0;
            unsigned short pixel = top[0];
            unsigned short pixel2 = top[sourceStride];
            unsigned short pixel3 = top[sourceStride + 1];
            if (pixel & 0x8000) {
                red += (pixel >> 7) & 0xf8;
                green += (pixel >> 2) & 0xf8;
                blue += (pixel & 0x1f) << 3;
                count++;
            }

            pixel = top[1];
            if (pixel & 0x8000) {
                red += (pixel >> 7) & 0xf8;
                green += (pixel >> 2) & 0xf8;
                blue += (pixel & 0x1f) << 3;
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
                *to++ = 0x8000 | ((red >> 3) << 10) | ((green >> 3) << 5) | (blue >> 3);
            } else {
                *to++ = 0;
            }
        }
        sourceRow += sourceStride * 2;
        row += destinationStride;
    }
    return 1;
}

// 0x004cee30. Not exact (1129 of 1133 bytes: after the keyed x loop retail
// reloads the source stride before the width; docs/PIXTRANS.md forms tried).
// Halves palette indices. Each 2x2 block's RGB entries are
// averaged and mapped back through the palette's 555-to-index table. If the
// palette holds magenta (255, 0, 255) that index is the key: key pixels are
// left out of the average and a block with fewer than two others stays the key.
int Halve8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, TextureColorMapperView* palette) {
    unsigned char* sourceRow = (unsigned char*)source;
    unsigned char* row = (unsigned char*)destination;
    if (palette) {
        unsigned char* indices = reinterpret_cast<ColorMapper*>(palette)->GetIndexFrom555();
        if (indices) {
            RgbPixel* entries = (RgbPixel*)palette->GetColors();
            if (entries) {
                RgbPixel a, b, c, d;
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
                            int count = 0, red = 0, blue = 0, green = 0;
                            int ia = from[0];
                            a = entries[ia];
                            int ib = from[1];
                            b = entries[ib];
                            int ic = from[sourceStride];
                            c = entries[ic];
                            int id = from[sourceStride + 1];
                            d = entries[id];
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
                } else {
                    for (int y = 0; y < height; y++) {
                        unsigned char* from = sourceRow;
                        unsigned char* to = row;
                        for (int x = 0; x < width; x++, from += 2) {
                            a = entries[from[0]];
                            b = entries[from[1]];
                            c = entries[from[sourceStride]];
                            d = entries[from[sourceStride + 1]];
                            *to++ = indices[((a.red + b.red + c.red + d.red + 16) >> 5) << 10
                                            | (a.blue + b.blue + c.blue + d.blue + 16) >> 5
                                            | ((a.green + b.green + c.green + d.green + 16) >> 5) << 5];
                        }
                        sourceRow += sourceStride * 2;
                        row += destinationStride;
                    }
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

// A 16.16 channel rounded to the nearest of the 32 5-bit levels. Retail
// stores each level to its frame slot as soon as it is formed, which the
// out-parameter reproduces (returned by value, VC6 kept it in a register).
static inline void QuantizeChannel(int value, int& level)
{
    if (value < 0)
        level = 0;
    else if (value >= 0x1000000)
        level = 0xffffff;
    else {
        level = value / 0x80000 * 0x80000;
        if (value - level > 0x40000)
            level += 0x80000;
    }
}

static inline int ClampHigh(int value)
{
    return value >= 0x1000000 ? 0xffffff : value;
}

static inline void ReadRow(int format, void* source, int* channels, int width)
{
    if (format == 555)
        ReadRow555((unsigned short*)source, channels, width);
    else if (format == 565)
        ReadRow565((unsigned short*)source, channels, width);
    else
        ReadRow24((RgbPixel*)source, channels, width);
}

// 0x004cf2a0. Not exact (276 of 1759 bytes, 1745 compiled: where ebp is pushed
// and the order of the y loop's pointer setup differ; docs/PIXTRANS.md forms
// tried).
// Dithers 24-bit, 565 or 555 pixels (Floyd-Steinberg over the two
// error rows) into the requested 24-bit, 565, 555 and palette-index outputs.
int DitherConvert(void* source, int format, int width, int height, int sourceStride,
                  int destinationStride, void* output24, void* output565, void* output555,
                  void* output8, TextureColorMapperView* palette)
{
    int bytesPerPixel = PixelFormatBytesPerPixel(format);
    unsigned char* indices;
    unsigned char* colors;
    if (palette) {
        indices = reinterpret_cast<ColorMapper*>(palette)->GetIndexFrom555();
        colors = palette->GetColors();
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
    for (int y = 0; y < height; y++) {
        int* current = g_ditherRows[y % 2];
        int* next = g_ditherRows[1 - y % 2];
        unsigned char* to8 = (unsigned char*)output8 + y * destinationStride;
        unsigned char* to24 = (unsigned char*)output24 + y * destinationStride * 3;
        unsigned short* to565 = (unsigned short*)output565 + y * destinationStride;
        unsigned short* to555 = (unsigned short*)output555 + y * destinationStride;
        if (y != height - 1)
            ReadRow(format, (unsigned char*)source + (y + 1) * bytesPerPixel * sourceStride, next, width);
        for (int x = 0; x < width; x++) {
            int red = current[0];
            int qr;
            QuantizeChannel(red, qr);
            int green = current[1];
            int qg;
            QuantizeChannel(green, qg);
            int blue = current[2];
            int qb;
            QuantizeChannel(blue, qb);
            int index;
            if (palette) {
                index = indices[(unsigned short)((ClampHigh(qr) >> 9 & 0xfc1f | ClampHigh(qg) >> 14) & 0xffe0
                                                 | ClampHigh(qb) >> 19)];
                int pr = colors[index * 3] << 16;
                int pg = colors[index * 3 + 1] << 16;
                int pb = colors[index * 3 + 2] << 16;
                if (abs(red - pr) <= 0 && abs(green - pg) <= 0 && abs(blue - pb) <= 0) {
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
                *to8 = (unsigned char)index;
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
    }
    return 1;
}

// 0x004cf980: spreads 24-bit pixels into 16.16 channel triples (the
// ditherer's row reader).
void ReadRow24(RgbPixel* source, int* channels, int count) {
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
    RgbPixel* row = (RgbPixel*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        RgbPixel* to = row;
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
            memcpy((RgbPixel*)destination + y * destinationStride, (RgbPixel*)source + y * sourceStride,
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
            memcpy((RgbaPixel*)destination + y * destinationStride, (RgbaPixel*)source + y * sourceStride,
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
int Downsample24To8888(void* destination, void* source, int width, int height, int destinationStride,
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
                          int sourceStride, int levels, TextureColorMapperView* palette) {
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

// 0x004d0700: converts 565 to 8888; the key colour becomes opaque magenta.
int Convert565To8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key) {
    unsigned short* sourceRow = (unsigned short*)source;
    RgbaPixel* row = (RgbaPixel*)destination;
    unsigned short transparent = Pack565(key);
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        RgbaPixel* to = row;
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
    RgbaPixel* row = (RgbaPixel*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        RgbaPixel* to = row;
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
    RgbaPixel* row = (RgbaPixel*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        RgbaPixel* to = row;
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

// 0x004d0900: converts 24-bit to 565, through the ditherer when asked.
int Convert24To565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither) {
    if (dither) {
        DitherConvert(source, 888, width, height, sourceStride, destinationStride, 0, destination,
                              0, 0, 0);
    } else {
        RgbPixel* sourceRow = (RgbPixel*)source;
        unsigned short* row = (unsigned short*)destination;
        for (int y = 0; y < height; y++) {
            RgbPixel* from = sourceRow;
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
        RgbPixel* sourceRow = (RgbPixel*)source;
        unsigned short* row = (unsigned short*)destination;
        for (int y = 0; y < height; y++) {
            RgbPixel* from = sourceRow;
            unsigned short* to = row;
            for (int x = 0; x < width; x++, to++, from++)
                *to = (from->red >> 3) << 10 | (from->green >> 3) << 5 | from->blue >> 3;
            sourceRow += sourceStride;
            row += destinationStride;
        }
    }
    return 1;
}

// 0x004d0aa0: converts 24-bit to palette indices through the 555 table.
int Convert24To8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, TextureColorMapperView* palette) {
    if (dither && palette) {
        DitherConvert(source, 888, width, height, sourceStride, destinationStride, 0, 0, 0,
                              destination, palette);
    } else {
        RgbPixel* sourceRow = (RgbPixel*)source;
        unsigned char* row = (unsigned char*)destination;
        unsigned char* indices = reinterpret_cast<ColorMapper*>(palette)->GetIndexFrom555();
        for (int y = 0; y < height; y++) {
            RgbPixel* from = sourceRow;
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
                          int sourceStride, int dither, TextureColorMapperView* palette) {
    if (dither && palette) {
        DitherConvert(source, 565, width, height, sourceStride, destinationStride, 0, 0, 0,
                              destination, palette);
    } else {
        unsigned short* sourceRow = (unsigned short*)source;
        unsigned char* row = (unsigned char*)destination;
        unsigned char* indices = reinterpret_cast<ColorMapper*>(palette)->GetIndexFrom565();
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

// 0x004d0c40: converts 555 to palette indices: dithered when asked, through
// the assembly 0x004d0d40 for unpadded rows of a multiple of eight pixels,
// otherwise through the 555 table.
int Convert555To8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, TextureColorMapperView* palette) {
    if (dither && palette) {
        DitherConvert(source, 555, width, height, sourceStride, destinationStride, 0, 0, 0,
                              destination, palette);
    } else if (sourceStride == width && sourceStride % 8 == 0) {
        Convert555To8Fast(source, destination, destinationStride, width, height,
                              reinterpret_cast<ColorMapper*>(palette)->GetIndexFrom555());
    } else {
        unsigned short* sourceRow = (unsigned short*)source;
        unsigned char* row = (unsigned char*)destination;
        unsigned char* indices = reinterpret_cast<ColorMapper*>(palette)->GetIndexFrom555();
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

// 0x004d0e40: converts 555 to 8888; the key colour becomes opaque magenta.
int Convert555To8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key) {
    unsigned short* sourceRow = (unsigned short*)source;
    RgbaPixel* row = (RgbaPixel*)destination;
    unsigned short transparent = Pack555(key);
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        RgbaPixel* to = row;
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
    RgbPixel* row = (RgbPixel*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* from = sourceRow;
        RgbPixel* to = row;
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
                          int sourceStride, TextureColorMapperView* palette) {
    unsigned char* sourceRow = (unsigned char*)source;
    RgbPixel* row = (RgbPixel*)destination;
    for (int y = 0; y < height; y++) {
        unsigned char* from = sourceRow;
        RgbPixel* to = row;
        for (int x = 0; x < width; x++, to++, from++) {
            to->red = palette->entries[*from][0];
            to->green = palette->entries[*from][1];
            to->blue = palette->entries[*from][2];
        }
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1150: converts palette indices to 565 through entries565 (+0x510).
int Convert8To565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, TextureColorMapperView* palette) {
    unsigned char* sourceRow = (unsigned char*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned char* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++)
            *to++ = palette->entries565[*from++];
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d11c0: converts palette indices to 555 through entries555 (+0x310).
int Convert8To555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, TextureColorMapperView* palette) {
    unsigned char* sourceRow = (unsigned char*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned char* from = sourceRow;
        unsigned short* to = row;
        for (int x = 0; x < width; x++)
            *to++ = palette->entries555[*from++];
        sourceRow += sourceStride;
        row += destinationStride;
    }
    return 1;
}

// 0x004d1230: converts 8888 to 4444.
int Convert8888To4444(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride) {
    RgbaPixel* sourceRow = (RgbaPixel*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        unsigned short* to = row;
        RgbaPixel* from = sourceRow;
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
    RgbaPixel* sourceRow = (RgbaPixel*)source;
    unsigned short* row = (unsigned short*)destination;
    for (int y = 0; y < height; y++) {
        RgbaPixel* from = sourceRow;
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
    RgbPixel* sourceRow = (RgbPixel*)source;
    unsigned short* row = (unsigned short*)destination;
    unsigned char keyRed = key >> 16;
    unsigned char keyGreen = key >> 8;
    for (int y = 0; y < height; y++) {
        unsigned short* to = row;
        RgbPixel* from = sourceRow;
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

// 0x004d1970: replaces 32-bit `from` pixels with `to`; pixels already equal
// to `to` get their blue byte nudged (255 down, otherwise up) first.
int ReplaceColor32(void* bits, RgbaPixel from, RgbaPixel to, int width, int height, int stride) {
    RgbaPixel* row = (RgbaPixel*)bits;
    for (int y = 0; y < height; y++) {
        RgbaPixel* pixel = row;
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
int ReplaceColor24(void* bits, RgbPixel from, RgbPixel to, int width, int height, int stride) {
    RgbPixel* row = (RgbPixel*)bits;
    for (int y = 0; y < height; y++) {
        RgbPixel* pixel = row;
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

// 0x004d1b90: hands the downsampling to the helper for `format`.
int Downsample(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, int format, TextureColorMapperView* palette, int filter) {
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

// 0x004d1d20: converts `source` (pixel format `sourceFormat`) into
// `destination` (`format`) with the matching converter; same-format copies
// go through the downsamplers with no halving. 0 for unsupported pairs.
int ConvertPixels(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, int format, int sourceFormat, int dither,
                           TextureColorMapperView* palette, int alphaThreshold, unsigned int key) {
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
            return Convert8To555(destination, source, width, height, destinationStride, sourceStride, palette);
        case 8:
            return Downsample8(destination, source, width, height, destinationStride, sourceStride, 0,
                                         palette);
        case 565:
            return Convert8To565(destination, source, width, height, destinationStride, sourceStride, palette);
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
            return Downsample24To8888(destination, source, width, height, destinationStride, sourceStride, 0, key);
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
int AverageColor(void* bits, int width, int height, int stride, int format,
                          TextureColorMapperView* palette) {
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
                red += palette->entries[*pixel][0];
                green += palette->entries[*pixel][1];
                blue += palette->entries[*pixel][2];
            }
            row += width;
        }
        return UNKNOWN_ARGB(0xff, red / count, green / count, blue / count);
    }
    case 888: {
        RgbPixel* row = (RgbPixel*)bits;
        for (y = 0; y < height; y++) {
            RgbPixel* pixel = row;
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
        RgbaPixel* row = (RgbaPixel*)bits;
        for (y = 0; y < height; y++) {
            RgbaPixel* pixel = row;
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
