// Near-miss Pixtrans.cpp candidate, kept out of src/reconstructed until it
// matches. See docs/PIXTRANS.md.
//
// UnknownFunction4cfaf0 (0x004cfaf0, 327 bytes): the 24-bit downsampler
// behind 0x004d1b90. Everything but the plain-copy path (levels == 0)
// matches: retail hoists width * 3, destinationStride * 3 and
// sourceStride * 3 in that order into the height, destination-stride and
// width argument slots; VC6 here computes the source step first and puts
// it in the source-stride slot. Advancing the parameters themselves is
// required (pointer locals cost 50 lines); reordering the advances, typed
// strides, a down-counting loop and for-increment advances do not fix the
// slots. Its 32-bit sibling 0x004cfc40 has the same copy path.

#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Pixtrans.h"

// 0x004cfaf0: shrinks 24-bit `source` by `levels` halvings into the width x
// height `destination` (0: a plain copy). Intermediate levels go through a
// buffer of the first level's size.
int UnknownFunction4cfaf0(void* destination, void* source, int width, int height, int destinationStride,
                          int sourceStride, int levels) {
    if (levels == 0) {
        for (int y = 0; y < height; y++) {
            memcpy(destination, source, width * 3);
            source = (unsigned char*)source + sourceStride * 3;
            destination = (unsigned char*)destination + destinationStride * 3;
        }
        return 1;
    }
    if (levels == 1) {
        UnknownFunction4cde20(destination, source, width, height, destinationStride, sourceStride);
        return 1;
    }
    int levelWidth = width << (levels - 1);
    int levelHeight = height << (levels - 1);
    void* buffer = DebugMalloc(levelHeight * levelWidth * 3, __FILE__, 1329);
    UnknownFunction4cde20(buffer, source, levelWidth, levelHeight, levelWidth, sourceStride);
    for (int i = 2; i < levels; i++) {
        levelWidth /= 2;
        levelHeight /= 2;
        UnknownFunction4cde20(buffer, buffer, levelWidth, levelHeight, levelWidth, levelWidth * 2);
    }
    UnknownFunction4cde20(destination, buffer, width, height, destinationStride, levelWidth);
    operator delete(buffer, __FILE__, 1350);
    return 1;
}

