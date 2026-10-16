#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "TrackGame.h"

#include "DebugAlloc.h"
#include "KeyboardDevice.h"
#include "SoundInterface.h"
#include "MenuId.h"

// KERNEL32, SHELL32 and USER32 imports.
extern "C" __declspec(dllimport) int __stdcall SetLocaleInfoA(unsigned long locale,
                                                             unsigned long type,
                                                             const char* data);
extern "C" __declspec(dllimport) int __stdcall GetLocaleInfoA(unsigned long locale,
                                                             unsigned long type, char* data,
                                                             int size);
extern "C" __declspec(dllimport) void* __stdcall LoadLibraryA(const char* name);
extern "C" __declspec(dllimport) int __stdcall FreeLibrary(void* module);
extern "C" __declspec(dllimport) void* __stdcall ShellExecuteA(void* window, const char* operation,
                                                              const char* file,
                                                              const char* parameters,
                                                              const char* directory, int show);
extern "C" __declspec(dllimport) int __stdcall SystemParametersInfoA(unsigned int action,
                                                                    unsigned int param,
                                                                    void* data,
                                                                    unsigned int flags);

extern "C" __declspec(dllimport) int __stdcall LoadStringA(void* instance, unsigned int id,
                                                          char* buffer, int size);

extern "C" __declspec(dllimport) int __stdcall ShowCursor(int show);
extern "C" __declspec(dllimport) int __stdcall MessageBoxA(void* window, const char* text,
                                                          const char* caption, unsigned int type);
extern "C" __declspec(dllimport) void* __stdcall GetProcAddress(void* module, const char* name);
extern "C" __declspec(dllimport) long __stdcall SendMessageA(void* window, unsigned int message,
                                                            unsigned int wParam, long lParam);
extern "C" __declspec(dllimport) int __stdcall PostMessageA(void* window, unsigned int message,
                                                           unsigned int wParam, long lParam);

// KERNEL32 MEMORYSTATUS.
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

// EBUEula.dll's entry point: shows the EULA for the registry key; nonzero
// when accepted.
typedef int (*UnknownEulaProc)(const char* key, const char* path, int a, int b);

// IMM32, called through the linker's import thunks.
extern "C" void* __stdcall ImmGetContext(void* window);
extern "C" int __stdcall ImmGetOpenStatus(void* context);
extern "C" int __stdcall ImmReleaseContext(void* window, void* context);

enum {
    kLocaleUserDefault = 0x400,
    kLocaleDecimalSeparator = 0xe,
    kWindowsNtPlatform = 2,
    kGetScreenSaverActive = 0x10,
    kSetScreenSaverActive = 0x11,
    kSendSystemCommand = 0x112,
    kScreenSaverCommand = 0xf020,
    kCloseWindow = 0x10,
    kSoundDeviceAlreadyAllocated = 0x8878000a
};

DebugSocket* g_DebugSocket;

// 0x00520820: the function right before this file's first (0x00520870),
// after TrackRecord.cpp's last literal reference (0x0052022d); ownership by
// position only. Formats `format` into 0x200 bytes and sends the text to
// the debug socket when one is open.
void SendDebugMessage(const char* format, ...)
{
    if (g_DebugSocket) {
        char buffer[0x200];
        va_list args;
        va_start(args, format);
        _vsnprintf(buffer, 0x200, format, args);
        g_DebugSocket->SendText(buffer);
    }
}

// 0x00520870
TrackGame::TrackGame() {
    sceneObject = 0;
    supercrossEvent = 0;
    quarryEvent = 0;
    bajaEnduroEvent = 0;
    nationalEvent = 0;
    tagEvent = 0;
    ui = 0;
    eventManager = 0;
    textureManager = 0;
    profileDirectory = 0;
    menuIsOpen = 0;
    uiInteractionBlocked = 0;
    cdAudio = 0;
    controlMapping = 0;
    controlSetup = 0;
    highScores = 0;
    languageModule = LoadLibraryA("lang.dll");
    if (languageModule)
        resourceInstance = languageModule;
    GetLocaleInfoA(kLocaleUserDefault, kLocaleDecimalSeparator, savedDecimalSeparator,
                   sizeof(savedDecimalSeparator));
    SetLocaleInfoA(kLocaleUserDefault, kLocaleDecimalSeparator, ".");
    const char* key = "Software\\Microsoft\\Microsoft Games\\Motocross Madness 2 Trial";
    int count = strlen(key);
    int length = count > 0x7f ? 0x7f : count;
    strncpy(registryKey, key, length);
    registryKey[length] = 0;
    fullRecordPacketIntervalSeconds =
        GetRegistryInt("IntervalBetweenFullRecordPacketsMS", 500) * 0.001f;
    shortRecordPacketIntervalSeconds =
        GetRegistryInt("IntervalBetweenShortRecordPacketsMS", 100) * 0.001f;
    fullNetPacketIntervalSeconds =
        GetRegistryInt("IntervalBetweenFullNetPacketsMS", 2000) * 0.001f;
    shortNetPacketIntervalSeconds =
        GetRegistryInt("IntervalBetweenShortNetPacketsMS", 67) * 0.001f;
    networkAiRacerCount = 0;
    replaySpeed = 1.0f;
    playingReplay = 0;
    useReplayTape = 0;
    proCircuit = 0;
    openLocalizedWebPageOnExit = 0;
    openStorePageOnExit = 0;
    g_DebugSocket = 0;
}

