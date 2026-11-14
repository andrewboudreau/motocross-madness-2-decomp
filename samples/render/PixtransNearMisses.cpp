// Near-miss Pixtrans.cpp candidates, kept out of src/reconstructed until they
// match. See docs/PIXTRANS.md.
//
// Halve8888 (0x004cdf10, 636 bytes with its jump table; candidate 628, 110
// strict, 182 of 203 aligned, 0.925 masked): the 8888 halver. Control flow,
// the count switch and the retail alpha quirk line up. Retail derives the
// block pointer from x (`sourceRow + x * 2` inside the x loop) and reads the
// lower pixels as `top[sourceStride]`: separate `top`/`bottom` pointers
// stepped by 2 in the for increment kept the upper pointer on the stack and
// most registers differed (82 of 203 aligned). Retail reads the first alpha
// twice rather than through a local; an `alpha0` local cost 0.23 masked.
// Still differs: VC6 strength-reduces the lower row to `top + sourceStride`
// (lea edx, [ecx+ebp*4]) where retail biases it to the alpha byte (+3) and
// spills it; the order of the green/blue reads moves the upper pointer's
// bias (+1 green first, +2 blue first) but not the lower one. Ruled out:
// accumulator declaration orders, dropping the alpha1/alpha2 locals (same
// code), fully indexed `sourceRow[x * 2 + k]` and `sourceRow[i + k]` forms,
// `bottom` declared in the loop, a count-down x loop.
//
// Halve1555 (0x004ce420, 456 bytes; candidate 458, 280 strict): the 1555
// halver behind 0x004d0170. The arithmetic, the >= 2 test and the packing
// match. Retail reads the lower pair into locals before the first test and
// keeps them in frame slots (0x10 frame), and reuses one `pixel` variable
// for the upper pair (read in place, both in dx): declaring the lower pair
// and the reused pixel after the accumulators gives that (55 -> 246), and
// `*to++ =` in both arms instead of `to++` in the x loop increment gives
// retail's shared `add eax, 2` tail (280). Still differs: VC6 here loads
// `source` into ebp at entry (so ebp is pushed before the height guard and
// popped last) and computes the next source row in ebp, where retail uses
// eax and copies it to ebp at the row head; the first opaque pixel's
// zero-extended value is formed in ecx (retail: in blue's edi). All 24
// accumulator orders, the row/pointer declaration orders, char/int row
// pointers, for-init pointers, count-down loops, the first arm's statement
// orders (48), pixel types and 1..33 dead locals leave it at 280 or worse;
// earlier: every pixel declaration order, a lower-row pointer, a 1555
// bitfield struct and an inline AddPixel1555 helper.
//
// Halve8 (0x004cee30, 1133 bytes; candidate 1133, 1121 of 1125 masked
// bytes, 1129 of 1133 strict): the palette-index halver. The search for the magenta entry, both
// block loops, the key test and the >= 2 rule match, and nested ifs
// reproduce retail's single `return 0` block. The keyed and plain loops are
// an if/else with one `return 1` after it: with a `return 1` after the keyed
// loop VC6 pushes ebp in the prologue, as an if/else it pushes ebp after the
// three checks as retail does (docs/VC6_SHRINK_WRAP.md). Declaring the keyed
// block's accumulators first in the x loop body gives retail's frame (0x48)
// and registers; declared after the four entry copies, VC6 homed
// `sourceRow` in `source`'s slot (frame 0x44) and every slot moved. The
// plain loop ORs the blue term before the green one as retail does. The
// four 3-byte entry copies are declared once at the top of the entries
// block, shared by both loops: declared in each x loop body (in any order)
// they took their slots in the reverse order. Declaring the accumulators
// count, red, blue, green orders the two zero stores as retail's. Still
// differs: after the keyed x loop retail reloads the source stride before
// the width. Updating `row` before `sourceRow`, the row steps or `to++` in
// the for increments, declaring `to` before `from`, a `below` row pointer,
// counting x or y down and hoisting the loop counters, pointers or entry
// indices are the same or worse. So are (40 forms, all 1129) `width > x`,
// the x increments swapped, `1 + sourceStride` and five spellings of the
// source row step.
// Session 14 parentheses (all 1129): `(sourceStride * 2)` in the keyed row
// step, `(count * 4)` in the three averages, a fully bracketed index
// expression, `from[(sourceStride + 1)]`, `(from + sourceStride)[1]` and
// `*(from + sourceStride)`. The two differing loads are the reloads of
// sourceStride (eax) and width (edx) at the keyed x loop's exit, both
// carried into the y loop's head.
// Session 15: a `stride` copy of sourceStride for the keyed branch is propagated back to
// the argument (1129); an end-pointer x loop (`to < end`) loses retail's width down-counter
// (225).
//
// DitherConvert (0x004cf2a0, 1759 bytes; candidate 1745, 276 strict, 534 of
// 566 aligned, 0.954 masked): the Floyd-Steinberg ditherer behind the
// dithered converters. Two 16.16 error rows (grown on demand,
// 0x00689a6c/0x00689a70/0x00689a74), each pixel rounded to the nearest 5-bit
// level per channel and, with a palette, to the palette entry when it is
// exactly that colour; errors spread 7/16 right, 5/16 below, 3/16 below left
// and the remainder below right (retail forms that as e - 7e/16 - 5e/16 -
// 3e/16 with the last term negated first, which is the same value). Checked
// against retail: free/malloc lines 0x429/0x42c, width * 24 bytes, formats
// 0x22b/0x235 select the 555/565 readers, the 555 palette lookup and the
// output packings. Shape: retail uses the CRT abs() intrinsic (cdq/xor/sub;
// a ternary gave jns/neg), stores each quantized level to its slot at once
// (the out-parameter QuantizeChannel), loads the palette entry as
// colors[index * 3 + k] and derives the four output row pointers and the
// next source row from y (`base + y * stride`), which VC6 strength-reduces
// into retail's shared induction variables; stepping separate row pointers
// at the end of the y loop gave 385 of 566 aligned. Still differs: where
// ebp is pushed, the order of the y loop's pointer setup and its rotated
// loop entry jump.
//
// k-scan (s22): compiled behind 0..63 prepended typedefs, 0x004cdf10,
// 0x004ce420, 0x004cee30, 0x004cf2a0 have the same code at every count, so
// their differences are not a declaration-count effect
// (docs/VC6_OPERAND_ORDER.md section 2).
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Pixtrans.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Quantize.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x004cdf10: halves 8888 pixels. Colour averages the 2x2 pixels whose
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

// 0x004ce420: halves 1555 pixels; colour averages the opaque pixels of
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

// 0x004cee30: halves palette indices. Each 2x2 block's RGB entries are
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
