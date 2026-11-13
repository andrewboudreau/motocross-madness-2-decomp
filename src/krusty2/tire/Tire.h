// Tire -- reconstructed class layout (Tire.cpp).
//
// Layout evidence (tier 1 unless noted), analysis/rtti_classes.json + vtables.json:
//  * RTTI .?AVTire@@ (COL 0x0055f5e0), direct bases in declaration order
//    CollisionObject, MovingPart, CollisionPoint (attributes=1: multiple inheritance).
//    All three bases are NON-virtual (pdisp -1).  Base class array offsets (mdisp):
//    CollisionObject/QuadTreeObject 0, GraphicsTest/GameObject/BaseObject 12,
//    CollisionPoint 184 (0xb8), MovingPart 380 (0x17c).
//    VC6 lays the polymorphic CollisionPoint out before the non-polymorphic MovingPart
//    even though MovingPart is declared first (MovingPart has no vtable record).
//  * vtables (object_offset): 0x00558578 @0 (2 slots, QuadTreeObject shape: 0x004dc610,
//    0x00434ce0 both inherited), 0x00558508 @12 (27 slots, GameObject shape; overrides
//    vs CollisionObject: slot 0 = dtor 0x005133a0, slot 8 = 0x00513490),
//    0x005584fc @184 (2 slots: slot 0 = adjustor thunk 0x00515730 `sub ecx,0xac; jmp
//    0x005133a0` -- the deleting dtor seen from the CollisionPoint subobject; slot 1 =
//    0x00515b50 overrides CollisionPoint slot 1 without any thunk).
//  * ctor 0x00512f10 (`ret 0x3c`, 15 argument dwords), size 0x2c0: the caller 0x004084d7
//    allocates `operator new(0x2c0, __FILE__, 0x6b8)`.
//  * dtor core 0x005133d0, deleting wrapper 0x005133a0 (virtual ~Tire).
//
// MovingPart is a non-polymorphic 0x4c-byte class (ctor 0x004a23a0 initialises 0x40..0x48
// and has ret 0xc, the dtor is the shared empty function 0x00464e90).  Its real size is
// only known to be >= 0x4c (tier 2: the next Tire member 0x1c8 is written by the Tire
// ctor).  PROVISIONAL area-local declaration: MovingPart has no RTTI virtuals and no
// reconstruction elsewhere yet.
//
// Member names: field_0xNN are Tire-object-relative for Tire's own members.  Names of
// inherited members are qualified where two bases share the number (CollisionObject and
// CollisionPoint both have field_0x88 / field_0x8c).  Semantic names are tier 3.
#ifndef TIRE_H
#define TIRE_H

#include <stddef.h>
#include "collision/CollisionObject.h"

#include "collision/CollisionPoint.h"

// Provisional (see above); src/reconstructed/MovingPart.cpp is the matched constructor.
// Constructor args (name, root, ownerRef): root->FindByName(name) (0x004fdae0) gives the
// node, ownerRef is stored at +0x44.
class TireNode;

class MovingPart {
public:
    MovingPart(void* name, int root, int ownerRef); // 0x004a23a0 (ret 0xc)
    ~MovingPart();                          // 0x00464e90, an empty out-of-line dtor

    char field_0x00[0x40];
    TireNode* sceneNode;                   // +0x40 root->FindByName(name) (0x004fdae0)
    int ownerRef;                         // +0x44 ctor arg ownerRef
    int field_0x48;                         // 0
};

