#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PCGame.h"

#include "Camera.h"
#include "D3DConstants.h"
#include "DebugAlloc.h"
#include "ControlInterface.h"
#include "TrackGame.h"

// USER32, KERNEL32, OLE32 and WINMM imports.
extern "C" __declspec(dllimport) long __stdcall CoInitialize(void* reserved);
extern "C" __declspec(dllimport) void __stdcall CoUninitialize();
extern "C" __declspec(dllimport) int __stdcall GetVersionExA(UnknownOSVersionInfo* info);
extern "C" __declspec(dllimport) unsigned int __stdcall timeBeginPeriod(unsigned int period);
extern "C" __declspec(dllimport) unsigned int __stdcall timeEndPeriod(unsigned int period);
extern "C" __declspec(dllimport) unsigned long __stdcall timeGetTime();
extern "C" __declspec(dllimport) unsigned long __stdcall GetCurrentDirectoryA(unsigned long size,
                                                                            char* buffer);
extern "C" __declspec(dllimport) void* __stdcall LoadLibraryA(const char* name);
extern "C" __declspec(dllimport) int __stdcall ShowCursor(int show);
extern "C" __declspec(dllimport) void* __stdcall GetActiveWindow();
extern "C" __declspec(dllimport) int __stdcall GetWindowRect(void* window, UnknownRect* rect);
extern "C" __declspec(dllimport) int __stdcall LoadStringA(void* instance, unsigned int id,
                                                          char* buffer, int size);
extern "C" __declspec(dllimport) int __stdcall MessageBoxA(void* window, const char* text,
                                                          const char* caption, unsigned int type);

extern "C" __declspec(dllimport) long __stdcall RegOpenKeyExA(void* key, const char* subKey,
                                                             unsigned long options,
                                                             unsigned long access, void** result);
extern "C" __declspec(dllimport) long __stdcall RegQueryValueExA(void* key, const char* name,
                                                                unsigned long* reserved,
                                                                unsigned long* type,
                                                                unsigned char* data,
                                                                unsigned long* size);
extern "C" __declspec(dllimport) long __stdcall RegCloseKey(void* key);
extern "C" __declspec(dllimport) long __stdcall RegDeleteKeyA(void* key, const char* subKey);
extern "C" __declspec(dllimport) long __stdcall RegEnumKeyA(void* key, unsigned long index,
                                                           char* name, unsigned long size);
extern "C" __declspec(dllimport) long __stdcall RegQueryInfoKeyA(
    void* key, char* className, unsigned long* classSize, unsigned long* reserved,
    unsigned long* subKeys, unsigned long* maxSubKeyLength, unsigned long* maxClassLength,
    unsigned long* values, unsigned long* maxValueNameLength, unsigned long* maxValueLength,
    unsigned long* securityDescriptorSize, void* lastWriteTime);
extern "C" __declspec(dllimport) long __stdcall RegCreateKeyExA(void* key, const char* subKey,
                                                               unsigned long reserved,
                                                               const char* className,
                                                               unsigned long options,
                                                               unsigned long access,
                                                               void* security, void** result,
                                                               unsigned long* disposition);
extern "C" __declspec(dllimport) long __stdcall RegSetValueExA(void* key, const char* name,
                                                              unsigned long reserved,
                                                              unsigned long type,
                                                              const unsigned char* data,
                                                              unsigned long size);

#define UNKNOWN_HKEY_LOCAL_MACHINE ((void*)0x80000002)
#define UNKNOWN_KEY_READ 0x20019
#define UNKNOWN_KEY_ALL_ACCESS 0xf003f
#define UNKNOWN_REG_SZ 1
#define UNKNOWN_REG_BINARY 3
#define UNKNOWN_REG_DWORD 4

// IMM32, called through the linker's import thunks.
extern "C" void* __stdcall ImmCreateContext();
extern "C" void* __stdcall ImmAssociateContext(void* window, void* context);

// Direct3D device GUIDs (their values are the DirectX IIDs of the same
// names); 0x00556040 is the "Blade" renderer's, which is not a DirectX one.
extern "C" const UnknownGuid IID_IDirect3DRampDevice;   // 0x00556190
extern "C" const UnknownGuid IID_IDirect3DRGBDevice;    // 0x005561a0
extern "C" const UnknownGuid IID_IDirect3DHALDevice;    // 0x005561b0
extern "C" const UnknownGuid IID_IDirect3DMMXDevice;    // 0x005561c0
extern "C" const UnknownGuid IID_IDirect3DRefDevice;    // 0x005561d0
extern "C" const UnknownGuid IID_IDirect3DNullDevice;   // 0x005561e0
extern const UnknownGuid g_UnknownBladeDevice556040;

