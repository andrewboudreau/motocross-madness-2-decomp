#pragma once

// RTTI: Point2D (no bases; vtable 0x00556f78, one slot: the deleting
// destructor 0x004d28d0, which calls 0x004d2910). 0xc bytes: two floats.
// Names are provisional.
class Point2D {
public:
    Point2D(float x, float y);                   // 0x004d28b0
    Point2D(const Point2D& other);               // 0x004d28f0
    virtual ~Point2D();                          // 0x004d2910

    float x;
    float y;
};

// RTTI: Rectangle2D (vtable 0x005577fc, one slot: the deleting destructor
// 0x004e8b40). 0x1c bytes: two points at +0x04 and +0x10, built by
// 0x004d28b0 from two values and copied by 0x004d28f0. Its code sits among
// ResourceManager.cpp's literals. Only what FontTexture.cpp and
// TextService.cpp use is declared; names are provisional.
class Rectangle2D {
public:
    Rectangle2D(float x1, float y1, float x2, float y2); // 0x004e8ad0: points (x1, y1) and (x2, y2)
    Rectangle2D(const Rectangle2D& other);       // 0x004e8b60
    virtual ~Rectangle2D();                      // 0x004e8bc0
    Rectangle2D& operator=(const Rectangle2D& other); // 0x004e8c20

    // Inline (TextService.cpp's constructor addresses the rectangle once).
    void UnknownSet(float x1, float y1, float x2, float y2) {
        field_0x04.x = x1;
        field_0x04.y = y1;
        field_0x10.x = x2;
        field_0x10.y = y2;
    }

    Point2D field_0x04;                          // (x1, y1)
    Point2D field_0x10;                          // (x2, y2)
};

// Globals holding 0 (0x0057972c, .bss), 1 (0x00568468), 2 (0x0056846c) and
// 3 (0x00568470): FontTexture.cpp reads them as indices into a four-value
// rectangle record (x1, y1, x2, y2 order presumed).
extern int g_UnknownGlobal57972c;
extern int g_UnknownGlobal568468;
extern int g_UnknownGlobal56846c;
extern int g_UnknownGlobal568470;
