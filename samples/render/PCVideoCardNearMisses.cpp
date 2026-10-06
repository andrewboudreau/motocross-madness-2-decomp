// Near misses for src/reconstructed/PCVideoCard.cpp (TU PCVideoCard.cpp).
// Each compiles from readable source but differs from retail as noted.
//
// UnknownDisplay::UnknownFunction4ca520 (0x004ca520, 116 bytes): logic and
//   calls match; retail loads +0x74 and all four mode fields into registers
//   before testing the argument and computes the free memory as
//   neg/lea (memory - pixels * 2); local orders and pixel temporaries tried.
// UnknownDisplay::UnknownFunction4ca5a0 (0x004ca5a0, 484 bytes): retail keeps
//   `target` in ebp, caches 480/16 in registers for the mode search and has
//   one more 4-byte local; the flow (EH-guarded new PCRenderTarget, 128x128
//   probe surface, caps bit 0x20000000) is as written.
// float UnknownFunction4cb330 (0x004cb330, 628 bytes): register allocation
//   (retail: ebp = first, esi/edi = pixel pointers) and the x87 sequence for
//   the three channel sums (retail keeps the green difference on the stack
//   and stores the blue one) differ; the expansion and loop shapes match.
// UnknownDisplay::UnknownFunction4cab00 (0x004cab00, 1129 bytes): the
//   PartialTexBlt driver; register allocation differs throughout (retail
//   keeps `this` in ebp and spills it), structure follows retail.
// UnknownDisplay::UnknownFunction4cb5b0 (0x004cb5b0, 186 bytes): retail
//   returns int (0 when a flip, GetGDISurface or the flip budget fails, else
//   1). Display.h keeps `void` because GUIManager.bindings.json binds
//   ?UnknownFunction4cb5b0@UnknownDisplay@@QAEXH@Z. With `int` the body
//   matches except the placement of the shared `return 0` block after the
//   loop (145/184 bytes).
// VideoCard::VideoCard (0x0052d180, 115 bytes; VideoCard.cpp): every store
//   matches; retail clears the zero register before the bit-2 mask, a
//   scheduling difference no statement order reproduced.
#include "../../src/reconstructed/PCVideoCard.cpp"

#include <string.h>

// 0x004ca520: the surfaces for the current mode; full screen needs the
// whole chain plus two 16-bit frames' worth of video memory.
int UnknownDisplay::UnknownFunction4ca520(int fullScreen) {
    int width = field_0x10[field_0x0c].width;
    int height = field_0x10[field_0x0c].height;
    int bitDepth = field_0x10[field_0x0c].bitDepth;
    int buffers = field_0x10[field_0x0c].field_0x10;
    if (fullScreen) {
        if (width * height * bitDepth / 8 * buffers > field_0x74 - width * height * 2)
            return 0;
        if (UnknownFunction4c9f30(buffers - 1))
            return 1;
    } else if (UnknownFunction4ca130(buffers - 1)) {
        return 1;
    }
    return 0;
}

// 0x004ca5a0: whether a 128 x 128 texture of the target's format can be
// created (with a temporary 640 x 480 x 16 target when `target` is 0).
int UnknownDisplay::UnknownFunction4ca5a0(int* value, RenderTarget* target) {
    RenderTarget* render = 0;
    UnknownSurfaceInterface* surface;
    UnknownSurfaceCaps caps;
    UnknownSurfaceDesc desc;
    int result;

    *value = 0;
    if (target) {
        render = target;
    } else {
        for (int i = 0; i < field_0x08; i++) {
            if (field_0x10[i].width == 640 && field_0x10[i].height == 480 && field_0x10[i].bitDepth == 16) {
                if (UnknownFunction4ca900(i, 1)) {
                    render = (new (__FILE__, 1185) PCRenderTarget)
                                 ->UnknownFunction4c4f80(this, &IID_IDirect3DHALDevice, field_0x1a0, 1,
                                                         field_0x78);
                    if (!render)
                        goto failed;
                }
                break;
            }
        }
    }
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    UnknownFunction5119c0(render->field_0x28, &desc.pixelFormat);
    desc.width = 128;
    desc.height = 128;
    desc.flags = 0x1007;                          // caps, height, width, pixel format
    desc.caps[0] = 0x20005000;
    if (field_0x190->UnknownMethod6(&desc, &surface, 0) == 0) {
        if (surface->UnknownMethod14(&caps)) {
            *value = 0;
            result = 0;
        } else if (caps.caps & 0x20000000) {
            result = 1;
        } else {
            *value = 0;
            result = 0;
        }
        surface->UnknownMethod2();
        if (!target && render)
            delete render;
        return result;
    }
    if (!target && render)
        delete render;
failed:
    *value = 0;
    return 0;
}


