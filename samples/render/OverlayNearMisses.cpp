// Overlay::RestoreRect, complete retail 0x004b6710..0x004b687c (365 bytes).
// Reviewed functional candidate; emitted code remains nonexact. See
// docs/OVERLAY_RESTORE_RECT.md for the ordered-read repair and finite proof.
#include <string.h>

#include "../../src/reconstructed/Overlay.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x004b6710. A rectangle as wide as the texture is copied in one block.
int Overlay::RestoreRect(const UnknownOverlayRect* rect) {
    // Model the decoded left/top/right/bottom access order, including faults.
    // This qualification describes reconstructed behavior, not an original type.
    const volatile UnknownOverlayRect* initial = rect;
    int initialLeft = initial->left;
    int initialTop = initial->top;
    int initialRight = initial->right;
    int initialBottom = initial->bottom;
    int height = initialBottom - initialTop;
    int width = initialRight - initialLeft;
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

    int top = rect->top;
    if (whole) {
        memcpy(bits + top * pitch, source + top * sourcePitch, bytes);
    } else {
        int offset = top * sourcePitch;
        offset += PixelFormatBytesPerPixel(overlayTexture->mapFormat) * rect->left;
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