// Display set-up outside PCGame (cdecl; names provisional).
int UnknownFunction4c9600(void* window);                // 0x004c9600: enumerates the displays
// 0x004ccd60: chooses the display (and whether the Blade renderer is used).
UnknownDisplay* UnknownFunction4ccd60(int flag, int useLast, int* blade);
int UnknownFunction4cd610(int useLast);                 // 0x004cd610: chooses the joystick
// 0x005119c0: fills a pixel format for texture format `format`.
void UnknownFunction5119c0(int format, void* pixelFormat);

// 0x00689940: a FILTERKEYS-sized structure (24 bytes) cleared by the
// constructor.
struct UnknownFilterKeys {
    unsigned int size;
    unsigned int flags;
    unsigned int waitMSec;
    unsigned int delayMSec;
    unsigned int repeatMSec;
    unsigned int bounceMSec;
};
UnknownFilterKeys g_UnknownFilterKeys689940;

// 0x004bfa80: the current time in milliseconds, read at 1 ms timer
// resolution. It sits just before PCGame's constructor; its TU is inferred
// from that position only.
unsigned int UnknownFunction4bfa80() {
    timeBeginPeriod(1);
    unsigned int time = timeGetTime();
    timeEndPeriod(1);
    return time;
}

// 0x004bfaa0
PCGame::PCGame() {
    field_0x318 = 0;
    field_0x31c = 0;
    CoInitialize(0);
    field_0x548_bit0 = 0;
    field_0x424.size = sizeof(field_0x424);
    GetVersionExA(&field_0x424);
    timeBeginPeriod(1);
    timeEndPeriod(1);
    strcpy(companyName, "Rainbow Studios");
    strcpy(field_0x3a0, "Rainbow Demo");
    strcpy(field_0x4b8, "SOFTWARE\\Rainbow Studios\\Demo");
    GetCurrentDirectoryA(0x104, field_0x1cc);
    field_0x420 = 0;
    field_0x538 = LoadLibraryA("IMM32.DLL");
    field_0x53c = ImmCreateContext();
    if (field_0x53c) {
        field_0x540 = ImmAssociateContext(field_0x31c, field_0x53c);
    } else {
        field_0x540 = 0;
        field_0x53c = 0;
        field_0x538 = 0;
    }
    memset(&g_UnknownFilterKeys689940, 0, sizeof(g_UnknownFilterKeys689940));
    g_UnknownFilterKeys689940.size = sizeof(g_UnknownFilterKeys689940);
}

// 0x004bfc20
PCGame::~PCGame() {
    CoUninitialize();
}

// 0x004bfc40
int PCGame::UnknownVirtualSlot2() {
    return Game::UnknownVirtualSlot2();
}

