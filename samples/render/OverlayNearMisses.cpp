// Near-miss overlay.cpp candidate, kept out of src/reconstructed until it
// matches (Overlay.cpp holds the rest of the file).
//
// Overlay::RestoreRect (0x004b6710, 361 bytes): `top` read once before
// the copy branch (retail loads rect->top ahead of the `whole` test) frees
// esi for `this` and reloads `rect` from the stack as retail does; height
// before width gives retail's subtrahend-first loads (53 -> 63 of 361).
// Left: retail keeps `whole` and the byte count on the stack (6 slots,
// the else branch's offset sharing whole's slot), the destination in ebp
// and the row count in eax; VC6 here keeps `whole` in edi and the byte
// count in ebp, and the row count and destination on the stack. Count-down
// for/while/do-while loops, `whole` set first, set from the comparison or
// preset to 0, and a bytes-per-pixel local do not get that.
#include <string.h>

#include "../../src/reconstructed/Overlay.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x004b6710. A rectangle as wide as the texture is copied in one block.
int Overlay::RestoreRect(const UnknownOverlayRect* rect) {
    int height = rect->bottom - rect->top;
    int width = rect->right - rect->left;
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
