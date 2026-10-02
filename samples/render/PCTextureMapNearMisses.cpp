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

