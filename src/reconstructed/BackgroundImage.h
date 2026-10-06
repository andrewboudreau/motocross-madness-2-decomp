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
    int field_0x00;              // frames left (the target's +0x14 plus one); 0 when free
    CameraRect field_0x04[3];    // the region per buffered frame
    int field_0x34;              // frame whose rectangle is pending reset, or -1
    int field_0x38;              // the target's frame count when last set
    int field_0x3c;              // owner value; nonzero regions are not blitted
};

// The fields BackgroundImage reads from the target's camera (Camera.h keeps
// them protected): GameObject's +0x25 bit 0 and the slot 13 viewport.
struct UnknownBackgroundCamera {
    unsigned char field_0x000[0x25];
    unsigned char field_0x25_bit0 : 1;
    unsigned char field_0x026[0x1a8 - 0x26];
    int field_0x1a8;             // viewport width
    int field_0x1ac;             // viewport height
    unsigned char field_0x1b0[0x1cc - 0x1b0];
    int field_0x1cc;             // enables the viewport rectangle
    int field_0x1d0;             // frames left to redraw
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

    void UnknownFunction4040b0(PCTextureMap* image);   // 0x004040b0: sets the image
    int UnknownFunction4040f0(int owner);               // 0x004040f0: allocates a region
    void UnknownFunction404200(int index);              // 0x00404200: releases a region
    void UnknownFunction404240(int index, CameraRect* rect); // 0x00404240
    int UnknownFunction4042e0();                        // 0x004042e0: restores the regions
    int UnknownFunction4043c0();                        // 0x004043c0: clears their depth
    int UnknownFunction404480(PCTextureMap* image, CameraRect* rect, void* sourceRect, int flags, int index,
                              int dirty, int* frames, int skip); // 0x00404480: draws an image
    // 0x00404700: draws `image` at (x, y) (GameCursor slot 15; not reconstructed).
    int UnknownFunction404700(PCTextureMap* image, int x, int y, void* sourceRect, int flags, int index,
                              int dirty, int* frames, int skip);
    int UnknownFunction404c80();                        // 0x00404c80: releases the DC
    void UnknownFunction404cb0(int index);              // 0x00404cb0
    void UnknownFunction404cd0();                       // 0x00404cd0: resets pending rectangles
    void UnknownFunction404d30(int index);              // 0x00404d30
    void UnknownFunction404da0();                       // 0x00404da0: marks everything dirty

    PCTextureMap* field_0x2c;              // image
    int field_0x30;                        // visible
    int field_0x34;                        // the image's region
    int field_0x38;                        // full redraws the image still owes
    UnknownSurfaceInterface* field_0x3c;   // off-screen copy
    int field_0x40;                        // the copy holds the background
    int field_0x44;
    int field_0x48;                        // regions allocated
    int field_0x4c;                        // regions in use
    UnknownBackgroundRegion* field_0x50;
    int field_0x54;
    int field_0x58;                        // frames left to restore fully
    int field_0x5c;
    int field_0x60;
    int field_0x64;
    int field_0x68;
    UnknownSurfaceInterface* field_0x6c;   // surface whose DC is held
    void* field_0x70;                      // the DC
};