// Provisional stand-ins for the two heap objects the dtor releases (rotatingShock/inlineShock).
// Their dtors are 0x004fa8a0 and 0x004f9f80 (both in the scene-graph range); tier 3.
// 0x00515660 reads both: +0x44 is a scene node and +0x98 a joint value. A keeps a
// direction at +0xc0 and drives the node's position along it; B keeps a rest matrix at
// +0x04, a rotation axis at +0xd4 and divides +0x98 by +0xc8 for the angle.
class TireNode;
// 0x00513c70 (UpdateShock) reads +0x54 (an "extending" flag copied to shockExtending),
// +0x70 (a ratio tested against zero) and adds the vector at +0x80 to the contact's
// world position after the shock solve: InlineShock::SolveContact 0x004fa400 and
// RotatingShock::SolveContact 0x004fac60 (src/reconstructed/Suspension.h).
// The solves are declared as in Suspension.h; the caller's dt and velocity slots are
// dead after the call and hold its `active` and `load` outputs.
#include "../../reconstructed/TireAttachmentViews.h"

// Provisional area-local stand-ins for two scene-side classes only known by their calls.
class TireNode {                                       // scene node, MovingPart::field_0x40
public:
    void GetPositionIn(TireNode* parent, CollisionVec3* out);                        // world position
    void GetLocalBounds(CollisionVec3* a, CollisionVec3* b);                          // extents, b.y = wheel radius
    CollisionVec3* LocalToWorldDirection(CollisionVec3* out, const CollisionVec3* in);       // rotate a direction
    void RotateAbout(float x, float y, float z, float angle);                      // 0x004fcce0
    void SetPosition(CollisionVec3 position);                                      // 0x004fc630
    void SetLocalMatrix(const CollisionMatrix4* m);                                // 0x004fca30
    void Rotate(CollisionVec3 axis, float angle);                                  // 0x004fceb0
};
class TireWorld {                                      // world/terrain query object (arg 1 of 0x00514550)
public:
    int QueryGround(CollisionVec3* pos, CollisionVec3* outNormal, int flags, unsigned char* outSurface);
};

// The vehicle passed to 0x00513f90 (Vehicle.cpp 0x00525d.. passes `this`): only the
// members read there are declared (provisional view of Vehicle).
class Tire;
class TireVehicle {
public:
    virtual void UpdateMass();
    virtual void ResetDynamicState();
    virtual void InitializePhysics();
    virtual void ApplyStaticContactImpulse();
    virtual void ApplyBodyContactImpulse();
    virtual void ShouldRefreshContacts();
    virtual void ApplyDrag();
    virtual void DistributeContactForce();
    virtual void UpdateLocalCenterOfMass();
    virtual void ComputeFrameSteps();
    virtual void IsAtBaseWeight();
    virtual void FindPlacement();
    virtual void ClassifyOutOfBounds();
    virtual void AccumulateContactForces();
    virtual void IntegrateMotion();
    virtual void ApplyWorldTorque();
    virtual void LocalTorqueToAngularAcceleration();
    virtual void AverageAttachmentPosition();
    virtual void UpdateContactEffect();
    virtual void UpdateSlideEffect();
    virtual void UpdateScrapeEffect();
    virtual void UpdateAttachmentEffects();
    virtual void ShouldUpdateShadowLight();
    virtual void ShouldEmitMovingEffects();
    virtual void ShouldRestartSteamEffect();
    virtual void ShouldBurstSteamEffect();
    virtual void UnknownVirtualSlot26();
    virtual void StartWreck();
    virtual void ResolveSubstepCollisions();
    virtual void SaveBodyOrientation();
    virtual void ClearContactEffectFlags();
    virtual void UpdateContactStates();
    virtual void EffectiveLoadWeight();
    virtual void ResetToPlacement();
    virtual void LoadBodyAxes();
    virtual void StoreBodyAxes();
    virtual void ResetBodyOrientation();
    virtual void AddAttachment();
    virtual void ApplyCollisionEvent();
    virtual void ApplyPendingRespawn();
    virtual void LoadPhysicsModel();
    virtual void EndMotion();
    virtual void CheckMotionComplete();
    virtual void SnapshotRespawnPose();
    virtual void RemoveFromRace();
    virtual void GetSpawnProtectionDuration();
    virtual void ComputeRotationPivot();
    virtual float GetDriveShareScale(Tire* wheel);   // grip factor for this wheel

