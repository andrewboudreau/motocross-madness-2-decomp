#pragma once

// RTTI: Rectangle2D (vtable 0x005577fc, one slot: the deleting destructor
// 0x004e8b40). 0x1c bytes: two 0xc-byte points (vtable 0x00556f78; built by
// 0x004d28b0 from two values, copied by 0x004d28f0) at +0x04 and +0x10. Its
// code sits among ResourceManager.cpp's literals. Only what FontTexture.cpp
// uses is declared; names are provisional.
class Rectangle2D {
public:
    Rectangle2D(float x1, float y1, float x2, float y2); // 0x004e8ad0: points (x1, y1) and (x2, y2)
    Rectangle2D(const Rectangle2D& other);       // 0x004e8b60
    virtual ~Rectangle2D();                      // 0x004e8bc0

    unsigned char field_0x04[0x1c - 0x04];
};

// Globals holding 0 (0x0057972c, .bss), 1 (0x00568468), 2 (0x0056846c) and
// 3 (0x00568470): FontTexture.cpp reads them as indices into a four-value
// rectangle record (x1, y1, x2, y2 order presumed).
extern int g_UnknownGlobal57972c;
extern int g_UnknownGlobal568468;
extern int g_UnknownGlobal56846c;
extern int g_UnknownGlobal568470;