// 0x004bfc50: PCGame's start-up. Chooses the Direct3D device from the
// "Renderer" setting, enumerates and (re)profiles the displays, picks the
// display and its feature flags and the joystick, then runs Game's
// initialiser.
int PCGame::StartUp(char* message) {
    char name[256];
    char renderer[260];
    char text[512];
    unsigned long size;
    if (!field_0x420)
        field_0x420 = field_0x318;
    size = sizeof(renderer);
    GetRegistryString("Renderer", "HAL", renderer, &size);
    field_0x2d0 = 0;
    if (!_stricmp(renderer, "RGB")) {
        deviceGuid = IID_IDirect3DRGBDevice;
    } else if (!_stricmp(renderer, "MMX")) {
        deviceGuid = IID_IDirect3DMMXDevice;
    } else if (!_stricmp(renderer, "Ramp")) {
        deviceGuid = IID_IDirect3DRampDevice;
    } else if (!_stricmp(renderer, "Null")) {
        deviceGuid = IID_IDirect3DNullDevice;
    } else if (!_stricmp(renderer, "Ref")) {
        deviceGuid = IID_IDirect3DRefDevice;
    } else if (!_stricmp(renderer, "Blade")) {
        deviceGuid = g_UnknownBladeDevice556040;
        field_0x2d0 = 1;
    } else {
        deviceGuid = IID_IDirect3DHALDevice;
    }
    field_0x2d4_bit1 = GetRegistryFlag("FullScreen", field_0x2d4_bit1);
    if (!UnknownVirtualSlot37()) {
        if (message)
            LoadStringA(field_0x420, 0x13d6, message, 0x100);
        return 0;
    }
    if (!UnknownFunction4c9600(field_0x31c))
        return 0;
    for (int i = 0; i < g_UnknownDisplayCount68a764; i++)
        g_UnknownDisplays68a754[i]->UnknownFunction4c9d20(0, 0, 0, 0);
    field_0x548_bit0 = IsAnyProfileStale();
    if (field_0x548_bit0) {
        if (LoadStringA(field_0x420, 0x13d8, text, sizeof(text))) {
            ShowCursor(1);
            if (MessageBoxA(field_0x31c, text, field_0x3a0, 0x1041) == 2)
                return 0;
            ShowCursor(0);
        }
        DeleteDisplayProfiles();
        SetRegistryFlag("UseLastVideoCard", 0);
        ProfileEveryDisplay();
    }
    ProfileDisplays();
    display = g_UnknownDisplays68a754[0];
    int flag = field_0x2d5_bit3 && !field_0x544;
    int useLast = GetRegistryFlag("UseLastVideoCard", 0);
    display = UnknownFunction4ccd60(flag, useLast, &field_0x2d0);
    if (!display) {
        *message = 0;
        return 0;
    }
    if (field_0x2d0)
        deviceGuid = g_UnknownBladeDevice556040;
    sprintf(name, "DriverInfo\\%s\\AllowDither", display->field_0x4bc);
    field_0x2d4_bit3 = GetRegistryFlag(name, field_0x2d4_bit3);
    sprintf(name, "DriverInfo\\%s\\AllowMipMapping", display->field_0x4bc);
    field_0x2d4_bit4 = GetRegistryFlag(name, field_0x2d4_bit4);
    sprintf(name, "DriverInfo\\%s\\AllowBiLinear", display->field_0x4bc);
    field_0x2d4_bit5 = GetRegistryFlag(name, field_0x2d4_bit5);
    sprintf(name, "DriverInfo\\%s\\AllowSortIndependantAntiAliasing", display->field_0x4bc);
    field_0x2d4_bit6 = GetRegistryFlag(name, field_0x2d4_bit6);
    sprintf(name, "DriverInfo\\%s\\AllowTriLinear", display->field_0x4bc);
    field_0x2d4_bit7 = GetRegistryFlag(name, field_0x2d4_bit7);
    int joystick = UnknownFunction4cd610(GetRegistryFlag("UseLastController", 0));
    if (joystick >= 0) {
        controlInterface->activeJoystickIndex = joystick;
        controlInterface->activeJoystick = controlInterface->joysticks[joystick];
    } else if (joystick == -2) {
        *message = 0;
        return 0;
    }
    return Game::UnknownFunction467b70(message);
}

// 0x004c0230
int PCGame::UnknownVirtualSlot15() {
    int result = Game::UnknownVirtualSlot15();
    ShowCursor(1);
    return result;
}

// 0x004c0250: while the window is active, hides the cursor (full screen),
// restores lost surfaces and runs the root object's slot 18.
int PCGame::UnknownVirtualSlot5() {
    if (display && GetActiveWindow() == field_0x31c) {
        if (field_0x2d4_bit1)
            while (ShowCursor(0) >= 0)
                ;
        if ((display && display->field_0x19c &&
             display->field_0x19c->IsLost() &&
             display->field_0x19c->Restore()) ||
            (field_0x2d5_bit3 && renderTarget && PCTarget()->zbuffer &&
             PCTarget()->zbuffer->IsLost() &&
             PCTarget()->zbuffer->Restore()))
            return 0;
        if (rootObject)
            rootObject->UnknownVirtualSlot18();
    }
    return 1;
}

// 0x004c0310
int PCGame::UnknownVirtualSlot6() {
    if (rootObject)
        rootObject->UnknownVirtualSlot17();
    if (field_0x2d4_bit1)
        ShowCursor(1);
    return 1;
}

// 0x004c0340
int PCGame::UnknownVirtualSlot35(int value) {
    if (field_0x2d5_bit0)
        return 1;
    return rootObject->UnknownVirtualSlot20(value);
}

// 0x004c0370
int PCGame::UnknownVirtualSlot36(int value) {
    if (field_0x2d5_bit0)
        return 1;
    return rootObject->UnknownVirtualSlot21(value);
}

// 0x004c03a0: control 0xb7 released (kind 0) triggers the +0x10 object.
int PCGame::UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (Game::UnknownVirtualSlot13(event, entry))
        return 1;
    if (event->kind == 0 && event->control == 0xb7) {
        if (renderTarget && !renderTarget->field_0x08)
            PCTarget()->SaveScreenshot();
        return 1;
    }
    return 0;
}

// 0x004c0400: with the debug bit, control 0x41 toggles the render target's
// fill mode between solid and wireframe.
int PCGame::UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (Game::UnknownVirtualSlot14(event, entry))
        return 1;
    if (field_0x2d4_bit2 && UnknownFunction43caa0(0x41, 0, event, 3)) {
        PCTarget()->fillMode = PCTarget()->fillMode == D3DFILL_SOLID ? D3DFILL_WIREFRAME : D3DFILL_SOLID;
        return 1;
    }
    return 0;
}

