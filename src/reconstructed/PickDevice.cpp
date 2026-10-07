// The display and controller selection windows (0x004ccbd0..0x004cde20).
// PCGame calls 0x004ccd60 to choose the display and 0x004cd610 to choose
// the joystick. Each one registers a window class ("VideoCardClass",
// "InputDeviceClass"), fills a list box and pumps messages until a window
// procedure below sets its done flag. No __FILE__ literal or RTTI reaches
// the range, so the file name is ours (tier 3).

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "TrackGame.h"
#include "PCGame.h"
#include "ControlInterface.h"
#include "JoystickDevice.h"
#include "Display.h"

// The display window (0x00689a2c..0x00689a48).
HWND g_DisplayWindow;          // 0x00689a30
HWND g_DisplayOkButton;        // 0x00689a34
HWND g_DisplayCancelButton;    // 0x00689a38
HWND g_DisplayList;            // 0x00689a3c
HWND g_DisplayLabel;           // 0x00689a40
int g_DisplayChoice;           // 0x00689a2c: the chosen item's data
int g_DisplayDone;             // 0x00689a44
int g_DisplayCancelled;        // 0x00689a48

// The controller window (0x00689a4c..0x00689a68).
HWND g_ControllerWindow;       // 0x00689a4c
HWND g_ControllerOkButton;     // 0x00689a50
HWND g_ControllerCancelButton; // 0x00689a54
HWND g_ControllerList;         // 0x00689a58
HWND g_ControllerCheckBox;     // 0x00689a5c: the "UseLastController" check box
HWND g_ControllerLabel;        // 0x00689a60: the title, then the note
int g_ControllerChoice = -1;   // 0x00571654
int g_UseLastController = 1;   // 0x00571658: the "UseLastController" check box
int g_ControllerDone;          // 0x00689a64
int g_ControllerCancelled;     // 0x00689a68

// Formats a GUID the way the registry "UseControllerId" value stores it.
inline void FormatGuid(char* text, UnknownGuid guid)
{
    sprintf(text, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.data1, guid.data2,
            guid.data3, guid.data4[0], guid.data4[1], guid.data4[2], guid.data4[3], guid.data4[4],
            guid.data4[5], guid.data4[6], guid.data4[7]);
}

// 0x004ccbd0: takes the selected display and closes the window.
void AcceptDisplayChoice()
{
    int index = SendMessage(g_DisplayList, LB_GETCURSEL, 0, 0);
    g_DisplayChoice = SendMessage(g_DisplayList, LB_GETITEMDATA, index, 0);
    DestroyWindow(g_DisplayWindow);
    g_DisplayDone = 1;
}

// 0x004ccc20
void CancelDisplayChoice()
{
    g_DisplayCancelled = 1;
    DestroyWindow(g_DisplayWindow);
    g_DisplayDone = 1;
}

// 0x004ccc50: the "VideoCardClass" window procedure.
LRESULT CALLBACK DisplayWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DESTROY:
        if (LOWORD(wParam) == 1002)
            AcceptDisplayChoice();
        break;
    case WM_CLOSE:
        if (window == g_DisplayWindow) {
            PostMessage((HWND)g_TrackGame->field_0x31c, WM_CLOSE, 0, 0);
            g_DisplayDone = 1;
            g_DisplayCancelled = 1;
            return 0;
        }
        if (window == (HWND)g_TrackGame->field_0x31c)
            DestroyWindow(g_DisplayWindow);
        g_DisplayDone = 1;
        g_DisplayCancelled = 1;
        return 0;
    case WM_CHAR:
        if (wParam == '\n' || wParam == '\r') {
            AcceptDisplayChoice();
            return 0;
        }
        break;
    case WM_COMMAND:
        if (HIWORD(wParam) == 0) {
            if (LOWORD(wParam) == 1000) {
                AcceptDisplayChoice();
                return 0;
            }
            if (LOWORD(wParam) == 1001)
                CancelDisplayChoice();
        }
        return 0;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

