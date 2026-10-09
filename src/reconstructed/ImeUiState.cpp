#include <windows.h>

// IME status-window helpers, 0x0052ff00..0x0052ff8e (provisional file name,
// tier 3). No source string attributes the unit: the two functions sit after
// VisibilityQuadTree.cpp's vector initializers (0x0052fdc0..0x0052fefb) and
// before wrecker.cpp's constructor (0x0052ff90), and their globals directly
// follow that TU's .bss (docs/WRECKER.md). PCVideoCard's cooperative-level
// switches (0x004c9c90, 0x004c9d20) and the window procedure's
// WM_ACTIVATEAPP case (0x004a0ea1) call them when IMM32.DLL is loaded
// (TrackGame+0x538). The SystemParametersInfo actions are confirmed:
// 0x6e is SPI_GETSHOWIMEUI and 0x6f SPI_SETSHOWIMEUI.

#ifndef SPI_GETSHOWIMEUI
#define SPI_GETSHOWIMEUI 0x006E
#define SPI_SETSHOWIMEUI 0x006F
#endif

static BOOL g_savedShowImeUi;      // 0x0068ace4
static int g_showImeUiSaved;       // 0x0068ace8

// 0x0052ff00: remembers whether the IME status window is shown.
void UnknownFunction52ff00()
{
    SystemParametersInfoA(SPI_GETSHOWIMEUI, 0, &g_savedShowImeUi, 0);
    g_showImeUiSaved = 1;
}

// 0x0052ff20: drains the message queue, then restores the remembered
// IME status-window setting.
void UnknownFunction52ff20()
{
    if (g_showImeUiSaved) {
        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        SystemParametersInfoA(SPI_SETSHOWIMEUI, g_savedShowImeUi, 0, 0);
    }
}