// 0x004c04a0: switches to display mode `mode` (full screen) or back to
// 640x480 (windowed), recreating the render target through slot 33, then
// reattaches the camera.
int PCGame::UnknownVirtualSlot19(int mode) {
    Camera* camera = 0;
    if (renderTarget)
        camera = renderTarget->field_0x08;
    if (field_0x2d4_bit1) {
        if (mode != display->field_0x0c) {
            if (renderTarget) {
                delete renderTarget;
                renderTarget = 0;
            }
            display->UnknownFunction4ca900(mode, field_0x2d0 == 0);
            if (!UnknownVirtualSlot33())
                return 0;
        }
    } else if (field_0x308.bottom - field_0x308.top != 480) {
        if (renderTarget) {
            delete renderTarget;
            renderTarget = 0;
        }
        display->UnknownFunction4ca790(640, 480, field_0x2d0 == 0);
        if (!UnknownVirtualSlot33())
            return 0;
    }
    if (renderTarget && camera) {
        camera->field_0x18 = renderTarget;
        renderTarget->UnknownFunction4e8cf0(camera);
        renderTarget->UnknownVirtualSlot14(camera->field_0x1a0);
    }
    return 1;
}

// 0x004c0470
void PCGame::SetWindowRect(const UnknownRect* rect) {
    field_0x308 = *rect;
}

// 0x004c05a0: in full screen, drops duplicate modes ("HighestRefreshOnly"
// keeps the last of each), modes other than 16-bit, and modes the video
// memory cannot hold with "MinimumTextureMB" left over.
int PCGame::UnknownVirtualSlot34(UnknownDisplay* display) {
    int reserve = g_TrackGame->GetRegistryInt("MinimumTextureMB", 2) << 20;
    if (field_0x2d4_bit1 && !display->field_0xb74_bit1) {
        int i;
        if (GetRegistryFlag("HighestRefreshOnly", 1)) {
            for (i = 0; i < display->field_0x08; i++) {
                if (i > 0 && display->field_0x10[i].width == display->field_0x10[i - 1].width &&
                    display->field_0x10[i].height == display->field_0x10[i - 1].height &&
                    display->field_0x10[i].bitDepth == display->field_0x10[i - 1].bitDepth &&
                    display->field_0x10[i].field_0x10 == display->field_0x10[i - 1].field_0x10) {
                    display->field_0x10[i - 1].field_0x14 = 0;
                    display->field_0x10[i - 1].field_0x18 = 0;
                }
            }
        } else {
            for (i = 0; i < display->field_0x08; i++) {
                if (i > 0 && display->field_0x10[i].width == display->field_0x10[i - 1].width &&
                    display->field_0x10[i].height == display->field_0x10[i - 1].height &&
                    display->field_0x10[i].bitDepth == display->field_0x10[i - 1].bitDepth &&
                    display->field_0x10[i].field_0x10 == display->field_0x10[i - 1].field_0x10) {
                    display->field_0x10[i].field_0x14 = 0;
                    display->field_0x10[i].field_0x18 = 0;
                }
            }
        }
        for (i = 0; i < display->field_0x08; i++) {
            if (display->field_0x10[i].bitDepth != 16) {
                display->field_0x10[i].field_0x14 = 0;
                display->field_0x10[i].field_0x18 = 0;
            }
            if (display->field_0x10[i].field_0x14 &&
                display->field_0x54 - display->field_0x10[i].bitDepth / 8 *
                    display->field_0x10[i].height * display->field_0x10[i].width * 3 < reserve &&
                display->field_0x10[i].width > 640)
                display->field_0x10[i].field_0x14 = 0;
            if (display->field_0x10[i].field_0x18 &&
                display->field_0x54 < display->field_0x10[i].bitDepth / 8 *
                    display->field_0x10[i].height * display->field_0x10[i].width * 2)
                display->field_0x10[i].field_0x18 = 0;
        }
    }
    return 1;
}

