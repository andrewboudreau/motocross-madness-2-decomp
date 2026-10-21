// Complete nonexact display-mode surface setup; original TU grouping is not asserted.
#include "../../src/reconstructed/Display.h"

// 0x004ca520: the surfaces for the current mode; full screen needs the
// whole chain plus two 16-bit frames' worth of video memory.
int UnknownDisplay::CreateModeSurfaces(int fullScreen) {
    int width = displayModes[currentDisplayMode].width;
    int height = displayModes[currentDisplayMode].height;
    int bitDepth = displayModes[currentDisplayMode].bitDepth;
    int buffers = displayModes[currentDisplayMode].field_0x10;
    if (fullScreen) {
        if (width * height * bitDepth / 8 * buffers > videoMemoryBudget - width * height * 2)
            return 0;
        if (CreateFlipChain(buffers - 1))
            return 1;
    } else if (CreateSystemRenderSurface(buffers - 1)) {
        return 1;
    }
    return 0;
}
