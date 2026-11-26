// Bike: motorcycle-specific layer over Vehicle (RTTI ".?AVBike@@", vtables
// 0x00550900 (primary, 103 slots), 0x005508cc @+540 (12 slots),
// 0x0055085c @+1848 (27 slots, virtual base)).
//
// Layout facts (tier 1 unless noted):
//  * Bike : Vehicle : SoultreePhysicsCharacter (SoultreePhysicsBaseObject at 0,
//    D3DIMSoultreeCharacter at object offset 540); GameObject/BaseObject is a
//    virtual base placed at +1848 in a complete Bike (vtordisp at +0x734) and
//    +1472 in a complete Vehicle, so the fields Bike adds occupy [0x5bc, 0x734).
//    (RTTI hierarchy + vtable_write_xrefs.)
//  * Slots 0..96 come from Vehicle (Bike overrides some); Bike introduces
//    97..102.  (vtable_overrides.json)
//
// The bases are the canonical classes (vehicle/Vehicle.h and the headers it
// includes); overrides use their signatures.  Every field named field_0xNN is
// tier 3 (offset is tier 1/2 from decoded instructions, the type and any role
// are guesses).
#ifndef MCM2_PHYSICS_BIKE_H
#define MCM2_PHYSICS_BIKE_H

#include "../../reconstructed/TrackGameGlobal.h"
#include "vehicle/Vehicle.h"
#include "core/DebugAlloc.h"


struct BikeWheel;
struct BikeWheelQ;
class Wrecker;
struct BikeA644;
class InlineShock;
class RotatingShock;


// 24-byte block the loader's sixth argument points to; copied into Bike+0x5c8 when present
// (tier 3: contents unknown, only the whole-block copy is decoded).
#include "../../reconstructed/TextureLoader.h"

