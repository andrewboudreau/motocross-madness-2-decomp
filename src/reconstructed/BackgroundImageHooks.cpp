// BackgroundImage's primary vtable, 0x005506d8, places the complete eight-byte
// body at 0x004627f0 in slot 27. RenderPre3D passes the destination surface.
// Keep this definition separate so the existing caller retains its call.
// This reconstructed grouping does not establish original TU ownership.
#include "BackgroundImage.h"

int BackgroundImage::UnknownVirtualSlot27(UnknownSurfaceInterface* surface) {
    return 1;
}
