#include "RenderTarget.h"

#include "Camera.h"
#include "TrackGame.h"

// 0x004e8c50
RenderTarget::RenderTarget() {
    field_0x04 = 0;
    field_0x08 = 0;
    field_0x14 = 0;
    field_0x18 = 0;
    field_0x1c = 0;
    field_0x28 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x44 = 0;
}

// 0x004e8ca0
void RenderTarget::UnknownFunction4e8ca0(UnknownDisplay* display) {
    field_0x04 = display;
}

// 0x004e8cb0
RenderTarget::~RenderTarget() {}

// 0x004e8cc0: unless the global's +0x0c object says otherwise, the frame index
// advances modulo field_0x14; the frame count always does.
void RenderTarget::UnknownFunction4e8cc0() {
    if (!g_TrackGame->display->freezeFrameIndex) {
        if (++field_0x18 == field_0x14)
            field_0x18 = 0;
    }
    field_0x1c++;
}

// 0x004e8cf0: a camera whose cached size differs from this target's is
// refreshed through Camera 0x0042e550.
void RenderTarget::UnknownFunction4e8cf0(Camera* camera) {
    field_0x08 = camera;
    if (camera && (field_0x0c != camera->field_0x1c4 || field_0x10 != camera->field_0x1c8))
        camera->UnknownFunction42e550();
}

// 0x004806f0 and 0x0044d710: empty bodies shared with other classes.
void RenderTarget::UnknownVirtualSlot18(int) {}

void RenderTarget::UnknownVirtualSlot19() {}
