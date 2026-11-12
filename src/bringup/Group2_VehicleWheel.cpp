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
//     TireWorld, TireDot ...). Each is forwarded to the existing provider at the
//     address its binding names (Tire.bindings.json). The shock views TireAttachA/B
//     are forwarded to the canonical Shock family in Group4_VehicleShock.cpp.
// Arguments are passed through positionally, so the retail argument order is kept
// even where two views name a parameter differently.
#include <new.h>

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
// The vehicle code's view of the wheel (Vehicle.h VehicleWheel): same storage, so each
// member forwards to the Tire member at the same retail address.
struct VehicleWheel {
    VehicleWheel(const char* name, SoultreeObject* root, float rollFriction_, float sideFriction_,
                 int field_0x298_, SteeringControl* steer, float slipThreshold_,
                 float rollFrictionScale_, float sideFrictionScale_, float driveStrength_,
                 float driveTorqueScale_, float levelRiseRate_, float levelFallRate_, float blendWeight_,
                 SoultreeSlot1f0* track);
    void UpdateRoll(float dt, float distanceScale, int locked, int driven, int crashState,
                    float driveScale);
    void UpdateContactPatch(Vec3* pos, Vec3 axis, Vec3 ref, float minLength, float* outValue,
                            int* outSign);
    void UpdateShock(float dt, int pointsTouching, int crashed, float unused, Vec3* velocity,
                     Vec3* axisB);
    void UpdateDriveShare(Vehicle* owner);
    void ApplyDrive(float share, float stepTime, int forward, float mass, float* speed, Vec3* torque,
                    Vec3* force);
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
                           float slipThreshold_, float rollFrictionScale_, float sideFrictionScale_,
                           float driveStrength_, float driveTorqueScale_, float levelRiseRate_,
                           float levelFallRate_, float blendWeight_, SoultreeSlot1f0* track)
{
    ::new ((void*)this) Tire((void*)name, (int)root, rollFriction_, sideFriction_, field_0x298_,
                             (int)steer, slipThreshold_, rollFrictionScale_, sideFrictionScale_,
                             driveStrength_, driveTorqueScale_, levelRiseRate_, levelFallRate_, blendWeight_,
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
                              Vec3* torque, Vec3* force)                         // 0x00514170
{
    AsTire(this)->ApplyDrive(share, stepTime, forward, mass, speed, (CollisionVec3*)torque,
                             (CollisionVec3*)force);
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
