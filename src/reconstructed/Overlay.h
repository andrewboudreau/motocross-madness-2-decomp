#pragma once

#include "GameObject.h"
#include "OverlayIconService.h"
#include "OverlayRect.h"
#include "RenderTarget.h"
#include "TextService.h"
#include "TextureMap.h"

// RTTI: Overlay : GameObject : BaseObject (vtable 0x00555a28). Its code cites
// D:\aardvark\VC\krusty2\overlay.cpp (0x0056f758) and OverlayIconService.h;
// Overlay.cpp reconstructs overlay.cpp (0x004b5e20-0x004b6b20). Names are
// provisional. Overlay introduces no slots: it
// overrides the destructor, slot 10 (0x00499ae0) and slot 14 (0x004b61a0).

// UnknownOverlayVertex and the text renderer at Overlay+0x38
// (UnknownOverlayText) are declared in TextService.h.

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
                                   char a7, const char* a8, const char** a9, int a10,
                                   int a11, int a12);

    // 0x004b6710: copies `rect` of the backing texture (+0x30) into the
    // overlay texture (ChatOverlay calls it before redrawing a line); 0 when
    // either texture does not lock.
    int UnknownFunction4b6710(const UnknownOverlayRect* rect);

    // 0x004b6380: lays the four vertices of the overlay quad out from the
    // screen and source rectangles, clipped to the camera viewport.
    void UnknownFunction4b6380();

    // 0x004b6880: for `count` rows from `row`, every texel of the shared
    // texture (+0x34) whose low 15 bits are set is replaced by `color` in
    // the overlay texture (16-bit formats 0x115c and 0x613 only) and cleared.
    int UnknownFunction4b6880(int row, int count, int color);

    RenderTarget* Target() const { return (RenderTarget*)field_0x18; }

    TextureMap* field_0x2c;                   // released by derived destructors
    TextureMap* field_0x30;                   // RadarOverlay: the loaded map texture
    TextureMap* field_0x34;                   // shared 256x256 texture (0x00689110)
    UnknownOverlayText* field_0x38;
    OverlayIconService* field_0x3c;
    int field_0x40;
    UnknownOverlayVertex field_0x44[5];       // cleared by the constructor
    int field_0xe4;                           // 1
    UnknownOverlayRect field_0xe8;            // screen rectangle
    UnknownOverlayRect field_0xf8;            // source rectangle
    int field_0x108;                          // has a source rectangle
    int field_0x10c;                          // alpha, 0xff
    int field_0x110;                          // alpha-blended
    float field_0x114;                        // depth
    int field_0x118;                          // constructor value
};
