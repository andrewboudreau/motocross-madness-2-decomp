/*
 * Start-up tracing through the executable's import address table.
 *
 * Each entry below replaces one IAT slot of mcm2.exe with a logging wrapper
 * that calls the original. Only calls the game makes through its imports are
 * seen (the CRT inside the exe also uses these imports). This patches data,
 * not code: game functions are untouched.
 */
#include <windows.h>
#include <string.h>

#include "runtime.h"

typedef HANDLE(WINAPI* CreateFileAProc)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD,
                                        DWORD, HANDLE);
typedef HANDLE(WINAPI* FindFirstFileAProc)(LPCSTR, LPWIN32_FIND_DATAA);
typedef void(WINAPI* OutputDebugStringAProc)(LPCSTR);
typedef int(WINAPI* MessageBoxAProc)(HWND, LPCSTR, LPCSTR, UINT);
typedef void(WINAPI* ExitProcessProc)(UINT);
typedef void(WINAPI* PostQuitMessageProc)(int);
typedef HWND(WINAPI* CreateWindowExAProc)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND,
                                          HMENU, HINSTANCE, LPVOID);
typedef HMODULE(WINAPI* LoadLibraryAProc)(LPCSTR);
typedef HRESULT(WINAPI* DirectDrawCreateExProc)(GUID*, void**, REFIID, IUnknown*);
typedef HRESULT(WINAPI* DirectDrawEnumerateAProc)(void*, void*);
typedef HRESULT(WINAPI* DirectInputCreateExProc)(HINSTANCE, DWORD, REFIID, void**, IUnknown*);
typedef HRESULT(WINAPI* DirectSoundCreateProc)(GUID*, void**, IUnknown*);
typedef HRESULT(WINAPI* CoCreateInstanceProc)(REFCLSID, IUnknown*, DWORD, REFIID, void**);

static CreateFileAProc s_CreateFileA;
static FindFirstFileAProc s_FindFirstFileA;
static OutputDebugStringAProc s_OutputDebugStringA;
static MessageBoxAProc s_MessageBoxA;
static ExitProcessProc s_ExitProcess;
static PostQuitMessageProc s_PostQuitMessage;
static CreateWindowExAProc s_CreateWindowExA;
static BOOL(WINAPI* s_SetWindowPos)(HWND, HWND, int, int, int, int, UINT);
static LONG(WINAPI* s_SetWindowLongA)(HWND, int, LONG);
static BOOL(WINAPI* s_ShowWindow)(HWND, int);
static BOOL(WINAPI* s_GetCursorPos)(POINT*);
static LoadLibraryAProc s_LoadLibraryA;
static DirectDrawCreateExProc s_DirectDrawCreateEx;
static DirectDrawEnumerateAProc s_DirectDrawEnumerateA;
static DirectInputCreateExProc s_DirectInputCreateEx;
static DirectSoundCreateProc s_DirectSoundCreate;
static CoCreateInstanceProc s_CoCreateInstance;
static FARPROC(WINAPI* s_BladeGetProcAddress)(HMODULE, LPCSTR);
static UINT(WINAPI* s_timeBeginPeriod)(UINT);
static UINT(WINAPI* s_timeEndPeriod)(UINT);
static DirectDrawCreateExProc s_InnerDirectDrawCreateEx;

