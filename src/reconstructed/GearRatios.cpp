// Engine gearbox and RPM curve (0x004d2940..0x004d347f). The TU name is
// unattested; see GearRatios.h. Names are provisional.

#include <stdio.h>
#include <stdlib.h>

#include "GearRatios.h"

// The four vector constants that close many retail files (see Cube.cpp):
// 0x00689a88, 0x00689a98, 0x00689aa8 and 0x00689a78, initialised by
// 0x004d3340..0x004d347b after this unit's code. 0x004d31b0 reads the first.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

static inline float UnknownMin(float a, float b)
{
    return a < b ? a : b;
}

static inline float UnknownMax(float a, float b)
{
    return a > b ? a : b;
}

// A random value in [0, 1).
#define UNKNOWN_RANDOM_UNIT() (rand() * (1.0f / 32768.0f))

// 0x004d2940
UnknownEngineGearbox::UnknownEngineGearbox(const char* name, float unused, int* torqueTable, int rpmLow,
                                           int rpmHigh, int rpmStep, int arg6, float riseRate, float fallRate)
{
    char message[256];
    float rpm[20];
    float torque[20];
    int i;

    field_0x1d8 = arg6;
    field_0x1dc = riseRate;
    field_0x1e0 = fallRate;
    shiftTime = 0.05f;
    gearRatios[0] = 2.0f;
    gearRatios[1] = 1.97f;
    gearRatios[2] = 1.95f;
    gearRatios[3] = 1.9f;
    gearRatios[4] = 1.85f;
    gearRatios[5] = -1.0f;
    gearRatios[6] = 1.0f;
    gearCount = 0;
    for (i = 0; i < 6; i++) {
        if (gearRatios[i] < 0.0) {
            if (i < 4) {
                sprintf(message, "Gear Ratios invalid in %s\n", name);
            }
            break;
        }
        gearCount++;
    }

    field_0x74 = 0;
    field_0x08 = 1;
    if (rpmHigh <= 0) {
        field_0x14c = 10000.0f;
    } else {
        field_0x14c = (float)rpmHigh;
    }
    currentRpm = 0;
    throttleFraction = 0;
    if (rpmHigh <= 0) {
        rpmLowLimit = 3000.0f;
        rpmHighLimit = 10250.0f;
    } else {
        rpmLowLimit = (float)rpmLow;
        rpmHighLimit = (float)rpmHigh;
    }

    curvePointCount = 0;
    if (rpmLowLimit != 0.0f && rpmHighLimit != 0.0f && rpmLowLimit <= rpmHighLimit) {
        rpm[0] = 0.0f;
        torque[0] = 0.0f;
        curvePointCount = 1;
        if (rpmHigh <= 0) {
            curvePointCount = 13;
            rpm[1] = 3000.0f;
            rpm[2] = 3250.0f;
            rpm[3] = 3750.0f;
            rpm[4] = 4750.0f;
            rpm[5] = 5250.0f;
            rpm[6] = 5750.0f;
            rpm[7] = 6750.0f;
            rpm[8] = 7250.0f;
            rpm[9] = 8250.0f;
            rpm[10] = 8750.0f;
            rpm[11] = 9250.0f;
            rpm[12] = 10250.0f;
            torque[1] = 10.0f;
            torque[2] = 11.5f;
            torque[3] = 15.0f;
            torque[4] = 20.5f;
            torque[5] = 24.0f;
            torque[6] = 26.0f;
            torque[7] = 39.0f;
            torque[8] = 43.0f;
            torque[9] = 40.5f;
            torque[10] = 37.0f;
            torque[11] = 28.5f;
            torque[12] = 25.0f;
        } else {
            for (int r = (int)rpmLowLimit; r < (int)rpmHighLimit + 1; r += rpmStep) {
                if (curvePointCount > 12) {
                    break;
                }
                rpm[curvePointCount] = (float)r;
                torque[curvePointCount] = (float)torqueTable[curvePointCount - 1];
                curvePointCount++;
            }
        }

        peakPower = -1.0f;
        peakPowerPoint = 0;
        if (curvePointCount > 1) {
            for (i = 0; i < curvePointCount; i++) {
                curveRpm[i] = rpm[i];
                curveTorque[i] = torque[i];
                if (i == 0) {
                    curvePower[i] = 0;
                } else {
                    curvePower[i] = curveTorque[i] * 5252.1128f / curveRpm[i];
                }
                if (curvePower[i] > peakPower) {
                    peakPower = curvePower[i];
                    peakPowerPoint = i;
                }
                if (i > 0) {
                    torqueSlope[i] = (curveTorque[i] - curveTorque[i - 1]) / (curveRpm[i] - curveRpm[i - 1]);
                    powerSlope[i] = (curvePower[i] - curvePower[i - 1]) / (curveRpm[i] - curveRpm[i - 1]);
                } else {
                    torqueSlope[i] = 0;
                    powerSlope[i] = 0;
                }
            }
        }
    } else {
        sprintf(message, "Rpm Limits (%d,%d) invalid in %s\n", rpmLowLimit, rpmHighLimit, name);
    }

    for (int gear = 0; gear < gearCount - 1; gear++) {
        float step = gearRatios[gear + 1] / gearRatios[gear];
        float bestDifference = 3.4028235e+38f;
        float bestRpm;
        for (int j = peakPowerPoint + 1; j < curvePointCount; j++) {
            currentRpm = UnknownMin(step * curveRpm[j], curveRpm[peakPowerPoint]);
            float difference = curvePower[j] - UnknownPower();
            if (difference < 0) {
                difference = -difference;
            }
            if (difference < bestDifference) {
                bestDifference = difference;
                bestRpm = curveRpm[j];
            }
        }
        if (bestDifference != 3.4028235e+38f) {
            shiftRanges[gear].lowerRpm = UnknownMax(bestRpm - 100.0f, 0.0f);
            shiftRanges[gear].upperRpm = UnknownMin(bestRpm + 100.0f, rpmHighLimit);
            shiftWindowWidths[gear] = shiftRanges[gear].upperRpm - shiftRanges[gear].lowerRpm;
        } else {
            sprintf(message, "Unable to find optimum shift range for GEAR=%d\n", gear);
        }
    }

    downShiftRpm = 1500.0f;
    field_0x80 = 3000.0f;
    field_0x8c = 1.0f;
    currentGear = 0;
    currentRpm = 0;
    lastUpdateRpm = 0;
    throttleFraction = 0;
    shiftTimer = 0;
    field_0x08 = 1;
    float random = UNKNOWN_RANDOM_UNIT();
    upShiftRpm = random * shiftWindowWidths[currentGear] + shiftRanges[currentGear].lowerRpm;
}

