// Complete reader at 0x00423ef0 (125 retail bytes); nonexact C++ reconstruction.
// Shared global state and eager next-word reads are intentional. Original TU unknown.
#include "../../src/reconstructed/GR_BitString.h"
extern unsigned int* g_LowBitMaskTable;
extern unsigned int g_BitReaderWord;
extern int g_BitReaderBitsUsed;
extern unsigned int* g_BitReaderCursor;

unsigned int GR_BitString::ReadBits(int count)
{
    unsigned int mask = g_LowBitMaskTable[count];
    int used = (int)((unsigned int)g_BitReaderBitsUsed + (unsigned int)count);
    unsigned int word = g_BitReaderWord;
    if (used < 32) {
        g_BitReaderBitsUsed = used;
        return (word >> ((32u - (unsigned int)used) & 31)) & mask;
    }
    used -= 32;
    g_BitReaderBitsUsed = used;
    unsigned int* next = g_BitReaderCursor + 1;
    unsigned int raw = *next;
    unsigned int nextWord = (raw >> 24) | ((raw >> 8) & 0xff00) |
                            ((raw << 8) & 0xff0000) | (raw << 24);
    unsigned int result = (word << ((unsigned int)used & 31)) & mask;
    g_BitReaderCursor = next;
    g_BitReaderWord = nextWord;
    if (used)
        result |= nextWord >> ((32u - (unsigned int)used) & 31);
    return result;
}
