#pragma once

// vfwdeco.cpp (literal __FILE__ at 0x005759cc, three xrefs inside
// 0x0052d050). The retail TU contributes only the destructor below; the
// object lives at PCTextureMap+0x7c. Names and field types are provisional.
struct HIC__;                                 // vfw.h's HIC handle type

class UnknownVideoDecoder {
public:
    ~UnknownVideoDecoder();                   // 0x0052d050

    void* field_0x00;                         // debug-allocated buffer
    unsigned char field_0x04[0x10 - 0x04];
    void* field_0x10;                         // debug-allocated buffer
    void* field_0x14;                         // debug-allocated buffer
    HIC__* field_0x18;                        // HIC (MSVFW32 compressor handle)
};
