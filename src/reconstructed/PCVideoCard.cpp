#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PCVideoCard.h"

#include "DebugAlloc.h"
#include "PCRenderTarget.h"
#include "PCTextureMap.h"
#include "TextureMap.h"
#include "Tgafile.h"
#include "TrackGame.h"

// DirectDraw result codes and flags used below (ddraw.h values).
#define UNKNOWN_DDERR_OUTOFVIDEOMEMORY ((long)0x8876017c)
#define UNKNOWN_DDERR_WASSTILLDRAWING ((long)0x8876021c)

// 0x004c94a0: DirectDrawEnumerateA callback; `context` points at the window.
static int __stdcall UnknownEnumCallback4c94a0(UnknownGuid* guid, char* description, char* name,
                                               void* context) {
    if (g_UnknownDisplayCount68a764 < 4) {
        UnknownDisplay* display = new (__FILE__, 280) UnknownDisplay;
        display = display->UnknownFunction4c9830(guid, description, name, *(void**)context);
        g_UnknownDisplays68a754[g_UnknownDisplayCount68a764] = display;
        if (display)
            g_UnknownDisplayCount68a764++;
    }
    return 1;
}

// 0x004c9550: DirectDrawEnumerateExA callback (the monitor is not used).
static int __stdcall UnknownEnumCallback4c9550(UnknownGuid* guid, char* description, char* name,
                                               void* context, void* monitor) {
    if (g_UnknownDisplayCount68a764 < 4) {
        UnknownDisplay* display = new (__FILE__, 307) UnknownDisplay;
        display = display->UnknownFunction4c9830(guid, description, name, *(void**)context);
        g_UnknownDisplays68a754[g_UnknownDisplayCount68a764] = display;
        if (display)
            g_UnknownDisplayCount68a764++;
    }
    return 1;
}

typedef long(__stdcall* UnknownEnumerateEx)(
    int(__stdcall* callback)(UnknownGuid*, char*, char*, void*, void*), void* context,
    unsigned long flags);

// 0x004c9600
int UnknownFunction4c9600(void* window) {
    HMODULE library = LoadLibraryA("ddraw.dll");
    if (!library)
        return 0;
    UnknownEnumerateEx enumerate = (UnknownEnumerateEx)GetProcAddress(library, "DirectDrawEnumerateExA");
    if (enumerate) {
        // DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES
        enumerate(UnknownEnumCallback4c9550, &window, 5);
        FreeLibrary(library);
        return g_UnknownDisplayCount68a764;
    }
    DirectDrawEnumerateA(UnknownEnumCallback4c94a0, &window);
    FreeLibrary(library);
    return g_UnknownDisplayCount68a764;
}

// 0x004c9660: EnumDisplayModes callback; appends the mode, growing the
// table by 20 entries.
static long __stdcall UnknownEnumModesCallback4c9660(UnknownSurfaceDesc* desc, void* context) {
    UnknownDisplay* display = (UnknownDisplay*)context;
    if (display->field_0x08 >= display->field_0x04 - 1) {
        display->field_0x04 += 20;
        display->field_0x10 = (UnknownDisplayMode*)DebugRealloc(
            display->field_0x10, display->field_0x04 * sizeof(UnknownDisplayMode), __FILE__, 381);
        if (!display->field_0x10) {
            display->field_0x08 = 0;
            display->field_0x04 = 0;
        }
    }
    if (display->field_0x08 < display->field_0x04) {
        display->field_0x10[display->field_0x08].width = desc->width;
        display->field_0x10[display->field_0x08].height = desc->height;
        display->field_0x10[display->field_0x08].bitDepth = desc->pixelFormat.bitCount;
        display->field_0x10[display->field_0x08].refreshRate = desc->refreshRate;
        display->field_0x10[display->field_0x08].field_0x14 = 1;
        display->field_0x10[display->field_0x08].field_0x18 = 1;
        display->field_0x10[display->field_0x08].field_0x1c = 0;
        display->field_0x10[display->field_0x08].field_0x20 = 0;
        display->field_0x10[display->field_0x08].field_0x10 = 2;
        display->field_0x08++;
    }
    return 1;
}

