// Complete nonexact BackgroundImage::GetDc candidate; original TU ownership
// is unproven. Retail and VC6 bodies are both 678 bytes, with 115 matching
// positions after all relocation bindings. Zero-register allocation and DC
// return-block placement still differ. See docs/BACKGROUND_DC.md.
// k-scan (s22): compiled behind 0..63 prepended typedefs, 0x004049d0 has the
// same code at every count, so its difference is not a declaration-count
// effect (docs/VC6_OPERAND_ORDER.md section 2).

#include <string.h>

#include "../../src/reconstructed/BackgroundImage.h"

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCVideoCard.h"
#include "../../src/reconstructed/DisplayMode.h"
#include "../../src/reconstructed/TrackGame.h"

#define Target() ((PCRenderTarget*)renderContext)

extern CameraRect g_BackgroundImageRect;
extern CameraRect g_DrawImageClipRect;
extern CameraRect g_DrawImageSourceRect;
// 0x005777a0 / 0x005777a8: GetDc's dirty-region index and clipped rectangle;
// decoded direct DWORD references lie within GetDc. Both retail ranges start
// at raw-backed zero bytes in writable .data; original COFF placement is unknown.
int g_GetDcRegionIndex;
CameraRect g_GetDcClipRect;

// 0x004049d0: a DC for drawing `rect`: the back buffer, or with the copy
// current the surfaces in turn (`more` set when the copy is returned), clipped
// to the next dirty region in `clip`.
int BackgroundImage::GetDc(void** dc, CameraRect* rect, int index, int dirty, int* frames, int* more,
                                           CameraRect* clip) {
    DirectDrawSurface* surface;
    *dc = 0;
    *more = 0;
    surface = 0;
    if (dirty) {
        if (copyValid) {
            if (*frames) {
                if (copyDcNext) {
                    surface = offscreenCopy;
                    *more = 1;
                    copyDcNext = 0;
                } else {
                    surface = Target()->renderSurface;
                    copyDcNext = 1;
                    (*frames)--;
                }
            } else {
                copyDcNext = 1;
                for (g_GetDcRegionIndex = 0; g_GetDcRegionIndex < regionCapacity; g_GetDcRegionIndex++) {
                    if (regionTable[g_GetDcRegionIndex].lastFrameCount != Target()->frameCount)
                        continue;
                    g_GetDcClipRect.left = CurrentRegionRect(g_GetDcRegionIndex).left > rect->left ? CurrentRegionRect(g_GetDcRegionIndex).left : rect->left;
                    g_GetDcClipRect.right = CurrentRegionRect(g_GetDcRegionIndex).right < rect->right ? CurrentRegionRect(g_GetDcRegionIndex).right : rect->right;
                    g_GetDcClipRect.top = CurrentRegionRect(g_GetDcRegionIndex).top > rect->top ? CurrentRegionRect(g_GetDcRegionIndex).top : rect->top;
                    g_GetDcClipRect.bottom = CurrentRegionRect(g_GetDcRegionIndex).bottom < rect->bottom ? CurrentRegionRect(g_GetDcRegionIndex).bottom : rect->bottom;
                    if (g_GetDcClipRect.left < g_GetDcClipRect.right &&
                        g_GetDcClipRect.top < g_GetDcClipRect.bottom) {
                        if (lastDcRegionCopied && g_GetDcRegionIndex > lastDcRegion) {
                            surface = Target()->renderSurface;
                            lastDcRegionCopied = 0;
                        } else if (!lastDcRegionCopied && g_GetDcRegionIndex == lastDcRegion) {
                            surface = offscreenCopy;
                            lastDcRegionCopied = 1;
                        } else {
                            continue;
                        }
                        *more = 1;
                        *clip = g_GetDcClipRect;
                        lastDcRegion = g_GetDcRegionIndex;
                        break;
                    }
                }
                if (g_GetDcRegionIndex == regionCapacity) {
                    lastDcRegionCopied = 1;
                    lastDcRegion = -1;
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
