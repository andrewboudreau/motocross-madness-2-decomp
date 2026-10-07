// DeviceSetup.cpp (provisional name). The control-layout unit
// 0x00448960..0x00449e60, after the DirectX probe 0x00448560 and before
// dirlist.cpp (0x00449ea6). No __FILE__ or RTTI: the file name and the
// function names are ours. The object is TrackGame+0x33fc
// (UnknownTrackGameObject33fc, TrackGame.h); its control file is the
// profile's "control.ctl" (UiInfo.cpp).

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DeviceSetup.h"

#include "ControlInterface.h"
#include "JoystickDevice.h"
#include "KeyboardDevice.h"
#include "MouseDevice.h"
#include "TrackGame.h"

// 0x00448960
UnknownTrackGameObject33fc::UnknownTrackGameObject33fc()
{
    for (int device = 0; device < 8; device++) {
        for (int row = 0; row < 14; row++) {
            field_0x04[device][row].kind = -1;
            field_0x04[device][row].code = -1;
        }
    }
}

// 0x00448990: reads the current device and every assignment.
void UnknownTrackGameObject33fc::UnknownFunction448990(const char* path)
{
    field_0x00 = UnknownFunction47b9e0("Setup", "CurrentInputDevice", 0, path);
    for (int device = 0; device < 8; device++)
        for (int row = 0; row < 14; row++)
            UnknownFunction448a50(device, row, path);
}

// 0x004489e0: writes the current device and every assignment.
void UnknownTrackGameObject33fc::UnknownFunction4489e0(const char* path)
{
    char value[0x80];

    sprintf(value, "%d", field_0x00);
    UnknownFunction47ba80("Setup", "CurrentInputDevice", value, path);
    for (int device = 0; device < 8; device++)
        for (int row = 0; row < 14; row++)
            UnknownFunction448bc0(device, row, path);
}

// 0x00448a50: an assignment is "<kind digit>,<code>"; "NONE" or empty
// leaves it unassigned.
void UnknownTrackGameObject33fc::UnknownFunction448a50(int device, int row, const char* path)
{
    char value[0x80];
    char key[0x80];
    char section[0x80];
    char kind[0x80];

    sprintf(section, "Controller%d", device);
    sprintf(key, "Key%d", row);
    UnknownFunction47b930(section, key, "NONE", value, 0x80, path);
    if (strcmp(value, "NONE") == 0 || strcmp(value, "") == 0) {
        field_0x04[device][row].kind = -1;
        field_0x04[device][row].code = 0;
        return;
    }
    sprintf(kind, "%c", value[0]);
    field_0x04[device][row].kind = atoi(kind);
    field_0x04[device][row].code = atoi(strchr(value, ',') + 1);
}

// 0x00448bc0
void UnknownTrackGameObject33fc::UnknownFunction448bc0(int device, int row, const char* path)
{
    char value[0x80];
    char key[0x80];
    char section[0x80];

    if (field_0x04[device][row].kind == -1)
        strcpy(value, "");
    else
        sprintf(value, "%d,%d", field_0x04[device][row].kind, field_0x04[device][row].code);
    sprintf(section, "Controller%d", device);
    sprintf(key, "Key%d", row);
    UnknownFunction47ba80(section, key, value, path);
}

// 0x00448c90
void UnknownTrackGameObject33fc::UnknownFunction448c90(int device, int row, int kind, int code)
{
    field_0x04[device][row].kind = kind;
    field_0x04[device][row].code = code;
}

// 0x00448cc0: whether another row of the current device already has this
// assignment (its row in `other`).
int UnknownTrackGameObject33fc::UnknownFunction448cc0(int row, int kind, int code, int* other)
{
    if (field_0x04[field_0x00][row].kind == kind && field_0x04[field_0x00][row].code == code)
        return 0;
    for (int i = 0; i < 14; i++) {
        if (field_0x04[field_0x00][i].kind == kind && field_0x04[field_0x00][i].code == code) {
            *other = i;
            return 1;
        }
    }
    return 0;
}

// 0x00448d30
int UnknownTrackGameObject33fc::UnknownFunction448d30(UnknownControlBinding* binding, int kind,
                                                      int code, int code2)
{
    if (code == -1)
        return 0;
    if (code2 == -1)
        return 0;
    switch (kind) {
    case 2:
        if (g_TrackGame->controlInterface->activeJoystick)
            g_TrackGame->controlInterface->activeJoystick->UnknownFunction489a20(binding, 0.018f,
                                                                                0.01f);
        break;
    case 1:
        if (g_TrackGame->controlInterface->mouse) {
            switch (code) {
            case -3:
                g_TrackGame->controlInterface->mouse->UnknownFunction48a420(binding);
                return 1;
            case -5:
                g_TrackGame->controlInterface->mouse->UnknownFunction48a420(binding);
                return 1;
            }
            return 1;
        }
        break;
    case 0:
        g_TrackGame->controlInterface->keyboard->UnknownFunction489f80(binding, code, code2, 0.018f,
                                                                      0.01f);
        return 1;
    }
    return 0;
}

