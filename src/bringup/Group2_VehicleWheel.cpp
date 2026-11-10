// Group2_VehicleWheel.cpp -- link shims between the canonical Tire unit and the
// views that call it (retail RTTI name Tire, ctor 0x00512f10, size 0x2c0).
//
// Status: bring-up glue, not a matching candidate. Every wheel body is the
// canonical src/krusty2/tire/Tire.cpp (strict exact except the near misses
// UpdateContactPatch and UpdateSuspensionProbe); nothing here re-implements a
// Tire function. Two directions are bridged:
//   * The vehicle code's flat call view VehicleWheel (src/krusty2/vehicle/Vehicle.h)
//     and Bike's BikeWheel (Bike.h) name the same retail functions under their own
//     class names. Each shim forwards to the Tire member at the same address; the
//     constructor builds the Tire in the storage Bike's loader allocated.
//   * Tire.cpp calls its callees through provisional area-local views (TireNode,
//     TireWorld, TireAttachA/B, TireDot ...). Each is forwarded to the existing
//     provider at the address its binding names (Tire.bindings.json).
// Arguments are passed through positionally, so the retail argument order is kept
// even where two views name a parameter differently (ApplyDrive's last two).
#include <new.h>
#include <math.h>
#include <float.h>

#include "tire/Tire.h"
#include "math/Math3D.h"                 // Vec3, Matrix4, SoultreeObject, Vec3DotCall

class SteeringControl;
class SoultreeSlot1f0;
class Vehicle;
struct SoultreeProbe;

// ---------------------------------------------------------------------------------------
// Providers for the names Tire.cpp declares.

static inline Vec3* AsVec3(CollisionVec3* v) { return (Vec3*)v; }
static inline const Vec3* AsVec3(const CollisionVec3* v) { return (const Vec3*)v; }
static inline CollisionVec3 FromVec3(const Vec3& v)
{
    CollisionVec3 r;
    r.x = v.x;
    r.y = v.y;
    r.z = v.z;
    return r;
}
static inline SoultreeObject* AsNode(TireNode* node) { return (SoultreeObject*)node; }

// 0x0040ae30 (src/reconstructed/VectorCalls.cpp).
float TireDot(const CollisionVec3* a, const CollisionVec3* b)
{
    return Vec3DotCall(AsVec3(a), AsVec3(b));
}

// 0x00515600 (src/reconstructed/CrossProduct.cpp).
Vec3 CrossProductCall(const Vec3& a, const Vec3& b);
CollisionVec3 TireCross(const CollisionVec3& a, const CollisionVec3& b)
{
    return FromVec3(CrossProductCall(*AsVec3(&a), *AsVec3(&b)));
}

// 0x0042a510 / 0x0042a580 (src/krusty2/bvh/BoundingBoxTreeQuery.cpp).
void Vec3TransformPoint(Vec3* out, Vec3 v, const Matrix4* m);
void TransformPointPtr(Vec3* out, const Vec3* v, const Matrix4* m);
void TireTransformPoint(CollisionVec3* out, CollisionVec3 v, const CollisionMatrix4* m)
{
    Vec3TransformPoint(AsVec3(out), *AsVec3(&v), (const Matrix4*)m);
}
void TireTransformDirection(CollisionVec3* out, const CollisionVec3* v, const CollisionMatrix4* m)
{
    TransformPointPtr(AsVec3(out), AsVec3(v), (const Matrix4*)m);
}

// Scene node (soultree.cpp SoultreeObject).
void TireNode::GetPositionIn(TireNode* parent, CollisionVec3* out)       // 0x004fc9a0
{
    AsNode(this)->GetPositionIn(AsNode(parent), AsVec3(out));
}
void TireNode::GetLocalBounds(CollisionVec3* a, CollisionVec3* b)        // 0x004fe0a0
{
    AsNode(this)->GetLocalBounds(AsVec3(a), AsVec3(b));
}
CollisionVec3* TireNode::LocalToWorldDirection(CollisionVec3* out, const CollisionVec3* in)  // 0x004fd5c0
{
    *AsVec3(out) = AsNode(this)->LocalToWorldDirection(*AsVec3(in));
    return out;
}
void TireNode::RotateAbout(float x, float y, float z, float angle)        // 0x004fcce0
{
    AsNode(this)->RotateAbout(x, y, z, angle);
}
void TireNode::SetPosition(CollisionVec3 position)                        // 0x004fc630
{
    AsNode(this)->SetPosition(position.x, position.y, position.z);
}
void TireNode::SetLocalMatrix(const CollisionMatrix4* m)                  // 0x004fca30
{
    AsNode(this)->SetLocalMatrix((const Matrix4*)m);
}
void TireNode::Rotate(CollisionVec3 axis, float angle)                    // 0x004fceb0
{
    AsNode(this)->Rotate(*AsVec3(&axis), angle);
}

