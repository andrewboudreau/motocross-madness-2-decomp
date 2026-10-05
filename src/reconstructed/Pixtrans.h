#pragma once

#include "RenderInterfaces.h"

struct UnknownTexturePalette;

// Pixtrans.cpp pixel converters (all cdecl; PCVideoCard.cpp's code comes
// before them). Strides are in pixels. Names are provisional.

// 0x004d1970, 0x004d1a20, 0x004d1ac0, 0x004d1b40: every `from` pixel in
// the width x height `bits` becomes `to` (32, 24, 16 and 8-bit). Except in
// 8-bit, pixels already equal to `to` are first nudged off it. Return 1.
int UnknownFunction4d1970(void* bits, UnknownPixel32 from, UnknownPixel32 to, int width, int height, int stride);
int UnknownFunction4d1a20(void* bits, UnknownPixel24 from, UnknownPixel24 to, int width, int height, int stride);
int UnknownFunction4d1ac0(void* bits, unsigned short from, unsigned short to, int width, int height, int stride);
int UnknownFunction4d1b40(void* bits, unsigned char from, unsigned char to, int width, int height, int stride);

// 0x004d1b90: downsamples `source` into the width x height `destination`;
// 0 on failure.
int UnknownFunction4d1b90(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int a, int format, UnknownTexturePalette* palette, int filter);

// 0x004d1d20: copies `source` into `destination`, converting the format;
// 0x004d24d0 inspects the converted bits.
void UnknownFunction4d1d20(void* destination, void* source, int width, int height, int destinationStride,
                           int sourceStride, int format, int sourceFormat, int a,
                           UnknownTexturePalette* palette, int alphaThreshold, unsigned int key);
int UnknownFunction4d24d0(void* bits, int width, int height, int stride, int format,
                          UnknownTexturePalette* palette);