// 0x004c07d0: sets the render target's default render states and the
// device's texture stage 0 states, choosing dithering, antialiasing and the
// filters from the "DriverInfo\<driver>\Allow..." bits (+0x2d4) and the
// device capabilities.
int PCGame::UnknownVirtualSlot7() {
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_DITHERENABLE,
                                    field_0x2d4_bit3 && (PCTarget()->triRasterCaps & D3DPRASTERCAPS_DITHER), 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_EDGEANTIALIAS, 0, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ANTIALIAS, 0, 1);
    if (field_0x2d4_bit6 && (PCTarget()->triRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT))
        renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ANTIALIAS, 2, 1); // D3DANTIALIAS_SORTINDEPENDENT
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_FOGENABLE, 0, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_RANGEFOGENABLE, 0, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_FILLMODE, PCTarget()->fillMode, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ZENABLE, field_0x2d5_bit3, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ZWRITEENABLE, field_0x2d5_bit3, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 1);
    renderTarget->UnknownVirtualSlot10(7, 0);
    if (field_0x2d0) {
        renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT, 1);
        renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0, 1);
    } else {
        renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD, 1);
        renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 1, 1);
    }
    PCTarget()->device->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_SPECULARENABLE, 0, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ZVISIBLE, 0, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_LASTPIXEL, 0, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_STIPPLEDALPHA, 0, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHATESTENABLE, 1, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHAREF, 0, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER, 1);
    magFilter = field_0x2d4_bit5 && (PCTarget()->triTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) ? D3DTFG_LINEAR
                                                                                               : D3DTFG_POINT;
    minFilter = field_0x2d4_bit5 && (PCTarget()->triTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) ? D3DTFN_LINEAR
                                                                                               : D3DTFN_POINT;
    mipFilter = D3DTFP_NONE;
    if (field_0x2d4_bit4) {
        if (field_0x2d4_bit7 && (PCTarget()->triTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR))
            mipFilter = D3DTFP_LINEAR;
        else
            mipFilter = D3DTFP_POINT;
    }
    PCTarget()->device->SetTextureStageState(0, D3DTSS_MAGFILTER, magFilter);
    PCTarget()->device->SetTextureStageState(0, D3DTSS_MINFILTER, minFilter);
    PCTarget()->device->SetTextureStageState(0, D3DTSS_MIPFILTER, mipFilter);
    PCTarget()->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    PCTarget()->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_LIGHTING, 0, 1);
    renderTarget->UnknownVirtualSlot8(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, 1);
    return 1;
}

// 0x004c0760
int PCGame::LimitDisplayModes(UnknownDisplay* display, int width, int height) {
    if (field_0x2d4_bit1 && !display->field_0xb74_bit1) {
        for (int i = 0; i < display->field_0x08; i++) {
            UnknownDisplayMode* mode = &display->field_0x10[i];
            if (mode->width > width || mode->height > height)
                mode->field_0x14 = 0;
        }
    }
    return 1;
}

// 0x004c0a90: creates the PCRenderTarget on the display's surface and
// records the display's two mode values (+0x58, +0x5c).
RenderTarget* PCGame::UnknownVirtualSlot31() {
    int frames = display->field_0x1a8 ? 1 : display->field_0x78;
    UnknownSurfaceInterface* surface =
        display->field_0x1a8 ? display->field_0x1a8 : display->field_0x1a0;
    RenderTarget* target = (new(__FILE__, 984) PCRenderTarget)
        ->UnknownFunction4c4f80(display, &deviceGuid, surface, field_0x2d5_bit3, frames);
    if (!target)
        return 0;
    if (field_0x2d4_bit1) {
        display->field_0x58 = display->field_0x10[display->field_0x0c].field_0x1c;
        display->field_0x5c = display->field_0x10[display->field_0x0c].field_0x20;
    } else {
        int value;
        if (display->UnknownFunction4ca5a0(&value, renderTarget)) {
            display->field_0x5c = value;
            PCTarget()->MeasureTextureMemory(&value);
            display->field_0x58 = value;
        } else {
            display->field_0x5c = 0;
            PCTarget()->MeasureTextureMemory(&value);
            display->field_0x58 = value;
        }
    }
    return target;
}

// 0x004c0c10: "lobby" on the command line (last match wins) starts the
// network object in mode 4.
int PCGame::UnknownVirtualSlot37() {
    if (__argc >= 2) {
        int i = __argc;
        while (i > 0) {
            i--;
            if (!_stricmp(__argv[i], "lobby")) {
                if (!CreateNetworkInterface(4))
                    return 0;
                break;
            }
        }
    }
    return 1;
}

// 0x004c0c60: records the window rectangle, then sets 640x480x16 (hiding
// the cursor in full screen) and runs slot 33.
int PCGame::UnknownVirtualSlot32() {
    GetWindowRect(field_0x31c, &field_0x308);
    if (field_0x2d4_bit1) {
        if (!display->UnknownFunction4c9c90())
            return 0;
        while (ShowCursor(0) >= 0)
            ;
    } else {
        if (!display->UnknownFunction4c9d20(0, 0, 640, 480))
            return 0;
    }
    if (!display->UnknownVirtualSlot2(640, 480, 16, 2, 0, field_0x2d0 == 0, 1))
        return 0;
    return UnknownVirtualSlot33() != 0;
}

