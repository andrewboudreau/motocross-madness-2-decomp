// Group5 bring-up definitions (best-guess, link frontier). Not strict matches.
//
// Vehicle.h's VehicleRotatingShock view, which Bike.cpp builds and Vehicle.cpp
// retracts:
//   0x004fa700  constructor (ret 0x2c, 0xe4 bytes)
//   0x004fab60  Retract (ret 8)
// Vehicle.h's view carries no vptr, but the retail object is the polymorphic
// RotatingShock (vtable 0x00557b9c: slot 0 Reset 0x004fab30, slot 1 the
// shared Shock::ClearForces 0x004f9c70; MovingPart at +4). The bodies follow
// samples/physics/suspension/Shock.cpp, checked again against the retail
// disassembly of all three functions. As in Group4_VehicleShock.cpp (the
// InlineShock twin), they run on a private mirror class with that layout, and
// the Vehicle.h constructor builds the mirror in place so the object gets a
// two-slot {Reset, ClearForces} vtable like retail.
// best guess: the vtable is this unit's copy, not the retail address
// 0x00557b9c; no code compares shock vptrs.
#include "../krusty2/vehicle/Vehicle.h"
#include <float.h>
#include <math.h>
#include <new.h>

namespace {

struct Group5Veh_Vec3 {
    float x, y, z;
    Group5Veh_Vec3() {}
    Group5Veh_Vec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};

// The shock TU's private constant vectors (retail 0x00689e58 zero, 0x00689e68
// +X, 0x00689e48 +Z).
const Group5Veh_Vec3 kGroup5VehZero(0.0f, 0.0f, 0.0f);
const Group5Veh_Vec3 kGroup5VehAxisX(1.0f, 0.0f, 0.0f);
const Group5Veh_Vec3 kGroup5VehAxisZ(0.0f, 0.0f, 1.0f);

} // namespace

// MovingPart (src/reconstructed/MovingPart.h; that header's SoultreeObject
// view conflicts with Vehicle.h's, so the class is declared here with the same
// mangled constructor 0x004a23a0, ret 0xc). Non-polymorphic, 0x4c bytes; it
// starts with the node's rest matrix, which Reset restores.
class MovingPart {
public:
    MovingPart(const char* name, SoultreeObject* root, int field_0x44_);
    char field_0x00[0x40];
    SoultreeObject* node;   // +0x40 (+0x44 in a shock): the node the shock moves
    int field_0x44;
    int field_0x48;
};

// Shock (vptr +0, MovingPart +4, 0xc0 bytes).
class Group5Veh_Shock : public MovingPart {
public:
    // 0x004f9a60 (ret 0x1c)
    Group5Veh_Shock(const char* name, SoultreeObject* root, float stiffnessMin_, float stiffnessMax_,
                    float dampingRatio_, float speedThreshold_, float retractScale_)
        : MovingPart(name, root, 0)
    {
        stiffnessMin = stiffnessMin_;
        stiffnessMax = stiffnessMax_;
        dampingRatio = dampingRatio_;
        atLimit = 0;
        extending = 0;
        prevLength = 0.0f;
        clearedLength = 0.0f;
        length = 0.0f;
        ratio = 0.0f;
        speedThreshold = speedThreshold_;
        retractScale = retractScale_;
        spring = kGroup5VehZero;
        damper = kGroup5VehZero;
        displacement = kGroup5VehZero;
        stepDisplacement = kGroup5VehZero;
        velocity = kGroup5VehZero;
        speed = 0.0f;
        impulseScale = 0.0f;
    }
    virtual void Reset();          // slot 0, 0x004f9ba0
    virtual void ClearForces();    // slot 1, 0x004f9c70

    int atLimit;                   // +0x50
    int extending;                 // +0x54
    Group5Veh_Vec3 spring;         // +0x58
    Group5Veh_Vec3 damper;         // +0x64
    float ratio;                   // +0x70
    Group5Veh_Vec3 displacement;   // +0x74
    Group5Veh_Vec3 stepDisplacement; // +0x80
    float prevLength;              // +0x8c
    float clearedLength;           // +0x90
    float impulseScale;            // +0x94
    float length;                  // +0x98
    float stiffnessMin;            // +0x9c
    float stiffnessMax;            // +0xa0
    float dampingRatio;            // +0xa4
    float speedThreshold;          // +0xa8
    float retractScale;            // +0xac
    Group5Veh_Vec3 velocity;       // +0xb0
    float speed;                   // +0xbc
};

// 0x004f9ba0
void Group5Veh_Shock::Reset()
{
    atLimit = 0;
    ratio = 0.0f;
    length = 0.0f;
    spring = kGroup5VehZero;
    damper = kGroup5VehZero;
    displacement = kGroup5VehZero;
    stepDisplacement = kGroup5VehZero;
    velocity = kGroup5VehZero;
    speed = 0.0f;
    extending = 0;
    prevLength = 0.0f;
    clearedLength = 0.0f;
    impulseScale = 0.0f;
}

