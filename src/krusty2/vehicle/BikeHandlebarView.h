#pragma once

class SoultreeObject;
class SteeringControl;
typedef SteeringControl VehicleSteerState;

struct BikeA60C {
    BikeA60C(const char* name, SoultreeObject* root, VehicleSteerState* owner);   // 0x004a23a0 (ret 0xc)
    ~BikeA60C();
    char pad_0x00[0x40];
    SoultreeObject* node;          // +0x40 root->FindByName(name); the shocks and contacts are anchored on it
    VehicleSteerState* owner;      // +0x44 ctor argument
    int field_0x48;
};
