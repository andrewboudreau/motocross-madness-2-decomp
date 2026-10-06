#pragma once

// GR_BitString: RTTI-confirmed (`.?AVGR_BitString@@`, COL 0x0055ae08,
// primary vtable 0x00550f50 at object offset 0, single base GR_PixelString).
// The class's code is 0x004238c0..0x00423f6d (GR_BitString.cpp; see the
// TU notes there). Member names are provisional. GR_PixelString has no
// vtable or constructor in retail; only its four slots are known.
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
};

class GR_BitString : public GR_PixelString {
public:
    // 0x00423d50: wraps `bits` bits at `buffer`; byte-swaps the first word
    // into the reader's globals.
    GR_BitString(const void* buffer, int bits, int flag);
    // 0x00423dd0 (slot 4's scalar deleting destructor 0x00423d30 calls it).
    virtual ~GR_BitString();
    // Slot 0 (0x00423e70): reverses the bit order, then drops the padding.
    virtual void UnknownVirtualSlot0();
    // Slot 1 (0x00423df0): reverses the bytes and their bits; *value
    // receives minus the padding bits (0..7) now at the front.
    virtual void UnknownVirtualSlot1(int* value);
    // Slot 2 (0x00423e90): shifts toward the front (value < 0) or the back.
    virtual void UnknownVirtualSlot2(int value);
    // Slot 3 (0x00423ec0): inverts every 32-bit word.
    virtual void UnknownVirtualSlot3();
    // 0x004238c0 / 0x00423940: shift by `count` bits toward the front / the
    // back, whole words first, then 0x004239c0 / 0x00423b70 for the rest.
    void UnknownFunction4238c0(int count);
    void UnknownFunction423940(int count);
    void UnknownFunction4239c0(int count);
    void UnknownFunction423b70(int count);
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
