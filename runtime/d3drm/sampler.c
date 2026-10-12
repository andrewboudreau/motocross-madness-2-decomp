/*
 * Statistical profiler for the main thread, for finding where retail code
 * spends its time when the game runs but shows nothing.
 *
 * Set MCM2_SAMPLE=<seconds> to enable: after that many seconds the sampler
 * suspends the main thread every few milliseconds, records its EIP and the
 * nearest return addresses into mcm2.exe found on its stack, and logs the
 * most frequent ones. Return addresses are found by scanning the stack for
 * values inside the executable's code that follow a call instruction, so a
 * few stale entries are expected.
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

#define SAMPLES 1500
#define CALLERS 4
#define TOP 20

static HANDLE s_mainThread;
static DWORD s_delaySeconds;
static BYTE* s_codeStart;
static BYTE* s_codeEnd;

typedef struct Count {
    DWORD address;
    int count;
} Count;

static Count s_eip[SAMPLES];
static Count s_caller[SAMPLES * CALLERS];

static void Add(Count* table, int capacity, DWORD address)
{
    int i;
    for (i = 0; i < capacity && table[i].count; i++)
        if (table[i].address == address) {
            table[i].count++;
            return;
        }
    if (i < capacity) {
        table[i].address = address;
        table[i].count = 1;
    }
}

static int ByCount(const void* a, const void* b)
{
    return ((const Count*)b)->count - ((const Count*)a)->count;
}

static int InCode(DWORD address)
{
    return (BYTE*)address >= s_codeStart + 6 && (BYTE*)address < s_codeEnd;
}

/* True when the bytes before `address` end in a call instruction. */
static int FollowsCall(DWORD address)
{
    const BYTE* p = (const BYTE*)address;
    return p[-5] == 0xe8 || (p[-2] == 0xff && (p[-1] & 0x38) == 0x10) ||
           (p[-3] == 0xff && (p[-2] & 0x38) == 0x10) || (p[-6] == 0xff && (p[-5] & 0x38) == 0x10);
}

/* Names `address` as module!nearest-export+offset when it lies in a DLL. */
static void Describe(DWORD address, char* out, int size)
{
    MEMORY_BASIC_INFORMATION info;
    char path[MAX_PATH];
    const char* name;
    BYTE* base;
    IMAGE_NT_HEADERS* nt;
    IMAGE_DATA_DIRECTORY* dir;
    const char* best = NULL;
    DWORD bestRva = 0, rva;

    out[0] = 0;
    if (!VirtualQuery((void*)address, &info, sizeof(info)) || !info.AllocationBase)
        return;
    base = (BYTE*)info.AllocationBase;
    if (base == s_codeStart - ((IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew))
                                  ->OptionalHeader.BaseOfCode)
        return;
    if (!GetModuleFileNameA((HMODULE)base, path, sizeof(path)))
        return;
    name = strrchr(path, '\\') ? strrchr(path, '\\') + 1 : path;
    rva = address - (DWORD)base;
    nt = (IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew);
    dir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (dir->VirtualAddress) {
        IMAGE_EXPORT_DIRECTORY* exports = (IMAGE_EXPORT_DIRECTORY*)(base + dir->VirtualAddress);
        DWORD* functions = (DWORD*)(base + exports->AddressOfFunctions);
        DWORD* names = (DWORD*)(base + exports->AddressOfNames);
        WORD* ordinals = (WORD*)(base + exports->AddressOfNameOrdinals);
        DWORD i;
        for (i = 0; i < exports->NumberOfNames; i++) {
            DWORD f = functions[ordinals[i]];
            if (f <= rva && f > bestRva) {
                bestRva = f;
                best = (const char*)(base + names[i]);
            }
        }
    }
    if (best)
        _snprintf(out, size, "%s!%s+0x%lx", name, best, rva - bestRva);
    else
        _snprintf(out, size, "%s+0x%lx", name, rva);
    out[size - 1] = 0;
}

static void Report(const char* title, Count* table, int capacity, int samples)
{
    char where[128];
    int i;
    qsort(table, capacity, sizeof(Count), ByCount);
    LogLine("sampler: %s", title);
    for (i = 0; i < TOP && table[i].count; i++) {
        Describe(table[i].address, where, sizeof(where));
        LogLine("sampler:   0x%08lx  %3d%%  (%d) %s", table[i].address,
                table[i].count * 100 / samples, table[i].count, where);
    }
}

static DWORD WINAPI SamplerThread(void* unused)
{
    HANDLE thread;
    CONTEXT context;
    int i, samples = 0, outside = 0;

    (void)unused;
    Sleep(s_delaySeconds * 1000);
    thread = s_mainThread;
    LogLine("sampler: sampling the main thread");
    for (i = 0; i < SAMPLES; i++) {
        const DWORD* stack;
        int found = 0, depth;
        Sleep(3);
        if (SuspendThread(thread) == (DWORD)-1)
            break;
        context.ContextFlags = CONTEXT_CONTROL;
        if (GetThreadContext(thread, &context)) {
            samples++;
            if (!InCode(context.Eip))
                outside++;
            Add(s_eip, SAMPLES, context.Eip);
            stack = (const DWORD*)context.Esp;
            for (depth = 0; depth < 1024 && found < CALLERS; depth++) {
                DWORD value;
                if (IsBadReadPtr(stack + depth, 4))
                    break;
                value = stack[depth];
                if (InCode(value) && FollowsCall(value)) {
                    /* Count each caller once per sample. */
                    Add(s_caller, SAMPLES * CALLERS, value);
                    found++;
                }
            }
        }
        ResumeThread(thread);
    }
    CloseHandle(thread);
    LogLine("sampler: %d samples, %d%% outside mcm2.exe code", samples,
            samples ? outside * 100 / samples : 0);
    Report("hottest EIPs", s_eip, SAMPLES, samples);
    Report("hottest return addresses into mcm2.exe", s_caller, SAMPLES * CALLERS, samples);
    return 0;
}

void InstallSampler(void)
{
    char value[16];
    BYTE* base = (BYTE*)GetModuleHandleA(NULL);
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew);

    if (!GetEnvironmentVariableA("MCM2_SAMPLE", value, sizeof(value)))
        return;
    s_delaySeconds = atoi(value);
    if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
                         &s_mainThread, THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, 0))
        return;
    s_codeStart = base + nt->OptionalHeader.BaseOfCode;
    s_codeEnd = s_codeStart + nt->OptionalHeader.SizeOfCode;
    CloseHandle(CreateThread(NULL, 0, SamplerThread, NULL, 0, NULL));
    LogLine("sampler: will sample the main thread after %lu s", s_delaySeconds);
}
