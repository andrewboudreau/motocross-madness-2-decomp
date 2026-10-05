#pragma once

// Lzw.cpp (D:\aardvark\VC\krusty2\Lzw.cpp): a variable-width LZW decoder.
// The constants, the banked dictionary and the decode loop follow the
// LZW15V scheme (15-bit codes, a 35023-entry table in 256-entry banks,
// codes 256/257/258 for end of stream, wider codes and a dictionary flush,
// new strings from 259). The names below describe that behaviour, which the
// retail code proves; the original identifiers are unknown.

// 0x004a03d0: decodes the LZW stream at `input` into `output`. `size` sets
// the bit reader's limit (size * 8 bits); callers pass the decoded size.
// The stream starts with a 32-bit word the decoder skips.
void UnknownFunction4a03d0(unsigned char* output, const void* input, int size);