// Objects reached through Bike fields.  Layout is only known where accessed.
// Frame object at BikeWheel+0x1c0 (provisional): only the vector at +0xc is read.
struct BikeWheelFrame {
    char pad_0x00[0xc];
    Vec3 axis;
};
// Objects at Bike+0x5f8 / +0x5fc (provisional, IntegrateSteerTorque): a point, an axis, an
// active flag and a state code that the method sets to 2.
struct BikeA5F8 {
    char pad_0x00[0x14];
    Vec3 point;               // +0x14
    char pad_0x20[0xc];
    Vec3 axis;                // +0x2c
    char pad_0x38[0x6c];
    int active;               // +0xa4
    char pad_0xa8[0xc];
    int state;                // +0xb4
};
struct BikeWheel {
    char pad_0x000[184];
    char contactPoint_0x0b8[20];   // registered in the owner's collisionPoints list; ~Bike removes it (0x00409a10)
    Vec3 wheelPosition;
    Vec3 groundPoint;  // +0xd8 VehicleWheel groundPoint (CollisionPoint+0x20 surfacePosition); KrustyBike 0x00413200 moves it with the body's step displacement
    Vec3 groundNormal;
    char pad_0x0f0[12];
    Vec3 appliedShare;
    Vec3 frictionDirection;  // +0x108 CollisionPoint+0x50 frictionDirection (VehicleWheel name)
    char pad_0x114[12];
    Vec3 normalLoadVector;                    // VehicleWheel +0x120 normalLoadVector; IntegrateSteerTorque crosses the wheel offset with it while in contact
    char pad_0x12c[24];
    float contactLoad;
    float tangentSpeed;  // +0x148 CollisionPoint+0x90 tangentSpeed (VehicleWheel name)
    float spinSpeed;  // +0x14c CollisionPoint+0x94 spinSpeed (VehicleWheel name); KrustyBike 0x00413200 clears it
    float penetration;  // +0x150 CollisionPoint+0x98 penetration (VehicleWheel name)
    char pad_0x154[4];
    float loadWeight;
    char pad_0x15c[16];
    int w_0x16c;              // IntegrateSteerTorque sets it to 2 while the wheel is in contact
    char pad_0x170[76];
    SoultreeObject* sceneNode;  // +0x1bc wheel scene node (VehicleWheel name)
    BikeWheelFrame* ownerRef;  // +0x1c0 MovingPart+0x44 ownerRef (Tire ctor arg c; sceneNode +0x1bc is MovingPart+0x40); IntegrateSteerTorque projects on its +0xc axis
    char pad_0x1c4[60];
    Vec3 nodePosition;        // +0x200 (VehicleWheel name)
    Vec3 wheelForwardAxis;  // +0x20c Tire.h wheelForwardAxis; scaled by wheelRadius while airborne (IntegrateSteerTorque)
    char pad_0x218[24];
    Vec3 rollDirection;  // +0x230 Tire.h/VehicleWheel rollDirection
    Vec3 sideAxis;  // +0x23c Tire.h/VehicleWheel sideAxis (wheel axle direction)
    Vec3 driveForce;                          // VehicleWheel +0x248 driveForce (DriveForce's force output); KrustyBike slots 48/86 zero or halve it
    char pad_0x254[12];
    int inContact;
    char pad_0x264[4];
    int shockEngaged;                         // VehicleWheel +0x268 shockEngaged; Bike slot 65 kicks the shock only while set
    char pad_0x26c[8];
    float wheelRadius;  // +0x274 Tire.h wheelRadius (ctor: y extent of the node)
    char pad_0x278[4];
    float slipSpeed;  // +0x27c Tire.h slipSpeed (written by UpdateContactPatch)
    float driveForceMagnitude;                // DriveForce's magnitude output (slot 86); Bike slot 61 and Tire's UpdateRoll scale by it
    char pad_0x284[4];
    float tangentSin;  // +0x288 Tire.h tangentSin
    char pad_0x28c[8];
    float restLoadWeight;                     // VehicleWheel +0x294 restLoadWeight; Bike's loader splits the weight by the wheel base
    char pad_0x298[4];
    float rampLevel;  // +0x29c Tire.h/VehicleWheel rampLevel
    char pad_0x2a0[4];
    float leanCos;  // +0x2a4 Tire.h leanCos
    BikeWheelQ* aux;  // +0x2a8 VehicleWheel aux (Tire.h TireAux)
    RotatingShock* secondaryAux;  // +0x2ac VehicleWheel secondaryAux (the fallback when primaryAux is null); the rear swingarm shock
    InlineShock* primaryAux;  // +0x2b0 VehicleWheel primaryAux (attachment AddWheel stores); the front fork shock
    char pad_0x2b4[4];
    float driveShare;  // +0x2b8 UpdateDriveShare (0x00513f90) stores the drive share here (fst [esi+0x2b8]; 1.0 on the other path)
    float driveShareComplement;  // +0x2bc UpdateDriveShare (0x00513f90) stores 1 - driveShare here (0 on the other path)
    void UpdateDriveShare(void* owner);   // 0x00513f90 (Tire.h name: drive share from the vehicle speed)
};

// Object at BikeWheel+0x2a8: the gearbox (engineState) for driven wheels (Bike's loader).
// Opaque provisional view; force calls use EngineGearbox directly.
struct BikeWheelQ {};


// Rate-limited smoother at Bike+0x644 (0x14 bytes; the loader builds it with
// `new(__FILE__, 0x7e0)` and an inline constructor). Bike+0x640 is a VehicleSmoother.
struct BikeA644 {
    BikeA644(float tc, float rise, float fall)
    {
        smoothTime = tc;
        smoothedValue = 0;
        maxRise = rise;
        maxFall = fall;
        smoothRatio = 1.0f;
    }
    int smoothedValue;
    float smoothTime;
    float smoothRatio;
    float maxRise;
    float maxFall;
};

// The scene node (d3d_field_0x1a0, poseNode, SoultreeObject::firstChild) is SoultreeObject
// (../common/SoultreeObject.h).

struct BikeGlobal_0056e26c { char pad_0x000[0x2f0]; float g_0x2f0; };

// Bike.cpp uses its Math3D header-local kVec3Zero, initialized at 0x00407890.

#include "BikeCore.h"

#endif
