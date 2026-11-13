// Complete behavioral candidate for 0x004cab00 (1129 retail bytes).
// Original translation-unit ownership is not established by this grouping.
// See docs/PARTIAL_TEXTURE_UPLOADS.md for validation and remaining differences.
//
// Standalone VC6 SP3 vc6_o2_mt: 423 of 1129 strict positions (candidate 1120
// bytes; was 140 of 1145). Matches retail instruction for instruction up to
// 0x004cac5c. What moved codegen:
// - The method returns `result`. Retail leaves it in eax at the single
//   `ret 4` (`mov eax, [esp+0x10]` before the +0x5bc store). As `void`,
//   VC6 duplicated the epilogue for each jump to the final store.
// - `render = 0` with the supplied target taken in the then-arm, and the
//   created target assigned straight to `render`. The created target, `render`
//   and the mode index then share esi as in retail, and the exhausted loop
//   jumps directly to the final store.
// - The temporary-target cleanup sits after the `if (render)` block.
// Left: retail keeps `this` at [esp+0x14] and `image` only in memory at
// [esp+0x18], with ebx as the zero register throughout. Ours swaps the two
// slots and caches `image` in ebx. Retail also places the CreateTextureSurface
// success arm after the `ret`.
// Ruled out on this form (s21): permuting the image/expected/rendered
// declarations, or declaring `image` at its assignment (both unchanged);
// swapped CreateTextureSurface arms (150, size 1129); FreeFile once after a
// named CreateTextureSurface result (397); braces on the -2 arm (unchanged).
// Earlier: `render = 0` inside `if (!render)` (unchanged); an early store and
// return on exhaustion (324, adds a return retail lacks).
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/PCVideoCard.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/D3DConstants.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Tgafile.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x004cab00: the "PartialTexBlt" test (+0x5bc): uploads 64 x 64 tiles of
// partblt.tga through a texture and compares the drawn result. 1: works;
// 0: wrong; -1: the full upload failed; -2: no image or texture; -3: no
// 640 x 480 x 16 target; -4: no texture surface.
int PCVideoCard::ProbePartialTextureUploads(RenderTarget* target) {
    int result = 1;
    Win32Rect area;
    Win32Rect tile;
    PCTextureMap* image;
    PCTextureMap* expected;
    PCTextureMap* rendered;

    area.left = 0;
    area.top = 0;
    area.right = 256;
    area.bottom = 256;
    rendered = 0;
    expected = 0;
    TgaFile* file = LoadFile("partblt.tga", 0, 0);
    RenderTarget* render = 0;
    if (target)
        render = target;
    else {
        for (int i = 0; i < displayModeCount; i++) {
            if (displayModes[i].width == 640 && displayModes[i].height == 480 && displayModes[i].bitDepth == 16) {
                if (!SetFullscreenDisplayMode(i, 1)) {
                    result = -3;
                } else {
                    render = (new (__FILE__, 2053) PCRenderTarget)
                                 ->InitializeRenderTarget(this, &IID_IDirect3DHALDevice, backBuffer, 1, frameBufferCount);
                    if (render) {
                        result = 1;
                        g_TrackGame->renderTarget = render;
                        g_TrackGame->display = this;
                        break;
                    }
                    result = -3;
                }
            }
        }
    }
    if (render) {
        if (file) {
            image = new (__FILE__, 2078) PCTextureMap(0, 0);
            if (!image->CreateSurface(file->bits, file->width, file->height, file->width, file->width,
                                      file->bitsPerPixel == 16 ? 0x22b : 0x378, render->surfacePixelFormat, 0, 0,
                                      render->targetDisplay->field_0x198, 0, 0, 2, 1, 0, 0x80, 0xff00ff)) {
                FreeFile(file);
                result = -2;
            } else {
                expected = new (__FILE__, 2097) PCTextureMap(0, 0);
                expected->CreateSurface(0, file->width, file->height, file->width, file->width,
                                        render->surfacePixelFormat, render->surfacePixelFormat, 0, 0,
                                        render->targetDisplay->field_0x198, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
                rendered = new (__FILE__, 2110) PCTextureMap(0, 0);
                rendered->CreateSurface(0, file->width, file->height, file->width, file->width,
                                        render->surfacePixelFormat, render->surfacePixelFormat, 0, 0,
                                        render->targetDisplay->field_0x198, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
                if (!rendered->CreateTextureSurface(1, 0, 1)) {
                    FreeFile(file);
                    result = -4;
                } else {
                    FreeFile(file);
                    if (TestPartialTextureUpload((PCRenderTarget*)render, &area, image, expected, rendered, this, 1) != 1) {
                        result = -1;
                    } else {
                        for (int y = 0; result == 1 && y < 256; y += 64) {
                            tile.top = y;
                            tile.bottom = y + 64;
                            for (int x = 0; result == 1 && x < 256; x += 64) {
                                tile.left = x;
                                tile.right = x + 64;
                                if (!TestPartialTextureUpload((PCRenderTarget*)render, &tile, image, expected, rendered,
                                                           this, 1))
                                    result = 0;
                            }
                        }
                    }
                }
            }
            if (image)
                image->Release();
            if (rendered)
                rendered->Release();
            if (expected)
                expected->Release();
        } else
            result = -2;
    }
    if (!target && render) {
        delete render;
        g_TrackGame->renderTarget = 0;
        g_TrackGame->display = 0;
    }
    partialTextureUploadResult = result;
    return result;
}
