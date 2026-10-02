#include <stdio.h>
#include <string.h>

#include "TrackGame.h"

#include "DebugAlloc.h"
#include "KeyboardDevice.h"
#include "SoundInterface.h"

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

UnknownTrackGameGlobal68a48c* g_UnknownGlobal68a48c;

// 0x00520870
TrackGame::TrackGame() {
    sceneObject = 0;
    field_0x558 = 0;
    field_0x55c = 0;
    field_0x560 = 0;
    field_0x564 = 0;
    field_0x568 = 0;
    ui = 0;
    eventManager = 0;
    field_0x3c = 0;
    profileDirectory = 0;
    menuIsOpen = 0;
    uiInteractionBlocked = 0;
    field_0x3340 = 0;
    controlMapping = 0;
    field_0x33fc = 0;
    field_0x3400 = 0;
    languageModule = LoadLibraryA("lang.dll");
    if (languageModule)
        field_0x420 = languageModule;
    GetLocaleInfoA(kLocaleUserDefault, kLocaleDecimalSeparator, savedDecimalSeparator,
                   sizeof(savedDecimalSeparator));
    SetLocaleInfoA(kLocaleUserDefault, kLocaleDecimalSeparator, ".");
    const char* key = "Software\\Microsoft\\Microsoft Games\\Motocross Madness 2 Trial";
    int count = strlen(key);
    int length = count > 0x7f ? 0x7f : count;
    strncpy(field_0x4b8, key, length);
    field_0x4b8[length] = 0;
    fullRecordPacketIntervalSeconds =
        UnknownVirtualSlot20("IntervalBetweenFullRecordPacketsMS", 500) * 0.001f;
    shortRecordPacketIntervalSeconds =
        UnknownVirtualSlot20("IntervalBetweenShortRecordPacketsMS", 100) * 0.001f;
    fullNetPacketIntervalSeconds =
        UnknownVirtualSlot20("IntervalBetweenFullNetPacketsMS", 2000) * 0.001f;
    shortNetPacketIntervalSeconds =
        UnknownVirtualSlot20("IntervalBetweenShortNetPacketsMS", 67) * 0.001f;
    field_0x3424 = 0;
    field_0x2e0 = 1.0f;
    field_0x3428 = 0;
    field_0x342c = 0;
    field_0x3444 = 0;
    openLocalizedWebPageOnExit = 0;
    openStorePageOnExit = 0;
    g_UnknownGlobal68a48c = 0;
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
        UnknownFunction521970(0x14df, url, 0x7f);
        ShellExecuteA(0, 0, url, 0, 0, 3);
    }
    delete field_0x3340;
    delete profileDirectory;
    delete controlMapping;
    delete field_0x33fc;
    delete field_0x3400;
    if (languageModule) {
        FreeLibrary(languageModule);
        languageModule = 0;
    }
    delete field_0x3444;
    delete g_UnknownGlobal68a48c;
    if (field_0x424.platformId == kWindowsNtPlatform && screenSaverWasActive)
        SystemParametersInfoA(kSetScreenSaverActive, 1, 0, 2);
}

