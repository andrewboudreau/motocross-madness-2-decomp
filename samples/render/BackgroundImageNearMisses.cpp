// Near-miss BackgroundImage.cpp candidates, kept out of src/reconstructed
// until they match. See docs/BACKGROUNDIMAGE.md.
//
// 0x004049d0 (678 bytes): retail stores the three leading zeros (and
// `field_0x60 = 0`) as immediates; VC6 here caches 0 in edi, which shifts
// the register choice of the region loop. Statement orders and a helper
// for the DC tail do not change that. The DC tail as `if (surface) { ... }
// return 0;` with the release branch returning on its own gives retail's
// block order (retail duplicates that `*dc = heldDc; return 1` tail and puts
// `return 0` last): 39 -> 108 of 606 masked bytes, 65 -> 46 instruction
// diffs, candidate 678 bytes like retail. Then `*a = 0` before `surface = 0`
// and `surface` before `*a = 1` in the copy branch align the rest. All six
// orders of the three leading stores, `surface` initialised at its
// declaration and `field_0x60 = 0` first in its branch keep the edi zero.

#include <string.h>

#include "../../src/reconstructed/BackgroundImage.h"

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/DisplayMode.h"
#include "../../src/reconstructed/TrackGame.h"

#define Target() ((PCRenderTarget*)renderContext)

extern CameraRect g_BackgroundImageRect;
extern CameraRect g_DrawImageClipRect;
extern CameraRect g_DrawImageSourceRect;
// 0x005777a0 / 0x005777a8: GetDc's dirty-region index and clipped rectangle;
// retail references them only inside GetDc (0x00404a56..0x00404ba8).
int g_GetDcRegionIndex;
CameraRect g_GetDcClipRect;

// 0x004049d0: a DC for drawing `rect`: the back buffer, or with the copy
// current the surfaces in turn (`a` set when the copy is returned), clipped
// to the next dirty region in `clip`.
int BackgroundImage::GetDc(void** dc, CameraRect* rect, int index, int dirty, int* frames, int* a,
                                           CameraRect* clip) {
    UnknownSurfaceInterface* surface;
    *dc = 0;
    *a = 0;
    surface = 0;
    if (dirty) {
        if (copyValid) {
            if (*frames) {
                if (field_0x60) {
                    surface = offscreenCopy;
                    *a = 1;
                    field_0x60 = 0;
                } else {
                    surface = Target()->renderSurface;
                    field_0x60 = 1;
                    (*frames)--;
                }
            } else {
                field_0x60 = 1;
                for (g_GetDcRegionIndex = 0; g_GetDcRegionIndex < regionCapacity; g_GetDcRegionIndex++) {
                    if (regionTable[g_GetDcRegionIndex].lastFrameCount != Target()->frameCount)
                        continue;
                    g_GetDcClipRect.left = CurrentRegionRect(g_GetDcRegionIndex).left > rect->left ? CurrentRegionRect(g_GetDcRegionIndex).left : rect->left;
                    g_GetDcClipRect.right = CurrentRegionRect(g_GetDcRegionIndex).right < rect->right ? CurrentRegionRect(g_GetDcRegionIndex).right : rect->right;
                    g_GetDcClipRect.top = CurrentRegionRect(g_GetDcRegionIndex).top > rect->top ? CurrentRegionRect(g_GetDcRegionIndex).top : rect->top;
                    g_GetDcClipRect.bottom = CurrentRegionRect(g_GetDcRegionIndex).bottom < rect->bottom ? CurrentRegionRect(g_GetDcRegionIndex).bottom : rect->bottom;
                    if (g_GetDcClipRect.left < g_GetDcClipRect.right &&
                        g_GetDcClipRect.top < g_GetDcClipRect.bottom) {
                        if (field_0x68 && g_GetDcRegionIndex > field_0x64) {
                            surface = Target()->renderSurface;
                            field_0x68 = 0;
                        } else if (!field_0x68 && g_GetDcRegionIndex == field_0x64) {
                            surface = offscreenCopy;
                            field_0x68 = 1;
                        } else {
                            continue;
                        }
                        *a = 1;
                        *clip = g_GetDcClipRect;
                        field_0x64 = g_GetDcRegionIndex;
                        break;
                    }
                }
                if (g_GetDcRegionIndex == regionCapacity) {
                    field_0x68 = 1;
                    field_0x64 = -1;
                }
            }
        } else {
            surface = Target()->renderSurface;
            *frames = Target()->frameModulus;
        }
        if (index >= 0)
            MarkRegionPending(index);
    } else {
        if (index >= 0)
            SetRegionRect(index, rect);
        surface = Target()->renderSurface;
        *frames = Target()->frameModulus;
    }
    if (surface) {
        if (heldDcSurface) {
            if (surface != heldDcSurface) {
                ReleaseDc();
                heldDcSurface = surface;
                surface->GetDC(&heldDc);
                *dc = heldDc;
                return 1;
            }
        } else {
            heldDcSurface = surface;
            surface->GetDC(&heldDc);
        }
        *dc = heldDc;
        return 1;
    }
    return 0;
}
