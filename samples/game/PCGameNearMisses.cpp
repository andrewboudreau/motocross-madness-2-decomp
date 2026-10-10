// Near-miss PCGame candidates, kept out of src/reconstructed until they
// match. See docs/PCGAME.md.
//
// PCGame::ProfileDisplays (0x004c0d10, 1790 bytes, returns 1 from both
// exits): 1776 of 1790 positions match. The capability block is cleared
// with memset before its caps word is set (retail stores 0 then
// 0x10000000 to +0x20); with four field stores the block ranked above the
// name buffer and every frame offset and register differed (208 of 1772).
// `display` is assigned before `renderTarget` (retail loads the display
// pointer before the renderTarget store). Remaining: retail stores the
// surface description's size (0x7c) right after the memset, VC6 here after
// pushing the 0x005119c0 arguments. Not moved by sizeof/0x7c spellings, a
// pointer to the description, an inline clear helper (reference or
// pointer), a format local or the description declared outside the loop.
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCGame.h"

extern "C" const UnknownGuid IID_IDirect3DHALDevice;
void UnknownFunction5119c0(int format, void* pixelFormat);

// 0x004c0d10: profiles each display that has no current profile: its mode
// list, whether windowed, full-screen, software and hardware rendering work,
// its video memory, AGP, whether ten 256x256 textures fit and the partial
// texture blit timing.
int PCGame::ProfileDisplays() {
    int loaded = 0;
    int i;
    for (i = 0; i < g_UnknownDisplayCount68a764; i++)
        if (LoadDisplayProfile(g_UnknownDisplays68a754[i]))
            loaded++;
    if (loaded == g_UnknownDisplayCount68a764)
        return 1;
    for (i = 0; i < g_UnknownDisplayCount68a764; i++) {
        char name[128];
        sprintf(name, "DriverInfo\\%s\\ProfiledCard", g_UnknownDisplays68a754[i]->driverGuidText);
        SetRegistryFlag(name, 1);
        sprintf(name, "DriverInfo\\%s\\TextureCacheLimit", g_UnknownDisplays68a754[i]->driverGuidText);
        SetRegistryInt(name, 0);
        sprintf(name, "DriverInfo\\%s\\NumberOfModes", g_UnknownDisplays68a754[i]->driverGuidText);
        SetRegistryInt(name, g_UnknownDisplays68a754[i]->displayModeCount);
        sprintf(name, "DriverInfo\\%s\\Modes", g_UnknownDisplays68a754[i]->driverGuidText);
        SetRegistryBinary(name, g_UnknownDisplays68a754[i]->displayModes,
                             g_UnknownDisplays68a754[i]->displayModeCount * sizeof(UnknownDisplayMode));
        sprintf(name, "DriverInfo\\%s\\IsAGP", g_UnknownDisplays68a754[i]->driverGuidText);
        SetRegistryFlag(name, 0);
        sprintf(name, "DriverInfo\\%s\\DisabledWindowed", g_UnknownDisplays68a754[i]->driverGuidText);
        if (!GetRegistryFlag(name, 0)) {
            SetRegistryFlag(name, 1);
            if (g_UnknownDisplays68a754[i]->SetWindowedCooperativeLevel(0, 0, 640, 480))
                SetRegistryFlag(name, 0);
        }
        sprintf(name, "DriverInfo\\%s\\DisabledFullScreen", g_UnknownDisplays68a754[i]->driverGuidText);
        if (GetRegistryFlag(name, 0))
            continue;
        SetRegistryFlag(name, 1);
        if (g_UnknownDisplays68a754[i]->SetFullscreenCooperativeLevel() &&
            g_UnknownDisplays68a754[i]->directDraw->SetDisplayMode(640, 480, 16, 0, 0) == 0) {
            UnknownSurfaceCaps caps;
            unsigned long total;
            unsigned long free;
            memset(&caps, 0, sizeof(caps));
            caps.caps = 0x10000000;
            g_UnknownDisplays68a754[i]->directDraw->GetAvailableVidMem(&caps, &total, &free);
            g_UnknownDisplays68a754[i]->totalVideoMemory = total;
            sprintf(name, "DriverInfo\\%s\\TotalVideoMemory", g_UnknownDisplays68a754[i]->driverGuidText);
            SetRegistryInt(name, total);
            sprintf(name, "DriverInfo\\%s\\DisabledFullScreen", g_UnknownDisplays68a754[i]->driverGuidText);
            SetRegistryFlag(name, 0);
            UnknownVirtualSlot34(g_UnknownDisplays68a754[i]);
            sprintf(name, "DriverInfo\\%s\\Modes", g_UnknownDisplays68a754[i]->driverGuidText);
            SetRegistryBinary(name, g_UnknownDisplays68a754[i]->displayModes,
                                 g_UnknownDisplays68a754[i]->displayModeCount * sizeof(UnknownDisplayMode));
            sprintf(name, "DriverInfo\\%s\\DisabledSoftware", g_UnknownDisplays68a754[i]->driverGuidText);
            if (!GetRegistryFlag(name, 0)) {
                SetRegistryFlag(name, 1);
                if (g_UnknownDisplays68a754[i]->UnknownVirtualSlot2(640, 480, 16, 2, 0, 0, 0))
                    SetRegistryFlag(name, 0);
            }
            sprintf(name, "DriverInfo\\%s\\DisabledHardware", g_UnknownDisplays68a754[i]->driverGuidText);
            if (!GetRegistryFlag(name, 0)) {
                SetRegistryFlag(name, 1);
                if (g_UnknownDisplays68a754[i]->UnknownVirtualSlot2(640, 480, 16, 2, 0, 1, 0)) {
                    g_UnknownDisplays68a754[i]->RefreshDriverCaps();
                    if (g_UnknownDisplays68a754[i]->driverCaps & 1) {
                        RenderTarget* target = (new(__FILE__, 1423) PCRenderTarget)
                            ->InitializeRenderTarget(g_UnknownDisplays68a754[i], &IID_IDirect3DHALDevice,
                                                    g_UnknownDisplays68a754[i]->backBuffer, 1,
                                                    g_UnknownDisplays68a754[i]->frameBufferCount);
                        if (target) {
                            if (osVersion.platformId != 2 &&
                                !(g_UnknownDisplays68a754[i]->driverCaps & 0x400)) {
                                if (((PCRenderTarget*)target)->deviceCaps & 0x4000)
                                    LimitDisplayModes(g_UnknownDisplays68a754[i], 800, 600);
                                else
                                    LimitDisplayModes(g_UnknownDisplays68a754[i], 640, 480);
                                sprintf(name, "DriverInfo\\%s\\Modes",
                                        g_UnknownDisplays68a754[i]->driverGuidText);
                                SetRegistryBinary(name, g_UnknownDisplays68a754[i]->displayModes,
                                                     g_UnknownDisplays68a754[i]->displayModeCount *
                                                         sizeof(UnknownDisplayMode));
                            }
                            int value;
                            g_UnknownDisplays68a754[i]->isAGP =
                                g_UnknownDisplays68a754[i]->ProbeNonLocalTextureMemory(&value, target);
                            sprintf(name, "DriverInfo\\%s\\IsAGP", g_UnknownDisplays68a754[i]->driverGuidText);
                            SetRegistryFlag(name, g_UnknownDisplays68a754[i]->isAGP);
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
                                if (g_UnknownDisplays68a754[i]->directDraw->CreateSurface(
                                        &desc, &surfaces[count], 0) != 0)
                                    break;
                            }
                            for (int j = 0; j < count; j++)
                                surfaces[j]->Release();
                            if (count == 10) {
                                sprintf(name, "DriverInfo\\%s\\DisabledHardware",
                                        g_UnknownDisplays68a754[i]->driverGuidText);
                                SetRegistryFlag(name, 0);
                                sprintf(name, "DriverInfo\\%s\\PartialTextureBlt",
                                        g_UnknownDisplays68a754[i]->driverGuidText);
                                if (GetRegistryInt(name, 1) > 0) {
                                    g_UnknownDisplays68a754[i]->partialTextureUploadResult = 0;
                                    SetRegistryInt(name, g_UnknownDisplays68a754[i]->partialTextureUploadResult);
                                    display = g_UnknownDisplays68a754[i];
                                    renderTarget = target;
                                    g_UnknownDisplays68a754[i]->ProbePartialTextureUploads(target);
                                    display = 0;
                                    renderTarget = 0;
                                    SetRegistryInt(name, g_UnknownDisplays68a754[i]->partialTextureUploadResult);
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
        g_UnknownDisplays68a754[i]->SetWindowedCooperativeLevel(0, 0, 0, 0);
    }
    return 1;
}