// 0x00520ab0: start-up checks: the first +0x578 check (error 0x13b3), the
// second, retried until it passes or is cancelled (0x13b5), a warning below
// 64 MB of available memory (0x14bb), then the EULA through EBUEula.dll.
// On NT it also turns the screen saver off, remembering the setting.
int TrackGame::UnknownVirtualSlot1() {
    UnknownMemoryStatus status;
    char text[128];
    if (!mode.UnknownFunction523c90() &&
        LoadStringA(field_0x420, 0x13b3, text, sizeof(text))) {
        ShowCursor(1);
        MessageBoxA(0, text, field_0x3a0, 0x10);
        return 0;
    }
    while (!mode.UnknownFunction523bf0()) {
        if (LoadStringA(field_0x420, 0x13b5, text, sizeof(text))) {
            ShowCursor(1);
            if (MessageBoxA(0, text, field_0x3a0, 0x15) == 2)
                return 0;
        }
    }
    GlobalMemoryStatus(&status);
    if (status.availPhys + status.availPageFile < 0x4000000 &&
        LoadStringA(field_0x420, 0x14bb, text, sizeof(text))) {
        ShowCursor(1);
        int answer = MessageBoxA(0, text, field_0x3a0, 0x23);
        if (answer == 2 || answer == 7)
            return 0;
    }
    if (field_0x424.platformId == kWindowsNtPlatform) {
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
    if (!UnknownVirtualSlot18(name, path))
        return 0;
    int accepted = eula(field_0x4b8, path, 0, 1);
    FreeLibrary(library);
    if (!accepted)
        return 0;
    return Game::UnknownVirtualSlot1();
}

// Resolves Res\\<file> through slot 18 and adds it to the resource manager;
// fails the caller when the path cannot be resolved.
#define ADD_RESOURCE_ARCHIVE(file)                                    \
    sprintf(name, "%s\\%s", "Res", file);                          \
    if (!UnknownVirtualSlot18(name, path))                           \
        return 0;                                                    \
    g_UnknownResourceManager572b44->UnknownFunction4e9030(path, 0);

// 0x00520d10
int TrackGame::UnknownVirtualSlot3() {
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
int TrackGame::UnknownVirtualSlot4() {
    int category = g_MemTagStack->Push("Scene");
    sceneObject = new(__FILE__, 305) UnknownTrackGameObject574;
    g_MemTagStack->Push("UI");
    profileDirectory = new(__FILE__, 309) DirectoryList;
    profileDirectory->UnknownFunction44a1d0("ui\\profile");
    profileDirectory->UnknownFunction44a220("*.*", 1);
    profileDirectory->UnknownVirtualSlot1();
    field_0x3400 = new(__FILE__, 313) UnknownTrackGameObject3400;
    mode.UnknownFunction522d00();
    mode.UnknownFunction522680();
    g_MemTagStack->Push("Audio");
    if (((PCSoundInterface*)field_0x04)->UnknownFunction4be5a0(22050, 1, mode.field_0xa48 ? 16 : 8,
                                                              4000000, mode.field_0xa34) ==
        (long)kSoundDeviceAlreadyAllocated) {
        char text[256];
        SendMessageA(field_0x31c, kSendSystemCommand, kScreenSaverCommand, 0);
        ShowCursor(1);
        LoadStringA(field_0x420, 0x13d3, text, sizeof(text));
        MessageBoxA(field_0x31c, text, field_0x3a0, 0x10);
        PostMessageA(field_0x31c, kCloseWindow, 0, 0);
        return 1;
    }
    field_0x3340 = new(__FILE__, 343) UnknownTrackGameObject3340;
    field_0x3340->UnknownFunction4310c0();
    g_MemTagStack->Push("Startup");
    controlMapping = new(__FILE__, 350) UnknownControlMapping;
    if (!controlMapping)
        return 0;
    field_0x14->UnknownFunction43ce70(controlMapping);
    field_0x33fc = new(__FILE__, 360) UnknownTrackGameObject33fc;
    if (!field_0x33fc)
        return 0;
    g_MemTagStack->Push("UI");
    ui = new(__FILE__, 379) KrustyUI(1);
    if (!field_0x34->UnknownFunction469190(ui->UnknownFunction4988a0(field_0x10, 1), -1))
        return 0;
    eventManager = new(__FILE__, 382) EventManager(1);
    if (!field_0x34->UnknownFunction469190(eventManager->UnknownVirtualSlot8(field_0x10), -1))
        return 0;
    if (field_0x08) {
        char address[32];
        unsigned long size = sizeof(address);
        UnknownVirtualSlot23("debugIP", "", address, &size);
        int port = UnknownVirtualSlot20("debugPort", 2001);
        if (address[0]) {
            g_UnknownGlobal68a48c = new(__FILE__, 395) UnknownTrackGameGlobal68a48c;
            if (g_UnknownGlobal68a48c) {
                if (g_UnknownGlobal68a48c->UnknownFunction4ad3e0(address, port) == 1) {
                    delete g_UnknownGlobal68a48c;
                    g_UnknownGlobal68a48c = 0;
                }
                UnknownFunction520820("Connected! to MCM2\n");
            }
        }
        if (field_0x08->field_0x14) {
            ui->field_0x2c->UnknownFunction486630(1);
            ui->UnknownFunction498cf0(-1);
            char name[16];
            field_0x08->UnknownFunction4ac720(field_0x08->field_0x0c, name);
            if (strcmp(mode.field_0x00, name)) {
                int count = strlen(name);
                int length = count > 15 ? 15 : count;
                strncpy(mode.field_0x00, name, length);
                mode.field_0x00[length] = 0;
                if (!mode.UnknownFunction5231f0())
                    ui->UnknownFunction499b20(0xbbb);
            }
            networkGameObject = new(__FILE__, 430) UnknownTrackGameObject3410;
            if (networkGameObject)
                networkGameObject->UnknownFunction4aa350(field_0x08->field_0x04, field_0x08->field_0x08);
            ui->UnknownFunction499b20(0x866);
        }
    }
    g_MemTagStack->Push("Audio");
    UnknownFunction521a30();
    g_MemTagStack->Pop(category);
    return 1;
}

// 0x00520d00
int TrackGame::UnknownVirtualSlot2() {
    return PCGame::UnknownVirtualSlot2();
}

// 0x00521660
int TrackGame::UnknownVirtualSlot10() {
    return Game::UnknownVirtualSlot10() != 0;
}

// 0x00521840
int TrackGame::UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry) {
    return PCGame::UnknownVirtualSlot13(event, entry);
}

// 0x00521670: input presses. With an IME open nothing happens. Control 0x1d
// with modifier 0x45 or control 0x3d/0xc5 toggles menu 0x190, and control 1
// opens menu 0x191 (in-game).
int TrackGame::UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind == 0) {
        void* context = ImmGetContext(field_0x31c);
        if (context) {
            int closed;
            if (field_0x538 && ImmGetOpenStatus(context))
                closed = 0;
            else
                closed = 1;
            ImmReleaseContext(field_0x31c, context);
            if (!closed)
                return 0;
        }
        if (event->kind == 0 && event->control == 0x1d &&
            field_0x14->keyboard->UnknownVirtualSlot5(0x45, 0x3f, 0) && ui) {
            if (field_0x08)
                goto handled;
            goto toggle;
        }
    }
    {
        int previous = field_0x1c4;
        if (PCGame::UnknownVirtualSlot14(event, entry))
            return 0;
        if (field_0x2d4_bit0 && field_0x1c4 != previous) {
            if (field_0x1c4)
                field_0x34->UnknownFunction468dd0("RaceSound");
            else
                field_0x34->UnknownFunction468f10("RaceSound");
        }
    }
    if (!ui)
        return 0;
    if (UnknownFunction43caa0(1, 0, event, 0x3f)) {
        g_MemTagStack->UnknownFunction4a2bc0("In Game");
        if (!menuIsOpen) {
            int category = g_MemTagStack->Push("UI");
            UnknownFunction521860(1, 0x191, 1);
            g_MemTagStack->Pop(category);
        }
        return 1;
    }
    if (!field_0x08 && (UnknownFunction43caa0(0x3d, 0, event, 0x80000000) ||
                        UnknownFunction43caa0(0xc5, 0, event, 0x80000000))) {
toggle:
        if (menuIsOpen)
            UnknownFunction521860(0, 0x190, 1);
        else
            UnknownFunction521860(1, 0x190, 1);
handled:
        return 1;
    }
    return 0;
}

