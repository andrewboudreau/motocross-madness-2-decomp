// Near-miss DebugOverlay.cpp candidates, kept out of src/reconstructed until
// they match (the exact members are in src/reconstructed/DebugOverlay.cpp).
//
// DebugOverlay::UnknownFunction447a00 (0x00447a00, 980 bytes; ours 989):
// the GDI font builder. The glyph loop is a top-tested `while (c < 256)`
// (a do-while lets VC6 peel the first u0 = 0 and reuse u1 as the next u0),
// and the GetDC/ReleaseDC/Slot8 failures share one `failed: return 0`.
// Zero in ebx, the run length in ebp and the glyph pointer in edi match.
// Left: VC6 here pushes ebp in the prologue and keeps `c` in ebp at the row
// head (retail reads it into eax once from its stack slot), which shifts the
// frame by 4; retail shares the row pointer's slot with the v0 temporary.
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
    fontTexture = (PCTextureMap*)UnknownFunction50a590(g_TrackGame->field_0x3c,
                                                        "DebugOverlayText.tga", 1555, 0, 0, 5, 6,
                                                        0, 0x80, 0xff00ff, 1, 1);
    if (fontTexture == 0) {
        fontTexture = new(__FILE__, 59) PCTextureMap(field_0x36d4, 1);
        fontTexture->UnknownVirtualSlot4(0, 0x100, 0x100, 0x100, 0x100, 0,
                                          ((RenderTarget*)field_0x18)->field_0x28, 0, 8, 0, 1, 0,
                                          5, 6, 0, 0x80, 0xff00ff);
        void* bits = fontTexture->UnknownVirtualSlot13(0, 0, 0);
        memset(bits, 0, UnknownFunction511970(fontTexture->field_0x20) << 16);
        fontTexture->UnknownVirtualSlot14(0);

        LOGFONT font;
        font.lfHeight = fontHeight;
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
        if (fontTexture->systemSurface->GetDC((void**)&dc))
            goto failed;
        SetBkColor(dc, 1);
        SetBkMode(dc, 1);
        HGDIOBJ previous = SelectObject(dc, handle);
        SetTextCharacterExtra(dc, 2);
        SetTextColor(dc, 0xffffff);
        SIZE size;
        GetTextExtentPoint32A(dc, characters, 256, &size);
        int top = 0;
        lineHeight = size.cy;
        for (int bottom = size.cy; bottom < 256; bottom += lineHeight) {
            if (c >= 256)
                break;
            char* row = &characters[c];
            int x = 0;
            int count = 1;
            float v0 = (top + 1) * (1.0f / 256.0f);
            float v1 = bottom * (1.0f / 256.0f);
            while (c < 256) {
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
            }
            top = bottom;
        }
        DeleteObject(handle);
        SelectObject(dc, previous);
        if (fontTexture->systemSurface->ReleaseDC(dc))
            goto failed;
    }
    if (!fontTexture->UnknownVirtualSlot8(1, 0, 0))
        goto failed;
    return 1;
failed:
    return 0;
}


DebugOverlay* DebugOverlay::UnknownFunction447de0(RenderTarget* a, TextureMapManager* b,
                                                  int c, int d, int e)
{
    GameObject::UnknownVirtualSlot8(a);
    fontHeight = c;
    rowsTop = e;
    rowsLeft = d;
    field_0x36d4 = b;
    if (UnknownFunction447a00()) {
        int y = e;
        int i = 0;
        do {
            field_0xb4[i].y = y;
            field_0xb4[i].bottom = lineHeight + y;
            field_0xb4[i].x = d;
            field_0xb4[i].right = 0x100;
            field_0xb4[i].text[0] = 0;
            field_0xb4[i].color = 0xffffff;
            i++;
            y += lineHeight;
        } while (lineHeight + y <= 0x280 && i < 64);
        rowLimit = i;
        return this;
    }
    Release();
    return 0;
}