// 0x00521ae0: opens the requested web pages, deletes the owned objects,
// releases lang.dll and (on Windows NT) re-enables the screen saver.
TrackGame::~TrackGame() {
    if (openStorePageOnExit)
        ShellExecuteA(0, 0,
                      "http://shop.microsoft.com/store/referral/selector.asp?siteid=10425&furl=2&"
                      "sku=773%2d00042&skuqty=1",
                      0, 0, 3);
    if (openLocalizedWebPageOnExit) {
        char url[128];
        LoadResourceString(0x14df, url, 0x7f);
        ShellExecuteA(0, 0, url, 0, 0, 3);
    }
    delete cdAudio;
    delete profileDirectory;
    delete controlMapping;
    delete controlSetup;
    delete highScores;
    if (languageModule) {
        FreeLibrary(languageModule);
        languageModule = 0;
    }
    delete proCircuit;
    delete g_DebugSocket;
    if (osVersion.platformId == kWindowsNtPlatform && screenSaverWasActive)
        SystemParametersInfoA(kSetScreenSaverActive, 1, 0, 2);
}

// 0x00520ab0: start-up checks: the first +0x578 check (error 0x13b3), the
// second, retried until it passes or is cancelled (0x13b5), a warning below
// 64 MB of available memory (0x14bb), then the EULA through EBUEula.dll.
// On NT it also turns the screen saver off, remembering the setting.
int TrackGame::Startup() {
    UnknownMemoryStatus status;
    char text[128];
    if (!mode.FindDataDirectory() &&
        LoadStringA(resourceInstance, 0x13b3, text, sizeof(text))) {
        ShowCursor(1);
        MessageBoxA(0, text, applicationName, 0x10);
        return 0;
    }
    while (!mode.FindCdDirectory()) {
        if (LoadStringA(resourceInstance, 0x13b5, text, sizeof(text))) {
            ShowCursor(1);
            if (MessageBoxA(0, text, applicationName, 0x15) == 2)
                return 0;
        }
    }
    GlobalMemoryStatus(&status);
    if (status.availPhys + status.availPageFile < 0x4000000 &&
        LoadStringA(resourceInstance, 0x14bb, text, sizeof(text))) {
        ShowCursor(1);
        int answer = MessageBoxA(0, text, applicationName, 0x23);
        if (answer == 2 || answer == 7)
            return 0;
    }
    if (osVersion.platformId == kWindowsNtPlatform) {
        SystemParametersInfoA(kGetScreenSaverActive, 0, &screenSaverWasActive, 0);
        if (screenSaverWasActive)
            SystemParametersInfoA(kSetScreenSaverActive, 0, 0, 2);
    }
    void* library = LoadLibraryA("EBUEula.dll");
    if (!library)
        return 0;
    UnknownEulaProc eula = (UnknownEulaProc)GetProcAddress(library, "EBUEula");
    if (!eula) {
        FreeLibrary(library);
        return 0;
    }
    char name[260];
    char path[260];
    int count = strlen("EULA.rtf");
    int length = count > 0x103 ? 0x103 : count;
    strncpy(name, "EULA.rtf", length);
    name[length] = 0;
    if (!FindDataFile(name, path))
        return 0;
    int accepted = eula(registryKey, path, 0, 1);
    FreeLibrary(library);
    if (!accepted)
        return 0;
    return Game::Startup();
}

// Resolves Res\\<file> through FindDataFile (slot 18) and adds it to the resource manager;
// fails the caller when the path cannot be resolved.
#define ADD_RESOURCE_ARCHIVE(file)                                    \
    sprintf(name, "%s\\%s", "Res", file);                          \
    if (!FindDataFile(name, path))                                  \
        return 0;                                                    \
    g_UnknownResourceManager572b44->AddArchive(path, 0);

