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
    field_0x88 = 0.05f;
    field_0x14[0] = 2.0f;
    field_0x14[1] = 1.97f;
    field_0x14[2] = 1.95f;
    field_0x14[3] = 1.9f;
    field_0x14[4] = 1.85f;
    field_0x14[5] = -1.0f;
    field_0x14[6] = 1.0f;
    field_0x90 = 0;
    for (i = 0; i < 6; i++) {
        if (field_0x14[i] < 0.0) {
            if (i < 4) {
                sprintf(message, "Gear Ratios invalid in %s\n", name);
            }
            break;
        }
        field_0x90++;
    }

    field_0x74 = 0;
    field_0x08 = 1;
    if (rpmHigh <= 0) {
        field_0x14c = 10000.0f;
    } else {
        field_0x14c = (float)rpmHigh;
    }
    field_0x04 = 0;
    field_0x00 = 0;
    if (rpmHigh <= 0) {
        field_0x150 = 3000.0f;
        field_0x154 = 10250.0f;
    } else {
        field_0x150 = (float)rpmLow;
        field_0x154 = (float)rpmHigh;
    }

    field_0x94 = 0;
    if (field_0x150 != 0.0f && field_0x154 != 0.0f && field_0x150 <= field_0x154) {
        rpm[0] = 0.0f;
        torque[0] = 0.0f;
        field_0x94 = 1;
        if (rpmHigh <= 0) {
            field_0x94 = 13;
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
            for (int r = (int)field_0x150; r < (int)field_0x154 + 1; r += rpmStep) {
                if (field_0x94 > 12) {
                    break;
                }
                rpm[field_0x94] = (float)r;
                torque[field_0x94] = (float)torqueTable[field_0x94 - 1];
                field_0x94++;
            }
        }

        field_0x158 = -1.0f;
        field_0x15c = 0;
        if (field_0x94 > 1) {
            for (i = 0; i < field_0x94; i++) {
                field_0x98[i] = rpm[i];
                field_0xd4[i] = torque[i];
                if (i == 0) {
                    field_0x160[i] = 0;
                } else {
                    field_0x160[i] = field_0xd4[i] * 5252.1128f / field_0x98[i];
                }
                if (field_0x160[i] > field_0x158) {
                    field_0x158 = field_0x160[i];
                    field_0x15c = i;
                }
                if (i > 0) {
                    field_0x110[i] = (field_0xd4[i] - field_0xd4[i - 1]) / (field_0x98[i] - field_0x98[i - 1]);
                    field_0x19c[i] = (field_0x160[i] - field_0x160[i - 1]) / (field_0x98[i] - field_0x98[i - 1]);
                } else {
                    field_0x110[i] = 0;
                    field_0x19c[i] = 0;
                }
            }
        }
    } else {
        sprintf(message, "Rpm Limits (%d,%d) invalid in %s\n", field_0x150, field_0x154, name);
    }

    for (int gear = 0; gear < field_0x90 - 1; gear++) {
        float step = field_0x14[gear + 1] / field_0x14[gear];
        float bestDifference = 3.4028235e+38f;
        float bestRpm;
        for (int j = field_0x15c + 1; j < field_0x94; j++) {
            field_0x04 = UnknownMin(step * field_0x98[j], field_0x98[field_0x15c]);
            float difference = field_0x160[j] - UnknownPower();
            if (difference < 0) {
                difference = -difference;
            }
            if (difference < bestDifference) {
                bestDifference = difference;
                bestRpm = field_0x98[j];
            }
        }
        if (bestDifference != 3.4028235e+38f) {
            field_0x30[gear].field_0x00 = UnknownMax(bestRpm - 100.0f, 0.0f);
            field_0x30[gear].field_0x04 = UnknownMin(bestRpm + 100.0f, field_0x154);
            field_0x60[gear] = field_0x30[gear].field_0x04 - field_0x30[gear].field_0x00;
        } else {
            sprintf(message, "Unable to find optimum shift range for GEAR=%d\n", gear);
        }
    }

    field_0x7c = 1500.0f;
    field_0x80 = 3000.0f;
    field_0x8c = 1.0f;
    field_0x10 = 0;
    field_0x04 = 0;
    field_0x84 = 0;
    field_0x00 = 0;
    field_0x0c = 0;
    field_0x08 = 1;
    float random = UNKNOWN_RANDOM_UNIT();
    field_0x78 = random * field_0x60[field_0x10] + field_0x30[field_0x10].field_0x00;
}

// 0x004d2f50
void UnknownEngineGearbox::UnknownFunction4d2f50(float dt, int engaged, unsigned char arg2)
{
    if (engaged && field_0x0c == 0.0f) {
        field_0x08 = arg2;
        if (field_0x00 < 1.0f) {
            field_0x00 += dt / field_0x8c * field_0x1dc;
            field_0x00 = UnknownMin(field_0x00, 1.0f);
        }
    } else {
        field_0x08 = 1;
        if (field_0x00 > 0.0f) {
            field_0x00 -= (field_0x0c != 0.0f ? dt * field_0x1e0 * 0.2f : dt * field_0x1e0 * 0.6f);
            field_0x00 = UnknownMax(field_0x00, 0.0f);
        }
    }
    field_0x04 = field_0x154 * field_0x00;
}

// 0x004d3030: shifts up past the randomised shift rpm and down below
// field_0x7c.
void UnknownEngineGearbox::UnknownFunction4d3030(int arg0, float arg1)
{
    if (field_0x0c != 0.0f) {
        return;
    }
    if (field_0x04 >= field_0x84 && field_0x04 > field_0x78 && field_0x10 < field_0x90 - 1) {
        field_0x10++;
        field_0x00 = UnknownMin(field_0x14[field_0x10] * field_0x00 / field_0x14[field_0x10 - 1], 1.0f);
        field_0x04 = field_0x00 * field_0x154;
        if (field_0x04 > field_0x78 && field_0x10 == field_0x90 - 1) {
            field_0x04 = field_0x78;
        }
        field_0x0c = field_0x88;
        float random = UNKNOWN_RANDOM_UNIT();
        field_0x78 = random * field_0x60[field_0x10] + field_0x30[field_0x10].field_0x00;
    } else if (field_0x10 > 0 && (field_0x04 < field_0x84 || field_0x04 == 0.0f) && field_0x04 < field_0x7c) {
        field_0x10--;
        field_0x00 = UnknownMin(field_0x14[field_0x10] * field_0x00 / field_0x14[field_0x10 + 1], 1.0f);
        field_0x04 = field_0x00 * field_0x154;
        field_0x0c = field_0x88;
        float random = UNKNOWN_RANDOM_UNIT();
        field_0x78 = random * field_0x60[field_0x10] + field_0x30[field_0x10].field_0x00;
    } else {
        field_0x0c = 0;
    }
    field_0x84 = field_0x04;
}

// 0x004d31b0
void UnknownEngineGearbox::UnknownFunction4d31b0(float speedRatio, const Vector3* direction, float scale,
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
        *magnitude = UnknownTorque() * field_0x14[field_0x10] * 341.62f;
    } else {
        *magnitude = UnknownTorque() * field_0x14[field_0x10] * 341.62f / speedRatio;
    }
    *force = scale * *magnitude * *direction;
}
