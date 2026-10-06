#include <string.h>

#include "Overlay.h"

#include "Camera.h"
#include "DebugAlloc.h"
#include "PCTextureMap.h"
#include "Tgafile.h"
#include "TrackGame.h"

// overlay.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\overlay.cpp" at
// 0x0056f758): 0x004b5e20-0x004b6b20, followed by Palette8.cpp. Names are
// provisional.

// The 256x256 texture every Overlay shares at +0x34 (only overlay.cpp
// references 0x00689110).
static TextureMap* g_UnknownOverlayTexture689110;

// 0x004b5e20
Overlay::Overlay(int flags, int value) : GameObject(flags) {
    field_0x2c = 0;
    field_0x30 = 0;
    field_0x40 = 0;
    field_0xe4 = 1;
    memset(field_0x44, 0, sizeof(field_0x44));
    memset(&field_0xf8, 0, sizeof(field_0xf8));
    field_0x108 = 0;
    field_0x10c = 0xff;
    field_0x110 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x118 = value;
}

// 0x004b5ec0 (scalar deleting wrapper 0x004b5ea0): the last Overlay to
// release the shared texture forgets it.
Overlay::~Overlay() {
    if (field_0x34) {
        int count = field_0x34->Release();
        if (count == 0)
            g_UnknownOverlayTexture689110 = 0;
    }
    if (field_0x38) {
        delete field_0x38;
        field_0x38 = 0;
    }
    if (field_0x3c) {
        delete field_0x3c;
        field_0x3c = 0;
    }
}

// 0x004b5f50
Overlay* Overlay::UnknownFunction4b5f50(RenderTarget* target, TextureMap* texture,
                                        const UnknownOverlayRect* rect, int a4,
                                        const UnknownOverlayRect* source, float depth,
                                        char a7, const char* a8, const char** a9, int a10,
                                        int a11, int a12) {
    GameObject::UnknownVirtualSlot8(target);
    field_0x114 = depth;
    field_0x2c = texture;
    field_0xe4 = a4;
    // Through the Target() accessor VC6 allocates the registers differently.
    field_0x40 = ((RenderTarget*)field_0x18)->field_0x28;
    field_0xe8 = *rect;
    if (source) {
        field_0xf8 = *source;
        field_0x108 = 1;
    }
    UnknownFunction4b6380();

    if (a7)
        field_0x38 = UnknownOverlayText::UnknownFunction50ae80(
            a8, a9, a10, a11, g_UnknownGlobal56e26c->field_0x3c, field_0x44[0].sx, field_0x44[0].sy,
            field_0x44[2].sx - field_0x44[0].sx, field_0x44[2].sy - field_0x44[0].sy, a12);
    else
        field_0x38 = 0;

    field_0x3c = UnknownCreateOverlayIconService();

    if (!g_UnknownOverlayTexture689110) {
        g_UnknownOverlayTexture689110 =
            new(__FILE__, 107) PCTextureMap(g_UnknownGlobal56e26c->field_0x3c, 1);
        g_UnknownOverlayTexture689110->UnknownVirtualSlot4(0, 0x100, 0x100, 0x100, 0x100,
                                                           Target()->field_0x28, Target()->field_0x28,
                                                           0, 8, 0, 1, 0, 5, 6, 0, 0x80, 0xff00ff);
        void* bits = g_UnknownOverlayTexture689110->UnknownVirtualSlot13(0, 0, 0);
        memset(bits, 0, UnknownFunction511970(g_UnknownOverlayTexture689110->field_0x20) << 16);
        g_UnknownOverlayTexture689110->UnknownVirtualSlot14(0);
        field_0x34 = g_UnknownOverlayTexture689110;
    } else {
        field_0x34 = g_UnknownOverlayTexture689110;
        g_UnknownOverlayTexture689110->AddRef();
    }
    return this;
}

