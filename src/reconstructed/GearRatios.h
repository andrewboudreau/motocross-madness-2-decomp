#pragma once

#include "MatrixUtil.h"

// Gearbox and RPM/torque curve of a vehicle engine (code
// 0x004d2940..0x004d333f, followed by its kVec3 `$E` set
// 0x004d3340..0x004d347b). No source string names the unit; GearRatios.cpp
// is a descriptive file name, not an attested TU name. Vehicle.cpp allocates
// 0x1e4 bytes and constructs it (0x005260df). All names are provisional.

// Shift window for one gear (+0x30 + 8 * gear).
struct UnknownGearShiftRange {
    float lowerRpm;                           // +0x00 lower rpm
    float upperRpm;                           // +0x04 upper rpm
};

class UnknownEngineGearbox {
public:
    // 0x004d2940: builds the curve from `rpmTable` between `rpmLow` and
    // `rpmHigh` (or a built-in curve when `rpmHigh` <= 0) and picks the shift
    // windows. `name` is only used by the debug messages.
    UnknownEngineGearbox(const char* name, float unused, int* torqueTable, int rpmLow, int rpmHigh,
                         int rpmStep, int arg6, float riseRate, float fallRate);

    void UpdateThrottle(float dt, int engaged, unsigned char disengaged);   // 0x004d2f50
    void AutoShift(int arg0, float arg1);                        // 0x004d3030
    // 0x004d31b0: drive force along `direction` and its magnitude.
    void DriveForce(float speedRatio, const Vector3* direction, float scale, char reversing,
                               float reverseForce, Vector3* force, float* magnitude);

    // Curve lookups at the current rpm (field_0x04); inline in retail.
    float UnknownTorque() const {
        int i = 0;
        while (i < curvePointCount && curveRpm[i] < currentRpm) {
            i++;
        }
        return (currentRpm - curveRpm[i]) * torqueSlope[i] + curveTorque[i];
    }
    float UnknownPower() const {
        int i = 0;
        while (i < curvePointCount && curveRpm[i] < currentRpm) {
            i++;
        }
        return (currentRpm - curveRpm[i]) * powerSlope[i] + curvePower[i];
    }

    float throttleFraction;                   // +0x00 throttle fraction, clamped to [0, 1]
    float currentRpm;                         // +0x04 current rpm
    int driveCut;                             // +0x8  set off the throttle and on a shift; else UpdateThrottle's third argument (Vehicle slot 49 passes its driveDisengaged flag +0x479); DriveForce gives no engine torque while set
    float shiftTimer;                         // +0x0c shift timer
    unsigned char currentGear;                // +0x10 current gear
    char pad_0x11[3];
    float gearRatios[7];                      // +0x14 gear ratios; a negative entry ends the list
    UnknownGearShiftRange shiftRanges[6];     // +0x30
    float shiftWindowWidths[5];               // +0x60 shift window widths
    int field_0x74;
    float upShiftRpm;                         // +0x78 randomised up-shift rpm
    float downShiftRpm;                       // +0x7c down-shift rpm
    float field_0x80;
    float lastUpdateRpm;                      // +0x84 rpm at the last update
    float shiftTime;                          // +0x88 shift time
    float throttleTimeScale;                  // +0x8c ctor 1.0; UpdateThrottle raises the throttle by dt / it * throttleRiseRate
    int gearCount;                            // +0x90 gear count
    int curvePointCount;                      // +0x94 curve point count
    float curveRpm[15];                       // +0x98 curve rpm
    float curveTorque[15];                    // +0xd4 curve torque
    float torqueSlope[15];                    // +0x110 torque slope
    float field_0x14c;
    float rpmLowLimit;                        // +0x150 rpm low limit
    float rpmHighLimit;                       // +0x154 rpm high limit
    float peakPower;                          // +0x158 peak power
    int peakPowerPoint;                       // +0x15c peak power point
    float curvePower[15];                     // +0x160 power
    float powerSlope[15];                     // +0x19c power slope
    int field_0x1d8;
    float throttleRiseRate;                   // +0x1dc ctor riseRate
    float throttleFallRate;                   // +0x1e0 ctor fallRate; off the throttle it falls by dt * it * 0.6 (0.2 while shifting)
};
