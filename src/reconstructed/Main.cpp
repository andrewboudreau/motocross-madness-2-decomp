// Main.cpp (provisional name): WinMain and the main window,
// 0x004a05e0..0x004a10d5. See Main.h and docs/MAIN.md.
#include <windows.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugOverlay.h"
#include "DeviceSetup.h"
#include "ExceptionHandler.h"
#include "Main.h"
#include "PCControl.h"
#include "PCVideoCard.h"
#include "TrackGame.h"

int g_AppActive = 1;
HWND g_MainWindow;
int g_MainWindowShown;

// Copies at most 127 characters of `src` into the 128-byte `dst` and ends it.
#define COPY_NAME(dst, src)                \
    {                                      \
        int length = strlen(src);          \
        int count = length > 0x7f ? 0x7f : length; \
        strncpy(dst, src, count);          \
        dst[count] = 0;                    \
    }

// 0x004a05e0
BOOL CALLBACK UnknownEnumWindowsProc4a05e0(HWND window, LPARAM found)
{
    char title[0x80];
    GetWindowText(window, title, 0x80);
    if (strcmp(title, g_TrackGame->applicationName) == 0) {
        ShowWindow(window, SW_RESTORE);
        *(int*)found = 1;
        return FALSE;
    }
    return TRUE;
}

// 0x004a0680
int WINAPI UnknownFunction4a0680(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine,
                                 int showCommand)
{
    char tempName[MAX_PATH];
    char message[0x100];
    SYSTEM_INFO systemInfo;
    MEMORYSTATUS memoryStatus;
    char text[0x80];
    MSG msg;
    unsigned long platform;
    unsigned long dxVersion;
    int found = 0;

    g_MemTagStack->Push("Startup");
    GetSystemInfo(&systemInfo);
    GlobalMemoryStatus(&memoryStatus);

    COPY_NAME(g_TrackGame->companyName, "Rainbow");
    COPY_NAME(g_TrackGame->applicationName, "Demo");
    if (LoadString((HINSTANCE)g_TrackGame->resourceInstance, 0xbbc, text, 0x80))
        COPY_NAME(g_TrackGame->companyName, text);
    if (LoadString((HINSTANCE)g_TrackGame->resourceInstance, 0xbbd, text, 0x80))
        COPY_NAME(g_TrackGame->applicationName, text);

    // A window with the game's title belongs to another instance.
    EnumWindows(UnknownEnumWindowsProc4a05e0, (LPARAM)&found);
    if (found)
        exit(0);

    setlocale(LC_ALL, "");
    GetCurrentDirectory(MAX_PATH, g_TrackGame->field_0x1cc);
    if (!g_TrackGame->UnknownVirtualSlot1())
        exit(0);

    if (!UnknownFunction4a0c50(instance, showCommand))
        return TRUE;

    ShowWindow(g_MainWindow, SW_SHOWNORMAL);
    UpdateWindow(g_MainWindow);
    g_TrackGame->windowHandle = g_MainWindow;
    g_TrackGame->instanceHandle = instance;
    if (!g_TrackGame->UnknownVirtualSlot2())
        exit(0);

    GetDXVersion(&dxVersion, &platform, (unsigned long*)&g_TrackGame->field_0x544);
    if (platform != VER_PLATFORM_WIN32_WINDOWS && platform != VER_PLATFORM_WIN32_NT) {
        if (LoadString((HINSTANCE)g_TrackGame->resourceInstance, 0xbb8, text, 0x80))
            MessageBox(0, text, g_TrackGame->applicationName, MB_ICONHAND);
        exit(0);
    }
    if (dxVersion < g_RequiredDXVersion) {
        if (LoadString((HINSTANCE)g_TrackGame->resourceInstance, 0xbb9, text, 0x80))
            MessageBox(0, text, g_TrackGame->applicationName, MB_ICONHAND);
        exit(0);
    }
    if (platform == VER_PLATFORM_WIN32_NT) {
        // Writing under HKEY_LOCAL_MACHINE needs administrator rights.
        if (!g_TrackGame->SetRegistryFlag("IsAdmin", 1)) {
            if (LoadString((HINSTANCE)g_TrackGame->resourceInstance, 0x14dc, text, 0x80))
                MessageBox(0, text, g_TrackGame->applicationName, MB_ICONHAND);
            exit(0);
        }
        // The current directory must be writable.
        GetTempFileName(".", "tst", 0, tempName);
        FILE* file = fopen(tempName, "w");
        if (!file) {
            if (LoadString((HINSTANCE)g_TrackGame->resourceInstance, 0x14dc, text, 0x80))
                MessageBox(0, text, g_TrackGame->applicationName, MB_ICONHAND);
            exit(0);
        }
        fclose(file);
        DeleteFile(tempName);
    }

    message[0] = 0;
    if (!g_TrackGame->StartUp(message)) {
        DestroyWindow(g_MainWindow);
        if (g_TrackGame->field_0x2d4_bit1)
            ShowCursor(TRUE);
        if (message[0])
            MessageBox(0, message, g_TrackGame->applicationName, MB_ICONHAND);
        PostQuitMessage(0);
    }

    g_MainWindowShown = 1;
    ShowWindow(g_MainWindow, SW_SHOWNORMAL);
    for (;;) {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                PostQuitMessage(msg.wParam);
                return TRUE;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (g_AppActive && !g_TrackGame->UnknownVirtualSlot10()) {
            g_TrackGame->UnknownVirtualSlot15();
            DestroyWindow(g_MainWindow);
            PostQuitMessage(0);
            return TRUE;
        }
    }
}

