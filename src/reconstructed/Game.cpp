#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugAlloc.h"
#include "Game.h"

#include "ControlInterface.h"
#include "DebugOverlay.h"
#include "GameObject.h"
#include "MemTag.h"
#include "PCControl.h"
#include "PeakHold.h"
#include "SoundInterface.h"
#include "TextureMapManager.h"
#include "TrackGame.h"

// 0x0065b4ac: a file cleared by the constructor and closed by the
// destructor (0x00534c3d is the CRT's fclose).
static FILE* s_UnknownFile65b4ac;

// 0x0056b334 and 0x0056b350: stored XOR 0x5b and decoded by the constructor
// ("SOFTWARE\\Rainbow Studios\\" and "TestKey").
static char s_UnknownEncoded56b334[] =
    "\x08\x14\x1d\x0f\x0c\x1a\x09\x1e\x07\x09\x3a\x32\x35\x39\x34\x2c\x7b\x08\x2f\x2e\x3f\x32\x34\x28\x07";
static char s_UnknownEncoded56b350[] = "\x0f\x3e\x28\x2f\x10\x3e\x22";

// ADVAPI32 imports (called through the import table).
extern "C" __declspec(dllimport) long __stdcall RegOpenKeyExA(void* key, const char* subKey,
                                                             unsigned long options,
                                                             unsigned long access, void** result);
extern "C" __declspec(dllimport) long __stdcall RegQueryValueExA(void* key, const char* name,
                                                                unsigned long* reserved,
                                                                unsigned long* type,
                                                                unsigned char* data,
                                                                unsigned long* size);
extern "C" __declspec(dllimport) long __stdcall RegCloseKey(void* key);

// KERNEL32 MEMORYSTATUS and its import (slot 8's memory page).
struct UnknownMemoryStatus {
    unsigned long length;
    unsigned long memoryLoad;
    unsigned long totalPhys;
    unsigned long availPhys;
    unsigned long totalPageFile;
    unsigned long availPageFile;
    unsigned long totalVirtual;
    unsigned long availVirtual;
};
extern "C" __declspec(dllimport) void __stdcall GlobalMemoryStatus(UnknownMemoryStatus* status);

// 0x0065b490..0x0065b528: peak-held frame timings for slot 8's profile page,
// constructed in this order (0x00467860..0x00467980).
static UnknownPeakHold s_NetPeak(5000);              // 0x0065b518
static UnknownPeakHold s_TickPeak(5000);             // 0x0065b4c8
static UnknownPeakHold s_PrepFramePeak(5000);        // 0x0065b528
static UnknownPeakHold s_UpdateScreenPeak(5000);     // 0x0065b4e8
static UnknownPeakHold s_PrepareGeometryPeak(5000);  // 0x0065b490
static UnknownPeakHold s_WaitForFlipPeak(5000);      // 0x0065b4b8
static UnknownPeakHold s_RenderPre3DPeak(5000);      // 0x0065b4f8
static UnknownPeakHold s_Render3DPeak(5000);         // 0x0065b4d8
static UnknownPeakHold s_RenderPost3DPeak(5000);     // 0x0065b4a0
static UnknownPeakHold s_ElapsedPeak(5000);          // 0x0065b508

// 0x0065b534..0x0065b540: phase times measured by slot 10, printed on the
// profile page as "Tick", "PrepFrameTime:", "UpdateScrnTime:" and "Net"
// (GUIManager's timer page prints the first as "Previous Game Tick");
// 0x0065b544 is the time of the previous frame.
int g_TickTime;                                      // 0x0065b534
int g_PrepFrameTime;                                 // 0x0065b538
int g_UpdateScreenTime;                              // 0x0065b53c
int g_NetTime;                                       // 0x0065b540
static unsigned int s_LastFrameTime;                 // 0x0065b544

static inline void DecodeString(char* text) {
    for (; *text; text++)
        *text ^= 0x5b;
}

// 0x00467990
Game::Game() {
    InitFastMath();
    useZBuffer = 1;
    testKey = 0;
    fullScreen = 1;
    soundInterface = 0;
    network = 0;
    display = 0;
    renderTarget = 0;
    controlInterface = 0;
    childRoot = 0;
    rootObject = 0;
    textureManager = 0;
    playerCount = 1;
    bladeRenderer = 0;
    debugOverlay = 0;
    s_UnknownFile65b4ac = 0;
    strcpy(field_0x40, "");
    field_0x2dc = 0;
    field_0x2d8 = 0;
    replaySpeed = 1.0f;
    allowDither = 1;
    allowMipMapping = 1;
    allowBilinear = 1;
    allowSortIndependentAntialiasing = 0;
    allowTrilinear = 0;
    paused = 0;
    stepOneFrame = 0;
    inputSuspended = 0;
    shuttingDown = 0;
    texturesCached = 1;
    field_0x2e4 = 1.0f;
    field_0x2e8 = 0;
    field_0x2ec = 0.05f;
    frameTime = 0;
    DecodeString(s_UnknownEncoded56b334);
    DecodeString(s_UnknownEncoded56b350);
}

