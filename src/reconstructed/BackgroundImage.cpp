#include <string.h>

#include "BackgroundImage.h"

#include "DebugAlloc.h"
#include "PCGame.h"
#include "TrackGame.h"

// The render target the image belongs to (GameObject+0x18).
#define Target() ((PCRenderTarget*)renderContext)

// 0x005777b8: the image's own rectangle (slot 15).
CameraRect g_BackgroundImageRect;
// 0x00577790 / 0x00577808: a dirty region clipped to the image and the
// matching source rectangle (0x00404480).
CameraRect g_DrawImageClipRect;
CameraRect g_DrawImageSourceRect;
// 0x005777d8 / 0x005777c8: the same for 0x00404700, whose source rectangle
// goes to PCTextureMap 0x004c7b40.
CameraRect g_DrawImageRectClipRect;
UnknownRect g_DrawImageRectSourceRect;
// 0x005777e8: the drawn rectangle 0x00404700 records for its region.
CameraRect g_DrawImageRectDrawnRect;

// 0x00403d50
BackgroundImage::BackgroundImage(int flags) : GameObject(flags) {
    imageVisible = 1;
    copyValid = 0;
    currentImage = 0;
    regionTable = 0;
    regionCapacity = 0;
    regionCount = 0;
    fullRestoreFrames = 0;
    heldDcSurface = 0;
    heldDc = 0;
    field_0x5c = 0;
    imageRegion = -1;
    lastDcRegion = -1;
    copyDcNext = 1;
    lastDcRegionCopied = 1;
}

// The current display mode's size. Read through these helpers, each test
// indexes the mode table afresh and loads the camera's field first, as
// retail does in 0x00403dc0; written inline, VC6 shares the row address.
static inline int ModeHeight(UnknownDisplay* display)
{
    return display->displayModes[display->currentDisplayMode].height;
}

static inline int ModeWidth(UnknownDisplay* display)
{
    return display->displayModes[display->currentDisplayMode].width;
}

// 0x00403dc0: while the camera covers the whole screen the background is
// restored from the off-screen copy region by region; otherwise (or while
// frames are left to restore) the whole copy is restored.
int BackgroundImage::RenderPre3D() {
    if (!fullRestoreFrames) {
        UnknownBackgroundCamera* camera = (UnknownBackgroundCamera*)Target()->currentCamera;
        if (!camera || !camera->isEnabled || !camera->redrawFrames ||
            camera->viewportHeight != ModeHeight(g_TrackGame->display) ||
            camera->viewportWidth != ModeWidth(g_TrackGame->display)) {
            if (!copyValid) {
                UnknownVirtualSlot27(offscreenCopy);
                copyValid = 1;
            }
            ResetPendingRects();
            if (Target()->currentCamera)
                ClearRegionDepth();
            RestoreRegions();
            return 1;
        }
    }
    UnknownVirtualSlot27(Target()->renderSurface);
    copyValid = 0;
    if (Target()->currentCamera)
        ClearRegionDepth();
    if (regionCount) {
        for (int i = 0; i < regionCapacity; i++)
            ClearRegionRect(i);
    }
    if (--fullRestoreFrames < 0)
        fullRestoreFrames = 0;
    return 1;
}

// 0x00404010
BackgroundImage::~BackgroundImage() {
    if (heldDc && heldDcSurface)
        heldDcSurface->ReleaseDC(heldDc);
    if (offscreenCopy) {
        offscreenCopy->Release();
        offscreenCopy = 0;
    }
    if (currentImage) {
        currentImage->Release();
        currentImage = 0;
    }
    if (regionTable)
        DebugFree(regionTable, __FILE__, 230);
}

// 0x00403ec0
int BackgroundImage::RenderPost3D() {
    if (currentImage && imageVisible) {
        g_BackgroundImageRect.left = 0;
        g_BackgroundImageRect.top = 0;
        g_BackgroundImageRect.right = currentImage->textureWidth;
        g_BackgroundImageRect.bottom = currentImage->textureHeight;
        DrawImage(currentImage, &g_BackgroundImageRect, 0, 0x1000000, imageRegion, field_0x5c, &fullRedrawsOwed,
                              0);
        field_0x5c = 1;
    }
    return 1;
}

