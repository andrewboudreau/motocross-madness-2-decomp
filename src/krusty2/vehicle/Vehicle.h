// Vehicle (Vehicle.cpp), reconstructed for the retail MCM2 x86 build (VC6 SP3).
//
// EVIDENCE
//   Confirmed (tier 1): RTTI .?AVVehicle@@, direct base SoultreePhysicsCharacter,
//   vtable_records: primary vtable 0x00558a84 (object offset 0), secondary vtables
//   0x00558a50 (offset 540, Character/D3DIMSoultreeCharacter part) and 0x005589e0
//   (offset 1472, the virtual GameObject base, vbptr at +4), 97 primary slots.
//   Slot numbers below come from analysis/vtables.json; slots 0..42 are inherited
//   from SoultreePhysicsCharacter, slots 43..96 are introduced by Vehicle.
//   Member offsets are decoded from instruction displacements (tier 1 as offsets);
//   member NAMES and TYPES are tier 3 (provisional): field_0xNNN until proven.
//
// BASES: Vehicle derives from the canonical SoultreePhysicsCharacter
//   (hierarchy/SoultreePhysicsCharacter.h), so SoultreePhysicsBaseObject (0x000..0x21c),
//   D3DIMSoultreeCharacter (0x21c..0x42c), SoultreePhysicsCharacter's own fields
//   (0x42c..0x434) and the virtual GameObject base (0x5c0, vtordisp 0x5bc) are real C++
//   bases; hierarchy/LayoutProbe.cpp proves those offsets.  Vehicle's own data is
//   0x434..0x5bc.  Overrides of slots 0..39 use the reconciled SoultreePhysicsBaseObject
//   signatures (see the tier notes there); slots 43..96 are declared here and are the
//   canonical signatures for Bike and KrustyBike.
//   * Vectors use the shared Math3D Vec3.
#ifndef MCM2_PHYSICS_VEHICLE_H
#define MCM2_PHYSICS_VEHICLE_H

#include <stddef.h>
#include "soultree/SoultreePhysicsCharacter.h"
#include "contact/ContactImpulse.h"

#define VEH_CHECK_OFFSET(cls, member, off) \
    typedef char veh_assert_##member##_##off[(offsetof(cls, member) == (off)) ? 1 : -1]

struct VehBlock7 { float f[7]; };   // 28-byte state block (pose/orientation values, provisional)

// This TU's zero vector (0x0068a6e8) is Math3D.h's per-TU kVec3Zero: the `$E` body at
// 0x005278f0 writes it, and methods on both sides of that block read it.
// Legacy address views: 0x005778a8 is zero; 0x005778c8 is Y axis despite its
// provisional name below. Slot 36 now uses the ordinary header-local constants.
extern Vec3 g_VehZeroVec3_005778a8;
extern Vec3 g_VehZeroVec3_005778c8;

// 0x00460b50 FastSqrt / 0x00460c00 FastInvSqrt are declared in math/FastMath.h.

// ---- provisional collaborator types (layout only what the code touches) ----

// The scene node (d3d_field_0x1a0, field_0x218, SteeringControl::node) is
// SoultreeObject (../common/SoultreeObject.h).
// Free helpers (cdecl) used by the vehicle code. Retail addresses; names are tier 3.
// 0x005004a0 (slot 4's solver helper) is ContactSolveImpulse, declared in
// ../contact/ContactImpulse.h.
struct VehicleContact;
// 0x0043ad80 / 0x0043aa30 / 0x0043aff0: contact-array helpers used by slot 49 (provisional).
// Parameter names follow SoultreeRefreshContacts / SoultreeMergeContacts (SoulTreePhysics.cpp).
extern int  __cdecl VehContactsA(int enabled, int* activeCount, int count, VehicleContact** contacts,
                                 SoultreeProbe* terrain, Vec3* center, int mode, float radius);
extern void __cdecl VehContactsB(int count, VehicleContact** contacts, Vec3* scale, Vec3* angVel,
                                 Vec3* vel, Vec3* center, Vec3* pos, Vec3* outArm,
                                 Vec3* outNormal, Vec3* outVel);
