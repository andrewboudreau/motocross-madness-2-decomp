// Private differential validation: compare the reconstructed startup probe
// with the original function loaded from the user's hash-pinned executable.
// This is a test harness, never a provider in the source game build.
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <string.h>

static BOOL WINAPI ProbeVersion(OSVERSIONINFO* info);
static HMODULE WINAPI ProbeLoadLibrary(const char* name);
static FARPROC WINAPI ProbeGetProcAddress(HMODULE module, const char* name);
static BOOL WINAPI ProbeFreeLibrary(HMODULE module);

#undef GetVersionEx
#undef LoadLibrary
#define GetVersionEx ProbeVersion
#define LoadLibrary ProbeLoadLibrary
#define GetProcAddress ProbeGetProcAddress
#define FreeLibrary ProbeFreeLibrary
#include "../../samples/game/DeviceSetupNearMisses.cpp"
#undef GetVersionEx
#undef LoadLibrary
#undef GetProcAddress
#undef FreeLibrary

struct Scenario {
    DWORD platform, major, build;
    int fail;
};
enum {
    FAIL_NONE, FAIL_VERSION, FAIL_BLADE, FAIL_DRAW_LOAD,
    FAIL_DRAW_PROC, FAIL_DRAW_CREATE, FAIL_DRAW2, FAIL_INPUT_LOAD,
    FAIL_INPUT_PROC, FAIL_COOPERATIVE, FAIL_SURFACE, FAIL_SURFACE3,
    FAIL_SURFACE4, FAIL_DRAW7_PROC, FAIL_DRAW7_CREATE
};
struct Observation {
    DWORD version, platform, blade;
    unsigned int count;
    int calls[80];
};
struct MockObject { void** vtable; int kind; };
static Scenario current;
static Observation observation;
static void* drawMethods[21];
static void* surfaceMethods[3];
static MockObject draw = {drawMethods, 1};
static MockObject draw2 = {surfaceMethods, 2};
static MockObject surface = {surfaceMethods, 3};
static MockObject surface3 = {surfaceMethods, 4};
static MockObject surface4 = {surfaceMethods, 5};
static MockObject draw7 = {surfaceMethods, 6};
// Empty test-owned address space, not embedded code/data. It prevents the
// Windows loader from assigning unrelated mappings at retail's probe pages.
static unsigned char referenceSpace[0x200000];

static void Record(int value) {
    if (observation.count >= 80) ExitProcess(90);
    observation.calls[observation.count++] = value;
}
static BOOL WINAPI ProbeVersion(OSVERSIONINFO* info) {
    Record(1);
    if (info->dwOSVersionInfoSize != sizeof(*info)) ExitProcess(91);
    info->dwPlatformId = current.platform;
    info->dwMajorVersion = current.major;
    info->dwBuildNumber = current.build;
    return current.fail != FAIL_VERSION;
}
static HMODULE WINAPI ProbeLoadLibrary(const char* name) {
    if (!strcmp(name, "Blade.dll")) {
        Record(2);
        return current.fail == FAIL_BLADE || current.fail == FAIL_DRAW_LOAD
            ? 0 : (HMODULE)1;
    }
    if (!strcmp(name, "DDRAW.DLL")) {
        Record(3);
        return current.fail == FAIL_DRAW_LOAD ? 0 : (HMODULE)2;
    }
    if (!strcmp(name, "DINPUT.DLL")) {
        Record(4);
        return current.fail == FAIL_INPUT_LOAD ? 0 : (HMODULE)3;
    }
    ExitProcess(92);
    return 0;
}
static BOOL WINAPI ProbeFreeLibrary(HMODULE module) {
    Record(10 + (int)module);
    return TRUE;
}
static HRESULT WINAPI MockDrawCreate(GUID*, LPDIRECTDRAW* result, IUnknown*) {
    Record(20);
    *result = (LPDIRECTDRAW)&draw;
    return current.fail == FAIL_DRAW_CREATE ? E_FAIL : S_OK;
}
static HRESULT WINAPI MockDrawCreateEx(GUID*, void** result, REFIID iid, IUnknown*) {
    Record(21);
    if (memcmp(&iid, &IID_IDirectDraw7, sizeof(GUID))) ExitProcess(93);
    *result = &draw7;
    return current.fail == FAIL_DRAW7_CREATE ? E_FAIL : S_OK;
}
static FARPROC WINAPI ProbeGetProcAddress(HMODULE module, const char* name) {
    Record(30 + (int)module);
    if (!strcmp(name, "DirectDrawCreate")) {
        Record(40);
        return current.fail == FAIL_DRAW_PROC ? 0 : (FARPROC)MockDrawCreate;
    }
    if (!strcmp(name, "DirectInputCreateA")) {
        Record(41);
        // The decoded function only tests this address, never calls it.
        return current.fail == FAIL_INPUT_PROC ? 0 : (FARPROC)1;
    }
    if (!strcmp(name, "DirectDrawCreateEx")) {
        Record(42);
        return current.fail == FAIL_DRAW7_PROC ? 0 : (FARPROC)MockDrawCreateEx;
    }
    ExitProcess(94);
    return 0;
}
static HRESULT WINAPI MockQuery(MockObject* object, REFIID iid, void** result) {
    Record(50 + object->kind);
    if (!memcmp(&iid, &IID_IDirectDraw2, sizeof(GUID))) {
        Record(60);
        *result = &draw2;
        return current.fail == FAIL_DRAW2 ? E_FAIL : S_OK;
    }
    if (!memcmp(&iid, &IID_IDirectDrawSurface3, sizeof(GUID))) {
        Record(61);
        *result = &surface3;
        return current.fail == FAIL_SURFACE3 ? E_FAIL : S_OK;
    }
    if (!memcmp(&iid, &IID_IDirectDrawSurface4, sizeof(GUID))) {
        Record(62);
        *result = &surface4;
        return current.fail == FAIL_SURFACE4 ? E_FAIL : S_OK;
    }
    ExitProcess(95);
    return E_FAIL;
}
static ULONG WINAPI MockRelease(MockObject* object) {
    Record(70 + object->kind);
    return 0;
}
static HRESULT WINAPI MockCooperative(MockObject* object, HWND window, DWORD flags) {
    Record(80);
    if (object != &draw || window || flags != DDSCL_NORMAL) ExitProcess(96);
    return current.fail == FAIL_COOPERATIVE ? E_FAIL : S_OK;
}
static HRESULT WINAPI MockCreateSurface(MockObject* object, DDSURFACEDESC* desc,
                                       void** result, IUnknown* outer) {
    Record(81);
    DDSURFACEDESC expected;
    memset(&expected, 0, sizeof(expected));
    expected.dwSize = sizeof(expected);
    expected.dwFlags = DDSD_CAPS;
    expected.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    if (object != &draw || outer || memcmp(desc, &expected, sizeof(expected)))
        ExitProcess(97);
    *result = &surface;
    return current.fail == FAIL_SURFACE ? E_FAIL : S_OK;
}