// The ground query 0x00507c10 (src/krusty2/broadphase/Terrain.h); the world the vehicle
// passes is the Terrain. Tire.cpp ignores the result.
struct TerrainVec3;
class Terrain {
public:
    void QueryGround(TerrainVec3* pos, TerrainVec3* outNormal, int flags, unsigned char* outSurface);
};
int TireWorld::QueryGround(CollisionVec3* pos, CollisionVec3* outNormal, int flags, unsigned char* outSurface)
{
    ((Terrain*)this)->QueryGround((TerrainVec3*)pos, (TerrainVec3*)outNormal, flags, outSurface);
    return 0;
}

// MovingPart: the constructor 0x004a23a0 is src/reconstructed/MovingPart.cpp under its
// (const char*, SoultreeObject*, int) signature; Group2_TireMovingPart.cpp reaches it.
// The destructor is retail's shared empty function 0x00464e90.
void TireConstructMovingPart(void* part, void* name, int root, int ownerRef);
MovingPart::MovingPart(void* name, int root, int ownerRef)
{
    TireConstructMovingPart(this, name, root, ownerRef);
}
MovingPart::~MovingPart()
{
}

// ---------------------------------------------------------------------------------------
// The attached shocks (InlineShock at +0x2b0, RotatingShock at +0x2ac). Field view as
// src/reconstructed/Suspension.h; vptr at +0, MovingPart at +4.
static inline CollisionVec3 WheelVec3(float x, float y, float z)
{
    CollisionVec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}
static const CollisionVec3 kWheelZero = WheelVec3(0.0f, 0.0f, 0.0f);

struct WheelShock {
    void* vptr;                      // +0x00
    Matrix4 restMatrix;              // +0x04 MovingPart::matrix
    SoultreeObject* sceneNode;       // +0x44 MovingPart::node
    int ownerRef;                    // +0x48
    int movingPartField48;           // +0x4c
    int atLimit;                     // +0x50
    int extending;                   // +0x54
    CollisionVec3 spring;            // +0x58
    CollisionVec3 damper;            // +0x64
    float ratio;                     // +0x70
    CollisionVec3 displacement;      // +0x74
    CollisionVec3 stepDisplacement;  // +0x80
    float prevLength;                // +0x8c
    float clearedLength;             // +0x90
    float impulseScale;              // +0x94
    float length;                    // +0x98
    float stiffnessMin;              // +0x9c
    float stiffnessMax;              // +0xa0
    float dampingRatio;              // +0xa4
    float speedThreshold;            // +0xa8
    float retractScale;              // +0xac
    CollisionVec3 velocity;          // +0xb0
    float speed;                     // +0xbc
};
struct WheelInlineShock : WheelShock {
    CollisionVec3 axis;              // +0xc0
    float minLength;                 // +0xcc
    SoultreeObject* anchorNode;      // +0xd0
    float axisLength;                // +0xd4
};                                   // 0xd8 bytes
struct WheelRotatingShock : WheelShock {
    float maxAngle;                  // +0xc0
    float chord;                     // +0xc4
    float armLength;                 // +0xc8
    float armLengthSq;               // +0xcc
    float angle;                     // +0xd0
    CollisionVec3 rotationAxis;      // +0xd4
    int axisId;                      // +0xe0
};                                   // 0xe4 bytes
typedef char wheel_assert_inline_shock[(sizeof(WheelInlineShock) == 0xd8) ? 1 : -1];
typedef char wheel_assert_rotating_shock[(sizeof(WheelRotatingShock) == 0xe4) ? 1 : -1];

