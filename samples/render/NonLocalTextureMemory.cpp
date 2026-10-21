// Complete nonexact candidate for 0x004ca5a0 (486 retail bytes).
// The earlier 484-byte target cut through the final ret 8 instruction.
// See docs/NONLOCAL_TEXTURE_MEMORY.md for evidence and validation limits.
#include <string.h>
#include "../../src/reconstructed/PCVideoCard.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCRenderTarget.h"

void FillPixelFormat(int format, void* pixelFormat);

// 0x004ca5a0: whether a 128 x 128 texture of the target's format can be
// created (with a temporary 640 x 480 x 16 target when `target` is 0).
int UnknownDisplay::ProbeNonLocalTextureMemory(int* value, RenderTarget* target) {
    RenderTarget* render = 0;
    UnknownSurfaceInterface* surface;
    UnknownSurfaceCaps caps;
    UnknownSurfaceDesc desc;
    int result;

    *value = 0;
    if (target) {
        render = target;
    } else {
        for (int i = 0; i < displayModeCount; i++) {
            if (displayModes[i].width == 640 && displayModes[i].height == 480 && displayModes[i].bitDepth == 16) {
                if (SetFullscreenDisplayMode(i, 1)) {
                    render = (new (__FILE__, 1185) PCRenderTarget)
                                 ->InitializeRenderTarget(this, &IID_IDirect3DHALDevice, backBuffer, 1,
                                                         frameBufferCount);
                    if (!render)
                        goto failed;
                }
                break;
            }
        }
    }
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    FillPixelFormat(render->surfacePixelFormat, &desc.pixelFormat);
    desc.width = 128;
    desc.height = 128;
    desc.flags = 0x1007;                          // caps, height, width, pixel format
    desc.caps[0] = 0x20005000;
    if (directDraw->CreateSurface(&desc, &surface, 0) == 0) {
        if (surface->GetCaps(&caps)) {
            *value = 0;
            result = 0;
        } else if (caps.caps & 0x20000000) {
            result = 1;
        } else {
            *value = 0;
            result = 0;
        }
        surface->Release();
        if (!target && render)
            delete render;
        return result;
    }
    if (!target && render)
        delete render;
failed:
    *value = 0;
    return 0;
}