// 0x00403f30: creates the off-screen copy with the back buffer's size and
// format and four regions; releases the object on failure.
GameObject* BackgroundImage::SetRenderContext(void* context) {
    UnknownSurfaceDesc desc;
    int i;

    GameObject::SetRenderContext(context);
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = 0x1006;
    if (Target()->renderSurface->GetSurfaceDesc(&desc) != 0)
        goto failed;
    desc.flags = 0x1007;
    desc.caps[0] = 0x2800;
    if (Target()->targetDisplay->directDraw->CreateSurface(&desc, &offscreenCopy, 0) != 0)
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
        if (!currentImage)
            imageRegion = AllocRegion(0);
        currentImage = image;
    } else {
        ReleaseRegion(imageRegion);
        currentImage = 0;
    }
}

// 0x004040f0: a free region, or four more.
int BackgroundImage::AllocRegion(int owner) {
    int i;

    if (regionCapacity > regionCount) {
        for (i = 0; i < regionCapacity; i++) {
            if (regionTable[i].framesLeft == 0) {
                regionTable[i].framesLeft = Target()->frameModulus + 1;
                regionTable[i].owner = owner;
                regionCount++;
                return i;
            }
        }
        return -1;
    }
    regionTable = (UnknownBackgroundRegion*)DebugRealloc(regionTable, (regionCapacity + 4) * sizeof(UnknownBackgroundRegion),
                                                        __FILE__, 270);
    if (regionTable) {
        for (i = regionCapacity; i < regionCapacity + 4; i++) {
            regionTable[i].framesLeft = 0;
            regionTable[i].pendingFrame = -1;
            regionTable[i].owner = 0;
        }
        regionTable[regionCapacity].framesLeft = Target()->frameModulus + 1;
        regionTable[regionCapacity].lastFrameCount = Target()->frameCount;
        regionTable[regionCapacity].owner = owner;
        regionCapacity += 4;
        regionCount++;
        return regionCapacity - 4;
    }
    return -1;
}

// 0x00404200
void BackgroundImage::ReleaseRegion(int index) {
    if (regionTable && index < regionCapacity) {
        if (regionTable[index].framesLeft)
            regionTable[index].framesLeft--;
        if (regionTable[index].framesLeft == 0)
            regionCount--;
    }
}

// 0x00404240: records `rect` (clipped to the target) for the current frame.
void BackgroundImage::SetRegionRect(int index, CameraRect* rect) {
    if (rect->bottom + rect->top && rect->right + rect->left) {
        if (regionTable[index].framesLeft) {
            regionTable[index].frameRects[Target()->frameIndex] = *rect;
            CameraRect* region = &regionTable[index].frameRects[Target()->frameIndex];
            region->right = region->right < Target()->targetWidth ? region->right : Target()->targetWidth;
            region->bottom = region->bottom < Target()->targetHeight ? region->bottom : Target()->targetHeight;
        }
        regionTable[index].lastFrameCount = Target()->frameCount;
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
                if (regionTable[i].framesLeft != Target()->frameModulus)
                    regionTable[i].framesLeft--;
                if (regionTable[i].framesLeft == 0)
                    regionCount--;
            }
        }
    }
    return 1;
}

// 0x004043c0: clears the depth buffer under each region's previous-frame
// rectangle.
int BackgroundImage::ClearRegionDepth() {
    if (regionCount && Target()->currentCamera) {
        int frame = Target()->frameIndex - 1;
        if (frame < 0)
            frame = Target()->frameModulus - 1;
        int i = 0;
        if (regionCapacity > 0) {
            do {
                if (regionTable[i].framesLeft &&
                    regionTable[i].frameRects[frame].right - regionTable[i].frameRects[frame].left > 0 &&
                    regionTable[i].frameRects[frame].bottom - regionTable[i].frameRects[frame].top > 0 &&
                    Target()->device->Clear(1, &regionTable[i].frameRects[frame], 2, 0,
                                            Target()->zClearValue, 0) != 0)
                    return 0;
                i++;
            } while (i < regionCapacity);
        }
    }
    return 1;
}

