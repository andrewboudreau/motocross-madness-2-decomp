// Complete sub-word shifts; provisional names and unknown original TU.
// Retail extents420/442 bytes; independently reviewed functional candidate.
#include "../../src/reconstructed/GR_BitString.h"
extern unsigned int g_WordHighBitMasks[33];
#ifndef UNKNOWN_BIG_ENDIAN_WORD
#define UNKNOWN_BIG_ENDIAN_WORD(x)                                                            \
    ((((((unsigned int)((unsigned char*)&(x))[0] << 8) | ((unsigned char*)&(x))[1]) << 8)     \
      | ((unsigned char*)&(x))[2]) << 8 | ((unsigned char*)&(x))[3])
#endif

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