// 0x00448df0
int UnknownTrackGameObject33fc::UnknownFunction448df0(int control, int code, int kind)
{
    if (code == -1 || code < 0)
        return 0;
    switch (kind) {
    case 2: {
        ControlInterface* controls = g_TrackGame->controlInterface;
        if (controls->activeJoystick)
            g_TrackGame->controlMapping->UnknownFunction43cb50(control, code,
                                                               controls->activeJoystickIndex);
        break;
    }
    case 1:
        if (g_TrackGame->controlInterface->mouse)
            g_TrackGame->controlMapping->UnknownFunction43cb80(control, code);
        break;
    case 0:
        g_TrackGame->controlMapping->UnknownFunction43cb30(control, code);
        break;
    }
    return 0;
}

// Joystick axis of an axis direction code (-3 ... -13).
static inline int AxisOfCode(int code)
{
    switch (code) {
    case -3:
        return 0;
    case -5:
        return 1;
    case -7:
        return 2;
    case -9:
        return 3;
    case -11:
        return 4;
    case -13:
        return 5;
    }
    return 1;
}

// 0x00448e90: installs the assignments of device `device` (the current one
// when -1): the steering and throttle bindings, an optional third axis and
// the button mapping of rows 4-13.
void UnknownTrackGameObject33fc::UnknownFunction448e90(const char* path, int device)
{
    if (g_TrackGame->controlMapping)
        g_TrackGame->controlMapping->UnknownFunction43caf0();
    if (device != -1)
        field_0x00 = device;
    g_TrackGame->field_0x3344.UnknownFunction43cde0();
    g_TrackGame->field_0x3380.UnknownFunction43cde0();
    g_TrackGame->field_0x33bc.UnknownFunction43cde0();

    g_TrackGame->field_0x3344 =
        UnknownControlBinding(-1.0f, 1.0f, 0.0f, g_TrackGame->controlInterface->UnknownFunction43ce90(),
                              AxisOfCode(field_0x04[field_0x00][0].code), 0.04f);
    g_TrackGame->field_0x3380 =
        UnknownControlBinding(-1.0f, 1.0f, 0.0f, g_TrackGame->controlInterface->UnknownFunction43ce90(),
                              AxisOfCode(field_0x04[field_0x00][2].code), 0.0f);
    UnknownFunction448d30(&g_TrackGame->field_0x3344, field_0x04[field_0x00][0].kind,
                          field_0x04[field_0x00][0].code, field_0x04[field_0x00][1].code);
    UnknownFunction448d30(&g_TrackGame->field_0x3380, field_0x04[field_0x00][2].kind,
                          field_0x04[field_0x00][2].code, field_0x04[field_0x00][3].code);

    if (field_0x04[field_0x00][4].kind == 2 && field_0x04[field_0x00][4].code < 0) {
        if (field_0x04[field_0x00][4].code == -6 || field_0x04[field_0x00][4].code == -7) {
            g_TrackGame->field_0x33bc = UnknownControlBinding(
                0.0f, 1.0f, 0.0f, g_TrackGame->controlInterface->UnknownFunction43ce90(), 2, 0.0f);
            UnknownFunction448d30(&g_TrackGame->field_0x33bc, field_0x04[field_0x00][4].kind, -7, -6);
        }
        if (field_0x04[field_0x00][4].code == -12 || field_0x04[field_0x00][4].code == -13) {
            g_TrackGame->field_0x33bc = UnknownControlBinding(
                1.0f, 1.0f, 0.0f, g_TrackGame->controlInterface->UnknownFunction43ce90(), 5, 0.0f);
            UnknownFunction448d30(&g_TrackGame->field_0x33bc, field_0x04[field_0x00][4].kind, -13,
                                  -12);
        }
    }

    for (int row = 4; row < 14; row++)
        UnknownFunction448df0(row, field_0x04[field_0x00][row].code, field_0x04[field_0x00][row].kind);
}