// 0x00520d10
int TrackGame::OpenResources() {
    char name[260];
    char path[260];
    ADD_RESOURCE_ARCHIVE("Bike.res")
    ADD_RESOURCE_ARCHIVE("Objects.res")
    ADD_RESOURCE_ARCHIVE("Models.res")
    ADD_RESOURCE_ARCHIVE("ProcVUE.res")
    ADD_RESOURCE_ARCHIVE("ColSeg.res")
    ADD_RESOURCE_ARCHIVE("Audio.res")
    ADD_RESOURCE_ARCHIVE("Engine.res")
    ADD_RESOURCE_ARCHIVE("Skies.res")
    ADD_RESOURCE_ARCHIVE("Global.res")
    ADD_RESOURCE_ARCHIVE("Eco.res")
    return 1;
}

// 0x00521050: creates the scene, profile list, record, audio (giving up
// with a message when the sound device is unavailable), collision, input
// mapping, KrustyUI and EventManager objects, and for network games the
// debug connection and the race set-up.
int TrackGame::CreateGameObjects() {
    int category = g_MemTagStack->Push("Scene");
    sceneObject = new(__FILE__, 305) UnknownTrackGameObject574;
    g_MemTagStack->Push("UI");
    profileDirectory = new(__FILE__, 309) DirectoryList;
    profileDirectory->SetDirectory("ui\\profile");
    profileDirectory->SetPattern("*.*", 1);
    profileDirectory->UnknownVirtualSlot1();
    highScores = new(__FILE__, 313) UnknownTrackGameObject3400;
    mode.CreateDirectoryLists();
    mode.ResetNetworkRace();
    g_MemTagStack->Push("Audio");
    if (((PCSoundInterface*)soundInterface)->InitializeSound(22050, 1, mode.use16BitSound ? 16 : 8,
                                                              4000000, mode.eaxEnabled) ==
        (long)kSoundDeviceAlreadyAllocated) {
        char text[256];
        SendMessageA(windowHandle, kSendSystemCommand, kScreenSaverCommand, 0);
        ShowCursor(1);
        LoadStringA(resourceInstance, 0x13d3, text, sizeof(text));
        MessageBoxA(windowHandle, text, applicationName, 0x10);
        PostMessageA(windowHandle, kCloseWindow, 0, 0);
        return 1;
    }
    cdAudio = new(__FILE__, 343) CDAudio;
    cdAudio->Initialize();
    g_MemTagStack->Push("Startup");
    controlMapping = new(__FILE__, 350) UnknownControlMapping;
    if (!controlMapping)
        return 0;
    controlInterface->SetMapping(controlMapping);
    controlSetup = new(__FILE__, 360) UnknownTrackGameObject33fc;
    if (!controlSetup)
        return 0;
    g_MemTagStack->Push("UI");
    ui = new(__FILE__, 379) KrustyUI(1);
    if (!childRoot->AppendChild(ui->Create(renderTarget, 1), -1))
        return 0;
    eventManager = new(__FILE__, 382) EventManager(1);
    if (!childRoot->AppendChild(eventManager->SetRenderContext(renderTarget), -1))
        return 0;
    if (network) {
        char address[32];
        unsigned long size = sizeof(address);
        GetRegistryString("debugIP", "", address, &size);
        int port = GetRegistryInt("debugPort", 2001);
        if (address[0]) {
            g_DebugSocket = new(__FILE__, 395) DebugSocket;
            if (g_DebugSocket) {
                if (g_DebugSocket->Connect(address, port) == 1) {
                    delete g_DebugSocket;
                    g_DebugSocket = 0;
                }
                SendDebugMessage("Connected! to MCM2\n");
            }
        }
        if (network->lobbyConnected) {
            ui->gui->ShowCursors(1);
            ui->BuildGarageScene(-1);
            char name[16];
            network->GetPlayerName(network->localPlayer, name);
            if (strcmp(mode.profileName, name)) {
                int count = strlen(name);
                int length = count > 15 ? 15 : count;
                strncpy(mode.profileName, name, length);
                mode.profileName[length] = 0;
                if (!mode.LoadProfile())
                    ui->OpenMenu(kMenuUserNameBbb);
            }
            networkGameObject = new(__FILE__, 430) UnknownTrackGameObject3410;
            if (networkGameObject)
                networkGameObject->SetDirectPlay(network->directPlay, network->lobby);
            ui->OpenMenu(kMenuMultiPlayer866);
        }
    }
    g_MemTagStack->Push("Audio");
    MuteSound();
    g_MemTagStack->Pop(category);
    return 1;
}

// 0x00520d00
int TrackGame::Initialize() {
    return PCGame::Initialize();
}

// 0x00521660
int TrackGame::Tick() {
    return Game::Tick() != 0;
}

