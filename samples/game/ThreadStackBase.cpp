#include "ThreadStackBase.h"

DWORD* ReadCurrentThreadStackBase() {
    typedef PNT_TIB (__cdecl *CurrentTebFunction)();
    HMODULE module = GetModuleHandleA("ntdll.dll");
    CurrentTebFunction currentTeb = module ?
        (CurrentTebFunction)GetProcAddress(module, "NtCurrentTeb") : 0;
    if (!currentTeb) {
        // The reporter calls this inside its stack-dump SEH region. An OS
        // without the export follows the existing failed-dump reporting path.
        RaiseException(0xc0000002, EXCEPTION_NONCONTINUABLE, 0, 0);
        return 0;
    }
    return (DWORD*)currentTeb()->StackBase;
}
