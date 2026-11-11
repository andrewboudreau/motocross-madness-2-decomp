// Complete sub-word shifts; provisional names and unknown original TU.
// Retail extents420/442 bytes; independently reviewed functional candidate.
#include "../../src/reconstructed/GR_BitString.h"
extern unsigned int g_WordHighBitMasks[33];
#ifndef UNKNOWN_BIG_ENDIAN_WORD
#define UNKNOWN_BIG_ENDIAN_WORD(x)                                                            \
    ((((((unsigned int)((unsigned char*)&(x))[0] << 8) | ((unsigned char*)&(x))[1]) << 8)     \
      | ((unsigned char*)&(x))[2]) << 8 | ((unsigned char*)&(x))[3])
#endif

// 0x004239c0 (343/420): retail keeps `this` in ebp and `count` in ebx and
// computes `32 - count` into edi before the first word; VC6 here swaps the
// two and computes it later. `src = (unsigned int*)data + 1;` gives retail's
// allocation and order (7 differing instructions) but re-reads `data` where
// retail forms `lea edx, [esi + 4]`, so it stays `dst + 1`. Unchanged at 343:
// every order of the words/dst/carryShift assignments (declared at the top or
// assigned as in ShiftTowardFront), for/while(--i)/for(;--i;) loops,
// `register` on any subset of the locals, `const int count`, a `count` copy,
// the first word in its own local or block, `src = dst; *src++ = ...`, the
// src step in the for header, swap-macro spellings (operands and `+`), a
// `fill` local or swapped fill arms (worse), and `32 - count` written out
// for any subset of the carryShift uses (worse).
void GR_BitString::ShiftBitsTowardFront(int count) {
    unsigned int* dst = (unsigned int*)data;
    unsigned int* src;
    int words = (bitCount + 31) / 32;
    int carryShift = 32 - count;
    unsigned int value;
    int i;

    value = *dst;
    value = UNKNOWN_BIG_ENDIAN_WORD(value) << count;
    *dst = UNKNOWN_BIG_ENDIAN_WORD(value);
    src = dst + 1;
    for (i = words - 1; i; i--) {
        if (*src == 0) {
            dst++;
        } else if (*src == 0xffffffff) {
            *dst++ |= ~g_WordHighBitMasks[carryShift];
            *src = g_WordHighBitMasks[carryShift];
        } else {
            *src = UNKNOWN_BIG_ENDIAN_WORD(*src);
            value = *src >> carryShift;
            if (value)
                *dst |= UNKNOWN_BIG_ENDIAN_WORD(value);
            dst++;
            *src <<= count;
            if (*src)
                *src = UNKNOWN_BIG_ENDIAN_WORD(*src);
        }
        src++;
    }
    if (!fillWord)
        *dst &= g_WordHighBitMasks[carryShift];
    else
        *dst |= ~g_WordHighBitMasks[carryShift];
}

// 0x00423b70 (387/442): retail computes `src = dst - 1` (lea esi, [edx - 4]),
// steps it forward with `dst` after the last word (add esi, 4) and
// pre-decrements it at the loop top; the bottom-decrement loop gave 93/438.
// Left: retail keeps `src` in esi and `dst` in edx (VC6 here the other way)
// and two swap-macro ORs end in the other register. Unchanged at 387: `src`
// assigned before or after the first read, `src++` and `dst++` as separate
// statements or a comma expression, `*--src` in the zero test, while(--i)
// and a split `*--dst` in the 0xffffffff arm. Worse: `src = dst - 1` after
// the store (83), src assigned from `data` (59), while(--words) (154),
// swapped fill arms (367), the first word read through `src[1]` (45).
void GR_BitString::ShiftBitsTowardBack(int count) {
    int words = (bitCount + 31) / 32;
    unsigned int* dst = (unsigned int*)data + words - 1;
    unsigned int* src = dst - 1;
    int carryShift = 32 - count;
    unsigned int value;
    int i;

    value = *dst;
    value = UNKNOWN_BIG_ENDIAN_WORD(value) >> count;
    *dst++ = UNKNOWN_BIG_ENDIAN_WORD(value);
    src++;
    for (i = words - 1; i; i--) {
        src--;
        if (*src == 0) {
            dst--;
        } else if (*src == 0xffffffff) {
            *--dst |= g_WordHighBitMasks[count];
            *src = ~g_WordHighBitMasks[count];
        } else {
            *src = UNKNOWN_BIG_ENDIAN_WORD(*src);
            value = *src << carryShift;
            if (value)
                *--dst |= UNKNOWN_BIG_ENDIAN_WORD(value);
            else
                dst--;
            *src >>= count;
            if (*src)
                *src = UNKNOWN_BIG_ENDIAN_WORD(*src);
        }
    }
    if (!fillWord)
        *--dst &= ~g_WordHighBitMasks[count];
    else
        *--dst |= g_WordHighBitMasks[count];
}
