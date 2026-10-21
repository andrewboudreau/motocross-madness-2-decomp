// Private retail differential fixture. All reporting logic and both SEH
// regions execute; only dependencies and the single FS stack-bound read vary.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static DWORD WINAPI ModuleName(HMODULE, char*, DWORD);
static HANDLE WINAPI OpenLog(const char*, DWORD, DWORD, SECURITY_ATTRIBUTES*, DWORD, DWORD, HANDLE);
static void WINAPI DebugMessage(const char*);
static DWORD WINAPI SeekLog(HANDLE, LONG, LONG*, DWORD);
static DWORD WINAPI QueryMemory(const void*, MEMORY_BASIC_INFORMATION*, DWORD);
static BOOL WINAPI CloseLog(HANDLE);
#include "../../src/reconstructed/ExceptionHandler.h"
#include "../../samples/game/ThreadStackBase.h"
extern "C" void __cdecl _except_handler3();

static BYTE referenceSpace[0x300000];

struct Scenario { int cached, open, path, query, module, access, parameters, write, codeMode, words, badStack; };
struct Event { int kind, a, b, c, d, e, f; char text[1100]; };
struct Observation { int result, count, started; Event events[256]; };
static Scenario scenario;
static Observation* observedStorage;
static Observation* expectedStorage;
#define observed (*observedStorage)
#define expected (*expectedStorage)
static EXCEPTION_RECORD record;
static CONTEXT context;
static EXCEPTION_POINTERS pointers = {&record, &context};
static BYTE* codePages;
static BYTE* stackPages;
static DWORD stackTop;
static int checks, moduleCalls;
static DWORD candidateAddress, candidateGuard;
static Event& EventFor(int kind) {
    if (observed.count >= 256) ExitProcess(80);
    Event& event = observed.events[observed.count++]; event.kind = kind; return event;
}
static void Text(Event& event, const char* value) {
    if (strlen(value) >= sizeof(event.text)) ExitProcess(81);
    strcpy(event.text, value);
}
static DWORD WINAPI ModuleName(HMODULE module, char* path, DWORD size) {
    Event& event = EventFor(1); event.a = (int)module; event.b = size;
    ++moduleCalls;
    const char* name = module ? "C:\\Modules\\fault.module.dll" :
        scenario.path == 0 ? "C:\\Games\\racer.demo.exe" :
        scenario.path == 1 ? "bare.exe" : scenario.path == 2 ? "C:\\Games\\no-extension" : "";
    if ((!module && scenario.path == 3) || (module && !scenario.module)) return 0;
    strcpy(path, name); Text(event, name); return strlen(name);
}
static HANDLE WINAPI OpenLog(const char* path, DWORD access, DWORD share, SECURITY_ATTRIBUTES* security,
                             DWORD disposition, DWORD flags, HANDLE templateFile) {
    Event& e = EventFor(2); Text(e, path); e.a = access; e.b = share; e.c = (int)security;
    e.d = disposition; e.e = flags; e.f = (int)templateFile;
    return scenario.open ? (HANDLE)0x12345678 : INVALID_HANDLE_VALUE;
}
static void WINAPI DebugMessage(const char* text) { Text(EventFor(3), text); }
static DWORD WINAPI SeekLog(HANDLE log, LONG distance, LONG* high, DWORD method) {
    Event& e = EventFor(4); e.a = (int)log; e.b = distance; e.c = (int)high; e.d = method;
    return 0xffffffff; // retail ignores append-seek failure
}
static DWORD WINAPI QueryMemory(const void* address, MEMORY_BASIC_INFORMATION* memory, DWORD size) {
    Event& e = EventFor(5); e.a = (int)address; e.b = size;
    if (!scenario.query) return 0;
    memset(memory, 0x59, sizeof(*memory)); memory->AllocationBase = (void*)0x13570000;
    return sizeof(*memory);
}
static BOOL WINAPI CloseLog(HANDLE log) { EventFor(6).a = (int)log; return 0; }
void hprintf(HANDLE log, char* format, ...) {
    Event& event = EventFor(7); event.a = (int)log;
    va_list args; va_start(args, format);
    wvsprintfA(event.text, format, args); va_end(args);
    if (strlen(event.text) >= sizeof(event.text)) ExitProcess(82);
}
void RecordSystemInformation(HANDLE log) { EventFor(8).a = (int)log; }
void RecordModuleList(HANDLE log) { EventFor(9).a = (int)log; }
const char* GetExceptionDescription(DWORD code) { EventFor(10).a = code; return "scripted exception"; }
char* GetFilePart(char* path) {
    Text(EventFor(11), path);
    char* slash = strrchr(path, '\\'); return slash ? slash + 1 : path;
}
DWORD* ReadCurrentThreadStackBase() { EventFor(12).a = stackTop; return (DWORD*)stackTop; }
static LONG WINAPI StackException(EXCEPTION_POINTERS* info) {
    if (info->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT ||
        info->ExceptionRecord->ExceptionAddress != (void*)0x4602ef) return EXCEPTION_CONTINUE_SEARCH;
    info->ContextRecord->Eax = (DWORD)ReadCurrentThreadStackBase();
    info->ContextRecord->Eip = 0x4602f5;
    return EXCEPTION_CONTINUE_EXECUTION;
}
static DWORD Replacement(DWORD target) {
    switch (target) {
    case 0x460490: return (DWORD)&hprintf;
    case 0x4604e0: return (DWORD)&RecordModuleList;
    case 0x4607a0: return (DWORD)&RecordSystemInformation;
    case 0x4608c0: return (DWORD)&GetExceptionDescription;
    case 0x460ab0: return (DWORD)&GetFilePart;
    case 0x534a60: return (DWORD)&strrchr;
    }
    return 0;
}
static int MapReference(const char* path, const char* patches) {
    FILE* file = fopen(path, "rb"); if (!file) return 0;
    fseek(file, 0, SEEK_END); long size = ftell(file); rewind(file);
    BYTE* image = (BYTE*)malloc(size);
    if (!image || fread(image, 1, size, file) != size) return 0;
    fclose(file);
    IMAGE_NT_HEADERS* pe = (IMAGE_NT_HEADERS*)(image + ((IMAGE_DOS_HEADER*)image)->e_lfanew);
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(pe);
    const DWORD pages[] = {0x45f000, 0x460000, 0x550000, 0x552000, 0x566000,
                           0x568000, 0x56a000, 0x577000, 0x59a000};
    for (int p = 0; p < 9; ++p) {
        DWORD page = pages[p], rva = page - 0x400000, protection;
        if (page < (DWORD)referenceSpace || page + 4096 > (DWORD)referenceSpace + sizeof(referenceSpace)) { printf("storage bounds %x/%x\n", page, referenceSpace); return 0; }
        if (!VirtualProtect((void*)page, 4096, PAGE_EXECUTE_READWRITE, &protection)) return 0;
        int copied = 0;
        for (int j = 0; j < pe->FileHeader.NumberOfSections; ++j) {
            IMAGE_SECTION_HEADER& section = sections[j];
            if (rva >= section.VirtualAddress && rva + 4096 <= section.VirtualAddress + section.SizeOfRawData) {
                memcpy((void*)page, image + section.PointerToRawData + rva - section.VirtualAddress, 4096);
                copied = 1;
            }
        }
        if (!copied && page != 0x59a000) return 0;
    }
    free(image);
    file = fopen(patches, "rb"); if (!file) return 0;
    DWORD patch[2]; int count = 0;
    while (fread(patch, sizeof(patch), 1, file) == 1) {
        DWORD at = patch[0], target = patch[1], replacement = Replacement(target);
        if (!replacement || *(BYTE*)at != 0xe8 || at + 5 + *(DWORD*)(at + 1) != target) return 0;
        *(DWORD*)(at + 1) = replacement - at - 5; ++count;
    }
    fclose(file); if (count != 23) { printf("call count %d\n", count); return 0; }
    if (*(DWORD*)0x45ff8b != 0x536444) return 0;
    *(DWORD*)0x45ff8b = (DWORD)&_except_handler3;
    *(DWORD*)0x5501e4 = (DWORD)&ModuleName; *(DWORD*)0x5501e8 = (DWORD)&lstrcpyA;
    *(DWORD*)0x5501ec = (DWORD)&OpenLog; *(DWORD*)0x5501f4 = (DWORD)&DebugMessage;
    *(DWORD*)0x5501f8 = (DWORD)&SeekLog; *(DWORD*)0x5501fc = (DWORD)&QueryMemory;
    *(DWORD*)0x550200 = (DWORD)&CloseLog; *(DWORD*)0x5502dc = (DWORD)&wsprintfA;
    if (*(WORD*)0x4602ef != 0xa164 || *(DWORD*)0x4602f1 != 4) return 0;
    memset((void*)0x4602ef, 0x90, 6); *(BYTE*)0x4602ef = 0xcc;
    typedef void* (WINAPI *AddHandler)(ULONG, LONG (WINAPI*)(EXCEPTION_POINTERS*));
    AddHandler add = (AddHandler)GetProcAddress(GetModuleHandleA("kernel32.dll"), "AddVectoredExceptionHandler");
    if (!add || !add(1, StackException)) return 0;
    FlushInstructionCache(GetCurrentProcess(), (void*)0x45f000, 8192);
    return 1;
}
static void Run(int reference) {
    memset(&observed, 0, sizeof(observed)); memset(&record, 0, sizeof(record));
    memset(&context, 0x37, sizeof(context));
    record.ExceptionCode = scenario.access ? EXCEPTION_ACCESS_VIOLATION : 0xc0000094;
    record.NumberParameters = scenario.parameters;
    record.ExceptionInformation[0] = scenario.write;
    record.ExceptionInformation[1] = 0xabcdef01;
    context.Eip = (DWORD)(codePages + 4096 - (scenario.codeMode == 0 ? 16 : scenario.codeMode == 1 ? 7 : 0));
    context.Esp = (DWORD)(stackPages + (scenario.badStack ? 8192 - 12 : 0));
    stackTop = context.Esp + scenario.words * 4;
    moduleCalls = 0;
    *(int*)candidateGuard = *(int*)0x59af58 = scenario.cached;
    typedef int (__cdecl *Report)(PEXCEPTION_POINTERS, const char*);
    Report report = (Report)(reference ? 0x45ff80 : candidateAddress);
    observed.result = report(&pointers, "test thread");
    observed.started = reference ? *(int*)0x59af58 : *(int*)candidateGuard;
    // A second invocation must be a no-op even when the first open failed.
    if (report(&pointers, "second invocation") != 0) ExitProcess(83);
}
static void Compare() {
    Run(1); expected = observed; Run(0); ++checks;
    if (memcmp(&observed, &expected, sizeof(observed))) {
        int i = 0; while (i < 256 && !memcmp(&observed.events[i], &expected.events[i], sizeof(Event))) ++i;
        printf("Mismatch%d event%d counts%d/%d path%d query%d module%d access%d params%d write%d code%d words%d bad%d\n",
            checks, i, expected.count, observed.count, scenario.path, scenario.query, scenario.module,
            scenario.access, scenario.parameters, scenario.write, scenario.codeMode, scenario.words, scenario.badStack);
        if (i < 256) printf("retail kind%d [%s]\nsource kind%d [%s]\n", expected.events[i].kind,
                            expected.events[i].text, observed.events[i].kind, observed.events[i].text);
        ExitProcess(2);
    }
}
static int PatchCandidateImports() {
    struct Hook { const char* name; DWORD address; };
    const Hook hooks[] = {
        {"GetModuleFileNameA", (DWORD)&ModuleName}, {"CreateFileA", (DWORD)&OpenLog},
        {"OutputDebugStringA", (DWORD)&DebugMessage}, {"SetFilePointer", (DWORD)&SeekLog},
        {"VirtualQuery", (DWORD)&QueryMemory}, {"CloseHandle", (DWORD)&CloseLog}
    };
    BYTE* base = (BYTE*)GetModuleHandleA(0);
    IMAGE_NT_HEADERS* pe = (IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew);
    IMAGE_IMPORT_DESCRIPTOR* imports = (IMAGE_IMPORT_DESCRIPTOR*)(base + pe->OptionalHeader.DataDirectory[1].VirtualAddress);
    int count = 0;
    for (; imports->Name; ++imports) {
        if (!imports->OriginalFirstThunk) return 0;
        DWORD* names = (DWORD*)(base + imports->OriginalFirstThunk);
        DWORD* slots = (DWORD*)(base + imports->FirstThunk);
        for (; *names; ++names, ++slots) {
            if (*names & 0x80000000) continue;
            const char* name = (const char*)(base + *names + 2);
            for (int h = 0; h < 6; ++h) if (!strcmp(name, hooks[h].name)) {
                DWORD old, restored;
                if (!VirtualProtect(slots, 4, PAGE_READWRITE, &old)) return 0;
                *slots = hooks[h].address;
                if (!VirtualProtect(slots, 4, old, &restored)) return 0;
                ++count;
            }
        }
    }
    return count == 6;
}
int main(int argc, char** argv) {
    if (argc != 5 || !MapReference(argv[1], argv[2])) return 3;
    candidateAddress = strtoul(argv[3], 0, 16);
    candidateGuard = strtoul(argv[4], 0, 16);
    if (!PatchCandidateImports()) return 7;
    observedStorage = (Observation*)malloc(sizeof(Observation));
    expectedStorage = (Observation*)malloc(sizeof(Observation));
    if (!observedStorage || !expectedStorage) return 6;
    codePages = (BYTE*)VirtualAlloc(0, 8192, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    stackPages = (BYTE*)VirtualAlloc(0, 16384, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!codePages || !stackPages) return 4;
    for (int i = 0; i < 8192; ++i) codePages[i] = (BYTE)(i * 19);
    for (i = 0; i < 4096; ++i) ((DWORD*)stackPages)[i] = i * 0x1234567u;
    DWORD protection;
    if (!VirtualProtect(codePages + 4096, 4096, PAGE_NOACCESS, &protection) ||
        !VirtualProtect(stackPages + 8192, 8192, PAGE_NOACCESS, &protection)) return 5;
    const int words[] = {-1, 0, 1, 7, 8, 9, 100, 107, 108, 511, 2048, 2049};
    for (int p = 0; p < 4; ++p) for (int q = 0; q < 2; ++q)
    for (int m = 0; m < 2; ++m) for (int code = 0; code < 3; ++code)
    for (int access = 0; access < 2; ++access) for (int parameters = 0; parameters < 4; ++parameters)
    for (int write = 0; write < 3; ++write) for (int w = 0; w < 12; ++w) {
        memset(&scenario, 0, sizeof(scenario)); scenario.open = 1;
        scenario.path = p; scenario.query = q; scenario.module = m; scenario.codeMode = code;
        scenario.access = access; scenario.parameters = parameters; scenario.write = write;
        scenario.words = words[w]; Compare();
    }
    for (int w = 0; w < 12; ++w) {
        scenario.words = words[w]; scenario.badStack = 1; Compare();
    }
    scenario.open = 0; Compare(); scenario.cached = 1; Compare();
    printf("RecordExceptionInfo: %d scenarios match ordered output, failures and one-shot state\n", checks);
    return 0;
}
