// Complete nonexact texture rectangle copy; original TU ownership is unknown.
// See docs/TEXTURE_RECT_COPY.md for behavior and low-address fault boundaries.
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/TrackGame.h"

#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/D3DConstants.h"

// 0x004c7b40: copies `rect` of +0x70 to (x, y) in `destination`. Without a
// table this is BltFast; with one, both surfaces are locked and each
// destination pixel becomes table[source << 8 | destination], clipped to
// the destination. Every non-8-bit format follows the WORD path and reads
// a byte through the destination WORD value, including its low-address fault.
int PCTextureMap::CopyRectTo(unsigned long x, unsigned long y, UnknownSurfaceInterface* destination,
                                        UnknownRect* rect, int flags, unsigned char* table) {
    UnknownSurfaceDesc source;
    UnknownSurfaceDesc target;
    long left, top, right, bottom;
    if (!table) {
        if (destination->BltFast(x, y, systemSurface, rect, flags))
            goto failed;
        return 1;
    }
    memset(&source, 0, sizeof(source));
    source.size = sizeof(source);
    if (systemSurface->Lock(0, &source, 0x811, 0))
        goto failed;
    memset(&target, 0, sizeof(target));
    target.size = sizeof(target);
    if (destination->Lock(0, &target, 0x811, 0))
        goto failed;
    top = rect->top;
    bottom = rect->bottom;
    left = rect->left;
    right = rect->right;
    if (x < target.width && y < target.height) {
        if (right - left + x + 1 >= target.width)
            right = target.width - x + left - 2;
        if (bottom - top + y + 1 >= target.height)
            bottom = target.height - y + top - 2;
        if (target.pixelFormat.bitCount == 8) {
            unsigned char* from = (unsigned char*)source.surface + source.pitch * top + left;
            unsigned char* to = (unsigned char*)target.surface + target.pitch * y + x;
            for (long row = top; row <= bottom; row++) {
                unsigned char* pixel = to;
                for (long column = left; column <= right; column++, pixel++)
                    *pixel = table[(from[column - left] << 8) + *pixel];
                from += source.pitch;
                to += target.pitch;
            }
        } else {
            unsigned short* from = (unsigned short*)source.surface + source.pitch * top / 2 + left;
            unsigned short* to = (unsigned short*)target.surface + target.pitch * y / 2 + x;
            for (long row = top; row <= bottom; row++) {
                unsigned short* pixel = to;
                for (long column = left; column <= right; column++, pixel++) {
                    // Preserve the observed destination-before-source access/fault order.
                    unsigned short destinationPixel = *(volatile unsigned short*)pixel;
                    unsigned short sourcePixel = *(volatile unsigned short*)(from + column - left);
                    *pixel = table[(sourcePixel << 8) + *(unsigned char*)destinationPixel];
                }
                from += source.pitch / 2;
                to += target.pitch / 2;
            }
        }
    }
    if (destination->Unlock(0))
        goto failed;
    if (systemSurface->Unlock(0))
        goto failed;
    return 1;
failed:
    return 0;
}