// 0x004ccd60: chooses the display. Returns the single usable display
// without asking (unless PCGame+0x544 forces the dialog), the display
// "UseVideoCardIdx" names when `useLast` is set, or the dialog's choice;
// `blade` is set when the software (Blade) entry was chosen.
UnknownDisplay* UnknownFunction4ccd60(int flag, int useLast, int* blade)
{
    char key[0x100];
    int usable = 0;
    int lastIndex = g_TrackGame->GetRegistryInt("UseVideoCardIdx", -1);
    int usableIndex;
    int i;
    for (i = 0; i < g_UnknownDisplayCount68a764; i++) {
        UnknownDisplay* display = g_UnknownDisplays68a754[i];
        if (display->field_0x1b8 & 1) {
            sprintf(key, "DriverInfo\\%s\\DisabledFullScreen", display->field_0x4bc);
            int fullScreenDisabled = g_TrackGame->GetRegistryFlag(key, 0);
            sprintf(key, "DriverInfo\\%s\\DisabledHardware", g_UnknownDisplays68a754[i]->field_0x4bc);
            int hardwareDisabled = g_TrackGame->GetRegistryFlag(key, 0);
            if (!fullScreenDisabled && !hardwareDisabled) {
                usable++;
                usableIndex = i;
            }
        }
        if (useLast && lastIndex == i)
            return g_UnknownDisplays68a754[i];
    }
    if (usable == 1 && !g_TrackGame->field_0x544)
        return g_UnknownDisplays68a754[usableIndex];

    WNDCLASS windowClass;
    windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = DisplayWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = (HINSTANCE)g_TrackGame->field_0x318;
    windowClass.hIcon = LoadIcon((HINSTANCE)g_TrackGame->field_0x318, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    windowClass.lpszMenuName = 0;
    windowClass.lpszClassName = "VideoCardClass";
    if (!RegisterClass(&windowClass))
        return 0;

    char title[0x80];
    char okText[0x20];
    char cancelText[0x20];
    char caption[0x80];
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0xfeb, title, sizeof(title));
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0x13e9, okText, sizeof(okText));
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0x140e, cancelText, sizeof(cancelText));
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0xbbd, caption, sizeof(caption));
    int x = GetSystemMetrics(SM_CXSCREEN) / 2 - 0xf0;
    int y = GetSystemMetrics(SM_CYSCREEN) / 2 - 0x78;
    g_DisplayWindow = CreateWindowEx(WS_EX_TOPMOST, "VideoCardClass", caption,
                                     WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU, x, y, 0x1e0,
                                     0xf0, (HWND)g_TrackGame->field_0x31c, 0,
                                     (HINSTANCE)g_TrackGame->field_0x318, 0);
    EnableWindow((HWND)g_TrackGame->field_0x31c, 0);
    g_DisplayLabel = CreateWindowEx(0, "STATIC", title, WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_SIMPLE,
                                    0xa, 2, 0x118, 0x18, g_DisplayWindow, 0,
                                    (HINSTANCE)g_TrackGame->field_0x318, 0);
    g_DisplayOkButton = CreateWindowEx(0, "BUTTON", okText,
                                       WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | BS_DEFPUSHBUTTON,
                                       0x188, 0x14, 0x4b, 0x28, g_DisplayWindow, (HMENU)0x3e8,
                                       (HINSTANCE)g_TrackGame->field_0x318, 0);
    g_DisplayCancelButton = CreateWindowEx(0, "BUTTON", cancelText,
                                           WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                           0x188, 0x46, 0x4b, 0x28, g_DisplayWindow, (HMENU)0x3e9,
                                           (HINSTANCE)g_TrackGame->field_0x318, 0);
    g_DisplayList = CreateWindowEx(0, "LISTBOX", "",
                                   WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | LBS_NOTIFY,
                                   0xa, 0x14, 0x177, 0xb8, g_DisplayWindow, (HMENU)0x3ea,
                                   (HINSTANCE)g_TrackGame->field_0x318, 0);
    SetFocus(g_DisplayOkButton);

    int entries = 0;
    sprintf(key, "DriverInfo\\%s\\DisabledFullScreen", g_UnknownDisplays68a754[0]->field_0x4bc);
    int fullScreenDisabled = g_TrackGame->GetRegistryFlag(key, 0);
    sprintf(key, "DriverInfo\\%s\\DisabledSoftware", g_UnknownDisplays68a754[0]->field_0x4bc);
    int softwareDisabled = g_TrackGame->GetRegistryFlag(key, 0);
    if (g_TrackGame->field_0x544 && !fullScreenDisabled && !softwareDisabled) {
        char text[0x80];
        g_TrackGame->LoadResourceString(0x1463, text, sizeof(text));
        SendMessage(g_DisplayList, LB_ADDSTRING, 0, (LPARAM)text);
        SendMessage(g_DisplayList, LB_SETITEMDATA, 0, -1);
        entries = 1;
    }
    for (i = 0; i < g_UnknownDisplayCount68a764; i++) {
        sprintf(key, "DriverInfo\\%s\\DisabledFullScreen", g_UnknownDisplays68a754[i]->field_0x4bc);
        int fullScreenDisabled = g_TrackGame->GetRegistryFlag(key, 0);
        sprintf(key, "DriverInfo\\%s\\DisabledHardware", g_UnknownDisplays68a754[i]->field_0x4bc);
        int hardwareDisabled = g_TrackGame->GetRegistryFlag(key, 0);
        if (!fullScreenDisabled && !hardwareDisabled && (g_UnknownDisplays68a754[i]->field_0x1b8 & 1)) {
            SendMessage(g_DisplayList, LB_ADDSTRING, 0,
                        (LPARAM)g_UnknownDisplays68a754[i]->field_0x5c0.description);
            SendMessage(g_DisplayList, LB_SETITEMDATA, entries, i);
            entries++;
        }
    }
    if (entries == 0) {
        SendMessage(g_DisplayList, LB_ADDSTRING, 0, (LPARAM) "No 3D Cards Found!");
        SendMessage(g_DisplayList, LB_SETITEMDATA, 0, -1);
        entries = 1;
    }
    if (lastIndex > -1)
        SendMessage(g_DisplayList, LB_SETCURSEL, lastIndex + 1, 0);
    else
        SendMessage(g_DisplayList, LB_SETCURSEL, entries - 1, 0);
    if (entries < 2) {
        AcceptDisplayChoice();
    } else {
        ShowWindow(g_DisplayWindow, SW_SHOW);
        UpdateWindow(g_DisplayWindow);
        ShowCursor(1);
    }
    while (!g_DisplayDone) {
        MSG message;
        if (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
    ShowCursor(0);
    EnableWindow((HWND)g_TrackGame->field_0x31c, 1);
    SetForegroundWindow((HWND)g_TrackGame->field_0x31c);
    UpdateWindow((HWND)g_TrackGame->field_0x31c);
    if (g_DisplayCancelled)
        return 0;
    if (g_DisplayChoice == -1) {
        g_DisplayChoice = 0;
        *blade = 1;
    } else {
        *blade = 0;
    }
    if (g_DisplayChoice >= 0 && g_DisplayChoice < g_UnknownDisplayCount68a764) {
        g_TrackGame->SetRegistryInt("UseVideoCardIdx", g_DisplayChoice);
        return g_UnknownDisplays68a754[g_DisplayChoice];
    }
    return 0;
}

// 0x004cd430: takes the selected controller and closes the window.
void AcceptControllerChoice()
{
    int index = SendMessage(g_ControllerList, LB_GETCURSEL, 0, 0);
    g_ControllerChoice = SendMessage(g_ControllerList, LB_GETITEMDATA, index, 0);
    DestroyWindow(g_ControllerWindow);
    g_ControllerDone = 1;
}

// 0x004cd480
void CancelControllerChoice()
{
    g_ControllerCancelled = 1;
    DestroyWindow(g_ControllerWindow);
    g_ControllerDone = 1;
}

// 0x004cd4b0: the "InputDeviceClass" window procedure.
LRESULT CALLBACK ControllerWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DESTROY:
        if (LOWORD(wParam) == 1002)
            AcceptControllerChoice();
        break;
    case WM_CLOSE:
        if (window == g_ControllerWindow)
            PostMessage((HWND)g_TrackGame->field_0x31c, WM_CLOSE, 0, 0);
        else if (window == (HWND)g_TrackGame->field_0x31c)
            DestroyWindow(g_ControllerWindow);
        g_ControllerDone = 1;
        g_ControllerCancelled = 1;
        return 0;
    case WM_CHAR:
        if (wParam == '\n' || wParam == '\r') {
            AcceptControllerChoice();
            return 0;
        }
        break;
    case WM_COMMAND:
        if (HIWORD(wParam) == 0) {
            if (LOWORD(wParam) == 1000) {
                AcceptControllerChoice();
                return 0;
            }
            if (LOWORD(wParam) == 1001) {
                CancelControllerChoice();
                return 0;
            }
            if (LOWORD(wParam) == 1003) {
                g_UseLastController = !g_UseLastController;
                SendMessage((HWND)lParam, BM_SETCHECK, g_UseLastController != 0, 0);
                g_TrackGame->SetRegistryFlag("UseLastController", g_UseLastController);
            }
        }
        break;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

// 0x004cd610: chooses the joystick. With `useLast`, returns the index of
// the device whose instance GUID the registry "UseControllerId" names;
// otherwise runs the dialog and returns its index (-2 on cancel or
// failure), saving the GUID.
int UnknownFunction4cd610(int useLast)
{
    char lastId[0x80];
    char text[0x80];
    MSG message;
    unsigned long size = sizeof(lastId);
    int i;
    g_TrackGame->GetRegistryString("UseControllerId", "", lastId, &size);
    g_UseLastController = useLast;
    if (useLast) {
        for (i = 0; i < __min(g_TrackGame->controlInterface->joystickCount, 8); i++) {
            FormatGuid(text, g_TrackGame->controlInterface->joysticks[i]->deviceInfo.instanceGuid);
            if (strcmp(text, lastId) == 0)
                return i;
        }
    }

    WNDCLASS windowClass;
    windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = ControllerWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = (HINSTANCE)g_TrackGame->field_0x318;
    windowClass.hIcon = LoadIcon((HINSTANCE)g_TrackGame->field_0x318, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    windowClass.lpszMenuName = 0;
    windowClass.lpszClassName = "InputDeviceClass";
    if (!RegisterClass(&windowClass))
        return -2;

    char title[0x80];
    char okText[0x20];
    char cancelText[0x20];
    char caption[0x80];
    char checkBoxText[0x80];
    char note[0x80];
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0x14c5, title, sizeof(title));
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0x13e9, okText, sizeof(okText));
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0x140e, cancelText, sizeof(cancelText));
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0xbbd, caption, sizeof(caption));
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0x14c6, checkBoxText, sizeof(checkBoxText));
    LoadString((HINSTANCE)g_TrackGame->field_0x420, 0x14d0, note, sizeof(note));
    int x = GetSystemMetrics(SM_CXSCREEN) / 2 - 0xf0;
    int y = GetSystemMetrics(SM_CYSCREEN) / 2 - 0x78;
    g_ControllerWindow = CreateWindowEx(WS_EX_TOPMOST, "InputDeviceClass", caption,
                                        WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU, x, y, 0x1e0,
                                        0xf0, (HWND)g_TrackGame->field_0x31c, 0,
                                        (HINSTANCE)g_TrackGame->field_0x318, 0);
    EnableWindow((HWND)g_TrackGame->field_0x31c, 0);
    g_ControllerLabel = CreateWindowEx(0, "STATIC", title, WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_SIMPLE,
                                       0xa, 2, 0x118, 0x18, g_ControllerWindow, 0,
                                       (HINSTANCE)g_TrackGame->field_0x318, 0);
    g_ControllerOkButton = CreateWindowEx(0, "BUTTON", okText,
                                          WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | BS_DEFPUSHBUTTON,
                                          0x188, 0x14, 0x4b, 0x28, g_ControllerWindow, (HMENU)0x3e8,
                                          (HINSTANCE)g_TrackGame->field_0x318, 0);
    g_ControllerCancelButton = CreateWindowEx(0, "BUTTON", cancelText,
                                              WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                              0x188, 0x46, 0x4b, 0x28, g_ControllerWindow, (HMENU)0x3e9,
                                              (HINSTANCE)g_TrackGame->field_0x318, 0);
    g_ControllerList = CreateWindowEx(0, "LISTBOX", "",
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | LBS_NOTIFY,
                                      0xa, 0x14, 0x177, 0xa0, g_ControllerWindow, (HMENU)0x3ea,
                                      (HINSTANCE)g_TrackGame->field_0x318, 0);
    g_ControllerCheckBox = CreateWindowEx(0, "BUTTON", checkBoxText,
                                          WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX,
                                          0xa, 0xa8, 0x177, 0x18, g_ControllerWindow, (HMENU)0x3eb,
                                          (HINSTANCE)g_TrackGame->field_0x318, 0);
    g_ControllerLabel = CreateWindowEx(0, "STATIC", note, WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_SIMPLE,
                                       0xa, 0xc0, 0x1c2, 0x18, g_ControllerWindow, 0,
                                       (HINSTANCE)g_TrackGame->field_0x318, 0);
    SendMessage(g_ControllerCheckBox, BM_SETCHECK, g_UseLastController ? 1 : 0, 0);
    SetFocus(g_ControllerOkButton);

    int selected = 0;
    for (i = 0; i < __min(g_TrackGame->controlInterface->joystickCount, 8); i++) {
        SendMessage(g_ControllerList, LB_ADDSTRING, 0,
                    (LPARAM)g_TrackGame->controlInterface->joysticks[i]->deviceInfo.productName);
        SendMessage(g_ControllerList, LB_SETITEMDATA, i, i);
        FormatGuid(text, g_TrackGame->controlInterface->joysticks[i]->deviceInfo.instanceGuid);
        if (strcmp(text, lastId) == 0)
            selected = i;
    }
    if (g_TrackGame->controlInterface->joystickCount == 0) {
        SendMessage(g_ControllerList, LB_ADDSTRING, 0, (LPARAM) "None");
        SendMessage(g_ControllerList, LB_SETITEMDATA, 0, -1);
        selected = 0;
        g_TrackGame->SetRegistryString("UseControllerId", "");
    }
    SendMessage(g_ControllerList, LB_SETCURSEL, selected, 0);
    if (g_TrackGame->controlInterface->joystickCount < 2) {
        AcceptControllerChoice();
    } else {
        ShowWindow(g_ControllerWindow, SW_SHOW);
        UpdateWindow(g_ControllerWindow);
        ShowCursor(1);
    }
    while (!g_ControllerDone) {
        if (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
    ShowCursor(0);
    EnableWindow((HWND)g_TrackGame->field_0x31c, 1);
    SetForegroundWindow((HWND)g_TrackGame->field_0x31c);
    UpdateWindow((HWND)g_TrackGame->field_0x31c);
    if (g_ControllerCancelled)
        return -2;
    FormatGuid(text, g_TrackGame->controlInterface->joysticks[g_ControllerChoice]->deviceInfo.instanceGuid);
    g_TrackGame->SetRegistryString("UseControllerId", text);
    return g_ControllerChoice;
}
