#pragma once

// Nonpolymorphic 0x101c-byte object whose code is attributed by the retail
// D:\aardvark\VC\krusty2\VCR.cpp literal (0x00575848). recorder.cpp code
// allocates it with operator new(0x101c) and constructs it at 0x004e7ad9,
// and destroys and frees it at 0x004e79b0. It has no vtable or RTTI, so the
// class name and every member name are provisional.
//
// The object is a ring of 256 16-byte buffer slots with three cursors, plus
// a chained list of 0x804-byte blocks of (value, extra) pairs.
struct UnknownVcrSlot {
    void* data;                 // +0x00, DebugMalloc'd buffer
    unsigned int size;          // +0x04, capacity recorded for data
    int field_0x08;
    char field_0x0c;            // set to 1 while the slot is handed out
    char field_0x0d;            // state: 1, 2 and 3 are tested
};

struct UnknownVcrBlock {
    int pairs[256][2];          // +0x000
    UnknownVcrBlock* next;      // +0x800
};

class UnknownVcr {
public:
    UnknownVcr(unsigned int size);                                          // 0x00524110
    ~UnknownVcr();                                                          // 0x005241b0
    int UnknownFunction524220(unsigned int size, int* index, void** data); // 0x00524220
    void UnknownFunction5242e0(int index);                                  // 0x005242e0
    void UnknownFunction524300(void** data, int* size, int* index);         // 0x00524300
    int UnknownFunction524350();                                            // 0x00524350
    int UnknownFunction524390(unsigned int size, int* index, void** data); // 0x00524390
    void UnknownFunction524440(int value, int index);                       // 0x00524440
    int UnknownFunction524460(void** data, unsigned int* size, int* value, int* index); // 0x00524460
    void UnknownFunction524540(int index);                                  // 0x00524540
    int UnknownFunction524560();                                            // 0x00524560
    void UnknownFunction524590(int value, int extra);                       // 0x00524590
    void UnknownFunction524640();                                           // 0x00524640
    int UnknownFunction5246c0(int* extra);                                  // 0x005246c0
    int UnknownFunction5247c0(int* extra);                                  // 0x005247c0
    void UnknownFunction524870(int value);                                  // 0x00524870
    void UnknownFunction5248f0(int value);                                  // 0x005248f0
    int UnknownFunction524900();                                            // 0x00524900
    void UnknownFunction524910();                                           // 0x00524910
    void UnknownFunction524940();                                           // 0x00524940
    void UnknownFunction524970();                                           // 0x00524970
    int UnknownFunction5249a0();                                            // 0x005249a0
    int UnknownFunction5249d0();                                            // 0x005249d0
    int UnknownFunction524a00();                                            // 0x00524a00
    int UnknownFunction524a30(int index);                                   // 0x00524a30

    UnknownVcrSlot slots[256];          // +0x0000
    int field_0x1000;                   // cursor advanced by 0x005249a0
    int field_0x1004;                   // cursor advanced by 0x005249d0
    int field_0x1008;                   // cursor advanced by 0x00524a00
    UnknownVcrBlock* field_0x100c;      // first pair block
    int field_0x1010;                   // pair index in the current block
    int field_0x1014;                   // current block number
    int field_0x1018;
};
