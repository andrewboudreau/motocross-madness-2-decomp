// Private differential fixture. Only external dependencies and the four RDTSC
// instructions are scripted; the complete retail bodies execute unchanged.
// Include the production candidate to reset its private cached timer between
// independent scenarios. No test switches or test storage enter production.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static BOOL WINAPI ScriptCounter(LARGE_INTEGER* value);
static BOOL WINAPI ScriptFrequency(LARGE_INTEGER* value);
#define QueryPerformanceCounter ScriptCounter
#define QueryPerformanceFrequency ScriptFrequency
#include "../../samples/game/GameTick.cpp"
#undef QueryPerformanceCounter
#undef QueryPerformanceFrequency

GameHiResMeterView* g_HiResMeter;
int g_TickTime, g_PrepFrameTime, g_UpdateScreenTime, g_NetTime;
struct Scenario {
    int count, found, cached, controls, network, suspended, step, mutate, clockMode, scale;
    int calibration, frequencyWorks, counterWorks, calibrationMode;
    unsigned int delta, seed;
};
struct Event {
    int kind, receiver, arg; unsigned __int64 value;
    int phaseTimes[4]; unsigned int currentStamp, previousStamp, frameTimeBits;
};
struct Observation {
    int result, count, profileReads, clockReads, counterReads;
    Event events[64];
    unsigned char game[sizeof(Game)], meters[2 * sizeof(HiResMeter)];
    unsigned char profilingMeters[2 * sizeof(HiResMeter)];
    int times[4], selectedMeter, cachedTimer, registered;
};
static Scenario scenario;
static Observation observed, expected;
static union { double align; unsigned char bytes[sizeof(Game)]; } gameSpace;
static union { double align; unsigned char bytes[2 * sizeof(HiResMeter)]; } meterSpace;
static unsigned char tokens[512];
static DWORD gameVtable[35], rootVtable[27];
static DWORD sourceTick, sourceCalibration;
static int checks;
static int referenceRun;
static unsigned char referenceSpace[0x300000];
static Game* CurrentGame() { return (Game*)gameSpace.bytes; }
static HiResMeter* Meter(int n) { return (HiResMeter*)(meterSpace.bytes + n * sizeof(HiResMeter)); }
static Event& Record(int kind, const void* receiver, int arg) {
    if (observed.count == 64) ExitProcess(80);
    Event& e = observed.events[observed.count++];
    e.kind = kind; e.receiver = (int)receiver; e.arg = arg;
    e.phaseTimes[0] = referenceRun ? *(int*)0x65b534 : g_TickTime;
    e.phaseTimes[1] = referenceRun ? *(int*)0x65b538 : g_PrepFrameTime;
    e.phaseTimes[2] = referenceRun ? *(int*)0x65b53c : g_UpdateScreenTime;
    e.phaseTimes[3] = referenceRun ? *(int*)0x65b540 : g_NetTime;
    e.currentStamp = CurrentGame()->frameClock;
    e.previousStamp = CurrentGame()->previousFrameClock;
    memcpy(&e.frameTimeBits, &CurrentGame()->frameTime, 4);
    return e;
}
static void SelectMeter(int n) {
    g_HiResMeter = (GameHiResMeterView*)Meter(n);
    *(GameHiResMeterView**)0x56c470 = g_HiResMeter;
}
static void Touch(int kind) {
    if (!scenario.mutate) return;
    Game* game = CurrentGame();
    game->frameClock += kind * 13;
    if (kind == 4) {
        game->inputSuspended = !game->inputSuspended;
        game->network = (NetworkInterface*)(tokens + 160);
    }
    if (kind == 6) {
        SelectMeter(1);
        game->rootObject = (GameObject*)(tokens + 64);
        game->renderTarget = (RenderTarget*)(tokens + 256);
    }
    if (kind == 9) game->stepOneFrame = -7;
}
unsigned int ReadClock() {
    int n = observed.clockReads++;
    unsigned int value = n == 0 ? 0xfffffff0u + scenario.delta
                                : 0xfffffffeu + n * (n + 1);
    Record(2, 0, n).value = value;
    return value;
}
void Game::ResetClock() {
    Record(3, this, 0);
    frameClock = ReadClock(); previousFrameClock = frameClock;
}
int ControlInterface::DispatchInput(int bits) { Record(4, this, bits); Touch(4); return -8; }
int NetworkInterface::Update(int bits) { Record(5, this, bits); Touch(5); return 0; }
void HiResMeter::ClearTickTimes() {
    Record(7, this, 0);
    for (int i = 0; i < timerCount; ++i) {
        timers[i].callsThisTick = 0; timers[i].msThisTick = 0;
    }
    Touch(7);
}
void GameObject::ReleaseDetached() { Record(10, this, 0); Touch(10); }
void RenderTarget::AdvanceFrame() { Record(11, this, 0); Touch(11); }
class Probe {
public:
    int RootTick(float seconds) {
        union { float f; int i; } arg; arg.f = seconds;
        Record(6, this, arg.i); Touch(6); return -4;
    }
    int Render() { Record(8, this, 0); Touch(8); return 0; }
    int Present() { Record(9, this, 0); Touch(9); return -1; }
    GameObject* Context(void* value) {
        Record(12, this, (int)value);
        *(void**)((char*)this + 0x18) = value;
        return (GameObject*)(tokens + 400); // return value is deliberately ignored
    }
};
GameObject* GameObject::SetRenderContext(void* value) { return ((Probe*)this)->Context(value); }

