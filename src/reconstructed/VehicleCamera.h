#pragma once

#include "FollowCamera.h"

// Object at VehicleCamera+0x394 (the followed vehicle; its class is not
// established). Only the fields the camera reads are declared.
class UnknownVehiclePart {
public:
    unsigned char field_0x000[0x140];
    UnknownVehiclePart* field_0x140;     // 0x0052bb60 prefers it when set

    // 0x004fc9a0: writes a position for `index` into *position.
    void UnknownFunction4fc9a0(int index, Vector3* position);
    // 0x004fc970: writes the part's translation into *translation.
    void UnknownFunction4fc970(Vector3* translation);
    // 0x004fdae0: looks up a named child part (BikeCamera asks for "Head").
    UnknownVehiclePart* UnknownFunction4fdae0(const char* name);
    void UnknownFunction4444e0(); // 0x004444e0 (BikeCamera slot 53)
    void UnknownFunction4fdb50(); // 0x004fdb50 (BikeCamera slot 54)
    // 0x004fc4f0 (src/krusty2's SoultreeObject::GetAxes): writes two axis
    // vectors; VehicleCamera slot 41 uses the first.
    void UnknownFunction4fc4f0(Vector3* first, Vector3* second);
};

// An entry of the vehicle's +0x53c list (VehicleCamera slot 35).
struct UnknownCameraRider {
    unsigned char field_0x000[0x150];
    float field_0x150;                   // slot 35 lifts the camera above -0.5
    unsigned char field_0x154[0x2a8 - 0x154];
    int field_0x2a8;
};

struct UnknownCameraVehicle {
    unsigned char field_0x000[0x48];
    float field_0x48;                    // VehicleCamera slot 51
    float field_0x4c;                    // VehicleCamera slot 35 pitch angle
    unsigned char field_0x050[0x64 - 0x50];
    Vector3 field_0x64;                  // VehicleCamera slot 33
    unsigned char field_0x070[0x88 - 0x70];
    Vector3 field_0x88;                  // VehicleCamera slot 41 heading fallback
    unsigned char field_0x094[0xbc - 0x94];
    float field_0xbc;                    // VehicleCamera slot 41 (at least 0.01)
    unsigned char field_0x0c0[0x3bc - 0xc0];
    UnknownVehiclePart* field_0x3bc;     // VehicleCamera slot 50
    unsigned char field_0x3c0[0x434 - 0x3c0];
    float field_0x434;                   // VehicleCamera slot 36 tests > 0.1
    float field_0x438;                   // KrustyBikeCamera slot 34 divisor
    float field_0x43c;                   // VehicleCamera slot 42 scale
    int field_0x440;
    int field_0x444;                     // VehicleCamera slots 72, 74, 75
    unsigned char field_0x448[0x45c - 0x448];
    float field_0x45c;                   // VehicleCamera slot 39
    int field_0x460;                     // BikeCamera slot 75 tests 6
    int field_0x464;                     // VehicleCamera slot 39
    unsigned char field_0x468[0x488 - 0x468];
    Vector3 field_0x488;                 // 0x0052bc50 in vehicle mode
    Vector3 field_0x494;                 // 0x0052bac0 in vehicle mode
    unsigned char field_0x4a0[0x53c - 0x4a0];
    UnknownCameraRider** field_0x53c;    // riders (VehicleCamera slot 35)
    int field_0x540;
    int field_0x544;                     // rider count
    unsigned char field_0x548[0x56c - 0x548];
    int field_0x56c;                     // nonzero: look for a rider with +0x2a8 set

    // Inlined by VehicleCamera slot 35: the first rider with +0x2a8 set
    // when +0x56c asks for one, else the first rider.
    UnknownCameraRider* UnknownInlineRider() {
        if (field_0x56c) {
            for (int i = 0; i < field_0x544; i++) {
                if (field_0x53c[i]->field_0x2a8)
                    return field_0x53c[i];
            }
        }
        return field_0x53c[0];
    }
};

