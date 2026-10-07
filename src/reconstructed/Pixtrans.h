#pragma once

#include "RenderInterfaces.h"

struct UnknownTexturePalette;

// A 24-bit 0xRRGGBB colour as a 555 or 565 pixel (blue first, cast as a
// whole: VC6 then keeps retail's per-component shifts).
inline unsigned short Pack555(unsigned int color) {
    return (unsigned short)(((color >> 3) & 0x1f) | ((color >> 6) & 0x3e0) | ((color >> 9) & 0x7c00));
}

inline unsigned short Pack565(unsigned int color) {
    return (unsigned short)(((color >> 3) & 0x1f) | ((color >> 5) & 0x7e0) | ((color >> 8) & 0xf800));
}

// Texture formats are numbered by their decimal bit layout: 8 (palette
// indices), 555, 565, 888 (24-bit RGB), 1555, 4444 and 8888 (ARGB); the
// downsampler switch 0x004d1b90 and the converter dispatch 0x004d1d20 select
// on these values (retail holds them as 0x22b, 0x235, 0x378, 0x613, 0x115c
// and 0x22b8).
//
// Pixtrans.cpp pixel converters (all cdecl; PCVideoCard.cpp's code comes
// before them). Strides are in pixels. Names are provisional.

// 0x004d1970, 0x004d1a20, 0x004d1ac0, 0x004d1b40: every `from` pixel in
// the width x height `bits` becomes `to` (32, 24, 16 and 8-bit). Except in
// 8-bit, pixels already equal to `to` are first nudged off it. Return 1.
int ReplaceColor32(void* bits, UnknownPixel32 from, UnknownPixel32 to, int width, int height, int stride);
int ReplaceColor24(void* bits, UnknownPixel24 from, UnknownPixel24 to, int width, int height, int stride);
int ReplaceColor16(void* bits, unsigned short from, unsigned short to, int width, int height, int stride);
int ReplaceColor8(void* bits, unsigned char from, unsigned char to, int width, int height, int stride);

// Converters from `source` into `destination` (width x height, strides in
// pixels); all return 1.
// 0x004d0700: 565 to 8888; the 0xRRGGBB `key` becomes opaque magenta.
int Convert565To8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d07d0: 1555 to 8888.
int Convert1555To8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d0870: 4444 to 8888.
int Convert4444To8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d0e40: 555 to 8888; the 0xRRGGBB `key` becomes opaque magenta.
int Convert555To8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d0f10: 555 to 24-bit; 555 magenta stays magenta.
int Convert555To24(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004cf2a0: dithers `source` (pixel format `format`) into whichever of
// the outputs is given: 24-bit, 565, 555 or palette indices (with
// `palette`).
int DitherConvert(void* source, int format, int width, int height, int sourceStride,
                          int destinationStride, void* output24, void* output565, void* output555,
                          void* output8, UnknownTexturePalette* palette);
// 0x004d0900, 0x004d09d0: 24-bit to 565 or 555, dithered when `dither`.
int Convert24To565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither);
int Convert24To555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither);
// 0x004d0aa0, 0x004d0b90, 0x004d0c40: 24-bit, 565 and 555 to palette
// indices, dithered against `palette` when `dither` and `palette` are set.
int Convert24To8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, UnknownTexturePalette* palette);
int Convert565To8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, UnknownTexturePalette* palette);
int Convert555To8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int dither, UnknownTexturePalette* palette);
// 0x004d0d40 (hand-written assembly): 555 to indices through `table`,
// eight pixels a step; 0x004d0c40 calls it for rows of a multiple of eight
// pixels with no padding.
void Convert555To8Fast(void* source, void* destination, int destinationStride, int width, int height,
                           unsigned char* table);
