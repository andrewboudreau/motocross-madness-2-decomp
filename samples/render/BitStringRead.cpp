// Complete reader at 0x00423ef0 (125 retail bytes); nonexact C++ reconstruction.
// Shared global state and eager next-word reads are intentional. Original TU unknown.
// Valid counts are 0..32 with 0..31 bits used, so every shift count is 0..31. The
// exception is a zero-count read at used 0, where `32 - used` is 32 but the mask is 0.
// VC6 emits a plain `shr/shl reg, cl` as retail does (docs/BITSTRING_READ.md).
#include "../../src/reconstructed/GR_BitString.h"
extern unsigned int* g_LowBitMaskTable;
extern unsigned int g_BitReaderWord;
extern int g_BitReaderBitsUsed;
extern unsigned int* g_BitReaderCursor;

unsigned int GR_BitString::ReadBits(int count)
{
    unsigned int result = g_LowBitMaskTable[count];
    unsigned int word = g_BitReaderWord;
    int used = g_BitReaderBitsUsed + count;
    if (used < 32) {
        g_BitReaderBitsUsed = used;
        result &= word >> (32 - used);
    } else {
        unsigned int* next = g_BitReaderCursor;
        used -= 32;
        g_BitReaderBitsUsed = used;
        next++;
        unsigned int raw = *next;
        result &= word << used;
        raw = (raw >> 24) | ((raw >> 8) & 0xff00) | ((raw << 8) & 0xff0000) | (raw << 24);
        g_BitReaderCursor = next;
        g_BitReaderWord = raw;
        if (used != 0)
            result |= raw >> (32 - used);
    }
    return result;
}
