#pragma once

// Provisional suspension field views shared by Tire's sample and the existing
// VehicleWheel call view. Offsets are decoded; this does not promote the full
// suspension hierarchy or establish original header ownership.
#include "../krusty2/collision/CollisionTypes.h"
class TireNode;

class TireAttachA {
public:
    ~TireAttachA();                      // 0x004f9f80
    void SolveContact(float dt, const CollisionVec3* offset, const CollisionVec3* base, const CollisionVec3* point,
                      const CollisionVec3* normal, float limit, float* outLoad,
                      int* outActive);   // 0x004fa400 (ret 0x20)
    char field_0x00[0x44];
    TireNode* node;                      // +0x44
    char field_0x48[0x54 - 0x48];
    int extending;                       // +0x54
    char field_0x58[0x70 - 0x58];
    float ratio;                         // +0x70
    char field_0x74[0x80 - 0x74];
    CollisionVec3 field_0x80;            // +0x80
    char field_0x8c[0x98 - 0x8c];
    float position;                      // +0x98
    char field_0x9c[0xc0 - 0x9c];
    CollisionVec3 direction;             // +0xc0
};
class TireAttachB {
public:
    ~TireAttachB();                      // 0x004fa8a0
    void SolveContact(float dt, const CollisionVec3* axisA, const CollisionVec3* axisB,
                      const CollisionVec3* point, const CollisionVec3* normal, const CollisionVec3* offsetA,
                      const CollisionVec3* offsetB, float limit, float* outLoad,
                      int* outActive);   // 0x004fac60 (ret 0x28)
    int field_0x00;
    CollisionMatrix4 restMatrix;         // +0x04
    TireNode* node;                      // +0x44
    char field_0x48[0x54 - 0x48];
    int extending;                       // +0x54
    char field_0x58[0x70 - 0x58];
    float ratio;                         // +0x70
    char field_0x74[0x80 - 0x74];
    CollisionVec3 field_0x80;            // +0x80
    char field_0x8c[0x98 - 0x8c];
    float position;                      // +0x98
    char field_0x9c[0xc8 - 0x9c];
    float scale;                         // +0xc8
    char field_0xcc[0xd4 - 0xcc];
    CollisionVec3 axis;                  // +0xd4
};