    char pad_0x04[0x24 - 0x04];
    float invMass;                         // +0x24 SoultreePhysicsBaseObject invMass (1 / bodyMass)
    char pad_0x28[0x60 - 0x28];
    float savedCosPitch;                   // +0x60 SoultreePhysicsBaseObject savedCosPitch
    char pad_0x64[0x70 - 0x64];
    CollisionVec3 acceleration;            // +0x70 SoultreePhysicsBaseObject acceleration; its length drives the factor
    char pad_0x7c[0xa4 - 0x7c];
    float savedForwardY;                   // +0xa4 y of SoultreePhysicsBaseObject savedForward (+0xa0)
    char pad_0xa8[0x444 - 0xa8];
    int crashState;                        // +0x444 Vehicle crashState
};

// Object at Tire+0x2a8 (Vehicle.h's VehicleWheelAux): the engine gearbox; only its first float is read here.
struct TireAux {
    float throttleFraction;  // +0x00 EngineGearbox throttleFraction (GearRatios.h)
};

class Tire : public CollisionObject, public MovingPart, public CollisionPoint {
public:
    // 0x00512f10, ret 0x3c: 15 dwords.  Argument roles come from the stores in the body.
    // Types follow the destination fields (tier 3): name, root and steer go to MovingPart,
    // sideFriction_ also to CollisionPoint::normalForce (+0x88), track to
    // CollisionPoint::surfaceOwner (+0xc0).
    Tire(void* name, int root, float rollFriction_, float sideFriction_, int field_0x298_,
         int steer, float slipThreshold_, float rollFrictionScale_, float sideFrictionScale_,
         float driveStrength_, float driveTorqueScale_, float levelRiseRate_, float levelFallRate_,
         float blendWeight_, int track);
    virtual ~Tire();                            // slot 0 @12 (0x005133a0), core 0x005133d0
    virtual GameObject* SetRenderContext(void* context);        // 0x00513490
    virtual void CollisionPointVirtualSlot1();  // CollisionPoint slot 1 @184, 0x00515b50

    // Non-virtual members (this == complete object).
    void HandleContact(int unused, int tag, CollisionObject* other);   // 0x00512e80 (ret 0xc)
    // 0x00514550 (thiscall, ret 0x20), tier 3 name: places the wheel on the ground.  Recomputes
    // the wheel axes (wheelForwardAxis/0x230/0x23c), probes the world/terrain for the contact point
    // and normal, builds the wheel frame and reports the contact through CollisionPoint's
    // penetration.  Callers 0x00528f65 and 0x00528fd8 pass body +0x0c, +0x4c, +0x54, +0x50, +0xa0, +0xac, +0x42c.
    void UpdateSuspensionProbe(TireWorld* world, const CollisionVec3* position, float pitch,
                               float sinRoll, float yaw, const CollisionVec3* forward,
                               const CollisionVec3* up, TireNode* node);
    // 0x00513560: sets the roll angle from a rolled distance (times 1 / radius) and
    // turns the wheel node to it about x; 0x005135b0 re-applies the stored angle.
    void SetRollDistance(float distance);
    void ApplyRollAngle();
    // 0x00515660: poses the attached part (inlineShock first, else rotatingShock) from its value.
    void UpdateAttachment();
    // 0x00513f90 (ret 4): sets driveShare (and driveShareComplement = 1 - it) from the
    // vehicle's speed while the wheel slips less than slipThreshold; scales driveForce by it.
    void UpdateDriveShare(TireVehicle* vehicle);
    // 0x00514170 (ret 0x1c): pushes along the roll direction with the drive strength,
    // limited by what the body can take this step; the push goes to *force and its
    // moment about the contact (times driveTorqueScale) to *torque.
    void ApplyDrive(float share, float stepTime, int forward, float mass, float* speed,
                    CollisionVec3* torque, CollisionVec3* force);
    // 0x005143d0 (ret 0x18): advances the wheel's roll angle for a step; crashState (what
    // Vehicle passes) is unused.
    void UpdateRoll(float dt, float distanceScale, int locked, int driven, int crashState,
                    float driveScale);
    // 0x00513c70 (ret 0x18), tier 3 name: runs the attached shock (inlineShock, else rotatingShock)
    // against the contact while the wheel touches; the solve's load and active flag
    // become the contact's penetration and inContact. Vehicle.cpp 0x00529ad4 is the
    // only caller (dt = +0x1e8, pointsTouching = byte +0x1d0, crashed = +0x444, offset = +0xbc,
    // velocity = &+0x64, axisB = &+0xa0); pointsTouching and offset are unused.
    void UpdateShock(float dt, int pointsTouching, int crashed, const CollisionVec3* offset,
                     const CollisionVec3* velocity, const CollisionVec3* axisB);

