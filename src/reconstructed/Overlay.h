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
    virtual int Tick(float frameTime);                 // 0x00499ae0
    virtual int UnknownVirtualSlot14();       // 0x004b61a0: draws the overlay quad

    // 0x004b5f50: binds the render target (slot 8), the texture (+0x2c), the
    // screen rectangle (+0xe8) and an optional source rectangle (+0xf8).
    Overlay* Attach(RenderTarget* target, TextureMap* texture,
                                   const UnknownOverlayRect* rect, int scaleRect,
                                   const UnknownOverlayRect* source, float depth,
                                   char withText, const char* textArchive, const char** fontNames,
                                   int fontCount, int textFormat, int a12);

    // 0x004b6710: copies `rect` of the backing texture (+0x30) into the
    // overlay texture (ChatOverlay calls it before redrawing a line); 0 when
    // either texture does not lock.
    int RestoreRect(const UnknownOverlayRect* rect);

    // 0x004b6380: lays the four vertices of the overlay quad out from the
    // screen and source rectangles, clipped to the camera viewport.
    void LayOutQuad();

    // 0x004b6880: for `count` rows from `row`, every texel of the shared
    // texture (+0x34) whose low 15 bits are set is replaced by `color` in
    // the overlay texture (16-bit formats 4444 and 1555 only) and cleared.
    int TintRows(int row, int count, unsigned short color);

    RenderTarget* Target() const { return (RenderTarget*)renderContext; }

    TextureMap* overlayTexture;               // released by derived destructors
    TextureMap* backingTexture;               // copied back by 0x004b6710; RadarOverlay: the map
    TextureMap* sharedTexture;                // shared 256x256 texture (0x00689110)
    UnknownOverlayText* textService;          // +0x38 fonts for the overlay (0x0050ae80), or 0
    OverlayIconService* iconService;          // +0x3c
    int field_0x40;
    UnknownOverlayVertex quadVertices[5];     // cleared by the constructor
    int scaleRect;                            // +0xe4 ctor 1, Attach's argument: LayOutQuad scales screenRect from 640x480 to the viewport
    UnknownOverlayRect screenRect;            // screen rectangle
    UnknownOverlayRect sourceRect;            // source rectangle
    int hasSourceRect;                        // has a source rectangle
    int overlayAlpha;                         // alpha, 0xff
    int alphaBlended;                         // alpha-blended
    float overlayDepth;                       // depth
    int field_0x118;                          // constructor value
};