// 0x004b61a0: draws the quad with the overlay texture, saving and restoring
// texture stage 0 state 0xc.
int Overlay::UnknownVirtualSlot14() {
    int saved;

    field_0x2c->UnknownVirtualSlot19();
    Target()->UnknownVirtualSlot6(0, 0xc, &saved);
    Target()->UnknownVirtualSlot7(0, 0xc, 3);
    Target()->UnknownVirtualSlot8(0x1c, 0, 0);
    if (field_0x118)
        UnknownFunction4b6380();

    int format = field_0x2c->field_0x20;
    if (format != 0x115c && format != 0x22b8) {
        Target()->UnknownVirtualSlot7(0, 1, 4);
        Target()->UnknownVirtualSlot7(0, 2, 2);
        Target()->UnknownVirtualSlot7(0, 3, 0);
        if (field_0x110) {
            Target()->UnknownVirtualSlot7(0, 4, 2);
            Target()->UnknownVirtualSlot7(0, 5, 2);
        } else {
            Target()->UnknownVirtualSlot7(0, 4, 1);
        }
    } else {
        Target()->UnknownVirtualSlot7(0, 1, 4);
        Target()->UnknownVirtualSlot7(0, 2, 2);
        Target()->UnknownVirtualSlot7(0, 3, 0);
        if (field_0x110) {
            Target()->UnknownVirtualSlot7(0, 4, 4);
            Target()->UnknownVirtualSlot7(0, 5, 2);
            Target()->UnknownVirtualSlot7(0, 6, 0);
        } else {
            Target()->UnknownVirtualSlot7(0, 4, 2);
            Target()->UnknownVirtualSlot7(0, 5, 2);
        }
        Target()->UnknownVirtualSlot8(0x1b, 1, 0);
    }

    Target()->UnknownVirtualSlot18(0);
    Target()->UnknownVirtualSlot8(0xe, 0, 0);
    Target()->UnknownVirtualSlot8(7, 0, 0);
    Target()->UnknownVirtualSlot8(4, 0, 0);
    if (!Target()->UnknownVirtualSlot16(6, 0x1c4, (int)field_0x44, 4, 0))
        return 0;
    Target()->UnknownVirtualSlot8(0xe, 1, 0);
    Target()->UnknownVirtualSlot8(7, 1, 0);
    Target()->UnknownVirtualSlot8(0xf, 0, 0);
    Target()->UnknownVirtualSlot7(0, 0xc, saved);
    return 1;
}

// 0x004b6380. The screen rectangle is in 640x480 units when +0xe4 is set;
// the quad is clipped (with its texture coordinates) to the right and
// bottom of the camera viewport and to the render target.
void Overlay::UnknownFunction4b6380() {
    float u0, v0, u1, v1;
    if (field_0x108) {
        float width = (float)field_0x2c->field_0x14;
        u0 = (float)field_0xf8.left / width;
        float height = (float)field_0x2c->field_0x18;
        v0 = (float)field_0xf8.top / height;
        u1 = (float)field_0xf8.right / width;
        v1 = (float)field_0xf8.bottom / height;
    } else {
        u0 = 0.0f;
        v0 = 0.0f;
        u1 = 1.0f;
        v1 = 1.0f;
    }

    int left, top, maxX, maxY;
    Camera* camera = Target()->field_0x08;
    if (camera) {
        CameraRect viewport;
        camera->UnknownFunction42f210(&viewport);
        maxX = viewport.right - 1;
        maxY = viewport.bottom - 1;
        left = viewport.left;
        top = viewport.top;
    } else {
        maxX = Target()->field_0x0c - 1;
        maxY = Target()->field_0x10 - 1;
        left = 0;
        top = 0;
    }

    float originX, originY, x0, y0, x1, y1;
    if (field_0xe4) {
        float scaleX = (float)maxX * (1.0f / 640.0f);
        originX = (float)left;
        x0 = (float)field_0xe8.left * scaleX + originX;
        float scaleY = (float)maxY * (1.0f / 480.0f);
        originY = (float)top;
        y0 = (float)field_0xe8.top * scaleY + originY;
        x1 = (float)field_0xe8.right * scaleX + originX;
        y1 = (float)field_0xe8.bottom * scaleY + originY;
    } else {
        originX = (float)left;
        x0 = (float)field_0xe8.left + originX;
        x1 = (float)field_0xe8.right + originX;
        originY = (float)top;
        y0 = (float)field_0xe8.top + originY;
        y1 = (float)field_0xe8.bottom + originY;
    }

    float limitX = (float)(left + maxX);
    if (x1 >= limitX) {
        u1 = u0 + (u1 - u0) * (1.0f - (x1 - maxX - originX) / (x1 - x0));
        x1 = limitX;
    }
    float limitY = (float)(top + maxY);
    if (y1 >= limitY) {
        v1 = v0 + (v1 - v0) * (1.0f - (y1 - maxY - originY) / (y1 - y0));
        y1 = limitY;
    }
    if (x0 <= 0.0f)
        x0 = 0.0f;
    if (y0 <= 0.0f)
        y0 = 0.0f;
    float targetWidth = (float)Target()->field_0x0c;
    if (x1 >= targetWidth)
        x1 = targetWidth;
    float targetHeight = (float)Target()->field_0x10;
    if (y1 >= targetHeight)
        y1 = targetHeight;

    field_0x44[0].sx = x0;
    field_0x44[0].sy = y0;
    field_0x44[0].sz = field_0x114;
    field_0x44[0].rhw = 1.0f;
    field_0x44[0].specular = 0;
    field_0x44[0].tu = u0;
    field_0x44[0].tv = v0;
    field_0x44[1].sx = x1;
    field_0x44[1].sy = y0;
    field_0x44[1].sz = field_0x114;
    field_0x44[1].rhw = 1.0f;
    field_0x44[1].specular = 0;
    field_0x44[1].tu = u1;
    field_0x44[1].tv = v0;
    field_0x44[2].sx = x1;
    field_0x44[2].sy = y1;
    field_0x44[2].sz = field_0x114;
    field_0x44[2].rhw = 1.0f;
    field_0x44[2].specular = 0;
    field_0x44[2].tu = u1;
    field_0x44[2].tv = v1;
    field_0x44[3].sx = x0;
    field_0x44[3].sy = y1;
    field_0x44[3].sz = field_0x114;
    field_0x44[3].rhw = 1.0f;
    field_0x44[3].specular = 0;
    field_0x44[3].tu = u0;
    field_0x44[3].tv = v1;

    unsigned int color = field_0x110 ? (field_0x10c << 24) | 0xffffff : 0xffffffff;
    field_0x44[0].color = color;
    field_0x44[1].color = color;
    field_0x44[2].color = color;
    field_0x44[3].color = color;
}