// 0x00449220: picks the default device for the active joystick (3 or 4 by
// its subtype, else 1 with at least four buttons, else 0); 6 without one.
int UnknownTrackGameObject33fc::UnknownFunction449220()
{
    ControlInterface* controls = g_TrackGame->controlInterface;
    if (controls->joystickCount && controls->activeJoystick) {
        JoystickDevice* joystick = controls->activeJoystick;
        if (joystick->deviceSubtype == 4)
            field_0x00 = 3;
        else if (joystick->deviceSubtype == 6)
            field_0x00 = 4;
        else
            field_0x00 = joystick->buttonCount >= 4;
    } else {
        field_0x00 = 6;
    }
    return field_0x00;
}

// 0x00449270
int UnknownFunction449270(int code, char* text)
{
    char name[0x80];
    char prefix[0x80];
    long keyData = code << 16;

    if (code > 0x80)
        keyData = (keyData & ~0x800000) | 0x1000000;
    if (UnknownFunction449d40(code) != code)
        keyData = ((UnknownFunction449e30(code, 0) << 8) + UnknownFunction449d40(code)) << 16;
    if (!GetKeyNameText(keyData, name, 0x80)) {
        strcpy(text, "Unknown");
        return 0;
    }
    g_TrackGame->LoadResourceString(0x1406, prefix, 0x80);
    sprintf(text, "%s %s", prefix, name);
    return 1;
}

// 0x00449350
void UnknownTrackGameObject33fc::UnknownFunction449350(int device, int row, char* text)
{
    UnknownFunction449380(field_0x04[device][row].kind, field_0x04[device][row].code, text);
}

// 0x00449380: the display name of an assignment.
int UnknownTrackGameObject33fc::UnknownFunction449380(int kind, int code, char* text)
{
    char name[0x80];

    strcpy(text, "");
    if (kind == 0) {
        switch (code) {
        case 0x02: return UnknownFunction449270(0x02, text);
        case 0x03: return UnknownFunction449270(0x03, text);
        case 0x04: return UnknownFunction449270(0x04, text);
        case 0x05: return UnknownFunction449270(0x05, text);
        case 0x06: return UnknownFunction449270(0x06, text);
        case 0x07: return UnknownFunction449270(0x07, text);
        case 0x08: return UnknownFunction449270(0x08, text);
        case 0x09: return UnknownFunction449270(0x09, text);
        case 0x0a: return UnknownFunction449270(0x0a, text);
        case 0x0b: return UnknownFunction449270(0x0b, text);
        case 0x0e: return UnknownFunction449270(0x0e, text);
        case 0x10: return UnknownFunction449270(0x10, text);
        case 0x11: return UnknownFunction449270(0x11, text);
        case 0x12: return UnknownFunction449270(0x12, text);
        case 0x13: return UnknownFunction449270(0x13, text);
        case 0x14: return UnknownFunction449270(0x14, text);
        case 0x15: return UnknownFunction449270(0x15, text);
        case 0x16: return UnknownFunction449270(0x16, text);
        case 0x17: return UnknownFunction449270(0x17, text);
        case 0x18: return UnknownFunction449270(0x18, text);
        case 0x19: return UnknownFunction449270(0x19, text);
        case 0x1a: return UnknownFunction449270(0x1a, text);
        case 0x1b: return UnknownFunction449270(0x1b, text);
        case 0x1c: return UnknownFunction449270(0x1c, text);
        case 0x1d:
            g_TrackGame->LoadResourceString(0x13c7, text, 0x80);
            return 1;
        case 0x1e: return UnknownFunction449270(0x1e, text);
        case 0x1f: return UnknownFunction449270(0x1f, text);
        case 0x20: return UnknownFunction449270(0x20, text);
        case 0x21: return UnknownFunction449270(0x21, text);
        case 0x22: return UnknownFunction449270(0x22, text);
        case 0x23: return UnknownFunction449270(0x23, text);
        case 0x24: return UnknownFunction449270(0x24, text);
        case 0x25: return UnknownFunction449270(0x25, text);
        case 0x26: return UnknownFunction449270(0x26, text);
        case 0x27: return UnknownFunction449270(0x27, text);
        case 0x28: return UnknownFunction449270(0x28, text);
        case 0x29: return UnknownFunction449270(0x29, text);
        case 0x2a:
            g_TrackGame->LoadResourceString(0x13c5, text, 0x80);
            return 1;
        case 0x2b: return UnknownFunction449270(0x2b, text);
        case 0x2c: return UnknownFunction449270(0x2c, text);
        case 0x2d: return UnknownFunction449270(0x2d, text);
        case 0x2e: return UnknownFunction449270(0x2e, text);
        case 0x2f: return UnknownFunction449270(0x2f, text);
        case 0x30: return UnknownFunction449270(0x30, text);
        case 0x32: return UnknownFunction449270(0x32, text);
        case 0x36:
            g_TrackGame->LoadResourceString(0x13c6, text, 0x80);
            return 1;
        case 0x37: return UnknownFunction449270(0x37, text);
        case 0x38:
            g_TrackGame->LoadResourceString(0x13c3, text, 0x80);
            return 1;
        case 0x39: return UnknownFunction449270(0x39, text);
        case 0x64: return UnknownFunction449270(0x64, text);
        case 0x65: return UnknownFunction449270(0x65, text);
        case 0x66: return UnknownFunction449270(0x66, text);
        case 0x9d:
            g_TrackGame->LoadResourceString(0x13c8, text, 0x80);
            return 1;
        case 0xb3: return UnknownFunction449270(0xb3, text);
        case 0xb5: return UnknownFunction449270(0xb5, text);
        case 0xb8:
            g_TrackGame->LoadResourceString(0x13c4, text, 0x80);
            return 1;
        case 0xc8: return UnknownFunction449270(0xc8, text);
        case 0xcb: return UnknownFunction449270(0xcb, text);
        case 0xcd: return UnknownFunction449270(0xcd, text);
        case 0xd0: return UnknownFunction449270(0xd0, text);
        }
    } else if (kind == 1) {
        if (code < 0) {
            switch (code) {
            case -3:
                g_TrackGame->LoadResourceString(0xfe1, text, 0x80);
                return 1;
            case -5:
                g_TrackGame->LoadResourceString(0xfe2, text, 0x80);
                return 1;
            }
        } else {
            g_TrackGame->LoadResourceString(0xfe3, name, 0x80);
            sprintf(text, "%s %d", name, code + 1);
            return 1;
        }
    } else if (kind == 2) {
        if (code < 0) {
            switch (code) {
            case -3:
                g_TrackGame->LoadResourceString(0xfe4, text, 0x80);
                return 1;
            case -5:
                g_TrackGame->LoadResourceString(0xfe5, text, 0x80);
                return 1;
            case -7:
                g_TrackGame->LoadResourceString(0xfe6, text, 0x80);
                return 1;
            case -9:
                g_TrackGame->LoadResourceString(0xfe7, text, 0x80);
                return 1;
            case -11:
                g_TrackGame->LoadResourceString(0xfe8, text, 0x80);
                return 1;
            case -13:
                g_TrackGame->LoadResourceString(0xfe9, text, 0x80);
                return 1;
            default:
                return 0;
            }
        } else {
            g_TrackGame->LoadResourceString(0xfea, name, 0x80);
            sprintf(text, "%s %d", name, code + 1);
            return 1;
        }
    }
    return 0;
}