// 0x004c9760
UnknownDisplay::UnknownDisplay() {
    memset(&field_0x4ac, 0, sizeof(field_0x4ac));
    memset(field_0x4bc, 0, sizeof(field_0x4bc));
    field_0x190 = 0;
    field_0x194 = 0;
    field_0x19c = 0;
    field_0x1a0 = 0;
    field_0x1a4 = 0;
    field_0x1a8 = 0;
    field_0x1ac = 0;
    field_0x198 = 0;
    field_0x5bc = 0;
    field_0xa70 = 8;                              // DDSCL_NORMAL
    field_0xb74_bit0 = 0;
    field_0xb74_bit1 = 0;
    field_0xb74_bit2 = 0;
    field_0xb74_bit3 = 0;
    field_0x70_bit3 = 0;
    field_0x9f0 = -1;
}

// 0x004c9830
UnknownDisplay* UnknownDisplay::UnknownFunction4c9830(UnknownGuid* guid, char* description, char* name,
                                                      void* window) {
    UnknownSurfaceCaps caps;
    unsigned long total;
    unsigned long free;
    UnknownSurfaceDesc desc;
    UnknownSurfaceDesc mode;
    char key[256];

    field_0x1b0 = window;
    field_0xb74_bit2 = g_UnknownGlobal56e26c->UnknownVirtualSlot22("IsPowerVR", 0);
    if (DirectDrawCreateEx(guid, (void**)&field_0x190, IID_IDirectDraw7, 0))
        goto failed;
    if (field_0x190->UnknownMethod27(&field_0x5c0, 0))
        goto failed;
    memset(&field_0x1b4, 0, 0x17c);
    field_0x1b4 = 0x17c;
    memset(&field_0x330, 0, 0x17c);
    field_0x330 = 0x17c;
    if (field_0x190->UnknownMethod11(&field_0x1b4, &field_0x330))
        goto failed;
    if (field_0x1bc & 0x100000)
        field_0xb74_bit3 = 0;
    memset(&caps, 0, sizeof(caps));
    caps.caps = 0x4000;                           // DDSCAPS_VIDEOMEMORY
    field_0x190->UnknownMethod23(&caps, &total, &free);
    field_0x08 = 0;
    if (field_0x190->UnknownMethod8(0, 0, this, UnknownEnumModesCallback4c9660))
        goto failed;
    qsort(field_0x10, field_0x08, sizeof(UnknownDisplayMode), UnknownCompare52d120);
    if (guid)
        field_0x4ac = *guid;
    sprintf(field_0x4bc, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", field_0x4ac.data1,
            field_0x4ac.data2, field_0x4ac.data3, field_0x4ac.data4[0], field_0x4ac.data4[1],
            field_0x4ac.data4[2], field_0x4ac.data4[3], field_0x4ac.data4[4], field_0x4ac.data4[5],
            field_0x4ac.data4[6], field_0x4ac.data4[7]);
    sprintf(key, "DriverInfo\\%s\\WaitForFlip", field_0x4bc);
    field_0xb74_bit0 = g_UnknownGlobal56e26c->UnknownVirtualSlot22(key, 0);
    strncpy(field_0xa74, description, sizeof(field_0xa74));
    strncpy(field_0xaf4, name, sizeof(field_0xaf4));
    field_0x74 = field_0x1f0;
    field_0xa74[sizeof(field_0xa74) - 1] = 0;
    field_0xaf4[sizeof(field_0xaf4) - 1] = 0;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (field_0x190->UnknownMethod12(&desc))
        goto failed;
    // The desktop's own surface comes off the video memory.
    field_0x74 += desc.height * desc.width * desc.pixelFormat.bitCount >> 3;
    memset(&mode, 0, sizeof(mode));
    mode.size = sizeof(mode);
    if (field_0x190->UnknownMethod12(&mode))
        goto failed;
    if (field_0x190->UnknownMethod0(&IID_IDirect3D7, (void**)&field_0x194))
        goto failed;
    return this;
failed:
    delete this;
    return 0;
}

