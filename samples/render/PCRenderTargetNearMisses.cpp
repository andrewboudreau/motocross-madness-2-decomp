// Near misses for src/reconstructed/PCRenderTarget.cpp (TU PCRenderTarget.cpp).
// Each compiles from readable source but differs from retail as noted.
//
// PCRenderTarget::UnknownVirtualSlot12 (0x004c5510, 313 bytes): the Clear.
//   Flow, flag selection and the three rectangles match. Retail keeps three
//   separate Clear calls whose result tests merge, reuses esi (the zero
//   register) for the Z value after pushing the stencil 0 and saves ebx only
//   in the camera branch; this source has the calls cross-jumped (or, with a
//   result variable, ebx saved in the prologue). Tried: struct copy versus
//   member copies, a result variable, a separate flags local (44/275).
#include "../../src/reconstructed/PCRenderTarget.cpp"

// 0x004c5510: resets the primitive counters and clears `rect`, else the
// current camera's viewport, else the whole target. Flags 0 pick the target
// (+0x250 == 2: 3) or the stencil bit from +0x34 plus the Z bit.
int PCRenderTarget::UnknownVirtualSlot12(const CameraRect* rect, int flags) {
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x44 = 0;
    if (!field_0x04->field_0xb74_bit2) {
        int clearFlags = flags;
        if (!clearFlags) {
            if (field_0x250 == 2)
                clearFlags = 3;
            else
                clearFlags = (field_0x34 != 0) | 2;
        }
        CameraRect area;
        if (rect) {
            area.left = rect->left;
            area.top = rect->top;
            area.right = rect->right;
            area.bottom = rect->bottom;
            if (field_0x50->UnknownMethod10(1, &area, clearFlags, field_0x30, field_0x2c, 0))
                return 0;
        } else if (field_0x08) {
            area.left = 0;
            area.top = 0;
            area.right = field_0x08->field_0x1a0[2] + field_0x08->field_0x1a0[0];
            area.bottom = field_0x08->field_0x1a0[3] + field_0x08->field_0x1a0[1];
            if (field_0x50->UnknownMethod10(1, &area, clearFlags, field_0x30, field_0x2c, 0))
                return 0;
        } else {
            area.left = 0;
            area.top = 0;
            area.right = field_0x0c;
            area.bottom = field_0x10;
            if (field_0x50->UnknownMethod10(1, &area, clearFlags, field_0x30, field_0x2c, 0))
                return 0;
        }
    }
    return 1;
}