// 0x00404480: draws `image` at `rect`. With the off-screen copy current,
// only the parts under this frame's dirty regions are redrawn (into both
// surfaces); `frames` counts full redraws still owed. The region rectangle is
// re-read for each use: the global stores could alias it.
int BackgroundImage::DrawImage(PCTextureMap* image, CameraRect* rect, void* sourceRect, int flags,
                                           int index, int dirty, int* frames, int skip) {
    if (dirty) {
        if (copyValid) {
            if (*frames) {
                ReleaseDc();
                image->BlitTo(rect, offscreenCopy, sourceRect, flags, skip);
                image->BlitTo(rect, Target()->renderSurface, sourceRect, flags, skip);
                (*frames)--;
            } else {
                for (int i = 0; i < regionCapacity; i++) {
                    if (regionTable[i].lastFrameCount != Target()->frameCount)
                        continue;
                    g_DrawImageClipRect.left =
                        CurrentRegionRect(i).left > rect->left ? CurrentRegionRect(i).left : rect->left;
                    g_DrawImageClipRect.right =
                        CurrentRegionRect(i).right < rect->right ? CurrentRegionRect(i).right : rect->right;
                    g_DrawImageClipRect.top =
                        CurrentRegionRect(i).top > rect->top ? CurrentRegionRect(i).top : rect->top;
                    g_DrawImageClipRect.bottom =
                        CurrentRegionRect(i).bottom < rect->bottom ? CurrentRegionRect(i).bottom : rect->bottom;
                    if (g_DrawImageClipRect.left < g_DrawImageClipRect.right &&
                        g_DrawImageClipRect.top < g_DrawImageClipRect.bottom) {
                        ReleaseDc();
                        g_DrawImageSourceRect.left = g_DrawImageClipRect.left - rect->left;
                        g_DrawImageSourceRect.right = g_DrawImageClipRect.right - rect->left;
                        g_DrawImageSourceRect.top = g_DrawImageClipRect.top - rect->top;
                        g_DrawImageSourceRect.bottom = g_DrawImageClipRect.bottom - rect->top;
                        image->BlitTo(&g_DrawImageClipRect, Target()->renderSurface,
                                                     &g_DrawImageSourceRect, flags, skip);
                        image->BlitTo(&g_DrawImageClipRect, offscreenCopy, &g_DrawImageSourceRect,
                                                     flags, skip);
                    }
                }
            }
        } else {
            ReleaseDc();
            image->BlitTo(rect, Target()->renderSurface, sourceRect, flags, skip);
            *frames = Target()->frameModulus;
        }
        if (index >= 0)
            MarkRegionPending(index);
        return 1;
    }
    ReleaseDc();
    image->BlitTo(rect, Target()->renderSurface, sourceRect, flags, skip);
    if (index >= 0)
        SetRegionRect(index, rect);
    *frames = Target()->frameModulus;
    return 1;
}