// 0x004c9b50: re-reads the driver and emulation caps.
void UnknownDisplay::UnknownFunction4c9b50() {
    memset(&field_0x1b4, 0, 0x17c);
    field_0x1b4 = 0x17c;
    memset(&field_0x330, 0, 0x17c);
    field_0x330 = 0x17c;
    field_0x190->UnknownMethod11(&field_0x1b4, &field_0x330);
}

// 0x004c9ba0
void UnknownDisplay::UnknownVirtualSlot1() {
    UnknownFunction4c9c10();
    if (field_0x1a8 != field_0x1a0 && field_0x1a8) {
        field_0x1a8->UnknownMethod2();
        field_0x1a8 = 0;
    }
    field_0x1a0 = 0;
    if (field_0x194) {
        field_0x194->UnknownMethod2();
        field_0x194 = 0;
    }
    if (field_0x190) {
        field_0x190->UnknownMethod2();
        field_0x190 = 0;
    }
    VideoCard::UnknownVirtualSlot1();
}

// 0x004c9c10: waits for the primary's blits, then releases it and the
// gamma control.
void UnknownDisplay::UnknownFunction4c9c10() {
    if (field_0x1a4) {
        field_0x1a4->UnknownMethod2();
        field_0x1a4 = 0;
    }
    if (field_0x19c) {
        while (field_0x19c->UnknownMethod13(2) == UNKNOWN_DDERR_WASSTILLDRAWING) // DDGBS_ISBLTDONE
            Sleep(1);
        field_0x19c->UnknownMethod2();
        field_0x19c = 0;
    }
    field_0x1a8 = 0;
    field_0x1a0 = 0;
}

// 0x004c9c90
int UnknownDisplay::UnknownFunction4c9c90() {
    SetWindowLongA((HWND)field_0x1b0, GWL_STYLE,
                   GetWindowLongA((HWND)field_0x1b0, GWL_STYLE) & (WS_POPUP | WS_SYSMENU));
    ShowWindow((HWND)field_0x1b0, SW_SHOW);
    if (g_UnknownGlobal56e26c->field_0x538)
        UnknownFunction52ff00();
    // DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | 0x800
    if (field_0x190->UnknownMethod20(field_0x1b0, 0x813))
        return 0;
    field_0xa70 = 0x813;
    if (g_UnknownGlobal56e26c->field_0x538)
        UnknownFunction52ff20();
    return 1;
}

// 0x004c9d20: a window of the client size `width` x `height` at (x, y),
// kept inside the work area; hidden when the size is 0 x 0.
int UnknownDisplay::UnknownFunction4c9d20(int x, int y, int width, int height) {
    RECT rect;
    RECT workArea;

    if (g_UnknownGlobal56e26c->field_0x538)
        UnknownFunction52ff00();
    if (field_0x190->UnknownMethod20(field_0x1b0, 0x808))   // DDSCL_NORMAL | 0x800
        return 0;
    field_0xa70 = 0x808;
    SetWindowLongA((HWND)field_0x1b0, GWL_STYLE,
                   GetWindowLongA((HWND)field_0x1b0, GWL_STYLE) & 0x7f39ffff | 0xc60000);
    SetRect(&rect, x, y, width, height);
    AdjustWindowRectEx(&rect, GetWindowLongA((HWND)field_0x1b0, GWL_STYLE), GetMenu((HWND)field_0x1b0) != 0,
                       GetWindowLongA((HWND)field_0x1b0, GWL_EXSTYLE));
    OffsetRect(&rect, rect.left < 0 ? -rect.left : 0, rect.top < 0 ? -rect.top : 0);
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &workArea, 0);
    OffsetRect(&rect, rect.left < workArea.left ? workArea.left - rect.left : 0,
               rect.top < workArea.top ? workArea.top - rect.top : 0);
    SetWindowPos((HWND)field_0x1b0, HWND_TOP, rect.left, rect.top, rect.right - rect.left,
                 rect.bottom - rect.top, SWP_NOZORDER);
    if (!width && !height)
        ShowWindow((HWND)field_0x1b0, SW_HIDE);
    if (g_UnknownGlobal56e26c->field_0x538)
        UnknownFunction52ff20();
    return 1;
}

