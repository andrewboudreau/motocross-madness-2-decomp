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
// src/krusty2/tire/Tire.h and src/reconstructed/Suspension.h.

#include "MatrixUtil.h"

// Scene-graph node (RTTI SoultreeObject, soultree.cpp; src/krusty2/core/SoultreeObject.h).
class SoultreeObject;

// 0x004a2350: the first three entries of `row` of `m`. Called by
// MorphBastardModifier and Bike.
Vector3 GetMatrixRow(const Matrix4* m, int row);

class MovingPart {
public:
    // 0x004a23a0 (ret 0xc): finds `name` below `root` and copies its local
    // matrix.
    MovingPart(const char* name, SoultreeObject* root, int ownerRef_);
    // 0x00464e90: out of line and empty (one shared RET; GameplayDefaults.cpp).
    ~MovingPart();

    Matrix4 matrix;          // +0x00 the node's local matrix at construction
    SoultreeObject* node;    // +0x40
    int ownerRef;            // +0x44 constructor argument (Tire.h, Suspension.h: ownerRef)
    int field_0x48;          // +0x48 0
};
