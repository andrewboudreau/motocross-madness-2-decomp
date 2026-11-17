// DebugOverlay.cpp -- see DebugOverlay.h for the evidence. The font
// builder 0x00447a00 is a near miss (see its status comment).

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "DebugOverlay.h"
#include "DebugAlloc.h"
#include "Game.h"
#include "PCTextureMap.h"
#include "RenderTarget.h"
#include "TrackGame.h"
#include "TrackOverlay.h"
#include "D3DConstants.h"

// A transformed, lit vertex (FVF 0x1c4: position and rhw, diffuse,
// specular, one texture coordinate pair), the D3DTLVERTEX layout.
struct DebugOverlayVertex {
    float x;
    float y;
    float z;
    float rhw;
    unsigned int diffuse;
    unsigned int specular;
    float tu;
    float tv;
};

// 0x00447910 (cdecl): stores the main window handle (the caller 0x004a0d30
// passes CreateWindowExA's result) in 0x0059ada4. Ownership is a bracket
// inference only (tier 3): it follows D3DIMSoultreeShadow.cpp's dynamic
// initialisers, which end that file as they end D3DIMSoultreeMotnctrl.cpp,
// and its global sits after D3DIMSoultreeShadow.cpp's data and before the
// keyboard-layout globals that precede dirlist.cpp's.
void* g_StoredMainWindow;

void StoreMainWindow(void* window)
{
    g_StoredMainWindow = window;
}

DebugOverlay::DebugOverlay(int flags)
    : GameObject(flags)
{
    nextPage = 0;
    shownPage = 0;
    drawTime = 0;
    rowLimit = 0;
    fontHeight = 0;
    pageRowCount = -1;
    vertexZ = 0.001f;
}

DebugOverlay::~DebugOverlay()
{
    if (fontTexture) {
        fontTexture->Release();
        fontTexture = 0;
    }
}

// 0x00447a00. Not exact (463 of 980 bytes: VC6 keeps one shared
// epilogue as retail does but places it after the first failure return
// instead of at the end;
// docs/DEBUGOVERLAY.md forms tried)
// Loads DebugOverlayText.tga, or builds the font texture with GDI: draws
// the 256 characters in Courier row by row into the system surface and
// records each glyph's texture rectangle.
int DebugOverlay::LoadFont()
{
    int c = 0;
    fontTexture = (PCTextureMap*)TerrainLoadTexture(g_TrackGame->textureManager,
                                                        "DebugOverlayText.tga", 1555, 0, 0, 5, 6,
                                                        0, 0x80, 0xff00ff, 1, 1);
    if (fontTexture == 0) {
        fontTexture = new(__FILE__, 59) PCTextureMap(textureManager, 1);
        fontTexture->CreateSurface(0, 0x100, 0x100, 0x100, 0x100, 0,
                                          ((RenderTarget*)renderContext)->surfacePixelFormat, 0, 8, 0, 1, 0,
                                          5, 6, 0, 0x80, 0xff00ff);
        void* bits = fontTexture->LockSurface(0, 0, 0);
        memset(bits, 0, PixelFormatBytesPerPixel(fontTexture->mapFormat) << 16);
        fontTexture->UnlockSurface(0);

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
        int copyLength = length > 31 ? 31 : length;
        strncpy(font.lfFaceName, "Courier", copyLength);
        font.lfFaceName[copyLength] = 0;
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
            float v0 = (top + 1) * (1.0f / 256.0f);
            float v1 = bottom * (1.0f / 256.0f);
            int x = 0;
            int count = 1;
            char* row = &characters[c];
            while (c < 256) {
                glyphs[c].u0 = x * (1.0f / 256.0f);
                glyphs[c].v0 = v0;
                glyphs[c].v1 = v1;
                GetTextExtentPoint32A(dc, row, count, &size);
                x = size.cx;
                if (x >= 256) {
                    char text[256];
                    strncpy(text, row, count - 1);
                    text[count - 1] = 0;
                    TextOutA(dc, 0, top, row, count - 1);
                    break;
                }
                glyphs[c].u1 = size.cx * (1.0f / 256.0f);
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
    if (fontTexture->CreateTextureSurface(1, 0, 0))
        return 1;
failed:
    return 0;
}

// 0x00447de0. `i` is set before `y` takes `top`, so the two cannot share
// a register: `top` stays in ebx until the loop, which then holds the row
// colour there, and edi is saved only on the success path.
DebugOverlay* DebugOverlay::Setup(RenderTarget* target, TextureMapManager* textures,
                                  int height, int left, int top)
{
    GameObject::SetRenderContext(target);
    fontHeight = height;
    rowsTop = top;
    rowsLeft = left;
    textureManager = textures;
    if (LoadFont()) {
        int i = 0;
        int y = top;
        do {
            rows[i].y = y;
            rows[i].bottom = lineHeight + y;
            rows[i].x = left;
            rows[i].right = 0x100;
            rows[i].text[0] = 0;
            rows[i].color = 0xffffff;
            i++;
            y += lineHeight;
        } while (lineHeight + y <= 0x280 && i < 64);
        rowLimit = i;
        return this;
    }
    Release();
    return 0;
}

void DebugOverlay::SetRowText(int row, const char* text, int flag)
{
    if (row < rowLimit && strcmp(rows[row].text, text) != 0) {
        int length = strlen(text);
        int n = length > 0x7f ? 0x7f : length;
        strncpy(rows[row].text, text, n);
        rows[row].text[n] = 0;
        rows[row].color = 0xffffff;
        rows[row].highlighted = flag;
    }
}

void DebugOverlay::Print(int page, const char* format, ...)
{
    if (isEnabled && page == shownPage && pageRowCount + 1 < rowLimit) {
        va_list args;
        va_start(args, format);
        vsprintf(rowText, format, args);
        SetRowText(++pageRowCount, rowText, 0);
    }
}

void DebugOverlay::Title(int page, const char* format, ...)
{
    if (isEnabled && page == shownPage && pageRowCount + 1 < rowLimit) {
        va_list args;
        va_start(args, format);
        vsprintf(rowText, format, args);
        SetRowText(++pageRowCount, rowText, 1);
    }
}

void DebugOverlay::SetRowHighlight(int page, int row, int flag)
{
    if (isEnabled && page == shownPage)
        rows[row].highlighted = flag;
}

int DebugOverlay::Render3D()
{
    fontTexture->Bind();
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_FOGENABLE, 0, 0);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, 0, 0);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0, 0);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ZENABLE, 0, 0);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0, 0);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID, 0);
    unsigned int start = ReadClock();
    int i;
    for (i = 0; i <= pageRowCount; i++) {
        if (!rows[i].highlighted)
            DrawString(rows[i].x + rowsLeft, rows[i].y + rowsTop,
                                  rows[i].text, strlen(rows[i].text), 0xa0ffffff);
    }
    for (i = 0; i <= pageRowCount; i++) {
        if (rows[i].highlighted)
            DrawString(rows[i].x + rowsLeft, rows[i].y + rowsTop,
                                  rows[i].text, strlen(rows[i].text), 0xc0ff0000);
    }
    pageRowCount = -1;
    drawTime = ReadClock() - start;
    return 1;
}

