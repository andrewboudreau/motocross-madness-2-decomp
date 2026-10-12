/*
 * Replacement dplayx.dll for the MCM2 demo executable.
 *
 * Retail imports one DirectPlay function, DirectPlayLobbyCreateA by ordinal
 * 4, which only ConnectUsingLobby calls (samples/net/NetNearMisses.cpp); a
 * negative result makes it report an error and give up. On Windows 10/11 the
 * system dplayx.dll is a stub that blocks the process loader on the
 * "install DirectPlay" feature prompt, so this module reports DirectPlay as
 * unavailable instead. Lobby-launched multiplayer is therefore off.
 */
#include <windows.h>

#define DPERR_UNAVAILABLE ((HRESULT)0x887700faL) /* MAKE_DPHRESULT(250), DPLAY.H */

HRESULT __stdcall DirectPlayLobbyCreateA(GUID* provider, void** lobby, void* outer, void* data,
                                         DWORD dataSize)
{
    if (lobby)
        *lobby = NULL;
    return DPERR_UNAVAILABLE;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(instance);
    return TRUE;
}
