#pragma once

#include "FollowCamera.h"

// Object at VehicleCamera+0x394 (the followed vehicle; its class is not
// established). Only the fields the camera reads are declared.
class UnknownVehiclePart {
public:
    // 0x004fc9a0: writes a position for `index` into *position.
    void UnknownFunction4fc9a0(int index, Vector3* position);
    // 0x004fc970: writes the part's translation into *translation.
    void UnknownFunction4fc970(Vector3* translation);
    // 0x004fdae0: looks up a named child part (BikeCamera asks for "Head").
    UnknownVehiclePart* UnknownFunction4fdae0(const char* name);
    void UnknownFunction4444e0(); // 0x004444e0 (BikeCamera slot 53)
    void UnknownFunction4fdb50(); // 0x004fdb50 (BikeCamera slot 54)
};

struct UnknownCameraVehicle {
    unsigned char field_0x000[0x48];
    float field_0x48;                    // VehicleCamera slot 51
    unsigned char field_0x04c[0x64 - 0x4c];
    Vector3 field_0x64;                  // VehicleCamera slot 33
    unsigned char field_0x070[0x3bc - 0x70];
    UnknownVehiclePart* field_0x3bc;     // VehicleCamera slot 50
    unsigned char field_0x3c0[0x43c - 0x3c0];
    float field_0x43c;                   // VehicleCamera slot 42 scale
    int field_0x440;
    int field_0x444;                     // VehicleCamera slots 72, 74, 75
    unsigned char field_0x448[0x45c - 0x448];
    float field_0x45c;                   // VehicleCamera slot 39
    int field_0x460;                     // BikeCamera slot 75 tests 6
    int field_0x464;                     // VehicleCamera slot 39
};

// Optional targets at VehicleCamera+0x384 / +0x388 (slot 33).
struct UnknownCameraTargetA {
    unsigned char field_0x000[0x224];
    Vector3 field_0x224;
};

struct UnknownCameraTargetB {
    unsigned char field_0x00[0x40];
    Vector3 field_0x40;
};

// .rdata float next to VehicleCamera's vtable, loaded by slot 67 (17.0f).
extern const float g_UnknownFloat558d60;
// .bss vector copied into +0x398 and +0x3a4 by the constructor.
extern Vector3 g_UnknownVector68a728;

// RTTI: VehicleCamera : FollowCamera. Vehicle.cpp is a name-overlap candidate
// for its translation unit, not established. Implements FollowCamera's pure
// slots 33, 35, 39, 41, 42, 50, 51, 57 and 74; names are provisional.
class VehicleCamera : public FollowCamera {
public:
    explicit VehicleCamera(int flags); // 0x0052b920
    virtual ~VehicleCamera();          // 0x0052b9d0 (deleting wrapper 0x0052b9b0)

    virtual Vector3 UnknownVirtualSlot33();

    virtual float UnknownVirtualSlot39();
    virtual void UnknownVirtualSlot42(bool flag);
    virtual Vector3 UnknownVirtualSlot50();
    virtual float UnknownVirtualSlot51();
    virtual void UnknownVirtualSlot67();
    virtual void UnknownVirtualSlot72();
    // Inline: retail's copy (0x00417490) is emitted next to BikeCamera code.
    virtual bool UnknownVirtualSlot74() {
        if (vehicleMode) {
            bool active = vehicle->field_0x444 != 0;
            return active;
        }
        return false;
    }
    virtual int UnknownVirtualSlot75();

    // Inlined by slot 75: no vehicle mode, or the vehicle's +0x444 is clear.
    int UnknownInlineIdle() {
        if (!vehicleMode)
            return 1;
        return vehicle->field_0x444 == 0;
    }

protected:
    UnknownCameraTargetA* field_0x384;
    UnknownCameraTargetB* field_0x388;
    int field_0x38c;
    bool vehicleMode;                    // +0x390, vehicle mode
    unsigned char field_0x391;
    unsigned char field_0x392;
    UnknownCameraVehicle* vehicle;       // +0x394
    Vector3 field_0x398;
    Vector3 field_0x3a4;
};