__int64 ReadProfilingCounter() {
    int n = observed.profileReads++;
    // The profiling peak is overwritten by gameplay duration later in Tick.
    // Capture it before that overwrite so the first update cannot disappear.
    if (n == 1 && !scenario.calibration)
        memcpy(observed.profilingMeters, meterSpace.bytes, sizeof(meterSpace.bytes));
    unsigned __int64 value = ((unsigned __int64)(scenario.seed * 0x1234567u) << 32)
                          | (0xfffffff0u + scenario.seed);
    value += (unsigned __int64)n * (scenario.calibrationMode == 2 ? 0 : 0x2345678u);
    if (scenario.calibrationMode == 3) value -= (unsigned __int64)n * 0x4567890u;
    Record(1, 0, n).value = value;
    return (__int64)value;
}
static BOOL WINAPI ScriptFrequency(LARGE_INTEGER* value) {
    value->QuadPart = scenario.calibrationMode == 0 ? 10 :
                     scenario.calibrationMode == 1 ? 0 :
                     scenario.calibrationMode == 2 ? -5 : 0x100000007i64;
    Record(13, 0, scenario.frequencyWorks).value = value->QuadPart;
    return scenario.frequencyWorks;
}
static BOOL WINAPI ScriptCounter(LARGE_INTEGER* value) {
    int n = observed.counterReads++;
    if (n > 8) ExitProcess(81);
    // Includes a backward first step, then recovery, low-word carry and overshoot.
    __int64 step = scenario.calibrationMode == 3 ? 0x80000005i64 : 6;
    value->QuadPart = 0xfffffff0i64 + (n == 0 ? 0 : n == 1 ? -3 : (n - 1) * step);
    Record(14, 0, scenario.counterWorks).value = value->QuadPart;
    return scenario.counterWorks;
}
template <class T> DWORD Address(T method) {
    typedef char OneWord[sizeof(T) == sizeof(DWORD) ? 1 : -1];
    union { T method; DWORD address; } p; p.method = method; return p.address;
}
static LONG WINAPI CounterException(EXCEPTION_POINTERS* info) {
    DWORD address = (DWORD)info->ExceptionRecord->ExceptionAddress;
    if (info->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT ||
        (address != 0x4686a0 && address != 0x468712 &&
         address != 0x488477 && address != 0x4884c6)) return EXCEPTION_CONTINUE_SEARCH;
    unsigned __int64 value = ReadProfilingCounter();
    info->ContextRecord->Eax = (DWORD)value;
    info->ContextRecord->Edx = (DWORD)(value >> 32);
    info->ContextRecord->Eip = address + 2;
    return EXCEPTION_CONTINUE_EXECUTION;
}
static DWORD Replacement(DWORD target) {
    switch (target) {
    case 0x468880: return Address(&Game::ResetClock);
    case 0x4bfa80: return (DWORD)&ReadClock;
    case 0x43cf00: return Address(&ControlInterface::DispatchInput);
    case 0x4acde0: return Address(&NetworkInterface::Update);
    case 0x488510: return Address(&HiResMeter::ClearTickTimes);
    case 0x469c80: return Address(&GameObject::ReleaseDetached);
    case 0x4e8cc0: return Address(&RenderTarget::AdvanceFrame);
    case 0x4692f0: return Address(&Probe::Context);
    case 0x536b60: return (DWORD)&strncpy;
    }
    return 0;
}
static int MapReference(const char* path, const char* patches) {
    FILE* file = fopen(path, "rb"); if (!file) return 0;
    fseek(file, 0, SEEK_END); long size = ftell(file); rewind(file);
    BYTE* bytes = (BYTE*)malloc(size);
    if (!bytes || fread(bytes, 1, size, file) != size) return 0;
    fclose(file);
    IMAGE_NT_HEADERS* pe = (IMAGE_NT_HEADERS*)(bytes + ((IMAGE_DOS_HEADER*)bytes)->e_lfanew);
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(pe);
    // Reserve private test storage through the native image, as in the other
    // fixed-address reference fixtures. Bounds are checked before every copy.
    const DWORD pages[] = {0x468000, 0x488000, 0x550000, 0x56b000, 0x56c000, 0x65b000};
    for (int p = 0; p < 6; ++p) {
        DWORD page = pages[p], rva = page - 0x400000, protection;
        if (page < (DWORD)referenceSpace || page + 4096 > (DWORD)referenceSpace + sizeof(referenceSpace)) return 0;
        if (!VirtualProtect((void*)page, 4096, PAGE_EXECUTE_READWRITE, &protection)) return 0;
        int copied = 0;
        for (int j = 0; j < pe->FileHeader.NumberOfSections; ++j) {
            IMAGE_SECTION_HEADER& section = sections[j];
            if (rva >= section.VirtualAddress && rva + 4096 <= section.VirtualAddress + section.SizeOfRawData) {
                memcpy((void*)page, bytes + section.PointerToRawData + rva - section.VirtualAddress, 4096);
                copied = 1;
            }
        }
        // The static timer and phase counters are zero-initialized BSS.
        if (!copied && page != 0x65b000) return 0;
    }
    free(bytes);
    file = fopen(patches, "rb"); if (!file) return 0;
    DWORD patch[2]; int count = 0;
    while (fread(patch, sizeof(patch), 1, file) == 1) {
        DWORD at = patch[0], target = patch[1], replacement = Replacement(target);
        if (!replacement || *(BYTE*)at != 0xe8 || at + 5 + *(DWORD*)(at + 1) != target) return 0;
        *(DWORD*)(at + 1) = replacement - at - 5; ++count;
    }
    fclose(file); if (count != 14) return 0;
    *(DWORD*)0x5501b0 = (DWORD)&ScriptFrequency;
    *(DWORD*)0x5501b4 = (DWORD)&ScriptCounter;
    const DWORD counters[] = {0x4686a0, 0x468712, 0x488477, 0x4884c6};
    for (int i = 0; i < 4; ++i) {
        if (*(WORD*)counters[i] != 0x310f) { printf("counter bytes %x %x\n", counters[i], *(WORD*)counters[i]); return 0; }
        *(WORD*)counters[i] = 0x90cc;
    }
    typedef void* (WINAPI *AddHandler)(ULONG, LONG (WINAPI*)(EXCEPTION_POINTERS*));
    AddHandler add = (AddHandler)GetProcAddress(GetModuleHandleA("kernel32.dll"),
                                               "AddVectoredExceptionHandler");
    if (!add || !add(1, CounterException)) { printf("handler failed\n"); return 0; }
    FlushInstructionCache(GetCurrentProcess(), (void*)0x468000, 4096);
    FlushInstructionCache(GetCurrentProcess(), (void*)0x488000, 4096);
    return 1;
}
static void Run(int reference) {
    referenceRun = reference;
    memset(&observed, 0, sizeof(observed));
    memset(gameSpace.bytes, 0xa5, sizeof(gameSpace.bytes));
    memset(meterSpace.bytes, 0x59, sizeof(meterSpace.bytes));
    memset(tokens, 0x37, sizeof(tokens));
    memset(gameVtable, 0, sizeof(gameVtable)); memset(rootVtable, 0, sizeof(rootVtable));
    gameVtable[8] = Address(&Probe::Render); gameVtable[9] = Address(&Probe::Present);
    rootVtable[10] = Address(&Probe::RootTick);
    *(DWORD*)CurrentGame() = (DWORD)gameVtable;
    *(DWORD*)tokens = *(DWORD*)(tokens + 64) = (DWORD)rootVtable;
    Game* game = CurrentGame();
    game->rootObject = (GameObject*)tokens; game->renderTarget = (RenderTarget*)(tokens + 224);
    game->controlInterface = scenario.controls ? (ControlInterface*)(tokens + 96) : 0;
    game->network = scenario.network ? (NetworkInterface*)(tokens + 128) : 0;
    game->inputSuspended = scenario.suspended; game->stepOneFrame = scenario.step;
    game->previousFrameClock = scenario.clockMode ? 0 : 0xfffffff0u;
    game->frameClock = 73;
    for (int m = 0; m < 2; ++m) {
        HiResMeter* meter = Meter(m); meter->timerCount = scenario.count;
        meter->field_0x38 = -31;
        const DWORD scales[] = {0x3a83126f, 0, 0xba83126f, 0x7fc01234, 0x358637bd};
        *(DWORD*)&meter->millisecondsPerCounterTick = scales[scenario.scale];
        for (int i = 0; i < 50; ++i) {
            UnknownHiResMeterEntry& e = meter->timers[i];
            e.startStampLow = 0xfffffff0u + i; e.startStampHigh = 0x10000000u - i;
            e.calls = i * 1.5f; e.totalMs = i * -0.375f;
            e.callsThisTick = i * -2.5f; e.msThisTick = i * 3.625f;
            e.peakMs = i * 5.75f;
            strcpy(e.name, i == scenario.found ? "Game::Tick" : "different timer");
        }
    }
    SelectMeter(0);
    gameTickTimerRegistered = *(BYTE*)0x65b488 = scenario.cached;
    gameTickTimer = *(int*)0x65b4b0 = scenario.cached ? 7 : -101;
    g_TickTime = *(int*)0x65b534 = -1; g_PrepFrameTime = *(int*)0x65b538 = -2;
    g_UpdateScreenTime = *(int*)0x65b53c = -3; g_NetTime = *(int*)0x65b540 = -4;
    if (scenario.calibration) {
        union { GameObject* (HiResMeter::*method)(void*); DWORD address; } call;
        call.address = reference ? 0x488440 : sourceCalibration;
        observed.result = (int)(Meter(0)->*call.method)(tokens + 384);
    } else {
        union { int (Game::*method)(); DWORD address; } call;
        call.address = reference ? 0x4685d0 : sourceTick;
        observed.result = (game->*call.method)();
    }
    memcpy(observed.game, gameSpace.bytes, sizeof(observed.game));
    memcpy(observed.meters, meterSpace.bytes, sizeof(observed.meters));
    observed.times[0] = reference ? *(int*)0x65b534 : g_TickTime;
    observed.times[1] = reference ? *(int*)0x65b538 : g_PrepFrameTime;
    observed.times[2] = reference ? *(int*)0x65b53c : g_UpdateScreenTime;
    observed.times[3] = reference ? *(int*)0x65b540 : g_NetTime;
    observed.selectedMeter = (int)g_HiResMeter;
    observed.cachedTimer = reference ? *(int*)0x65b4b0 : gameTickTimer;
    observed.registered = reference ? *(BYTE*)0x65b488 : gameTickTimerRegistered;
}
static void Compare() {
    Run(1); expected = observed; Run(0); ++checks;
    if (memcmp(&expected, &observed, sizeof(observed))) {
        DWORD* a = (DWORD*)&expected; DWORD* b = (DWORD*)&observed;
        unsigned int i = 0; while (i < sizeof(observed) / 4 && a[i] == b[i]) ++i;
        printf("Mismatch %d word%u %08x/%08x count%d found%d cached%d mode%d scale%d delta%u seed%u\n",
               checks, i, a[i], b[i], scenario.count, scenario.found, scenario.cached,
               scenario.calibration, scenario.scale, scenario.delta, scenario.seed);
        ExitProcess(2);
    }
}
int main(int argc, char** argv) {
    if (argc != 5 || !MapReference(argv[1], argv[2])) return 3;
    sourceTick = strtoul(argv[3], 0, 16); sourceCalibration = strtoul(argv[4], 0, 16);
    const int counts[] = {0, 1, 7, 49, 50};
    const unsigned int deltas[] = {0, 1, 199, 200, 201, 1000, 0x80000000u, 0xffffffffu};
    for (int c = 0; c < 5; ++c) for (int found = 0; found < 3; ++found)
    for (int mode = 0; mode < 64; ++mode) for (int d = 0; d < 8; ++d)
    for (int scale = 0; scale < 5; ++scale) {
        memset(&scenario, 0, sizeof(scenario));
        scenario.count = counts[c]; scenario.found = found == 0 ? -1 : found == 1 ? 0 : counts[c] - 1;
        scenario.controls = mode & 1; scenario.network = (mode >> 1) & 1;
        scenario.suspended = (mode >> 2) & 1; scenario.step = (mode >> 3) & 1;
        scenario.clockMode = (mode >> 4) & 1; scenario.mutate = (mode >> 5) & 1;
        scenario.scale = scale; scenario.delta = deltas[d];
        scenario.seed = checks * 197u; Compare();
        // The cached path must not search/register, even if timerCount changes.
        if (d == 0 && scale == 0) { scenario.cached = 1; Compare(); }
    }
    for (int success = 0; success < 2; ++success) for (int qpc = 0; qpc < 2; ++qpc)
    for (int mode = 0; mode < 4; ++mode) {
        memset(&scenario, 0, sizeof(scenario)); scenario.calibration = 1;
        scenario.frequencyWorks = success; scenario.counterWorks = qpc;
        scenario.calibrationMode = mode; scenario.seed = checks * 107u; Compare();
    }
    printf("GameTick and calibration: %d scenarios match full state and ordered calls\n", checks);
    return 0;
}
