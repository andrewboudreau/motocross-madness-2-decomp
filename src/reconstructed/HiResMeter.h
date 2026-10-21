#pragma once

#include "GameObject.h"

// RTTI: HiResMeter : GameObject (vtable 0x00554170). A global instance at
// 0x0067b468 (initializer 0x00488340) prints timer rows on a DebugOverlay
// page.
struct UnknownHiResMeterEntry {
    unsigned long startStampLow;              // +0x00: raw counter low word (0x00468712)
    unsigned long startStampHigh;             // +0x04: high word, subtracted with borrow
    // Named after slot 10's "MS Timers [CallsThisTick-->TotMSThisTick
    // (AvgMSPerTick)] : (Per-call Peak/Avg)" columns.
    float calls;                              // +0x08
    float totalMs;                            // +0x0c: total time (per-call average = totalMs / calls)
    float callsThisTick;                      // +0x10
    float msThisTick;                         // +0x14
    float msAllTicks;                         // +0x18: msThisTick summed each tick
    float ticks;                              // +0x1c
    float peakMs;                             // +0x20: per-call peak
    char name[0x24];                          // +0x24
};

class HiResMeter : public GameObject {
public:
    explicit HiResMeter(int flags);           // 0x00488380
    virtual ~HiResMeter();                    // 0x00488430 (deleting wrapper 0x00488410)
    // 0x00488440: retail calibrates RDTSC. The functional candidate uses
    // the same calibration algorithm with a QPC profiling counter.
    virtual GameObject* SetRenderContext(void* value);
    virtual int Tick(float frameTime);                 // 0x004885c0

    void ClearTickTimes();                    // 0x00488510: per-tick counts (each game tick, 0x00468807)
    void ResetTimerStats();                   // 0x00488540: all counts of the used timers (page not shown)
    void ClearTimers();                       // 0x00488580: all 50 entries and their names (constructor)

    unsigned char counterUsable;              // +0x2c: slot 8 calibrated the time stamp counter
    float millisecondsPerCounterTick;        // +0x30: 1000 / signed counter delta
    int timerCount;                           // +0x34: entries in use
    int field_0x38;                           // +0x38: last selected timer; cleared by resets
    int overlayPage;                          // +0x3c: debug overlay page, -1 until assigned
    UnknownHiResMeterEntry timers[50];        // +0x40
};