// Builds the registry key path for a setting: the game's key, plus the
// subkey part of a "Sub\\Value" name, which is then reduced to Value.
#define UNKNOWN_SETTING_PATH(path, name)          \
    const char* slash = strrchr(name, '\\');      \
    strcpy(path, field_0x4b8);                     \
    if (slash) {                                   \
        strcat(path, "\\");                         \
        strcat(path, name);                        \
        *strrchr(path, '\\') = 0;                   \
        name = slash + 1;                          \
    }

// 0x004c1610: records the display's identifier under "DriverInfo\\<name>"
// and marks it profiled (version 7).
int PCGame::SaveDisplayProfile(UnknownDisplay* display) {
    char name[256];
    sprintf(name, "DriverInfo\\%s\\DeviceIdentifier", display->field_0x4bc);
    SetRegistryBinary(name, &display->field_0x5c0, sizeof(display->field_0x5c0));
    sprintf(name, "DriverInfo\\%s\\ProfiledCard", display->field_0x4bc);
    SetRegistryFlag(name, 0);
    sprintf(name, "DriverInfo\\%s\\ProfileVersion", display->field_0x4bc);
    SetRegistryInt(name, 7);
    return 1;
}

// 0x004c1410: whether any display lacks a current profile (version 7 with
// an identical saved identifier).
int PCGame::IsAnyProfileStale() {
    for (int i = 0; i < g_UnknownDisplayCount68a764; i++) {
        char name[256];
        UnknownDeviceIdentifier saved;
        unsigned long size = sizeof(saved);
        sprintf(name, "DriverInfo\\%s\\ProfileVersion", g_UnknownDisplays68a754[i]->field_0x4bc);
        if (GetRegistryInt(name, 0) != 7)
            return 1;
        sprintf(name, "DriverInfo\\%s\\DeviceIdentifier", g_UnknownDisplays68a754[i]->field_0x4bc);
        if (!GetRegistryBinary(name, &saved, &size) || size != sizeof(saved) ||
            strcmp(g_UnknownDisplays68a754[i]->field_0x5c0.driver, saved.driver) ||
            strcmp(g_UnknownDisplays68a754[i]->field_0x5c0.description, saved.description) ||
            memcmp(g_UnknownDisplays68a754[i]->field_0x5c0.driverVersion, saved.driverVersion,
                   sizeof(saved.driverVersion)) ||
            g_UnknownDisplays68a754[i]->field_0x5c0.vendorId != saved.vendorId ||
            g_UnknownDisplays68a754[i]->field_0x5c0.deviceId != saved.deviceId ||
            g_UnknownDisplays68a754[i]->field_0x5c0.subSysId != saved.subSysId ||
            g_UnknownDisplays68a754[i]->field_0x5c0.revision != saved.revision ||
            memcmp(g_UnknownDisplays68a754[i]->field_0x5c0.deviceGuid, saved.deviceGuid,
                   sizeof(saved.deviceGuid)) ||
            g_UnknownDisplays68a754[i]->field_0x5c0.whqlLevel != saved.whqlLevel)
            return 1;
    }
    return 0;
}

// 0x004c16b0
int PCGame::ProfileEveryDisplay() {
    for (int i = 0; i < g_UnknownDisplayCount68a764; i++)
        SaveDisplayProfile(g_UnknownDisplays68a754[i]);
    return 1;
}

