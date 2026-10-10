// Near-miss CacheTexture candidates, kept out of src/reconstructed until they
// match. See docs/TEXTUREMAPMANAGER.md.
//
// CacheTexture::UnknownFunction5102d0 (0x005102d0, 558 bytes): blits the
// occupant's mip levels into its region. Everything lines up except that
// retail places the shared `return 0` block between the second loop and the
// overlay check (its loop ends `jle; jmp top`); VC6 here puts it after the
// final returns. 18 instructions differ, in jump targets and the loop end.

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
int CacheTexture::UnknownFunction5102d0(UnknownTextureRegion* region, int level) {
    region->field_0x14->UnknownFunction510610(level);
    float scale = 1.0f / (1 << level);
    int width = field_0x14;
    UnknownRect rect;
    rect.left = (long)(width * region->field_0x1c);
    rect.top = (long)(field_0x18 * region->field_0x20);
    rect.right = (long)(((region->field_0x24 - region->field_0x1c) * scale + region->field_0x1c) * width);
    rect.bottom = (long)(((region->field_0x28 - region->field_0x20) * scale + region->field_0x20) * field_0x18);
    int side = rect.right - rect.left;
    int levelWidth = width;
    UnknownRect whole = rect;
    UnknownSurfaceInterface* source = region->field_0x14->systemSurface;
    UnknownSurfaceInterface* destination = systemSurface;
    UnknownSurfaceInterface* nextSource = 0;
    UnknownSurfaceInterface* nextDestination = 0;
    int sourceSide = region->field_0x14->field_0x14;
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
            field_0x80->field_0x1d0 += (rect.bottom - rect.top) * (rect.right - rect.left);
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
        UnknownVirtualSlot9(&whole, -1);
    else
        field_0x188 = 1;
    return 1;
}