void DebugOverlay::DrawString(int x, int y, const char* text, int length,
                                         unsigned int color)
{
    DebugOverlayVertex vertices[192];
    float left = (float)x;
    float top = (float)y;
    float bottom = top + lineHeight;
    if (!(bottom < ((RenderTarget*)renderContext)->targetHeight))
        return;
    int index = 0;
    while (length > 0) {
        int count = 0;
        while (count <= 0xba) {
            if (count >= length * 6)
                break;
            int ch = text[index];
            float right = (glyphs[ch].u1 - glyphs[ch].u0) * 256.0f + left;
            if (!(right < ((RenderTarget*)renderContext)->targetWidth)) {
                if (count)
                    ((RenderTarget*)renderContext)->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, (int)vertices, count, 0);
                return;
            }
            vertices[count].x = left;
            vertices[count].y = top;
            vertices[count].z = vertexZ;
            vertices[count].rhw = 1.0f;
            vertices[count].diffuse = color;
            vertices[count].specular = 0;
            vertices[count].tu = glyphs[ch].u0;
            vertices[count].tv = glyphs[ch].v0;
            vertices[count + 2].x = left;
            vertices[count + 2].y = bottom;
            vertices[count + 2].z = vertexZ;
            vertices[count + 2].rhw = 1.0f;
            vertices[count + 2].diffuse = color;
            vertices[count + 2].specular = 0;
            vertices[count + 2].tu = glyphs[ch].u0;
            vertices[count + 2].tv = glyphs[ch].v1;
            vertices[count + 1].x = right;
            vertices[count + 1].y = bottom;
            vertices[count + 1].z = vertexZ;
            vertices[count + 1].rhw = 1.0f;
            vertices[count + 1].diffuse = color;
            vertices[count + 1].specular = 0;
            vertices[count + 1].tu = glyphs[ch].u1;
            vertices[count + 1].tv = glyphs[ch].v1;
            vertices[count + 3].x = left;
            vertices[count + 3].y = top;
            vertices[count + 3].z = vertexZ;
            vertices[count + 3].rhw = 1.0f;
            vertices[count + 3].diffuse = color;
            vertices[count + 3].specular = 0;
            vertices[count + 3].tu = glyphs[ch].u0;
            vertices[count + 3].tv = glyphs[ch].v0;
            vertices[count + 5].x = right;
            vertices[count + 5].y = bottom;
            vertices[count + 5].z = vertexZ;
            vertices[count + 5].rhw = 1.0f;
            vertices[count + 5].diffuse = color;
            vertices[count + 5].specular = 0;
            vertices[count + 5].tu = glyphs[ch].u1;
            vertices[count + 5].tv = glyphs[ch].v1;
            vertices[count + 4].x = right;
            vertices[count + 4].y = top;
            vertices[count + 4].z = vertexZ;
            vertices[count + 4].rhw = 1.0f;
            vertices[count + 4].diffuse = color;
            vertices[count + 4].specular = 0;
            vertices[count + 4].tu = glyphs[ch].u1;
            vertices[count + 4].tv = glyphs[ch].v0;
            left = right;
            count += 6;
            index++;
        }
        ((RenderTarget*)renderContext)->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, (int)vertices, count, 0);
        ((RenderTarget*)renderContext)->trianglesDrawn -= count / 3;
        length -= count / 6;
    }
}

int DebugOverlay::ShowNextPage()
{
    if (++shownPage >= nextPage) {
        shownPage = 0;
        return 1;
    }
    return 0;
}

int DebugOverlay::AttachRenderContext(void* context)
{
    GameObject::AttachRenderContext(context);
    if (!fontTexture->RestoreTextureSurface()) {
        fontTexture->Release();
        return LoadFont();
    }
    return 1;
}
