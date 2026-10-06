#pragma once

#include "MatrixUtil.h"

// Gearbox and RPM/torque curve of a vehicle engine (code
// 0x004d2940..0x004d333f, followed by its kVec3 `$E` set
// 0x004d3340..0x004d347b). No source string names the unit; GearRatios.cpp
// is a descriptive file name, not an attested TU name. Vehicle.cpp allocates
// 0x1e4 bytes and constructs it (0x005260df). All names are provisional.

// Shift window for one gear (+0x30 + 8 * gear).
struct UnknownGearShiftRange {
    float field_0x00;                            // lower rpm
    float field_0x04;                            // upper rpm
};

class UnknownEngineGearbox {
public:
    // 0x004d2940: builds the curve from `rpmTable` between `rpmLow` and
    // `rpmHigh` (or a built-in curve when `rpmHigh` <= 0) and picks the shift
    // windows. `name` is only used by the debug messages.
    UnknownEngineGearbox(const char* name, float unused, int* torqueTable, int rpmLow, int rpmHigh,
                         int rpmStep, int arg6, float riseRate, float fallRate);

    void UnknownFunction4d2f50(float dt, int engaged, unsigned char arg2);   // 0x004d2f50
    void UnknownFunction4d3030(int arg0, float arg1);                        // 0x004d3030
    // 0x004d31b0: drive force along `direction` and its magnitude.
    void UnknownFunction4d31b0(float speedRatio, const Vector3* direction, float scale, bool braking,
                               float brakeValue, Vector3* force, float* magnitude);

    // Curve lookups at the current rpm (field_0x04); inline in retail.
    float UnknownTorque() const {
        int i = 0;
        while (i < field_0x94 && field_0x98[i] < field_0x04) {
            i++;
        }
        return (field_0x04 - field_0x98[i]) * field_0x110[i] + field_0xd4[i];
    }
    float UnknownPower() const {
        int i = 0;
        while (i < field_0x94 && field_0x98[i] < field_0x04) {
            i++;
        }
        return (field_0x04 - field_0x98[i]) * field_0x19c[i] + field_0x160[i];
    }

    float field_0x00;                            // throttle fraction, clamped to [0, 1]
    float field_0x04;                            // current rpm
    int field_0x08;
    float field_0x0c;                            // shift timer
    unsigned char field_0x10;                    // current gear
    char pad_0x11[3];
    float field_0x14[7];                         // gear ratios; a negative entry ends the list
    UnknownGearShiftRange field_0x30[6];
    float field_0x60[5];                         // shift window widths
    int field_0x74;
    float field_0x78;                            // randomised up-shift rpm
    float field_0x7c;                            // down-shift rpm
    float field_0x80;
    float field_0x84;                            // rpm at the last update
    float field_0x88;                            // shift time
    float field_0x8c;
    int field_0x90;                              // gear count
    int field_0x94;                              // curve point count
    float field_0x98[15];                        // curve rpm
    float field_0xd4[15];                        // curve torque
    float field_0x110[15];                       // torque slope
    float field_0x14c;
    float field_0x150;                           // rpm low limit
    float field_0x154;                           // rpm high limit
    float field_0x158;                           // peak power
    int field_0x15c;                             // peak power point
    float field_0x160[15];                       // power
    float field_0x19c[15];                       // power slope
    int field_0x1d8;
    float field_0x1dc;
    float field_0x1e0;
};
