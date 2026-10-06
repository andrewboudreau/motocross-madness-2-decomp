// Near-miss BackgroundImage.cpp candidates, kept out of src/reconstructed
// until they match. See docs/BACKGROUNDIMAGE.md.
//
// Constructor 0x00403d50 (77 bytes): the stores match, but retail emits the
// vptr store between the -1 stores and the 1 stores. With body assignments
// VC6 here stores it first; with member initializers it stores it last.
//
// Slot 13 0x00403dc0 (246 bytes): retail loads the camera's viewport
// height into ebx before reading the display mode; VC6 here loads the mode
// first (an inverted condition and a nested test give the same code).
//
// 0x004040f0 (261 bytes): retail places the shared `return -1` after the
// search loop and jumps there on a failed realloc; VC6 here places it at the
// end (early returns or `goto failed`).
//
// 0x004042e0 / 0x004043c0 (207 / 169 bytes): retail keeps the rectangle
// pointer in a stack slot (frame 0xc) and the region offset in ebp; VC6 here
// keeps both in registers.
//
// 0x00404480 (639 bytes): the dirty-region branch leads, as in retail, but
// the loop counters and the clipped rectangle land in other registers.

#include <string.h>

#include "../../src/reconstructed/BackgroundImage.h"

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/DisplayMode.h"
#include "../../src/reconstructed/TrackGame.h"

#define Target() ((PCRenderTarget*)field_0x18)

extern CameraRect g_UnknownGlobal5777b8;
extern CameraRect g_UnknownGlobal577790;
extern CameraRect g_UnknownGlobal577808;

// 0x00403d50
BackgroundImage::BackgroundImage(int flags) : GameObject(flags) {
    field_0x40 = 0;
    field_0x2c = 0;
    field_0x50 = 0;
    field_0x48 = 0;
    field_0x4c = 0;
    field_0x58 = 0;
    field_0x6c = 0;
    field_0x70 = 0;
    field_0x5c = 0;
    field_0x34 = -1;
    field_0x64 = -1;
    field_0x30 = 1;
    field_0x60 = 1;
    field_0x68 = 1;
}

// 0x00403dc0: while the camera covers the whole screen the background is
// restored from the off-screen copy region by region; otherwise (or while
// frames are left to restore) the whole copy is restored.
int BackgroundImage::UnknownVirtualSlot13() {
    if (!field_0x58) {
        UnknownBackgroundCamera* camera = (UnknownBackgroundCamera*)Target()->field_0x08;
        if (!camera || !camera->field_0x25_bit0 || !camera->field_0x1d0 ||
            camera->field_0x1ac != g_UnknownGlobal56e26c->field_0x0c->field_0x10[g_UnknownGlobal56e26c->field_0x0c->field_0x0c].height ||
            camera->field_0x1a8 != g_UnknownGlobal56e26c->field_0x0c->field_0x10[g_UnknownGlobal56e26c->field_0x0c->field_0x0c].width) {
            if (!field_0x40) {
                UnknownVirtualSlot27(field_0x3c);
                field_0x40 = 1;
            }
            UnknownFunction404cd0();
            if (Target()->field_0x08)
                UnknownFunction4043c0();
            UnknownFunction4042e0();
            return 1;
        }
    }
    UnknownVirtualSlot27(Target()->field_0x48);
    field_0x40 = 0;
    if (Target()->field_0x08)
        UnknownFunction4043c0();
    if (field_0x4c) {
        for (int i = 0; i < field_0x48; i++)
            UnknownFunction404d30(i);
    }
    if (--field_0x58 < 0)
        field_0x58 = 0;
    return 1;
}

// 0x004040f0: a free region, or four more.
int BackgroundImage::UnknownFunction4040f0(int owner) {
    int i;

    if (field_0x48 > field_0x4c) {
        for (i = 0; i < field_0x48; i++) {
            if (field_0x50[i].field_0x00 == 0) {
                field_0x50[i].field_0x00 = Target()->field_0x14 + 1;
                field_0x50[i].field_0x3c = owner;
                field_0x4c++;
                return i;
            }
        }
        goto failed;
    }
    field_0x50 = (UnknownBackgroundRegion*)DebugRealloc(field_0x50, (field_0x48 + 4) * sizeof(UnknownBackgroundRegion),
                                                        __FILE__, 270);
    if (!field_0x50)
        goto failed;
    for (i = field_0x48; i < field_0x48 + 4; i++) {
        field_0x50[i].field_0x00 = 0;
        field_0x50[i].field_0x34 = -1;
        field_0x50[i].field_0x3c = 0;
    }
    field_0x50[field_0x48].field_0x00 = Target()->field_0x14 + 1;
    field_0x50[field_0x48].field_0x38 = Target()->field_0x1c;
    field_0x50[field_0x48].field_0x3c = owner;
    field_0x48 += 4;
    field_0x4c++;
    return field_0x48 - 4;
failed:
    return -1;
}