// 0x00404700: as 0x00404480, but copies `rect` of the image to (x, y)
// through PCTextureMap 0x004c7b40.
int BackgroundImage::DrawImageRect(PCTextureMap* image, int x, int y, CameraRect* rect, int flags, int index,
                                           int dirty, int* frames, unsigned char* table) {
    if (dirty) {
        if (copyValid) {
            if (*frames) {
                ReleaseDc();
                image->CopyRectTo(x, y, offscreenCopy, (UnknownRect*)rect, flags, table);
                image->CopyRectTo(x, y, Target()->renderSurface, (UnknownRect*)rect, flags, table);
                (*frames)--;
            } else {
                for (int i = 0; i < regionCapacity; i++) {
                    if (regionTable[i].lastFrameCount != Target()->frameCount)
                        continue;
                    g_DrawImageRectClipRect.left =
                        CurrentRegionRect(i).left > rect->left ? CurrentRegionRect(i).left : rect->left;
                    g_DrawImageRectClipRect.right =
                        CurrentRegionRect(i).right < rect->right ? CurrentRegionRect(i).right : rect->right;
                    g_DrawImageRectClipRect.top =
                        CurrentRegionRect(i).top > rect->top ? CurrentRegionRect(i).top : rect->top;
                    g_DrawImageRectClipRect.bottom =
                        CurrentRegionRect(i).bottom < rect->bottom ? CurrentRegionRect(i).bottom : rect->bottom;
                    if (g_DrawImageRectClipRect.left < g_DrawImageRectClipRect.right &&
                        g_DrawImageRectClipRect.top < g_DrawImageRectClipRect.bottom) {
                        ReleaseDc();
                        g_DrawImageRectSourceRect.left = g_DrawImageRectClipRect.left - rect->left;
                        g_DrawImageRectSourceRect.right = g_DrawImageRectClipRect.right - rect->left;
                        g_DrawImageRectSourceRect.top = g_DrawImageRectClipRect.top - rect->top;
                        g_DrawImageRectSourceRect.bottom = g_DrawImageRectClipRect.bottom - rect->top;
                        image->CopyRectTo(x, y, Target()->renderSurface, &g_DrawImageRectSourceRect, flags, table);
                        image->CopyRectTo(x, y, offscreenCopy, &g_DrawImageRectSourceRect, flags, table);
                    }
                }
            }
        } else {
            ReleaseDc();
            image->CopyRectTo(x, y, Target()->renderSurface, (UnknownRect*)rect, flags, table);
            *frames = Target()->frameModulus;
        }
        if (index >= 0)
            MarkRegionPending(index);
        return 1;
    }
    ReleaseDc();
    image->CopyRectTo(x, y, Target()->renderSurface, (UnknownRect*)rect, flags, table);
    if (index >= 0) {
        g_DrawImageRectDrawnRect.left = x;
        g_DrawImageRectDrawnRect.right = rect->right - rect->left + x;
        g_DrawImageRectDrawnRect.top = y;
        g_DrawImageRectDrawnRect.bottom = rect->bottom - rect->top + y;
        SetRegionRect(index, &g_DrawImageRectDrawnRect);
    }
    *frames = Target()->frameModulus;
    return 1;
}

// 0x00404c80
int BackgroundImage::ReleaseDc() {
    if (heldDcSurface)
        heldDcSurface->ReleaseDC(heldDc);
    heldDcSurface = 0;
    heldDc = 0;
    return 1;
}

// 0x00404cb0
void BackgroundImage::MarkRegionPending(int index) {
    regionTable[index].pendingFrame = Target()->frameIndex;
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
void BackgroundImage::ClearRegionRect(int index) {
    regionTable[index].pendingFrame = -1;
    regionTable[index].frameRects[Target()->frameIndex].top = 0;
    regionTable[index].frameRects[Target()->frameIndex].bottom = 0;
    regionTable[index].frameRects[Target()->frameIndex].left = 0;
    regionTable[index].frameRects[Target()->frameIndex].right = 0;
}

// 0x00404da0: owes a full restore for every buffered frame (and asks the
// camera to redraw).
void BackgroundImage::MarkAllDirty() {
    fullRestoreFrames = Target()->frameModulus + 1;
    UnknownBackgroundCamera* camera = (UnknownBackgroundCamera*)Target()->currentCamera;
    if (camera) {
        camera->redrawFrames = Target()->frameModulus + 1;
        if ((unsigned)((UnknownBackgroundCamera*)Target()->currentCamera)->viewportHeight < (unsigned)Target()->targetHeight)
            ((UnknownBackgroundCamera*)Target()->currentCamera)->viewportEnabled = Target()->frameModulus + 1;
    }
}

// 0x00404de0
int BackgroundImage::OnActivate() {
    MarkAllDirty();
    return 1;
}

// 0x00404df0: the function after this file's last method and before its
// Vec3 constructor copy (0x00404e60); ownership by position only. Flags
// the +0x58 objects of the two entries the owner currently picks, then
// picks `a` and `b` and clears theirs.
void TrackGameViewOwner::SelectLapGates(int a, int b, void* racer) {
    if (gates[lapGate]->field_0x58) {
        gates[lapGate]->field_0x58->field_0x44 = 1;
        gates[nextLapGate]->field_0x58->field_0x44 = 1;
        lapGate = a;
        nextLapGate = b;
        gates[lapGate]->field_0x58->field_0x44 = 0;
        gates[nextLapGate]->field_0x58->field_0x44 = 0;
    }
}
