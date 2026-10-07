// The display and controller selection windows (0x004ccbd0..0x004cde20).
// PCGame calls 0x004ccd60 to choose the display and 0x004cd610 to choose
// the joystick. Each one registers a window class ("VideoCardClass",
// "InputDeviceClass"), fills a list box and pumps messages until a window
// procedure below sets its done flag. No __FILE__ literal or RTTI reaches
// the range, so the file name is ours (tier 3). The two builders
// (0x004ccd60, 0x004cd610) are not reconstructed yet.

#include <windows.h>
#include "TrackGame.h"

// The display window (0x00689a2c..0x00689a48).
HWND g_DisplayWindow;          // 0x00689a30
HWND g_DisplayList;            // 0x00689a3c
int g_DisplayChoice;           // 0x00689a2c: the chosen item's data
int g_DisplayDone;             // 0x00689a44
int g_DisplayCancelled;        // 0x00689a48

// The controller window (0x00689a4c..0x00689a68).
HWND g_ControllerWindow;       // 0x00689a4c
HWND g_ControllerList;         // 0x00689a58
int g_ControllerChoice = -1;   // 0x00571654
int g_UseLastController = 1;   // 0x00571658: the "UseLastController" check box
int g_ControllerDone;          // 0x00689a64
int g_ControllerCancelled;     // 0x00689a68

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
