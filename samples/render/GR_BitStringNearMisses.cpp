// Near-miss GR_BitString candidates (0x004238c0..0x00423f6d), kept out of
// src/reconstructed until they match. The canonical file is included first so
// the tables, the byte-order macro and the class are the same.
//
// 0x004239c0 (420 bytes; sub-word shift toward the front): control flow,
// loop shape and memory accesses match. Retail keeps the count in ebx and
// `this` in ebp (swapped here), places `32 - count` before the first swap,
// and ends the swaps whose result is stored with `or reg_b3, reg_rest`
// (the candidate emits the opposite operand order). Rewriting the OR tree,
// an inline swap helper and declaration order had no effect.
//
// 0x00423b70 (442 bytes; sub-word shift toward the back): as 0x004239c0;
// in addition retail rotates the source-pointer decrement to the loop head
// (`lea esi,[edx-4]` ... `add esi,4`, then `[esi-4]` / `sub esi,4`).

#include "../../src/reconstructed/GR_BitString.cpp"

// 0x004239c0
void GR_BitString::UnknownFunction4239c0(int count) {
    unsigned int* dst = (unsigned int*)field_0x08;
    unsigned int* src;
    int words = (field_0x04 + 31) / 32;
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
            *dst++ |= ~g_UnknownHighMasks568098[carryShift];
            *src = g_UnknownHighMasks568098[carryShift];
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
    if (!field_0x18)
        *dst &= g_UnknownHighMasks568098[carryShift];
    else
        *dst |= ~g_UnknownHighMasks568098[carryShift];
}

// 0x00423b70
void GR_BitString::UnknownFunction423b70(int count) {
    int words = (field_0x04 + 31) / 32;
    unsigned int* dst = (unsigned int*)field_0x08 + words - 1;
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
            *--dst |= g_UnknownHighMasks568098[count];
            *src = ~g_UnknownHighMasks568098[count];
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
    if (!field_0x18)
        *--dst &= ~g_UnknownHighMasks568098[count];
    else
        *--dst |= g_UnknownHighMasks568098[count];
}
