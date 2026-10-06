// Near-miss CacheTexture candidates, kept out of src/reconstructed until they
// match. See docs/TEXTUREMAPMANAGER.md.
//
// CacheTexture::UnknownFunction5102d0 (0x005102d0, 558 bytes): blits the
// occupant's mip levels into its region. Everything lines up except that
// retail places the shared `return 0` block between the second loop and the
// overlay check (its loop ends `jle; jmp top`); VC6 here puts it after the
// final returns. 18 instructions differ, in jump targets and the loop end.
//
// CacheTexture::UnknownFunction50f9b0 (0x0050f9b0, 646 bytes): reserves the
// regions a plan wants. Only two register choices differ: retail keeps
// `this` in ebx and the free texels in ebp during the first loop (VC6 here
// swaps them), and the final walk addresses its stack entries through the
// index field rather than the region field. 11 instructions differ.

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
    UnknownSurfaceInterface* source = region->field_0x14->field_0x70;
    UnknownSurfaceInterface* destination = field_0x70;
    UnknownSurfaceInterface* nextSource = 0;
    UnknownSurfaceInterface* nextDestination = 0;
    int sourceSide = region->field_0x14->field_0x14;
    UnknownSurfaceCaps caps;
    memset(&caps, 0, sizeof(caps));
    caps.caps = 0x401000;
    long result = 0;
    while (sourceSide > side) {
        result = source->UnknownMethod12(&caps, &nextSource);
        if (result)
            break;
        source = nextSource;
        sourceSide >>= 1;
    }
    if (!result) {
        while (sourceSide > 0) {
            if (destination->UnknownMethod5(&rect, source, 0, 0x1000000, 0))
                return 0;
            field_0x80->field_0x1d0 += (rect.bottom - rect.top) * (rect.right - rect.left);
            long sourceResult = source->UnknownMethod12(&caps, &nextSource);
            long destinationResult = destination->UnknownMethod12(&caps, &nextDestination);
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
    if (g_UnknownGlobal56e26c->field_0x0c->field_0x5bc > 0)
        UnknownVirtualSlot9(&whole, -1);
    else
        field_0x188 = 1;
    return 1;
}

// A step of the depth-first walk in 0x0050f9b0.
struct UnknownRegionStep {
    UnknownTextureRegion* region;
    int index;
};

// The largest level still wanted, taken off `wanted`; -1 when none is.
static inline int TakeLargest(int* wanted) {
    for (int level = 8; level >= 0; level--) {
        if (wanted[level]) {
            wanted[level]--;
            return level;
        }
    }
    return -1;
}

// 0x0050f9b0
int CacheTexture::UnknownFunction50f9b0(int* levels, int count) {
    int wanted[9];
    memset(wanted, 0, sizeof(wanted));
    int space = field_0x184;
    int fitted = 1;
    int level;
    for (level = 8; level >= 0; level--) {
        int n = levels[level];
        while (n) {
            int area = LevelArea(level);
            if (space < area) {
                fitted = 0;
                break;
            }
            space -= area;
            wanted[level]++;
            levels[level]--;
            n--;
        }
    }
    int same = 1;
    for (level = 0; level <= 8; level++) {
        if (field_0xac[level] != wanted[level]) {
            same = 0;
            break;
        }
    }
    if (same)
        return fitted;

    UnknownRegionStep steps[10];
    UnknownTextureRegion* region = field_0x84;
    int depth = -1;
    int first = 1;
    for (;;) {
        level = TakeLargest(wanted);
        if (level == -1)
            break;
        if (!first) {
            while (steps[depth].index++ == 3)
                depth--;
            region = steps[depth].region->field_0x00[steps[depth].index];
        } else {
            first = 0;
        }
        while (region->field_0x18 > level) {
            depth++;
            steps[depth].region = region;
            steps[depth].index = 0;
            region = UnknownFunction50fc60(region);
        }
        UnknownFunction50fd60(region);
        if (region->field_0x2c)
            continue;
        region->field_0x2c = 1;
        field_0xac[level]++;
        field_0x88[level]--;
        field_0xd0[level].Add(region);
    }
    for (; depth >= 0; depth--) {
        int i = steps[depth].index + 1;
        UnknownTextureRegion* parent = steps[depth].region;
        for (; i < 4; i++) {
            UnknownTextureRegion* quarter = parent->field_0x00[i];
            if (quarter->field_0x00[0])
                UnknownFunction50fd60(quarter);
            else
                UnknownFunction50fc90(quarter);
        }
    }
    return fitted;
}