// 0x004b6710 is a near miss: samples/render/OverlayNearMisses.cpp.

// 0x004b6880
int Overlay::UnknownFunction4b6880(int row, int count, unsigned short color) {
    long sourcePitch, pitch;
    void* source = field_0x34->UnknownVirtualSlot13(0, &sourcePitch, 0x801);
    void* bits = field_0x2c->UnknownVirtualSlot13(0, &pitch, 0x801);
    if (!source || !bits)
        return 0;

    if (field_0x2c->field_0x20 == 0x115c) {
        unsigned short* from = (unsigned short*)source + sourcePitch * row / 2;
        unsigned short* to = (unsigned short*)bits + pitch * row / 2;
        for (int y = 0; y < count; y++) {
            unsigned short* s = from;
            unsigned short* d = to;
            for (int x = 0; x < field_0x2c->field_0x14; x++) {
                if ((*s & 0x7fff) == 0x7fff) {
                    *d = (unsigned short)color;
                    *s = 0;
                }
                s++;
                d++;
            }
            to += pitch / 2;
            from += sourcePitch / 2;
        }
    } else if (field_0x2c->field_0x20 == 0x613) {
        unsigned short* from = (unsigned short*)source + sourcePitch * row / 2;
        unsigned short* to = (unsigned short*)bits + pitch * row / 2;
        unsigned short converted =
            (unsigned short)((((color & 0xff00) | 0xf000) << 3) | ((color & 0xf0) << 2) | ((color & 0xf) << 1));
        for (int y = 0; y < count; y++) {
            unsigned short* s = from;
            unsigned short* d = to;
            for (int x = 0; x < field_0x2c->field_0x14; x++) {
                if ((*s & 0x7fff) == 0x7fff) {
                    *d = converted;
                    *s = 0;
                }
                s++;
                d++;
            }
            to += pitch / 2;
            from += sourcePitch / 2;
        }
    }
    if (field_0x2c->UnknownVirtualSlot14(0) && field_0x34->UnknownVirtualSlot14(0))
        return 1;
    return 0;
}

// 0x004b6a70 (declared in OverlayIconService.h)
OverlayIconService::OverlayIconService(int capacity) {
    field_0x08.Init(capacity, capacity);
}
