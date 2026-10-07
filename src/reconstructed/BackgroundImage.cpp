#include <string.h>

#include "BackgroundImage.h"

#include "DebugAlloc.h"
#include "PCGame.h"
#include "TrackGame.h"

// The render target the image belongs to (GameObject+0x18).
#define Target() ((PCRenderTarget*)field_0x18)

// 0x005777b8: the image's own rectangle (slot 15).
CameraRect g_UnknownGlobal5777b8;
// 0x00577790 / 0x00577808: a dirty region clipped to the image and the
// matching source rectangle (0x00404480).
CameraRect g_UnknownGlobal577790;
CameraRect g_UnknownGlobal577808;
// 0x005777d8 / 0x005777c8: the same for 0x00404700, whose source rectangle
// goes to PCTextureMap 0x004c7b40.
CameraRect g_UnknownGlobal5777d8;
UnknownRect g_UnknownGlobal5777c8;
// 0x005777e8: the drawn rectangle 0x00404700 records for its region.
CameraRect g_UnknownGlobal5777e8;

// 0x00404010
BackgroundImage::~BackgroundImage() {
    if (heldDc && heldDcSurface)
        heldDcSurface->ReleaseDC(heldDc);
    if (offscreenCopy) {
        offscreenCopy->Release();
        offscreenCopy = 0;
    }
    if (field_0x2c) {
        field_0x2c->Release();
        field_0x2c = 0;
    }
    if (regionTable)
        DebugFree(regionTable, __FILE__, 230);
}

// 0x00403ec0
int BackgroundImage::UnknownVirtualSlot15() {
    if (field_0x2c && field_0x30) {
        g_UnknownGlobal5777b8.left = 0;
        g_UnknownGlobal5777b8.top = 0;
        g_UnknownGlobal5777b8.right = field_0x2c->field_0x14;
        g_UnknownGlobal5777b8.bottom = field_0x2c->field_0x18;
        UnknownFunction404480(field_0x2c, &g_UnknownGlobal5777b8, 0, 0x1000000, imageRegion, field_0x5c, &fullRedrawsOwed,
                              0);
        field_0x5c = 1;
    }
    return 1;
}

// 0x00403f30: creates the off-screen copy with the back buffer's size and
// format and four regions; releases the object on failure.
GameObject* BackgroundImage::UnknownVirtualSlot8(void* value) {
    UnknownSurfaceDesc desc;
    int i;

    GameObject::UnknownVirtualSlot8(value);
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = 0x1006;
    if (Target()->renderSurface->GetSurfaceDesc(&desc) != 0)
        goto failed;
    desc.flags = 0x1007;
    desc.caps[0] = 0x2800;
    if (Target()->field_0x04->directDraw->CreateSurface(&desc, &offscreenCopy, 0) != 0)
        goto failed;
    field_0x44 = 0;
    regionCapacity = 4;
    regionCount = 0;
    regionTable = (UnknownBackgroundRegion*)DebugMalloc(0x100, __FILE__, 212);
    for (i = 0; i < regionCapacity; i++)
        regionTable[i].framesLeft = 0;
    return this;
failed:
    Release();
    return 0;
}

// 0x004040b0
void BackgroundImage::SetImage(PCTextureMap* image) {
    if (image) {
        if (!field_0x2c)
            imageRegion = UnknownFunction4040f0(0);
        field_0x2c = image;
    } else {
        UnknownFunction404200(imageRegion);
        field_0x2c = 0;
    }
}

// 0x00404200
void BackgroundImage::UnknownFunction404200(int index) {
    if (regionTable && index < regionCapacity) {
        if (regionTable[index].framesLeft)
            regionTable[index].framesLeft--;
        if (regionTable[index].framesLeft == 0)
            regionCount--;
    }
}

// 0x00404240: records `rect` (clipped to the target) for the current frame.
void BackgroundImage::UnknownFunction404240(int index, CameraRect* rect) {
    if (rect->bottom + rect->top && rect->right + rect->left) {
        if (regionTable[index].framesLeft) {
            regionTable[index].frameRects[Target()->field_0x18] = *rect;
            CameraRect* region = &regionTable[index].frameRects[Target()->field_0x18];
            region->right = region->right < Target()->field_0x0c ? region->right : Target()->field_0x0c;
            region->bottom = region->bottom < Target()->field_0x10 ? region->bottom : Target()->field_0x10;
        }
        regionTable[index].lastFrameCount = Target()->field_0x1c;
    }
}