// 0x004a0bc0: crashes in the start-up and the game loop end in the error log.
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine, int showCommand)
{
    int result = -1;
    __try {
        result = UnknownFunction4a0680(instance, previous, commandLine, showCommand);
    } __except (RecordExceptionInfo(GetExceptionInformation(), "main thread")) {
    }
    return result;
}

// 0x004a0c50
int UnknownFunction4a0c50(HINSTANCE instance, int showCommand)
{
    WNDCLASS windowClass;
    windowClass.style = CS_DBLCLKS;
    windowClass.hIcon = (HICON)LoadImage(instance, IDI_APPLICATION, IMAGE_ICON, 0, 0,
                                         LR_DEFAULTSIZE);
    windowClass.hCursor = LoadCursorFromFile("blank.cur");
    windowClass.lpfnWndProc = UnknownWindowProc4a0d50;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = instance;
    windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClass.lpszMenuName = 0;
    windowClass.lpszClassName = g_TrackGame->companyName;
    if (!RegisterClass(&windowClass))
        return 0;

    g_MainWindow = CreateWindowEx(WS_EX_APPWINDOW, g_TrackGame->companyName,
                                  g_TrackGame->applicationName, WS_POPUP | WS_VISIBLE, 0, 0,
                                  GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                                  0, 0, instance, 0);
    if (!g_MainWindow) {
        GetLastError();
        return 0;
    }
    UnknownFunction447910(g_MainWindow);
    return 1;
}

// 0x004a0d50
LRESULT CALLBACK UnknownWindowProc4a0d50(HWND window, UINT message, WPARAM wParam,
                                         LPARAM lParam)
{
    RECT rect;
    switch (message) {
    case WM_KILLFOCUS:
        if (g_TrackGame->controlInterface)
            ((PCControlInterface*)g_TrackGame->controlInterface)->SetDevicesAcquired(0);
        break;
    case WM_SETFOCUS:
        if (g_TrackGame->controlInterface)
            ((PCControlInterface*)g_TrackGame->controlInterface)->SetDevicesAcquired(1);
        break;
    case WM_MOVE:
    case WM_SIZE:
        if (g_MainWindowShown && IsIconic(g_MainWindow)) {
            g_TrackGame->UnknownVirtualSlot6();
        } else {
            GetClientRect(window, &rect);
            ClientToScreen(window, (POINT*)&rect.left);
            ClientToScreen(window, (POINT*)&rect.right);
            g_TrackGame->SetWindowRect((UnknownRect*)&rect);
        }
        break;
    case WM_CLOSE:
        g_TrackGame->UnknownVirtualSlot15();
        DestroyWindow(g_MainWindow);
        PostQuitMessage(0);
        return 0;
    case WM_ACTIVATEAPP:
        g_AppActive = wParam;
        if (wParam) {
            g_TrackGame->UnknownVirtualSlot5();
            if (g_TrackGame->controlInterface)
                ((PCControlInterface*)g_TrackGame->controlInterface)->SetDevicesAcquired(1);
            if (g_TrackGame->imeLibrary)
                UnknownFunction52ff20();
        } else {
            g_TrackGame->UnknownVirtualSlot6();
        }
        break;
    case WM_CHAR:
        g_TrackGame->UnknownVirtualSlot35(wParam);
        break;
    case WM_KEYDOWN:
        g_TrackGame->UnknownVirtualSlot36(wParam);
        break;
    case WM_SYSCOMMAND:
        // No menu, screen saver or power saving while the game is active.
        switch (wParam & ~0xf) {
        case SC_KEYMENU:
        case SC_SCREENSAVE:
        case SC_MONITORPOWER:
        case 0xf190:
            if (g_AppActive)
                return 0;
            break;
        }
        break;
    }
    return DefWindowProc(window, message, wParam, lParam);
}
