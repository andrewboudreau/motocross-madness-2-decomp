#pragma once

#include "GameObject.h"

class PCTextureMap;
class RenderTarget;
class TextureMapManager;

// DebugOverlay.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\DebugOverlay.cpp"
// at 0x00568d04, one xref 0x00447a67: the debug `new` at line 59).
// Code 0x00447910 (provisional, see UnknownFunction447910) or 0x00447920
// ..0x00448559, after D3DIMSoultreeShadow.cpp's initialisers and before the unattributed code from 0x00448560 (callers outside this
// file only, no source literal) that runs up to dirlist.cpp at 0x00449e60.
//
// Confirmed (tier 1): RTTI .?AVDebugOverlay@@ (COL 0x0055b460), single
// non-virtual base GameObject; vtable 0x0055166c (27 slots), overriding
// slot 0 (deleting destructor 0x00447970), slot 14 (0x00448030, draws the
// rows) and slot 25 (0x00448520, rebuilds a lost font texture). The
// constructor 0x00447920 and the destructor 0x00447990 write the vptr.
// 0x36e0 bytes (the size Game's initialiser allocates). Member and method
// names are provisional (tier 3).

// 0x00447910 (cdecl): sets the window handle global 0x0059ada4; see
// DebugOverlay.cpp for the (provisional) ownership.
extern void* g_UnknownGlobal59ada4;
void UnknownFunction447910(void* window);

// One text row, 0x98 bytes from +0xb4 (stride and fields from 0x00447de0,
// 0x00447e90 and slot 14).
struct DebugOverlayRow {
    int x;                                    // left, relative to field_0x26b8
    int y;                                    // top, relative to field_0x26bc
    int right;                                // 0x100
    int bottom;                               // y + line height
    char text[0x80];
    int highlighted;                          // drawn in the second (red) pass when set
    unsigned int color;                       // 0xffffff
};

// A character cell of the font texture in texture coordinates, 0x10 bytes
// from +0x26d0, indexed by the (signed) character.
struct DebugOverlayGlyph {
    float u0;
    float v0;
    float u1;
    float v1;
};

class DebugOverlay : public GameObject {
public:
    explicit DebugOverlay(int flags);                                   // 0x00447920
    virtual ~DebugOverlay();                  // 0x00447990 (deleting wrapper 0x00447970)
    virtual int UnknownVirtualSlot14();       // 0x00448030
    virtual int UnknownVirtualSlot25(void* value); // 0x00448520

    // 0x00447de0: sets the overlay up; returns it, or 0.
    DebugOverlay* UnknownFunction447de0(RenderTarget* a, TextureMapManager* b,
                                        int c, int d, int e);
    // 0x00447a00: loads "DebugOverlayText.tga", or renders the font
    // ("Courier", height field_0x36d0) into a new 256x256 texture through GDI.
    int UnknownFunction447a00();
    // 0x00447e90 (ret 0xc): sets the text of row `row` when it changed.
    void SetRowText(int row, const char* text, int flag);
    int UnknownFunction4484f0();                                        // 0x004484f0: next page
    // 0x00447f40 and 0x00447fa0: format a row onto page `page` when that page
    // is shown (+0x25 bit 0 and +0x26c4); cdecl, `this` on the stack.
    void UnknownFunction447f40(int page, const char* format, ...);
    void UnknownFunction447fa0(int page, const char* format, ...);
    // 0x00448000 (ret 0xc): sets field_0x90 of row `row` on a shown page.
    void UnknownFunction448000(int page, int row, int flag);
    // 0x00448200 (ret 0x14): draws `length` characters of `text` at (x, y).
    void DrawString(int x, int y, const char* text, int length, unsigned int color);
    int NewPage() { return nextPage++; }

    char rowText[0x80];                       // formatted row
    int pageRowCount;                         // rows on the shown page
    int field_0xb0;
    DebugOverlayRow field_0xb4[64];
    int lineHeight;                           // line height
    int rowsLeft;                             // left of the rows
    int rowsTop;                              // top of the rows
    int nextPage;                             // next page number
    int field_0x26c4;                         // page shown
    int rowLimit;                             // row limit
    float vertexZ;                            // vertex z, 0.001
    DebugOverlayGlyph field_0x26d0[256];
    int fontHeight;                           // font height
    TextureMapManager* field_0x36d4;
    PCTextureMap* fontTexture;                // font texture
    int field_0x36dc;                         // Game slot 8: "[this overlay %d]"
};