// 0x004042e0: copies each region's current-frame rectangle back from the
// off-screen copy and ages the regions.
int BackgroundImage::RestoreRegions() {
    if (regionCount) {
        for (int i = 0; i < regionCapacity; i++) {
            if (regionTable[i].framesLeft) {
                if (CurrentRegionRect(i).right - CurrentRegionRect(i).left > 0 &&
                    CurrentRegionRect(i).bottom - CurrentRegionRect(i).top > 0 && !regionTable[i].owner)
                    Target()->renderSurface->BltFast(CurrentRegionRect(i).left, CurrentRegionRect(i).top,
                                                         offscreenCopy, &CurrentRegionRect(i), 0x10);
                if (regionTable[i].framesLeft != Target()->field_0x14)
                    regionTable[i].framesLeft--;
                if (regionTable[i].framesLeft == 0)
                    regionCount--;
            }
        }
    }
    return 1;
}

// 0x00404480: draws `image` at `rect`. With the off-screen copy current,
// only the parts under this frame's dirty regions are redrawn (into both
// surfaces); `frames` counts full redraws still owed. The region rectangle is
// re-read for each use: the global stores could alias it.
int BackgroundImage::UnknownFunction404480(PCTextureMap* image, CameraRect* rect, void* sourceRect, int flags,
                                           int index, int dirty, int* frames, int skip) {
    if (dirty) {
        if (copyValid) {
            if (*frames) {
                UnknownFunction404c80();
                image->BlitTo(rect, offscreenCopy, sourceRect, flags, skip);
                image->BlitTo(rect, Target()->renderSurface, sourceRect, flags, skip);
                (*frames)--;
            } else {
                for (int i = 0; i < regionCapacity; i++) {
                    if (regionTable[i].lastFrameCount != Target()->field_0x1c)
                        continue;
                    g_UnknownGlobal577790.left =
                        CurrentRegionRect(i).left > rect->left ? CurrentRegionRect(i).left : rect->left;
                    g_UnknownGlobal577790.right =
                        CurrentRegionRect(i).right < rect->right ? CurrentRegionRect(i).right : rect->right;
                    g_UnknownGlobal577790.top =
                        CurrentRegionRect(i).top > rect->top ? CurrentRegionRect(i).top : rect->top;
                    g_UnknownGlobal577790.bottom =
                        CurrentRegionRect(i).bottom < rect->bottom ? CurrentRegionRect(i).bottom : rect->bottom;
                    if (g_UnknownGlobal577790.left < g_UnknownGlobal577790.right &&
                        g_UnknownGlobal577790.top < g_UnknownGlobal577790.bottom) {
                        UnknownFunction404c80();
                        g_UnknownGlobal577808.left = g_UnknownGlobal577790.left - rect->left;
                        g_UnknownGlobal577808.right = g_UnknownGlobal577790.right - rect->left;
                        g_UnknownGlobal577808.top = g_UnknownGlobal577790.top - rect->top;
                        g_UnknownGlobal577808.bottom = g_UnknownGlobal577790.bottom - rect->top;
                        image->BlitTo(&g_UnknownGlobal577790, Target()->renderSurface,
                                                     &g_UnknownGlobal577808, flags, skip);
                        image->BlitTo(&g_UnknownGlobal577790, offscreenCopy, &g_UnknownGlobal577808,
                                                     flags, skip);
                    }
                }
            }
        } else {
            UnknownFunction404c80();
            image->BlitTo(rect, Target()->renderSurface, sourceRect, flags, skip);
            *frames = Target()->field_0x14;
        }
        if (index >= 0)
            UnknownFunction404cb0(index);
        return 1;
    }
    UnknownFunction404c80();
    image->BlitTo(rect, Target()->renderSurface, sourceRect, flags, skip);
    if (index >= 0)
        UnknownFunction404240(index, rect);
    *frames = Target()->field_0x14;
    return 1;
}

