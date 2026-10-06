#pragma once

#include "BaseObject.h"

class UnknownTextureStream;

// Quantize.cpp (literal __FILE__ at 0x00572084, xrefs 0x004dde47..0x004de12e).
// RTTI: ColorMapper : BaseObject (vtable 0x00557658, four slots; slot 0 is
// the deleting destructor 0x004ddea0). A 256-colour palette loaded from a
// resource stream with its 555/565 lookup tables. TextureMap.h views the
// same object as UnknownTexturePalette. Member names are provisional.
class ColorMapper : public BaseObject {
public:
    explicit ColorMapper(UnknownTextureStream* stream); // 0x004ddf40 (ret 4)
    virtual ~ColorMapper();                              // 0x004de200 (deleting wrapper 0x004ddea0)

    // 0x004dddd0 (cdecl): the mapper loaded from resource `name`, shared
    // through the resource manager (AddRef on reuse); 0 if there is none.
    static ColorMapper* UnknownFunction4dddd0(const char* name);

    void UnknownFunction4ddec0();                        // 0x004ddec0: builds the 16-bit tables
    unsigned char* UnknownFunction4de270();              // 0x004de270: &field_0x10
    unsigned char* UnknownFunction4de280();              // 0x004de280: &field_0x710
    unsigned char* UnknownFunction4de290();              // 0x004de290: &field_0x8710

    int field_0x08;                     // first palette index used
    int field_0x0c;                     // number of palette entries
    unsigned char field_0x10[256][3];   // RGB entries
    unsigned short field_0x310[256];    // entries as 555
    unsigned short field_0x510[256];    // entries as 565
    unsigned char field_0x710[0x8000];  // 555 colour -> palette index
    unsigned char field_0x8710[0x10000]; // 565 colour -> palette index
    unsigned char field_0x18710[0x400];  // allocation is 0x18b10 bytes; not touched here
};
