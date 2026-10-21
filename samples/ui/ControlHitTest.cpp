// Complete nonexact menu hit tests; original TU ownership is unproven.
// Full retail extents: UIButton354 bytes, UIMultiState329 bytes.
// See docs/CONTROL_HIT_TEST.md.
#include <windows.h>
#include "../../src/reconstructed/UIDialog.h"
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x004734c0: whether `point` is on an opaque pixel of the image.
int UIButton::ContainsPoint(POINT point, int state) {
    int opaque = 0;
    int format = g_TrackGame->renderTarget->surfacePixelFormat;
    RECT rect = *(RECT*)screenArea;
    if (PtInRect(&rect, point)) {
        point.x -= rect.left;
        point.y -= rect.top;
        int x = (int)(point.x / ownerDialog->scaleX);
        int y = (int)(point.y / ownerDialog->scaleY);
        UnknownSurfaceDesc desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)GetStateTexture(-1);
        if (!texture->systemSurface->Lock(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / PixelFormatBytesPerPixel(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.field_0x28[0x18])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.field_0x28[0x18])
                    opaque = 1;
            }
            if (texture->systemSurface->Unlock(0))
                return opaque;
        }
        return opaque;
    }
    return 0;
}

// 0x00478e10: the same test as UIButton slot 28 (0x004734c0), but on the
// control's own area (no RECT copy: retail passes &screenArea to PtInRect).
int UIMultiState::ContainsPoint(POINT point, int state) {
    int opaque = 0;
    int format = g_TrackGame->renderTarget->surfacePixelFormat;
    if (PtInRect((RECT*)screenArea, point)) {
        point.x -= ((RECT*)screenArea)->left;
        point.y -= ((RECT*)screenArea)->top;
        int x = (int)(point.x / ownerDialog->scaleX);
        int y = (int)(point.y / ownerDialog->scaleY);
        UnknownSurfaceDesc desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)GetStateTexture(-1);
        if (!texture->systemSurface->Lock(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / PixelFormatBytesPerPixel(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.field_0x28[0x18])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.field_0x28[0x18])
                    opaque = 1;
            }
            if (texture->systemSurface->Unlock(0))
                return opaque;
        }
        return opaque;
    }
    return 0;
}
