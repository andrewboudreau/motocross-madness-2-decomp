#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "PCRenderTarget.h"
#include "DebugAlloc.h"
#include "Tgafile.h"
#include "Camera.h"

// 0x004c4ee0: every cached render state starts as {i, 0}.
PCRenderTarget::PCRenderTarget() {
    memset(&field_0x54, 0, sizeof(field_0x54));
    field_0x48 = 0;
    field_0x50 = 0;
    field_0x4c = 0;
    field_0x254 = 0;
    field_0x25c = 0;
    field_0x258 = 0;
    field_0x260 = 0;
    field_0x250 = 3;
    memset(field_0x264, 0, sizeof(field_0x264));
    for (int i = 0; i < 300; i++)
        field_0x264[i].state = i;
}

// 0x004c5320
PCRenderTarget::~PCRenderTarget() {
    if (field_0x258) {
        operator delete(field_0x258, __FILE__, 196);
        field_0x258 = 0;
        field_0x254 = 0;
    }
    if (field_0x50) {
        field_0x50->UnknownMethod2();
        field_0x50 = 0;
    }
    if (field_0x4c) {
        field_0x4c->UnknownMethod2();
        field_0x4c = 0;
    }
    if (field_0x260)
        operator delete(field_0x260, __FILE__, 210);
}

// 0x004c53d0
int PCRenderTarget::UnknownVirtualSlot1() {
    return field_0x50->UnknownMethod5() == 0;
}

// 0x004c53e0
int PCRenderTarget::UnknownVirtualSlot2() {
    return field_0x50->UnknownMethod6() == 0;
}

// 0x004c53f0
int PCRenderTarget::UnknownVirtualSlot3(void* destination, void* source, void* sourceRect, int flags) {
    return field_0x48->UnknownMethod5(destination, static_cast<UnknownBlitSource*>(source)->field_0x70,
                                      sourceRect, flags, 0) == 0;
}

// 0x004c5490
int PCRenderTarget::UnknownVirtualSlot5(void* rect) {
    return field_0x48->UnknownMethod32(rect) == 0;
}

// 0x004c54b0
long PCRenderTarget::UnknownVirtualSlot6(int stage, int type, int* value) {
    return field_0x50->UnknownMethod36(stage, type, value);
}

// 0x004c54d0
long PCRenderTarget::UnknownVirtualSlot7(int stage, int type, int value) {
    return field_0x50->UnknownMethod37(stage, type, value);
}

// 0x004c54f0
int PCRenderTarget::UnknownVirtualSlot14(void* viewport) {
    return field_0x50->UnknownMethod13(viewport) == 0;
}

// 0x004c56e0: skips states whose cached value already matches unless forced.
void PCRenderTarget::UnknownVirtualSlot8(int state, int value, int force) {
    if (force || field_0x264[state].value != value) {
        field_0x50->UnknownMethod20(state, value);
        field_0x264[state].value = value;
    }
}

// 0x004c5720
long PCRenderTarget::UnknownVirtualSlot9(int state, int* value) {
    return field_0x50->UnknownMethod21(state, value);
}

// 0x004c5930
long PCRenderTarget::UnknownVirtualSlot11(int stage) {
    return field_0x50->UnknownMethod35(stage, 0);
}

// 0x004c5ed0: three render states (0x19 = 5, 0x18 = 0, 0x0f = 0), unforced.
void PCRenderTarget::UnknownVirtualSlot19() {
    UnknownVirtualSlot8(0x19, 5, 0);
    UnknownVirtualSlot8(0x18, 0, 0);
    UnknownVirtualSlot8(0x0F, 0, 0);
}

// 0x004c5230: appends a texture format the device enumerates.
long __stdcall PCRenderTarget::UnknownFunction4c5230(UnknownPixelFormat* format, void* context) {
    PCRenderTarget* target = static_cast<PCRenderTarget*>(context);
    target->field_0x258 = static_cast<UnknownPixelFormat*>(
        DebugRealloc(target->field_0x258, (target->field_0x254 + 1) * sizeof(UnknownPixelFormat), __FILE__, 20));
    if (target->field_0x258) {
        target->field_0x258[target->field_0x254] = *format;
        target->field_0x254++;
        return 1;
    }
    return 1;
}

