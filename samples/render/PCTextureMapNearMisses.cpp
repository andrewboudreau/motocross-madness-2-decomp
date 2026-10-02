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
//
// PCTextureMap::UnknownVirtualSlot6 (0x004c71c0, 600 bytes): the copy.
// Control flow, both debug `new`s (lines 976/986), the slot 4 and slot 18
// calls and the folded 555/565 key expansion line up. Retail keeps the
// constant 1 in ebx and the copy in ebp, and also stores `field_0x68 & 1`
// to a frame slot. With a `cached` local VC6 here stores it but loses the
// ebx constant; without it, it keeps the constant but drops the store.
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
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

// 0x004c71c0: a copy of the texture (a CacheTexture registered with this
// one's cache, or a PCTextureMap of the game's manager), filled from +0x70
// through its slot 4 and given the same colour key; 0 on failure.
TextureMap* PCTextureMap::UnknownVirtualSlot6() {
    TextureMap* copy;
    TextureMapManager* manager;
    int cached = field_0x68 & 1;
    if (cached) {
        manager = field_0x10;
        CacheTexture* cache = new(__FILE__, 976) CacheTexture(manager);
        cache->field_0x68 |= 1;
        ((CacheTexture*)this)->field_0x90->UnknownFunction50c6c0(cache);
        copy = cache;
    } else {
        manager = g_UnknownGlobal56e26c->field_0x3c;
        copy = new(__FILE__, 986) PCTextureMap(manager, 1);
    }
    if (field_0x70) {
        UnknownSurfaceDesc desc;
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (!field_0x70->UnknownMethod25(0, &desc, 0x811, 0)) {
            int flags = (desc.caps[0] & 0x1000 ? 0 : 4) | (desc.caps[0] & 0x30000000 ? 0 : 8);
            int created = copy->UnknownVirtualSlot4(desc.surface, field_0x14, field_0x18,
                                                    desc.pitch / UnknownFunction511970(field_0x20), field_0x1c,
                                                    field_0x20, field_0x20, field_0x2c, flags, field_0x78, 0, 0,
                                                    2, 1, 0, 0x80, 0xff00ff);
            if (!field_0x70->UnknownMethod32(0) && created) {
                if (field_0x30) {
                    if (field_0x20 == 8) {
                        copy->UnknownVirtualSlot18((field_0x2c->field_0x010[field_0x34][0] << 8 |
                                                    field_0x2c->field_0x010[field_0x34][1]) << 8 |
                                                   field_0x2c->field_0x010[field_0x34][2]);
                    } else {
                        unsigned int key = field_0x34;
                        unsigned int color;
                        if (field_0x20 == 0x22b)
                            color = (key & 0x7c00) << 9 | (key & 0x3e0) << 6 | (key & 0x1f) << 3;
                        else
                            color = (key & 0xf800) << 8 | (key & 0x7e0) << 5 | (key & 0x1f) << 3;
                        copy->UnknownVirtualSlot18(color);
                    }
                }
                return copy;
            }
        }
    }
    delete copy;
    return 0;
}
