// ConstraintTypes.h -- vector / scene-node / rigid-body views used by
// ConstraintMethodCollisionModel.cpp. PROVISIONAL (tier 3 names, tier 1 offsets).
//
// Why local types: the retail TU builds Vec3 values through an out-of-line
// constructor (thiscall, ret 0xc, 0x00404e60) and calls out-of-line copies of the
// vector operators (0x00421cb0 a+b, 0x005015b0 v*s, 0x00428090 s*v, 0x0043c890 v/s,
// 0x00515600 cross, 0x00435ec0 length, 0x005087b0 normalize, 0x0043ca20 v*M).
// ../common/Math3D.h defines the same operations inline, which VC6 would expand;
// so the operators here are declared, not defined, to force the same calls.
// The physics body / node classes are minimal views of PhysicsBody
// (../rigidbody/PhysicsBody.h) and SoultreeObject (../common/SoultreeObject.h).
#ifndef CONSTRAINT_TYPES_H
#define CONSTRAINT_TYPES_H

struct ConVec3 {
    float x, y, z;
    ConVec3() {}
    ConVec3(float x_, float y_, float z_);          // 0x00404e60 (out of line)
    ConVec3& operator+=(const ConVec3& v);          // 0x00428060
};

ConVec3 operator+(const ConVec3& a, const ConVec3& b);   // 0x00421cb0
ConVec3 operator*(const ConVec3& v, float s);             // 0x005015b0
ConVec3 operator*(float s, const ConVec3& v);             // 0x00428090
ConVec3 operator/(const ConVec3& v, float s);             // 0x0043c890 (v * (1/s))
ConVec3 CrossProduct(const ConVec3& a, const ConVec3& b); // 0x00515600
float Magnitude(const ConVec3& v);                        // 0x00435ec0 (1.0 if |v|^2 == 1 else FastSqrt)
ConVec3 Normalize(const ConVec3& v);                      // 0x005087b0

// 4x4 row-major float matrix (D3DMATRIX layout), only the upper 3x3 is read here.
struct ConMatrix {
    float m[4][4];
};
// 0x0043ca20: row vector times the upper 3x3 of m (x' = v.x*m00 + v.y*m10 + v.z*m20).
ConVec3 RotateVector(const ConVec3& v, const ConMatrix& m);

// Scene-graph node (SoultreeObject). All thiscall, struct results through a hidden pointer.
class ConNode {
public:
    ConVec3 LocalToWorldPoint(const ConVec3& p);                 // 0x004fd660
    ConVec3 WorldToLocalDirection(const ConVec3& v);             // 0x004fd710
    void GetPositionIn(ConNode* frame, ConVec3* out);            // 0x004fc9a0
    void SetPositionIn(ConNode* frame, const ConVec3* p);        // 0x004fc740
};

// PhysicsBody / PhysicsRigidBody view: 44 virtual slots (0x00556e0c), slot 43 is
// GetWorldAngularVelocity (0x004cbed0).
class ConBody {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6(); virtual void Slot7();
    virtual void Slot8(); virtual void Slot9(); virtual void Slot10(); virtual void Slot11();
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    virtual void Slot16(); virtual void Slot17(); virtual void Slot18(); virtual void Slot19();
    virtual void Slot20(); virtual void Slot21(); virtual void Slot22(); virtual void Slot23();
    virtual void Slot24(); virtual void Slot25(); virtual void Slot26(); virtual void Slot27();
    virtual void Slot28(); virtual void Slot29(); virtual void Slot30(); virtual void Slot31();
    virtual void Slot32(); virtual void Slot33(); virtual void Slot34(); virtual void Slot35();
    virtual void Slot36(); virtual void Slot37(); virtual void Slot38(); virtual void Slot39();
    virtual void Slot40(); virtual void Slot41(); virtual void Slot42();
    virtual ConVec3 GetWorldAngularVelocity();                   // slot 43, 0x004cbed0

    char field_0x04[0x150];
    ConVec3 velocity;             // 0x154
    ConVec3 prevVelocity;         // 0x160
    ConVec3 angularVelocity;      // 0x16c (body frame)
    float mass;                   // 0x178
    float invMass;                // 0x17c
    char field_0x180[0x10];
    ConMatrix inertia;            // 0x190
    ConMatrix invInertia;         // 0x1d0
    char field_0x210[0x18];
    ConVec3 centerOfMass;         // 0x228
    ConNode* node;                // 0x234
};

// Ground/terrain query object stored at ConstraintMethodCollisionModel+0xe4.
class ConGroundQuery {
public:
    // 0x00507c10 (thiscall, ret 0x10): snaps *point to the surface height under it
    // (in/out) and writes the surface normal. Semantics tier 3.
    void QueryPoint(ConVec3* point, ConVec3* normalOut, int a, int b);
};

#endif