    // 0x00515c90 (thiscall, three vector pointers and a hidden result pointer first).
    CollisionVec3* NormalizedCross(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b,
                               const CollisionVec3* c);
    // 0x005135f0 (thiscall, ret 0x28), tier 3 name: contact-patch update.  Recomputes the
    // relative position/lever vectors and the tangent direction, length and slip-like scalar
    // slipSpeed; see Tire.cpp.  Single caller 0x00529aa7 (centerOfMass, worldAngularVelocity, velocity, linearSpeed).
    void UpdateContactPatch(const CollisionVec3* center, CollisionVec3 angularVelocity, CollisionVec3 velocity,
                            float speed, float* outSpeed, int* outForward);

    // --- Tire's own data 0x1c8..0x2c0 (MovingPart ends at 0x1c8) ---
    float invWheelRadius;  // +0x1c8 ctor: 1.0f / extentB.y (wheelRadius is the radius)
    float sideFriction;  // +0x1cc ctor arg a4; slot 1 uses it times sideFrictionScale times sin of the tangent angle (sideways grip)
    float rollFrictionScale;  // +0x1d0 ctor arg a8; multiplies rollFriction in the cos term of the slot 1 friction
    float sideFrictionScale;  // +0x1d4 ctor arg a9; multiplies sideFriction in the sin term
    float driveTorqueScale;  // +0x1d8 ctor arg a11; ApplyDrive 0x00514170 adds cross(relativePosition, push) * it to the torque
    float levelRiseRate;  // +0x1dc ctor arg a12; Vehicle 0x00529280: rampLevel += stepTime * it, clamped
    float levelFallRate;  // +0x1e0 ctor arg a13; Vehicle 0x00529280: rampLevel -= stepTime * it, clamped
    int hasContactObjectVelocity;  // +0x1e4 HandleContact: set 1 when a vector was captured, 0 for tag 0x65/other
    CollisionVec3 slipVector;  // +0x1e8 UpdateContactPatch: tangent * tangentSpeed + normalLeverCross, normalised into CollisionPoint::frictionDirection
    CollisionVec3 contactObjectVelocity;  // +0x1f4 HandleContact: copy of other owner vector at +0x224 (CollisionCharacter::wheelVelocity is its velocity), +0x40 or +0x64
    CollisionVec3 wheelCenter;  // +0x200 UpdateSuspensionProbe: world position of the scene node (0x004fc9a0); probes are offset from it by the radius
    CollisionVec3 wheelForwardAxis;  // +0x20c UpdateSuspensionProbe: node-rotated kVec3ZAxis, else the caller's savedForward; rollDirection fallback and frame axis passed to TireBuildFrame
    CollisionVec3 normalLeverCross;  // +0x218 UpdateContactPatch: cross(n*(axis.n), relativePos) with n = contact normal; fallback tangent source when the projected dot <= 0.001; ctor zero vector
    CollisionVec3 bodyPosition;  // +0x224 UpdateSuspensionProbe: copy of its position argument (Vehicle passes &position, body +0x0c) at the end; ctor zero vector
    CollisionVec3 rollDirection;  // +0x230 normalise(cross(normal, side)); slot 1 and UpdateContactPatch dot it with the tangent
    CollisionVec3 sideAxis;  // +0x23c cross(up, node axis) normalised; the wheel axle direction used for the probes
    CollisionVec3 driveForce;  // +0x248 force output of the engine's DriveForce (Vehicle); UpdateDriveShare scales it by driveShare
    CollisionVec3 field_0x254;
    int inContact;  // +0x260 UpdateSuspensionProbe: penetration >= CollisionPoint::penetrationThreshold ? 1 : 0
    int reportContactOutputs;  // +0x264 UpdateContactPatch writes *outValue and *outSign only when it is nonzero; ctor 0 (tier 3: meaning of the flag)
    int shockEngaged;  // +0x268 UpdateShock sets it after a shock solve; UpdateSuspensionProbe clears it
    int shockExtending;  // +0x26c UpdateShock copies the shock's extending flag (+0x54) here
    float rollFriction;  // +0x270 ctor arg a3; cos term factor in slot 1
    float wheelRadius;  // +0x274 ctor: y extent of the node; slot 8 builds the 8-point rim polyline with it; probes offset by it
    float rollAngle;  // +0x278 0x00513560: distance * invWheelRadius; the wheel node's x rotation
    float slipSpeed;                      // +0x27c written by UpdateContactPatch (float, tier 2)
    float driveMagnitude;  // +0x280 magnitude output of the engine's DriveForce; UpdateDriveShare: driveMagnitude * invMass - |acceleration|
    float surfaceScaleB;                      // +0x284 1.0f default or a surface table entry (tier 2)
    float tangentSin;  // +0x288 slot 1: sqrt(1 - cos^2)
    float tangentCos;  // +0x28c slot 1: |tangent . rollDirection| clamped to 1
    float driveDistance;  // +0x290 UpdateRoll: driveShareComplement * driveMagnitude * dt * driveScale
    float restLoadWeight;  // +0x294 ctor arg a14 (also CollisionPoint::blendWeight)
    int field_0x298;
    float rampLevel;  // +0x29c 0x005143d0 unwinds the roll angle by (1 - rampLevel) while set
    float driveStrength;  // +0x2a0 ctor arg a10; ApplyDrive 0x00514170 scales the push by it
    float field_0x2a4;
    TireAux* engine;  // +0x2a8 Vehicle calls EngineGearbox::DriveForce on it, Bike's loader stores engineState; UpdateRoll scales the roll angle by its throttle in (0, 1)
    TireAttachB* rotatingShock;             // +0x2ac RotatingShock (SolveContact 0x004fac60); released by the dtor when inlineShock is null
    TireAttachA* inlineShock;               // +0x2b0 InlineShock (SolveContact 0x004fa400); released first (dtor 0x004f9f80)
    float slipThreshold;  // +0x2b4 ctor arg a7; UpdateDriveShare 0x00513f90 compares slipSpeed with it
    float driveShare;  // +0x2b8 UpdateDriveShare 0x00513f90: drive share (1.0 otherwise)
    float driveShareComplement;  // +0x2bc UpdateDriveShare: 1 - driveShare; UpdateRoll scales driveDistance by it
};

// Base offsets (CollisionObject @0, CollisionPoint @0xb8 = 184, MovingPart @0x17c) are proven by
// the dtor core 0x005133d0 / deleting dtor 0x005133a0 and the vtable record Tire+184 (tier 1/2).
// The polymorphic CollisionPoint is placed before the non-polymorphic MovingPart by the compiler
// (so the declaration order of the two bases above does not matter for the layout).
typedef char tire_assert_first_field[(offsetof(Tire, invWheelRadius) == 0x1c8) ? 1 : -1];
typedef char tire_assert_last_field[(offsetof(Tire, driveShareComplement) == 0x2bc) ? 1 : -1];
typedef char tire_assert_sizeof[(sizeof(Tire) == 0x2c0) ? 1 : -1];
typedef char tire_assert_collision_point[(sizeof(CollisionPoint) == 0xc4) ? 1 : -1];

#endif
