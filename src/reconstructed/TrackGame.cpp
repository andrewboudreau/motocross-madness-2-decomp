#include <string.h>

#include "TrackGame.h"

#include "DebugAlloc.h"
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
