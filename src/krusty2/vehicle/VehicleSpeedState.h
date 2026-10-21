#pragma once


struct VehicleSpeedEntry { float a; float b; };   // 8-byte table entry (slot 1 reads .a)
struct VehicleSpeedState {
    float throttleFraction;                   // +0x0  UnknownEngineGearbox +0x00 (GearRatios.h); UpdateThrottle ramps it in [0, 1]
    int currentRpm;                           // +0x4  UnknownEngineGearbox +0x04 current rpm (an int view of the float)
    int driveCut;                             // +0x8  UnknownEngineGearbox +0x08; DriveForce gives no engine torque while set
    float gearTimer;                // +0x0c countdown timer
    unsigned char gear;        // +0x10 table index (slot 1)
    char pad_0x11[0x1F];
    VehicleSpeedEntry shiftRanges[6];         // +0x30  UnknownEngineGearbox +0x30 shift ranges (.a = lower rpm)
    float shiftWindowWidths[6];               // +0x60  UnknownEngineGearbox +0x60 shift window widths
    float randomStart;                // +0x78 randomised start value (slot 1)
    char pad_0x7C[0x8];
    int lastUpdateRpm;                        // +0x84  UnknownEngineGearbox +0x84 rpm at the last update
    char pad_0x88[0x15c];           // the object is 0x1e4 bytes (LoadVehicle's allocation)
    ~VehicleSpeedState();                           // 0x00464e90, the shared empty out-of-line destructor (~Vehicle)
    // Inline in Vehicle slot 49: counts the timer down to zero (retail loads dt before the
    // timer, the inline-parameter order).
    void TickGearTimer(float dt)
    {
        if (gearTimer != 0.0f) {
            gearTimer -= dt;
            if (gearTimer < 0.0f)
                gearTimer = 0.0f;
        }
    }
};
