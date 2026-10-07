#pragma once

#include "GameObject.h"
#include "PCRenderTarget.h"
#include "PCTextureMap.h"

// BackgroundImage.cpp (literal __FILE__ at 0x00566978): RTTI BackgroundImage
// : GameObject (vtable 0x005506d8). It draws a PCTextureMap behind the scene
// into its render target (GameObject+0x18, a PCRenderTarget), keeps an
// off-screen copy (+0x3c) and tracks the rectangles other objects dirty in
// each of the target's buffered frames. Names are provisional.

// One tracked region (0x40 bytes, a DebugMalloc'd array at +0x50).
struct UnknownBackgroundRegion {
    int framesLeft;              // frames left (the target's +0x14 plus one); 0 when free
    CameraRect frameRects[3];    // the region per buffered frame
    int pendingFrame;            // frame whose rectangle is pending reset, or -1
    int lastFrameCount;          // the target's frame count when last set
    int owner;                   // owner value; nonzero regions are not blitted
};

// The fields BackgroundImage reads from the target's camera (Camera.h keeps
// them protected): GameObject's +0x25 bit 0 and the slot 13 viewport.
struct UnknownBackgroundCamera {
    unsigned char field_0x000[0x25];
    unsigned char field_0x25_bit0 : 1;
    unsigned char field_0x026[0x1a8 - 0x26];
    int viewportWidth;           // viewport width
    int viewportHeight;          // viewport height
    unsigned char field_0x1b0[0x1cc - 0x1b0];
    int viewportEnabled;         // enables the viewport rectangle
    int redrawFrames;            // frames left to redraw
};

class BackgroundImage : public GameObject {
public:
    explicit BackgroundImage(int flags);  // 0x00403d50
    virtual ~BackgroundImage();           // 0x00404010 (deleting wrapper 0x00403da0)

    virtual GameObject* UnknownVirtualSlot8(void* value); // 0x00403f30: creates the off-screen copy
    virtual int UnknownVirtualSlot13();   // 0x00403dc0: restores the background
    virtual int UnknownVirtualSlot15();   // 0x00403ec0: draws the image
    virtual int UnknownVirtualSlot18();   // 0x00404de0
    // Slot 27 (shared `return 1` body 0x004627f0): called with the surface
    // the background is restored into.
    virtual int UnknownVirtualSlot27(UnknownSurfaceInterface* surface);

    void SetImage(PCTextureMap* image);                // 0x004040b0: sets the image
    int UnknownFunction4040f0(int owner);               // 0x004040f0: allocates a region
    void UnknownFunction404200(int index);              // 0x00404200: releases a region
    void UnknownFunction404240(int index, CameraRect* rect); // 0x00404240
    int RestoreRegions();                               // 0x004042e0: restores the regions
    int ClearRegionDepth();                             // 0x004043c0: clears their depth
    int UnknownFunction404480(PCTextureMap* image, CameraRect* rect, void* sourceRect, int flags, int index,
                              int dirty, int* frames, int skip); // 0x00404480: draws an image
    // 0x00404700: copies `rect` of `image` to (x, y) (GameCursor slot 15).
    int UnknownFunction404700(PCTextureMap* image, int x, int y, CameraRect* rect, int flags, int index,
                              int dirty, int* frames, unsigned char* table);
    // 0x004049d0: a DC clipped to `rect` (gameui.cpp 0x0046ed70; near miss in
    // samples/render/BackgroundImageNearMisses.cpp).
    int UnknownFunction4049d0(void** dc, CameraRect* rect, int index, int dirty, int* frames, int* a,
                              CameraRect* clip);
    // Region `index`'s rectangle for the target's current frame.
    CameraRect& CurrentRegionRect(int index) {
        return regionTable[index].frameRects[((PCRenderTarget*)field_0x18)->field_0x18];
    }
    int UnknownFunction404c80();                        // 0x00404c80: releases the DC
    void UnknownFunction404cb0(int index);              // 0x00404cb0
    void ResetPendingRects();                           // 0x00404cd0: resets pending rectangles
    void UnknownFunction404d30(int index);              // 0x00404d30
    void UnknownFunction404da0();                       // 0x00404da0: marks everything dirty

    PCTextureMap* field_0x2c;              // image
    int field_0x30;                        // visible
    int imageRegion;                       // the image's region
    int fullRedrawsOwed;                   // full redraws the image still owes
    UnknownSurfaceInterface* offscreenCopy; // off-screen copy
    int copyValid;                         // the copy holds the background
    int field_0x44;
    int regionCapacity;                    // regions allocated
    int regionCount;                       // regions in use
    UnknownBackgroundRegion* regionTable;
    int field_0x54;
    int fullRestoreFrames;                 // frames left to restore fully
    int field_0x5c;
    int field_0x60;
    int field_0x64;
    int field_0x68;
    UnknownSurfaceInterface* heldDcSurface; // surface whose DC is held
    void* heldDc;                          // the DC
};
