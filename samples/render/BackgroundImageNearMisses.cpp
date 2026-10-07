// Near-miss BackgroundImage.cpp candidates, kept out of src/reconstructed
// until they match. See docs/BACKGROUNDIMAGE.md.
//
// Constructor 0x00403d50 (77 bytes): the stores match, but retail emits the
// vptr store between the -1 stores and the 1 stores. With body assignments
// VC6 here stores it first; with member initializers it stores it last.
//
// Slot 13 0x00403dc0 (246 bytes): retail loads the camera's viewport
// height into ebx before reading the display mode; VC6 here loads the mode
// first (an inverted condition and a nested test give the same code).
//
// 0x004040f0 (261 bytes): retail places the shared `return -1` after the
// search loop and jumps there on a failed realloc; VC6 here places it at the
// end (early returns or `goto failed`).
//
// 0x004043c0 (177 bytes): the instructions match; retail lays the
// `return 0` block out between the loop and the final `return 1` (`jge`
// out, `jmp` back), VC6 here puts it last. A local rectangle pointer, the
// accessor, early returns, `continue`, `goto` and a while loop all compile
// to the same layout.
//
// 0x004049d0 (678 bytes): retail stores the three leading zeros (and
// `field_0x60 = 0`) as immediates; VC6 here caches 0 in edi, which shifts
// the register choice of the region loop. Statement orders and a helper
// for the DC tail do not change that.

#include <string.h>

#include "../../src/reconstructed/BackgroundImage.h"

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/DisplayMode.h"
#include "../../src/reconstructed/TrackGame.h"

#define Target() ((PCRenderTarget*)field_0x18)

extern CameraRect g_UnknownGlobal5777b8;
extern CameraRect g_UnknownGlobal577790;
extern CameraRect g_UnknownGlobal577808;
// 0x005777a0 / 0x005777a8: 0x004049d0's region index and clipped rectangle.
int g_UnknownGlobal5777a0;
CameraRect g_UnknownGlobal5777a8;

// 0x00403d50
BackgroundImage::BackgroundImage(int flags) : GameObject(flags) {
    copyValid = 0;
    field_0x2c = 0;
    regionTable = 0;
    regionCapacity = 0;
    regionCount = 0;
    fullRestoreFrames = 0;
    heldDcSurface = 0;
    heldDc = 0;
    field_0x5c = 0;
    imageRegion = -1;
    field_0x64 = -1;
    field_0x30 = 1;
    field_0x60 = 1;
    field_0x68 = 1;
}

// 0x00403dc0: while the camera covers the whole screen the background is
// restored from the off-screen copy region by region; otherwise (or while
// frames are left to restore) the whole copy is restored.
int BackgroundImage::UnknownVirtualSlot13() {
    if (!fullRestoreFrames) {
        UnknownBackgroundCamera* camera = (UnknownBackgroundCamera*)Target()->field_0x08;
        if (!camera || !camera->field_0x25_bit0 || !camera->redrawFrames ||
            camera->viewportHeight != g_TrackGame->display->field_0x10[g_TrackGame->display->field_0x0c].height ||
            camera->viewportWidth != g_TrackGame->display->field_0x10[g_TrackGame->display->field_0x0c].width) {
            if (!copyValid) {
                UnknownVirtualSlot27(offscreenCopy);
                copyValid = 1;
            }
            ResetPendingRects();
            if (Target()->field_0x08)
                ClearRegionDepth();
            RestoreRegions();
            return 1;
        }
    }
    UnknownVirtualSlot27(Target()->field_0x48);
    copyValid = 0;
    if (Target()->field_0x08)
        ClearRegionDepth();
    if (regionCount) {
        for (int i = 0; i < regionCapacity; i++)
            UnknownFunction404d30(i);
    }
    if (--fullRestoreFrames < 0)
        fullRestoreFrames = 0;
    return 1;
}

// 0x004040f0: a free region, or four more.
int BackgroundImage::UnknownFunction4040f0(int owner) {
    int i;

    if (regionCapacity > regionCount) {
        for (i = 0; i < regionCapacity; i++) {
            if (regionTable[i].framesLeft == 0) {
                regionTable[i].framesLeft = Target()->field_0x14 + 1;
                regionTable[i].owner = owner;
                regionCount++;
                return i;
            }
        }
        goto failed;
    }
    regionTable = (UnknownBackgroundRegion*)DebugRealloc(regionTable, (regionCapacity + 4) * sizeof(UnknownBackgroundRegion),
                                                        __FILE__, 270);
    if (!regionTable)
        goto failed;
    for (i = regionCapacity; i < regionCapacity + 4; i++) {
        regionTable[i].framesLeft = 0;
        regionTable[i].pendingFrame = -1;
        regionTable[i].owner = 0;
    }
    regionTable[regionCapacity].framesLeft = Target()->field_0x14 + 1;
    regionTable[regionCapacity].lastFrameCount = Target()->field_0x1c;
    regionTable[regionCapacity].owner = owner;
    regionCapacity += 4;
    regionCount++;
    return regionCapacity - 4;
failed:
    return -1;
}

