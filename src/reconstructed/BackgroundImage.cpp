#include <string.h>

#include "BackgroundImage.h"

#include "DebugAlloc.h"
#include "TrackGame.h"

// The render target the image belongs to (GameObject+0x18).
#define Target() ((PCRenderTarget*)field_0x18)

// 0x005777b8: the image's own rectangle (slot 15).
CameraRect g_UnknownGlobal5777b8;
// 0x00577790 / 0x00577808: a dirty region clipped to the image and the
// matching source rectangle (0x00404480).
CameraRect g_UnknownGlobal577790;
CameraRect g_UnknownGlobal577808;

// 0x00404010
BackgroundImage::~BackgroundImage() {
    if (field_0x70 && field_0x6c)
        field_0x6c->UnknownMethod26(field_0x70);
    if (field_0x3c) {
        field_0x3c->UnknownMethod2();
        field_0x3c = 0;
    }
    if (field_0x2c) {
        field_0x2c->Release();
        field_0x2c = 0;
    }
    if (field_0x50)
        operator delete(field_0x50, __FILE__, 230);
}

// 0x00403ec0
int BackgroundImage::UnknownVirtualSlot15() {
    if (field_0x2c && field_0x30) {
        g_UnknownGlobal5777b8.left = 0;
        g_UnknownGlobal5777b8.top = 0;
        g_UnknownGlobal5777b8.right = field_0x2c->field_0x14;
        g_UnknownGlobal5777b8.bottom = field_0x2c->field_0x18;
        UnknownFunction404480(field_0x2c, &g_UnknownGlobal5777b8, 0, 0x1000000, field_0x34, field_0x5c, &field_0x38,
                              0);
        field_0x5c = 1;
    }
    return 1;
}

// 0x00403f30: creates the off-screen copy with the back buffer's size and
// format and four regions; releases the object on failure.
GameObject* BackgroundImage::UnknownVirtualSlot8(void* value) {
    UnknownSurfaceDesc desc;
    int i;

    GameObject::UnknownVirtualSlot8(value);
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = 0x1006;
    if (Target()->field_0x48->UnknownMethod22(&desc) != 0)
        goto failed;
    desc.flags = 0x1007;
    desc.caps[0] = 0x2800;
    if (Target()->field_0x04->field_0x190->UnknownMethod6(&desc, &field_0x3c, 0) != 0)
        goto failed;
    field_0x44 = 0;
    field_0x48 = 4;
    field_0x4c = 0;
    field_0x50 = (UnknownBackgroundRegion*)DebugMalloc(0x100, __FILE__, 212);
    for (i = 0; i < field_0x48; i++)
        field_0x50[i].field_0x00 = 0;
    return this;
failed:
    Release();
    return 0;
}

// 0x004040b0
void BackgroundImage::UnknownFunction4040b0(PCTextureMap* image) {
    if (image) {
        if (!field_0x2c)
            field_0x34 = UnknownFunction4040f0(0);
        field_0x2c = image;
    } else {
        UnknownFunction404200(field_0x34);
        field_0x2c = 0;
    }
}

// 0x00404200
void BackgroundImage::UnknownFunction404200(int index) {
    if (field_0x50 && index < field_0x48) {
        if (field_0x50[index].field_0x00)
            field_0x50[index].field_0x00--;
        if (field_0x50[index].field_0x00 == 0)
            field_0x4c--;
    }
}

// 0x00404240: records `rect` (clipped to the target) for the current frame.
void BackgroundImage::UnknownFunction404240(int index, CameraRect* rect) {
    if (rect->bottom + rect->top && rect->right + rect->left) {
        if (field_0x50[index].field_0x00) {
            field_0x50[index].field_0x04[Target()->field_0x18] = *rect;
            CameraRect* region = &field_0x50[index].field_0x04[Target()->field_0x18];
            region->right = region->right < Target()->field_0x0c ? region->right : Target()->field_0x0c;
            region->bottom = region->bottom < Target()->field_0x10 ? region->bottom : Target()->field_0x10;
        }
        field_0x50[index].field_0x38 = Target()->field_0x1c;
    }
}

// 0x00404c80
int BackgroundImage::UnknownFunction404c80() {
    if (field_0x6c)
        field_0x6c->UnknownMethod26(field_0x70);
    field_0x6c = 0;
    field_0x70 = 0;
    return 1;
}

// 0x00404cb0
void BackgroundImage::UnknownFunction404cb0(int index) {
    field_0x50[index].field_0x34 = Target()->field_0x18;
}

// 0x00404cd0
void BackgroundImage::UnknownFunction404cd0() {
    for (int i = 0; i < field_0x48; i++) {
        if (field_0x50[i].field_0x00 > 0 && field_0x50[i].field_0x34 != -1) {
            int frame = field_0x50[i].field_0x34;
            field_0x50[i].field_0x34 = -1;
            field_0x50[i].field_0x04[frame].top = 0;
            field_0x50[i].field_0x04[frame].bottom = 0;
            field_0x50[i].field_0x04[frame].left = 0;
            field_0x50[i].field_0x04[frame].right = 0;
        }
    }
}

// 0x00404d30
void BackgroundImage::UnknownFunction404d30(int index) {
    field_0x50[index].field_0x34 = -1;
    field_0x50[index].field_0x04[Target()->field_0x18].top = 0;
    field_0x50[index].field_0x04[Target()->field_0x18].bottom = 0;
    field_0x50[index].field_0x04[Target()->field_0x18].left = 0;
    field_0x50[index].field_0x04[Target()->field_0x18].right = 0;
}

// 0x00404da0: owes a full restore for every buffered frame (and asks the
// camera to redraw).
void BackgroundImage::UnknownFunction404da0() {
    field_0x58 = Target()->field_0x14 + 1;
    UnknownBackgroundCamera* camera = (UnknownBackgroundCamera*)Target()->field_0x08;
    if (camera) {
        camera->field_0x1d0 = Target()->field_0x14 + 1;
        if ((unsigned)((UnknownBackgroundCamera*)Target()->field_0x08)->field_0x1ac < (unsigned)Target()->field_0x10)
            ((UnknownBackgroundCamera*)Target()->field_0x08)->field_0x1cc = Target()->field_0x14 + 1;
    }
}

// 0x00404de0
int BackgroundImage::UnknownVirtualSlot18() {
    UnknownFunction404da0();
    return 1;
}