// Expands 16-bit 5:5:5 and 5:6:5 pixels to 0xRRGGBB (low bits zero).
#define UNKNOWN_EXPAND_555(c) (((((c) & 0x7c00) << 3 | ((c) & 0x3e0)) << 3 | ((c) & 0x1f)) << 3)
#define UNKNOWN_EXPAND_565(c) (((((c) & 0xf800) << 3 | ((c) & 0x7e0)) << 2 | ((c) & 0x1f)) << 3)

// 0x004cb330: the mean absolute channel difference (1/256 units) of two
// 16-bit textures of `first`'s size, or -1 when one cannot be locked.
float UnknownFunction4cb330(TextureMap* first, TextureMap* second) {
    long firstPitch;
    long secondPitch;
    unsigned short* a = (unsigned short*)first->UnknownVirtualSlot13(0, &firstPitch, 0x811);
    if (!a)
        return -1.0f;
    unsigned short* b = (unsigned short*)second->UnknownVirtualSlot13(0, &secondPitch, 0x811);
    if (!b) {
        second->UnknownVirtualSlot14(0);
        return -1.0f;
    }
    int width = first->field_0x14;
    int is555 = first->field_0x20 == 0x22b;
    float total = 0.0f;
    int firstSkip = firstPitch / sizeof(unsigned short) - width;
    int secondSkip = secondPitch / sizeof(unsigned short) - width;
    int count = 0;
    for (int rows = first->field_0x18; rows--;) {
        for (int columns = width; columns--;) {
            int pixelA;
            int pixelB;
            count++;
            if (is555) {
                pixelA = UNKNOWN_EXPAND_555(*a);
                pixelB = UNKNOWN_EXPAND_555(*b);
            } else {
                pixelA = UNKNOWN_EXPAND_565(*a);
                pixelB = UNKNOWN_EXPAND_565(*b);
            }
            a++;
            b++;
            float difference = (float)fabs(((pixelB >> 16) & 0xff) * 0.00390625f - ((pixelA >> 16) & 0xff) * 0.00390625f);
            total += difference;
            difference = (float)fabs(((pixelB >> 8) & 0xff) * 0.00390625f - ((pixelA >> 8) & 0xff) * 0.00390625f);
            total += difference;
            difference = (float)fabs((pixelB & 0xff) * 0.00390625f - (pixelA & 0xff) * 0.00390625f);
            total += difference;
        }
        a += firstSkip;
        b += secondSkip;
    }
    first->UnknownVirtualSlot14(0);
    second->UnknownVirtualSlot14(0);
    return total / (count * 3);
}


