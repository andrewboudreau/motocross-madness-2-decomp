#include <stdlib.h>
#include <string.h>

#include "PCGame.h"

#include "ControlInterface.h"

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

extern "C" __declspec(dllimport) long __stdcall RegOpenKeyExA(void* key, const char* subKey,
                                                             unsigned long options,
                                                             unsigned long access, void** result);
extern "C" __declspec(dllimport) long __stdcall RegQueryValueExA(void* key, const char* name,
                                                                unsigned long* reserved,
                                                                unsigned long* type,
                                                                unsigned char* data,
                                                                unsigned long* size);
extern "C" __declspec(dllimport) long __stdcall RegCloseKey(void* key);
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
    strcpy(field_0x320, "Rainbow Studios");
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

// 0x004c0230
int PCGame::UnknownVirtualSlot15() {
    int result = Game::UnknownVirtualSlot15();
    ShowCursor(1);
    return result;
}

// 0x004c0250: while the window is active, hides the cursor (full screen),
// restores lost surfaces and runs the root object's slot 18.
int PCGame::UnknownVirtualSlot5() {
    if (field_0x0c && GetActiveWindow() == field_0x31c) {
        if (field_0x2d4_bit1)
            while (ShowCursor(0) >= 0)
                ;
        if ((field_0x0c && field_0x0c->field_0x19c &&
             field_0x0c->field_0x19c->UnknownMethod24() &&
             field_0x0c->field_0x19c->UnknownMethod27()) ||
            (field_0x2d5_bit3 && field_0x10 && PCTarget()->field_0x4c &&
             PCTarget()->field_0x4c->UnknownMethod24() &&
             PCTarget()->field_0x4c->UnknownMethod27()))
            return 0;
        if (field_0x2f4)
            field_0x2f4->UnknownVirtualSlot18();
    }
    return 1;
}

// 0x004c0310
int PCGame::UnknownVirtualSlot6() {
    if (field_0x2f4)
        field_0x2f4->UnknownVirtualSlot17();
    if (field_0x2d4_bit1)
        ShowCursor(1);
    return 1;
}

// 0x004c0340
int PCGame::UnknownVirtualSlot35(int value) {
    if (field_0x2d5_bit0)
        return 1;
    return field_0x2f4->UnknownVirtualSlot20(value);
}

// 0x004c0370
int PCGame::UnknownVirtualSlot36(int value) {
    if (field_0x2d5_bit0)
        return 1;
    return field_0x2f4->UnknownVirtualSlot21(value);
}

// 0x004c03a0: control 0xb7 released (kind 0) triggers the +0x10 object.
int PCGame::UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (Game::UnknownVirtualSlot13(event, entry))
        return 1;
    if (event->kind == 0 && event->control == 0xb7) {
        if (field_0x10 && !field_0x10->field_0x08)
            PCTarget()->UnknownFunction4c5d00();
        return 1;
    }
    return 0;
}

// 0x004c0400: with the debug bit, control 0x41 toggles the +0x10 object's
// +0x250 between 2 and 3.
int PCGame::UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (Game::UnknownVirtualSlot14(event, entry))
        return 1;
    if (field_0x2d4_bit2 && UnknownFunction43caa0(0x41, 0, event, 3)) {
        PCTarget()->field_0x250 = PCTarget()->field_0x250 == 3 ? 2 : 3;
        return 1;
    }
    return 0;
}

// 0x004c0470
void PCGame::UnknownFunction4c0470(const UnknownRect* rect) {
    field_0x308 = *rect;
}

// 0x004c0760
int PCGame::UnknownFunction4c0760(UnknownDisplay* display, int width, int height) {
    if (field_0x2d4_bit1 && !(display->field_0xb74 & 2)) {
        for (int i = 0; i < display->field_0x08; i++) {
            UnknownDisplayMode* mode = &display->field_0x10[i];
            if (mode->width > width || mode->height > height)
                mode->field_0x14 = 0;
        }
    }
    return 1;
}

// 0x004c0c10: "lobby" on the command line (last match wins) starts the
// network object in mode 4.
int PCGame::UnknownVirtualSlot37() {
    if (__argc >= 2) {
        int i = __argc;
        while (i > 0) {
            i--;
            if (!_stricmp(__argv[i], "lobby")) {
                if (!UnknownVirtualSlot16(4))
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
        if (!field_0x0c->UnknownFunction4c9c90())
            return 0;
        while (ShowCursor(0) >= 0)
            ;
    } else {
        if (!field_0x0c->UnknownFunction4c9d20(0, 0, 640, 480))
            return 0;
    }
    if (!field_0x0c->UnknownVirtualSlot2(640, 480, 16, 2, 0, field_0x2d0 == 0, 1))
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

// 0x004c1c20: a DWORD setting.
int PCGame::UnknownVirtualSlot20(const char* name, int defaultValue) {
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
float PCGame::UnknownVirtualSlot21(const char* name, float defaultValue) {
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
int PCGame::UnknownVirtualSlot22(const char* name, int defaultValue) {
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
int PCGame::UnknownVirtualSlot23(const char* name, const char* defaultValue, char* buffer,
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
int PCGame::UnknownVirtualSlot24(const char* name, void* data, unsigned long* size) {
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
int PCGame::UnknownVirtualSlot25(const char* name, int value) {
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

int PCGame::UnknownVirtualSlot27(const char* name, int value) {
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
int PCGame::UnknownVirtualSlot28(const char* name, const char* value) {
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
int PCGame::UnknownVirtualSlot29(const char* name, const void* data, unsigned long size) {
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
