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
// Zero vectors in the retail data section at 0x005778a8 / 0x005778c8 (read-only copies).
extern Vec3 g_VehZeroVec3_005778a8;
extern Vec3 g_VehZeroVec3_005778c8;

// 0x00460b50 FastSqrt / 0x00460c00 FastInvSqrt are declared in math/FastMath.h.

// ---- provisional collaborator types (layout only what the code touches) ----

// The scene node (d3d_field_0x1a0, field_0x218, VehicleSteerState::field_0x00) is
// SoultreeObject (../common/SoultreeObject.h).
// Free helpers (cdecl) used by the vehicle code. Retail addresses; names are tier 3.
// 0x005004a0 (slot 4's solver helper) is ContactSolveImpulse, declared in
// ../contact/ContactImpulse.h.
struct VehicleContact;
// 0x0043ad80 / 0x0043aa30 / 0x0043aff0: contact-array helpers used by slot 49 (provisional).
extern int  __cdecl VehContactsA(int a, int* count, int n, VehicleContact** arr, SoultreeProbe* b, Vec3* pos, int c, float d);
extern void __cdecl VehContactsB(int n, VehicleContact** arr, Vec3* scale, Vec3* p, Vec3* v,
                                 Vec3* pos18, Vec3* pos0c, Vec3* o1, Vec3* o2, Vec3* o3);
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
    int  field_0x04;
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
// Table object at Vehicle+0x1f0: field_0xa4 points at a byte table (+0x400) indexed by material id.
struct VehicleMaterialSet {
    char pad_0x00[0xA4];
    char* field_0xa4;
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
    char field_0x24;
};
// Object at Vehicle+0x480 (provisional): lean/speed state; field_0x00 is read by slots 42/57 as a float.
struct VehicleSpeedEntry { float a; float b; };   // 8-byte table entry (slot 1 reads .a)
struct VehicleSpeedState {
    float field_0x00;
    int field_0x04;
    int field_0x08;
    float gearTimer;                // +0x0c countdown timer
    unsigned char gear;        // +0x10 table index (slot 1)
    char pad_0x11[0x1F];
    VehicleSpeedEntry field_0x30[6];
    float field_0x60[6];
    float randomStart;                // +0x78 randomised start value (slot 1)
    char pad_0x7C[0x8];
    int field_0x84;
    char pad_0x88[0x15c];           // the object is 0x1e4 bytes (LoadVehicle's allocation)
    ~VehicleSpeedState();                           // 0x00464e90, the shared empty out-of-line destructor (~Vehicle)
    // 0x004d2940 (ret 0x24): the gearbox constructor (src/reconstructed/GearRatios.h names the
    // parameters); LoadVehicle passes 0.02f, the table/rpm arguments or zeros, and field_0x524[2..3].
    VehicleSpeedState(const char* name, float unused, int* torqueTable, int rpmLow, int rpmHigh,
                      int rpmStep, int a6, float riseRate, float fallRate);
    void Method_004D2F50(float dt, int a, int b);   // 0x004d2f50, purpose unknown
    void Method_004D3030(int a, float speed);       // 0x004d3030, purpose unknown
};
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
    float smoothedValue;  // +0x00 slot 49 VehSmooth: x += (target-x)*blend
    float timeConstant;                // +0x04 time constant / cap
    float blendFactor;                // +0x08 last blend factor
    float minValue;                   // +0x0c FLT_MAX at construction
    float maxValue;                   // +0x10 -FLT_MAX at construction
};
struct VehicleContactSet;            // object at SoultreePhysicsCharacter+0x128
// Object at Vehicle+0x47c (provisional layout: only what Vehicle touches).
struct VehicleSteerState {
    SoultreeObject* steerNode;  // +0x00 scene node positioned/oriented in slots 36/58
    float steerAngle;  // +0x04 slot 60 subtracts it; slot 73 uses sin(it) and |it|
    float field_0x08;
    char pad_0x0c[0xc];              // 0x18 bytes (LoadVehicle's allocation)
    ~VehicleSteerState();                                      // 0x00504c50 (~Vehicle deletes it)
    // 0x00504b60 / 0x00504c70: SteeringControl's constructor and SetAxisFromDirection
    // (motion/SteeringControl.h); LoadVehicle builds the object with its two axes.
    VehicleSteerState(float scale, float t, const Vec3* axisZ, const Vec3* axisY);
    int SetAxisFromDirection(const Vec3* worldDir, SoultreeObject* frame);
    void AddAngle(float value, SoultreeObject* node);   // 0x00504ec0, purpose unknown
    void SetAngle(float a, SoultreeObject* node);       // 0x00504e20 (SteeringControl::SetAngle)
};
struct VehicleAxisSource {           // object at VehicleAxis+0x00 (provisional)
    char pad_0x00[0xC];
    int kindCode;                  // +0x0c kind code (2 and 3 are tested by slot 63)
    int Method_004897E0(int a);      // 0x004897e0, purpose unknown
};
struct VehicleAxis {                 // objects at Vehicle+0x4f8/0x4fc/0x500 (provisional)
    VehicleAxisSource* axisSource;  // +0x00 VehicleAxisSource queried in slot 63
    char pad_0x04[0x20];
    float axisValue;                // +0x24 axis value (slot 63 reads it)
};
// Object at Vehicle+0x5ac: the SparkParticleEmitter (vtable 0x00555c1c, ctor 0x004b9830,
// 0x60c bytes; samples/physics/effects/ParticleEmitters.h).  LoadVehicle builds it and hands the
// result of its virtual slot 27 to GameObject::Method_0x00469190; the impact handlers view
// it as VehicleImpactSink.  Provisional view.
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
    char pad_0x04[0x608];            // 0x60c bytes (LoadVehicle's allocation)
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
// 0x0043a570: appends a contact to the owner's contact array (provisional).
extern void __cdecl VehAddContact(int capacity, SoultreeContact** arr, int a2, int* count, void* contact);
extern void __stdcall VehWheelApply(float weight, float a, float b, float* speed);
class Vehicle;
// Suspension objects hung on a wheel (VehicleWheel+0x2b0 / +0x2ac).  PlaceWheels retracts
// them by a fraction of the last step and clears their forces while the wheel is in contact;
// the two Retract bodies differ (0x004fa310 / 0x004fab60), the ClearForces body 0x004f9c70 is
// shared and called directly.  Provisional views; samples/physics/suspension/Suspension.h
// reads them as InlineShock / RotatingShock.
struct VehicleWheel;
struct VehicleShock {
    char pad_0x00[0x58];
    Vec3 spring;                     // +0x58 spring force (Suspension.h name); Method_00529450 projects it on the ground normal
    char pad_0x64[0x10];
    Vec3 displacement;               // +0x74 Suspension.h name; scaled by field_0x94 and projected on the Y axis
    char pad_0x80[0x14];
    float field_0x94;                // scale of the displacement impulse, consumed (zeroed) by Method_00529450
    void ClearForces();                                         // 0x004f9c70
};
struct VehicleInlineShock : VehicleShock {
    void Retract(float amount, const VehicleWheel* carrier);   // 0x004fa310
};
struct VehicleRotatingShock : VehicleShock {
    void Retract(float amount, const VehicleWheel* carrier);   // 0x004fab60
};
struct VehicleWheelAux {             // object at VehicleWheel+0x2a8 (provisional)
    char pad_0x00[0x8C];
    int field_0x8c;
    int field_0x90;
    void Method_004D31B0(float a, Vec3* b, float c, bool d, float e, Vec3* f, Vec3* g);
};
// The wheel's collision point (VehicleWheel+0xb8, the CollisionPoint base of Tire): only its
// virtual slot 1 (friction update) is called here, by Method_00527A20.  Provisional view.
struct VehicleContactPoint {
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
};
// Elements of Vehicle+0x53c (provisional: only touched offsets are named).
struct VehicleWheel {
    char pad_0x00[0xB8];
    VehicleContactPoint contactPoint;  // +0xb8 AddWheel registers it in collisionPoints
    char pad_0xBC[0x10];
    Vec3 wheelPosition;              // +0xcc wheel contact position (slot 7 distance source)
    Vec3 groundPoint;              // +0xd8 (y at +0xdc is read as a height by slot 58)
    Vec3 groundNormal;              // +0xe4 contact normal (averaged by GetAverageGroundNormal)
    Vec3 field_0xf0;
    Vec3 appliedShare;              // +0xfc per-wheel share of the applied vector
    Vec3 field_0x108;
    Vec3 field_0x114;
    Vec3 field_0x120;
    float field_0x12c;  // Method_00527A20: -(appliedShare . groundNormal) when that is negative, else 0
    Vec3 field_0x130;
    float field_0x13c;  // Method_00527A20: contact time scale (the impulse is limited by it)
    char pad_0x140[4];
    float contactLoad;  // +0x144 slot 72 accumulates it over wheels and averages by the contact count
    float field_0x148;  // Method_00527A20 adds it to field_0x14c for the available load
    float field_0x14c;
    float field_0x150;
    char pad_0x154[4];
    float loadWeight;               // +0x158 blend weight
    int field_0x15c;
    int impactPosted;                 // +0x160 impact-handled flag (slot 18)
    int slidePosted;                 // +0x164 impact-handled flag (slot 19)
    int scrapePosted;                 // +0x168 impact-handled flag (slot 20)
    int field_0x16c;
    char pad_0x170[4];
    unsigned char surfaceMaterial;       // +0x174 surface material id
    char pad_0x175[0x47];
    SoultreeObject* sceneNode;     // +0x1bc wheel scene node (slot 33 reads its position)
    int field_0x1c0;
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
    Vec3 field_0x248;
    Vec3 field_0x254;
    int inContact;  // +0x260 selects wheels touching the ground (slots 7, 54, 72, 75-86); count equals Vehicle::wheelsInContact
    int reportContactOutputs;  // +0x264 (Tire.h name) PlaceWheels sets it on the first in-contact wheel without field_0x1c0 (else on wheel 0)
    int field_0x268;
    int field_0x26c;
    char pad_0x270[8];
    int field_0x278;
    int field_0x27c;
    Vec3 field_0x280;
    float tangentCos;              // +0x28c Tire.h: |tangent . rollDirection| clamped to 1
    float field_0x290;
    float field_0x294;
    char pad_0x298[0x4];
    float rampLevel;               // +0x29c 0 selects a flag passed to slot 77 (slot 83)
    float driveStrength;           // +0x2a0 Tire.h ctor arg a10, read by 0x00514170; Method_00527A20 counts the in-contact wheels where it is nonzero
    float field_0x2a4;
    VehicleWheelAux* aux;          // +0x2a8 Tire.h TireAux
    int secondaryAux;  // +0x2ac same use as primaryAux, the fallback when it is null (UpdateWheelsInContact, Method_005293E0)
    int primaryAux;  // +0x2b0 AddWheel stores an attachment here; UpdateWheelsInContact treats it as an object whose +0x8c is reset from +0x90
    char pad_0x2B4[0x4];
    float field_0x2b8;
    float field_0x2bc;
    void UpdateRoll(float a, float b, int c, int d, int e, float f);   // 0x005143d0
    void UpdateContactPatch(Vec3* origin, Vec3 position, Vec3 velocity, float speed, float* outA, int* outB);   // 0x005135f0 (Tire.h name)
    void Method_00513C70(float bias, int flag, int crashState, float speed, Vec3* velocity, Vec3* anchor);   // 0x00513c70
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

class Vehicle : public SoultreePhysicsCharacter {
public:
    Vehicle();
    explicit Vehicle(int flags);                 // 0x005257a0 (ret 8: flags + hidden most-derived flag)
    virtual ~Vehicle();                          // vbase deleting dtor 0x0052b630
    virtual int GameObjectVirtualSlot10(float dt);  // 0x0052a830 via vtordisp thunk 0x0040cab0

