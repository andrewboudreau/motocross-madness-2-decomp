// Near-miss DebugOverlay.cpp candidates, kept out of src/reconstructed until
// they match (the exact members are in src/reconstructed/DebugOverlay.cpp).
//
// DebugOverlay::UnknownFunction447a00 (0x00447a00, 980 bytes; ours 1027):
// the GDI font builder. The calls, LOGFONT fields (stored in retail's
// order), glyph table writes and control flow follow retail, but VC6
// allocates registers differently: retail keeps the zero constant in ebx,
// the glyph index `c`, `x` and `top` in stack slots and the run length in
// ebp (pushed only around the GDI section); here ebp holds zero for the
// whole function and `c` lives in ebx, which also grows the frame
// (0x264 vs 0x26c) and turns the byte stores into immediates.
//
// DebugOverlay::UnknownFunction447de0 (0x00447de0, 174 bytes): 66/171 by
// position. Retail keeps the row colour 0xffffff in ebx inside the row loop
// (and `e` in ebx before it, pushing edi late); VC6 here stores the
// immediate and keeps `e` in edi. do/while, for, while(1), pointer rows,
// a named colour local, an unsigned literal and an inline packer were
// tried.
#include <windows.h>
#include <string.h>

#include "../../src/reconstructed/DebugOverlay.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/RenderTarget.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/TrackOverlay.h"

int DebugOverlay::UnknownFunction447a00()
{
    int c = 0;
    field_0x36d8 = (PCTextureMap*)UnknownFunction50a590(g_UnknownGlobal56e26c->field_0x3c,
                                                        "DebugOverlayText.tga", 0x613, 0, 0, 5, 6,
                                                        0, 0x80, 0xff00ff, 1, 1);
    if (field_0x36d8 == 0) {
        field_0x36d8 = new(__FILE__, 59) PCTextureMap(field_0x36d4, 1);
        field_0x36d8->UnknownVirtualSlot4(0, 0x100, 0x100, 0x100, 0x100, 0,
                                          ((RenderTarget*)field_0x18)->field_0x28, 0, 8, 0, 1, 0,
                                          5, 6, 0, 0x80, 0xff00ff);
        void* bits = field_0x36d8->UnknownVirtualSlot13(0, 0, 0);
        memset(bits, 0, UnknownFunction511970(field_0x36d8->field_0x20) << 16);
        field_0x36d8->UnknownVirtualSlot14(0);

        LOGFONT font;
        font.lfHeight = field_0x36d0;
        font.lfWidth = 0;
        font.lfEscapement = 0;
        font.lfOrientation = 0;
        font.lfUnderline = 0;
        font.lfStrikeOut = 0;
        font.lfCharSet = 0;
        font.lfOutPrecision = 0;
        font.lfClipPrecision = 0;
        font.lfQuality = 2;
        font.lfPitchAndFamily = 2;
        font.lfItalic = 0;
        font.lfWeight = 0;
        int length = strlen("Courier");
        int n = length > 31 ? 31 : length;
        strncpy(font.lfFaceName, "Courier", n);
        font.lfFaceName[n] = 0;
        HFONT handle = CreateFontIndirectA(&font);
        if (handle == 0) {
            Release();
            return 0;
        }

        char characters[256];
        for (int i = 0; i < 256; i++)
            characters[i] = (char)i;
        HDC dc;
        if (field_0x36d8->field_0x70->UnknownMethod17((void**)&dc))
            return 0;
        SetBkColor(dc, 1);
        SetBkMode(dc, 1);
        HGDIOBJ previous = SelectObject(dc, handle);
        SetTextCharacterExtra(dc, 2);
        SetTextColor(dc, 0xffffff);
        SIZE size;
        GetTextExtentPoint32A(dc, characters, 256, &size);
        int top = 0;
        field_0x26b4 = size.cy;
        for (int bottom = size.cy; bottom < 256; bottom += field_0x26b4) {
            if (c >= 256)
                break;
            char* row = &characters[c];
            int x = 0;
            int count = 1;
            float v0 = (top + 1) * (1.0f / 256.0f);
            float v1 = bottom * (1.0f / 256.0f);
            do {
                field_0x26d0[c].u0 = x * (1.0f / 256.0f);
                field_0x26d0[c].v0 = v0;
                field_0x26d0[c].v1 = v1;
                GetTextExtentPoint32A(dc, row, count, &size);
                x = size.cx;
                if (x >= 256) {
                    char text[256];
                    strncpy(text, row, count - 1);
                    text[count - 1] = 0;
                    TextOutA(dc, 0, top, row, count - 1);
                    break;
                }
                field_0x26d0[c].u1 = size.cx * (1.0f / 256.0f);
                count++;
                c++;
            } while (c < 256);
            top = bottom;
        }
        DeleteObject(handle);
        SelectObject(dc, previous);
        if (field_0x36d8->field_0x70->UnknownMethod26(dc))
            return 0;
    }
    if (field_0x36d8->UnknownVirtualSlot8(1, 0, 0))
        return 1;
    return 0;
}


DebugOverlay* DebugOverlay::UnknownFunction447de0(RenderTarget* a, TextureMapManager* b,
                                                  int c, int d, int e)
{
    GameObject::UnknownVirtualSlot8(a);
    field_0x36d0 = c;
    field_0x26bc = e;
    field_0x26b8 = d;
    field_0x36d4 = b;
    if (UnknownFunction447a00()) {
        int y = e;
        int i = 0;
        do {
            field_0xb4[i].y = y;
            field_0xb4[i].bottom = field_0x26b4 + y;
            field_0xb4[i].x = d;
            field_0xb4[i].right = 0x100;
            field_0xb4[i].text[0] = 0;
            field_0xb4[i].color = 0xffffff;
            i++;
            y += field_0x26b4;
        } while (field_0x26b4 + y <= 0x280 && i < 64);
        field_0x26c8 = i;
        return this;
    }
    Release();
    return 0;
}