// 0x004f9c70
void Group5Veh_Shock::ClearForces()
{
    atLimit = 0;
    spring = kGroup5VehZero;
    damper = kGroup5VehZero;
    clearedLength = length;
    extending = 0;
    speed = 0.0f;
    displacement = kGroup5VehZero;
    stepDisplacement = kGroup5VehZero;
    velocity = kGroup5VehZero;
}

// RotatingShock (0xe4 bytes): the shock node swings about rotationAxis.
class Group5Veh_RotatingShock : public Group5Veh_Shock {
public:
    // 0x004fa700
    Group5Veh_RotatingShock(const char* name, SoultreeObject* root, float maxAngle_, float armLength_,
                            float stiffnessMin_, float stiffnessMax_, float dampingRatio_,
                            float speedThreshold_, float retractScale_, int axisId_,
                            const Vec3* axisOverride)
        : Group5Veh_Shock(name, root, stiffnessMin_, stiffnessMax_, dampingRatio_, speedThreshold_,
                          retractScale_)
    {
        maxAngle = maxAngle_;
        armLengthSq = armLength_ * armLength_;
        armLength = armLength_;
        axisId = axisId_;
        chord = 2.0f * (float)sin(maxAngle_ * 0.5f) * armLength_;
        if (axisOverride) {
            rotationAxis = Group5Veh_Vec3(axisOverride->x, axisOverride->y, axisOverride->z);
            return;
        }
        switch (axisId_) {
        case 1:
            rotationAxis = Group5Veh_Vec3(-kGroup5VehAxisX.x, -kGroup5VehAxisX.y, -kGroup5VehAxisX.z);
            break;
        case 2:
            rotationAxis = Group5Veh_Vec3(-kGroup5VehAxisZ.x, -kGroup5VehAxisZ.y, -kGroup5VehAxisZ.z);
            break;
        case 3:
            rotationAxis = kGroup5VehAxisZ;
            break;
        default:
            rotationAxis = kGroup5VehAxisX;
            break;
        }
    }
    virtual void Reset();          // slot 0, 0x004fab30
    void Retract(float amount, const void* carrier);

    float maxAngle;                // +0xc0
    float chord;                   // +0xc4 2 * sin(maxAngle / 2) * armLength
    float armLength;               // +0xc8
    float armLengthSq;             // +0xcc
    float angle;                   // +0xd0 rotation applied so far
    Group5Veh_Vec3 rotationAxis;   // +0xd4
    int axisId;                    // +0xe0 1 = -X, 2 = -Z, 3 = +Z, otherwise +X
};

typedef char group5veh_check_size[(sizeof(Group5Veh_RotatingShock) == 0xe4) ? 1 : -1];
typedef char group5veh_check_view[(sizeof(VehicleRotatingShock) == 0xe4) ? 1 : -1];

// 0x004fab30: restores the node's rest matrix (MovingPart +0) and the shock.
void Group5Veh_RotatingShock::Reset()
{
    node->SetLocalMatrix(reinterpret_cast<const Matrix4*>(field_0x00));
    angle = 0.0f;
    Group5Veh_Shock::Reset();
}

// 0x004fab60: shortens the shock by `amount` scaled by chord / (carrier scale
// * retractScale) and swings the node back by the matching angle; a
// non-finite or negative step is ignored, a step past the current length
// resets the shock (virtual slot 0).
void Group5Veh_RotatingShock::Retract(float amount, const void* carrier)
{
    if (ratio != 0.0f) {
        // The carrier's float at +0x2a4 (Suspension.h ShockCarrier::field_0x2a4).
        float carrierScale = *(const float*)((const char*)carrier + 0x2a4);
        float retraction = amount * (chord / (carrierScale * retractScale));
        if (!_finite(retraction) || retraction < 0.0f)
            return;
        if (retraction >= length) {
            Reset();
            return;
        }
        length = length - retraction;
        float delta = length / armLength - angle;
        if (_finite(delta)) {
            node->Rotate(Vec3(rotationAxis.x, rotationAxis.y, rotationAxis.z), delta);
            ratio = length / chord;
            angle = delta + angle;
        }
    }
    Group5Veh_Shock::ClearForces();
}

// ---- the Vehicle.h entry points ------------------------------------------

VehicleRotatingShock::VehicleRotatingShock(const char* name, SoultreeObject* root, float swingAngle,
                                           float armLength, float stiffnessMin, float stiffnessMax,
                                           float dampingRatio, float speedThreshold, float retractScale,
                                           int axisId, const Vec3* axisOverride)
{
    new (this) Group5Veh_RotatingShock(name, root, swingAngle, armLength, stiffnessMin, stiffnessMax,
                                       dampingRatio, speedThreshold, retractScale, axisId, axisOverride);
}

void VehicleRotatingShock::Retract(float amount, const VehicleWheel* carrier)
{
    reinterpret_cast<Group5Veh_RotatingShock*>(this)->Retract(amount, carrier);
}