    // ---- overrides of inherited slots (order as in the vtable) ----
    virtual void UnknownVirtualSlot0(float dt);
    virtual void UnknownVirtualSlot1(float value);                               // 0x00525550
    virtual void UnknownVirtualSlot3(const Vec3* a, const Vec3* b, const Vec3* c,
                                     const Vec3* d, int e, int f, float* g); // 0x005264c0
    virtual void UnknownVirtualSlot4(const Vec3* a0, Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, float a5, int a6,
                                     const Vec3* a7, const Vec3* a8, const Vec3* a9,
                                     Vec3* a10, Vec3* a11, int a12, float* a13, float a14);
    virtual void UnknownVirtualSlot7(const Vec3* arg);
    // Slots 18..20 take the canonical 40-byte record; the bodies view it as
    // VehicleImpactEvent (same offsets: +0x04/+0x08/+0x0c sinks, +0x24 flag).
    virtual void UnknownVirtualSlot18(SoultreeAttachment* arg);
    virtual void UnknownVirtualSlot19(SoultreeAttachment* arg);
    virtual void UnknownVirtualSlot20(SoultreeAttachment* arg);
    virtual void UnknownVirtualSlot21();
    virtual int  UnknownVirtualSlot23();
    virtual int  UnknownVirtualSlot24();
    virtual int  UnknownVirtualSlot25();
    virtual int  UnknownVirtualSlot28(int arg);
    virtual int  UnknownVirtualSlot33(const Vec3* a, const Vec3* b, const Vec3* c,
                                      const Vec3* d, int e, float f);   // 0x00527430
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35(int a, int b);
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot38(int a, int b, void* c);
    virtual int  UnknownVirtualSlot39(float dt);
    virtual int  UnknownVirtualSlot42();

