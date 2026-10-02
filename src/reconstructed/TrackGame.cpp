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

// IMM32, called through the linker's import thunks.
extern "C" void* __stdcall ImmGetContext(void* window);
extern "C" int __stdcall ImmGetOpenStatus(void* context);
extern "C" int __stdcall ImmReleaseContext(void* window, void* context);

#define UNKNOWN_LOCALE_USER_DEFAULT 0x400
#define UNKNOWN_LOCALE_SDECIMAL 0xe

UnknownTrackGameGlobal68a48c* g_UnknownGlobal68a48c;

// 0x00520870
TrackGame::TrackGame() {
    field_0x574 = 0;
    field_0x558 = 0;
    field_0x55c = 0;
    field_0x560 = 0;
    field_0x564 = 0;
    field_0x568 = 0;
    field_0x56c = 0;
    field_0x570 = 0;
    field_0x3c = 0;
    field_0x3338 = 0;
    field_0x333c = 0;
    field_0x3430 = 0;
    field_0x3340 = 0;
    field_0x33f8 = 0;
    field_0x33fc = 0;
    field_0x3400 = 0;
    field_0x340c = LoadLibraryA("lang.dll");
    if (field_0x340c)
        field_0x420 = field_0x340c;
    GetLocaleInfoA(UNKNOWN_LOCALE_USER_DEFAULT, UNKNOWN_LOCALE_SDECIMAL, field_0x3404,
                   sizeof(field_0x3404));
    SetLocaleInfoA(UNKNOWN_LOCALE_USER_DEFAULT, UNKNOWN_LOCALE_SDECIMAL, ".");
    const char* key = "Software\\Microsoft\\Microsoft Games\\Motocross Madness 2 Trial";
    int count = strlen(key);
    int length = count > 0x7f ? 0x7f : count;
    strncpy(field_0x4b8, key, length);
    field_0x4b8[length] = 0;
    field_0x341c = UnknownVirtualSlot20("IntervalBetweenFullRecordPacketsMS", 500) * 0.001f;
    field_0x3420 = UnknownVirtualSlot20("IntervalBetweenShortRecordPacketsMS", 100) * 0.001f;
    field_0x3414 = UnknownVirtualSlot20("IntervalBetweenFullNetPacketsMS", 2000) * 0.001f;
    field_0x3418 = UnknownVirtualSlot20("IntervalBetweenShortNetPacketsMS", 67) * 0.001f;
    field_0x3424 = 0;
    field_0x2e0 = 1.0f;
    field_0x3428 = 0;
    field_0x342c = 0;
    field_0x3444 = 0;
    field_0x343c = 0;
    field_0x3440 = 0;
    g_UnknownGlobal68a48c = 0;
}

// 0x00521ae0: opens the requested web pages, deletes the owned objects,
// releases lang.dll and (on Windows NT) re-enables the screen saver.
TrackGame::~TrackGame() {
    if (field_0x3440)
        ShellExecuteA(0, 0,
                      "http://shop.microsoft.com/store/referral/selector.asp?siteid=10425&furl=2&"
                      "sku=773%2d00042&skuqty=1",
                      0, 0, 3);
    if (field_0x343c) {
        char url[128];
        UnknownFunction521970(0x14df, url, 0x7f);
        ShellExecuteA(0, 0, url, 0, 0, 3);
    }
    delete field_0x3340;
    delete field_0x3338;
    delete field_0x33f8;
    delete field_0x33fc;
    delete field_0x3400;
    if (field_0x340c) {
        FreeLibrary(field_0x340c);
        field_0x340c = 0;
    }
    delete field_0x3444;
    delete g_UnknownGlobal68a48c;
    if (field_0x424.platformId == 2 && field_0x34c8)
        SystemParametersInfoA(0x11, 1, 0, 2);
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
            field_0x14->field_0x34->UnknownVirtualSlot5(0x45, 0x3f, 0) && field_0x56c) {
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
    if (!field_0x56c)
        return 0;
    if (UnknownFunction43caa0(1, 0, event, 0x3f)) {
        g_MemTagStack->UnknownFunction4a2bc0("In Game");
        if (!field_0x333c) {
            int category = g_MemTagStack->Push("UI");
            UnknownFunction521860(1, 0x191, 1);
            g_MemTagStack->Pop(category);
        }
        return 1;
    }
    if (!field_0x08 && (UnknownFunction43caa0(0x3d, 0, event, 0x80000000) ||
                        UnknownFunction43caa0(0xc5, 0, event, 0x80000000))) {
toggle:
        if (field_0x333c)
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
    GameObject* menu = field_0x570->UnknownFunction45d2b0();
    if (!field_0x56c || !menu || field_0x3430)
        return;
    if (open) {
        GameObject* other = field_0x570->UnknownFunction45d2f0();
        if (other && !other->field_0x25_bit0)
            return;
        if (field_0x333c)
            return;
        field_0x56c->UnknownFunction499b20(id);
        if (!menu->field_0x25_bit0)
            return;
        menu->UnknownVirtualSlot4();
        field_0x333c = 1;
        field_0x34->UnknownFunction468dd0("RaceSound");
    } else {
        if (!field_0x333c)
            return;
        if (!field_0x56c->field_0x2c->UnknownFunction485df0())
            return;
        if (id != field_0x56c->field_0x3c)
            return;
        field_0x56c->field_0x2c->UnknownFunction485df0()->UnknownVirtualSlot26();
        GameObject* current = field_0x570->UnknownFunction45d2b0();
        if (current)
            current->UnknownVirtualSlot5();
        field_0x333c = 0;
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
    if (field_0x574) {
        delete field_0x574;
        field_0x574 = 0;
    }
    if (!PCGame::UnknownVirtualSlot15())
        return 0;
    field_0x56c = 0;
    SetLocaleInfoA(0x400, 0xe, field_0x3404);
    field_0x578.UnknownFunction523580();
    return 1;
}

// 0x00521cb0
int TrackGame::UnknownVirtualSlot18(const char* name, char* path) {
    return field_0x578.UnknownFunction5238f0(name, path);
}

// 0x00521cd0
int TrackGame::UnknownFunction521cd0() {
    if (field_0x3444)
        return field_0x56c->field_0x4a8 == 0;
    return 0;
}