// 0x004c52a0: appends an enumerated Z-buffer format (flag 0x400).
long __stdcall PCRenderTarget::UnknownFunction4c52a0(UnknownPixelFormat* format, void* context) {
    if (format->flags & 0x400) {
        PCRenderTarget* target = static_cast<PCRenderTarget*>(context);
        target->field_0x260 = static_cast<UnknownPixelFormat*>(
            DebugRealloc(target->field_0x260, (target->field_0x25c + 1) * sizeof(UnknownPixelFormat), __FILE__, 39));
        if (target->field_0x260) {
            target->field_0x260[target->field_0x25c] = *format;
            target->field_0x25c++;
            return 1;
        }
    }
    return 1;
}

// 0x004c5420: locks the surface; returns the bits (0 on failure) and the pitch.
void* PCRenderTarget::UnknownVirtualSlot4(void* rect, long* pitch, int flags) {
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (field_0x48->UnknownMethod25(rect, &desc, flags, 0) != 0)
        return 0;
    if (pitch)
        *pitch = desc.pitch;
    return desc.surface;
}

// 0x004c5640: whether the device enumerated a texture format matching `format`.
int PCRenderTarget::UnknownVirtualSlot13(int format) {
    UnknownPixelFormat wanted;
    UnknownFunction5119c0(format, &wanted);
    for (int i = 0; i < field_0x254; i++) {
        UnknownPixelFormat entry = field_0x258[i];
        if (entry.flags == wanted.flags && entry.fourCC == wanted.fourCC && entry.bitCount == wanted.bitCount &&
            entry.masks[1] == wanted.masks[1] && entry.masks[3] == wanted.masks[3])
            return 1;
    }
    return 0;
}

// 0x004c5b20: DrawIndexedPrimitive, counting vertices of the two lit formats
// (+0x38) and points (+0x3c), lines (+0x40) and triangles (+0x44) by type.
int PCRenderTarget::UnknownVirtualSlot15(int type, int vertexFormat, int vertices, int vertexCount,
                                         int indices, int indexCount, int flags) {
    if (vertexFormat == 0x112 || vertexFormat == 0x1e2)
        field_0x38 += vertexCount;
    switch (type) {
    case 1: field_0x3c += indexCount; break;
    case 2:
    case 3: field_0x40 += (unsigned int)indexCount / 2; break;
    case 4: field_0x44 += (unsigned int)indexCount / 3; break;
    case 5:
    case 6: field_0x44 += indexCount; break;
    }
    return field_0x50->UnknownMethod26(type, vertexFormat, (void*)vertices, vertexCount, (void*)indices,
                                       indexCount, flags) == 0;
}

// 0x004c5bd0: DrawPrimitive with the same counters.
int PCRenderTarget::UnknownVirtualSlot16(int type, int vertexFormat, int vertices, int count, int flags) {
    if (vertexFormat == 0x112 || vertexFormat == 0x1e2)
        field_0x38 += count;
    switch (type) {
    case 1: field_0x3c += count; break;
    case 2:
    case 3: field_0x40 += (unsigned int)count / 2; break;
    case 4: field_0x44 += (unsigned int)count / 3; break;
    case 5:
    case 6: field_0x44 += count; break;
    }
    return field_0x50->UnknownMethod25(type, vertexFormat, (void*)vertices, count, flags) == 0;
}

// 0x004c5c70: DrawIndexedPrimitiveVB with the primitive counters.
int PCRenderTarget::UnknownVirtualSlot17(int type, int vertexBuffer, int start, int vertexCount,
                                         int indices, int indexCount, int flags) {
    switch (type) {
    case 1: field_0x3c += indexCount; break;
    case 2:
    case 3: field_0x40 += (unsigned int)indexCount / 2; break;
    case 4: field_0x44 += (unsigned int)indexCount / 3; break;
    case 5:
    case 6: field_0x44 += indexCount; break;
    }
    return field_0x50->UnknownMethod32(type, (void*)vertexBuffer, start, vertexCount, (void*)indices,
                                       indexCount, flags) == 0;
}

// 0x004c5e60: fog/blend states by capability: 0x10 sets state 0x18 to the
// low byte of `value`, 0x20 clears it.
void PCRenderTarget::UnknownVirtualSlot18(int value) {
    if (field_0x1b8 & 0x10) {
        UnknownVirtualSlot8(0x18, value & 0xff, 0);
        UnknownVirtualSlot8(0x0f, 1, 0);
        UnknownVirtualSlot8(0x19, 5, 0);
    } else if (field_0x1b8 & 0x20) {
        UnknownVirtualSlot8(0x18, 0, 0);
        UnknownVirtualSlot8(0x0f, 1, 0);
        UnknownVirtualSlot8(0x19, 6, 0);
    }
}

