// Near-miss PCTextureMap candidates, kept out of src/reconstructed until
// they match. See docs/PCTEXTUREMAP.md.
//
// PCTextureMap::UnknownFunction4c83a0 (0x004c83a0, 142 bytes): retail keeps
// the attached surface in the dead `width` parameter's stack slot (its frame
// is 4 bytes smaller) and clears the capabilities in another order.
//
// PCTextureMap::UnknownVirtualSlot20 (0x004c8430, 161 bytes): the
// capabilities are cleared as caps = 0, caps = 0x401000, then the other
// three, after +0x70 is loaded. memset, `= {0}` and field-by-field forms
// place the stores differently.
#include <string.h>

#include "../../src/reconstructed/PCTextureMap.h"

// 0x004c83a0: follows the attached mip surfaces, halving the width, until it
// reaches `width`.
UnknownSurfaceInterface* PCTextureMap::UnknownFunction4c83a0(int width) {
    UnknownSurfaceCaps caps;
    memset(&caps, 0, sizeof(caps));
    int size = field_0x14;
    UnknownSurfaceInterface* surface = field_0x70;
    caps.caps = 0x401000;
    if (size != width) {
        long result;
        do {
            result = surface->UnknownMethod12(&caps, &surface);
            if (result)
                break;
            size /= 2;
        } while (size != width);
        if (result && result != (long)0x887600ff) {
            UnknownReportDirectDrawError(result, __FILE__, 2034);
            return 0;
        }
        if (size != width)
            return 0;
    }
    return surface;
}

// 0x004c8430
int PCTextureMap::UnknownVirtualSlot20() {
    if (field_0x70) {
        UnknownFunction4c84e0(field_0x70, 0);
        if (field_0x24 > 1) {
            UnknownSurfaceInterface* surface;
            UnknownSurfaceCaps caps;
            memset(&caps, 0, sizeof(caps));
            caps.caps = 0x401000;
            long result = field_0x70->UnknownMethod12(&caps, &surface);
            while (!result) {
                UnknownFunction4c84e0(surface, 0);
                result = surface->UnknownMethod12(&caps, &surface);
            }
            if (result != (long)0x887600ff) {
                UnknownReportDirectDrawError(result, __FILE__, 2084);
                return 0;
            }
        }
    }
    return 1;
}

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

