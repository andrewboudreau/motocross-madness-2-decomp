// Near-miss overlay.cpp candidate, kept out of src/reconstructed until it
// matches (Overlay.cpp holds the rest of the file).
//
// Overlay::RestoreRect (0x004b6710, 361 bytes): the same
// instructions in the same order, but VC6 here keeps `rect` in esi, `this`
// in edi and the byte count in ebp, with the locked destination on the
// stack; retail keeps `this` in esi (spilled), the byte count in edi, the
// source and destination in ebx/ebp and reloads `rect` from the stack.
// Pointer locals for the row loop, declaration order, inline width/height
// expressions, a nested success block, char* pointers and a while loop do
// not change the allocation.
#include <string.h>

#include "../../src/reconstructed/Overlay.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x004b6710. A rectangle as wide as the texture is copied in one block.
int Overlay::RestoreRect(const UnknownOverlayRect* rect) {
    int width = rect->right - rect->left;
    int height = rect->bottom - rect->top;
    int whole;
    int bytes;
    if (width == overlayTexture->field_0x14) {
        bytes = UnknownFunction511970(overlayTexture->field_0x20) * overlayTexture->field_0x14 * height;
        whole = 1;
    } else {
        bytes = UnknownFunction511970(overlayTexture->field_0x20) * width;
        whole = 0;
    }

    long sourcePitch, pitch;
    unsigned char* source = (unsigned char*)backingTexture->UnknownVirtualSlot13(0, &sourcePitch, 0x811);
    unsigned char* bits = (unsigned char*)overlayTexture->UnknownVirtualSlot13(0, &pitch, 0x801);
    if (!source || !bits)
        return 0;

    if (whole) {
        memcpy(bits + rect->top * pitch, source + rect->top * sourcePitch, bytes);
    } else {
        int offset = rect->top * sourcePitch;
        offset += UnknownFunction511970(overlayTexture->field_0x20) * rect->left;
        source += offset;
        bits += offset;
        for (int y = 0; y < height; y++) {
            memcpy(bits, source, bytes);
            source += sourcePitch;
            bits += pitch;
        }
    }
    backingTexture->UnknownVirtualSlot14(0);
    overlayTexture->UnknownVirtualSlot14(0);
    return 1;
}
