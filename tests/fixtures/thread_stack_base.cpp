#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#include "ThreadStackBase.h"

static int missingModule, moduleLookups, exportLookups;
static HMODULE WINAPI MissingModule(const char*) { ++moduleLookups; return missingModule ? 0 : (HMODULE)1; }
static FARPROC WINAPI MissingExport(HMODULE, const char*) { ++exportLookups; return 0; }
#define ReadCurrentThreadStackBase MissingStackBase
#define GetModuleHandleA MissingModule
#define GetProcAddress MissingExport
#include "../../samples/game/ThreadStackBase.cpp"
#undef ReadCurrentThreadStackBase
#undef GetModuleHandleA
#undef GetProcAddress

static int FailureFilter(EXCEPTION_POINTERS* data) {
    if (data->ExceptionRecord->ExceptionCode != 0xc0000002 ||
        data->ExceptionRecord->ExceptionFlags != EXCEPTION_NONCONTINUABLE) ExitProcess(7);
    return EXCEPTION_EXECUTE_HANDLER;
}

typedef void (WINAPI *StackLimitsFunction)(DWORD*, DWORD*);
static StackLimitsFunction limits;
static void* mainFiber;
static LONG checks;
static int failure;
static void Check() {
    for (int i = 0; i < 256; ++i) {
        DWORD low, high;
        limits(&low, &high);
        DWORD* base = ReadCurrentThreadStackBase();
        if ((DWORD)base != high || (DWORD)&low < low || (DWORD)&low >= high)
            InterlockedExchange((LONG*)&failure, 1);
        InterlockedIncrement(&checks);
    }
}
static DWORD WINAPI Worker(void*) { Check(); return 0; }
static void WINAPI Fiber(void*) { Check(); SwitchToFiber(mainFiber); }
int main() {
    limits = (StackLimitsFunction)GetProcAddress(GetModuleHandleA("kernel32.dll"),
                                                "GetCurrentThreadStackLimits");
    if (!limits) return 2;
    Check();
    for (int i = 0; i < 3; ++i) {
        DWORD id;
        HANDLE thread = CreateThread(0, (i + 1) * 0x20000, Worker, 0, 0, &id);
        if (!thread) return 3;
        WaitForSingleObject(thread, INFINITE); CloseHandle(thread);
    }
    mainFiber = ConvertThreadToFiber(0);
    if (!mainFiber) return 4;
    for (i = 0; i < 2; ++i) {
        void* fiber = CreateFiber((i + 1) * 0x40000, Fiber, 0);
        if (!fiber) return 5;
        SwitchToFiber(fiber); DeleteFiber(fiber);
    }
    if (failure || checks != 1536) return 6;
    for (i = 0; i < 2; ++i) {
        missingModule = i; moduleLookups = exportLookups = 0;
        int caught = 0;
        __try { MissingStackBase(); }
        __except (FailureFilter(GetExceptionInformation())) { caught = 1; }
        if (!caught || moduleLookups != 1 || exportLookups != !missingModule) return 8;
    }
    printf("1536 actual stack bounds match independent OS limits on main, worker and fiber stacks\n");
    printf("Both unavailable-backend paths raise the guarded stack-dump exception\n");
    return 0;
}