// Optional targets at VehicleCamera+0x384 / +0x388 (slot 33).
struct UnknownCameraTargetA {
    unsigned char field_0x000[0x1a0];
    UnknownVehiclePart* field_0x1a0;     // 0x0052bb60
    unsigned char field_0x1a4[0x224 - 0x1a4];
    Vector3 field_0x224;
};

struct UnknownCameraTargetB {
    unsigned char field_0x00[0x34];
    UnknownVehiclePart* field_0x34;      // 0x0052bb60
    unsigned char field_0x38[0x40 - 0x38];
    Vector3 field_0x40;
};

// .rdata floats next to VehicleCamera's vtable: 17.0f (slot 67) and the
// 0.18f/0.82f blend weights of slots 35 and 41.
extern const float g_UnknownFloat558d60;
extern const float g_UnknownFloat558d64;
extern const float g_UnknownFloat558d68;

// RTTI: VehicleCamera : FollowCamera. Vehicle.cpp is a name-overlap candidate
// for its translation unit, not established. Implements FollowCamera's pure
// slots 33, 35, 39, 41, 42, 50, 51, 57 and 74; names are provisional.
class VehicleCamera : public FollowCamera {
public:
    explicit VehicleCamera(int flags); // 0x0052b920
    virtual ~VehicleCamera();          // 0x0052b9d0 (deleting wrapper 0x0052b9b0)

    // 0x0052b9e0: FollowCamera's setup (0x00463140); returns this, or 0
    // when it failed.
    VehicleCamera* UnknownFunction52b9e0(void* value, float rate294, float rate298, float value228,
                                         float value2d0, float value2e8, int capacity, int count,
                                         const int* list);

    // 0x0052bac0 / 0x0052bc50: the vehicle's +0x494 / +0x488 in vehicle
    // mode; otherwise slot 33 / 0x0052bb60, latched in +0x398 / +0x3a4 by
    // +0x392 / +0x391.
    Vector3 UnknownFunction52bac0();
    Vector3 UnknownFunction52bc50();
    // 0x0052bb60: the position of the followed part or target.
    Vector3 UnknownFunction52bb60();

    virtual Vector3 UnknownVirtualSlot33();
    virtual Vector3 UnknownVirtualSlot35(unsigned char a, float dt);

    virtual float UnknownVirtualSlot39();
    virtual Vector3 UnknownVirtualSlot41();
    virtual void UnknownVirtualSlot42(bool flag);
    virtual Vector3 UnknownVirtualSlot50();
    virtual float UnknownVirtualSlot51();
    virtual bool UnknownVirtualSlot36(const Vector3& point, bool enable, bool force);
    virtual Vector3 UnknownVirtualSlot57(float dt);
    virtual void UnknownVirtualSlot67();
    virtual void UnknownVirtualSlot68();
    virtual void UnknownVirtualSlot72();
    // Inline: retail's copy (0x00417490) is emitted next to BikeCamera code.
    virtual bool UnknownVirtualSlot74() {
        if (vehicleMode) {
            bool active = vehicle->field_0x444 != 0;
            return active;
        }
        return false;
    }
    virtual bool UnknownVirtualSlot75();

    // Inlined by slot 75: no vehicle mode, or the vehicle's +0x444 is clear.
    int UnknownInlineIdle() {
        if (!vehicleMode)
            return 1;
        return vehicle->field_0x444 == 0;
    }

protected:
    UnknownCameraTargetA* field_0x384;
    UnknownCameraTargetB* field_0x388;
    UnknownVehiclePart* field_0x38c;     // 0x0052bb60
    bool vehicleMode;                    // +0x390, vehicle mode
    unsigned char field_0x391;
    unsigned char field_0x392;
    UnknownCameraVehicle* vehicle;       // +0x394
    Vector3 field_0x398;
    Vector3 field_0x3a4;
};