// 0x004c9ec0
int UnknownDisplay::UnknownVirtualSlot2(int width, int height, int bitDepth, int a, int b, int windowed,
                                        int c) {
    if (field_0xa70 & 8)
        return UnknownFunction4ca790(width, height, windowed);
    int mode = UnknownFunction52d250(width, height, bitDepth, a, b);
    if (mode == -1)
        return 0;
    return UnknownFunction4ca900(mode, windowed);
}

// 0x004c9f30: creates the full-screen flipping chain with `backBuffers`
// back buffers, clears and flips every buffer, then times ten flips.
int UnknownDisplay::UnknownFunction4c9f30(int backBuffers) {
    UnknownSurfaceCaps caps;
    RECT rect;
    UnknownSurfaceDesc desc;
    UnknownBltFx fx;

    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = 0x21;                            // DDSD_CAPS | DDSD_BACKBUFFERCOUNT
    desc.caps[0] = 0x6218;                        // primary, flip, complex, 3D device, video memory
    desc.backBufferCount = backBuffers;
    long result = field_0x190->UnknownMethod6(&desc, &field_0x19c, 0);
    if (result == UNKNOWN_DDERR_OUTOFVIDEOMEMORY)
        goto failed;
    if (result)
        goto failed;
    memset(&caps, 0, sizeof(caps));
    caps.caps = 4;                                // DDSCAPS_BACKBUFFER
    if (field_0x19c->UnknownMethod12(&caps, &field_0x1a0))
        goto failed;
    field_0x78 = backBuffers + 1;
    field_0x19c->UnknownMethod0(&IID_IDirectDrawGammaControl, (void**)&field_0x1a4);
    if (field_0x1a4 && field_0xb74_bit3) {
        if (field_0x1a4->UnknownMethod4(1, g_UnknownGammaRamp570570))
            goto failed;
    }
    memset(&fx, 0, sizeof(fx));
    fx.size = sizeof(fx);
    fx.fillColor = 0;
    rect.left = 0;
    rect.top = 0;
    rect.right = field_0x10[field_0x0c].width;
    rect.bottom = field_0x10[field_0x0c].height;
    int i;
    for (i = 0; i <= backBuffers; i++) {
        if (field_0x1a0->UnknownMethod5(&rect, 0, 0, 0x1000400, &fx)) // DDBLT_COLORFILL | DDBLT_WAIT
            goto failed;
        if (field_0x19c->UnknownMethod11(0, 1))  // DDFLIP_WAIT
            goto failed;
    }
    field_0x80 = 0x7fffffff;
    field_0x84 = 0x7fffffff;
    field_0x7c = UnknownFunction4bfa80();
    if (field_0x19c->UnknownMethod11(0, 1))
        goto failed;
    for (i = 0; i < 10; i++) {
        if (field_0x19c->UnknownMethod11(0, 1))
            goto failed;
        UnknownRecordFrameTime();
    }
    return 1;
failed:
    UnknownFunction4c9c10();
    return 0;
}

// 0x004ca130: the primary surface (with `backBuffers` back buffers, if any)
// and a system-memory render surface of the current mode's size.
int UnknownDisplay::UnknownFunction4ca130(int backBuffers) {
    UnknownSurfaceCaps caps;
    UnknownSurfaceDesc desc;

    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = (backBuffers ? 0x20 : 0) | 1;    // DDSD_BACKBUFFERCOUNT, DDSD_CAPS
    desc.backBufferCount = backBuffers;
    desc.caps[0] = (backBuffers ? 0x18 : 0) | 0x4200; // flip | complex; primary, video memory
    long result = field_0x190->UnknownMethod6(&desc, &field_0x19c, 0);
    if (result == UNKNOWN_DDERR_OUTOFVIDEOMEMORY)
        goto failed;
    if (result)
        goto failed;
    if (!backBuffers) {
        field_0x1a0 = 0;
    } else {
        memset(&caps, 0, sizeof(caps));
        caps.caps = 4;                            // DDSCAPS_BACKBUFFER
        if (field_0x19c->UnknownMethod12(&caps, &field_0x1a0))
            goto failed;
    }
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = 7;                               // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH
    desc.width = field_0x10[field_0x0c].width;
    desc.height = field_0x10[field_0x0c].height;
    desc.caps[0] = 0x2800;                        // 3D device, system memory
    if (field_0x190->UnknownMethod6(&desc, &field_0x1a8, 0) == 0) {
        field_0x78 = 1;
        return 1;
    }
failed:
    return 0;
}

