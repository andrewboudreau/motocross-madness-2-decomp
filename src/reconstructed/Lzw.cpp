// Lzw.cpp -- reconstruction of D:\aardvark\VC\krusty2\Lzw.cpp (0x004a01d0..0x004a05da).
// See Lzw.h and docs/LZW.md. The two __FILE__ references (lines 96 and 104)
// are in 0x004a0360.

#include "Lzw.h"

#include "DebugAlloc.h"
#include "GR_BitString.h"

// Per-TU vector constants. Tier 2: like about 73 other retail TUs, this one
// opens (0x004a01d0..0x004a030b) with four VC6 dynamic initializers, each a
// `jmp` thunk into a body that builds (0,0,0), (1,0,0), (0,1,0) or (0,0,1)
// in a stack temporary and copies it into a TU-private 12-byte global
// (0x0067c4f8, 0x0067c508, 0x0067c518, 0x0067c4e8). In the original they come
// from a widely included math header; a TU-local stand-in type reproduces
// them (copy-initialisation gives the temporary). The names are tier 3.
struct LzwConstVec3 {
    float x, y, z;
    LzwConstVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};
static const LzwConstVec3 kVec3Zero = LzwConstVec3(0.0f, 0.0f, 0.0f);
static const LzwConstVec3 kVec3XAxis = LzwConstVec3(1.0f, 0.0f, 0.0f);
static const LzwConstVec3 kVec3YAxis = LzwConstVec3(0.0f, 1.0f, 0.0f);
static const LzwConstVec3 kVec3ZAxis = LzwConstVec3(0.0f, 0.0f, 1.0f);

// Decoder constants (tier 1: literal operands in 0x004a0310/0x004a03d0).
#define LZW_TABLE_SIZE    35023                       // 0x88cf entries
#define LZW_TABLE_BANKS   ((LZW_TABLE_SIZE >> 8) + 1) // 137 banks of 256
#define LZW_END_OF_STREAM 256
#define LZW_BUMP_CODE     257
#define LZW_FLUSH_CODE    258
#define LZW_FIRST_CODE    259
#define LZW_UNUSED        -1

// One 12-byte dictionary entry (DebugCalloc(256, 12) per bank). The decoder
// reads +4 and +8; the reset marks +0 unused (an encoder's code value; no
// encoder is linked).
struct LzwDictionaryEntry {
    int codeValue;
    unsigned int parentCode;
    char character;
};

#define LZW_DICT(i) g_lzwDictionary[(i) >> 8][(i) & 0xff]

unsigned int g_lzwNextCode;                           // 0x0067c524
char g_lzwDecodeStack[LZW_TABLE_SIZE];                // 0x0067c528
unsigned int g_lzwNextBumpCode;                       // 0x00684df8
int g_lzwCodeBits;                                    // 0x00684dfc
LzwDictionaryEntry* g_lzwDictionary[LZW_TABLE_BANKS]; // 0x00684e00

// 0x004a0310: marks every entry unused and restarts at 9-bit codes.
void UnknownFunction4a0310()
{
    unsigned int i;

    for (i = 0; i < LZW_TABLE_SIZE; i++)
        LZW_DICT(i).codeValue = LZW_UNUSED;
    g_lzwNextCode = LZW_FIRST_CODE;
    g_lzwCodeBits = 9;
    g_lzwNextBumpCode = 511;
}

// 0x004a0360: allocates the banks still missing. On failure it frees the
// banks before the failing one and returns 0.
int UnknownFunction4a0360()
{
    int i;

    for (i = 0; i < LZW_TABLE_BANKS; i++) {
        if (g_lzwDictionary[i] == 0) {
            g_lzwDictionary[i] = (LzwDictionaryEntry*)DebugCalloc(
                256, sizeof(LzwDictionaryEntry), __FILE__, 96);
            if (g_lzwDictionary[i] == 0) {
                for (int j = 0; j < i; j++) {
                    operator delete(g_lzwDictionary[j], __FILE__, 104);
                    g_lzwDictionary[j] = 0;
                }
                return 0;
            }
        }
    }
    return 1;
}

// Pushes the string for `code` onto the decode stack (last character first)
// and returns the new depth. Retail inlines it at both call sites.
inline unsigned int LzwDecodeString(unsigned int count, unsigned int code)
{
    while (code > 255) {
        g_lzwDecodeStack[count++] = LZW_DICT(code).character;
        code = LZW_DICT(code).parentCode;
    }
    g_lzwDecodeStack[count++] = (char)code;
    return count;
}

// 0x004a03d0: the decode loop. The leading 32-bit word is read and dropped;
// a flush code restarts the dictionary, a bump code widens the codes, and a
// code not yet in the dictionary (the KwKwK case) decodes as the previous
// string plus its own first character.
void UnknownFunction4a03d0(unsigned char* output, const void* input, int size)
{
    unsigned int newCode;
    unsigned int oldCode;
    int character;
    unsigned int count;
    // Retail keeps the cursor in ebp and spills `character` into the dead
    // `output` slot; writing through `output` directly spills the cursor.
    unsigned char* dst = output;

    GR_BitString bits(input, size * 8, 0);
    bits.UnknownFunction423ef0(32);
    if (!UnknownFunction4a0360())
        return;
    for (;;) {
        UnknownFunction4a0310();
        oldCode = bits.UnknownFunction423ef0(g_lzwCodeBits);
        if (oldCode == LZW_END_OF_STREAM)
            return;
        character = oldCode;
        *dst++ = (unsigned char)oldCode;
        for (;;) {
            newCode = bits.UnknownFunction423ef0(g_lzwCodeBits);
            if (newCode == LZW_END_OF_STREAM)
                return;
            if (newCode == LZW_FLUSH_CODE)
                break;
            if (newCode == LZW_BUMP_CODE) {
                g_lzwCodeBits++;
                continue;
            }
            if (newCode >= g_lzwNextCode) {
                g_lzwDecodeStack[0] = (char)character;
                count = LzwDecodeString(1, oldCode);
            } else {
                count = LzwDecodeString(0, newCode);
            }
            character = g_lzwDecodeStack[count - 1];
            while (count > 0)
                *dst++ = g_lzwDecodeStack[--count];
            LZW_DICT(g_lzwNextCode).parentCode = oldCode;
            LZW_DICT(g_lzwNextCode).character = (char)character;
            g_lzwNextCode++;
            oldCode = newCode;
        }
    }
}
