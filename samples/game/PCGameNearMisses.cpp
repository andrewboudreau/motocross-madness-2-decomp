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
// Also unchanged: a memset+size helper (pointer or reference), a helper
// that also makes the 0x005119c0 call, a derived description whose
// constructor clears and sizes it, a pixel-format pointer local (before or
// after the clear), a format local after the size store, `(char*)&desc +
// 0x48`, a PCRenderTarget cast for the format and sizeof(type); storing
// the size through memset's return value is worse (1344). The canonical
// PCRenderTarget::MeasureTextureMemory (0x004c5950) has the same three
// statements and retail there does store the size after the pushes.
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCGame.h"

extern "C" const UnknownGuid IID_IDirect3DHALDevice;
void FillPixelFormat(int format, void* pixelFormat);

// 0x004c0d10: unless all cached profiles load, profiles every display: its mode
// list, whether windowed, full-screen, software and hardware rendering work,
// its video memory, AGP, whether ten 256x256 textures fit and the partial
// texture blit timing.
int PCGame::ProfileDisplays() {
    int loaded = 0;
    int i;
    for (i = 0; i < g_DisplayCount; i++)
        if (LoadDisplayProfile(g_Displays[i]))
            loaded++;
    if (loaded == g_DisplayCount)
        return 1;
    for (i = 0; i < g_DisplayCount; i++) {
        char name[128];
        sprintf(name, "DriverInfo\\%s\\ProfiledCard", g_Displays[i]->driverGuidText);
        SetRegistryFlag(name, 1);
        sprintf(name, "DriverInfo\\%s\\TextureCacheLimit", g_Displays[i]->driverGuidText);
        SetRegistryInt(name, 0);
        sprintf(name, "DriverInfo\\%s\\NumberOfModes", g_Displays[i]->driverGuidText);
        SetRegistryInt(name, g_Displays[i]->displayModeCount);
        sprintf(name, "DriverInfo\\%s\\Modes", g_Displays[i]->driverGuidText);
        SetRegistryBinary(name, g_Displays[i]->displayModes,
                             g_Displays[i]->displayModeCount * sizeof(UnknownDisplayMode));
        sprintf(name, "DriverInfo\\%s\\IsAGP", g_Displays[i]->driverGuidText);
        SetRegistryFlag(name, 0);
        sprintf(name, "DriverInfo\\%s\\DisabledWindowed", g_Displays[i]->driverGuidText);
        if (!GetRegistryFlag(name, 0)) {
            SetRegistryFlag(name, 1);
            if (g_Displays[i]->SetWindowedCooperativeLevel(0, 0, 640, 480))
                SetRegistryFlag(name, 0);
        }
        sprintf(name, "DriverInfo\\%s\\DisabledFullScreen", g_Displays[i]->driverGuidText);
        if (GetRegistryFlag(name, 0))
            continue;
        SetRegistryFlag(name, 1);
        if (g_Displays[i]->SetFullscreenCooperativeLevel() &&
            g_Displays[i]->directDraw->SetDisplayMode(640, 480, 16, 0, 0) == 0) {
            UnknownSurfaceCaps caps;
            unsigned long total;
            unsigned long free;
            memset(&caps, 0, sizeof(caps));
            caps.caps = 0x10000000;
            g_Displays[i]->directDraw->GetAvailableVidMem(&caps, &total, &free);
            g_Displays[i]->totalVideoMemory = total;
            sprintf(name, "DriverInfo\\%s\\TotalVideoMemory", g_Displays[i]->driverGuidText);
            SetRegistryInt(name, total);
            sprintf(name, "DriverInfo\\%s\\DisabledFullScreen", g_Displays[i]->driverGuidText);
            SetRegistryFlag(name, 0);
            FilterDisplayModes(g_Displays[i]);
            sprintf(name, "DriverInfo\\%s\\Modes", g_Displays[i]->driverGuidText);
            SetRegistryBinary(name, g_Displays[i]->displayModes,
                                 g_Displays[i]->displayModeCount * sizeof(UnknownDisplayMode));
            sprintf(name, "DriverInfo\\%s\\DisabledSoftware", g_Displays[i]->driverGuidText);
            if (!GetRegistryFlag(name, 0)) {
                SetRegistryFlag(name, 1);
                if (g_Displays[i]->SetDisplayMode(640, 480, 16, 2, 0, 0, 0))
                    SetRegistryFlag(name, 0);
            }
            sprintf(name, "DriverInfo\\%s\\DisabledHardware", g_Displays[i]->driverGuidText);
            if (!GetRegistryFlag(name, 0)) {
                SetRegistryFlag(name, 1);
                if (g_Displays[i]->SetDisplayMode(640, 480, 16, 2, 0, 1, 0)) {
                    g_Displays[i]->RefreshDriverCaps();
                    if (g_Displays[i]->driverCaps & 1) {
                        RenderTarget* target = (new(__FILE__, 1423) PCRenderTarget)
                            ->InitializeRenderTarget(g_Displays[i], &IID_IDirect3DHALDevice,
                                                    g_Displays[i]->backBuffer, 1,
                                                    g_Displays[i]->frameBufferCount);
                        if (target) {
                            if (osVersion.platformId != 2 &&
                                !(g_Displays[i]->driverCaps & 0x400)) {
                                if (((PCRenderTarget*)target)->deviceCaps & 0x4000)
                                    LimitDisplayModes(g_Displays[i], 800, 600);
                                else
                                    LimitDisplayModes(g_Displays[i], 640, 480);
                                sprintf(name, "DriverInfo\\%s\\Modes",
                                        g_Displays[i]->driverGuidText);
                                SetRegistryBinary(name, g_Displays[i]->displayModes,
                                                     g_Displays[i]->displayModeCount *
                                                         sizeof(UnknownDisplayMode));
                            }
                            int value;
                            g_Displays[i]->isAGP =
                                g_Displays[i]->ProbeNonLocalTextureMemory(&value, target);
                            sprintf(name, "DriverInfo\\%s\\IsAGP", g_Displays[i]->driverGuidText);
                            SetRegistryFlag(name, g_Displays[i]->isAGP);
                            UnknownSurfaceInterface* surfaces[10];
                            int count;
                            for (count = 0; count < 10; count++) {
                                UnknownSurfaceDesc desc;
                                memset(&desc, 0, sizeof(desc));
                                desc.size = sizeof(desc);
                                FillPixelFormat(target->surfacePixelFormat, &desc.pixelFormat);
                                desc.height = desc.width = 256;
                                desc.flags = 0x1007;
                                desc.caps[0] = 0x10005000;
                                if (g_Displays[i]->directDraw->CreateSurface(
                                        &desc, &surfaces[count], 0) != 0)
                                    break;
                            }
                            for (int j = 0; j < count; j++)
                                surfaces[j]->Release();
                            if (count == 10) {
                                sprintf(name, "DriverInfo\\%s\\DisabledHardware",
                                        g_Displays[i]->driverGuidText);
                                SetRegistryFlag(name, 0);
                                sprintf(name, "DriverInfo\\%s\\PartialTextureBlt",
                                        g_Displays[i]->driverGuidText);
                                if (GetRegistryInt(name, 1) > 0) {
                                    g_Displays[i]->partialTextureUploadResult = 0;
                                    SetRegistryInt(name, g_Displays[i]->partialTextureUploadResult);
                                    display = g_Displays[i];
                                    renderTarget = target;
                                    g_Displays[i]->ProbePartialTextureUploads(target);
                                    display = 0;
                                    renderTarget = 0;
                                    SetRegistryInt(name, g_Displays[i]->partialTextureUploadResult);
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
        g_Displays[i]->SetWindowedCooperativeLevel(0, 0, 0, 0);
    }
    return 1;
}
