// Shock, InlineShock and RotatingShock views retained from the suspension sample.
// Evidence: docs/INLINE_SHOCK_AXIS.md and samples/physics/suspension/README.md.
// Semantic fields and original TU/header ownership remain provisional.
#ifndef SUSPENSION_H
#define SUSPENSION_H

#include "core/GameObject.h"

// Local stand-in for the shared Vec3; including Math3D.h would add private
// vector initializers. The original Shock sample supplies four vectors. Same shape as Math3D's
// Vec3: an empty default constructor and an inline (x, y, z) constructor.
struct ShockVec3 {
    float x, y, z;
    ShockVec3() {}
    ShockVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    float LengthSquared() const { return z * z + (x * x + y * y); }
    ShockVec3 operator-(const ShockVec3& o) const { return ShockVec3(x - o.x, y - o.y, z - o.z); }
    ShockVec3 operator+(const ShockVec3& o) const { return ShockVec3(x + o.x, y + o.y, z + o.z); }
    ShockVec3 operator*(float s) const { return ShockVec3(x * s, y * s, z * s); }
    ShockVec3& operator+=(const ShockVec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
};

// VC6 reassociates float sums; the parenthesised form reproduces retail's (x*x + y*y) + z*z
// (same finding as Math3D.h Vec3Normalize).
inline float ShockDot(const ShockVec3& a, const ShockVec3& b) { return a.z * b.z + (a.x * b.x + a.y * b.y); }

// Out-of-line vector helpers (cdecl).  Same entries as Vec3DotCall / Vec3ScaleCall in
// src/krusty2/math/Math3D.h.
float ShockDotCall(const ShockVec3& a, const ShockVec3& b);                  // 0x0040ae30
ShockVec3 ShockScaleCall(const ShockVec3& v, float s);                       // 0x005015b0 (hidden return pointer)
ShockVec3 ShockAddCall(const ShockVec3& a, const ShockVec3& b);              // 0x00421cb0 (hidden return pointer)
float ShockLength(const ShockVec3& v);                                       // 0x00435ec0 (1.0 when |v|^2 == 1, else FastSqrt)

// The parenthesised first products are significant: they put each fsubp between the load and
// the store of the previous component's copy, as retail does in RotatingShock::SolveContact
// (docs/VC6_OPERAND_ORDER.md section 3).
inline ShockVec3 ShockCross(const ShockVec3& a, const ShockVec3& b)
{
    return ShockVec3((a.y * b.z) - a.z * b.y, (a.z * b.x) - a.x * b.z, (a.x * b.y) - a.y * b.x);
}

// The four TU-private constant vectors built by the dynamic initializers 0x004fafd0..0x004fb0c0
// (zero, +X, +Y, +Z; globals 0x00689e58, 0x00689e68, 0x00689e78, 0x00689e48).
extern const ShockVec3 kShockZero;
extern const ShockVec3 kShockAxisX;
extern const ShockVec3 kShockAxisY;
extern const ShockVec3 kShockAxisZ;

// Scene-graph node (SoultreeObject, soultree.cpp).  Only the calls the shocks make.
class ShockNode {
public:
    void SetPosition(float x, float y, float z);            // 0x004fc630 (ret 0xc)
    void SetPosition(ShockVec3 p);                           // same entry, Vec3 passed by value
    void SetPositionVec(const ShockVec3& p);                 // 0x004fc660 (ret 4)
    void GetPositionIn(ShockNode* frame, ShockVec3* out);    // 0x004fc9a0 (ret 8)
    void SetMatrixLike(const void* m);                       // 0x004fca30 (ret 4)
    void Rotate(ShockVec3 axis, float angle);                // 0x004fceb0 (ret 0x10)
};

// MovingPart: the non-polymorphic 0x4c byte base placed at +4 of every Shock (RTTI base array
// mdisp 4).  Ctor 0x004a23a0 (ret 0xc), dtor 0x00464e90 (out of line, empty).
// The same provisional shape is declared in samples/physics/tire/Tire.h.
class MovingPart {
public:
    MovingPart(void* name, int root, int ownerRef);
    ~MovingPart();

    char field_0x00[0x40];
    ShockNode* sceneNode;   // +0x40 (+0x44 in a Shock): the node the shocks move
    int ownerRef;           // +0x44 ctor arg ownerRef
    int field_0x48;
};

class Shock : public MovingPart {
public:
    Shock(void* name, int root, float stiffnessMin, float stiffnessMax, float dampingRatio, float speedThreshold,
          float retractScale);                                                   // 0x004f9a60 (ret 0x1c)
    ~Shock();                                                                    // 0x004f9b80
    virtual void Reset();            // slot 0, 0x004f9ba0
    virtual void ClearForces();      // slot 1, 0x004f9c70 (shared by InlineShock/RotatingShock)
    void ApplyDamping(float loadWeight, float massFactor, float dt, float speedOut); // 0x004f9d30 (ret 0x10)