// 0x004d0fb0: 555 to 565.
int Convert555To565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d1030: 565 to 555.
int Convert565To555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d10b0: palette indices to 24-bit RGB.
int Convert8To24(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette);
// 0x004d1150, 0x004d11c0: palette indices through the palette's 16-bit
// tables (+0x510, +0x310).
int UnknownFunction4d1150(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette);
int UnknownFunction4d11c0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette);

// 0x004d1230: 8888 to 4444.
int Convert8888To4444(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d12d0: 8888 to 1555, opaque where alpha >= `alphaThreshold`.
int Convert8888To1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold);
// 0x004d1370: 24-bit to 1555; the 0xRRGGBB `key` becomes transparent 0.
int Convert24To1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int key);
// 0x004d1440, 0x004d15f0: 4444 to 555 or 565; pixels with alpha below
// `alphaThreshold` become `color` (0xRRGGBB).
int Convert4444To555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold, unsigned int color);
int Convert4444To565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold, unsigned int color);
// 0x004d1530: 4444 to 1555; alpha below `alphaThreshold` becomes 0.
int Convert4444To1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold);
// 0x004d16e0: 555 to 1555, transparent where the pixel is `key`.
int Convert555To1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d1780: 1555 to 555; transparent pixels become `key`.
int Convert1555To555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d1810: 565 to 1555, transparent where the pixel is `key`.
int Convert565To1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d18c0: 1555 to 565; transparent pixels become `key`.
int Convert1555To565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);

// 0x004cde20: halves 24-bit `source` into the width x height `destination`
// (each pixel averages a 2x2 block); returns 1.
int Halve24(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);

// 0x004cdf10: halves 8888 pixels, averaging colour over the non-transparent
// pixels of each 2x2 block; returns 1.
int Halve8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);

// Per-format halvers behind the downsamplers (not reconstructed): 4444
// 0x004ce190 (its last argument is always 0), 1555 0x004ce420, 555
// 0x004ce5f0 and 565 0x004cea10 (with the filter flag and the magenta key)
// and palette indices 0x004cee30.
int Halve4444(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int value);
int Halve1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
int Halve555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int filter, unsigned short key);
int Halve565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int filter, unsigned short key);
int Halve8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette);

// Per-format halving downsamplers behind 0x004d1b90 (one per format in its
// switch); `levels` (the number of halvings) and `filter` are passed through.
int Downsample24(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels);
int Downsample8888(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels);
int Downsample4444(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels);
int Downsample1555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels);
int Downsample565(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, int filter);
int Downsample555(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, int filter);
int Downsample8(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, UnknownTexturePalette* palette);

// 0x004cf980, 0x004cf9c0, 0x004cfa10: the ditherer's row readers. Each
// spreads `count` 24-bit, 565 or 555 pixels into 16.16 fixed-point channel
// triples.
void ReadRow24(UnknownPixel24* source, int* channels, int count);
void ReadRow565(unsigned short* source, int* channels, int count);
void ReadRow555(unsigned short* source, int* channels, int count);
// 0x004cfa60: 565 to 24-bit.
int Convert565To24(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004cfda0: 24-bit to 8888; the 0xRRGGBB `key` becomes transparent.
void Convert24To8888(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, unsigned int key);
// 0x004cfe70: 24-bit to 8888 through 0x004cfda0, then `levels` halvings
// (0x004cdf10) like 0x004cfc40.
int UnknownFunction4cfe70(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, unsigned int key);

// 0x004d1b90: downsamples `source` into the width x height `destination`
// with the helper for `format`; 0 for other formats.
int UnknownFunction4d1b90(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels, int format, UnknownTexturePalette* palette, int filter);

// 0x004d1d20: copies `source` into `destination`, converting the format;
// 0x004d24d0 inspects the converted bits.
int UnknownFunction4d1d20(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int format, int sourceFormat, int dither,
                          UnknownTexturePalette* palette, int alphaThreshold, unsigned int key);
int UnknownFunction4d24d0(void* bits, int width, int height, int stride, int format,
                          UnknownTexturePalette* palette);
