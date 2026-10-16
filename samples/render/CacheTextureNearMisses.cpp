// Near-miss CacheTexture candidates, kept out of src/reconstructed until they
// match. See docs/TEXTUREMAPMANAGER.md.
//
// CacheTexture::CopyMipLevels (0x005102d0, 558 bytes): blits the
// occupant's mip levels into its region. Everything lines up except that
// retail places the shared `return 0` block between the second loop and the
// overlay check (its loop ends `jle; jmp top`); VC6 here puts it after the
// final returns. 18 instructions differ, in jump targets and the loop end.
// Tried (s10, all unchanged at 450/558 or worse): the Blt failure as an
// else arm or a goto into the else, `for (;;)`-style and for-increment
// loops, the inner break as an if/else, the first loop returning 0
// directly, an early `if (result) return 0;` (397), a do/while (293),
// the Blt result stored and tested after the loop (338) and `goto failed`
// to a label at the end or in an `if (result)` block (450-454). This is the
// known `jle exit; jmp top` loop-bottom shape that VC6 has not reproduced.

#include <string.h>

#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/PCGame.h"
#include "../../src/reconstructed/TrackGame.h"

static inline int LevelArea(int level) {
    if (level == 0)
        return 1;
    if (level < 0)
        return 0;
    int side = 2 << (level - 1);
    return side * side;
}

// 0x005102d0
int CacheTexture::CopyMipLevels(UnknownTextureRegion* region, int level) {
    region->occupant->UpdatePageCoords(level);
    float scale = 1.0f / (1 << level);
    int width = textureWidth;
    UnknownRect rect;
    rect.left = (long)(width * region->left);
    rect.top = (long)(textureHeight * region->top);
    rect.right = (long)(((region->right - region->left) * scale + region->left) * width);
    rect.bottom = (long)(((region->bottom - region->top) * scale + region->top) * textureHeight);
    int side = rect.right - rect.left;
    int levelWidth = width;
    UnknownRect whole = rect;
    UnknownSurfaceInterface* source = region->occupant->systemSurface;
    UnknownSurfaceInterface* destination = systemSurface;
    UnknownSurfaceInterface* nextSource = 0;
    UnknownSurfaceInterface* nextDestination = 0;
    int sourceSide = region->occupant->textureWidth;
    UnknownSurfaceCaps caps;
    memset(&caps, 0, sizeof(caps));
    caps.caps = 0x401000;
    long result = 0;
    while (sourceSide > side) {
        result = source->GetAttachedSurface(&caps, &nextSource);
        if (result)
            break;
        source = nextSource;
        sourceSide >>= 1;
    }
    if (!result) {
        while (sourceSide > 0) {
            if (destination->Blt(&rect, source, 0, 0x1000000, 0))
                return 0;
            ownerGroup->field_0x1d0 += (rect.bottom - rect.top) * (rect.right - rect.left);
            long sourceResult = source->GetAttachedSurface(&caps, &nextSource);
            long destinationResult = destination->GetAttachedSurface(&caps, &nextDestination);
            if (sourceResult || destinationResult)
                break;
            source = nextSource;
            destination = nextDestination;
            levelWidth >>= 1;
            rect.left >>= 1;
            sourceSide >>= 1;
            side >>= 1;
            rect.right >>= 1;
            rect.top >>= 1;
            rect.bottom >>= 1;
        }
    } else {
        return 0;
    }
    if (g_TrackGame->display->partialTextureUploadResult > 0)
        Upload(&whole, -1);
    else
        field_0x188 = 1;
    return 1;
}

