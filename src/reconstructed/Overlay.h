#pragma once

#include "GameObject.h"
#include "RenderTarget.h"
#include "TextureMap.h"

// RTTI: Overlay : GameObject : BaseObject (vtable 0x00555a28). Its code cites
// D:\aardvark\VC\krusty2\overlay.cpp (0x0056f758) and OverlayIconService.h;
// it is not reconstructed. Only what TrackOverlay.cpp's derived classes use
// is declared; names are provisional. Overlay introduces no slots: it
// overrides the destructor, slot 10 (0x00499ae0) and slot 14 (0x004b61a0).

// A screen rectangle as Overlay keeps it (+0xe8, +0xf8).
struct UnknownOverlayRect {
    int left;
    int top;
    int right;
    int bottom;
};

inline UnknownOverlayRect UnknownMakeOverlayRect(int left, int top, int right, int bottom)
{
    UnknownOverlayRect rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}

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

class Overlay : public GameObject {
public:
    // 0x004b5e20: GameObject(flags); `value` is kept at +0x118.
    Overlay(int flags, int value);
    virtual ~Overlay();                       // 0x004b5ec0 (deleting wrapper 0x004b5ea0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00499ae0
    virtual int UnknownVirtualSlot14();       // 0x004b61a0: draws the overlay quad

    // 0x004b5f50: binds the render target (slot 8), the texture (+0x2c), the
    // screen rectangle (+0xe8) and an optional source rectangle (+0xf8).
    Overlay* UnknownFunction4b5f50(RenderTarget* target, TextureMap* texture,
                                   const UnknownOverlayRect* rect, int a4,
                                   const UnknownOverlayRect* source, float depth,
                                   char a7, int a8, int a9, int a10, int a11, int a12);

    RenderTarget* Target() const { return (RenderTarget*)field_0x18; }

    TextureMap* field_0x2c;                   // released by derived destructors
    BaseObject* field_0x30;
    BaseObject* field_0x34;
    void* field_0x38;
    BaseObject* field_0x3c;
    int field_0x40;
    UnknownOverlayVertex field_0x44[5];       // cleared by the constructor
    int field_0xe4;                           // 1
    UnknownOverlayRect field_0xe8;            // screen rectangle
    UnknownOverlayRect field_0xf8;            // source rectangle
    int field_0x108;                          // has a source rectangle
    int field_0x10c;                          // alpha, 0xff
    int field_0x110;
    float field_0x114;                        // depth
    int field_0x118;                          // constructor value
};
