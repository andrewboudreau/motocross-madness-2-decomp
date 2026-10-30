// Group2_TexturePalette.cpp -- bring-up (link) definition.
//
// 0x004de270 (`lea eax, [ecx+0x10]; ret`): the palette's RGB entry table.
// Retail has one body; src/reconstructed/Quantize.cpp defines it as
// ColorMapper::GetColors, while Pixtrans code views the same object as
// UnknownTexturePalette (TextureMap.h) and calls it by this name. The two
// views are provisional; status best-guess-unreviewed.

#include "TextureMap.h"

// 0x004de270
unsigned char* UnknownTexturePalette::GetColors() {
    return &entries[0][0];
}
