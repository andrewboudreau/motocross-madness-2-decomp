#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "PCRenderTarget.h"
#include "DebugAlloc.h"
#include "Tgafile.h"
#include "Camera.h"
#include "D3DConstants.h"

// 0x004c4ee0: every cached render state starts as {i, 0}.
PCRenderTarget::PCRenderTarget() {
    memset(&deviceGuid, 0, sizeof(deviceGuid));
    renderSurface = 0;
    device = 0;
    zbuffer = 0;
    textureFormatCount = 0;
    zbufferFormatCount = 0;
    textureFormats = 0;
    zbufferFormats = 0;
    fillMode = 3;
    memset(renderStates, 0, sizeof(renderStates));
    for (int i = 0; i < 300; i++)
        renderStates[i].state = i;
}

// 0x004c5320
PCRenderTarget::~PCRenderTarget() {
    if (textureFormats) {
        DebugFree(textureFormats, __FILE__, 196);
        textureFormats = 0;
        textureFormatCount = 0;
    }
    if (device) {
        device->Release();
        device = 0;
    }
    if (zbuffer) {
        zbuffer->Release();
        zbuffer = 0;
    }
    if (zbufferFormats)
        DebugFree(zbufferFormats, __FILE__, 210);
}

// 0x004c53d0
int PCRenderTarget::UnknownVirtualSlot1() {
    return device->BeginScene() == 0;
}

// 0x004c53e0
int PCRenderTarget::UnknownVirtualSlot2() {
    return device->EndScene() == 0;
}

// 0x004c53f0
int PCRenderTarget::UnknownVirtualSlot3(void* destination, void* source, void* sourceRect, int flags) {
    return renderSurface->Blt(destination, static_cast<UnknownBlitSource*>(source)->field_0x70,
                                      sourceRect, flags, 0) == 0;
}

// 0x004c5490
int PCRenderTarget::UnknownVirtualSlot5(void* rect) {
    return renderSurface->Unlock(rect) == 0;
}

// 0x004c54b0
long PCRenderTarget::UnknownVirtualSlot6(int stage, int type, int* value) {
    return device->GetTextureStageState(stage, type, value);
}

// 0x004c54d0
long PCRenderTarget::UnknownVirtualSlot7(int stage, int type, int value) {
    return device->SetTextureStageState(stage, type, value);
}

// 0x004c54f0
int PCRenderTarget::UnknownVirtualSlot14(void* viewport) {
    return device->SetViewport(viewport) == 0;
}

// 0x004c56e0: skips states whose cached value already matches unless forced.
void PCRenderTarget::UnknownVirtualSlot8(int state, int value, int force) {
    if (force || renderStates[state].value != value) {
        device->SetRenderState(state, value);
        renderStates[state].value = value;
    }
}

// 0x004c5720
long PCRenderTarget::UnknownVirtualSlot9(int state, int* value) {
    return device->GetRenderState(state, value);
}

// 0x004c5930
long PCRenderTarget::UnknownVirtualSlot11(int stage) {
    return device->SetTexture(stage, 0);
}

// 0x004c5ed0: resets the alpha test (GREATER than 0, disabled), unforced.
void PCRenderTarget::UnknownVirtualSlot19() {
    UnknownVirtualSlot8(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER, 0);
    UnknownVirtualSlot8(D3DRENDERSTATE_ALPHAREF, 0, 0);
    UnknownVirtualSlot8(D3DRENDERSTATE_ALPHATESTENABLE, 0, 0);
}

// 0x004c5230: appends a texture format the device enumerates.
long __stdcall PCRenderTarget::EnumTextureFormatCallback(UnknownPixelFormat* format, void* context) {
    PCRenderTarget* target = static_cast<PCRenderTarget*>(context);
    target->textureFormats = static_cast<UnknownPixelFormat*>(
        DebugRealloc(target->textureFormats, (target->textureFormatCount + 1) * sizeof(UnknownPixelFormat), __FILE__, 20));
    if (target->textureFormats) {
        target->textureFormats[target->textureFormatCount] = *format;
        target->textureFormatCount++;
        return 1;
    }
    return 1;
}