static void LogGuid(const char* label, const GUID* guid)
{
    if (!guid) {
        LogLine("  %s NULL", label);
        return;
    }
    LogLine("  %s {%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", label, guid->Data1,
            guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2],
            guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

static HANDLE WINAPI TraceCreateFileA(LPCSTR name, DWORD access, DWORD share,
                                      LPSECURITY_ATTRIBUTES security, DWORD disposition,
                                      DWORD flags, HANDLE templateFile)
{
    HANDLE result = s_CreateFileA(name, access, share, security, disposition, flags, templateFile);
    DWORD error = GetLastError();
    if (result == INVALID_HANDLE_VALUE)
        LogLine("CreateFileA(\"%s\", access 0x%lx) failed, error %lu", name ? name : "(null)",
                access, error);
    else if (access & GENERIC_WRITE)
        LogLine("CreateFileA(\"%s\") for writing", name);
    SetLastError(error);
    return result;
}

static HANDLE WINAPI TraceFindFirstFileA(LPCSTR pattern, LPWIN32_FIND_DATAA data)
{
    HANDLE result = s_FindFirstFileA(pattern, data);
    DWORD error = GetLastError();
    if (result == INVALID_HANDLE_VALUE)
        LogLine("FindFirstFileA(\"%s\") found nothing, error %lu", pattern ? pattern : "(null)",
                error);
    SetLastError(error);
    return result;
}

static void WINAPI TraceOutputDebugStringA(LPCSTR text)
{
    char line[400];
    int length;

    if (text) {
        lstrcpynA(line, text, sizeof(line));
        length = lstrlenA(line);
        while (length && (line[length - 1] == '\n' || line[length - 1] == '\r'))
            line[--length] = 0;
        LogLine("debug: %s", line);
    }
    s_OutputDebugStringA(text);
}

static int WINAPI TraceMessageBoxA(HWND owner, LPCSTR text, LPCSTR caption, UINT type)
{
    int result;
    LogLine("MessageBoxA(\"%s\", \"%s\", 0x%x)", caption ? caption : "", text ? text : "", type);
    result = s_MessageBoxA(owner, text, caption, type);
    LogLine("  -> %d", result);
    return result;
}

static void WINAPI TraceExitProcess(UINT code)
{
    LogLine("ExitProcess(%u) called from 0x%08lx", code, CALLER_OF(code));
    s_ExitProcess(code);
}

static void WINAPI TracePostQuitMessage(int code)
{
    LogLine("PostQuitMessage(%d) called from 0x%08lx", code, CALLER_OF(code));
    s_PostQuitMessage(code);
}

static BOOL WINAPI TraceSetWindowPos(HWND window, HWND after, int x, int y, int width, int height,
                                     UINT flags)
{
    LogLine("SetWindowPos(%p, %d,%d %dx%d, flags 0x%x) called from 0x%08lx", window, x, y, width,
            height, flags, CALLER_OF(window));
    return s_SetWindowPos(window, after, x, y, width, height, flags);
}

static LONG WINAPI TraceSetWindowLongA(HWND window, int index, LONG value)
{
    LogLine("SetWindowLongA(%p, %d, 0x%lx) called from 0x%08lx", window, index, value,
            CALLER_OF(window));
    return s_SetWindowLongA(window, index, value);
}

static BOOL WINAPI TraceShowWindow(HWND window, int command)
{
    LogLine("ShowWindow(%p, %d) called from 0x%08lx", window, command, CALLER_OF(window));
    return s_ShowWindow(window, command);
}

static BOOL WINAPI GameGetCursorPos(POINT* point)
{
    static LONG s_logged;
    BOOL result = s_GetCursorPos(point);
    if (InterlockedIncrement(&s_logged) <= 3)
        LogLine("GetCursorPos called from 0x%08lx", CALLER_OF(point));
    if (result && EmulatesFullscreen())
        MapCursorToGame(point);
    return result;
}

static HWND WINAPI TraceCreateWindowExA(DWORD exStyle, LPCSTR className, LPCSTR title,
                                        DWORD style, int x, int y, int width, int height,
                                        HWND parent, HMENU menu, HINSTANCE instance, LPVOID param)
{
    HWND result = s_CreateWindowExA(exStyle, className, title, style, x, y, width, height, parent,
                                    menu, instance, param);
    DWORD error = GetLastError();
    LogLine("CreateWindowExA(\"%s\", %dx%d) -> %p (error %lu)",
            title ? title : "", width, height, result, result ? 0 : error);
    SetLastError(error);
    return result;
}

static HMODULE WINAPI TraceLoadLibraryA(LPCSTR name)
{
    HMODULE result = s_LoadLibraryA(name);
    DWORD error = GetLastError();
    LogLine("LoadLibraryA(\"%s\") -> %p", name ? name : "(null)", result);
    SetLastError(error);
    return result;
}

static HRESULT WINAPI TraceDirectDrawCreateEx(GUID* guid, void** out, REFIID iid, IUnknown* outer)
{
    HRESULT result = s_DirectDrawCreateEx(guid, out, iid, outer);
    LogLine("blade DirectDrawCreateEx -> 0x%08lx, object %p", result, out ? *out : 0);
    LogGuid("device", guid);
    if (result == 0 && out)
        TraceDirectDrawObject(*out);
    return result;
}

static HRESULT WINAPI TraceDirectDrawEnumerateA(void* callback, void* context)
{
    HRESULT result = s_DirectDrawEnumerateA(callback, context);
    LogLine("blade DirectDrawEnumerateA -> 0x%08lx", result);
    return result;
}

static HRESULT WINAPI TraceDirectInputCreateEx(HINSTANCE instance, DWORD version, REFIID iid,
                                               void** out, IUnknown* outer)
{
    HRESULT result = s_DirectInputCreateEx(instance, version, iid, out, outer);
    LogLine("DirectInputCreateEx(version 0x%lx) -> 0x%08lx", version, result);
    if (result == 0 && out)
        TraceDirectInputObject(*out);
    return result;
}

static HRESULT WINAPI TraceDirectSoundCreate(GUID* guid, void** out, IUnknown* outer)
{
    HRESULT result = s_DirectSoundCreate(guid, out, outer);
    LogLine("DirectSoundCreate -> 0x%08lx", result);
    return result;
}

static HRESULT WINAPI TraceCoCreateInstance(REFCLSID clsid, IUnknown* outer, DWORD context,
                                            REFIID iid, void** out)
{
    HRESULT result = s_CoCreateInstance(clsid, outer, context, iid, out);
    LogLine("CoCreateInstance -> 0x%08lx", result);
    LogGuid("clsid", clsid);
    return result;
}

/*
 * Blade forwards to the system DirectDraw, which it finds with
 * GetProcAddress(ddraw, "DirectDrawCreateEx"). Its objects are traced (and
 * see the emulated display mode) through TraceInnerDirectDrawObject.
 */
static HRESULT WINAPI InnerDirectDrawCreateEx(GUID* guid, void** out, REFIID iid, IUnknown* outer)
{
    HRESULT result = s_InnerDirectDrawCreateEx(guid, out, iid, outer);
    LogLine("blade -> ddraw DirectDrawCreateEx -> 0x%08lx, object %p", result, out ? *out : 0);
    if (result == 0 && out)
        TraceInnerDirectDrawObject(*out);
    return result;
}

static FARPROC WINAPI BladeGetProcAddress(HMODULE module, LPCSTR name)
{
    FARPROC result = s_BladeGetProcAddress(module, name);
    if (result && HIWORD(name) && !strcmp(name, "DirectDrawCreateEx")) {
        s_InnerDirectDrawCreateEx = (DirectDrawCreateExProc)result;
        return (FARPROC)InnerDirectDrawCreateEx;
    }
    return result;
}

/*
 * The game's clock (0x4bfa80) brackets every timeGetTime with
 * timeBeginPeriod(1)/timeEndPeriod(1). Each pair reprograms the system timer
 * (NtSetTimerResolution), which took most of the game's CPU time. Raise the
 * resolution once for the life of the process and ignore the rest.
 */
static UINT WINAPI GameTimeBeginPeriod(UINT period)
{
    static LONG s_raised;
    if (!InterlockedExchange(&s_raised, 1)) {
        UINT result = s_timeBeginPeriod(period);
        LogLine("timer: timeBeginPeriod(%u) -> %u, kept for the process", period, result);
        return result;
    }
    return 0;
}

static UINT WINAPI GameTimeEndPeriod(UINT period)
{
    (void)period;
    return 0;
}

typedef struct ImportHook {
    const char* module;
    const char* name;     /* NULL: match by ordinal */
    WORD ordinal;
    void* replacement;
    void** original;
} ImportHook;

static ImportHook s_hooks[] = {
    {"KERNEL32.dll", "CreateFileA", 0, TraceCreateFileA, (void**)&s_CreateFileA},
    {"KERNEL32.dll", "FindFirstFileA", 0, TraceFindFirstFileA, (void**)&s_FindFirstFileA},
    {"KERNEL32.dll", "OutputDebugStringA", 0, TraceOutputDebugStringA,
     (void**)&s_OutputDebugStringA},
    {"KERNEL32.dll", "ExitProcess", 0, TraceExitProcess, (void**)&s_ExitProcess},
    {"KERNEL32.dll", "LoadLibraryA", 0, TraceLoadLibraryA, (void**)&s_LoadLibraryA},
    {"USER32.dll", "MessageBoxA", 0, TraceMessageBoxA, (void**)&s_MessageBoxA},
    {"USER32.dll", "PostQuitMessage", 0, TracePostQuitMessage, (void**)&s_PostQuitMessage},
    {"USER32.dll", "CreateWindowExA", 0, TraceCreateWindowExA, (void**)&s_CreateWindowExA},
    {"USER32.dll", "SetWindowPos", 0, TraceSetWindowPos, (void**)&s_SetWindowPos},
    {"USER32.dll", "SetWindowLongA", 0, TraceSetWindowLongA, (void**)&s_SetWindowLongA},
    {"USER32.dll", "ShowWindow", 0, TraceShowWindow, (void**)&s_ShowWindow},
    {"USER32.dll", "GetCursorPos", 0, GameGetCursorPos, (void**)&s_GetCursorPos},
    {"blade.dll", "DirectDrawCreateEx", 0, TraceDirectDrawCreateEx, (void**)&s_DirectDrawCreateEx},
    {"blade.dll", "DirectDrawEnumerateA", 0, TraceDirectDrawEnumerateA,
     (void**)&s_DirectDrawEnumerateA},
    {"DINPUT.dll", "DirectInputCreateEx", 0, TraceDirectInputCreateEx,
     (void**)&s_DirectInputCreateEx},
    {"DSOUND.dll", NULL, 1, TraceDirectSoundCreate, (void**)&s_DirectSoundCreate},
    {"WINMM.dll", "timeBeginPeriod", 0, GameTimeBeginPeriod, (void**)&s_timeBeginPeriod},
    {"WINMM.dll", "timeEndPeriod", 0, GameTimeEndPeriod, (void**)&s_timeEndPeriod},
    {"ole32.dll", "CoCreateInstance", 0, TraceCoCreateInstance, (void**)&s_CoCreateInstance},
};

static ImportHook s_bladeHook = {"KERNEL32.dll", "GetProcAddress", 0, BladeGetProcAddress,
                                 (void**)&s_BladeGetProcAddress};

static int PatchImport(BYTE* base, const ImportHook* hook)
{
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew);
    IMAGE_DATA_DIRECTORY* dir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    IMAGE_IMPORT_DESCRIPTOR* desc = (IMAGE_IMPORT_DESCRIPTOR*)(base + dir->VirtualAddress);
    IMAGE_THUNK_DATA* names;
    IMAGE_THUNK_DATA* slots;
    IMAGE_IMPORT_BY_NAME* byName;
    DWORD oldProtect;
    int match;

    for (; desc->Name; desc++) {
        if (lstrcmpiA((char*)(base + desc->Name), hook->module))
            continue;
        names = (IMAGE_THUNK_DATA*)(base + desc->OriginalFirstThunk);
        slots = (IMAGE_THUNK_DATA*)(base + desc->FirstThunk);
        for (; names->u1.AddressOfData; names++, slots++) {
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) {
                match = !hook->name && IMAGE_ORDINAL(names->u1.Ordinal) == hook->ordinal;
            } else {
                byName = (IMAGE_IMPORT_BY_NAME*)(base + (DWORD)names->u1.AddressOfData);
                match = hook->name && !strcmp((char*)byName->Name, hook->name);
            }
            if (!match)
                continue;
            *hook->original = (void*)slots->u1.Function;
            VirtualProtect(&slots->u1.Function, sizeof(DWORD), PAGE_READWRITE, &oldProtect);
            *(void**)&slots->u1.Function = hook->replacement;
            VirtualProtect(&slots->u1.Function, sizeof(DWORD), oldProtect, &oldProtect);
            return 1;
        }
    }
    return 0;
}

void InstallStartupTrace(void)
{
    BYTE* base = (BYTE*)GetModuleHandleA(NULL);
    int i, patched = 0;

    for (i = 0; i < sizeof(s_hooks) / sizeof(s_hooks[0]); i++) {
        if (PatchImport(base, &s_hooks[i]))
            patched++;
        else
            LogLine("trace: import %s!%s not found", s_hooks[i].module,
                    s_hooks[i].name ? s_hooks[i].name : "(ordinal)");
    }
    LogLine("trace: patched %d import slots", patched);
    base = (BYTE*)GetModuleHandleA("blade.dll");
    if (base && PatchImport(base, &s_bladeHook))
        LogLine("trace: patched blade.dll's GetProcAddress import");
    else
        LogLine("trace: blade.dll's GetProcAddress import not found");
}