    // ---- slots introduced by Vehicle (43..96) ----
    virtual void UnknownVirtualSlot43();
    virtual void UnknownVirtualSlot44();
    virtual float UnknownVirtualSlot45();
    virtual Vec3* UnknownVirtualSlot46(Vec3* out, float arg);
    // pure virtual: entry is _purecall (0x00534cfe).  Bike's override 0x0040cbb0 is
    // `ret 4` and returns a float (tier 1 via its byte match), hence float(float).
    virtual float UnknownVirtualSlot47(float arg) = 0;
    virtual void UnknownVirtualSlot48();
    // 0x0052a940 forwards its argument unchanged to slots 9 (float dt), 39, 64 and 65,
    // so it and they take the float time step (tier 2).
    virtual void UnknownVirtualSlot49(float dt);
    virtual void UnknownVirtualSlot50(int a, float b, int c);
    virtual int UnknownVirtualSlot51();
    virtual int UnknownVirtualSlot52();
    virtual float UnknownVirtualSlot53();
    virtual Vec3* UnknownVirtualSlot54(Vec3* out);
    virtual Vec3* UnknownVirtualSlot55(Vec3* unused, Vec3* out);
    virtual void UnknownVirtualSlot56(Vec3* a, int b, Vec3* c);
    virtual float UnknownVirtualSlot57();
    virtual void UnknownVirtualSlot58(Vec3* a, int b);
    virtual float UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60(float a, float b, int c);
    virtual float UnknownVirtualSlot61(float arg);
    virtual int UnknownVirtualSlot62(float arg);
    virtual void UnknownVirtualSlot63(float dt);
    // void: 0x00528e50 leaves eax as whatever the last store used (no return value is
    // materialised) and the only caller, slot 49, ignores eax; byte-exact as void (tier 2).
    virtual void UnknownVirtualSlot64(float dt);
    virtual void UnknownVirtualSlot65(float dt);  // entry is the shared empty stub 0x00464e80
    virtual int UnknownVirtualSlot66() = 0;  // pure virtual: entry is _purecall (0x00534cfe)
    virtual void UnknownVirtualSlot67();
    virtual int UnknownVirtualSlot68(int* out);
    virtual void UnknownVirtualSlot69();
    virtual int UnknownVirtualSlot70(float arg);
    virtual void UnknownVirtualSlot71(int arg);
    virtual void UnknownVirtualSlot72(Vec3* out, VehicleWheel* wheel);
    virtual float UnknownVirtualSlot73(const Vec3* a, const Vec3* b);  // only read (Bike 0x00406090)
    virtual float UnknownVirtualSlot74(Vec3* point, Vec3* dir, float c, float d);
    virtual float UnknownVirtualSlot75();
    // ret 0xc = hidden result pointer + two arguments; KrustyBike 0x00491ca0 byte-matches
    // as a by-value Vec3 return (tier 1 for the ABI shape, tier 2 for the Vec3 pointers).
    virtual Vec3 UnknownVirtualSlot76(const Vec3* a, const Vec3* b);
    virtual int UnknownVirtualSlot77(int arg);
    virtual int UnknownVirtualSlot78();
    virtual int UnknownVirtualSlot79();
    virtual int UnknownVirtualSlot80();
    virtual int UnknownVirtualSlot81();
    virtual int UnknownVirtualSlot82();
    virtual int UnknownVirtualSlot83(VehicleWheel* wheel);
    // Returns the input-map query (KrustyBike 0x00491630 uses the result; the Vehicle
    // body 0x00529250 leaves it in eax) (tier 2).
    virtual int UnknownVirtualSlot84(int a, int b);
    virtual void UnknownVirtualSlot85();
    virtual void UnknownVirtualSlot86();
    virtual void UnknownVirtualSlot87();  // entry is the shared empty stub 0x00464e90
    virtual int UnknownVirtualSlot88();
    virtual int UnknownVirtualSlot89(float arg);      // Bike 0x0040ae50 passes a float (tier 2)
    virtual void UnknownVirtualSlot90(int* a, float b);
    virtual void UnknownVirtualSlot91();  // entry is the shared empty stub 0x00464e90
    virtual void UnknownVirtualSlot92(const Vec3* a, const Vec3* b);
    virtual void UnknownVirtualSlot93();
    virtual void UnknownVirtualSlot94();  // entry is the shared empty stub 0x00464e90
    virtual void UnknownVirtualSlot95();
    virtual void UnknownVirtualSlot96();

