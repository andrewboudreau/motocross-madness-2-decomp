// Complete functional candidates for 0x004685d0/683 and 0x00488440/196.
// The profiling counter is QPC instead of retail's RDTSC. Simulation still uses
// ReadClock, including unsigned wraparound and the retail 0.2-second clamp.
// Audited for the linked game 2026-11-11: apart from the counter, every call,
// field, constant and branch agrees with retail. The timer registration now
// follows retail's static-initialiser shape (guard bit test, one value store),
// 93 -> 98 of 208 aligned instructions; the rest of the gap is the RDTSC
// inline assembly (an ebp frame, stamps through [ebp-0xc]/[ebp-8]) against a
// ReadProfilingCounter call and 64-bit composition here.
#include <windows.h>
#include <string.h>
#include "../../src/reconstructed/Game.h"
#include "../../src/reconstructed/ControlInterface.h"
#include "../../src/reconstructed/HiResMeter.h"
#include "ProfilingCounter.h"

extern int g_TickTime, g_PrepFrameTime, g_UpdateScreenTime, g_NetTime;

// Retail registers the profiling timer through what is a function-local static
// with a dynamic initialiser in VC6's expansion: guard byte 0x0065b488 (bit 0
// tested, then or-ed in) and value 0x0065b4b0. The value is the first timer
// with the name, else a new entry, else 49 once all 50 are taken; a found or
// new timer becomes the meter's selected one (+0x38). The guard and value are
// written out as file statics so tests/fixtures/game_tick_compare.cpp can
// reset them between scenarios.
static unsigned char gameTickTimerRegistered;
static int gameTickTimer;

static inline int RegisterHiResTimer(HiResMeter* meter, const char* name) {
    int i;
    for (i = 0; i < meter->timerCount; ++i)
        if (!strcmp(meter->timers[i].name, name)) {
            meter->field_0x38 = i;
            return i;
        }
    if (meter->timerCount >= 50)
        return 49;
    strncat(meter->timers[meter->timerCount].name, name, 31);
    meter->field_0x38 = meter->timerCount;
    return meter->timerCount++;
}

int Game::Tick() {
    HiResMeter* meter;
    if (!(gameTickTimerRegistered & 1)) {
        gameTickTimerRegistered |= 1;
        gameTickTimer = RegisterHiResTimer((HiResMeter*)g_HiResMeter, "Game::Tick");
    }

    meter = (HiResMeter*)g_HiResMeter;
    HiResMeterTimer* entry = &meter->timers[gameTickTimer];
    unsigned __int64 stamp = ReadProfilingCounter();
    unsigned __int64 previous = ((unsigned __int64)entry->startStampHigh << 32)
                             | entry->startStampLow;
    // Subtract as raw 64-bit words, then interpret the result as signed for
    // retail's FILD. Keep the intermediate above float precision until stores.
    double elapsed = (__int64)(stamp - previous) * (double)meter->millisecondsPerCounterTick;
    entry->calls += 1.0f;
    entry->totalMs = (float)(entry->totalMs + elapsed);
    entry->callsThisTick += 1.0f;
    entry->msThisTick = (float)(entry->msThisTick + elapsed);
    if (elapsed > entry->peakMs)
        entry->peakMs = (float)elapsed;

    meter = (HiResMeter*)g_HiResMeter;
    entry = &meter->timers[gameTickTimer];
    stamp = ReadProfilingCounter();
    entry->startStampLow = (unsigned long)stamp;
    entry->startStampHigh = (unsigned long)(stamp >> 32);

    if (!previousFrameClock)
        ResetClock();
    else
        frameClock = ReadClock();
    double seconds = (double)(frameClock - previousFrameClock) * 0.001f;
    float delta = (float)seconds;
    // Retail overwrites the profiling peak with the unclamped frame duration.
    ((HiResMeter*)g_HiResMeter)->timers[gameTickTimer].peakMs = (float)(seconds * 1000.0f);
    frameTime = delta;
    if (delta > 0.2f)
        delta = 0.2f;

    // These existing provisional declarations carry opaque DWORD arguments.
    // The caller passes float bits, not a numeric float-to-int conversion.
    union { float seconds; int bits; } argument;
    argument.seconds = delta;
    if (controlInterface)
        controlInterface->DispatchInput(argument.bits);
    if (!inputSuspended) {
        unsigned int start = ReadClock();
        if (network)
            network->Update(argument.bits);
        unsigned int now = ReadClock();
        g_NetTime = now - start;
        start = now;
        rootObject->Tick(delta);
        now = ReadClock();
        g_TickTime = now - start;
        start = now;
        ((HiResMeter*)g_HiResMeter)->ClearTickTimes();
        RenderFrame();
        now = ReadClock();
        g_PrepFrameTime = now - start;
        start = now;
        PresentFrame();
        now = ReadClock();
        g_UpdateScreenTime = now - start;
        rootObject->ReleaseDetached();
        renderTarget->AdvanceFrame();
    }
    previousFrameClock = frameClock;
    if (stepOneFrame) {
        paused = 1;
        stepOneFrame = 0;
    }
    return 1;
}

GameObject* HiResMeter::SetRenderContext(void* context) {
    GameObject::SetRenderContext(context);
    LARGE_INTEGER frequency, previous, current;
    if (!QueryPerformanceFrequency(&frequency))
        return 0;
    QueryPerformanceCounter(&previous);
    unsigned __int64 start = ReadProfilingCounter();
    unsigned __int64 accumulated = 0;
    while ((__int64)accumulated < frequency.QuadPart) {
        QueryPerformanceCounter(&current);
        accumulated += (unsigned __int64)current.QuadPart - previous.QuadPart;
        previous = current;
    }
    unsigned __int64 end = ReadProfilingCounter();
    millisecondsPerCounterTick = (float)(1000.0f / (double)(__int64)(end - start));
    return this;
}
