// Group4_VehicleShock.cpp -- link shims between the canonical Shock family
// (src/krusty2/suspension/Shock.cpp: Shock, InlineShock, RotatingShock; RTTI, vtables
// 0x00557b84 / 0x00557b90 / 0x00557b9c, all 27 targets strict exact) and the views that
// call it.
//
// Status: bring-up glue, not a matching candidate. Nothing here re-implements a shock
// function; three directions are bridged:
//   * Vehicle.h's flat call views VehicleShock / VehicleInlineShock / VehicleRotatingShock
//     (Bike.cpp builds them, Vehicle.cpp retracts and damps them) name the same retail
//     functions. Each forwards to the canonical member at the same address; the
//     constructors build the canonical object, with its real vtable, in the storage Bike's
//     loader allocated (0xd8 / 0xe4 bytes). They call the canonical constructor explicitly
//     (an MSVC extension) rather than through placement new: placement new added an EH
//     frame and a null test, so tools/linked_accuracy.py scored that wrapper (0.25 / 0.49)
//     instead of following the plain forwarder to the exact constructor it calls.
//   * Tire.cpp reaches the wheel's shocks through TireAttachA/B (TireAttachmentViews.h).
//   * Shock.cpp calls its callees through Suspension.h's provisional ShockNode and
//     Shock*Call names; each is forwarded to the existing provider at the address its
//     binding names (Shock.bindings.json).
#include "vehicle/Vehicle.h"
#include "Suspension.h"
#include "TireAttachmentViews.h"

static inline SoultreeObject* AsNode(ShockNode* node) { return (SoultreeObject*)node; }
static inline Vec3* AsVec3(ShockVec3* v) { return (Vec3*)v; }
static inline const Vec3* AsVec3(const ShockVec3* v) { return (const Vec3*)v; }
static inline ShockVec3 FromVec3(const Vec3& v) { return ShockVec3(v.x, v.y, v.z); }
static inline const ShockVec3* AsShockVec3(const CollisionVec3* v) { return (const ShockVec3*)v; }

// ---------------------------------------------------------------------------------------
// Providers for the names Shock.cpp declares.

// Scene node (soultree.cpp SoultreeObject).
void ShockNode::SetPosition(float x, float y, float z)            // 0x004fc630
{
    AsNode(this)->SetPosition(x, y, z);
}
void ShockNode::SetPosition(ShockVec3 p)                           // 0x004fc630
{
    AsNode(this)->SetPosition(p.x, p.y, p.z);
}
void ShockNode::SetPositionVec(const ShockVec3& p)                 // 0x004fc660
{
    AsNode(this)->SetPosition(*AsVec3(&p));
}
void ShockNode::GetPositionIn(ShockNode* frame, ShockVec3* out)    // 0x004fc9a0
{
    AsNode(this)->GetPositionIn(AsNode(frame), AsVec3(out));
}
void ShockNode::SetMatrixLike(const void* m)                       // 0x004fca30
{
    AsNode(this)->SetLocalMatrix((const Matrix4*)m);
}
void ShockNode::Rotate(ShockVec3 axis, float angle)                // 0x004fceb0
{
    AsNode(this)->Rotate(*AsVec3(&axis), angle);
}

// Out-of-line vector helpers (src/reconstructed/VectorCalls.cpp; 0x00435ec0 is
// CollisionObject.cpp's CollisionLength).
float ShockDotCall(const ShockVec3& a, const ShockVec3& b)        // 0x0040ae30
{
    return Vec3DotCall(AsVec3(&a), AsVec3(&b));
}
ShockVec3 ShockScaleCall(const ShockVec3& v, float s)             // 0x005015b0
{
    Vec3 r;
    Vec3ScaleCall(&r, AsVec3(&v), s);
    return FromVec3(r);
}
ShockVec3 ShockAddCall(const ShockVec3& a, const ShockVec3& b)    // 0x00421cb0
{
    Vec3 r;
    Vec3AddCall(&r, AsVec3(&a), AsVec3(&b));
    return FromVec3(r);
}
float CollisionLength(const CollisionVec3* v);
float ShockLength(const ShockVec3& v)                              // 0x00435ec0
{
    return CollisionLength((const CollisionVec3*)&v);
}

