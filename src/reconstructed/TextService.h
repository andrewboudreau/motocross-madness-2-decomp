#pragma once

#include "OverlayRect.h"
#include "Rectangle2D.h"

template <class T> class EArray;
class FontTexture;
class RenderTarget;
class TextureMap;
class TextureMapManager;

// A pre-transformed, lit vertex (D3DTLVERTEX, 0x20 bytes) as passed to
// RenderTarget slot 16 with vertex format 0x1c4.
struct UnknownOverlayVertex {
    float sx;
    float sy;
    float sz;
    float rhw;
    unsigned int color;
    unsigned int specular;
    float tu;
    float tv;
};

// Pixel helpers (cdecl, among the collision code's addresses; names
// provisional). 0x0043b660 converts a 32-bit colour to texture format
// `format`; the others blend a font texel over a destination texel with
// `color`: 0x0043b560 32-bit, 0x0043b320 16-bit 4444, 0x0043b440 16-bit
// 1555.
unsigned int UnknownFunction43b660(unsigned int color, int format);
unsigned int UnknownFunction43b560(unsigned int texel, unsigned int dest, unsigned int color);
unsigned short UnknownFunction43b320(unsigned short texel, unsigned short dest, unsigned short color);
unsigned short UnknownFunction43b440(unsigned short texel, unsigned short dest, unsigned short color);

// TextService.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\TextService.cpp"
// at 0x00574b30; code 0x0050ac30-0x0050bed0). No vtable or RTTI, so the
// class name is not established: Overlay keeps one at +0x38 and
// TrackOverlay.cpp calls it by this name. 0x122c bytes (0x0050ae80
// allocates it): a list of fonts, the selected one and room for 24
// characters of laid-out quads. Names are provisional.
class UnknownOverlayText {
public:
    // 0x0050ac30: room for `count` fonts; the rectangle (x, y)-(x + width,
    // y + height).
    UnknownOverlayText(unsigned int count, float x, float y, float width, float height);
    ~UnknownOverlayText();                    // 0x0050ad30: releases the fonts

    // 0x0050ae80 (cdecl): adds the archive `archive`, then loads the
    // `count` fonts `names` (cell file "<name>.cell", texture
    // "<name>.tga") and selects the first.
    static UnknownOverlayText* UnknownFunction50ae80(const char* archive, const char** names, int count,
                                                     int format, TextureMapManager* manager, float x,
                                                     float y, float width, float height, int a10);

    void UnknownFunction50ade0(const char* font); // 0x0050ade0: selects the font named `font`
    void UnknownFunction50ae60(unsigned int index, FontTexture* font); // 0x0050ae60

    // 0x0050b080: draws `text` at (x, y) into `texture`, wrapping at its
    // width; returns the x past its end. 0x0050b700 (format 0x613) and
    // 0x0050b400 (format 0x115c) are the 16-bit versions; they take the
    // colour already converted.
    int UnknownFunction50b080(TextureMap* texture, int x, int y, const char* text, int color);
    int UnknownFunction50b400(TextureMap* texture, int x, int y, const char* text, int color);
    int UnknownFunction50b700(TextureMap* texture, int x, int y, const char* text, int color);

    // 0x0050ba00: lays `text` out from the top left of `rect` as two
    // triangles per character; returns the vertices, their count in *count.
    void* UnknownFunction50ba00(const UnknownOverlayRect* rect, const char* text,
                                unsigned int color, int* count);
    // 0x0050bd30: draws vertices laid out by 0x0050ba00.
    void UnknownFunction50bd30(RenderTarget* target, void* vertices, int count);

    FontTexture* field_0x00;                  // selected font
    EArray<FontTexture*>* field_0x04;         // fonts
    float field_0x08;                         // 0.2
    float field_0x0c;                         // 0.001: vertex depth
    Rectangle2D field_0x10;
    UnknownOverlayVertex field_0x2c[24 * 6];
};
