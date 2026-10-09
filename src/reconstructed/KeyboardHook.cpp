#include <windows.h>

// Alt-Tab / Alt-Esc keyboard hook, 0x005053b0..0x0050547b. No source string
// attributes the unit; it sits between SurfaceMap (0x005051ed) and Terrain.cpp's
// static initializers (0x00505480), so the file name is descriptive. The
// cdecl entry 0x005053b0 is called by KrustyUI's shutdown step with 0
// (docs/KRUSTYUI.md); the hook procedure swallows VK_TAB and VK_ESCAPE while
// the ALT key is down (KF_ALTDOWN in the high word of lParam) so the game
// cannot be task-switched away from while the flag is clear.

static int g_taskSwitchAllowed = 1;        // 0x00574684
static HHOOK g_taskSwitchHook;             // 0x00689fdc

static int InstallTaskSwitchHook();
static void RemoveTaskSwitchHook();

// 0x005053d0 (WH_KEYBOARD hook procedure). The key flags are the high word
// of lParam; retail shifts the signed LPARAM (sar) and tests the 32-bit
// result, which is the WORD cast of the signed shift (the SDK HIWORD macro
// casts to DWORD first and would shift unsigned; the plain
// `(lParam >> 16) & KF_ALTDOWN` folds into one test against 0x20000000).
static LRESULT CALLBACK TaskSwitchHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (!g_taskSwitchAllowed && code >= 0 && (wParam == VK_TAB || wParam == VK_ESCAPE)) {
        if ((WORD)(lParam >> 16) & KF_ALTDOWN) {
            return 1;
        }
    }
    return CallNextHookEx(g_taskSwitchHook, code, wParam, lParam);
}

// 0x005053b0: records the flag and installs or removes the hook accordingly.
void SetTaskSwitchAllowed(int allowed)
{
    g_taskSwitchAllowed = allowed;
    if (allowed) {
        RemoveTaskSwitchHook();
    } else {
        InstallTaskSwitchHook();
    }
}

// 0x00505420: installs the hook once while task switching is disallowed;
// returns whether a hook is installed.
static int InstallTaskSwitchHook()
{
    if (!g_taskSwitchHook && !g_taskSwitchAllowed) {
        g_taskSwitchHook = SetWindowsHookEx(WH_KEYBOARD, TaskSwitchHookProc, 0, 0);
    }
    return g_taskSwitchHook != 0;
}

// 0x00505460
static void RemoveTaskSwitchHook()
{
    if (g_taskSwitchHook) {
        UnhookWindowsHookEx(g_taskSwitchHook);
    }
    g_taskSwitchHook = 0;
}