// 0x004d2f50
void UnknownEngineGearbox::UnknownFunction4d2f50(float dt, int engaged, unsigned char arg2)
{
    if (engaged && shiftTimer == 0.0f) {
        field_0x08 = arg2;
        if (throttleFraction < 1.0f) {
            throttleFraction += dt / field_0x8c * field_0x1dc;
            throttleFraction = UnknownMin(throttleFraction, 1.0f);
        }
    } else {
        field_0x08 = 1;
        if (throttleFraction > 0.0f) {
            throttleFraction -= (shiftTimer != 0.0f ? dt * field_0x1e0 * 0.2f : dt * field_0x1e0 * 0.6f);
            throttleFraction = UnknownMax(throttleFraction, 0.0f);
        }
    }
    currentRpm = rpmHighLimit * throttleFraction;
}

// 0x004d3030: shifts up past the randomised shift rpm and down below
// field_0x7c.
void UnknownEngineGearbox::UnknownFunction4d3030(int arg0, float arg1)
{
    if (shiftTimer != 0.0f) {
        return;
    }
    if (currentRpm >= lastUpdateRpm && currentRpm > upShiftRpm && currentGear < gearCount - 1) {
        currentGear++;
        throttleFraction = UnknownMin(gearRatios[currentGear] * throttleFraction / gearRatios[currentGear - 1], 1.0f);
        currentRpm = throttleFraction * rpmHighLimit;
        if (currentRpm > upShiftRpm && currentGear == gearCount - 1) {
            currentRpm = upShiftRpm;
        }
        shiftTimer = shiftTime;
        float random = UNKNOWN_RANDOM_UNIT();
        upShiftRpm = random * shiftWindowWidths[currentGear] + shiftRanges[currentGear].lowerRpm;
    } else if (currentGear > 0 && (currentRpm < lastUpdateRpm || currentRpm == 0.0f) && currentRpm < downShiftRpm) {
        currentGear--;
        throttleFraction = UnknownMin(gearRatios[currentGear] * throttleFraction / gearRatios[currentGear + 1], 1.0f);
        currentRpm = throttleFraction * rpmHighLimit;
        shiftTimer = shiftTime;
        float random = UNKNOWN_RANDOM_UNIT();
        upShiftRpm = random * shiftWindowWidths[currentGear] + shiftRanges[currentGear].lowerRpm;
    } else {
        shiftTimer = 0;
    }
    lastUpdateRpm = currentRpm;
}

// 0x004d31b0
void UnknownEngineGearbox::DriveForce(float speedRatio, const Vector3* direction, float scale,
                                                 bool braking, float brakeValue, Vector3* force,
                                                 float* magnitude)
{
    if (field_0x08) {
        if (braking) {
            *magnitude = brakeValue;
            *force = -brakeValue * *direction;
            return;
        }
        *magnitude = 0;
        *force = kVec3Zero;
        return;
    }
    if (speedRatio < 1.01f) {
        *magnitude = UnknownTorque() * gearRatios[currentGear] * 341.62f;
    } else {
        *magnitude = UnknownTorque() * gearRatios[currentGear] * 341.62f / speedRatio;
    }
    *force = scale * *magnitude * *direction;
}
