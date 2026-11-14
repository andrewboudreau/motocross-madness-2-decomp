// Overlay::RestoreRect, complete retail 0x004b6710..0x004b687c (365 bytes).
// Reviewed functional candidate; emitted code remains nonexact. See
// docs/OVERLAY_RESTORE_RECT.md for the ordered-read repair and finite proof.
// k-scan (s22): compiled behind 0..63 prepended typedefs, 0x004b6710 has the
// same code at every count, so its difference is not a declaration-count
// effect (docs/VC6_OPERAND_ORDER.md section 2).
#include <string.h>

#include "../../src/reconstructed/Overlay.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x004b6710. A rectangle as wide as the texture is copied in one block.
int Overlay::RestoreRect(const OverlayRect* rect) {
    // Model the decoded left/top/right/bottom access order, including faults,
    // and the two rereads after the lock callbacks. This qualification
    // describes reconstructed behavior, not an original type.
    const volatile OverlayRect* view = rect;
    int left = view->left;
    int initialTop = view->top;
    int width = view->right - left;
    int height = view->bottom - initialTop;
    int whole;
    int bytes;
    if (width == overlayTexture->textureWidth) {
        bytes = PixelFormatBytesPerPixel(overlayTexture->mapFormat) * overlayTexture->textureWidth * height;
        whole = 1;
    } else {
        bytes = PixelFormatBytesPerPixel(overlayTexture->mapFormat) * width;
        whole = 0;
    }

    long sourcePitch, pitch;
    unsigned char* source = (unsigned char*)backingTexture->LockSurface(0, &sourcePitch, 0x811);
    unsigned char* bits = (unsigned char*)overlayTexture->LockSurface(0, &pitch, 0x801);
    if (!source || !bits)
        return 0;

    int top = view->top;
    if (whole) {
        memcpy(bits + top * pitch, source + top * sourcePitch, bytes);
    } else {
        int offset = top * sourcePitch;
        offset += PixelFormatBytesPerPixel(overlayTexture->mapFormat) * view->left;
        source += offset;
        bits += offset;
        for (int y = 0; y < height; y++) {
            memcpy(bits, source, bytes);
            source += sourcePitch;
            bits += pitch;
        }
    }
    backingTexture->UnlockSurface(0);
    overlayTexture->UnlockSurface(0);
    return 1;
}