// 0x004c16f0: loads the display's saved profile. Fails (0) unless the saved
// mode list matches the display's; marks the display disabled (+0xb74 bit 1)
// when the profile says so. The retail function returns without freeing the
// saved modes on the two disabled paths.
int PCGame::LoadDisplayProfile(UnknownDisplay* display) {
    char name[256];
    sprintf(name, "DriverInfo\\%s\\TextureCacheLimit", display->field_0x4bc);
    display->field_0x60 = GetRegistryInt(name, 0);
    if (display->field_0x60 <= 0)
        display->field_0x60 = 0x7fffffff;
    sprintf(name, "DriverInfo\\%s\\TotalVideoMemory", display->field_0x4bc);
    display->field_0x54 = GetRegistryInt(name, 0);
    sprintf(name, "DriverInfo\\%s\\NumberOfModes", display->field_0x4bc);
    int count = GetRegistryInt(name, 0);
    if (count != display->field_0x08)
        return 0;
    unsigned long size = count * sizeof(UnknownDisplayMode);
    sprintf(name, "DriverInfo\\%s\\Modes", display->field_0x4bc);
    UnknownDisplayMode* saved = (UnknownDisplayMode*)DebugMalloc(size, __FILE__, 1726);
    if (!saved)
        return 0;
    GetRegistryBinary(name, saved, &size);
    int i;
    for (i = 0; i < count; i++) {
        if (display->field_0x10[i].width != saved[i].width ||
            display->field_0x10[i].height != saved[i].height ||
            display->field_0x10[i].bitDepth != saved[i].bitDepth ||
            display->field_0x10[i].refreshRate != saved[i].refreshRate)
            break;
    }
    if (i != count) {
        DebugFree(saved, __FILE__, 1844);
        return 0;
    }
    // The copy loop counts the remaining modes down: an ascending index
    // swaps the base and index registers of its four stores.
    int remaining;
    for (remaining = count; remaining > 0; remaining--) {
        int index = count - remaining;
        display->field_0x10[index].field_0x14 = saved[index].field_0x14;
        display->field_0x10[index].field_0x18 = saved[index].field_0x18;
        display->field_0x10[index].field_0x1c = saved[index].field_0x1c;
        display->field_0x10[index].field_0x20 = saved[index].field_0x20;
    }
    sprintf(name, "DriverInfo\\%s\\PartialTextureBlt", display->field_0x4bc);
    display->field_0x5bc = GetRegistryInt(name, 0);
    sprintf(name, "DriverInfo\\%s\\Use8BitTextures", display->field_0x4bc);
    display->field_0x70_bit0 = GetRegistryFlag(name, 1);
    sprintf(name, "DriverInfo\\%s\\IsAGP", display->field_0x4bc);
    display->field_0x9f0 = GetRegistryFlag(name, 0);
    if (field_0x2d4_bit1)
        sprintf(name, "DriverInfo\\%s\\DisabledFullScreen", display->field_0x4bc);
    else
        sprintf(name, "DriverInfo\\%s\\DisabledWindowed", display->field_0x4bc);
    if (GetRegistryFlag(name, 0)) {
        display->field_0xb74_bit1 = 1;
        return 1;
    }
    sprintf(name, "DriverInfo\\%s\\DisabledHardware", display->field_0x4bc);
    if (GetRegistryFlag(name, 0)) {
        display->field_0xb74_bit1 = 1;
        return 1;
    }
    DebugFree(saved, __FILE__, 1841);
    return 1;
}

// 0x004c1a00: deletes every display's cached data under DriverInfo (the
// mode lists, "PartialTextureBlt" and the 32 "BltSpeed" entries), then the
// display keys themselves.
int PCGame::DeleteDisplayProfiles() {
    char path[256];
    char display[256];
    char name[16];
    void* driverInfo;
    void* key;
    unsigned long count = 0;
    strcpy(path, field_0x4b8);
    strcat(path, "\\DriverInfo");
    if (RegOpenKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, UNKNOWN_KEY_ALL_ACCESS, &driverInfo) != 0)
        return 1;
    if (RegQueryInfoKeyA(driverInfo, 0, 0, 0, &count, 0, 0, 0, 0, 0, 0, 0) != 0)
        return 1;
    while (count) {
        if (RegEnumKeyA(driverInfo, count - 1, display, sizeof(display)) != 0)
            break;
        count--;
        strcpy(path, field_0x4b8);
        strcat(path, "\\DriverInfo\\");
        strcat(path, display);
        if (RegOpenKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, UNKNOWN_KEY_ALL_ACCESS, &key) == 0) {
            RegDeleteKeyA(key, "NumberOfModes");
            RegDeleteKeyA(key, "Modes");
            RegDeleteKeyA(key, "PartialTextureBlt");
            for (int i = 0; i < 32; i++) {
                sprintf(name, "BltSpeed%d", i);
                RegDeleteKeyA(key, name);
            }
            RegCloseKey(key);
        }
        RegDeleteKeyA(driverInfo, display);
    }
    RegCloseKey(driverInfo);
    return 1;
}

// 0x004c1c20: a DWORD setting.
int PCGame::GetRegistryInt(const char* name, int defaultValue) {
    unsigned long size = sizeof(int);
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long type;
    int value;
    if (RegOpenKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, UNKNOWN_KEY_READ, &key) != 0 ||
        RegQueryValueExA(key, name, 0, &type, (unsigned char*)&value, &size) != 0 ||
        type != UNKNOWN_REG_DWORD)
        value = defaultValue;
    RegCloseKey(key);
    return value;
}