// 0x004ca270: flips (full screen) or blits the render surface to the
// primary, and records the frame time.
int UnknownDisplay::UnknownVirtualSlot3() {
    RECT rect;
    UnknownSurfaceInterface* source;
    unsigned int now;

    if (field_0x70_bit3 && !field_0x6c) {
        UnknownFunction4bfa80();
        // DDFLIP_WAIT when waiting for flips, else DDFLIP_DONOTWAIT
        field_0x70_bit2 = field_0x19c->UnknownMethod11(0, field_0xb74_bit0 ? 1 : 0x20) != 0;
        UnknownFunction4bfa80();
        if (field_0x70_bit2)
            return 1;
        UnknownRecordFrameTime();
        return 1;
    }
    if (field_0xa70 & 1) {                        // DDSCL_FULLSCREEN
        rect.left = 0;
        rect.top = 0;
        rect.right = field_0x10[field_0x0c].width;
        rect.bottom = field_0x10[field_0x0c].height;
        if (field_0x1a0 && !field_0x6c) {
            if (field_0x1a0->UnknownMethod7(0, 0, field_0x1a8, &rect, 0x10)) // DDBLTFAST_WAIT
                goto failed;
            field_0x70_bit2 = field_0x19c->UnknownMethod11(0, field_0xb74_bit0) != 0;
            UnknownRecordFrameTime();
            return 1;
        }
        source = field_0x1a8 ? field_0x1a8 : field_0x1a0;
        while (source->UnknownMethod13(1) == UNKNOWN_DDERR_WASSTILLDRAWING) // DDGBS_CANBLT
            ;
        if (field_0x190->UnknownMethod22(1, 0))   // DDWAITVB_BLOCKBEGIN
            goto failed;
        UnknownRecordFrameTime();
        if (field_0x19c->UnknownMethod5(0, source, &rect, 0x1000000, 0)) // DDBLT_WAIT
            goto failed;
        return 1;
    }
    source = field_0x1a8 ? field_0x1a8 : field_0x1a0;
    if (field_0x19c->UnknownMethod5(&g_UnknownGlobal56e26c->field_0x308, source, 0, 0x1000000, 0))
        goto failed;
    now = UnknownFunction4bfa80();
    field_0x80 = now - field_0x7c;
    field_0x7c = now;
    return 1;
failed:
    return 0;
}

// 0x004ca4a0
void UnknownDisplay::UnknownVirtualSlot4(int value) {
    if (value)
        field_0x19c->UnknownMethod11(0, 1);
    else
        field_0x70_bit2 = field_0x19c->UnknownMethod11(0, field_0xb74_bit0 ? 1 : 0x20) != 0;
    if (!field_0x70_bit2)
        UnknownRecordFrameTime();
}