// 0x00467ae0 (identical code for slots 5 and 6)
int Game::Startup() {
    return 1;
}

int Game::OnActivate() {
    return 1;
}

int Game::OnDeactivate() {
    return 1;
}

// 0x00467af0: creates the PC control interface and sets it up (slot 1).
int Game::Initialize() {
    controlInterface = new(__FILE__, 187) PCControlInterface;
    return controlInterface->CreateDevices() != 0;
}

// 0x00467b70: builds the two root objects (the second a child of the
// first), the sound interface and the random seed, reads the "TestKey"
// value under HKEY_LOCAL_MACHINE\\SOFTWARE\\Rainbow Studios into bit 2 of
// +0x2d4, then (when slot 32 allows) the texture manager, the debug overlay
// (only with that bit) and the "AllowFreezeCamera" setting.
int Game::CreateObjects(char*) {
    rootObject = new(__FILE__, 222) GameObject(1);
    childRoot = new(__FILE__, 227) GameObject(1);
    rootObject->AppendChild(childRoot, -1);
    soundInterface = new(__FILE__, 235) PCSoundInterface;
    srand(ReadClock());
    int count = ReadClock() & 0xff;
    do
        rand();
    while (count--);
    testKey = 0;
    void* key;
    unsigned long type;
    unsigned long value;
    unsigned long size = sizeof(value);
    if (RegOpenKeyExA((void*)0x80000002, s_UnknownEncoded56b334, 0, 0x20019, &key) == 0 &&
        RegQueryValueExA(key, s_UnknownEncoded56b350, 0, &type, (unsigned char*)&value, &size) == 0 &&
        type == 4)                            // REG_DWORD
        testKey = value;
    RegCloseKey(key);
    if (!InitializeDisplay())
        return 0;
    textureManager = new(__FILE__, 282) TextureMapManager;
    childRoot->AppendChild(textureManager->SetRenderContext(renderTarget), -1);
    textureContext = textureManager;
    textureOpaqueGroup = 0;
    textureAlphaGroup = 0;
    textureOpaqueFormat = renderTarget->surfacePixelFormat;
    textureAlphaFormat = 0x115c;
    textureHalvings = 0;
    if (testKey) {
        int category = g_MemTagStack->Push("DebugOverlay");
        debugOverlay = (new(__FILE__, 305) DebugOverlay(0))->Setup(renderTarget, textureManager, 8, 3, 3);
        if (debugOverlay) {
            rootObject->AppendChild(debugOverlay, -1);
            if (g_HiResMeter->counterUsable) {
                rootObject->AppendChild(g_HiResMeter, -1);
                g_HiResMeter->Enable();
            }
        }
        g_MemTagStack->Pop(category);
    }
    allowFreezeCamera = GetRegistryFlag("AllowFreezeCamera", 0);
    if (!OpenResources())
        return 0;
    return CreateGameObjects() != 0;
}

// 0x00467e80: slot 31's result is kept at +0x10 and handed to the interface.
int Game::AttachRenderTarget() {
    renderTarget = CreateRenderTarget();
    if (!renderTarget)
        return 0;
    if (rootObject)
        rootObject->AttachRenderContext(renderTarget);
    return 1;
}

