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

// Pixtrans.cpp pixel converters (all cdecl; PCVideoCard.cpp's code comes
// before them). Strides are in pixels. Names are provisional.

// 0x004d1970, 0x004d1a20, 0x004d1ac0, 0x004d1b40: every `from` pixel in
// the width x height `bits` becomes `to` (32, 24, 16 and 8-bit). Except in
// 8-bit, pixels already equal to `to` are first nudged off it. Return 1.
int UnknownFunction4d1970(void* bits, UnknownPixel32 from, UnknownPixel32 to, int width, int height, int stride);
int UnknownFunction4d1a20(void* bits, UnknownPixel24 from, UnknownPixel24 to, int width, int height, int stride);
int UnknownFunction4d1ac0(void* bits, unsigned short from, unsigned short to, int width, int height, int stride);
int UnknownFunction4d1b40(void* bits, unsigned char from, unsigned char to, int width, int height, int stride);

// Converters from `source` into `destination` (width x height, strides in
// pixels); all return 1.
// 0x004d0700: 565 to 8888; the 0xRRGGBB `key` becomes opaque magenta.
int UnknownFunction4d0700(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d07d0: 1555 to 8888.
int UnknownFunction4d07d0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d0870: 4444 to 8888.
int UnknownFunction4d0870(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d0e40: 555 to 8888; the 0xRRGGBB `key` becomes opaque magenta.
int UnknownFunction4d0e40(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d0f10: 555 to 24-bit; 555 magenta stays magenta.
int UnknownFunction4d0f10(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d0fb0: 555 to 565.
int UnknownFunction4d0fb0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d1030: 565 to 555.
int UnknownFunction4d1030(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d10b0: palette indices to 24-bit RGB.
int UnknownFunction4d10b0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette);
// 0x004d1150, 0x004d11c0: palette indices through the palette's 16-bit
// tables (+0x510, +0x310).
int UnknownFunction4d1150(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette);
int UnknownFunction4d11c0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, UnknownTexturePalette* palette);

// 0x004d1230: 8888 to 4444.
int UnknownFunction4d1230(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride);
// 0x004d12d0: 8888 to 1555, opaque where alpha >= `alphaThreshold`.
int UnknownFunction4d12d0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold);
// 0x004d1370: 24-bit to 1555; the 0xRRGGBB `key` becomes transparent 0.
int UnknownFunction4d1370(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int key);
// 0x004d1440, 0x004d15f0: 4444 to 555 or 565; pixels with alpha below
// `alphaThreshold` become `color` (0xRRGGBB).
int UnknownFunction4d1440(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold, unsigned int color);
int UnknownFunction4d15f0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold, unsigned int color);
// 0x004d1530: 4444 to 1555; alpha below `alphaThreshold` becomes 0.
int UnknownFunction4d1530(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int alphaThreshold);
// 0x004d16e0: 555 to 1555, transparent where the pixel is `key`.
int UnknownFunction4d16e0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d1780: 1555 to 555; transparent pixels become `key`.
int UnknownFunction4d1780(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d1810: 565 to 1555, transparent where the pixel is `key`.
int UnknownFunction4d1810(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);
// 0x004d18c0: 1555 to 565; transparent pixels become `key`.
int UnknownFunction4d18c0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, unsigned int key);

// 0x004cde20: halves 24-bit `source` into the width x height `destination`.
void UnknownFunction4cde20(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride);

// Per-format halving downsamplers behind 0x004d1b90 (8888, 24-bit, 4444,
// 1555, 565, 555, palette); `a` and `filter` are passed through.
int UnknownFunction4cfaf0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a);
int UnknownFunction4cfc40(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a);
int UnknownFunction4d0020(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a);
int UnknownFunction4d0170(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a);
int UnknownFunction4d02c0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a, int filter);
int UnknownFunction4d0440(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a, int filter);
int UnknownFunction4d05c0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a, UnknownTexturePalette* palette);

// 0x004d1b90: downsamples `source` into the width x height `destination`
// with the helper for `format`; 0 for other formats.
int UnknownFunction4d1b90(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a, int format, UnknownTexturePalette* palette, int filter);

// 0x004d1d20: copies `source` into `destination`, converting the format;
// 0x004d24d0 inspects the converted bits.
void UnknownFunction4d1d20(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, int format, int sourceFormat, int a,
                           UnknownTexturePalette* palette, int alphaThreshold, unsigned int key);
int UnknownFunction4d24d0(void* bits, int width, int height, int stride, int format,
                          UnknownTexturePalette* palette);