    // 0x00526830 (next to slot 70): stores the crash answer in crashState and, when crashed,
    // clears the controls and records the heading; returns it (tier 3 name).
    int CheckCrash();
    // 0x00525e20 (`ret 0x94` = 37 argument dwords; not in any vtable: Bike's loader 0x004079c0
    // calls it directly).  Forwards a1..a11, a13, a14 and a21 to SoultreePhysicsCharacter slot
    // 40 with (0.02f, 100, 0.001f, 0.1f, 3) and the track byte; the other arguments configure
    // the vehicle (names tier 3).
    GameObject* LoadVehicle(int a1, const char* engineName, const char* a3, const SoultreeLoadDesc* a4, int a5,
                            Vec3 a6, Vec3 a7, Vec3 a8, void* a9, VehicleInputMap* map, void* a10,
                            float a11, float arg18, float steerScale, void* device, int wheelCap,
                            int extraContacts, int a13, int earlyCap, int lateCap, int defaultEngine,
                            int* torqueTable, int rpmLow, int rpmHigh, int rpmStep, int gearArg,
                            VehicleAxis* steer, VehicleAxis* lean, VehicleAxis* throttle,
                            SoultreeSlot1f0* a14, int a21);
    int Method_00478FE0();      // 0x00478fe0, shared `xor eax,eax; ret` stub (direct call from CheckCrash)
    int AddEarlyTicker(VehicleTicker* ticker);                  // 0x00525cb0, append to earlyTickers
    int AddLateTicker(VehicleTicker* ticker);                   // 0x00525cf0, append to lateTickers
    int AddWheel(VehicleWheel* wheel, int a2, int a3, int a4, VehicleWheelAux* aux);   // 0x00525d30, appends a wheel and registers its contact point
    // 0x00528400: normalized average of the wheels' contact normals (+0xe4); returns out.
    Vec3* GetAverageGroundNormal(Vec3* out);
    int   UpdateWheelRampLevels();       // 0x00529280, ramps each wheel's rampLevel by its slot 83 answer
    void  PlaceWheels();                 // 0x00528eb0, retracts the shocks and probes each wheel against the ground
    void  UpdateWheelsInContact();       // 0x00529a20, updates in-contact wheels, resets the aux ramp of the others
    void  Method_00527A20(float* speed, Vec3* zero, Vec3* up);   // 0x00527a20
    void  Method_005293E0(float* speed);                         // 0x005293e0
    void  Method_00529450(Vec3* up, Vec3* zero);           // 0x00529450
    void  AccumulateWheelContacts(Vec3* up, Vec3* zero, float d);  // 0x00529c20, folds in-contact wheel vectors into the accumulators
    // 0x00525c60: calls virtual slot 0 of every object in the two owned arrays (+0x554/+0x55c, +0x560/+0x568).
    void RunTickers();
    void ResetWheelContacts();           // 0x00525a90, resets every wheel's contact state and vectors