// Read the four needed private PE pages as-is into the empty reservation;
// only the four observed API IAT slots are redirected. No game bytes enter
// repository source or production. Bounds checks ensure test code and globals
// cannot be overwritten by the private reference pages.
static void* MapReference(const char* path) {
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE) return 0;
    DWORD size = GetFileSize(file, 0), got = 0;
    unsigned char* input = new unsigned char[size];
    BOOL read = ReadFile(file, input, size, &got, 0);
    CloseHandle(file);
    if (!read || got != size || size < sizeof(IMAGE_DOS_HEADER)) return 0;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)input;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0 ||
        (DWORD)dos->e_lfanew > size - sizeof(IMAGE_NT_HEADERS)) return 0;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(input + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != 0x14c ||
        nt->OptionalHeader.ImageBase != 0x400000) return 0;
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    const DWORD pages[] = {0x448000, 0x550000, 0x556000, 0x568000};
    for (int page = 0; page < 4; ++page) {
        void* destination = (void*)pages[page];
        DWORD oldProtection;
        if (pages[page] < (DWORD)referenceSpace ||
            pages[page] + 0x1000 > (DWORD)referenceSpace + sizeof(referenceSpace) ||
            !VirtualProtect(destination, 0x1000, PAGE_EXECUTE_READWRITE, &oldProtection))
            return 0;
        DWORD rva = pages[page] - 0x400000;
        int copied = 0;
        for (unsigned int i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
            IMAGE_SECTION_HEADER* section = sections + i;
            if (rva >= section->VirtualAddress &&
                rva + 0x1000 <= section->VirtualAddress + section->SizeOfRawData) {
                DWORD offset = section->PointerToRawData + rva - section->VirtualAddress;
                if (offset + 0x1000 > size) return 0;
                memcpy(destination, input + offset, 0x1000);
                copied = 1;
            }
        }
        if (!copied) return 0;
    }
    delete[] input;
    *(void**)0x550268 = (void*)ProbeVersion;
    *(void**)0x55026c = (void*)ProbeLoadLibrary;
    *(void**)0x550270 = (void*)ProbeGetProcAddress;
    *(void**)0x550274 = (void*)ProbeFreeLibrary;
    FlushInstructionCache(GetCurrentProcess(), (void*)0x448000, 0x1000);
    return (void*)0x448000;
}

typedef void (*ProbeFunction)(DWORD*, DWORD*, DWORD*);
static Observation Run(ProbeFunction function, Scenario scenario) {
    current = scenario;
    memset(&observation, 0, sizeof(observation));
    observation.version = observation.platform = observation.blade = 0xdeadbeef;
    function(&observation.version, &observation.platform, &observation.blade);
    return observation;
}
int main(int argc, char** argv) {
    if (argc != 2 || !MapReference(argv[1])) {
        puts("Cannot map private reference executable");
        return 1;
    }
    drawMethods[0] = surfaceMethods[0] = (void*)MockQuery;
    drawMethods[2] = surfaceMethods[2] = (void*)MockRelease;
    drawMethods[6] = (void*)MockCreateSurface;
    drawMethods[20] = (void*)MockCooperative;
    unsigned int count = 0;
    const DWORD platforms[] = {VER_PLATFORM_WIN32_NT, VER_PLATFORM_WIN32_WINDOWS, 0};
    const DWORD majors[] = {3, 4, 5, 10};
    const DWORD builds[] = {0x548, 0x549, 0x54a, 0x100054a};
    for (int p = 0; p < 3; ++p)
        for (int m = 0; m < 4; ++m)
            for (int b = 0; b < 4; ++b)
                for (int failure = FAIL_NONE; failure <= FAIL_DRAW7_CREATE; ++failure) {
                    Scenario scenario = {platforms[p], majors[m], builds[b], failure};
                    Observation original = Run((ProbeFunction)0x448560, scenario);
                    Observation candidate = Run(GetDXVersion, scenario);
                    if (memcmp(&original, &candidate, sizeof(original))) {
                        printf("Mismatch platform=%lu major=%lu build=%lu failure=%d\n",
                               scenario.platform, scenario.major, scenario.build, failure);
                        return 2;
                    }
                    ++count;
                }
    printf("GetDXVersion: %u scenarios match outputs and ordered API/COM calls\n", count);
    return 0;
}