    int atLimit;                     // +0x50
    int extending;                   // +0x54
    ShockVec3 spring;                // +0x58
    ShockVec3 damper;                // +0x64
    float ratio;                     // +0x70
    ShockVec3 displacement;          // +0x74
    ShockVec3 stepDisplacement;      // +0x80 ClampAndStep: displacement * (length - prevLength) / length, zero at length 0;
                                     //       SolveContact's load subtracts its normal component, Tire::UpdateShock adds it to the contact
    float prevLength;                // +0x8c
    float clearedLength;             // +0x90 ClearForces copies length here (ctor/Reset 0); RotatingShock::SolveContact measures from it
    float impulseScale;              // +0x94 zeroed here; Vehicle.h's VehicleShock view: Bike slot 65 sets it, the wheel update consumes it
    float length;                    // +0x98
    float stiffnessMin;              // +0x9c
    float stiffnessMax;              // +0xa0
    float dampingRatio;              // +0xa4
    float speedThreshold;            // +0xa8
    float retractScale;              // +0xac Retract divides the retraction by carrier->field_0x2a4 * it
    ShockVec3 velocity;              // +0xb0
    float speed;                     // +0xbc
};


// Object whose float at +0x2a4 scales InlineShock::Retract (caller 0x00528f1c, wrecker.cpp).
struct ShockCarrier {
    char field_0x00[0x2a4];
    float field_0x2a4;
};

class InlineShock : public Shock {
public:
    InlineShock(void* name, int root, ShockNode* anchorNode, float minLength, float stiffnessMin,
                float stiffnessMax, float dampingRatio, float speedThreshold,
                float retractScale);                                  // 0x004f9ee0 (ret 0x24)
    ~InlineShock();                                                   // 0x004f9f80
    virtual void Reset();                                             // slot 0, 0x004fa2f0
    void UpdateAxis(ShockNode* wheelNode, ShockNode* mountNode);      // 0x004f9f90 (ret 8)
    void Retract(float amount, const ShockCarrier* carrier);          // 0x004fa310 (ret 8)
    void ClampAndStep(float maxDelta, float dt, ShockVec3& pos);      // 0x004fa090 (ret 0xc)
    void SolveContact(float dt, const ShockVec3* offset, const ShockVec3* base,
                      const ShockVec3* point, const ShockVec3* normal, float limit,
                      float* outLoad, int* outActive);                // 0x004fa400 (ret 0x20)

    ShockVec3 axis;                  // +0xc0
    float minLength;                 // +0xcc ctor; lowered to the measured length by 0x004f9f90
    ShockNode* anchorNode;           // +0xd0 ctor; GetPositionIn is called on it in 0x004fa400
    float axisLength;                // +0xd4 ctor minLength; stores the measured |axis| in 0x004f9f90
};

// 0x004fb110: projection of v onto 'onto' (zero when 'onto' is the zero vector), cdecl with a
// hidden return pointer.
ShockVec3 ShockProject(const ShockVec3& v, const ShockVec3& onto);

class RotatingShock : public Shock {
public:
    RotatingShock(void* name, int root, float maxAngle, float armLength, float stiffnessMin,
                  float stiffnessMax, float dampingRatio, float speedThreshold, float retractScale,
                  int axisId, const ShockVec3* axisOverride);         // 0x004fa700 (ret 0x2c)
    ~RotatingShock();                                                 // 0x004fa8a0
    virtual void Reset();                                             // slot 0, 0x004fab30
    void Retract(float amount, const ShockCarrier* carrier);          // 0x004fab60 (ret 8)
    void ClampAndStep(float maxDelta, float dt);                      // 0x004fa8b0 (ret 8)
    void SolveContact(float dt, const ShockVec3* axisA, const ShockVec3* axisB,
                      const ShockVec3* point, const ShockVec3* normal, const ShockVec3* offsetA,
                      const ShockVec3* offsetB, float limit, float* outLoad,
                      int* outActive);                                // 0x004fac60 (ret 0x28)

    float maxAngle;                  // +0xc0 ctor; the chord below is 2*sin(maxAngle/2)*armLength
    float chord;                     // +0xc4 ctor 2 * sin(maxAngle * 0.5) * armLength
    float armLength;                 // +0xc8 ctor
    float armLengthSq;               // +0xcc ctor armLength * armLength
    float angle;                     // +0xd0 zeroed in Reset, accumulates the rotation applied by Retract
    ShockVec3 rotationAxis;          // +0xd4 copy of the ctor's vector, else +/-X or +/-Z picked by axisId
    int axisId;                      // +0xe0 ctor: 1 = -X, 2 = -Z, 3 = +Z, otherwise +X
};

#endif