// The NEC PC-98 scan code table (.data 0x00568df8, 256 entries, -1 where the
// code has no translation) and the detected layout (0x005691f8, initialised
// to -1) and platform (.bss 0x0059ada8). The table data is not reconstructed.
extern int g_Pc98ScanCodes[256];
extern int g_KeyboardSubtype;
extern int g_IsWindowsNT;

// 0x00449d40
int UnknownFunction449d40(int code)
{
    int translated = code;

    if (code >= 0 && code <= 0xff) {
        UnknownFunction449d90();
        if (g_KeyboardSubtype == 0x62 && !g_IsWindowsNT)
            translated = g_Pc98ScanCodes[code];
        return translated == -1 ? code : translated;
    }
    return code;
}

// 0x00449d90
void UnknownFunction449d90()
{
    if (g_KeyboardSubtype == -1) {
        g_KeyboardSubtype = UnknownFunction449de0();
        g_IsWindowsNT = UnknownFunction449db0();
    }
}

// 0x00449db0
int UnknownFunction449db0()
{
    OSVERSIONINFO info;

    info.dwOSVersionInfoSize = sizeof(info);
    GetVersionEx(&info);
    return info.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

// 0x00449de0: GetKeyboardType(0) 7 is a Japanese keyboard; the high byte of
// GetKeyboardType(1) is the OEM (0x0d NEC) and the low byte the subtype.
int UnknownFunction449de0()
{
    int subtype = 0;

    if (GetKeyboardType(0) == 7) {
        switch ((GetKeyboardType(1) >> 8) & 0xff) {
        case 0x0d:
            subtype = (BYTE)GetKeyboardType(1) == 5 ? 0x6a : 0x62;
            break;
        case 0:
            subtype = (BYTE)GetKeyboardType(1) ? 0x6a : 0x65;
            break;
        }
    }
    return subtype;
}

// 0x00449e30
int UnknownFunction449e30(int code, int value)
{
    int result = value;

    UnknownFunction449d90();
    if (g_KeyboardSubtype == 0x62 && !g_IsWindowsNT)
        result = 0;
    return result;
}