// 0x004cab00: the "PartialTexBlt" test (+0x5bc): uploads 64 x 64 tiles of
// partblt.tga through a texture and compares the drawn result. 1: works;
// 0: wrong; -1: the full upload failed; -2: no image or texture; -3: no
// 640 x 480 x 16 target; -4: no texture surface.
void UnknownDisplay::UnknownFunction4cab00(RenderTarget* target) {
    int result = 1;
    UnknownRect area;
    UnknownRect tile;
    PCTextureMap* image;
    PCTextureMap* expected;
    PCTextureMap* rendered;

    area.left = 0;
    area.top = 0;
    area.right = 256;
    area.bottom = 256;
    rendered = 0;
    expected = 0;
    UnknownTgaFile* file = UnknownFunction5125c0("partblt.tga", 0, 0);
    RenderTarget* render = target;
    if (!render) {
        for (int i = 0; i < field_0x08; i++) {
            if (field_0x10[i].width == 640 && field_0x10[i].height == 480 && field_0x10[i].bitDepth == 16) {
                if (!UnknownFunction4ca900(i, 1)) {
                    result = -3;
                } else {
                    render = (new (__FILE__, 2053) PCRenderTarget)
                                 ->UnknownFunction4c4f80(this, &IID_IDirect3DHALDevice, field_0x1a0, 1, field_0x78);
                    if (render) {
                        result = 1;
                        g_UnknownGlobal56e26c->field_0x10 = render;
                        g_UnknownGlobal56e26c->field_0x0c = this;
                        break;
                    }
                    result = -3;
                }
            }
        }
    }
    if (render) {
        if (!file) {
            result = -2;
        } else {
            image = new (__FILE__, 2078) PCTextureMap(0, 0);
            if (!image->UnknownVirtualSlot4(file->bits, file->width, file->height, file->width, file->width,
                                            file->bitsPerPixel == 16 ? 0x22b : 0x378, render->field_0x28, 0, 0,
                                            render->field_0x04->field_0x198, 0, 0, 2, 1, 0, 0x80, 0xff00ff)) {
                UnknownFunction512dd0(file);
                result = -2;
            } else {
                expected = new (__FILE__, 2097) PCTextureMap(0, 0);
                expected->UnknownVirtualSlot4(0, file->width, file->height, file->width, file->width,
                                              render->field_0x28, render->field_0x28, 0, 0,
                                              render->field_0x04->field_0x198, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
                rendered = new (__FILE__, 2110) PCTextureMap(0, 0);
                rendered->UnknownVirtualSlot4(0, file->width, file->height, file->width, file->width,
                                              render->field_0x28, render->field_0x28, 0, 0,
                                              render->field_0x04->field_0x198, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
                if (!rendered->UnknownVirtualSlot8(1, 0, 1)) {
                    UnknownFunction512dd0(file);
                    result = -4;
                } else {
                    UnknownFunction512dd0(file);
                    if (UnknownFunction4caf70((PCRenderTarget*)render, &area, image, expected, rendered, this, 1) != 1) {
                        result = -1;
                    } else {
                        for (int y = 0; result == 1 && y < 256; y += 64) {
                            for (int x = 0; result == 1 && x < 256; x += 64) {
                                tile.left = x;
                                tile.top = y;
                                tile.right = x + 64;
                                tile.bottom = y + 64;
                                if (!UnknownFunction4caf70((PCRenderTarget*)render, &tile, image, expected, rendered,
                                                           this, 1))
                                    result = 0;
                            }
                        }
                    }
                }
            }
            if (image)
                image->Release();
            if (rendered)
                rendered->Release();
            if (expected)
                expected->Release();
        }
        if (!target && render) {
            delete render;
            g_UnknownGlobal56e26c->field_0x10 = 0;
            g_UnknownGlobal56e26c->field_0x0c = 0;
        }
    }
    field_0x5bc = result;
}


// 0x004cb5b0: with `enable`, flips until the GDI surface is the primary
// (at most back buffers + 1 times); then sets +0x6c.
void UnknownDisplay::UnknownFunction4cb5b0(int enable) {
    if (enable) {
        UnknownSurfaceInterface* gdi = 0;
        field_0x190->UnknownMethod14(&gdi);
        for (int i = 0; field_0x19c != gdi; ) {
            if (gdi)
                gdi->UnknownMethod2();
            gdi = 0;
            if (field_0x19c->UnknownMethod11(0, 1))
                return;
            if (field_0x190->UnknownMethod14(&gdi))
                return;
            g_UnknownGlobal56e26c->field_0x10->UnknownFunction4e8cc0();
            if (++i > field_0x78)
                return;
        }
        if (gdi)
            gdi->UnknownMethod2();
    }
    field_0x6c = enable;
}

// 0x0052d180
VideoCard::VideoCard() {
    field_0x70_bit2 = 0;
    field_0x80 = 0x7fffffff;
    field_0x84 = 0x7fffffff;
    field_0x68 = 0;
    field_0x64 = 0;
    field_0x0c = -1;
    field_0x88 = 0;
    field_0x78 = 0;
    field_0x04 = 0;
    field_0x10 = 0;
    field_0x7c = 0;
    field_0x58 = 0;
    field_0x5c = 0;
    memset(field_0x90, 0, sizeof(field_0x90));
    field_0x8c = 0;
    field_0x70_bit0 = 0;
    field_0x6c = 0;
    field_0x54 = 0;
}
