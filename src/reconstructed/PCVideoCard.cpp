#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PCVideoCard.h"

#include "DebugAlloc.h"
#include "D3DConstants.h"
#include "PCRenderTarget.h"
#include "PCTextureMap.h"
#include "TextureMap.h"
#include "Tgafile.h"
#include "TrackGame.h"

// 0x004c94a0: DirectDrawEnumerateA callback; `context` points at the window.
static int __stdcall EnumDisplayCallback(UnknownGuid* guid, char* description, char* name,
    void* context) {
    if (g_UnknownDisplayCount68a764 < 4) {
        UnknownDisplay* display = new (__FILE__, 280) UnknownDisplay;
        display = display->InitializeDisplay(guid, description, name, *(void**)context);
        g_UnknownDisplays68a754[g_UnknownDisplayCount68a764] = display;
        if (display)
            g_UnknownDisplayCount68a764++;
    }
    return 1;
}

// 0x004c9550: DirectDrawEnumerateExA callback (the monitor is not used).
static int __stdcall EnumDisplayExCallback(UnknownGuid* guid, char* description, char* name,
    void* context, void* monitor) {
    if (g_UnknownDisplayCount68a764 < 4) {
        UnknownDisplay* display = new (__FILE__, 307) UnknownDisplay;
        display = display->InitializeDisplay(guid, description, name, *(void**)context);
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
int EnumerateDisplays(void* window) {
    HMODULE library = LoadLibraryA("ddraw.dll");
    if (!library)
        return 0;
    UnknownEnumerateEx enumerate = (UnknownEnumerateEx)GetProcAddress(library, "DirectDrawEnumerateExA");
    if (enumerate) {
        enumerate(EnumDisplayExCallback, &window,
                  DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
        FreeLibrary(library);
        return g_UnknownDisplayCount68a764;
    }
    DirectDrawEnumerateA(EnumDisplayCallback, &window);
    FreeLibrary(library);
    return g_UnknownDisplayCount68a764;
}

// 0x004c9660: EnumDisplayModes callback; appends the mode, growing the
// table by 20 entries.
static long __stdcall EnumDisplayModeCallback(UnknownSurfaceDesc* desc, void* context) {
    UnknownDisplay* display = (UnknownDisplay*)context;
    if (display->displayModeCount >= display->displayModeCapacity - 1) {
        display->displayModeCapacity += 20;
        display->displayModes = (UnknownDisplayMode*)DebugRealloc(
            display->displayModes, display->displayModeCapacity * sizeof(UnknownDisplayMode), __FILE__, 381);
        if (!display->displayModes) {
            display->displayModeCount = 0;
            display->displayModeCapacity = 0;
        }
    }
    if (display->displayModeCount < display->displayModeCapacity) {
        display->displayModes[display->displayModeCount].width = desc->width;
        display->displayModes[display->displayModeCount].height = desc->height;
        display->displayModes[display->displayModeCount].bitDepth = desc->pixelFormat.bitCount;
        display->displayModes[display->displayModeCount].refreshRate = desc->refreshRate;
        display->displayModes[display->displayModeCount].field_0x14 = 1;
        display->displayModes[display->displayModeCount].field_0x18 = 1;
        display->displayModes[display->displayModeCount].field_0x1c = 0;
        display->displayModes[display->displayModeCount].field_0x20 = 0;
        display->displayModes[display->displayModeCount].field_0x10 = 2;
        display->displayModeCount++;
    }
    return 1;
}

// 0x004c9760
UnknownDisplay::UnknownDisplay() {
    memset(&driverGuid, 0, sizeof(driverGuid));
    memset(driverGuidText, 0, sizeof(driverGuidText));
    directDraw = 0;
    direct3D = 0;
    primarySurface = 0;
    backBuffer = 0;
    gammaControl = 0;
    renderSurface = 0;
    clipper = 0;
    field_0x198 = 0;
    partialTextureUploadResult = 0;
    cooperativeLevel = DDSCL_NORMAL;
    waitForFlip = 0;
    keepAllDisplayModes = 0;
    isPowerVR = 0;
    useGammaRamp = 0;
    useFlipChain = 0;
    isAGP = -1;
}

// 0x004c9830
UnknownDisplay* UnknownDisplay::InitializeDisplay(UnknownGuid* guid, char* description, char* name,
    void* window) {
    UnknownSurfaceCaps caps;
    unsigned long total;
    unsigned long free;
    UnknownSurfaceDesc desc;
    UnknownSurfaceDesc mode;
    char key[256];

    windowHandle = window;
    isPowerVR = g_TrackGame->GetRegistryFlag("IsPowerVR", 0);
    if (DirectDrawCreateEx(guid, (void**)&directDraw, IID_IDirectDraw7, 0))
        goto failed;
    if (directDraw->GetDeviceIdentifier(&deviceIdentifier, 0))
        goto failed;
    memset(&driverCapsSize, 0, 0x17c);
    driverCapsSize = 0x17c;
    memset(&emulationCapsSize, 0, 0x17c);
    emulationCapsSize = 0x17c;
    if (directDraw->GetCaps(&driverCapsSize, &emulationCapsSize))
        goto failed;
    if (driverCaps2 & 0x100000)
        useGammaRamp = 0;
    memset(&caps, 0, sizeof(caps));
    caps.caps = DDSCAPS_VIDEOMEMORY;
    directDraw->GetAvailableVidMem(&caps, &total, &free);
    displayModeCount = 0;
    if (directDraw->EnumDisplayModes(0, 0, this, EnumDisplayModeCallback))
        goto failed;
    qsort(displayModes, displayModeCount, sizeof(UnknownDisplayMode), CompareDisplayModes);
    if (guid)
        driverGuid = *guid;
    sprintf(driverGuidText, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", driverGuid.data1,
            driverGuid.data2, driverGuid.data3, driverGuid.data4[0], driverGuid.data4[1],
            driverGuid.data4[2], driverGuid.data4[3], driverGuid.data4[4], driverGuid.data4[5],
            driverGuid.data4[6], driverGuid.data4[7]);
    sprintf(key, "DriverInfo\\%s\\WaitForFlip", driverGuidText);
    waitForFlip = g_TrackGame->GetRegistryFlag(key, 0);
    strncpy(driverDescription, description, sizeof(driverDescription));
    strncpy(driverName, name, sizeof(driverName));
    videoMemoryBudget = driverVideoMemory;
    driverDescription[sizeof(driverDescription) - 1] = 0;
    driverName[sizeof(driverName) - 1] = 0;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (directDraw->GetDisplayMode(&desc))
        goto failed;
    // The desktop's own surface comes off the video memory.
    videoMemoryBudget += desc.height * desc.width * desc.pixelFormat.bitCount >> 3;
    memset(&mode, 0, sizeof(mode));
    mode.size = sizeof(mode);
    if (directDraw->GetDisplayMode(&mode))
        goto failed;
    if (directDraw->QueryInterface(&IID_IDirect3D7, (void**)&direct3D))
        goto failed;
    return this;
failed:
    delete this;
    return 0;
}

// 0x004c9b50: re-reads the driver and emulation caps.
void UnknownDisplay::RefreshDriverCaps() {
    memset(&driverCapsSize, 0, 0x17c);
    driverCapsSize = 0x17c;
    memset(&emulationCapsSize, 0, 0x17c);
    emulationCapsSize = 0x17c;
    directDraw->GetCaps(&driverCapsSize, &emulationCapsSize);
}

// 0x004c9ba0
void UnknownDisplay::UnknownVirtualSlot1() {
    ReleasePrimarySurface();
    if (renderSurface != backBuffer && renderSurface) {
        renderSurface->Release();
        renderSurface = 0;
    }
    backBuffer = 0;
    if (direct3D) {
        direct3D->Release();
        direct3D = 0;
    }
    if (directDraw) {
        directDraw->Release();
        directDraw = 0;
    }
    VideoCard::UnknownVirtualSlot1();
}

// 0x004c9c10: waits for the primary's blits, then releases it and the
// gamma control.
void UnknownDisplay::ReleasePrimarySurface() {
    if (gammaControl) {
        gammaControl->Release();
        gammaControl = 0;
    }
    if (primarySurface) {
        while (primarySurface->GetBltStatus(DDGBS_ISBLTDONE) == ((long)DDERR_WASSTILLDRAWING))
            Sleep(1);
        primarySurface->Release();
        primarySurface = 0;
    }
    renderSurface = 0;
    backBuffer = 0;
}

// 0x004c9c90
int UnknownDisplay::SetFullscreenCooperativeLevel() {
    SetWindowLongA((HWND)windowHandle, GWL_STYLE,
                   GetWindowLongA((HWND)windowHandle, GWL_STYLE) & (WS_POPUP | WS_SYSMENU));
    ShowWindow((HWND)windowHandle, SW_SHOW);
    if (g_TrackGame->imeLibrary)
        UnknownFunction52ff00();
    if (directDraw->SetCooperativeLevel(windowHandle,
                                      DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE |
                                          DDSCL_FPUSETUP))
        return 0;
    cooperativeLevel = DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FPUSETUP;
    if (g_TrackGame->imeLibrary)
        UnknownFunction52ff20();
    return 1;
}

// 0x004c9d20: a window of the client size `width` x `height` at (x, y),
// kept inside the work area; hidden when the size is 0 x 0.
int UnknownDisplay::SetWindowedCooperativeLevel(int x, int y, int width, int height) {
    RECT rect;
    RECT workArea;

    if (g_TrackGame->imeLibrary)
        UnknownFunction52ff00();
    if (directDraw->SetCooperativeLevel(windowHandle, DDSCL_NORMAL | DDSCL_FPUSETUP))
        return 0;
    cooperativeLevel = DDSCL_NORMAL | DDSCL_FPUSETUP;
    SetWindowLongA((HWND)windowHandle, GWL_STYLE,
                   GetWindowLongA((HWND)windowHandle, GWL_STYLE) & 0x7f39ffff | 0xc60000);
    SetRect(&rect, x, y, width, height);
    AdjustWindowRectEx(&rect, GetWindowLongA((HWND)windowHandle, GWL_STYLE), GetMenu((HWND)windowHandle) != 0,
                       GetWindowLongA((HWND)windowHandle, GWL_EXSTYLE));
    OffsetRect(&rect, rect.left < 0 ? -rect.left : 0, rect.top < 0 ? -rect.top : 0);
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &workArea, 0);
    OffsetRect(&rect, rect.left < workArea.left ? workArea.left - rect.left : 0,
               rect.top < workArea.top ? workArea.top - rect.top : 0);
    SetWindowPos((HWND)windowHandle, HWND_TOP, rect.left, rect.top, rect.right - rect.left,
                 rect.bottom - rect.top, SWP_NOZORDER);
    if (!width && !height)
        ShowWindow((HWND)windowHandle, SW_HIDE);
    if (g_TrackGame->imeLibrary)
        UnknownFunction52ff20();
    return 1;
}

// 0x004c9ec0
int UnknownDisplay::UnknownVirtualSlot2(int width, int height, int bitDepth, int a, int b, int windowed,
                                        int c) {
    if (cooperativeLevel & DDSCL_NORMAL)
        return CreateWindowedSurfaces(width, height, windowed);
    int mode = FindDisplayMode(width, height, bitDepth, a, b);
    if (mode == -1)
        return 0;
    return SetFullscreenDisplayMode(mode, windowed);
}

// 0x004c9f30: creates the full-screen flipping chain with `backBuffers`
// back buffers, clears and flips every buffer, then times ten flips.
int UnknownDisplay::CreateFlipChain(int backBuffers) {
    UnknownSurfaceCaps caps;
    RECT rect;
    UnknownSurfaceDesc desc;
    UnknownBltFx fx;

    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    desc.caps[0] = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX |
                   DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
    desc.backBufferCount = backBuffers;
    long result = directDraw->CreateSurface(&desc, &primarySurface, 0);
    if (result == ((long)DDERR_OUTOFVIDEOMEMORY))
        goto failed;
    if (result)
        goto failed;
    memset(&caps, 0, sizeof(caps));
    caps.caps = DDSCAPS_BACKBUFFER;
    if (primarySurface->GetAttachedSurface(&caps, &backBuffer))
        goto failed;
    frameBufferCount = backBuffers + 1;
    primarySurface->QueryInterface(&IID_IDirectDrawGammaControl, (void**)&gammaControl);
    if (gammaControl && useGammaRamp) {
        if (gammaControl->SetGammaRamp(DDSGR_CALIBRATE, g_UnknownGammaRamp570570))
            goto failed;
    }
    memset(&fx, 0, sizeof(fx));
    fx.size = sizeof(fx);
    fx.fillColor = 0;
    rect.left = 0;
    rect.top = 0;
    rect.right = displayModes[currentDisplayMode].width;
    rect.bottom = displayModes[currentDisplayMode].height;
    int i;
    for (i = 0; i <= backBuffers; i++) {
        if (backBuffer->Blt(&rect, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &fx))
            goto failed;
        if (primarySurface->Flip(0, DDFLIP_WAIT))
            goto failed;
    }
    lastFrameTime = 0x7fffffff;
    shortestFrameTime = 0x7fffffff;
    lastPresentTime = UnknownFunction4bfa80();
    if (primarySurface->Flip(0, DDFLIP_WAIT))
        goto failed;
    for (i = 0; i < 10; i++) {
        if (primarySurface->Flip(0, DDFLIP_WAIT))
            goto failed;
        RecordFrameTime();
    }
    return 1;
failed:
    ReleasePrimarySurface();
    return 0;
}

// 0x004ca130: the primary surface (with `backBuffers` back buffers, if any)
// and a system-memory render surface of the current mode's size.
int UnknownDisplay::CreateSystemRenderSurface(int backBuffers) {
    UnknownSurfaceCaps caps;
    UnknownSurfaceDesc desc;

    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = (backBuffers ? DDSD_BACKBUFFERCOUNT : 0) | DDSD_CAPS;
    desc.backBufferCount = backBuffers;
    desc.caps[0] = (backBuffers ? DDSCAPS_FLIP | DDSCAPS_COMPLEX : 0) |
                   DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
    long result = directDraw->CreateSurface(&desc, &primarySurface, 0);
    if (result == ((long)DDERR_OUTOFVIDEOMEMORY))
        goto failed;
    if (result)
        goto failed;
    if (!backBuffers) {
        backBuffer = 0;
    } else {
        memset(&caps, 0, sizeof(caps));
        caps.caps = DDSCAPS_BACKBUFFER;
        if (primarySurface->GetAttachedSurface(&caps, &backBuffer))
            goto failed;
    }
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.width = displayModes[currentDisplayMode].width;
    desc.height = displayModes[currentDisplayMode].height;
    desc.caps[0] = DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;
    if (directDraw->CreateSurface(&desc, &renderSurface, 0) == 0) {
        frameBufferCount = 1;
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

    if (useFlipChain && !freezeFrameIndex) {
        UnknownFunction4bfa80();
        lastFlipFailed = primarySurface->Flip(0, waitForFlip ? DDFLIP_WAIT : DDFLIP_DONOTWAIT) != 0;
        UnknownFunction4bfa80();
        if (lastFlipFailed)
            return 1;
        RecordFrameTime();
        return 1;
    }
    if (cooperativeLevel & DDSCL_FULLSCREEN) {
        rect.left = 0;
        rect.top = 0;
        rect.right = displayModes[currentDisplayMode].width;
        rect.bottom = displayModes[currentDisplayMode].height;
        if (backBuffer && !freezeFrameIndex) {
            if (backBuffer->BltFast(0, 0, renderSurface, &rect, DDBLTFAST_WAIT))
                goto failed;
            lastFlipFailed = primarySurface->Flip(0, waitForFlip) != 0;
            RecordFrameTime();
            return 1;
        }
        source = renderSurface ? renderSurface : backBuffer;
        while (source->GetBltStatus(DDGBS_CANBLT) == ((long)DDERR_WASSTILLDRAWING))
            ;
        if (directDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0))
            goto failed;
        RecordFrameTime();
        if (primarySurface->Blt(0, source, &rect, DDBLT_WAIT, 0))
            goto failed;
        return 1;
    }
    source = renderSurface ? renderSurface : backBuffer;
    if (primarySurface->Blt(&g_TrackGame->windowRect, source, 0, DDBLT_WAIT, 0))
        goto failed;
    now = UnknownFunction4bfa80();
    lastFrameTime = now - lastPresentTime;
    lastPresentTime = now;
    return 1;
failed:
    return 0;
}

// 0x004ca4a0
void UnknownDisplay::UnknownVirtualSlot4(int value) {
    if (value)
        primarySurface->Flip(0, DDFLIP_WAIT);
    else
        lastFlipFailed = primarySurface->Flip(0, waitForFlip ? DDFLIP_WAIT : DDFLIP_DONOTWAIT) != 0;
    if (!lastFlipFailed)
        RecordFrameTime();
}

// 0x004ca790: windowed surfaces: a primary clipped to the window and a
// `width` x `height` render surface (video memory when `windowed`).
int UnknownDisplay::CreateWindowedSurfaces(int width, int height, int windowed) {
    UnknownSurfaceDesc desc;

    ReleasePrimarySurface();
    if (renderSurface) {
        renderSurface->Release();
        renderSurface = 0;
    }
    if (clipper) {
        clipper->Release();
        clipper = 0;
    }
    memset(&desc, 0, sizeof(desc));
    unsigned long memory = windowed ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY;
    desc.size = sizeof(desc);
    desc.flags = DDSD_CAPS;
    desc.caps[0] = memory | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;               // primary, 3D device
    if (directDraw->CreateSurface(&desc, &primarySurface, 0))
        goto failed;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.width = width;
    desc.height = height;
    desc.caps[0] = memory | DDSCAPS_3DDEVICE;               // 3D device
    if (directDraw->CreateSurface(&desc, &backBuffer, 0))
        goto failed;
    if (directDraw->CreateClipper(0, &clipper, 0))
        goto failed;
    if (clipper->SetHWnd(0, windowHandle))
        goto failed;
    if (primarySurface->SetClipper(clipper))
        goto failed;
    currentDisplayMode = -1;
    useFlipChain = 0;
    return 1;
failed:
    return 0;
}

// 0x004ca900: switches to display mode `mode` (retrying without the
// refresh rate) and creates its surfaces.
int UnknownDisplay::SetFullscreenDisplayMode(int mode, int windowed) {
    UnknownSurfaceDesc desc;

    if (mode < 0 || mode >= displayModeCount)
        return 0;
    int refreshRate = displayModes[mode].refreshRate;
    int bitDepth = displayModes[mode].bitDepth;
    int height = displayModes[mode].height;
    int width = displayModes[mode].width;
    ReleasePrimarySurface();
    if (directDraw->SetDisplayMode(width, height, bitDepth, refreshRate, 0) &&
        directDraw->SetDisplayMode(width, height, bitDepth, 0, 0))
        return 0;
    currentDisplayMode = mode;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (directDraw->GetDisplayMode(&desc))
        return 0;
    if (!CreateModeSurfaces(windowed))
        return 0;
    useFlipChain = windowed;
    return 1;
}

// 0x004ca9f0
int CopyRenderSurfaceToTexture(PCRenderTarget* target, TextureMap* texture) {
    UnknownSurfaceDesc desc;
    long pitch;

    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (target->renderSurface->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_READONLY | DDLOCK_NOSYSLOCK, 0))
        return 0;
    char* bits = (char*)texture->UnknownVirtualSlot13(0, &pitch, DDLOCK_WAIT | DDLOCK_NOSYSLOCK);
    if (!bits) {
        target->renderSurface->Unlock(0);
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
    target->renderSurface->Unlock(0);
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
int TestPartialTextureUpload(PCRenderTarget* target, UnknownRect* area, PCTextureMap* image,
    PCTextureMap* expected, PCTextureMap* rendered, UnknownDisplay* display,
    int present) {
    UnknownViewport viewport;
    UnknownRect clear;
    UnknownScreenVertex vertices[4];
    UnknownBltFx fx;

    memset(&fx, 0, sizeof(fx));
    fx.size = sizeof(fx);
    fx.fillColor = 0;
    rendered->systemSurface->Blt(0, 0, 0, DDBLT_COLORFILL, &fx);
    rendered->UnknownVirtualSlot9(0, 1);
    expected->systemSurface->Blt(0, 0, 0, DDBLT_COLORFILL, &fx);
    rendered->systemSurface->Blt(0, image->systemSurface, 0, DDBLT_WAIT, 0);
    rendered->UnknownVirtualSlot9(area, 1);
    expected->systemSurface->Blt(area, image->systemSurface, area, DDBLT_WAIT, 0);
    rendered->systemSurface->Blt(0, 0, 0, DDBLT_COLORFILL, &fx);
    target->renderSurface->Blt(0, 0, 0, DDBLT_COLORFILL, &fx);

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
        failed |= target->device->Clear(1, &clear, D3DCLEAR_ZBUFFER, 0, target->field_0x2c, 0) != 0; // Clear z
        g_TrackGame->UnknownVirtualSlot7();
        rendered->UnknownVirtualSlot19();
        target->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
        target->UnknownVirtualSlot8(D3DRENDERSTATE_COLORKEYENABLE, 0, 0);
        target->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHATESTENABLE, 0, 0);
        target->UnknownVirtualSlot8(D3DRENDERSTATE_ZWRITEENABLE, 0, 0);
        target->UnknownVirtualSlot8(D3DRENDERSTATE_ZENABLE, 0, 0);
        target->UnknownVirtualSlot7(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
        target->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        target->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        target->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        target->UnknownVirtualSlot7(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        target->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        failed |= target->UnknownVirtualSlot16(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX,
                                              (int)vertices, 4, 0) != 1;
        target->UnknownVirtualSlot2();
    }
    failed |= CopyRenderSurfaceToTexture(target, rendered) != 1;
    if (present)
        display->UnknownVirtualSlot4(1);
    if (failed)
        return 2;
    float difference = MeanTextureChannelDifference(rendered, expected);
    if (difference < 0.0f)
        return 3;
    if (difference > 0.05f)
        return 0;
    return 1;
}