// 0x004ca790: windowed surfaces: a primary clipped to the window and a
// `width` x `height` render surface (video memory when `windowed`).
int UnknownDisplay::UnknownFunction4ca790(int width, int height, int windowed) {
    UnknownSurfaceDesc desc;

    UnknownFunction4c9c10();
    if (field_0x1a8) {
        field_0x1a8->UnknownMethod2();
        field_0x1a8 = 0;
    }
    if (field_0x1ac) {
        field_0x1ac->UnknownMethod2();
        field_0x1ac = 0;
    }
    memset(&desc, 0, sizeof(desc));
    unsigned long memory = windowed ? 0x4000 : 0x800; // DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY
    desc.size = sizeof(desc);
    desc.flags = 1;                               // DDSD_CAPS
    desc.caps[0] = memory | 0x2200;               // primary, 3D device
    if (field_0x190->UnknownMethod6(&desc, &field_0x19c, 0))
        goto failed;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = 7;                               // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH
    desc.width = width;
    desc.height = height;
    desc.caps[0] = memory | 0x2000;               // 3D device
    if (field_0x190->UnknownMethod6(&desc, &field_0x1a0, 0))
        goto failed;
    if (field_0x190->UnknownMethod4(0, &field_0x1ac, 0))
        goto failed;
    if (field_0x1ac->UnknownMethod8(0, field_0x1b0))
        goto failed;
    if (field_0x19c->UnknownMethod28(field_0x1ac))
        goto failed;
    field_0x0c = -1;
    field_0x70_bit3 = 0;
    return 1;
failed:
    return 0;
}

// 0x004ca900: switches to display mode `mode` (retrying without the
// refresh rate) and creates its surfaces.
int UnknownDisplay::UnknownFunction4ca900(int mode, int windowed) {
    UnknownSurfaceDesc desc;

    if (mode < 0 || mode >= field_0x08)
        return 0;
    int refreshRate = field_0x10[mode].refreshRate;
    int bitDepth = field_0x10[mode].bitDepth;
    int height = field_0x10[mode].height;
    int width = field_0x10[mode].width;
    UnknownFunction4c9c10();
    if (field_0x190->UnknownMethod21(width, height, bitDepth, refreshRate, 0) &&
        field_0x190->UnknownMethod21(width, height, bitDepth, 0, 0))
        return 0;
    field_0x0c = mode;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (field_0x190->UnknownMethod12(&desc))
        return 0;
    if (!UnknownFunction4ca520(windowed))
        return 0;
    field_0x70_bit3 = windowed;
    return 1;
}

// 0x004ca9f0
int UnknownFunction4ca9f0(PCRenderTarget* target, TextureMap* texture) {
    UnknownSurfaceDesc desc;
    long pitch;

    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (target->field_0x48->UnknownMethod25(0, &desc, 0x811, 0)) // wait, read only, no sys lock
        return 0;
    char* bits = (char*)texture->UnknownVirtualSlot13(0, &pitch, 0x801);
    if (!bits) {
        target->field_0x48->UnknownMethod32(0);
        return 0;
    }
    int rows = texture->field_0x18;
    int rowSize = UnknownFunction511970(texture->field_0x20) * texture->field_0x14;
    char* source = (char*)desc.surface;
    while (rows--) {
        memcpy(bits, source, rowSize);
        source += desc.pitch;
        bits += pitch;
    }
    target->field_0x48->UnknownMethod32(0);
    texture->UnknownVirtualSlot14(0);
    return 1;
}

// Pre-transformed, lit vertex (the D3DTLVERTEX layout, FVF 0x1c4).
struct UnknownScreenVertex {
    float x;
    float y;
    float z;
    float rhw;
    unsigned long color;
    unsigned long specular;
    float u;
    float v;
};

// Viewport (the D3DVIEWPORT7 layout).
struct UnknownViewport {
    unsigned long x;
    unsigned long y;
    unsigned long width;
    unsigned long height;
    float minZ;
    float maxZ;
};