// 0x00404700: as 0x00404480, but copies `rect` of the image to (x, y)
// through PCTextureMap 0x004c7b40.
int BackgroundImage::UnknownFunction404700(PCTextureMap* image, int x, int y, CameraRect* rect, int flags, int index,
                                           int dirty, int* frames, unsigned char* table) {
    if (dirty) {
        if (copyValid) {
            if (*frames) {
                UnknownFunction404c80();
                image->CopyRectTo(x, y, offscreenCopy, (UnknownRect*)rect, flags, table);
                image->CopyRectTo(x, y, Target()->renderSurface, (UnknownRect*)rect, flags, table);
                (*frames)--;
            } else {
                for (int i = 0; i < regionCapacity; i++) {
                    if (regionTable[i].lastFrameCount != Target()->field_0x1c)
                        continue;
                    g_UnknownGlobal5777d8.left =
                        CurrentRegionRect(i).left > rect->left ? CurrentRegionRect(i).left : rect->left;
                    g_UnknownGlobal5777d8.right =
                        CurrentRegionRect(i).right < rect->right ? CurrentRegionRect(i).right : rect->right;
                    g_UnknownGlobal5777d8.top =
                        CurrentRegionRect(i).top > rect->top ? CurrentRegionRect(i).top : rect->top;
                    g_UnknownGlobal5777d8.bottom =
                        CurrentRegionRect(i).bottom < rect->bottom ? CurrentRegionRect(i).bottom : rect->bottom;
                    if (g_UnknownGlobal5777d8.left < g_UnknownGlobal5777d8.right &&
                        g_UnknownGlobal5777d8.top < g_UnknownGlobal5777d8.bottom) {
                        UnknownFunction404c80();
                        g_UnknownGlobal5777c8.left = g_UnknownGlobal5777d8.left - rect->left;
                        g_UnknownGlobal5777c8.right = g_UnknownGlobal5777d8.right - rect->left;
                        g_UnknownGlobal5777c8.top = g_UnknownGlobal5777d8.top - rect->top;
                        g_UnknownGlobal5777c8.bottom = g_UnknownGlobal5777d8.bottom - rect->top;
                        image->CopyRectTo(x, y, Target()->renderSurface, &g_UnknownGlobal5777c8, flags, table);
                        image->CopyRectTo(x, y, offscreenCopy, &g_UnknownGlobal5777c8, flags, table);
                    }
                }
            }
        } else {
            UnknownFunction404c80();
            image->CopyRectTo(x, y, Target()->renderSurface, (UnknownRect*)rect, flags, table);
            *frames = Target()->field_0x14;
        }
        if (index >= 0)
            UnknownFunction404cb0(index);
        return 1;
    }
    UnknownFunction404c80();
    image->CopyRectTo(x, y, Target()->renderSurface, (UnknownRect*)rect, flags, table);
    if (index >= 0) {
        g_UnknownGlobal5777e8.left = x;
        g_UnknownGlobal5777e8.right = rect->right - rect->left + x;
        g_UnknownGlobal5777e8.top = y;
        g_UnknownGlobal5777e8.bottom = rect->bottom - rect->top + y;
        UnknownFunction404240(index, &g_UnknownGlobal5777e8);
    }
    *frames = Target()->field_0x14;
    return 1;
}

// 0x00404c80
int BackgroundImage::UnknownFunction404c80() {
    if (heldDcSurface)
        heldDcSurface->ReleaseDC(heldDc);
    heldDcSurface = 0;
    heldDc = 0;
    return 1;
}

// 0x00404cb0
void BackgroundImage::UnknownFunction404cb0(int index) {
    regionTable[index].pendingFrame = Target()->field_0x18;
}

// 0x00404cd0
void BackgroundImage::ResetPendingRects() {
    for (int i = 0; i < regionCapacity; i++) {
        if (regionTable[i].framesLeft > 0 && regionTable[i].pendingFrame != -1) {
            int frame = regionTable[i].pendingFrame;
            regionTable[i].pendingFrame = -1;
            regionTable[i].frameRects[frame].top = 0;
            regionTable[i].frameRects[frame].bottom = 0;
            regionTable[i].frameRects[frame].left = 0;
            regionTable[i].frameRects[frame].right = 0;
        }
    }
}

// 0x00404d30
void BackgroundImage::UnknownFunction404d30(int index) {
    regionTable[index].pendingFrame = -1;
    regionTable[index].frameRects[Target()->field_0x18].top = 0;
    regionTable[index].frameRects[Target()->field_0x18].bottom = 0;
    regionTable[index].frameRects[Target()->field_0x18].left = 0;
    regionTable[index].frameRects[Target()->field_0x18].right = 0;
}

// 0x00404da0: owes a full restore for every buffered frame (and asks the
// camera to redraw).
void BackgroundImage::UnknownFunction404da0() {
    fullRestoreFrames = Target()->field_0x14 + 1;
    UnknownBackgroundCamera* camera = (UnknownBackgroundCamera*)Target()->field_0x08;
    if (camera) {
        camera->redrawFrames = Target()->field_0x14 + 1;
        if ((unsigned)((UnknownBackgroundCamera*)Target()->field_0x08)->viewportHeight < (unsigned)Target()->field_0x10)
            ((UnknownBackgroundCamera*)Target()->field_0x08)->viewportEnabled = Target()->field_0x14 + 1;
    }
}

// 0x00404de0
int BackgroundImage::UnknownVirtualSlot18() {
    UnknownFunction404da0();
    return 1;
}
