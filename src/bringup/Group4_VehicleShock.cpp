// Bring-up definitions (best-guess-unreviewed) for the Vehicle.h suspension views
// VehicleShock / VehicleInlineShock that Vehicle.cpp and Bike.cpp call:
//   0x004f9c70  VehicleShock::ClearForces        (Shock slot 1, shared body)
//   0x004f9d30  VehicleShock::ApplyDamping       (ret 0x10; 4th argument unused)
//   0x004f9ee0  VehicleInlineShock constructor   (ret 0x24, 0xd8 bytes)
//   0x004fa310  VehicleInlineShock::Retract      (ret 8)
//
// Vehicle.h's views carry no vptr, but the retail objects are the polymorphic
// Shock / InlineShock (RTTI, vtables 0x00557b84 / 0x00557b90: slot 0 Reset,
// slot 1 ClearForces; MovingPart at +4). The bodies follow the strict-exact
// candidate samples/physics/suspension/Shock.cpp (same retail bytes, decoded
// again here); they run on a private mirror class Group4Veh_InlineShock with
// that layout, and the Vehicle.h constructor builds that mirror in place so
// the object gets a two-slot {Reset, ClearForces} vtable like retail.
// best guess: the vtable is this unit's copy, not the retail address
// 0x00557b90; no code compares shock vptrs.
#include "../krusty2/vehicle/Vehicle.h"
#include "../krusty2/math/FastMath.h"
#include <float.h>
#include <new.h>

namespace {

struct Group4Veh_Vec3 {
    float x, y, z;
    Group4Veh_Vec3() {}
    Group4Veh_Vec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    float LengthSquared() const { return z * z + (x * x + y * y); }
    Group4Veh_Vec3 operator*(float s) const { return Group4Veh_Vec3(x * s, y * s, z * s); }
    Group4Veh_Vec3& operator+=(const Group4Veh_Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
};

// The shock TU's private constant vectors (retail 0x00689e58 zero, 0x00689e78 +Y).
const Group4Veh_Vec3 kGroup4VehZero(0.0f, 0.0f, 0.0f);
const Group4Veh_Vec3 kGroup4VehAxisY(0.0f, 1.0f, 0.0f);

} // namespace

// MovingPart (src/reconstructed/MovingPart.h; that header's SoultreeObject view
// conflicts with Vehicle.h's, so the class is declared here with the same
// mangled constructor 0x004a23a0, ret 0xc). Non-polymorphic, 0x4c bytes.
class MovingPart {
public:
    MovingPart(const char* name, SoultreeObject* root, int field_0x44_);
    char field_0x00[0x40];
    SoultreeObject* node;   // +0x40 (+0x44 in a shock): the node the shock moves
    int field_0x44;
    int field_0x48;
};

// Shock (vptr +0, MovingPart +4, 0xc0 bytes).
class Group4Veh_Shock : public MovingPart {
public:
    // 0x004f9a60 (ret 0x1c)
    Group4Veh_Shock(const char* name, SoultreeObject* root, float stiffnessMin_, float stiffnessMax_,
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
        spring = kGroup4VehZero;
        damper = kGroup4VehZero;
        displacement = kGroup4VehZero;
        stepDisplacement = kGroup4VehZero;
        velocity = kGroup4VehZero;
        speed = 0.0f;
        impulseScale = 0.0f;
    }
    virtual void Reset();          // slot 0, 0x004f9ba0
    virtual void ClearForces();    // slot 1, 0x004f9c70
    void ApplyDamping(float loadWeight, float massFactor, float dt);

