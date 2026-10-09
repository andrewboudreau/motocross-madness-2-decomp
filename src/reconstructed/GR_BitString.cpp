// GR_BitString's translation unit (the file name is not recovered; the code
// has no __FILE__ reference). Extent 0x004238c0..0x00423f6d, strong
// inference: it follows bikerace.cpp's last function (0x004238bb) and
// precedes BlockAllocator.cpp (0x00423f70); its .data (0x00567f90..
// 0x00568227, then the GR_PixelString/GR_BitString type descriptors) sits
// between bikerace.cpp's and BlockAllocator.cpp's literals, and its .bss
// (0x00578e90..0x00578e9b) is referenced only from this range. No `$E`
// initializers: every table is statically initialized.
//
// Not reconstructed: the reader 0x00423ef0 (Lzw.cpp's caller binds it). It
// has an ebp frame, a `bswap` and the `mov [ebp-4], esi` / `mov eax,
// [ebp-4]` return of a VC6 __asm body, so it is assumed to be inline
// assembly. The shifts 0x004239c0 and 0x00423b70 are near misses
// in samples/render/GR_BitStringNearMisses.cpp.

#include "GR_BitString.h"

// The words are stored big-endian: the first byte in memory holds the first
// (most significant) eight bits. The shifts swap each word into host order,
// shift it and swap it back.
#define UNKNOWN_BIG_ENDIAN_WORD(x)                                                            \
    ((((((unsigned int)((unsigned char*)&(x))[0] << 8) | ((unsigned char*)&(x))[1]) << 8)     \
      | ((unsigned char*)&(x))[2]) << 8 | ((unsigned char*)&(x))[3])

// Reverses the byte order of a 32-bit value.
#define UNKNOWN_SWAP_BYTES(x) (((x) >> 24) | (((x) >> 8) & 0xff00) | (((x) << 8) & 0xff0000) | ((x) << 24))

// 0x00567f90: byte masks of the top 1..8 bits (no reference found).
unsigned char g_UnknownByteMasks567f90[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};

