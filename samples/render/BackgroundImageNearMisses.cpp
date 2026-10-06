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
// 0x004043c0 (177 bytes): the instructions match; retail lays the
// `return 0` block out between the loop and the final `return 1` (`jge`
// out, `jmp` back), VC6 here puts it last. A local rectangle pointer, the
// accessor, early returns, `continue`, `goto` and a while loop all compile
// to the same layout.
//
// 0x004049d0 (678 bytes): retail stores the three leading zeros (and
// `field_0x60 = 0`) as immediates; VC6 here caches 0 in edi, which shifts
// the register choice of the region loop. Statement orders and a helper
// for the DC tail do not change that.

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
// 0x005777a0 / 0x005777a8: 0x004049d0's region index and clipped rectangle.
int g_UnknownGlobal5777a0;
CameraRect g_UnknownGlobal5777a8;

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

// 0x004043c0: clears the depth buffer under each region's previous-frame
// rectangle.
int BackgroundImage::UnknownFunction4043c0() {
    if (field_0x4c && Target()->field_0x08) {
        int frame = Target()->field_0x18 - 1;
        if (frame < 0)
            frame = Target()->field_0x14 - 1;
        for (int i = 0; i < field_0x48; i++) {
            if (field_0x50[i].field_0x00 &&
                field_0x50[i].field_0x04[frame].right - field_0x50[i].field_0x04[frame].left > 0 &&
                field_0x50[i].field_0x04[frame].bottom - field_0x50[i].field_0x04[frame].top > 0 &&
                Target()->field_0x50->UnknownMethod10(1, &field_0x50[i].field_0x04[frame], 2, 0,
                                                      Target()->field_0x2c, 0) != 0)
                return 0;
        }
    }
    return 1;
}

// 0x004049d0: a DC for drawing `rect`: the back buffer, or with the copy
// current the surfaces in turn (`a` set when the copy is returned), clipped
// to the next dirty region in `clip`.
int BackgroundImage::UnknownFunction4049d0(void** dc, CameraRect* rect, int index, int dirty, int* frames, int* a,
                                           CameraRect* clip) {
    UnknownSurfaceInterface* surface;
    *dc = 0;
    surface = 0;
    *a = 0;
    if (dirty) {
        if (field_0x40) {
            if (*frames) {
                if (field_0x60) {
                    *a = 1;
                    surface = field_0x3c;
                    field_0x60 = 0;
                } else {
                    surface = Target()->field_0x48;
                    field_0x60 = 1;
                    (*frames)--;
                }
            } else {
                field_0x60 = 1;
                for (g_UnknownGlobal5777a0 = 0; g_UnknownGlobal5777a0 < field_0x48; g_UnknownGlobal5777a0++) {
                    if (field_0x50[g_UnknownGlobal5777a0].field_0x38 != Target()->field_0x1c)
                        continue;
                    g_UnknownGlobal5777a8.left = CurrentRegionRect(g_UnknownGlobal5777a0).left > rect->left ? CurrentRegionRect(g_UnknownGlobal5777a0).left : rect->left;
                    g_UnknownGlobal5777a8.right = CurrentRegionRect(g_UnknownGlobal5777a0).right < rect->right ? CurrentRegionRect(g_UnknownGlobal5777a0).right : rect->right;
                    g_UnknownGlobal5777a8.top = CurrentRegionRect(g_UnknownGlobal5777a0).top > rect->top ? CurrentRegionRect(g_UnknownGlobal5777a0).top : rect->top;
                    g_UnknownGlobal5777a8.bottom = CurrentRegionRect(g_UnknownGlobal5777a0).bottom < rect->bottom ? CurrentRegionRect(g_UnknownGlobal5777a0).bottom : rect->bottom;
                    if (g_UnknownGlobal5777a8.left < g_UnknownGlobal5777a8.right &&
                        g_UnknownGlobal5777a8.top < g_UnknownGlobal5777a8.bottom) {
                        if (field_0x68 && g_UnknownGlobal5777a0 > field_0x64) {
                            surface = Target()->field_0x48;
                            field_0x68 = 0;
                        } else if (!field_0x68 && g_UnknownGlobal5777a0 == field_0x64) {
                            surface = field_0x3c;
                            field_0x68 = 1;
                        } else {
                            continue;
                        }
                        *a = 1;
                        *clip = g_UnknownGlobal5777a8;
                        field_0x64 = g_UnknownGlobal5777a0;
                        break;
                    }
                }
                if (g_UnknownGlobal5777a0 == field_0x48) {
                    field_0x68 = 1;
                    field_0x64 = -1;
                }
            }
        } else {
            surface = Target()->field_0x48;
            *frames = Target()->field_0x14;
        }
        if (index >= 0)
            UnknownFunction404cb0(index);
    } else {
        if (index >= 0)
            UnknownFunction404240(index, rect);
        surface = Target()->field_0x48;
        *frames = Target()->field_0x14;
    }
    if (!surface)
        return 0;
    if (field_0x6c) {
        if (surface != field_0x6c) {
            UnknownFunction404c80();
            field_0x6c = surface;
            surface->UnknownMethod17(&field_0x70);
        }
    } else {
        field_0x6c = surface;
        surface->UnknownMethod17(&field_0x70);
    }
    *dc = field_0x70;
    return 1;
}