    int atLimit;                   // +0x50
    int extending;                 // +0x54
    Group4Veh_Vec3 spring;         // +0x58
    Group4Veh_Vec3 damper;         // +0x64
    float ratio;                   // +0x70
    Group4Veh_Vec3 displacement;   // +0x74
    Group4Veh_Vec3 stepDisplacement; // +0x80
    float prevLength;              // +0x8c
    float clearedLength;           // +0x90
    float impulseScale;            // +0x94
    float length;                  // +0x98
    float stiffnessMin;            // +0x9c
    float stiffnessMax;            // +0xa0
    float dampingRatio;            // +0xa4
    float speedThreshold;          // +0xa8
    float retractScale;            // +0xac
    Group4Veh_Vec3 velocity;       // +0xb0
    float speed;                   // +0xbc
};

// 0x004f9ba0
void Group4Veh_Shock::Reset()
{
    atLimit = 0;
    ratio = 0.0f;
    length = 0.0f;
    spring = kGroup4VehZero;
    damper = kGroup4VehZero;
    displacement = kGroup4VehZero;
    stepDisplacement = kGroup4VehZero;
    velocity = kGroup4VehZero;
    speed = 0.0f;
    extending = 0;
    prevLength = 0.0f;
    clearedLength = 0.0f;
    impulseScale = 0.0f;
}

// 0x004f9c70
void Group4Veh_Shock::ClearForces()
{
    atLimit = 0;
    spring = kGroup4VehZero;
    damper = kGroup4VehZero;
    clearedLength = length;
    extending = 0;
    speed = 0.0f;
    displacement = kGroup4VehZero;
    stepDisplacement = kGroup4VehZero;
    velocity = kGroup4VehZero;
}

// 0x004f9d30: spring = displacement * stiffness + damper, where the stiffness is
// interpolated by the compression ratio and the damper is critical damping
// (-2 * sqrt(m k) * ratio) on the velocity, softened above speedThreshold.
void Group4Veh_Shock::ApplyDamping(float loadWeight, float massFactor, float dt)
{
    float stiffness = stiffnessMin + (stiffnessMax - stiffnessMin) * ratio;
    float mass = loadWeight * massFactor;
    float len = velocity.LengthSquared();
    len = (len == 1.0f) ? 1.0f : FastSqrt(len);
    if (len <= speedThreshold) {
        float coeff = FastSqrt(mass * stiffness) * dampingRatio * -2.0f;
        damper = velocity * coeff;
    } else {
        float coeff = FastSqrt(mass * stiffness) * dampingRatio * -2.0f;
        coeff *= ((len - speedThreshold) * 0.5f + speedThreshold) / len;
        damper = velocity * coeff;
    }
    spring = displacement * stiffness;
    spring += damper;
}

// InlineShock (0xd8 bytes).
class Group4Veh_InlineShock : public Group4Veh_Shock {
public:
    // 0x004f9ee0
    Group4Veh_InlineShock(const char* name, SoultreeObject* root, SoultreeObject* anchorNode_,
                          float minLength_, float stiffnessMin_, float stiffnessMax_,
                          float dampingRatio_, float speedThreshold_, float retractScale_)
        : Group4Veh_Shock(name, root, stiffnessMin_, stiffnessMax_, dampingRatio_, speedThreshold_,
                          retractScale_)
    {
        minLength = minLength_;
        axisLength = minLength_;
        anchorNode = anchorNode_;
        axis = Group4Veh_Vec3(-kGroup4VehAxisY.x, -kGroup4VehAxisY.y, -kGroup4VehAxisY.z);
    }
    virtual void Reset();          // slot 0, 0x004fa2f0
    void Retract(float amount, const void* carrier);

    Group4Veh_Vec3 axis;           // +0xc0
    float minLength;               // +0xcc
    SoultreeObject* anchorNode;    // +0xd0
    float axisLength;              // +0xd4
};

typedef char group4veh_check_size[(sizeof(Group4Veh_InlineShock) == 0xd8) ? 1 : -1];
typedef char group4veh_check_view[(sizeof(VehicleInlineShock) == 0xd8) ? 1 : -1];

// 0x004fa2f0
void Group4Veh_InlineShock::Reset()
{
    node->SetPosition(0.0f, 0.0f, 0.0f);
    Group4Veh_Shock::Reset();
}

// 0x004fa310: shortens the shock by `amount` scaled by minLength / (carrier
// scale * retractScale); a non-finite or negative step is ignored, a step past
// the current length resets the shock (virtual slot 0).
void Group4Veh_InlineShock::Retract(float amount, const void* carrier)
{
    if (ratio != 0.0f) {
        // The carrier's float at +0x2a4 (Suspension.h ShockCarrier::field_0x2a4).
        float carrierScale = *(const float*)((const char*)carrier + 0x2a4);
        float retraction = amount * (minLength / (carrierScale * retractScale));
        if (!_finite(retraction) || retraction < 0.0f)
            return;
        if (retraction >= length) {
            Reset();
            return;
        }
        length = length - retraction;
        ratio = length / minLength;
        Group4Veh_Vec3 p = axis * -length;
        node->SetPosition(p.x, p.y, p.z);
    }
    Group4Veh_Shock::ClearForces();
}

// ---- the Vehicle.h entry points ------------------------------------------

void VehicleShock::ClearForces()
{
    reinterpret_cast<Group4Veh_Shock*>(this)->Group4Veh_Shock::ClearForces();
}

void VehicleShock::ApplyDamping(float loadWeight, float massFactor, float dt, float* speed)
{
    // The fourth argument is never read by the retail body.
    reinterpret_cast<Group4Veh_Shock*>(this)->ApplyDamping(loadWeight, massFactor, dt);
}

VehicleInlineShock::VehicleInlineShock(const char* name, SoultreeObject* root, SoultreeObject* anchor,
                                       float minLength, float stiffnessMin, float stiffnessMax,
                                       float dampingRatio, float speedThreshold, float retractScale)
{
    new (this) Group4Veh_InlineShock(name, root, anchor, minLength, stiffnessMin, stiffnessMax,
                                     dampingRatio, speedThreshold, retractScale);
}

void VehicleInlineShock::Retract(float amount, const VehicleWheel* carrier)
{
    reinterpret_cast<Group4Veh_InlineShock*>(this)->Retract(amount, carrier);
}