// 0x00567f98: each byte with its bit order reversed.
unsigned char g_UnknownBitReverse567f98[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

// 0x00568098: the top 0..32 bits of a big-endian word, as stored in memory.
unsigned int g_UnknownHighMasks568098[33] = {
    0x00000000, 0x00000080, 0x000000c0, 0x000000e0, 0x000000f0, 0x000000f8, 0x000000fc, 0x000000fe,
    0x000000ff, 0x000080ff, 0x0000c0ff, 0x0000e0ff, 0x0000f0ff, 0x0000f8ff, 0x0000fcff, 0x0000feff,
    0x0000ffff, 0x0080ffff, 0x00c0ffff, 0x00e0ffff, 0x00f0ffff, 0x00f8ffff, 0x00fcffff, 0x00feffff,
    0x00ffffff, 0x80ffffff, 0xc0ffffff, 0xe0ffffff, 0xf0ffffff, 0xf8ffffff, 0xfcffffff, 0xfeffffff,
    0xffffffff,
};

// 0x0056811c: bit 0..31 of a big-endian word, as stored in memory, then 0
// (no reference found).
unsigned int g_UnknownBitMasks56811c[33] = {
    0x00000080, 0x00000040, 0x00000020, 0x00000010, 0x00000008, 0x00000004, 0x00000002, 0x00000001,
    0x00008000, 0x00004000, 0x00002000, 0x00001000, 0x00000800, 0x00000400, 0x00000200, 0x00000100,
    0x00800000, 0x00400000, 0x00200000, 0x00100000, 0x00080000, 0x00040000, 0x00020000, 0x00010000,
    0x80000000, 0x40000000, 0x20000000, 0x10000000, 0x08000000, 0x04000000, 0x02000000, 0x01000000,
    0x00000000,
};

// 0x005681a0: the low 0..32 bits of a host-order word.
unsigned int g_UnknownLowMasks5681a0[33] = {
    0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff,
};

// 0x00568224: the table UnknownFunction423ef0 reads its masks through.
unsigned int* g_UnknownLowMaskTable568224 = g_UnknownLowMasks5681a0;

// The sequential reader's state (0x00578e90..0x00578e9b): the current word in
// host order, the bits of it consumed, and the word's address.
unsigned int g_UnknownReadWord578e90;
int g_UnknownReadBits578e94;
unsigned int* g_UnknownReadCursor578e98;

// 0x004238c0
void GR_BitString::UnknownFunction4238c0(int count) {
    int remaining;
    int shiftWords;
    int words;
    unsigned int* dst;
    unsigned int* src;

    if (count >= 32) {
        words = (field_0x04 + 31) / 32;
        shiftWords = count / 32;
        count -= shiftWords * 32;
        remaining = words - shiftWords;
        if (remaining > 0) {
            dst = (unsigned int*)field_0x08;
            src = dst + shiftWords;
            do {
                *dst++ = *src++;
            } while (--remaining);
            do {
                *dst++ = field_0x18;
            } while (--shiftWords);
        } else {
            dst = (unsigned int*)field_0x08;
            remaining = words;
            do {
                *dst++ = field_0x18;
            } while (--remaining);
        }
    }
    if (count > 0)
        UnknownFunction4239c0(count);
}

// 0x00423940
void GR_BitString::UnknownFunction423940(int count) {
    int words;
    int shiftWords;
    int remaining;
    unsigned int* dst;
    unsigned int* src;

    if (count >= 32) {
        words = (field_0x04 + 31) / 32;
        shiftWords = count / 32;
        count -= shiftWords * 32;
        remaining = words - shiftWords;
        if (remaining > 0) {
            src = (unsigned int*)field_0x08 + remaining;
            dst = (unsigned int*)field_0x08 + words;
            do {
                *--dst = *--src;
            } while (--remaining);
            do {
                *--dst = field_0x18;
            } while (--shiftWords);
        } else {
            dst = (unsigned int*)field_0x08;
            do {
                *dst++ = field_0x18;
            } while (--words);
        }
    }
    if (count > 0)
        UnknownFunction423b70(count);
}

// 0x00423d50
GR_BitString::GR_BitString(const void* buffer, int bits, int flag) {
    field_0x04 = bits;
    field_0x18 = flag ? -1 : 0;
    field_0x08 = (void*)buffer;
    field_0x1c = 1;
    field_0x0c = (void*)buffer;
    field_0x10 = 0x80;
    field_0x14 = 0;
    g_UnknownReadCursor578e98 = (unsigned int*)buffer;
    g_UnknownReadBits578e94 = 0;
    g_UnknownReadWord578e90 = UNKNOWN_SWAP_BYTES(*g_UnknownReadCursor578e98);
}

// 0x00423dd0
GR_BitString::~GR_BitString() {
    if (!field_0x1c && field_0x08)
        delete field_0x08;
}

// 0x00423df0
void GR_BitString::UnknownVirtualSlot1(int* value) {
    unsigned char* front;
    unsigned char* back;
    unsigned char c;
    int bytes;
    int pairs;

    front = (unsigned char*)field_0x08;
    if (front && field_0x04) {
        bytes = (field_0x04 + 7) / 8;
        pairs = bytes / 2;
        back = (unsigned char*)field_0x08 + bytes;
        do {
            c = g_UnknownBitReverse567f98[*front];
            *front++ = g_UnknownBitReverse567f98[*--back];
            *back = c;
        } while (--pairs);
        *value = -((8 - field_0x04 % 8) % 8);
    }
}

// 0x00423e70
void GR_BitString::UnknownVirtualSlot0() {
    int shift;

    UnknownVirtualSlot1(&shift);
    UnknownVirtualSlot2(shift);
}

// 0x00423e90
void GR_BitString::UnknownVirtualSlot2(int value) {
    if (field_0x08 && field_0x04) {
        if (value < 0)
            UnknownFunction4238c0(-value);
        else if (value > 0)
            UnknownFunction423940(value);
    }
}

// 0x00423ec0
void GR_BitString::UnknownVirtualSlot3() {
    unsigned int* p;
    int words;

    if (field_0x08 && field_0x04) {
        words = (field_0x04 + 31) / 32;
        p = (unsigned int*)field_0x08;
        do {
            *p = ~*p;
            p++;
        } while (--words);
    }
}
