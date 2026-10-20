// Near-miss CacheTexture candidates, kept out of src/reconstructed until they
// match. See docs/TEXTUREMAPMANAGER.md.
//
// CacheTexture::CopyMipLevels (0x005102d0, 558 bytes; ours 569): blits the
// occupant's mip levels into its region. Retail places the shared `return 0`
// block between the second loop and the overlay check (its loop ends
// `jle; jmp top`). Both failures as one early-failure test after the loops
// (the Blt result kept in `result` and `break`, then `if (result) return 0;`
// in place of the else arm) gives that block order: 16 -> 5 differing
// instructions (s14). Left: VC6 keeps `result` in a frame slot and tests
// it again at the loop's normal exit, where retail jumps straight to the
// overlay check. Unchanged at 5: `(result = Blt) != 0`, `int result`,
// `!= 0`. Worse: a separate Blt variable (16-17), `!result` in the loop
// condition (38-57), the early test without the Blt in `result` (16), the
// first loop returning directly (18). The tries below were made on the
// earlier form with the Blt `return 0` in the loop (16 diffs). The
// halvings are written sourceSide, side, levelWidth, left, right, top,
// bottom (the best of all 5040 orders, 450 -> 462 of 558, which matches
// retail's loads).
// Tried (s14, 450 at the old shift order): the Blt failure as `break`
// (the else's `return 0` still goes last), the else without braces,
// `goto failed` with `failed:` placed right after the loop and the
// overlay check behind `upload:` (450-454), `if (sourceSide > 0)` around
// a do/while or a bottom-tested for(;;) (293), a top-tested `for (;;)`
// and a trailing `continue` (450). With `if (result) { return 0; } else`
// VC6 keeps the `return 0` in place, so it is not always moved to the end.
// Tried (s10, all unchanged at 450/558 or worse): the Blt failure as an
// else arm or a goto into the else, `for (;;)`-style and for-increment
// loops, the inner break as an if/else, the first loop returning 0
// directly, an early `if (result) return 0;` (397), a do/while (293),
// the Blt result stored and tested after the loop (338) and `goto failed`
// to a label at the end or in an `if (result)` block (450-454). This is the
// known `jle exit; jmp top` loop-bottom shape that VC6 has not reproduced.
// Also 462 (s14, current order): the Blt `return 0` braced, with and
// without an unbraced else, the Blt result in a named local, and `!= 0`.
// Also 462 at the current order (s14 corpus pass): `goto failed` with
// `failed: return 0;` after the loop and the overlay check behind
// `upload:`, `failed:` inside the else arm, and the Blt `return 0` as an
// if arm with the rest of the body in its else. `if (result) { return 0;
// }` ahead of the loop (braced or not) gives 545 bytes. In the exact
// loop-bottom examples (docs/VC6_PATTERN_CORPUS.md) the early return is
// the body's last test; here it is the first.

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
            result = destination->Blt(&rect, source, 0, 0x1000000, 0);
            if (result)
                break;
            ownerGroup->texelsBlitted += (rect.bottom - rect.top) * (rect.right - rect.left);
            long sourceResult = source->GetAttachedSurface(&caps, &nextSource);
            long destinationResult = destination->GetAttachedSurface(&caps, &nextDestination);
            if (sourceResult || destinationResult)
                break;
            source = nextSource;
            destination = nextDestination;
            sourceSide >>= 1;
            side >>= 1;
            levelWidth >>= 1;
            rect.left >>= 1;
            rect.right >>= 1;
            rect.top >>= 1;
            rect.bottom >>= 1;
        }
    }
    if (result)
        return 0;
    if (g_TrackGame->display->partialTextureUploadResult > 0)
        Upload(&whole, -1);
    else
        field_0x188 = 1;
    return 1;
}

