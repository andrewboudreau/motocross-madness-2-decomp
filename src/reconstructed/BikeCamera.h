#pragma once

#include "VehicleCamera.h"

struct UnknownCameraBikeRider {
    unsigned char field_0x000[0x1a0];
    UnknownVehiclePart* field_0x1a0;     // the rider model; has a "Head" part
};

struct UnknownCameraBikeState {
    unsigned char field_0x00[0x44];
    int field_0x44;                      // BikeCamera slot 75 tests 3
};

// Object at BikeCamera+0x3b0. It shares UnknownCameraVehicle's offsets
// (+0x48, +0x3bc, +0x444, +0x460) and adds the fields below; its class is not
// established.
struct UnknownCameraBike {
    unsigned char field_0x000[0x48];
    float field_0x48;
    unsigned char field_0x04c[0x58 - 0x4c];
    float field_0x58;                    // slot 51 multiplies it with +0x48
    unsigned char field_0x05c[0x3bc - 0x5c];
    UnknownVehiclePart* field_0x3bc;
    unsigned char field_0x3c0[0x444 - 0x3c0];
    int field_0x444;
    unsigned char field_0x448[0x460 - 0x448];
    int field_0x460;
    unsigned char field_0x464[0x5c4 - 0x464];
    UnknownCameraBikeRider* field_0x5c4;
    unsigned char field_0x5c8[0x604 - 0x5c8];
    UnknownCameraBikeState* field_0x604;
};

// RTTI: BikeCamera : VehicleCamera. Its code sits near BikeAI.cpp references;
// the translation unit is not established.
class BikeCamera : public VehicleCamera {
public:
    explicit BikeCamera(int flags); // 0x00416e20
    virtual ~BikeCamera();          // 0x00416e70 (deleting wrapper 0x00416e50)

    virtual Vector3 UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot40(int a);
    virtual Vector3 UnknownVirtualSlot50();
    virtual float UnknownVirtualSlot51();
    virtual void UnknownVirtualSlot53();
    virtual void UnknownVirtualSlot54();
    virtual int UnknownVirtualSlot75();

protected:
    UnknownCameraBike* bike; // +0x3b0
};
