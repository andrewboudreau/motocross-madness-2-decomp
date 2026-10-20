// Near-miss GR_BitString candidates (0x004238c0..0x00423f6d), kept out of
// src/reconstructed until they match. The canonical file is included first so
// the tables, the byte-order macro and the class are the same.
//
// 0x004239c0 (420 bytes; sub-word shift toward the front): control flow,
// loop shape and memory accesses match. Retail keeps the count in ebx and
// `this` in ebp (swapped here), places `32 - count` before the first swap,
// and ends the swaps whose result is stored with `or reg_b3, reg_rest`
// (the candidate emits the opposite operand order). Rewriting the OR tree,
// an inline swap helper and declaration order had no effect; `32 - count`
// written at each use (no carryShift) is worse, assigned after the
// declarations unchanged. Also 343 in all 864 forms of a 2026-10-20 sweep:
// count copied into a local, the fillWord test inverted, `*dst = *dst | `
// either way round, the first swap stored through `value`, `i > 0` and
// `while (i--)` loops, and the six orders of the dst/words/carryShift
// declarations.
//
// 0x00423b70 (442 bytes; sub-word shift toward the back): as 0x004239c0;
// in addition retail rotates the source-pointer decrement to the loop head
// (`lea esi,[edx-4]` ... `add esi,4`, then `[esi-4]` / `sub esi,4`).
// `*--src` at the loop head with `src = dst - 1` set after the first store
// reproduces the head (`[esi-4]` / `sub esi,4`) but not the init/`add` pair
// (11 diffs, 439 bytes); `src--` in the for increment drops the bottom
// decrement only (11, 435); `i > 0`, `while (--i)`, an upward count and
// src derived from `data + i - 1` inside the loop are no better (s14).

#include "../../src/reconstructed/GR_BitString.cpp"

// 0x004239c0
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

// 0x00423b70
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
    for (i = words - 1; i; i--) {
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
        src--;
    }
    if (!fillWord)
        *--dst &= ~g_WordHighBitMasks[count];
    else
        *--dst |= g_WordHighBitMasks[count];
}
