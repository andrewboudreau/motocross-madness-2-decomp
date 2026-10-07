#pragma once

// Main.cpp (provisional name, fits the Lzw.cpp .. matrix link-order
// bracket): the program entry unit
// 0x004a05e0..0x004a10d5 (WinMain, the start-up, the main window and its
// window procedure). No __FILE__, no RTTI. See docs/MAIN.md.
#include <windows.h>

// 0x004a05e0: EnumWindows callback; restores a window titled like the game
// and sets *(int*)lParam (another instance runs).
BOOL CALLBACK UnknownEnumWindowsProc4a05e0(HWND window, LPARAM found);
// 0x004a0680: start-up checks, the main window and the message loop.
int WINAPI UnknownFunction4a0680(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine,
                                 int showCommand);
// 0x004a0c50: registers the window class and creates the full-screen window.
int UnknownFunction4a0c50(HINSTANCE instance, int showCommand);
// 0x004a0d50: the main window's window procedure.
LRESULT CALLBACK UnknownWindowProc4a0d50(HWND window, UINT message, WPARAM wParam,
                                         LPARAM lParam);

extern HWND g_MainWindow;            // 0x00685024
extern int g_MainWindowShown;        // 0x00685028: set before the message loop
extern int g_AppActive;              // 0x0056de98: WM_ACTIVATEAPP state, 1 initially
// 0x0056e270 (0x700, next to g_TrackGame): the lowest DirectX version
// GetDXVersion may report.
extern unsigned long g_RequiredDXVersion;