// 0x004c5740: texture stage 0 colour/alpha operations for a blend mode
// (SetTextureStageState types 1-6).
void PCRenderTarget::UnknownVirtualSlot10(int mode, int flag) {
    switch (mode) {
    case 1:
    case 7:
        field_0x50->UnknownMethod37(0, 1, 2);
        field_0x50->UnknownMethod37(0, 2, 2);
        field_0x50->UnknownMethod37(0, 4, 2);
        field_0x50->UnknownMethod37(0, 5, 2);
        break;
    case 2:
        field_0x50->UnknownMethod37(0, 1, 4);
        field_0x50->UnknownMethod37(0, 2, 2);
        field_0x50->UnknownMethod37(0, 3, 0);
        if (flag) {
            field_0x50->UnknownMethod37(0, 4, 2);
            field_0x50->UnknownMethod37(0, 5, 2);
        } else {
            field_0x50->UnknownMethod37(0, 4, 2);
            field_0x50->UnknownMethod37(0, 5, 0);
        }
        break;
    case 4:
        field_0x50->UnknownMethod37(0, 1, 4);
        field_0x50->UnknownMethod37(0, 2, 2);
        field_0x50->UnknownMethod37(0, 3, 0);
        field_0x50->UnknownMethod37(0, 4, 4);
        field_0x50->UnknownMethod37(0, 5, 2);
        field_0x50->UnknownMethod37(0, 6, 0);
        break;
    case 8:
        field_0x50->UnknownMethod37(0, 1, 7);
        field_0x50->UnknownMethod37(0, 2, 2);
        field_0x50->UnknownMethod37(0, 3, 0);
        field_0x50->UnknownMethod37(0, 4, 2);
        field_0x50->UnknownMethod37(0, 5, 0);
        break;
    case 3:
        field_0x50->UnknownMethod37(0, 1, 13);
        field_0x50->UnknownMethod37(0, 2, 2);
        field_0x50->UnknownMethod37(0, 3, 0);
        field_0x50->UnknownMethod37(0, 4, 2);
        field_0x50->UnknownMethod37(0, 5, 0);
        break;
    }
}

// 0x004c5950: counts the 256x256 (128 KB each) and then 32x32 (2 KB each)
// textures that fit in video memory (surface caps 0x10000000) until creation
// fails or a surface lands in system memory (0x20000000), then frees them.
int PCRenderTarget::UnknownFunction4c5950(int* value) {
    UnknownSurfaceInterface* surfaces[0x4000];
    UnknownSurfaceDesc desc;
    UnknownSurfaceCaps caps;
    int count = 0;
    int found = 0;
    for (; count < 0x4000; count++) {
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        UnknownFunction5119c0(field_0x28, &desc.pixelFormat);
        desc.width = 0x100;
        desc.height = 0x100;
        desc.flags = 0x1007;
        desc.caps[0] = 0x10005000;
        if (field_0x04->field_0x190->UnknownMethod6(&desc, &surfaces[count], 0) != 0)
            break;
        surfaces[count]->UnknownMethod14(&caps);
        if (caps.caps & 0x10000000)
            found++;
        if (caps.caps & 0x20000000)
            break;
    }
    int bytes = found << 17;
    found = 0;
    for (; count < 0x4000; count++) {
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        UnknownFunction5119c0(field_0x28, &desc.pixelFormat);
        desc.width = 0x20;
        desc.height = 0x20;
        desc.flags = 0x1007;
        desc.caps[0] = 0x10005000;
        if (field_0x04->field_0x190->UnknownMethod6(&desc, &surfaces[count], 0) != 0)
            break;
        surfaces[count]->UnknownMethod14(&caps);
        if (caps.caps & 0x10000000)
            found++;
        if (caps.caps & 0x20000000)
            break;
    }
    bytes += found << 11;
    for (int i = 0; i < count; i++)
        surfaces[i]->UnknownMethod2();
    *value = bytes;
    return 1;
}

// 0x0068995c: the next screenshot number.
static int s_screenshotIndex;