// 0x004caf70: one partial-upload test: clears `rendered` and `expected`,
// uploads `area` of `image` into `rendered`'s texture (slot 9) and blits
// the same area into `expected`, draws `rendered` full screen and reads
// the frame back into it, then compares. 0: different, 1: same, 2: a
// device call failed, 3: a lock failed.
int UnknownFunction4caf70(PCRenderTarget* target, UnknownRect* area, PCTextureMap* image,
                                 PCTextureMap* expected, PCTextureMap* rendered, UnknownDisplay* display,
                                 int present) {
    UnknownViewport viewport;
    UnknownRect clear;
    UnknownScreenVertex vertices[4];
    UnknownBltFx fx;

    memset(&fx, 0, sizeof(fx));
    fx.size = sizeof(fx);
    fx.fillColor = 0;
    rendered->field_0x70->UnknownMethod5(0, 0, 0, 0x400, &fx); // DDBLT_COLORFILL
    rendered->UnknownVirtualSlot9(0, 1);
    expected->field_0x70->UnknownMethod5(0, 0, 0, 0x400, &fx);
    rendered->field_0x70->UnknownMethod5(0, image->field_0x70, 0, 0x1000000, 0);
    rendered->UnknownVirtualSlot9(area, 1);
    expected->field_0x70->UnknownMethod5(area, image->field_0x70, area, 0x1000000, 0);
    rendered->field_0x70->UnknownMethod5(0, 0, 0, 0x400, &fx);
    target->field_0x48->UnknownMethod5(0, 0, 0, 0x400, &fx);

    memset(vertices, 0, sizeof(vertices));
    vertices[0].x = 0.0f;
    vertices[0].y = 0.0f;
    vertices[0].u = 0.0f;
    vertices[0].v = 0.0f;
    vertices[0].z = 0.1f;
    vertices[0].rhw = 1.0f;
    vertices[0].color = 0xffffffff;
    vertices[1].x = (float)rendered->field_0x14;
    vertices[1].y = 0.0f;
    vertices[1].z = 0.1f;
    vertices[1].rhw = 1.0f;
    vertices[1].color = 0xffffffff;
    vertices[1].u = 1.0f;
    vertices[1].v = 0.0f;
    vertices[2].x = (float)rendered->field_0x14;
    vertices[2].y = (float)rendered->field_0x14;
    vertices[2].z = 0.1f;
    vertices[2].rhw = 1.0f;
    vertices[2].color = 0xffffffff;
    vertices[2].u = 1.0f;
    vertices[2].v = 1.0f;
    vertices[3].x = 0.0f;
    vertices[3].y = (float)rendered->field_0x14;
    vertices[3].z = 0.1f;
    vertices[3].rhw = 1.0f;
    vertices[3].color = 0xffffffff;
    vertices[3].u = 0.0f;
    vertices[3].v = 1.0f;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = target->field_0x0c;
    viewport.height = target->field_0x10;
    viewport.minZ = 0.0f;
    viewport.maxZ = 1.0f;
    int failed = target->UnknownVirtualSlot14(&viewport) != 1;
    if (!failed) {
        failed = target->UnknownVirtualSlot1() != 1;
        clear.left = 0;
        clear.right = viewport.x + viewport.width;
        clear.top = 0;
        clear.bottom = viewport.y + viewport.height;
        failed |= target->device->UnknownMethod10(1, &clear, 2, 0, target->field_0x2c, 0) != 0; // Clear z
        g_UnknownGlobal56e26c->UnknownVirtualSlot7();
        rendered->UnknownVirtualSlot19();
        target->UnknownVirtualSlot8(0x1b, 0, 0);
        target->UnknownVirtualSlot8(0x29, 0, 0);
        target->UnknownVirtualSlot8(0xf, 0, 0);
        target->UnknownVirtualSlot8(0xe, 0, 0);
        target->UnknownVirtualSlot8(7, 0, 0);
        target->UnknownVirtualSlot7(0, 0x12, 1);
        target->UnknownVirtualSlot7(0, 0x10, 1);
        target->UnknownVirtualSlot7(0, 0x11, 1);
        target->UnknownVirtualSlot7(0, 1, 2);
        target->UnknownVirtualSlot7(0, 2, 2);
        target->UnknownVirtualSlot7(0, 4, 1);
        failed |= target->UnknownVirtualSlot16(6, 0x1c4, (int)vertices, 4, 0) != 1; // a fan of 4
        target->UnknownVirtualSlot2();
    }
    failed |= UnknownFunction4ca9f0(target, rendered) != 1;
    if (present)
        display->UnknownVirtualSlot4(1);
    if (failed)
        return 2;
    float difference = UnknownFunction4cb330(rendered, expected);
    if (difference < 0.0f)
        return 3;
    if (difference > 0.05f)
        return 0;
    return 1;
}
