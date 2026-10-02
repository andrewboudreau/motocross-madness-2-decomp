// Near-miss PCTextureMap candidates, kept out of src/reconstructed until
// they match. See docs/PCTEXTUREMAP.md.
//
// PCTextureMap::UnknownFunction4c7e30 (0x004c7e30, 189 bytes): the colour
// key conversion. Retail packs each component as `(color >> n) & mask`
// joined with `or`. VC6 here factors the common shift out of every `|`
// form tried (grouping, order, mask-first, component and inline-helper
// forms); `+` keeps retail's shape but emits `add`.
//
// PCTextureMap::UnknownVirtualSlot9 (0x004c7640, 385 bytes): the upload.
// With separate `next` surfaces VC6 packs them into the dead parameter
// slots as retail does (the frame matches), but retail keeps `this` in ebp
// from the start, the area in registers across the level loop and tests
// the loop at the top on every pass; VC6 here rotates the loop and pushes
// ebp late. while, for(;;) and goto loops compile alike. 117 of 385 bytes.
#include <string.h>

#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/TrackGame.h"

#include "../../src/reconstructed/PCTextureMap.h"

// 0x004c7e30: converts a 24-bit colour to the texture's format (555, 565 or
// a palette index) and stores it as the colour key.
void PCTextureMap::UnknownFunction4c7e30(unsigned int color) {
    int key;
    if (field_0x20 == 0x22b)
        key = (color >> 9) & 0x7c00 | (color >> 6) & 0x3e0 | (color >> 3) & 0x1f;
    else if (field_0x20 == 0x235)
        key = (color >> 8) & 0xf800 | (color >> 5) & 0x7e0 | (color >> 3) & 0x1f;
    else if (field_0x20 == 8)
        key = field_0x2c->field_0x710[(color >> 9) & 0x7c00 | (color >> 6) & 0x3e0 | (color >> 3) & 0x1f];
    else
        key = color;
    field_0x34 = field_0x38 = key;
}

// 0x004c7640: with partial texture blits (Display+0x5bc) or a positive
// `mode`, copies `rect` (or the whole texture) down the mip chain with
// BltFast, halving it per level; otherwise lets the device Load it. Then
// applies the colour key.
int PCTextureMap::UnknownVirtualSlot9(UnknownRect* rect, int mode) {
    UnknownSurfaceInterface* source = field_0x70;
    UnknownSurfaceInterface* destination = field_0x74;
    if (!source || !destination)
        return 0;
    if (mode > 0 || mode == -1 && g_UnknownGlobal56e26c->field_0x0c->field_0x5bc > 0 ||
        g_UnknownGlobal56e26c->field_0x0c->field_0x5bc <= 0) {
        UnknownRect area;
        if (rect && g_UnknownGlobal56e26c->field_0x0c->field_0x5bc > 0) {
            area = *rect;
        } else {
            area.left = 0;
            area.top = 0;
            area.right = field_0x14;
            area.bottom = field_0x18;
        }
        UnknownSurfaceCaps caps;
        memset(&caps, 0, sizeof(caps));
        caps.caps = 0x401000;
        while (area.right - area.left > 0 && area.bottom - area.top > 0) {
            if (destination->UnknownMethod7(area.left, area.top, source, &area, 0x10))
                return 0;
            UnknownSurfaceInterface* nextSource;
            UnknownSurfaceInterface* nextDestination;
            long sourceResult = source->UnknownMethod12(&caps, &nextSource);
            long destinationResult = destination->UnknownMethod12(&caps, &nextDestination);
            if (sourceResult || destinationResult)
                break;
            source = nextSource;
            destination = nextDestination;
            area.top >>= 1;
            area.left >>= 1;
            area.bottom >>= 1;
            area.right >>= 1;
        }
        if (field_0x30 && field_0x74->UnknownMethod29(8, &field_0x34))
            return 0;
        return 1;
    }
    if (destination != source)
        ((PCRenderTarget*)g_UnknownGlobal56e26c->field_0x10)
            ->field_0x50->UnknownMethod43(destination, 0, source, 0, 0);
    return 0;
}
