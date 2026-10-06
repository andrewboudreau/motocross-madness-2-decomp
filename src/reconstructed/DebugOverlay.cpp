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
void* g_UnknownGlobal59ada4;

void UnknownFunction447910(void* window)
{
    g_UnknownGlobal59ada4 = window;
}

DebugOverlay::DebugOverlay(int flags)
    : GameObject(flags)
{
    field_0x26c0 = 0;
    field_0x26c4 = 0;
    field_0x36dc = 0;
    field_0x26c8 = 0;
    field_0x36d0 = 0;
    field_0xac = -1;
    field_0x26cc = 0.001f;
}

DebugOverlay::~DebugOverlay()
{
    if (field_0x36d8) {
        field_0x36d8->Release();
        field_0x36d8 = 0;
    }
}

void DebugOverlay::UnknownFunction447e90(int row, const char* text, int flag)
{
    if (row < field_0x26c8 && strcmp(field_0xb4[row].text, text) != 0) {
        int length = strlen(text);
        int n = length > 0x7f ? 0x7f : length;
        strncpy(field_0xb4[row].text, text, n);
        field_0xb4[row].text[n] = 0;
        field_0xb4[row].color = 0xffffff;
        field_0xb4[row].field_0x90 = flag;
    }
}

void DebugOverlay::UnknownFunction447f40(int page, const char* format, ...)
{
    if (field_0x25_bit0 && page == field_0x26c4 && field_0xac + 1 < field_0x26c8) {
        va_list args;
        va_start(args, format);
        vsprintf(field_0x2c, format, args);
        UnknownFunction447e90(++field_0xac, field_0x2c, 0);
    }
}

void DebugOverlay::UnknownFunction447fa0(int page, const char* format, ...)
{
    if (field_0x25_bit0 && page == field_0x26c4 && field_0xac + 1 < field_0x26c8) {
        va_list args;
        va_start(args, format);
        vsprintf(field_0x2c, format, args);
        UnknownFunction447e90(++field_0xac, field_0x2c, 1);
    }
}

void DebugOverlay::UnknownFunction448000(int page, int row, int flag)
{
    if (field_0x25_bit0 && page == field_0x26c4)
        field_0xb4[row].field_0x90 = flag;
}

int DebugOverlay::UnknownVirtualSlot14()
{
    field_0x36d8->UnknownVirtualSlot19();
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1c, 0, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 0x10, 1);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 0x11, 1);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 4);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 2, 2);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 3, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 2);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 5, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 1, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0xf, 0, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0xe, 0, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(7, 0, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(4, 0, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(8, 3, 0);
    unsigned int start = UnknownFunction4bfa80();
    int i;
    for (i = 0; i <= field_0xac; i++) {
        if (!field_0xb4[i].field_0x90)
            UnknownFunction448200(field_0xb4[i].x + field_0x26b8, field_0xb4[i].y + field_0x26bc,
                                  field_0xb4[i].text, strlen(field_0xb4[i].text), 0xa0ffffff);
    }
    for (i = 0; i <= field_0xac; i++) {
        if (field_0xb4[i].field_0x90)
            UnknownFunction448200(field_0xb4[i].x + field_0x26b8, field_0xb4[i].y + field_0x26bc,
                                  field_0xb4[i].text, strlen(field_0xb4[i].text), 0xc0ff0000);
    }
    field_0xac = -1;
    field_0x36dc = UnknownFunction4bfa80() - start;
    return 1;
}

void DebugOverlay::UnknownFunction448200(int x, int y, const char* text, int length,
                                         unsigned int color)
{
    DebugOverlayVertex vertices[192];
    float left = (float)x;
    float top = (float)y;
    float bottom = top + field_0x26b4;
    if (!(bottom < ((RenderTarget*)field_0x18)->field_0x10))
        return;
    int index = 0;
    while (length > 0) {
        int count = 0;
        while (count <= 0xba) {
            if (count >= length * 6)
                break;
            int ch = text[index];
            float right = (field_0x26d0[ch].u1 - field_0x26d0[ch].u0) * 256.0f + left;
            if (!(right < ((RenderTarget*)field_0x18)->field_0x0c)) {
                if (count)
                    ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(4, 0x1c4, (int)vertices, count, 0);
                return;
            }
            vertices[count].x = left;
            vertices[count].y = top;
            vertices[count].z = field_0x26cc;
            vertices[count].rhw = 1.0f;
            vertices[count].diffuse = color;
            vertices[count].specular = 0;
            vertices[count].tu = field_0x26d0[ch].u0;
            vertices[count].tv = field_0x26d0[ch].v0;
            vertices[count + 2].x = left;
            vertices[count + 2].y = bottom;
            vertices[count + 2].z = field_0x26cc;
            vertices[count + 2].rhw = 1.0f;
            vertices[count + 2].diffuse = color;
            vertices[count + 2].specular = 0;
            vertices[count + 2].tu = field_0x26d0[ch].u0;
            vertices[count + 2].tv = field_0x26d0[ch].v1;
            vertices[count + 1].x = right;
            vertices[count + 1].y = bottom;
            vertices[count + 1].z = field_0x26cc;
            vertices[count + 1].rhw = 1.0f;
            vertices[count + 1].diffuse = color;
            vertices[count + 1].specular = 0;
            vertices[count + 1].tu = field_0x26d0[ch].u1;
            vertices[count + 1].tv = field_0x26d0[ch].v1;
            vertices[count + 3].x = left;
            vertices[count + 3].y = top;
            vertices[count + 3].z = field_0x26cc;
            vertices[count + 3].rhw = 1.0f;
            vertices[count + 3].diffuse = color;
            vertices[count + 3].specular = 0;
            vertices[count + 3].tu = field_0x26d0[ch].u0;
            vertices[count + 3].tv = field_0x26d0[ch].v0;
            vertices[count + 5].x = right;
            vertices[count + 5].y = bottom;
            vertices[count + 5].z = field_0x26cc;
            vertices[count + 5].rhw = 1.0f;
            vertices[count + 5].diffuse = color;
            vertices[count + 5].specular = 0;
            vertices[count + 5].tu = field_0x26d0[ch].u1;
            vertices[count + 5].tv = field_0x26d0[ch].v1;
            vertices[count + 4].x = right;
            vertices[count + 4].y = top;
            vertices[count + 4].z = field_0x26cc;
            vertices[count + 4].rhw = 1.0f;
            vertices[count + 4].diffuse = color;
            vertices[count + 4].specular = 0;
            vertices[count + 4].tu = field_0x26d0[ch].u1;
            vertices[count + 4].tv = field_0x26d0[ch].v0;
            left = right;
            count += 6;
            index++;
        }
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(4, 0x1c4, (int)vertices, count, 0);
        ((RenderTarget*)field_0x18)->field_0x44 -= count / 3;
        length -= count / 6;
    }
}

int DebugOverlay::UnknownFunction4484f0()
{
    if (++field_0x26c4 >= field_0x26c0) {
        field_0x26c4 = 0;
        return 1;
    }
    return 0;
}

int DebugOverlay::UnknownVirtualSlot25(void* value)
{
    GameObject::UnknownVirtualSlot25(value);
    if (!field_0x36d8->UnknownFunction4c7420()) {
        field_0x36d8->Release();
        return UnknownFunction447a00();
    }
    return 1;
}