static inline float WheelDotInline(const CollisionVec3& a, const CollisionVec3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}
static CollisionVec3 WheelCross(const CollisionVec3& a, const CollisionVec3& b)
{
    CollisionVec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

// ---- shock solves (samples/physics/suspension/Shock.cpp, strict exact there) -----------

static inline float ShockLengthOf(const CollisionVec3& v)
{
    float lenSq = WheelDotInline(v, v);
    return (lenSq == 1.0f) ? 1.0f : FastSqrt(lenSq);
}

// Projection of v onto `onto` (zero when `onto` is zero): 0x004fb110.
static inline CollisionVec3 ShockProjectOnto(const CollisionVec3& v, const CollisionVec3& onto)
{
    if (onto.x == kWheelZero.x && onto.y == kWheelZero.y && onto.z == kWheelZero.z)
        return kWheelZero;
    float scale = TireDot(&v, &onto) / TireDot(&onto, &onto);
    return onto * scale;
}

// Shared tail of both ClampAndStep bodies.
static void ShockStepVelocity(WheelShock* s, float maxDelta, float dt)
{
    float delta = s->prevLength - s->length;
    if (s->length != 0.0f) {
        float clamped;
        if (delta >= 0.0f)
            clamped = (delta < maxDelta) ? delta : maxDelta;
        else
            clamped = (delta > -maxDelta) ? delta : -maxDelta;
        s->velocity = s->displacement * (clamped / (dt * s->length));
        s->stepDisplacement = s->displacement * -(delta / s->length);
    } else {
        s->velocity = kWheelZero;
        s->stepDisplacement = kWheelZero;
    }
    s->extending = (s->length > s->prevLength);
    s->prevLength = s->length;
}

// 0x004fa090 InlineShock::ClampAndStep.
static void InlineShockClampAndStep(WheelInlineShock* s, float maxDelta, float dt, CollisionVec3& pos)
{
    if (s->length >= s->minLength) {
        s->atLimit = 1;
        s->ratio = 1.0f;
        pos = s->axis * -s->minLength;
        s->displacement = s->displacement * (s->minLength / s->length);
        s->length = s->minLength;
    } else {
        s->atLimit = 0;
        s->ratio = s->length / s->minLength;
    }
    s->sceneNode->SetPosition(*AsVec3(&pos));   // 0x004fc660
    ShockStepVelocity(s, maxDelta, dt);
}

// 0x004fa400 InlineShock::SolveContact.
void TireAttachA::SolveContact(float dt, const CollisionVec3* offset, const CollisionVec3* base,
                               const CollisionVec3* point, const CollisionVec3* normal, float limit,
                               float* outLoad, int* outActive)
{
    WheelInlineShock* s = (WheelInlineShock*)this;
    int miss;
    float contactRatio;
    CollisionVec3 anchor;
    s->anchorNode->GetPositionIn(0, AsVec3(&anchor));
    CollisionVec3 toPoint = *point - anchor;
    float dist = WheelDotInline(toPoint, *normal);
    if (dist >= 0.0f)
        dist = WheelDotInline(toPoint, WheelVec3(-normal->x, normal->y, -normal->z));
    if (dist < 0.0f) {
        contactRatio = limit / dist;
        miss = 0;
    } else {
        contactRatio = 0.0f;
        miss = 1;
    }
    float len = WheelDotInline(toPoint, toPoint);
    len = (len == 1.0f) ? 1.0f : (float)sqrt(len);
    float over = s->axisLength - len;
    if (!(over > 0.0f))
        over = 0.0f;
    s->length = over - len * contactRatio;
    s->displacement = toPoint * -(s->length / len);
    CollisionVec3 pos = s->axis * -s->length;
    CollisionVec3 pointVelocity = *base + *offset;
    CollisionVec3 proj = ShockProjectOnto(pointVelocity, s->displacement);
    s->speed = ShockLengthOf(proj);
    InlineShockClampAndStep(s, s->speed * dt, dt, pos);
    if (s->atLimit == 0 && miss == 0) {
        *outLoad = 0.0f;
        *outActive = 0;
        return;
    }
    *outLoad = limit - WheelDotInline(s->stepDisplacement, *normal);
    *outActive = 1;
}

// 0x004fa8b0 RotatingShock::ClampAndStep.
static void RotatingShockClampAndStep(WheelRotatingShock* s, float maxDelta, float dt)
{
    if (s->length >= s->chord) {
        float limitScale = s->chord / s->length;
        s->atLimit = 1;
        s->ratio = 1.0f;
        s->angle = s->maxAngle;
        s->displacement = s->displacement * limitScale;
        s->length = s->chord;
    } else {
        s->angle = s->length / s->armLength;
        s->atLimit = 0;
        s->ratio = s->length / s->chord;
    }
    if (_finite(s->angle)) {
        s->sceneNode->SetLocalMatrix(&s->restMatrix);                  // 0x004fca30
        s->sceneNode->Rotate(*AsVec3(&s->rotationAxis), s->angle);     // 0x004fceb0
    }
    ShockStepVelocity(s, maxDelta, dt);
}

// 0x004fac60 RotatingShock::SolveContact.
void TireAttachB::SolveContact(float dt, const CollisionVec3* axisA, const CollisionVec3* axisB,
                               const CollisionVec3* point, const CollisionVec3* normal,
                               const CollisionVec3* offsetA, const CollisionVec3* offsetB, float limit,
                               float* outLoad, int* outActive)
{
    WheelRotatingShock* s = (WheelRotatingShock*)this;
    int miss = 0;
    float contactRatio;
    CollisionVec3 anchor;
    s->sceneNode->GetPositionIn(0, AsVec3(&anchor));
    CollisionVec3 toPoint = *point - anchor;
    CollisionVec3 swing = kWheelZero;   // best guess: retail leaves it unset for axisId > 3
    switch (s->axisId) {
    case 0: swing = WheelCross(*axisA, toPoint); break;
    case 1: swing = WheelCross(toPoint, *axisA); break;
    case 2: swing = WheelCross(*axisB, toPoint); break;
    case 3: swing = WheelCross(toPoint, *axisB); break;
    }
    float dist = (swing.x * normal->x + swing.y * normal->y) + swing.z * normal->z;
    if (dist >= 0.0f) {
        if (dist == 0.0f) {
            *outLoad = limit;
            float side = (offsetA->y * normal->y + normal->x * offsetA->x) + offsetA->z * normal->z;
            *outActive = (side < -1.0f) ? 1 : 0;
            s->speed = 0.0f;
            return;
        }
        dist = WheelDotInline(swing, WheelVec3(-normal->x, normal->y, -normal->z));
    }
    if (dist < 0.0f) {
        contactRatio = limit / dist;
    } else {
        contactRatio = 0.0f;
        miss = 1;
    }
    s->length = s->clearedLength - contactRatio * s->armLength;
    s->displacement = swing * ((-s->length) / s->armLength);
    s->speed = ShockLengthOf(ShockProjectOnto(*offsetA + *offsetB, s->displacement));
    RotatingShockClampAndStep(s, s->speed * dt, dt);
    if (s->atLimit == 0 && miss == 0) {
        *outLoad = 0.0f;
        *outActive = 0;
        return;
    }
    *outLoad = limit - TireDot(normal, &s->stepDisplacement);
    *outActive = 1;
}

// Both shock destructors (0x004f9f80, 0x004fa8a0) only run the empty MovingPart destructor.
TireAttachA::~TireAttachA()
{
}
TireAttachB::~TireAttachB()
{
}

// ---------------------------------------------------------------------------------------
// The vehicle code's view of the wheel (Vehicle.h VehicleWheel): same storage, so each
// member forwards to the Tire member at the same retail address.
struct VehicleWheel {
    VehicleWheel(const char* name, SoultreeObject* root, float rollFriction_, float sideFriction_,
                 int field_0x298_, SteeringControl* steer, float field_0x2b4_,
                 float rollFrictionScale_, float sideFrictionScale_, float field_0x2a0_,
                 float field_0x1d8_, float field_0x1dc_, float field_0x1e0_, float blendWeight_,
                 SoultreeSlot1f0* track);
    void UpdateRoll(float dt, float distanceScale, int locked, int driven, int crashState,
                    float driveScale);
    void UpdateContactPatch(Vec3* pos, Vec3 axis, Vec3 ref, float minLength, float* outValue,
                            int* outSign);
    void UpdateShock(float dt, int pointsTouching, int crashed, float unused, Vec3* velocity,
                     Vec3* axisB);
    void UpdateDriveShare(Vehicle* owner);
    void ApplyDrive(float share, float stepTime, int forward, float mass, float* speed, Vec3* a6,
                    Vec3* a7);
    void UpdateSuspensionProbe(SoultreeProbe* world, const Vec3* velocity, float angle,
                               float radiusScale, float yaw, const Vec3* forward, const Vec3* up,
                               SoultreeObject* node);

    char storage[0x2c0];
};
typedef char wheel_assert_storage[(sizeof(VehicleWheel) == sizeof(Tire)) ? 1 : -1];

static inline Tire* AsTire(VehicleWheel* wheel) { return (Tire*)wheel; }

// 0x00512f10: Bike's loader allocates 0x2c0 bytes and constructs the wheel in them.
VehicleWheel::VehicleWheel(const char* name, SoultreeObject* root, float rollFriction_,
                           float sideFriction_, int field_0x298_, SteeringControl* steer,
                           float field_0x2b4_, float rollFrictionScale_, float sideFrictionScale_,
                           float field_0x2a0_, float field_0x1d8_, float field_0x1dc_,
                           float field_0x1e0_, float blendWeight_, SoultreeSlot1f0* track)
{
    ::new ((void*)this) Tire((void*)name, (int)root, rollFriction_, sideFriction_, field_0x298_,
                             (int)steer, field_0x2b4_, rollFrictionScale_, sideFrictionScale_,
                             field_0x2a0_, field_0x1d8_, field_0x1dc_, field_0x1e0_, blendWeight_,
                             (int)track);
}

void VehicleWheel::UpdateRoll(float dt, float distanceScale, int locked, int driven, int crashState,
                              float driveScale)                                   // 0x005143d0
{
    AsTire(this)->UpdateRoll(dt, distanceScale, locked, driven, crashState, driveScale);
}

void VehicleWheel::UpdateContactPatch(Vec3* pos, Vec3 axis, Vec3 ref, float minLength,
                                      float* outValue, int* outSign)              // 0x005135f0
{
    AsTire(this)->UpdateContactPatch((const CollisionVec3*)pos, FromVec3(axis), FromVec3(ref),
                                     minLength, outValue, outSign);
}

// The fourth argument (Vehicle passes +0xbc) is unused by 0x00513c70.
void VehicleWheel::UpdateShock(float dt, int pointsTouching, int crashed, float unused,
                               Vec3* velocity, Vec3* axisB)                      // 0x00513c70
{
    AsTire(this)->UpdateShock(dt, pointsTouching, crashed, 0, (const CollisionVec3*)velocity,
                              (const CollisionVec3*)axisB);
}

void VehicleWheel::UpdateDriveShare(Vehicle* owner)                              // 0x00513f90
{
    AsTire(this)->UpdateDriveShare((TireVehicle*)owner);
}

void VehicleWheel::ApplyDrive(float share, float stepTime, int forward, float mass, float* speed,
                              Vec3* a6, Vec3* a7)                                // 0x00514170
{
    AsTire(this)->ApplyDrive(share, stepTime, forward, mass, speed, (CollisionVec3*)a6,
                             (CollisionVec3*)a7);
}

void VehicleWheel::UpdateSuspensionProbe(SoultreeProbe* world, const Vec3* velocity, float angle,
                                         float radiusScale, float yaw, const Vec3* forward,
                                         const Vec3* up, SoultreeObject* node)   // 0x00514550
{
    AsTire(this)->UpdateSuspensionProbe((TireWorld*)world, (const CollisionVec3*)velocity, angle,
                                        radiusScale, yaw, (const CollisionVec3*)forward,
                                        (const CollisionVec3*)up, (TireNode*)node);
}

// KrustyBike's view of the same wheel (src/krusty2/vehicle/Bike.h BikeWheel) calls
// 0x00513f90 with the bike as an untyped pointer.
struct BikeWheel {
    void UpdateDriveShare(void* vehicle);
};

void BikeWheel::UpdateDriveShare(void* vehicle)
{
    ((Tire*)this)->UpdateDriveShare((TireVehicle*)vehicle);
}
