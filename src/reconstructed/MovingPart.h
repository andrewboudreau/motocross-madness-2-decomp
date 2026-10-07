#pragma once

// MovingPart.h -- the two functions between the Math3D vector set (XCU
// 179-182, 0x004a2210..0x004a2340) and MediaControl (0x004a23d0):
// 0x004a2350, a matrix row getter, and 0x004a23a0, the MovingPart
// constructor. No literal, RTTI or __FILE__ names either; the file name,
// class name and member names are ours (tier 3). Which retail unit owns
// them (the Math3D unit before, MediaControl's after, or one of their own)
// is open.
//
// MovingPart is the non-polymorphic base the Tire (RTTI base array mdisp
// 0x17c) and the Shock family (mdisp 4) carry: see
// samples/physics/tire/Tire.h and samples/physics/suspension/Suspension.h.

#include "MatrixUtil.h"

// Scene-graph node (RTTI SoultreeObject, soultree.cpp); only the two calls
// the constructor makes.
class SoultreeObject {
public:
    SoultreeObject* FindByName(const char* name);   // 0x004fdae0
    void GetLocalMatrix(Matrix4* out);               // 0x004fca60
};

// 0x004a2350: the first three entries of `row` of `m`. Called by
// MorphBastardModifier and Bike.
Vector3 GetMatrixRow(const Matrix4* m, int row);

class MovingPart {
public:
    // 0x004a23a0 (ret 0xc): finds `name` below `root` and copies its local
    // matrix. The destructor is the shared empty function 0x00464e90.
    MovingPart(const char* name, SoultreeObject* root, int field_0x44_);

    Matrix4 matrix;          // +0x00 the node's local matrix at construction
    SoultreeObject* node;    // +0x40
    int field_0x44;          // +0x44 constructor argument
    int field_0x48;          // +0x48 0
};