extern void __cdecl VehContactsC(int n, VehicleContact** arr);
extern void __cdecl VehBasisToBlock(Vec3 a, Vec3 b, float* p0, float* p1, float* p2,
                                    float* p3, float* p4, float* p5, float* p6);

// Object at Vehicle+0x468: control/input map with two virtual queries.
struct VehicleValueSource {          // object at VehicleInputMap+0x0c (provisional)
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual int  UnknownVirtualSlot3(int a, float* out);
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();   // ~Vehicle (0x00526380) calls slots 15 and 16 on it
    virtual void UnknownVirtualSlot16();
};
struct VehicleKeyTable {             // object at VehicleInputMap+0x34 (provisional)
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual int  UnknownVirtualSlot5(int a, int b, int c);
};
struct VehicleInputMap {
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual int  UnknownVirtualSlot2(int a, int b);
    virtual int  UnknownVirtualSlot3(int a, int b, int c, int d);
    char pad_0x04[0x8];
    VehicleValueSource* valueSource;  // +0x0c float query used by slot 77
    char pad_0x10[0x24];
    VehicleKeyTable* keyTable;  // +0x34 slot 39 asks it to accept the code from slot 68
};

struct VehicleContact {              // elements of Vehicle+0x12c
    char pad_0x00[4];
    int  ownerNode;                    // +0x04 CollisionPoint::ownerNode (null for unowned points)
    char pad_0x08[0xC];
    Vec3 contactPosition;              // +0x14 contact position (distance source in slot 7)
    Vec3 impactPosition;              // +0x20 second contact point (impact position in slot 18)
    char pad_0x2C[0x18];
    Vec3 appliedShare;              // +0x44 per-contact share of the applied vector
    char pad_0x50[0x50];
    float blendWeight;                // +0xa0 blend weight
    int  contactActive;                 // +0xa4 contact active flag
    int  impactPosted;                 // +0xa8 impact-handled flag (slot 18)
    char pad_0xAC[0x10];
    unsigned char surfaceMaterial;        // +0xbc surface material id
};
// Object at Vehicle+0x1f0, viewed as the Scene (SceneManager.h): environment points at the
// "Environment" section, whose generateDust/generateDirt bytes are indexed by material id.
struct VehicleMaterialSet {
    char pad_0x00[0xA4];
    char* environment;                        // +0xA4  Scene +0xa4 environment (SceneManager.h UnknownSceneEnvironment, byte view): slot 18 tests generateDust (+0x400), slot 19 generateDirt (+0x408) per surface material
};
// Sink at VehicleImpactEvent+0x04 (provisional): fields 0x44/0x50/0x60 are touched by slot 18.
struct VehicleImpactSink {
    char pad_0x00[0x44];
    Vec3 currentVector;  // +0x44 slots 18-20 commit copies it to previousVector
    Vec3 previousVector;  // +0x50 latched copy made by VehCommitImpact
    char pad_0x5C[4];
    int updatePending;  // +0x60 set to 1 by VehCommitImpact
    char pad_0x64[0x10];
    Vec3 scrapeVector;  // +0x74 slots 19/20 write the clamped scrape vector
    void SetPosition(Vec3 pos, float intensity);   // 0x004b8d90, emitter position setter (samples/physics/effects/ParticleEmitters.h)
    void SetPosition(Vec3 pos);                    // 0x004b9dc0, the other emitter class's position setter (ParticleEmitters.h)
};
struct VehicleImpactEvent {          // argument of slots 18..20 (provisional)
    char pad_0x00[4];
    VehicleImpactSink* impactSink;  // +0x04 slot 18 target
    VehicleImpactSink* slideSink;  // +0x08 slot 19 target
    VehicleImpactSink* scrapeSink;  // +0x0c slot 20 target
    char pad_0x10[0x14];
    char resetTrail;                // +0x24 SoultreeAttachment::resetTrail: VehCommitImpact latches the vector, then clears it
};
// Object at Vehicle+0x480: the engine gearbox (ctor 0x004d2940, 0x1e4 bytes), the same object as
// src/reconstructed/GearRatios.h's UnknownEngineGearbox; slots 42/57 read throttleFraction.
// Keep this provisional field view; construction/throttle/shift calls use the
// canonical provider. The destructor is separately identified through the owning
// delete path; this does not establish complete equivalence of the two views.
#include "VehicleSpeedState.h"
// Exponential smoother objects at Vehicle+0x58c / +0x598 (provisional): f0 is the smoothed value.
// 0x14 bytes: LoadVehicle allocates two with `new(__FILE__, 0x19d/0x1a0)` and fills all five
// fields inline (no EH state, so the constructor is inline and calls nothing).
struct VehicleSmoother {
    VehicleSmoother(float tc)
    {
        smoothedValue = 0.0f;
        timeConstant = tc;
        minValue = 3.402823466e+38f;
        maxValue = -3.402823466e+38f;
        blendFactor = 1.0f;
    }
    // Bike's loader re-arms the two smoothers in place: the value is reset; a given time
    // constant also resets the blend factor (FLT_MAX keeps both; retail stores [+8] only on
    // that branch, 0x00409334).
    void Reset(float tc)
    {
        smoothedValue = 0.0f;
        if (tc != 3.402823466e+38f) {
            timeConstant = tc;
            blendFactor = 1.0f;
        }
    }
    float smoothedValue;  // +0x00 slot 49 VehSmooth: x += (target-x)*blend
    float timeConstant;                // +0x04 time constant / cap
    float blendFactor;                // +0x08 last blend factor
    float minValue;                   // +0x0c FLT_MAX at construction
    float maxValue;                   // +0x10 -FLT_MAX at construction
};
struct VehicleContactSet;            // object at SoultreePhysicsCharacter+0x128
// Vehicle+0x47c owns the SteeringControl allocated at 0x00504b60 (0x18 bytes).
// Keep the vehicle-side name as an alias; the five shared call targets and fields
// belong to the canonical declaration in motion/SteeringControl.h.
#include "motion/SteeringControl.h"
typedef SteeringControl VehicleSteerState;
struct VehicleAxisSource {           // object at VehicleAxis+0x00 (provisional)
    char pad_0x00[0xC];
    int kindCode;                  // +0x0c kind code (2 and 3 are tested by slot 63)
};
struct VehicleAxis {                 // objects at Vehicle+0x4f8/0x4fc/0x500 (provisional)
    VehicleAxisSource* axisSource;  // +0x00 VehicleAxisSource queried in slot 63
    char pad_0x04[0x20];
    float axisValue;                // +0x24 axis value (slot 63 reads it)
};
// Object at Vehicle+0x5ac: the SparkParticleEmitter (vtable 0x00555c1c, ctor 0x004b9830,
// 0x60c bytes; samples/physics/effects/ParticleEmitters.h).  LoadVehicle builds it and hands the
// result of its virtual slot 27 to GameObject::AppendChild; the impact handlers view
// it as VehicleImpactSink.  Provisional view; Vehicle and Bike slot 38 store (0, 12, 0) at +0x78 and
// set +0x60 after SetPosition, matching SparkParticleEmitter::launchVelocity / field_0x60.
struct VehicleSparkEmitter {
    explicit VehicleSparkEmitter(int flags);
    virtual void UnknownVirtualSlot0(); virtual void UnknownVirtualSlot1(); virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3(); virtual void UnknownVirtualSlot4(); virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6(); virtual void UnknownVirtualSlot7(); virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9(); virtual void UnknownVirtualSlot10(); virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12(); virtual void UnknownVirtualSlot13(); virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15(); virtual void UnknownVirtualSlot16(); virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18(); virtual void UnknownVirtualSlot19(); virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21(); virtual void UnknownVirtualSlot22(); virtual void UnknownVirtualSlot23();
    virtual void UnknownVirtualSlot24(); virtual void UnknownVirtualSlot25(); virtual void UnknownVirtualSlot26();
    virtual void* UnknownVirtualSlot27(void* a, GameObject* manager);
    void SetPosition(Vec3 p);        // 0x004b9dc0 (SparkParticleEmitter::SetPosition)
    char pad_0x04[0x5c];
    int burstPending;  // +0x60  SparkParticleEmitter +0x60 burstPending; Bike slot 38 and the Vehicle impact handler set it to 1 after SetPosition
    char pad_0x64[0x14];
    Vec3 launchVelocity;             // +0x78 SparkParticleEmitter::launchVelocity; Bike slot 38 stores (0, 12, 0)
    char pad_0x84[0x588];            // 0x60c bytes (LoadVehicle's allocation)
};
// The track object (SoultreeSlot1f0) as LoadVehicle reads it: only the byte at +0x88, forwarded
// to the base loader's a20.
struct VehicleTrackView {
    char pad_0x00[0x88];
    unsigned char field_0x88;
};
struct VehicleTicker {               // elements of Vehicle+0x554/+0x560 (provisional): only virtual slot 0 is called
    virtual void UnknownVirtualSlot0();
};
class SoultreeContact;
// 0x0043a570: appends a contact to the owner's contact array.  The same function as
// AddExistingCollisionPoint, whose third argument is the float penetration threshold;
// this view passes its bits as an int.
extern void __cdecl VehAddContact(int capacity, SoultreeContact** arr, int threshold, int* count,
                                  void* contact);