// 0x00521840
int TrackGame::OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry) {
    return PCGame::OnControlRelease(event, entry);
}

// 0x00521670: input presses. With an IME open nothing happens. Control 0x1d
// with modifier 0x45 or control 0x3d/0xc5 toggles menu 0x190, and control 1
// opens menu 0x191 (in-game).
int TrackGame::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind == 0) {
        void* context = ImmGetContext(windowHandle);
        if (context) {
            int closed;
            if (imeLibrary && ImmGetOpenStatus(context))
                closed = 0;
            else
                closed = 1;
            ImmReleaseContext(windowHandle, context);
            if (!closed)
                return 0;
        }
        if (event->kind == 0 && event->control == 0x1d &&
            controlInterface->keyboard->IsButtonDown(0x45, 0x3f, 0) && ui) {
            if (network)
                goto handled;
            goto toggle;
        }
    }
    {
        int previous = paused;
        if (PCGame::OnControlPress(event, entry))
            return 0;
        if (allowFreezeCamera && paused != previous) {
            if (paused)
                childRoot->DisableByClassName("RaceSound");
            else
                childRoot->EnableByClassName("RaceSound");
        }
    }
    if (!ui)
        return 0;
    if (TestInputEvent(1, 0, event, 0x3f)) {
        g_MemTagStack->ReportTotals("In Game");
        if (!menuIsOpen) {
            int category = g_MemTagStack->Push("UI");
            SetMenuOpen(1, kMenuExit, 1);
            g_MemTagStack->Pop(category);
        }
        return 1;
    }
    if (!network && (TestInputEvent(0x3d, 0, event, 0x80000000) ||
                        TestInputEvent(0xc5, 0, event, 0x80000000))) {
toggle:
        if (menuIsOpen)
            SetMenuOpen(0, kMenuContinue, 1);
        else
            SetMenuOpen(1, kMenuContinue, 1);
handled:
        return 1;
    }
    return 0;
}

// 0x00521860
void TrackGame::SetMenuOpen(int open, int id, int sound) {
    GameObject* menu = eventManager->FindRaceMode();
    if (!ui || !menu || uiInteractionBlocked)
        return;
    if (open) {
        GameObject* other = eventManager->FindRaceView();
        if (other && !other->isEnabled)
            return;
        if (menuIsOpen)
            return;
        ui->OpenMenu(id);
        if (!menu->isEnabled)
            return;
        menu->Disable();
        menuIsOpen = 1;
        childRoot->DisableByClassName("RaceSound");
    } else {
        if (!menuIsOpen)
            return;
        if (!ui->gui->FindInputDialog())
            return;
        if (id != ui->currentMenu)
            return;
        ui->gui->FindInputDialog()->UnknownVirtualSlot26();
        GameObject* current = eventManager->FindRaceMode();
        if (current)
            current->Enable();
        menuIsOpen = 0;
        if (sound)
            childRoot->EnableByClassName("RaceSound");
    }
}

// 0x00521970
int TrackGame::LoadResourceString(int id, char* buffer, int size) {
    if (!resourceInstance) {
        strcpy(buffer, "Resource String Unavailable");
        return 0;
    }
    if (!LoadStringA(resourceInstance, id, buffer, size)) {
        char message[128];
        sprintf(message, "Resource string '%d' load fail\n", id);
        strcpy(buffer, "Resource String Unavailable");
        return 0;
    }
    return 1;
}

// 0x00521a30
void TrackGame::MuteSound() {
    ((PCSoundInterface*)soundInterface)->SetPrimaryVolume(0);
}

// 0x00521a40
void TrackGame::StopCDAudio() {
    if (cdAudio)
        cdAudio->Stop();
}

// 0x00521a50
int TrackGame::OnActivate() {
    PCGame::OnActivate();
    if (quarryEvent)
        ResetClock();
    return 1;
}

// 0x00521a70: deletes the +0x574 object, shuts down, then restores the
// decimal separator saved at +0x3404.
int TrackGame::Shutdown() {
    if (sceneObject) {
        delete sceneObject;
        sceneObject = 0;
    }
    if (!PCGame::Shutdown())
        return 0;
    ui = 0;
    SetLocaleInfoA(kLocaleUserDefault, kLocaleDecimalSeparator, savedDecimalSeparator);
    mode.SaveProfile();
    return 1;
}

// 0x00521cb0
int TrackGame::FindDataFile(const char* name, char* path) {
    return mode.FindDataFile(name, path);
}

// 0x00521cd0
int TrackGame::InProCircuitCareer() {
    if (proCircuit)
        return ui->playingDemo == 0;
    return 0;
}