// 0x004042e0: copies each region's current-frame rectangle back from the
// off-screen copy and ages the regions.
int BackgroundImage::UnknownFunction4042e0() {
    if (field_0x4c) {
        for (int i = 0; i < field_0x48; i++) {
            if (field_0x50[i].field_0x00) {
                CameraRect* rect = &field_0x50[i].field_0x04[Target()->field_0x18];
                if (rect->right - rect->left > 0 && rect->bottom - rect->top > 0 && !field_0x50[i].field_0x3c)
                    Target()->field_0x48->UnknownMethod7(rect->left, rect->top, field_0x3c, rect, 0x10);
                if (field_0x50[i].field_0x00 != Target()->field_0x14)
                    field_0x50[i].field_0x00--;
                if (field_0x50[i].field_0x00 == 0)
                    field_0x4c--;
            }
        }
    }
    return 1;
}

// 0x004043c0: clears the depth buffer under each region's previous-frame
// rectangle.
int BackgroundImage::UnknownFunction4043c0() {
    if (field_0x4c && Target()->field_0x08) {
        int frame = Target()->field_0x18 - 1;
        if (frame < 0)
            frame = Target()->field_0x14 - 1;
        for (int i = 0; i < field_0x48; i++) {
            if (field_0x50[i].field_0x00) {
                CameraRect* rect = &field_0x50[i].field_0x04[frame];
                if (rect->right - rect->left > 0 && rect->bottom - rect->top > 0 &&
                    Target()->field_0x50->UnknownMethod10(1, rect, 2, 0, Target()->field_0x2c, 0) != 0)
                    return 0;
            }
        }
    }
    return 1;
}

// 0x00404480: draws `image` at `rect`. With the off-screen copy current,
// only the parts under this frame's dirty regions are redrawn (into both
// surfaces); `frames` counts full redraws still owed.
int BackgroundImage::UnknownFunction404480(PCTextureMap* image, CameraRect* rect, void* sourceRect, int flags,
                                           int index, int dirty, int* frames, int skip) {
    if (dirty) {
        if (field_0x40) {
            if (*frames) {
                UnknownFunction404c80();
                image->UnknownFunction4c7b00(rect, field_0x3c, sourceRect, flags, skip);
                image->UnknownFunction4c7b00(rect, Target()->field_0x48, sourceRect, flags, skip);
                (*frames)--;
            } else {
                for (int i = 0; i < field_0x48; i++) {
                    if (field_0x50[i].field_0x38 != Target()->field_0x1c)
                        continue;
                    CameraRect* region = &field_0x50[i].field_0x04[Target()->field_0x18];
                    g_UnknownGlobal577790.left = region->left > rect->left ? region->left : rect->left;
                    g_UnknownGlobal577790.right = region->right < rect->right ? region->right : rect->right;
                    g_UnknownGlobal577790.top = region->top > rect->top ? region->top : rect->top;
                    g_UnknownGlobal577790.bottom = region->bottom < rect->bottom ? region->bottom : rect->bottom;
                    if (g_UnknownGlobal577790.left < g_UnknownGlobal577790.right &&
                        g_UnknownGlobal577790.top < g_UnknownGlobal577790.bottom) {
                        UnknownFunction404c80();
                        g_UnknownGlobal577808.left = g_UnknownGlobal577790.left - rect->left;
                        g_UnknownGlobal577808.right = g_UnknownGlobal577790.right - rect->left;
                        g_UnknownGlobal577808.top = g_UnknownGlobal577790.top - rect->top;
                        g_UnknownGlobal577808.bottom = g_UnknownGlobal577790.bottom - rect->top;
                        image->UnknownFunction4c7b00(&g_UnknownGlobal577790, Target()->field_0x48,
                                                     &g_UnknownGlobal577808, flags, skip);
                        image->UnknownFunction4c7b00(&g_UnknownGlobal577790, field_0x3c, &g_UnknownGlobal577808,
                                                     flags, skip);
                    }
                }
            }
        } else {
            UnknownFunction404c80();
            image->UnknownFunction4c7b00(rect, Target()->field_0x48, sourceRect, flags, skip);
            *frames = Target()->field_0x14;
        }
        if (index >= 0)
            UnknownFunction404cb0(index);
        return 1;
    }
    UnknownFunction404c80();
    image->UnknownFunction4c7b00(rect, Target()->field_0x48, sourceRect, flags, skip);
    if (index >= 0)
        UnknownFunction404240(index, rect);
    *frames = Target()->field_0x14;
    return 1;
}