class Vehicle;
// Suspension objects hung on a wheel (VehicleWheel+0x2b0 / +0x2ac).  PlaceWheels retracts
// them by a fraction of the last step and clears their forces while the wheel is in contact;
// the two Retract bodies differ (0x004fa310 / 0x004fab60), the ClearForces body 0x004f9c70 is
// shared and called directly.  Provisional views; samples/physics/suspension/Suspension.h
// reads them as InlineShock / RotatingShock.
struct VehicleWheel;
struct VehicleShock {
    char pad_0x00[0x44];
    SoultreeObject* node;            // +0x44 the shock's scene node (Suspension.h: found below the root by name)
    char pad_0x48[0x10];
    Vec3 spring;                     // +0x58 spring force (Suspension.h name); Method_00529450 projects it on the ground normal
    char pad_0x64[0x10];
    Vec3 displacement;               // +0x74 Suspension.h name; scaled by impulseScale and projected on the Y axis
    char pad_0x80[0x14];
    float impulseScale;              // +0x94 one-shot scale of the displacement impulse: Bike slot 65 sets it on a fast pitch-input rise; Method_00529450 adds the Y part of displacement * it (times leanCos) and zeroes it
    void ClearForces();                                         // 0x004f9c70
    // 0x004f9d30 (ret 0x10): Suspension.h's Shock::ApplyDamping; Method_005293E0 passes the
    // wheel's load weight, Vehicle+0x4dc suspensionMass, the last step time and its speed output.
    void ApplyDamping(float weight, float a, float dt, float* speed);
};
struct VehicleInlineShock : VehicleShock {
    // 0x004f9ee0 (ret 0x24): Bike's loader builds it with the shock name, the model node, the
    // handlebar node and six floats (0xd8 bytes, `new(__FILE__, 0x696)`); parameter roles from the
    // stores in Suspension.h's InlineShock constructor.
    VehicleInlineShock(const char* name, SoultreeObject* root, SoultreeObject* anchor, float minLength,
                       float stiffnessMin, float stiffnessMax, float dampingRatio, float speedThreshold,
                       float retractScale);
    void Retract(float amount, const VehicleWheel* carrier);   // 0x004fa310
    void UpdateAxis(SoultreeObject* wheelNode, SoultreeObject* mountNode); // 0x004f9f90 (ret 8)
    char pad_0x98[0x40];             // 0xd8 bytes (the loader's allocation)
};
struct VehicleRotatingShock : VehicleShock {
    // 0x004fa700 (ret 0x2c): name, model node, seven floats, the wheel kind and an optional axis
    // (0xe4 bytes, `new(__FILE__, 0x683)`); parameter roles from the stores in Suspension.h's
    // RotatingShock constructor.
    VehicleRotatingShock(const char* name, SoultreeObject* root, float swingAngle, float armLength,
                         float stiffnessMin, float stiffnessMax, float dampingRatio, float speedThreshold,
                         float retractScale, int axisId, const Vec3* axisOverride);
    void Retract(float amount, const VehicleWheel* carrier);   // 0x004fab60
    char pad_0x98[0x28];
    float swingAngle;                // +0xc0 Bike's loader: sin(swingAngle * 0.5) * 2 * armLength -> chord
    float chord;  // +0xC4  = sin(swingAngle * 0.5) * 2 * armLength (Bike's loader); Suspension.h RotatingShock +0xc4 chord
    float armLength;                 // +0xc8 Bike's loader: distance from the wheel node to the shock node
    float armLengthSq;               // +0xcc armLength squared
    char pad_0xd0[0x14];             // 0xe4 bytes (the loader's allocation)
};
// Object at VehicleWheel+0x2a8: Bike's loader passes engineState (the gearbox) for driven wheels.
struct VehicleWheelAux {             // object at VehicleWheel+0x2a8 (provisional view)
    char pad_0x00[0x8C];
    int field_0x8c;
    int field_0x90;
};
// The wheel's collision point (VehicleWheel+0xb8, the CollisionPoint base of Tire): only its
// virtual slot 1 (friction update) is called here, by Method_00527A20.  Provisional view.
struct VehicleContactPoint {
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
};
// Elements of Vehicle+0x53c (provisional: only touched offsets are named).
struct VehicleWheel {
    // 0x00512f10 (ret 0x3c; samples/physics/tire/Tire.h Tire::Tire): wheel name, model node,
    // twelve tuning values, the steer state of a steered wheel (else 0) and the track
    // (0x2c0 bytes, `new(__FILE__, 0x6b8)` in Bike's loader).
    VehicleWheel(const char* name, SoultreeObject* root, float rollFriction_, float sideFriction_,
                 int field_0x298_, VehicleSteerState* steer, float field_0x2b4_,
                 float rollFrictionScale_, float sideFrictionScale_, float field_0x2a0_,
                 float field_0x1d8_, float field_0x1dc_, float field_0x1e0_, float blendWeight_,
                 SoultreeSlot1f0* track);
    char pad_0x00[0xB8];
    VehicleContactPoint contactPoint;  // +0xb8 AddWheel registers it in collisionPoints
    char pad_0xBC[0x10];
    Vec3 wheelPosition;              // +0xcc wheel contact position (slot 7 distance source)
    Vec3 groundPoint;              // +0xd8 (y at +0xdc is read as a height by slot 58)
    Vec3 groundNormal;              // +0xe4 contact normal (averaged by GetAverageGroundNormal)
    Vec3 relativePosition;  // +0xF0 CollisionPoint+0x38 relativePosition (base at +0xb8); crossed with the friction force for the torque
    Vec3 appliedShare;              // +0xfc per-wheel share of the applied vector
    Vec3 frictionDirection;  // +0x108 CollisionPoint+0x50 frictionDirection (base at +0xb8)
    Vec3 field_0x114;
    Vec3 normalLoadVector;                    // +0x120  CollisionPoint+0x68 (base at +0xb8); Method_00527A20: = groundNormal * normalLoad, zero when appliedShare points away from the ground
    float normalLoad;                         // +0x12C  CollisionPoint+0x74 (named frictionCoefficient in the base); = -(appliedShare . groundNormal) when negative, else 0 (Method_00527A20, before the contact point's slot 1)
    Vec3 frictionForce;  // +0x130 CollisionPoint+0x78 frictionForce (base at +0xb8)
    float frictionMagnitude;                  // +0x13C  CollisionPoint+0x84 frictionMagnitude (base at +0xb8); Method_00527A20 limits the friction impulse by it
    char pad_0x140[4];
    float contactLoad;  // +0x144 slot 72 accumulates it over wheels and averages by the contact count
    float tangentSpeed;  // +0x148 CollisionPoint+0x90 tangentSpeed (base at +0xb8); Method_00527A20 adds it to spinSpeed for the available load
    float spinSpeed;  // +0x14C CollisionPoint+0x94 spinSpeed (base at +0xb8)
    float penetration;  // +0x150 CollisionPoint+0x98 penetration (base at +0xb8); reset to -999
    char pad_0x154[4];
    float loadWeight;               // +0x158 blend weight
    int pointInContact;  // +0x15C CollisionPoint+0xa4 inContact (base at +0xb8); Tire's own inContact is +0x260
    int impactPosted;                 // +0x160 impact-handled flag (slot 18)
    int slidePosted;                 // +0x164 impact-handled flag (slot 19)
    int scrapePosted;                 // +0x168 impact-handled flag (slot 20)
    int field_0x16c;
    char pad_0x170[4];
    unsigned char surfaceMaterial;       // +0x174 surface material id
    char pad_0x175[0x47];
    SoultreeObject* sceneNode;     // +0x1bc wheel scene node (slot 33 reads its position)
    int ownerRef;  // +0x1C0 MovingPart+0x44 ownerRef (Tire ctor arg c; sceneNode +0x1bc is MovingPart+0x40)
    char pad_0x1C4[0x18];
    float levelRiseRate;               // +0x1dc +0x29c ramp-up rate (UpdateWheelRampLevels)
    float levelFallRate;               // +0x1e0 +0x29c ramp-down rate
    char pad_0x1E4[4];
    Vec3 slipVector;                 // +0x1e8 Tire.h slipVector (UpdateContactPatch)
    char pad_0x1F4[0xC];
    Vec3 nodePosition;                // +0x200 wheel node position (written by slot 33)
    char pad_0x20C[0x1C];
    float field_0x228;
    char pad_0x22C[4];
    Vec3 rollDirection;              // +0x230 Tire.h: normalise(cross(normal, side))
    Vec3 sideAxis;                   // +0x23c Tire.h: wheel axle direction
    Vec3 driveForce;                          // +0x248  DriveForce's force output (slot 86); AccumulateWheelContacts adds it and its moment; ResetWheelContacts clears it
    Vec3 field_0x254;
    int inContact;  // +0x260 selects wheels touching the ground (slots 7, 54, 72, 75-86); count equals Vehicle::wheelsInContact
    int reportContactOutputs;  // +0x264 (Tire.h name) PlaceWheels sets it on the first in-contact wheel without field_0x1c0 (else on wheel 0)
    int shockEngaged;                         // +0x268  Tire 0x00513c70 sets it after a shock solve; Method_005293E0 damps and Method_00529450 applies the spring only while set
    int shockExtending;                       // +0x26C  Tire 0x00513c70 copies the shock's extending flag here (Tire.h); ResetWheelContacts clears it
    char pad_0x270[8];
    int rollAngle;  // +0x278 Tire.h rollAngle (float there; only cleared here)
    int slipSpeed;  // +0x27C Tire.h slipSpeed (float there; only cleared here)
    float driveMagnitude;          // +0x280 DriveForce's magnitude output (slot 86; Tire.h: drive factor)
    float surfaceScaleB;           // +0x284 Tire.h surfaceScaleB
    float tangentSin;              // +0x288 Tire.h: sqrt(1 - tangentCos^2); slots 19/20 scale the slide vector by it
    float tangentCos;              // +0x28c Tire.h: |tangent . rollDirection| clamped to 1
    float driveDistance;                      // +0x290  Tire 0x005143d0: driveShareComplement * drive magnitude * dt * scale; slots 19/20 turn it into a speed with invStepTime
    float restLoadWeight;                     // +0x294  Tire ctor argument a14; Bike's loader stores the static weight split (rear = share / wheelBase, front = 1 - rear); loadWeight is reset to it
    char pad_0x298[0x4];
    float rampLevel;               // +0x29c 0 selects a flag passed to slot 77 (slot 83)
    float driveStrength;           // +0x2a0 Tire.h ctor arg a10, read by 0x00514170; Method_00527A20 counts the in-contact wheels where it is nonzero
    float field_0x2a4;
    VehicleWheelAux* aux;          // +0x2a8 Tire.h TireAux
    int secondaryAux;  // +0x2ac same use as primaryAux, the fallback when it is null (UpdateWheelsInContact, Method_005293E0)
    int primaryAux;  // +0x2b0 AddWheel stores an attachment here; UpdateWheelsInContact treats it as an object whose +0x8c is reset from +0x90
    char pad_0x2B4[0x4];
    float driveShare;  // +0x2B8 UpdateDriveShare (0x00513f90) stores the drive share here (fst [esi+0x2b8]; 1.0 on the other path)
    float driveShareComplement;  // +0x2BC UpdateDriveShare (0x00513f90) stores 1 - driveShare here (0 on the other path)
    // 0x005143d0
    void UpdateRoll(float dt, float distanceScale, int locked, int driven, int crashState, float driveScale);
    void UpdateContactPatch(Vec3* origin, Vec3 position, Vec3 velocity, float speed, float* outA, int* outB);   // 0x005135f0 (Tire.h name)
    void UpdateShock(float bias, int flag, int crashState, float speed, Vec3* velocity, Vec3* anchor);   // 0x00513c70 (Tire.h UpdateShock, tier 3 name; sets shockEngaged/shockExtending)
    void UpdateDriveShare(Vehicle* owner);       // 0x00513f90, Tire.h: drive share +0x2b8 from the vehicle speed
    void UpdateAttachment();                    // 0x00515660, called per wheel by Bike slot 41 (0x0040cbd0)
    // 0x00514170 (ret 0x1c): per-wheel drive/brake step called by Method_00527A20 with the
    // reciprocal of the driven-wheel count, the body's step time, direction flag and mass.
    void ApplyDrive(float share, float stepTime, int movingForward, float bodyMass,
                         float* speed, Vec3* force, Vec3* torque);
    // 0x00514550 (ret 0x20): places the wheel on the ground (samples/physics/tire/Tire.h
    // UpdateSuspensionProbe); PlaceWheels passes the body's probe, position, saved
    // pitch/sin-roll/yaw, saved forward/up and the pose node.
    void UpdateSuspensionProbe(SoultreeProbe* world, const Vec3* position, float pitch, float sinRoll,
                         float yaw, const Vec3* forward, const Vec3* up, SoultreeObject* node);
};