// 0x00467eb0: renders a frame, timing each phase, and with bit 2 of +0x2d4
// fills the debug overlay's profile and memory pages.
int Game::RenderFrame() {
    if (display->lastFlipFailed)
        display->RetryFlip(0);
    unsigned int last = ReadClock();
    rootObject->PrepareGeometry();
    unsigned int now = ReadClock();
    int prepareGeometry = now - last;
    last = now;
    if (display->lastFlipFailed)
        display->RetryFlip(1);
    now = ReadClock();
    int waitForFlip = now - last;
    last = now;
    rootObject->RenderPre3D();
    now = ReadClock();
    int renderPre3D = now - last;
    int render3D = 0;
    if (useZBuffer) {
        if (!renderTarget->BeginScene())
            goto failed;
        renderTarget->Clear(0, 0);
        SetRenderStates();
        last = ReadClock();
        rootObject->UnknownVirtualSlot14();
        render3D = ReadClock() - last;
        if (!renderTarget->EndScene()) {
        failed:
            return 0;
        }
    }
    last = ReadClock();
    rootObject->RenderPost3D();
    now = ReadClock();
    int renderPost3D = now - last;
    int elapsed = now - s_LastFrameTime;
    if (testKey) {
        if (debugOverlay) {
            static int profilePage = -1;
            if (profilePage < 0)
                profilePage = debugOverlay->NewPage();
            s_NetPeak.Record(g_NetTime);
            s_TickPeak.Record(g_TickTime);
            s_PrepFramePeak.Record(g_PrepFrameTime);
            s_UpdateScreenPeak.Record(g_UpdateScreenTime);
            s_PrepareGeometryPeak.Record(prepareGeometry);
            s_WaitForFlipPeak.Record(waitForFlip);
            s_RenderPre3DPeak.Record(renderPre3D);
            s_Render3DPeak.Record(render3D);
            s_RenderPost3DPeak.Record(renderPost3D);
            s_ElapsedPeak.Record(elapsed);
            if (debugOverlay->isEnabled && debugOverlay->shownPage == profilePage) {
                UnknownDisplayMode* mode = &display->displayModes[display->currentDisplayMode];
                debugOverlay->Title(profilePage, "%c %d x %d %d bit(x%d)", 'R',
                                                  mode->width, mode->height, mode->bitDepth,
                                                  display->frameBufferCount);
                debugOverlay->Print(profilePage, "%s", display->deviceIdentifier.description);
                debugOverlay->Print(profilePage, "ElapsedTime:   % 3d (%d)",
                                                  elapsed, s_ElapsedPeak.Peak());
                debugOverlay->Print(profilePage, "PrepFrameTime: % 3d (%d)",
                                                  g_PrepFrameTime,
                                                  s_PrepFramePeak.Peak());
                debugOverlay->Print(profilePage, "UpdateScrnTime:% 3d (%d)",
                                                  g_UpdateScreenTime,
                                                  s_UpdateScreenPeak.Peak());
                debugOverlay->Print(profilePage, "Net            % 3d (%d)",
                                                  g_NetTime, s_NetPeak.Peak());
                debugOverlay->Print(profilePage, "Tick           % 3d (%d)",
                                                  g_TickTime, s_TickPeak.Peak());
                debugOverlay->Print(profilePage, "PrepareGeometry% 3d (%d)",
                                                  prepareGeometry,
                                                  s_PrepareGeometryPeak.Peak());
                debugOverlay->Print(profilePage, "WaitForFlip    % 3d (%d)",
                                                  waitForFlip, s_WaitForFlipPeak.Peak());
                debugOverlay->Print(profilePage, "RenderPre3D    % 3d (%d)",
                                                  renderPre3D, s_RenderPre3DPeak.Peak());
                debugOverlay->Print(profilePage, "Render3D       % 3d (%d)",
                                                  render3D, s_Render3DPeak.Peak());
                debugOverlay->Print(profilePage, "   [this overlay %d]",
                                                  debugOverlay->drawTime);
                debugOverlay->Print(profilePage, "RenderPost3D   % 3d (%d)",
                                                  renderPost3D, s_RenderPost3DPeak.Peak());
                debugOverlay->Print(profilePage, "TotalTransforms% 5d", renderTarget->verticesDrawn);
                debugOverlay->Print(profilePage, "TotalPoints    % 5d", renderTarget->pointsDrawn);
                debugOverlay->Print(profilePage, "TotalLines     % 5d", renderTarget->linesDrawn);
                debugOverlay->Print(profilePage, "TotalTriangles % 5d", renderTarget->trianglesDrawn);
            }
            static int memoryPage = -1;
            if (memoryPage < 0)
                memoryPage = debugOverlay->NewPage();
            if (debugOverlay->isEnabled && debugOverlay->shownPage == memoryPage) {
                UnknownMemoryStatus status;
                GlobalMemoryStatus(&status);
                debugOverlay->Title(memoryPage, "Memory");
                debugOverlay->Print(memoryPage, "  Ours   in DirectX       Area");
                int count = g_MemTagStack->count;
                int ours = 0;
                int directx = 0;
                for (int i = 0; i < count; i++) {
                    debugOverlay->Print(memoryPage, "%8d %8d %12s",
                                                      g_MemTagStack->ours[i],
                                                      g_MemTagStack->directx[i],
                                                      g_MemTagStack->names[i]);
                    ours += g_MemTagStack->ours[i];
                    directx += g_MemTagStack->directx[i];
                }
                debugOverlay->Print(memoryPage, "%8d %8d %12s", ours, directx, "Totals");
                debugOverlay->Print(memoryPage, "Grand Total    %10d", ours + directx);
                debugOverlay->Print(memoryPage, "TotalPhys       %10d", status.totalPhys);
                debugOverlay->Print(memoryPage, "AvailPhys       %10d", status.availPhys);
                debugOverlay->Print(memoryPage, "TotalVirtual    %10d", status.totalVirtual);
                debugOverlay->Print(memoryPage, "AvailVirtual    %10d", status.availVirtual);
                debugOverlay->Print(memoryPage, "Memory Load     %8d %%", status.memoryLoad);
                debugOverlay->Print(memoryPage, "%s", display->deviceIdentifier.description);
                debugOverlay->Print(memoryPage, "Total VidMem    %10d", display->totalVideoMemory);
                debugOverlay->Print(memoryPage, "IsAGP           %s",
                                                  display->isAGP ? "TRUE" : "FALSE");
                debugOverlay->Print(memoryPage, "VideoMemoryMB   %d",
                                                  GetRegistryInt("VideoMemoryMB", -1));
                debugOverlay->Print(memoryPage, "PartialTexBlt   %s",
                                                  g_TrackGame->display->partialTextureUploadResult ? "Yes" : "No");
                debugOverlay->Print(memoryPage, "TexturesCached  %s",
                                                  g_TrackGame->texturesCached ? "Yes" : "No");
            }
        }
        s_LastFrameTime = now;
    }
    return 1;
}

