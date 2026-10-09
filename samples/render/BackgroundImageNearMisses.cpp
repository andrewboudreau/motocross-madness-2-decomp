// Near-miss BackgroundImage.cpp candidates, kept out of src/reconstructed
// until they match. See docs/BACKGROUNDIMAGE.md.
//
// Slot 13 0x00403dc0 (246 bytes): retail loads the camera's viewport
// height into ebx before reading the display mode; VC6 here loads the mode
// first (an inverted condition and a nested test give the same code).
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

// 0x00403dc0: while the camera covers the whole screen the background is
// restored from the off-screen copy region by region; otherwise (or while
// frames are left to restore) the whole copy is restored.
int BackgroundImage::UnknownVirtualSlot13() {
    if (!fullRestoreFrames) {
        UnknownBackgroundCamera* camera = (UnknownBackgroundCamera*)Target()->field_0x08;
        if (!camera || !camera->field_0x25_bit0 || !camera->redrawFrames ||
            camera->viewportHeight != g_TrackGame->display->displayModes[g_TrackGame->display->currentDisplayMode].height ||
            camera->viewportWidth != g_TrackGame->display->displayModes[g_TrackGame->display->currentDisplayMode].width) {
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
    UnknownVirtualSlot27(Target()->renderSurface);
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
                    surface = Target()->renderSurface;
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
                            surface = Target()->renderSurface;
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
            surface = Target()->renderSurface;
            *frames = Target()->field_0x14;
        }
        if (index >= 0)
            UnknownFunction404cb0(index);
    } else {
        if (index >= 0)
            UnknownFunction404240(index, rect);
        surface = Target()->renderSurface;
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
