// Complete functional candidates for 0x004685d0/683 and 0x00488440/196.
// The profiling counter is QPC instead of retail's RDTSC. Simulation still uses
// ReadClock, including unsigned wraparound and the retail 0.2-second clamp.
#include <windows.h>
#include <string.h>
#include "../../src/reconstructed/Game.h"
#include "../../src/reconstructed/ControlInterface.h"
#include "../../src/reconstructed/HiResMeter.h"
#include "ProfilingCounter.h"

extern int g_TickTime, g_PrepFrameTime, g_UpdateScreenTime, g_NetTime;

static int gameTickTimerRegistered;
static int gameTickTimer;

int Game::Tick() {
    HiResMeter* meter;
    if (!gameTickTimerRegistered) {
        gameTickTimerRegistered = 1;
        meter = (HiResMeter*)g_HiResMeter;
        int i;
        for (i = 0; i < meter->timerCount; ++i)
            if (!strcmp(meter->timers[i].name, "Game::Tick"))
                break;
        if (i < meter->timerCount) {
            meter->field_0x38 = i;
            gameTickTimer = i;
        } else if (meter->timerCount >= 50) {
            gameTickTimer = 49;
        } else {
            strncpy(meter->timers[meter->timerCount].name, "Game::Tick", 31);
            meter->field_0x38 = meter->timerCount;
            gameTickTimer = meter->timerCount++;
        }
    }

    meter = (HiResMeter*)g_HiResMeter;
    UnknownHiResMeterEntry* entry = &meter->timers[gameTickTimer];
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

GameObject* HiResMeter::SetRenderContext(void* value) {
    GameObject::SetRenderContext(value);
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