// 0x004c52a0: appends an enumerated Z-buffer format (flag 0x400).
long __stdcall PCRenderTarget::EnumZBufferFormatCallback(UnknownPixelFormat* format, void* context) {
    if (format->flags & DDPF_ZBUFFER) {
        PCRenderTarget* target = static_cast<PCRenderTarget*>(context);
        target->zbufferFormats = static_cast<UnknownPixelFormat*>(
            DebugRealloc(target->zbufferFormats, (target->zbufferFormatCount + 1) * sizeof(UnknownPixelFormat), __FILE__, 39));
        if (target->zbufferFormats) {
            target->zbufferFormats[target->zbufferFormatCount] = *format;
            target->zbufferFormatCount++;
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
    if (renderSurface->Lock(rect, &desc, flags, 0) != 0)
        return 0;
    if (pitch)
        *pitch = desc.pitch;
    return desc.surface;
}

// 0x004c5640: whether the device enumerated a texture format matching `format`.
int PCRenderTarget::UnknownVirtualSlot13(int format) {
    UnknownPixelFormat wanted;
    UnknownFunction5119c0(format, &wanted);
    for (int i = 0; i < textureFormatCount; i++) {
        UnknownPixelFormat entry = textureFormats[i];
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
    if (vertexFormat == D3DFVF_VERTEX || vertexFormat == D3DFVF_LVERTEX)
        field_0x38 += vertexCount;
    switch (type) {
    case D3DPT_POINTLIST: field_0x3c += indexCount; break;
    case D3DPT_LINELIST:
    case D3DPT_LINESTRIP: field_0x40 += (unsigned int)indexCount / 2; break;
    case D3DPT_TRIANGLELIST: field_0x44 += (unsigned int)indexCount / 3; break;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN: field_0x44 += indexCount; break;
    }
    return device->DrawIndexedPrimitive(type, vertexFormat, (void*)vertices, vertexCount, (void*)indices,
                                       indexCount, flags) == 0;
}

// 0x004c5bd0: DrawPrimitive with the same counters.
int PCRenderTarget::UnknownVirtualSlot16(int type, int vertexFormat, int vertices, int count, int flags) {
    if (vertexFormat == D3DFVF_VERTEX || vertexFormat == D3DFVF_LVERTEX)
        field_0x38 += count;
    switch (type) {
    case D3DPT_POINTLIST: field_0x3c += count; break;
    case D3DPT_LINELIST:
    case D3DPT_LINESTRIP: field_0x40 += (unsigned int)count / 2; break;
    case D3DPT_TRIANGLELIST: field_0x44 += (unsigned int)count / 3; break;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN: field_0x44 += count; break;
    }
    return device->DrawPrimitive(type, vertexFormat, (void*)vertices, count, flags) == 0;
}

// 0x004c5c70: DrawIndexedPrimitiveVB with the primitive counters.
int PCRenderTarget::UnknownVirtualSlot17(int type, int vertexBuffer, int start, int vertexCount,
                                         int indices, int indexCount, int flags) {
    switch (type) {
    case D3DPT_POINTLIST: field_0x3c += indexCount; break;
    case D3DPT_LINELIST:
    case D3DPT_LINESTRIP: field_0x40 += (unsigned int)indexCount / 2; break;
    case D3DPT_TRIANGLELIST: field_0x44 += (unsigned int)indexCount / 3; break;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN: field_0x44 += indexCount; break;
    }
    return device->DrawIndexedPrimitiveVB(type, (void*)vertexBuffer, start, vertexCount, (void*)indices,
                                       indexCount, flags) == 0;
}

// 0x004c5e60: enables the alpha test: alpha GREATER than the low byte of
// `reference` when the device supports it, else alpha NOTEQUAL to 0.
void PCRenderTarget::UnknownVirtualSlot18(int reference) {
    if (triAlphaCmpCaps & D3DPCMPCAPS_GREATER) {
        UnknownVirtualSlot8(D3DRENDERSTATE_ALPHAREF, reference & 0xff, 0);
        UnknownVirtualSlot8(D3DRENDERSTATE_ALPHATESTENABLE, 1, 0);
        UnknownVirtualSlot8(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER, 0);
    } else if (triAlphaCmpCaps & D3DPCMPCAPS_NOTEQUAL) {
        UnknownVirtualSlot8(D3DRENDERSTATE_ALPHAREF, 0, 0);
        UnknownVirtualSlot8(D3DRENDERSTATE_ALPHATESTENABLE, 1, 0);
        UnknownVirtualSlot8(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_NOTEQUAL, 0);
    }
}

// 0x004c5740: texture stage 0 colour and alpha operations for a blend mode
// (SetTextureStageState). Mode 2 takes the alpha from the texture only when
// `textureAlpha` is set.
void PCRenderTarget::UnknownVirtualSlot10(int mode, int textureAlpha) {
    switch (mode) {
    case 1:
    case 7:
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        break;
    case 2:
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        if (textureAlpha) {
            device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        } else {
            device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        }
        break;
    case 4:
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        break;
    case 8:
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        break;
    case 3:
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        break;
    }
}

// 0x004c5950: counts the 256x256 (128 KB each) and then 32x32 (2 KB each)
// textures that fit in video memory (surface caps 0x10000000) until creation
// fails or a surface lands in system memory (0x20000000), then frees them.
int PCRenderTarget::MeasureTextureMemory(int* value) {
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
        desc.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        desc.caps[0] = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
        if (field_0x04->directDraw->CreateSurface(&desc, &surfaces[count], 0) != 0)
            break;
        surfaces[count]->GetCaps(&caps);
        if (caps.caps & DDSCAPS_LOCALVIDMEM)
            found++;
        if (caps.caps & DDSCAPS_NONLOCALVIDMEM)
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
        desc.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        desc.caps[0] = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
        if (field_0x04->directDraw->CreateSurface(&desc, &surfaces[count], 0) != 0)
            break;
        surfaces[count]->GetCaps(&caps);
        if (caps.caps & DDSCAPS_LOCALVIDMEM)
            found++;
        if (caps.caps & DDSCAPS_NONLOCALVIDMEM)
            break;
    }
    bytes += found << 11;
    for (int i = 0; i < count; i++)
        surfaces[i]->Release();
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
void PCRenderTarget::SaveScreenshot() {
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
    if (renderSurface->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_READONLY | DDLOCK_NOSYSLOCK, 0) == 0) {
        if (desc.pixelFormat.bitCount == 16)
            WriteTga16(desc.surface, desc.width, desc.height, desc.pitch, desc.pixelFormat.masks[1],
                                  path, 0x20);
        else if (desc.pixelFormat.bitCount == 24)
            WriteTga24(desc.surface, desc.width, desc.height, desc.pitch, path, 0x20);
        else if (desc.pixelFormat.bitCount == 32)
            WriteTga32(desc.surface, desc.width, desc.height, desc.pitch, path, 0x20);
        renderSurface->Unlock(0);
    }
}

// 0x004c4f80: attaches the display, device GUID, surface and frame modulus,
// reads the surface size and pixel format, attaches a Z buffer of the same
// depth when `zbuffer` is set, then creates the device and collects its caps
// and texture formats. On any failure the target deletes itself.
RenderTarget* PCRenderTarget::InitializeRenderTarget(UnknownDisplay* display, const UnknownGuid* deviceId,
    UnknownSurfaceInterface* surface, int wantZBuffer, int frames) {
    UnknownPixelFormat format;
    UnknownSurfaceDesc desc;
    UnknownFunction4e8ca0(display);
    renderSurface = surface;
    field_0x14 = frames;
    field_0x04 = display;
    deviceGuid = *deviceId;
    if (display->direct3D->EnumZBufferFormats(&deviceGuid, EnumZBufferFormatCallback, this) != 0)
        goto fail;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    if (renderSurface->GetSurfaceDesc(&desc) != 0)
        goto fail;
    field_0x0c = desc.width;
    field_0x10 = desc.height;
    {
        int memory = desc.caps[0] & DDSCAPS_VIDEOMEMORY;
        memset(&format, 0, sizeof(format));
        format.size = sizeof(format);
        if (renderSurface->GetPixelFormat(&format) != 0)
            goto fail;
        field_0x28 = FormatFromPixelFormat(&format);
        field_0x24 = memory ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY;
        int depth;
        if (field_0x28 == 555 || field_0x28 == 565)
            depth = 16;
        else
            depth = 32;
        field_0x20 = depth;
        if (wantZBuffer && depth) {
            int i;
            for (i = 0; i < zbufferFormatCount; i++)
                if (zbufferFormats[i].bitCount == (unsigned long)depth)
                    break;
            if (i == zbufferFormatCount)
                goto fail;
            memory = field_0x24 | DDSCAPS_ZBUFFER;
            memset(&desc, 0, sizeof(desc));
            desc.caps[0] = memory;
            desc.width = field_0x0c;
            desc.size = sizeof(desc);
            desc.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
            desc.height = field_0x10;
            desc.pixelFormat = zbufferFormats[i];
            desc.pixelFormat.size = sizeof(UnknownPixelFormat);
            long result = field_0x04->directDraw->CreateSurface(&desc, &zbuffer, 0);
            if (result == DDERR_OUTOFVIDEOMEMORY || result != 0)
                goto fail;
            if (renderSurface->AddAttachedSurface(zbuffer) != 0)
                goto fail;
        }
    }
    if (field_0x04->direct3D->CreateDevice(&deviceGuid, renderSurface, &device) != 0)
        goto fail;
    memset(&deviceCaps, 0, 0x250 - 0x164); // the D3DDEVICEDESC7
    if (device->GetCaps(&deviceCaps) != 0)
        goto fail;
    memset(&format, 0, sizeof(format));
    format.size = sizeof(format);
    if (renderSurface->GetPixelFormat(&format) != 0)
        goto fail;
    field_0x28 = FormatFromPixelFormat(&format);
    field_0x2c = 1.0f;
    if (device->EnumTextureFormats(EnumTextureFormatCallback, this) != 0) {
    fail:
        delete this;
        return 0;
    }
    return this;
}
