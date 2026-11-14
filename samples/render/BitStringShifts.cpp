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
// for any subset of the carryShift uses (worse), and 0..8 dead initialised
// locals.
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
// swapped fill arms (367), the first word read through `src[1]` (45),
// a mask local in the 0xffffffff arm (204), a count-up loop (155),
// `32 - count` in place of carryShift (15) and `src` re-derived from
// `data` (17); `&data[words - 1]` and a separate `>>= count` stay 387.
// Also 387: 0..8 dead initialised locals, `i != 0`/`--i` and `for (i =
// words; --i;)` loops, `--src`, `!*src`, a split `dst--` in the 0xffffffff
// arm and the head's `dst++`/`src++` in either order. `i > 0` is 386, `dst--`
// hoisted ahead of the value test 155 and ahead of the fill arms 343.
// Wave 32: whenever VC6 folds `src = dst - 1; src++` away (src defined after
// `dst++`, `src++` ahead of the store, `src = data + words - 2; dst = src +
// 1`) src gets esi and dst edx as retail and the loop matches (149-151 of 158
// instructions, 437-439 bytes), but the head then lacks retail's unfolded
// `lea esi, [edx - 4]` .. `add edx, 4` / `add esi, 4`; every form that keeps
// the pair unfolded gives dst esi. Also 387: the swap macro with its terms
// reversed, `<< 24` forms or casts, an inline swap taking a reference or a
// pointer, a `w = *--src` local, the head steps in the for initialiser
// (`dst++, src++`; `src++, dst++` 6/443), `dst[0] =`. Worse: `dst[-1] =` 378,
// an inline swap by value 170, `i` assigned before the first store 122, a
// post-decrement mirror of the loop 60/417, an indexed `w[i]` loop 33/428, a
// `last` temporary 13.
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
