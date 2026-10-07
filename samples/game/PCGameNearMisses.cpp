// Near-miss PCGame candidates, kept out of src/reconstructed until they
// match. See docs/PCGAME.md.
//
// PCGame::LoadDisplayProfile (0x004c16f0, 771 bytes): 767 of 771 bytes
// match. The four stores in the profile copy loop encode their address as
// [mode table + offset] where retail has [offset + mode table] (SIB base and
// index swapped). Pointer-walk, `(table + i)->`, reference and 16-byte
// struct-copy forms do not change it.
//
// PCGame::ProfileDisplays (0x004c0d10, 1790 bytes): the control flow,
// calls and constants line up, but the frame and registers do not. Retail
// keeps the 16-byte capability block below the name buffer, and in the first
// loop holds the count in ebp, the index in edi and the array pointer in
// ebx. Declaration order, scope placement, a struct or array for the
// capabilities and separate loop counters leave VC6's layout unchanged.
// Small test functions place the smaller array lower, as retail does, so
// something specific to this function decides it.
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCGame.h"

extern "C" const UnknownGuid IID_IDirect3DHALDevice;
void UnknownFunction5119c0(int format, void* pixelFormat);

// 0x004c16f0: loads the display's saved profile. Fails (0) unless the saved
// mode list matches the display's; marks the display disabled (+0xb74 bit 1)
// when the profile says so.
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
    for (i = 0; i < count; i++) {
        display->field_0x10[i].field_0x14 = saved[i].field_0x14;
        display->field_0x10[i].field_0x18 = saved[i].field_0x18;
        display->field_0x10[i].field_0x1c = saved[i].field_0x1c;
        display->field_0x10[i].field_0x20 = saved[i].field_0x20;
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

// 0x004c0d10: profiles each display that has no current profile: its mode
// list, whether windowed, full-screen, software and hardware rendering work,
// its video memory, AGP, whether ten 256x256 textures fit and the partial
// texture blit timing.
void PCGame::ProfileDisplays() {
    int loaded = 0;
    int i;
    for (i = 0; i < g_UnknownDisplayCount68a764; i++)
        if (LoadDisplayProfile(g_UnknownDisplays68a754[i]))
            loaded++;
    if (loaded == g_UnknownDisplayCount68a764)
        return;
    for (i = 0; i < g_UnknownDisplayCount68a764; i++) {
        char name[128];
        sprintf(name, "DriverInfo\\%s\\ProfiledCard", g_UnknownDisplays68a754[i]->field_0x4bc);
        SetRegistryFlag(name, 1);
        sprintf(name, "DriverInfo\\%s\\TextureCacheLimit", g_UnknownDisplays68a754[i]->field_0x4bc);
        SetRegistryInt(name, 0);
        sprintf(name, "DriverInfo\\%s\\NumberOfModes", g_UnknownDisplays68a754[i]->field_0x4bc);
        SetRegistryInt(name, g_UnknownDisplays68a754[i]->field_0x08);
        sprintf(name, "DriverInfo\\%s\\Modes", g_UnknownDisplays68a754[i]->field_0x4bc);
        SetRegistryBinary(name, g_UnknownDisplays68a754[i]->field_0x10,
                             g_UnknownDisplays68a754[i]->field_0x08 * sizeof(UnknownDisplayMode));
        sprintf(name, "DriverInfo\\%s\\IsAGP", g_UnknownDisplays68a754[i]->field_0x4bc);
        SetRegistryFlag(name, 0);
        sprintf(name, "DriverInfo\\%s\\DisabledWindowed", g_UnknownDisplays68a754[i]->field_0x4bc);
        if (!GetRegistryFlag(name, 0)) {
            SetRegistryFlag(name, 1);
            if (g_UnknownDisplays68a754[i]->UnknownFunction4c9d20(0, 0, 640, 480))
                SetRegistryFlag(name, 0);
        }
        sprintf(name, "DriverInfo\\%s\\DisabledFullScreen", g_UnknownDisplays68a754[i]->field_0x4bc);
        if (GetRegistryFlag(name, 0))
            continue;
        SetRegistryFlag(name, 1);
        if (g_UnknownDisplays68a754[i]->UnknownFunction4c9c90() &&
            g_UnknownDisplays68a754[i]->field_0x190->UnknownMethod21(640, 480, 16, 0, 0) == 0) {
            UnknownSurfaceCaps caps;
            unsigned long total;
            unsigned long free;
            caps.caps = 0x10000000;
            caps.caps2 = 0;
            caps.caps3 = 0;
            caps.caps4 = 0;
            g_UnknownDisplays68a754[i]->field_0x190->UnknownMethod23(&caps, &total, &free);
            g_UnknownDisplays68a754[i]->field_0x54 = total;
            sprintf(name, "DriverInfo\\%s\\TotalVideoMemory", g_UnknownDisplays68a754[i]->field_0x4bc);
            SetRegistryInt(name, total);
            sprintf(name, "DriverInfo\\%s\\DisabledFullScreen", g_UnknownDisplays68a754[i]->field_0x4bc);
            SetRegistryFlag(name, 0);
            UnknownVirtualSlot34(g_UnknownDisplays68a754[i]);
            sprintf(name, "DriverInfo\\%s\\Modes", g_UnknownDisplays68a754[i]->field_0x4bc);
            SetRegistryBinary(name, g_UnknownDisplays68a754[i]->field_0x10,
                                 g_UnknownDisplays68a754[i]->field_0x08 * sizeof(UnknownDisplayMode));
            sprintf(name, "DriverInfo\\%s\\DisabledSoftware", g_UnknownDisplays68a754[i]->field_0x4bc);
            if (!GetRegistryFlag(name, 0)) {
                SetRegistryFlag(name, 1);
                if (g_UnknownDisplays68a754[i]->UnknownVirtualSlot2(640, 480, 16, 2, 0, 0, 0))
                    SetRegistryFlag(name, 0);
            }
            sprintf(name, "DriverInfo\\%s\\DisabledHardware", g_UnknownDisplays68a754[i]->field_0x4bc);
            if (!GetRegistryFlag(name, 0)) {
                SetRegistryFlag(name, 1);
                if (g_UnknownDisplays68a754[i]->UnknownVirtualSlot2(640, 480, 16, 2, 0, 1, 0)) {
                    g_UnknownDisplays68a754[i]->UnknownFunction4c9b50();
                    if (g_UnknownDisplays68a754[i]->field_0x1b8 & 1) {
                        RenderTarget* target = (new(__FILE__, 1423) PCRenderTarget)
                            ->UnknownFunction4c4f80(g_UnknownDisplays68a754[i], &IID_IDirect3DHALDevice,
                                                    g_UnknownDisplays68a754[i]->field_0x1a0, 1,
                                                    g_UnknownDisplays68a754[i]->field_0x78);
                        if (target) {
                            if (field_0x424.platformId != 2 &&
                                !(g_UnknownDisplays68a754[i]->field_0x1b8 & 0x400)) {
                                if (((PCRenderTarget*)target)->field_0x164 & 0x4000)
                                    LimitDisplayModes(g_UnknownDisplays68a754[i], 800, 600);
                                else
                                    LimitDisplayModes(g_UnknownDisplays68a754[i], 640, 480);
                                sprintf(name, "DriverInfo\\%s\\Modes",
                                        g_UnknownDisplays68a754[i]->field_0x4bc);
                                SetRegistryBinary(name, g_UnknownDisplays68a754[i]->field_0x10,
                                                     g_UnknownDisplays68a754[i]->field_0x08 *
                                                         sizeof(UnknownDisplayMode));
                            }
                            int value;
                            g_UnknownDisplays68a754[i]->field_0x9f0 =
                                g_UnknownDisplays68a754[i]->UnknownFunction4ca5a0(&value, target);
                            sprintf(name, "DriverInfo\\%s\\IsAGP", g_UnknownDisplays68a754[i]->field_0x4bc);
                            SetRegistryFlag(name, g_UnknownDisplays68a754[i]->field_0x9f0);
                            UnknownSurfaceInterface* surfaces[10];
                            int count;
                            for (count = 0; count < 10; count++) {
                                UnknownSurfaceDesc desc;
                                memset(&desc, 0, sizeof(desc));
                                desc.size = sizeof(desc);
                                UnknownFunction5119c0(target->field_0x28, &desc.pixelFormat);
                                desc.height = desc.width = 256;
                                desc.flags = 0x1007;
                                desc.caps[0] = 0x10005000;
                                if (g_UnknownDisplays68a754[i]->field_0x190->UnknownMethod6(
                                        &desc, &surfaces[count], 0) != 0)
                                    break;
                            }
                            for (int j = 0; j < count; j++)
                                surfaces[j]->UnknownMethod2();
                            if (count == 10) {
                                sprintf(name, "DriverInfo\\%s\\DisabledHardware",
                                        g_UnknownDisplays68a754[i]->field_0x4bc);
                                SetRegistryFlag(name, 0);
                                sprintf(name, "DriverInfo\\%s\\PartialTextureBlt",
                                        g_UnknownDisplays68a754[i]->field_0x4bc);
                                if (GetRegistryInt(name, 1) > 0) {
                                    g_UnknownDisplays68a754[i]->field_0x5bc = 0;
                                    SetRegistryInt(name, g_UnknownDisplays68a754[i]->field_0x5bc);
                                    renderTarget = target;
                                    display = g_UnknownDisplays68a754[i];
                                    g_UnknownDisplays68a754[i]->UnknownFunction4cab00(target);
                                    display = 0;
                                    renderTarget = 0;
                                    SetRegistryInt(name, g_UnknownDisplays68a754[i]->field_0x5bc);
                                }
                                delete target;
                            } else {
                                delete target;
                            }
                        }
                    }
                }
            }
        }
        g_UnknownDisplays68a754[i]->UnknownFunction4c9d20(0, 0, 0, 0);
    }
}
