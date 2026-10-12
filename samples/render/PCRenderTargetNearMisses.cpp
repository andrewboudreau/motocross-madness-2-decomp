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
// current camera's viewport, else the whole target. Flags 0 clear the Z
// buffer, plus the target in wireframe or when +0x34 is set.
int PCRenderTarget::UnknownVirtualSlot12(const CameraRect* rect, int flags) {
    verticesDrawn = 0;
    pointsDrawn = 0;
    linesDrawn = 0;
    trianglesDrawn = 0;
    if (!targetDisplay->isPowerVR) {
        int clearFlags = flags;
        if (!clearFlags) {
            if (fillMode == D3DFILL_WIREFRAME)
                clearFlags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER;
            else
                clearFlags = (clearTarget != 0) | D3DCLEAR_ZBUFFER;
        }
        CameraRect area;
        if (rect) {
            area.left = rect->left;
            area.top = rect->top;
            area.right = rect->right;
            area.bottom = rect->bottom;
            if (device->Clear(1, &area, clearFlags, clearColor, zClearValue, 0))
                return 0;
        } else if (currentCamera) {
            area.left = 0;
            area.top = 0;
            area.right = currentCamera->field_0x1a0[2] + currentCamera->field_0x1a0[0];
            area.bottom = currentCamera->field_0x1a0[3] + currentCamera->field_0x1a0[1];
            if (device->Clear(1, &area, clearFlags, clearColor, zClearValue, 0))
                return 0;
        } else {
            area.left = 0;
            area.top = 0;
            area.right = targetWidth;
            area.bottom = targetHeight;
            if (device->Clear(1, &area, clearFlags, clearColor, zClearValue, 0))
                return 0;
        }
    }
    return 1;
}
