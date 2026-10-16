// DebugOverlay.cpp -- see DebugOverlay.h for the evidence. The font
// builder 0x00447a00 and the set-up 0x00447de0 are near misses in
// samples/render/DebugOverlayNearMisses.cpp.

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "DebugOverlay.h"
#include "Game.h"
#include "PCTextureMap.h"
#include "RenderTarget.h"
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

int DebugOverlay::UnknownVirtualSlot14()
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

int DebugOverlay::AttachRenderContext(void* value)
{
    GameObject::AttachRenderContext(value);
    if (!fontTexture->RestoreTextureSurface()) {
        fontTexture->Release();
        return LoadFont();
    }
    return 1;
}
