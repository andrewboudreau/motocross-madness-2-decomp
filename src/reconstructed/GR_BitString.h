#pragma once

// GR_BitString: RTTI-confirmed (`.?AVGR_BitString@@`, COL 0x0055ae08,
// primary vtable 0x00550f50 at object offset 0, single base GR_PixelString).
// Only the members Lzw.cpp uses are declared; their bodies sit at
// 0x00423d30..0x00423f6d. Member names are provisional.
//
// Layout (from the constructor 0x00423d50, 0x20 bytes):
//   +0x04 bit count, +0x08 buffer, +0x0c read cursor, +0x10 mask byte (0x80),
//   +0x14 bits consumed in the current word, +0x18 constructor flag (0/-1),
//   +0x1c 1: the buffer is not owned (the destructor frees it when 0).
class GR_PixelString {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1(int* value);
    virtual void UnknownVirtualSlot2(int value);
    virtual void UnknownVirtualSlot3();
    virtual ~GR_PixelString() {}
};

class GR_BitString : public GR_PixelString {
public:
    // 0x00423d50: wraps `bits` bits at `buffer`; byte-swaps the first word
    // into the reader's globals.
    GR_BitString(const void* buffer, int bits, int flag);
    // 0x00423dd0 (slot 4's scalar deleting destructor 0x00423d30 calls it).
    virtual ~GR_BitString();
    // 0x00423ef0: reads the next `count` bits, most significant first.
    unsigned int UnknownFunction423ef0(int count);

    int field_0x04;
    void* field_0x08;
    void* field_0x0c;
    unsigned char field_0x10;
    int field_0x14;
    int field_0x18;
    int field_0x1c;
};