// 0x004685c0
int Game::PresentFrame() {
    display->Present();
    return 1;
}

// 0x00468880
void Game::ResetClock() {
    field_0x2d8 = field_0x2dc = ReadClock();
}

// 0x004688a0
int Game::SetInputSuspended(int value) {
    if (network)
        return 0;
    inputSuspended = value;
    rootObject->SetPaused(value);
    return 1;
}

// 0x004688e0
int Game::UnknownVirtualSlot12(int value) {
    return rootObject->UnknownVirtualSlot19(value) != 0;
}

// 0x00468900: releases go to the interface unless inputSuspended is set.
int Game::OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (inputSuspended)
        return 1;
    return rootObject->OnControlRelease(event, entry) != 0;
}

// 0x00468930: presses go to the interface unless inputSuspended is set.
// Control 0x20 also drives the +0x38 object; with bit 0 of +0x2d4, Ctrl+F
// (key 0x21 with modifier 0xc) toggles +0x1c4 and E (0x12) clears it,
// setting +0x1c8.
int Game::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (inputSuspended)
        return 1;
    if (testKey && debugOverlay && TestInputEvent(0x20, 0, event, 0x80)) {
        if (debugOverlay->isEnabled) {
            if (debugOverlay->ShowNextPage())
                debugOverlay->Disable();
        } else {
            debugOverlay->Enable();
        }
    }
    if (allowFreezeCamera) {
        if (controlInterface->IsDeviceButtonDown(0x21, 0, 0xc, 0)) {
            paused = 1 - paused;
        } else if (controlInterface->IsDeviceButtonDown(0x12, 0, 0x3f, 0) && paused) {
            stepOneFrame = 1;
            paused = 0;
        }
    }
    return rootObject->OnControlPress(event, entry) != 0;
}

// 0x00468a10
Game::~Game() {
    if (s_UnknownFile65b4ac)
        fclose(s_UnknownFile65b4ac);
}

// 0x00468a30: tears down the interface, the network object, the owners and
// the ControlInterface.
int Game::Shutdown() {
    shuttingDown = 1;
    if (g_HiResMeter && debugOverlay && g_HiResMeter->counterUsable)
        g_HiResMeter->Unlink();
    g_UnknownStatic65b478.DeleteFonts();
    if (rootObject) {
        rootObject->Release();
        rootObject = 0;
    }
    if (network) {
        delete network;
        network = 0;
    }
    if (soundInterface) {
        delete soundInterface;
        soundInterface = 0;
    }
    if (controlInterface) {
        delete controlInterface;
        controlInterface = 0;
    }
    if (renderTarget) {
        delete renderTarget;
        renderTarget = 0;
    }
    display = 0;
    DeleteDisplays();
    return 1;
}

// 0x00468ae0
int Game::CreateNetworkInterface(int value) {
    network = new(__FILE__, 979) NetworkInterface;
    if (network && network->Initialize(value) < 0) {
        delete network;
        network = 0;
        return 0;
    }
    return network != 0;
}

// 0x00468ba0: hands a network message to the +0x2f4 object.
int Game::OnNetworkMessage(int type, void* data, int from, int to, int flags) {
    return rootObject->OnNetworkMessage(type, data, from, to, flags) != 0;
}

// 0x00468bd0: `path` = workingDirectory (+0x1cc), "\\" and `name`.
int Game::FindDataFile(const char* name, char* path) {
    strcpy(path, workingDirectory);
    strcat(path, "\\");
    strcat(path, name);
    return 1;
}

// 0x00468c60
int Game::GetStringResource(int, char* text) {
    strcpy(text, "No Strings Available");
    return 0;
}

// 0x00468c90 (identical code for slot 4)
int Game::OpenResources() {
    return 1;
}

int Game::CreateGameObjects() {
    return 1;
}