    // Inherited members whose canonical types are still separate classes are viewed
    // through casts at the use site (MIGRATION.md rule 6): field_0x12c as
    // VehicleContact**, field_0x128 as
    // VehicleContactSet*, field_0x1f0 as VehicleMaterialSet*.  Inline accessor functions
    // were tried; VC6 schedules their loads differently (slots 4, 19, 35 and 49 lost
    // bytes, Bike slot 8 lost its exact match), so the casts are written out.

    float field_0x434;
    float field_0x438;        // read by Bike slot 59 (0x0040a330) as a float divisor
    float turnRate;  // +0x43c = turnAngle * field_0x140 (1/dt) in slot 72: velocity deflection angle per second; reset to 0 whenever no wheel drives it
    int movingForward;  // +0x440 slot 85 passes +dt when set and -dt when clear to every wheel; slot 74 flips the sign of its result on it; slot 1 resets it to 1
    int crashState;  // +0x444 nonzero disables input (slots 52/62/64/60 test ==0), switches solver vectors (+0xf0, 0.1 scale, 1,1,1 mask); Bike slot 99 writes it and runs the crash timer 0x454 while set
    int crashDirection;          // +0x448 Bike slot 99 (0x00409b30) stores small codes (1..5)
    float field_0x44c;        // Bike slot 99 (0x00409b30) multiplies it (float)
    float speedGainLimit;  // +0x450 slot 70: true when (speed field_0xbc - previous speed field_0xb8) < arg * speedGainLimit
    // 0x454/0x458: Bike slot 39 (0x0040a1f0) decrements 0x454 by the frame time and
    // compares it with 0.0f/-2.0f, so both are floats (tier 2; slot 69 copies 0x458).
    float crashTimer;  // +0x454 Bike slot 39 decrements it by the frame time while crashed and compares with 0/-0.1/-2.0; slots 1/67 clear it
    float crashTimerReload;  // +0x458 slot 69 copies it into crashTimer
    float field_0x45c;        // Bike slot 100 (0x0040a090) stores field_0x50 here
    int crashReason;  // +0x460 Bike slot 99 stores reason codes (2,4,6,7,8,10,11,13) beside crashDirection
    int field_0x464;
    VehicleInputMap* inputMap;  // +0x468 object whose virtual slots 2/3 are queried with action ids 0x0e/0x1d/0x1f/0x2d/0xc8/0xd0 (slots 77-84); slot 39 also uses its key table
    void* inputDevice;  // +0x46c tested non-null together with throttleAxis before the throttle axis is latched in slot 63 (-1 when absent)
    float throttleInput;  // +0x470 slot 63 latches axis 0x500 value (dead zone 0.2, else -1 when no device); slot 80 tests it > 0.33
    float steerInput;  // +0x474 slot 63: = -axis(0x4f8).value; slot 88 classifies it with 0x504.y into 8 stick sectors; slot 64 zeroes it when crashed
    bool field_0x478;
    bool field_0x479;
    bool field_0x47a;
    char pad_0x47B[0x1];
    VehicleSteerState* steerState;  // +0x47c VehicleSteerState*: node plus steer angle; slots 53/60/73/36 use it
    VehicleSpeedState* engineState;  // +0x480 VehicleSpeedState*: 6-entry per-gear tables, advanced by Method_004D2F50/004D3030 with throttle/brake flags and speed in slot 49 (provisional)
    int justLanded;  // +0x484 slot 49 clears it per frame; slot 71 sets it to 1 when the airborne flag 0x108 drops (landing)
    Vec3 takeoffPosition;  // +0x488 slot 71 copies position field_0x0c here when the vehicle leaves the ground
    Vec3 takeoffVelocity;  // +0x494 slot 71 copies velocity field_0x64 here when the vehicle leaves the ground
    int field_0x4a0;          // set to 1 by slot 44
    float field_0x4a4;
    int wheelsInContact;  // +0x4a8 slot 54 divides the wheel normal sum by it; slot 49: 0x5a8 = (it == wheel count), 0x5a4 = (it != 0); slots 46/7 branch on 0/1/2
    float leanAngle;  // +0x4ac slot 56 stores asin(...) of the ground normal against the forward axis; slot 57 returns it; Bike rotates the body by it
    float leanCos;  // +0x4b0 slot 56 stores cos(leanAngle) there
    float field_0x4b4;
    float turnAngle;  // +0x4b8 result of slot 74 (signed deflection angle of the velocity), see turnRate
    float steerRate;  // +0x4bc slot 60: -(a*b) - steerAngle divided by (c*dt+...); slot 53 feeds steerRate*dt to the steer state
    Vec3 tiltAxisLocal;  // +0x4c0 slot 49: WorldToLocalDirection of the lift axis, used as the RotateAboutPoint axis
    Vec3 sideAxis;  // +0x4cc slots 34/35: cross product of the two basis vectors field_0x88 and field_0x94
    float field_0x4d8;
    float field_0x4dc;
    float targetLeanAngle;  // +0x4e0 Bike clamps it to +-maxLeanAngle and subtracts it from the current lean (slot 57)
    float leanError;        // +0x4e4 Bike slot 92 (0x0040ba30): _finite()/fabs on it (float, tier 2)
    float field_0x4e8;
    float wheelBase;  // +0x4ec slot 73 returns wheelBase / sin(steer angle) (bicycle-model turn radius)
    int spawnProtected;  // +0x4f0 slot 50 stores arg a here, slot 51 returns it; slot 33 (respawn) sets it using slot 45 (3.0)
    float spawnProtectTimer;   // +0x4f4 tier 2: countdown; slot 49 tests > 0 and subtracts field_0x13c (frame dt), slot 50 stores it
    VehicleAxis* steerAxis;  // +0x4f8 slot 63 reads its value into steerInput and 0x504.x
    VehicleAxis* leanAxis;  // +0x4fc slot 63 reads -value into controlInput.y
    VehicleAxis* throttleAxis;  // +0x500 slot 63 reads its value into throttleInput (dead zone 0.2)
    Vec3 controlInput;  // +0x504 slot 63 writes x = steer (cubed for axis kinds 2/3), y = -lean axis; slot 64 zeroes it when crashed
    Vec3 prevControlInput;  // +0x510 slot 49 copies controlInput here before slot 64 refreshes it
    float field_0x51c;
    int field_0x520;
    float field_0x524[6];
    VehicleWheel** wheelList;  // +0x53c array of VehicleWheel*, iterated up to wheelCount in slots 7/54/72/75/85/86
    int wheelCapacity;               // +0x540 wheel capacity (AddWheel)
    int wheelCount;  // +0x544 loop bound over wheelList in every wheel loop
    VehicleWheel* primaryWheel;  // +0x548 slot 7 (single contact) and slot 58 (single wheel) use it as the lead wheel; slot 46 pairs it with secondaryWheel
    VehicleWheel* secondaryWheel;  // +0x54c partner of primaryWheel in slot 46 (two-wheel case)
    int field_0x550;
    VehicleTicker** lateTickers;   // +0x554 owned arrays of objects with a virtual slot 0 (RunTickers)
    int lateTickerCapacity;               // +0x558 capacity of lateTickers
    int lateTickerCount;               // +0x55c count of lateTickers
    VehicleTicker** earlyTickers;  // +0x560 array of objects whose virtual slot 0 is called first by RunTickers; AddEarlyTicker appends to it
    int earlyTickerCapacity;               // +0x564 capacity of earlyTickers
    int earlyTickerCount;               // +0x568 count of earlyTickers
    int auxWheelCount;               // +0x56c wheels added with an aux object (AddWheel)
    int contactTotal;  // +0x570 = field_0x1cc + wheelsInContact (body contacts plus wheels in contact) in slot 7; slot 49 branches on > 0
    Vec3 field_0x574;      // Bike slot 100 (0x0040a090) builds (a0.x, 0, a0.z) here
    float maxLeanAngle;  // +0x580 Bike clamps the target lean to +-maxLeanAngle
    float maxLeanRate;  // +0x584 Bike limits the lean error change to maxLeanRate * dt
    float smoothedVerticalAccel;  // +0x588 slot 49: = verticalAccelSmoother->value, fed with (vy - prev vy) * (1/dt)
    VehicleSmoother* verticalAccelSmoother;  // +0x58c VehicleSmoother fed in slot 49 with the per-step change of velocity.y
    float smoothedForwardAccel;  // +0x590 slot 49: = forwardAccelSmoother->value
    float prevLocalForwardVelocity;  // +0x594 slot 49: stores local velocity z after smoothing (previous-step value for the next difference)
    VehicleSmoother* forwardAccelSmoother;  // +0x598 VehicleSmoother fed with (local vel z - prevLocalForwardVelocity)/dt
    int landingLatched;  // +0x59c slot 71: set when landing while not yet latched, cleared in slot 49 while airborne
    int field_0x5a0;
    int anyWheelInContact;  // +0x5a4 slot 49: = (wheelsInContact != 0)
    int allWheelsInContact;  // +0x5a8 slot 49: = (wheelsInContact == wheelCount)
    VehicleSparkEmitter* field_0x5ac;  // spark emitter built by LoadVehicle (impact handlers cast it to VehicleImpactSink)
    float field_0x5b0;
    int prevCrashState;  // +0x5b4 slot 49 stores crashState there at the end of each step; slots 1/67 clear it
    char pad_0x5B8[0x4];      // own data ends at 0x5bc; the compiler places the vtordisp there
};

VEH_CHECK_OFFSET(Vehicle, field_0x434, 0x434);
VEH_CHECK_OFFSET(Vehicle, turnRate, 0x43C);
VEH_CHECK_OFFSET(Vehicle, movingForward, 0x440);
VEH_CHECK_OFFSET(Vehicle, crashState, 0x444);
VEH_CHECK_OFFSET(Vehicle, speedGainLimit, 0x450);
VEH_CHECK_OFFSET(Vehicle, crashTimer, 0x454);
VEH_CHECK_OFFSET(Vehicle, crashTimerReload, 0x458);
VEH_CHECK_OFFSET(Vehicle, inputMap, 0x468);
VEH_CHECK_OFFSET(Vehicle, inputDevice, 0x46C);
VEH_CHECK_OFFSET(Vehicle, throttleInput, 0x470);
VEH_CHECK_OFFSET(Vehicle, steerInput, 0x474);
VEH_CHECK_OFFSET(Vehicle, field_0x478, 0x478);
VEH_CHECK_OFFSET(Vehicle, field_0x479, 0x479);
VEH_CHECK_OFFSET(Vehicle, field_0x47a, 0x47A);
VEH_CHECK_OFFSET(Vehicle, steerState, 0x47C);
VEH_CHECK_OFFSET(Vehicle, engineState, 0x480);
VEH_CHECK_OFFSET(Vehicle, justLanded, 0x484);
VEH_CHECK_OFFSET(Vehicle, takeoffPosition, 0x488);
VEH_CHECK_OFFSET(Vehicle, takeoffVelocity, 0x494);
VEH_CHECK_OFFSET(Vehicle, field_0x4a4, 0x4A4);
VEH_CHECK_OFFSET(Vehicle, wheelsInContact, 0x4A8);
VEH_CHECK_OFFSET(Vehicle, leanAngle, 0x4AC);
VEH_CHECK_OFFSET(Vehicle, leanCos, 0x4B0);
VEH_CHECK_OFFSET(Vehicle, field_0x4b4, 0x4B4);
VEH_CHECK_OFFSET(Vehicle, turnAngle, 0x4B8);
VEH_CHECK_OFFSET(Vehicle, steerRate, 0x4BC);
VEH_CHECK_OFFSET(Vehicle, tiltAxisLocal, 0x4C0);
VEH_CHECK_OFFSET(Vehicle, sideAxis, 0x4CC);
VEH_CHECK_OFFSET(Vehicle, field_0x4d8, 0x4D8);
VEH_CHECK_OFFSET(Vehicle, field_0x4dc, 0x4DC);
VEH_CHECK_OFFSET(Vehicle, targetLeanAngle, 0x4E0);
VEH_CHECK_OFFSET(Vehicle, leanError, 0x4E4);
VEH_CHECK_OFFSET(Vehicle, field_0x4e8, 0x4E8);
VEH_CHECK_OFFSET(Vehicle, wheelBase, 0x4EC);
VEH_CHECK_OFFSET(Vehicle, spawnProtected, 0x4F0);
VEH_CHECK_OFFSET(Vehicle, spawnProtectTimer, 0x4F4);
VEH_CHECK_OFFSET(Vehicle, steerAxis, 0x4F8);
VEH_CHECK_OFFSET(Vehicle, leanAxis, 0x4FC);
VEH_CHECK_OFFSET(Vehicle, throttleAxis, 0x500);
VEH_CHECK_OFFSET(Vehicle, controlInput, 0x504);
VEH_CHECK_OFFSET(Vehicle, prevControlInput, 0x510);
VEH_CHECK_OFFSET(Vehicle, field_0x51c, 0x51C);
VEH_CHECK_OFFSET(Vehicle, field_0x520, 0x520);
VEH_CHECK_OFFSET(Vehicle, field_0x524, 0x524);
VEH_CHECK_OFFSET(Vehicle, wheelList, 0x53C);
VEH_CHECK_OFFSET(Vehicle, wheelCount, 0x544);
VEH_CHECK_OFFSET(Vehicle, primaryWheel, 0x548);
VEH_CHECK_OFFSET(Vehicle, secondaryWheel, 0x54C);
VEH_CHECK_OFFSET(Vehicle, contactTotal, 0x570);
VEH_CHECK_OFFSET(Vehicle, smoothedVerticalAccel, 0x588);
VEH_CHECK_OFFSET(Vehicle, verticalAccelSmoother, 0x58C);
VEH_CHECK_OFFSET(Vehicle, smoothedForwardAccel, 0x590);
VEH_CHECK_OFFSET(Vehicle, prevLocalForwardVelocity, 0x594);
VEH_CHECK_OFFSET(Vehicle, forwardAccelSmoother, 0x598);
VEH_CHECK_OFFSET(Vehicle, landingLatched, 0x59C);
VEH_CHECK_OFFSET(Vehicle, field_0x5a0, 0x5A0);
VEH_CHECK_OFFSET(Vehicle, anyWheelInContact, 0x5A4);
VEH_CHECK_OFFSET(Vehicle, allWheelsInContact, 0x5A8);
VEH_CHECK_OFFSET(Vehicle, field_0x5ac, 0x5AC);
VEH_CHECK_OFFSET(Vehicle, field_0x5b0, 0x5B0);
VEH_CHECK_OFFSET(Vehicle, prevCrashState, 0x5B4);
typedef char veh_assert_sizeof[(sizeof(Vehicle) == 0x5ec) ? 1 : -1];

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
    // Same cross product with the y-component written b.x * z (member order is honoured).
    Vec3 CrossB(const Vec3& b) const
    {
        Vec3 r;
        r.x = y * b.z - z * b.y;
        r.y = b.x * z - x * b.z;
        r.z = x * b.y - b.x * y;
        return r;
    }
};

// Scene-node method 0x004444e0 (purpose unknown) is not in SoultreeObject yet; view the
// node through a local stand-in that declares it.
struct VehSceneNodeView {
    void Method_004444E0();
    void SetAxesPtr(const Vec3* axisZ, const Vec3* axisY, int a, int b);   // 0x004fbd70, purpose unknown
};

#endif
