#pragma once

#include "BaseObject.h"

class ColorMapper;
class UnknownTextureStream;
struct UnknownPaletteInterface;

// PALETTEENTRY-shaped (red, green, blue, flags); passed to
// GetSystemPaletteEntries and DirectDraw's CreatePalette.
struct UnknownPaletteEntry {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char flags;
};

// Palette8.cpp (literal __FILE__ at 0x0056f7cc). RTTI: Palette8 : BaseObject
// (vtable 0x00555aa4: the deleting destructor 0x004b6e80, then BaseObject's
// AddRef, Release and GetRefCount). An 8-bit palette read from a resource
// stream: the colours, a DirectDraw palette made from them and the
// ColorMapper of the same name. Member names are provisional.
class Palette8 : public BaseObject {
public:
    explicit Palette8(UnknownTextureStream* stream); // 0x004b6c00 (ret 4)
    virtual ~Palette8();                             // 0x004b6ea0 (deleting wrapper 0x004b6e80)

    // 0x004b6b30 (cdecl): the palette loaded from resource `name`, shared
    // through the resource manager (AddRef on reuse); 0 if there is none.
    static Palette8* UnknownFunction4b6b30(const char* name);

    unsigned char field_0x008[256][3];        // RGB entries from the stream
    UnknownPaletteEntry field_0x308[256];     // the DirectDraw palette's entries
    ColorMapper* field_0x708;                 // the mapper named in the stream
    UnknownPaletteInterface* field_0x70c;
    int field_0x710;                          // first entry; 10 keeps the system colours
    int field_0x714;                          // number of entries
};