#include "VehicleCore.h"

// ---- helpers shared by Vehicle.cpp and the out-of-bracket Vehicle inlines (samples) ----

// Cross product as a member of a Vec3 view: VC6 honours source multiplicand order here
// (free-function form canonicalises it).
struct VehV3 : Vec3
{
    Vec3 Cross(const Vec3& b) const
    {
        Vec3 r;
        r.x = y * b.z - z * b.y;
        r.y = z * b.x - x * b.z;
        r.z = x * b.y - b.x * y;
        return r;
    }
    // The plain formula with each first product parenthesised: the parentheses put each
    // fsubp between the load and the store of the previous component's copy, as retail does
    // in the slot 34/35 inlines (docs/VC6_OPERAND_ORDER.md section 3).
    Vec3 CrossB(const Vec3& b) const
    {
        Vec3 r;
        r.x = (y * b.z) - z * b.y;
        r.y = (z * b.x) - x * b.z;
        r.z = (x * b.y) - y * b.x;
        return r;
    }
};

// Scene-node method 0x004444e0 (purpose unknown) is not in SoultreeObject yet; view the
// node through a local stand-in that declares it.
struct VehSceneNodeView {
    void SetAxesPtr(const Vec3* axisZ, const Vec3* axisY, int orthogonalize, int keepZ); // 0x004fbd70
};

#endif