// 0x004c1d50: a float setting, stored as a DWORD.
float PCGame::GetRegistryFloat(const char* name, float defaultValue) {
    unsigned long size = sizeof(float);
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long type;
    float value;
    if (RegOpenKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, UNKNOWN_KEY_READ, &key) != 0 ||
        RegQueryValueExA(key, name, 0, &type, (unsigned char*)&value, &size) != 0 ||
        type != UNKNOWN_REG_DWORD)
        value = defaultValue;
    RegCloseKey(key);
    return value;
}

// 0x004c1e80: a boolean setting.
int PCGame::GetRegistryFlag(const char* name, int defaultValue) {
    unsigned long size = sizeof(int);
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long type;
    int value;
    if (RegOpenKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, UNKNOWN_KEY_READ, &key) == 0 &&
        RegQueryValueExA(key, name, 0, &type, (unsigned char*)&value, &size) == 0 &&
        type == UNKNOWN_REG_DWORD)
        value = value != 0;
    else
        value = defaultValue;
    RegCloseKey(key);
    return value;
}

// 0x004c1fc0: a string setting; copies the default when it is missing.
int PCGame::GetRegistryString(const char* name, const char* defaultValue, char* buffer,
                              unsigned long* size) {
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long type;
    int result;
    if (RegOpenKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, UNKNOWN_KEY_READ, &key) == 0 &&
        RegQueryValueExA(key, name, 0, &type, (unsigned char*)buffer, size) == 0 &&
        type == UNKNOWN_REG_SZ) {
        result = 1;
    } else {
        strcpy(buffer, defaultValue);
        result = 0;
    }
    RegCloseKey(key);
    return result;
}

// 0x004c2110: a binary setting.
int PCGame::GetRegistryBinary(const char* name, void* data, unsigned long* size) {
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long type;
    int result;
    if (RegOpenKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, UNKNOWN_KEY_READ, &key) == 0 &&
        RegQueryValueExA(key, name, 0, &type, (unsigned char*)data, size) == 0 &&
        type == UNKNOWN_REG_BINARY)
        result = 1;
    else
        result = 0;
    RegCloseKey(key);
    return result;
}

// 0x004c2240 (slots 25-27 fold to this body): writes a DWORD setting.
int PCGame::SetRegistryInt(const char* name, int value) {
    int result = 1;
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long disposition;
    if (RegCreateKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, "", 0, UNKNOWN_KEY_ALL_ACCESS, 0,
                        &key, &disposition) != 0 ||
        RegSetValueExA(key, name, 0, UNKNOWN_REG_DWORD, (const unsigned char*)&value,
                       sizeof(value)) != 0)
        result = 0;
    RegCloseKey(key);
    return result;
}

int PCGame::UnknownVirtualSlot26(const char* name, int value) {
    int result = 1;
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long disposition;
    if (RegCreateKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, "", 0, UNKNOWN_KEY_ALL_ACCESS, 0,
                        &key, &disposition) != 0 ||
        RegSetValueExA(key, name, 0, UNKNOWN_REG_DWORD, (const unsigned char*)&value,
                       sizeof(value)) != 0)
        result = 0;
    RegCloseKey(key);
    return result;
}

int PCGame::SetRegistryFlag(const char* name, int value) {
    int result = 1;
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long disposition;
    if (RegCreateKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, "", 0, UNKNOWN_KEY_ALL_ACCESS, 0,
                        &key, &disposition) != 0 ||
        RegSetValueExA(key, name, 0, UNKNOWN_REG_DWORD, (const unsigned char*)&value,
                       sizeof(value)) != 0)
        result = 0;
    RegCloseKey(key);
    return result;
}

// 0x004c2370: writes a string setting.
int PCGame::SetRegistryString(const char* name, const char* value) {
    int result = 1;
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long disposition;
    if (RegCreateKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, "", 0, UNKNOWN_KEY_ALL_ACCESS, 0,
                        &key, &disposition) != 0 ||
        RegSetValueExA(key, name, 0, UNKNOWN_REG_SZ, (const unsigned char*)value,
                       strlen(value) + 1) != 0)
        result = 0;
    RegCloseKey(key);
    return result;
}

// 0x004c24b0: writes a binary setting.
int PCGame::SetRegistryBinary(const char* name, const void* data, unsigned long size) {
    int result = 1;
    char path[256];
    UNKNOWN_SETTING_PATH(path, name)
    void* key;
    unsigned long disposition;
    if (RegCreateKeyExA(UNKNOWN_HKEY_LOCAL_MACHINE, path, 0, "", 0, UNKNOWN_KEY_ALL_ACCESS, 0,
                        &key, &disposition) != 0 ||
        RegSetValueExA(key, name, 0, UNKNOWN_REG_BINARY, (const unsigned char*)data, size) != 0)
        result = 0;
    RegCloseKey(key);
    return result;
}