// ---------------------------------------------------------------------------------------
// Tire's views of its attached shocks (InlineShock at Tire+0x2b0, RotatingShock at +0x2ac).

void TireAttachA::SolveContact(float dt, const CollisionVec3* offset, const CollisionVec3* base,
                               const CollisionVec3* point, const CollisionVec3* normal, float limit,
                               float* outLoad, int* outActive)          // 0x004fa400
{
    ((InlineShock*)this)->SolveContact(dt, AsShockVec3(offset), AsShockVec3(base), AsShockVec3(point),
                                       AsShockVec3(normal), limit, outLoad, outActive);
}
TireAttachA::~TireAttachA()                                             // 0x004f9f80
{
    ((InlineShock*)this)->InlineShock::~InlineShock();
}
void TireAttachB::SolveContact(float dt, const CollisionVec3* axisA, const CollisionVec3* axisB,
                               const CollisionVec3* point, const CollisionVec3* normal,
                               const CollisionVec3* offsetA, const CollisionVec3* offsetB, float limit,
                               float* outLoad, int* outActive)          // 0x004fac60
{
    ((RotatingShock*)this)->SolveContact(dt, AsShockVec3(axisA), AsShockVec3(axisB), AsShockVec3(point),
                                         AsShockVec3(normal), AsShockVec3(offsetA), AsShockVec3(offsetB),
                                         limit, outLoad, outActive);
}
TireAttachB::~TireAttachB()                                             // 0x004fa8a0
{
    ((RotatingShock*)this)->RotatingShock::~RotatingShock();
}

// ---------------------------------------------------------------------------------------
// The vehicle code's views (Vehicle.h): same objects, primary pointer at offset 0.

typedef char shock_assert_inline[(sizeof(VehicleInlineShock) == sizeof(InlineShock)) ? 1 : -1];
typedef char shock_assert_rotating[(sizeof(VehicleRotatingShock) == sizeof(RotatingShock)) ? 1 : -1];

void VehicleShock::ClearForces()                                        // 0x004f9c70
{
    ((Shock*)this)->Shock::ClearForces();
}

void VehicleShock::ApplyDamping(float loadWeight, float massFactor, float dt, float* speed)  // 0x004f9d30
{
    // Shock.cpp declares the fourth argument as a float; the retail body never reads it.
    ((Shock*)this)->ApplyDamping(loadWeight, massFactor, dt, 0.0f);
}

VehicleInlineShock::VehicleInlineShock(const char* name, SoultreeObject* root, SoultreeObject* anchor,
                                       float minLength, float stiffnessMin, float stiffnessMax,
                                       float dampingRatio, float speedThreshold, float retractScale)  // 0x004f9ee0
{
    ((InlineShock*)this)->InlineShock::InlineShock((void*)name, (int)root, (ShockNode*)anchor, minLength,
                                                   stiffnessMin, stiffnessMax, dampingRatio,
                                                   speedThreshold, retractScale);
}

void VehicleInlineShock::Retract(float amount, const VehicleWheel* carrier)    // 0x004fa310
{
    ((InlineShock*)this)->Retract(amount, (const ShockCarrier*)carrier);
}

VehicleRotatingShock::VehicleRotatingShock(const char* name, SoultreeObject* root, float swingAngle,
                                           float armLength, float stiffnessMin, float stiffnessMax,
                                           float dampingRatio, float speedThreshold, float retractScale,
                                           int axisId, const Vec3* axisOverride)  // 0x004fa700
{
    ((RotatingShock*)this)->RotatingShock::RotatingShock((void*)name, (int)root, swingAngle, armLength,
                                                         stiffnessMin, stiffnessMax, dampingRatio,
                                                         speedThreshold, retractScale, axisId,
                                                         (const ShockVec3*)axisOverride);
}

void VehicleRotatingShock::Retract(float amount, const VehicleWheel* carrier)  // 0x004fab60
{
    ((RotatingShock*)this)->Retract(amount, (const ShockCarrier*)carrier);
}