static inline int FileExists(const char* path) {
    FILE* file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

// 0x004c5d00: writes the locked surface (16, 24 or 32 bits) to the first
// "<computer name><index>.TGA" that does not exist yet.
void PCRenderTarget::UnknownFunction4c5d00() {
    char computer[16];
    unsigned long size = sizeof(computer);
    char path[MAX_PATH];
    UnknownSurfaceDesc desc;
    GetComputerNameA(computer, &size);
    sprintf(path, "%s%05d.TGA", computer, s_screenshotIndex);
    while (FileExists(path)) {
        s_screenshotIndex++;
        sprintf(path, "%s%05d.TGA", computer, s_screenshotIndex);
    }
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (field_0x48->UnknownMethod25(0, &desc, 0x811, 0) == 0) {
        if (desc.pixelFormat.bitCount == 16)
            UnknownFunction5128c0(desc.surface, desc.width, desc.height, desc.pitch, desc.pixelFormat.masks[1],
                                  path, 0x20);
        else if (desc.pixelFormat.bitCount == 24)
            UnknownFunction512720(desc.surface, desc.width, desc.height, desc.pitch, path, 0x20);
        else if (desc.pixelFormat.bitCount == 32)
            UnknownFunction5127f0(desc.surface, desc.width, desc.height, desc.pitch, path, 0x20);
        field_0x48->UnknownMethod32(0);
    }
}

// 0x004c4f80: attaches the display, device GUID, surface and frame modulus,
// reads the surface size and pixel format, attaches a Z buffer of the same
// depth when `zbuffer` is set, then creates the device and collects its caps
// and texture formats. On any failure the target deletes itself.
RenderTarget* PCRenderTarget::UnknownFunction4c4f80(UnknownDisplay* display, const UnknownGuid* device,
                                                    UnknownSurfaceInterface* surface, int zbuffer, int frames) {
    UnknownPixelFormat format;
    UnknownSurfaceDesc desc;
    UnknownFunction4e8ca0(display);
    field_0x48 = surface;
    field_0x14 = frames;
    field_0x04 = display;
    field_0x54 = *device;
    if (display->field_0x194->UnknownMethod6(&field_0x54, UnknownFunction4c52a0, this) != 0)
        goto fail;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = 0x1007;
    if (field_0x48->UnknownMethod22(&desc) != 0)
        goto fail;
    field_0x0c = desc.width;
    field_0x10 = desc.height;
    {
        int memory = desc.caps[0] & 0x4000;
        memset(&format, 0, sizeof(format));
        format.size = sizeof(format);
        if (field_0x48->UnknownMethod21(&format) != 0)
            goto fail;
        field_0x28 = UnknownFunction511af0(&format);
        field_0x24 = memory ? 0x4000 : 0x800;
        int depth;
        if (field_0x28 == 0x22b || field_0x28 == 0x235)
            depth = 16;
        else
            depth = 32;
        field_0x20 = depth;
        if (zbuffer && depth) {
            int i;
            for (i = 0; i < field_0x25c; i++)
                if (field_0x260[i].bitCount == (unsigned long)depth)
                    break;
            if (i == field_0x25c)
                goto fail;
            memory = field_0x24 | 0x20000;
            memset(&desc, 0, sizeof(desc));
            desc.caps[0] = memory;
            desc.width = field_0x0c;
            desc.size = sizeof(desc);
            desc.flags = 0x1007;
            desc.height = field_0x10;
            desc.pixelFormat = field_0x260[i];
            desc.pixelFormat.size = sizeof(UnknownPixelFormat);
            long result = field_0x04->field_0x190->UnknownMethod6(&desc, &field_0x4c, 0);
            if (result == 0x8876017c || result != 0)
                goto fail;
            if (field_0x48->UnknownMethod3(field_0x4c) != 0)
                goto fail;
        }
    }
    if (field_0x04->field_0x194->UnknownMethod4(&field_0x54, field_0x48, &field_0x50) != 0)
        goto fail;
    memset(&field_0x164, 0, 0x250 - 0x164);
    if (field_0x50->UnknownMethod3(&field_0x164) != 0)
        goto fail;
    memset(&format, 0, sizeof(format));
    format.size = sizeof(format);
    if (field_0x48->UnknownMethod21(&format) != 0)
        goto fail;
    field_0x28 = UnknownFunction511af0(&format);
    field_0x2c = 1.0f;
    if (field_0x50->UnknownMethod4(UnknownFunction4c5230, this) != 0) {
    fail:
        delete this;
        return 0;
    }
    return this;
}