// 0x004043c0: clears the depth buffer under each region's previous-frame
// rectangle.
int BackgroundImage::ClearRegionDepth() {
    if (regionCount && Target()->field_0x08) {
        int frame = Target()->field_0x18 - 1;
        if (frame < 0)
            frame = Target()->field_0x14 - 1;
        for (int i = 0; i < regionCapacity; i++) {
            if (regionTable[i].framesLeft &&
                regionTable[i].frameRects[frame].right - regionTable[i].frameRects[frame].left > 0 &&
                regionTable[i].frameRects[frame].bottom - regionTable[i].frameRects[frame].top > 0 &&
                Target()->device->Clear(1, &regionTable[i].frameRects[frame], 2, 0,
                                                      Target()->field_0x2c, 0) != 0)
                return 0;
        }
    }
    return 1;
}

// 0x004049d0: a DC for drawing `rect`: the back buffer, or with the copy
// current the surfaces in turn (`a` set when the copy is returned), clipped
// to the next dirty region in `clip`.
int BackgroundImage::UnknownFunction4049d0(void** dc, CameraRect* rect, int index, int dirty, int* frames, int* a,
                                           CameraRect* clip) {
    UnknownSurfaceInterface* surface;
    *dc = 0;
    surface = 0;
    *a = 0;
    if (dirty) {
        if (copyValid) {
            if (*frames) {
                if (field_0x60) {
                    *a = 1;
                    surface = offscreenCopy;
                    field_0x60 = 0;
                } else {
                    surface = Target()->field_0x48;
                    field_0x60 = 1;
                    (*frames)--;
                }
            } else {
                field_0x60 = 1;
                for (g_UnknownGlobal5777a0 = 0; g_UnknownGlobal5777a0 < regionCapacity; g_UnknownGlobal5777a0++) {
                    if (regionTable[g_UnknownGlobal5777a0].lastFrameCount != Target()->field_0x1c)
                        continue;
                    g_UnknownGlobal5777a8.left = CurrentRegionRect(g_UnknownGlobal5777a0).left > rect->left ? CurrentRegionRect(g_UnknownGlobal5777a0).left : rect->left;
                    g_UnknownGlobal5777a8.right = CurrentRegionRect(g_UnknownGlobal5777a0).right < rect->right ? CurrentRegionRect(g_UnknownGlobal5777a0).right : rect->right;
                    g_UnknownGlobal5777a8.top = CurrentRegionRect(g_UnknownGlobal5777a0).top > rect->top ? CurrentRegionRect(g_UnknownGlobal5777a0).top : rect->top;
                    g_UnknownGlobal5777a8.bottom = CurrentRegionRect(g_UnknownGlobal5777a0).bottom < rect->bottom ? CurrentRegionRect(g_UnknownGlobal5777a0).bottom : rect->bottom;
                    if (g_UnknownGlobal5777a8.left < g_UnknownGlobal5777a8.right &&
                        g_UnknownGlobal5777a8.top < g_UnknownGlobal5777a8.bottom) {
                        if (field_0x68 && g_UnknownGlobal5777a0 > field_0x64) {
                            surface = Target()->field_0x48;
                            field_0x68 = 0;
                        } else if (!field_0x68 && g_UnknownGlobal5777a0 == field_0x64) {
                            surface = offscreenCopy;
                            field_0x68 = 1;
                        } else {
                            continue;
                        }
                        *a = 1;
                        *clip = g_UnknownGlobal5777a8;
                        field_0x64 = g_UnknownGlobal5777a0;
                        break;
                    }
                }
                if (g_UnknownGlobal5777a0 == regionCapacity) {
                    field_0x68 = 1;
                    field_0x64 = -1;
                }
            }
        } else {
            surface = Target()->field_0x48;
            *frames = Target()->field_0x14;
        }
        if (index >= 0)
            UnknownFunction404cb0(index);
    } else {
        if (index >= 0)
            UnknownFunction404240(index, rect);
        surface = Target()->field_0x48;
        *frames = Target()->field_0x14;
    }
    if (!surface)
        return 0;
    if (heldDcSurface) {
        if (surface != heldDcSurface) {
            UnknownFunction404c80();
            heldDcSurface = surface;
            surface->GetDC(&heldDc);
        }
    } else {
        heldDcSurface = surface;
        surface->GetDC(&heldDc);
    }
    *dc = heldDc;
    return 1;
}