// 0x00521860
void TrackGame::UnknownFunction521860(int open, int id, int sound) {
    GameObject* menu = eventManager->UnknownFunction45d2b0();
    if (!ui || !menu || uiInteractionBlocked)
        return;
    if (open) {
        GameObject* other = eventManager->UnknownFunction45d2f0();
        if (other && !other->field_0x25_bit0)
            return;
        if (menuIsOpen)
            return;
        ui->UnknownFunction499b20(id);
        if (!menu->field_0x25_bit0)
            return;
        menu->UnknownVirtualSlot4();
        menuIsOpen = 1;
        field_0x34->UnknownFunction468dd0("RaceSound");
    } else {
        if (!menuIsOpen)
            return;
        if (!ui->field_0x2c->UnknownFunction485df0())
            return;
        if (id != ui->field_0x3c)
            return;
        ui->field_0x2c->UnknownFunction485df0()->UnknownVirtualSlot26();
        GameObject* current = eventManager->UnknownFunction45d2b0();
        if (current)
            current->UnknownVirtualSlot5();
        menuIsOpen = 0;
        if (sound)
            field_0x34->UnknownFunction468f10("RaceSound");
    }
}

// 0x00521970
int TrackGame::UnknownFunction521970(int id, char* buffer, int size) {
    if (!field_0x420) {
        strcpy(buffer, "Resource String Unavailable");
        return 0;
    }
    if (!LoadStringA(field_0x420, id, buffer, size)) {
        char message[128];
        sprintf(message, "Resource string '%d' load fail\n", id);
        strcpy(buffer, "Resource String Unavailable");
        return 0;
    }
    return 1;
}

// 0x00521a30
void TrackGame::UnknownFunction521a30() {
    ((PCSoundInterface*)field_0x04)->UnknownFunction4be9b0(0);
}

// 0x00521a40
void TrackGame::UnknownFunction521a40() {
    if (field_0x3340)
        field_0x3340->UnknownFunction4310e0();
}

// 0x00521a50
int TrackGame::UnknownVirtualSlot5() {
    PCGame::UnknownVirtualSlot5();
    if (field_0x55c)
        UnknownFunction468880();
    return 1;
}

// 0x00521a70: deletes the +0x574 object, shuts down, then restores the
// decimal separator saved at +0x3404.
int TrackGame::UnknownVirtualSlot15() {
    if (sceneObject) {
        delete sceneObject;
        sceneObject = 0;
    }
    if (!PCGame::UnknownVirtualSlot15())
        return 0;
    ui = 0;
    SetLocaleInfoA(kLocaleUserDefault, kLocaleDecimalSeparator, savedDecimalSeparator);
    mode.UnknownFunction523580();
    return 1;
}

// 0x00521cb0
int TrackGame::UnknownVirtualSlot18(const char* name, char* path) {
    return mode.UnknownFunction5238f0(name, path);
}

// 0x00521cd0
int TrackGame::UnknownFunction521cd0() {
    if (field_0x3444)
        return ui->field_0x4a8 == 0;
    return 0;
}
