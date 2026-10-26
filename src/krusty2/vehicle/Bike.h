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
struct BikeA34;
struct BikeA5C4;
struct BikeA1A0;
struct BikeA128;
struct BikeElem;
struct BikeA1F4;
struct BikeA640;
struct BikeA644;
struct BikeQ;
struct BikeA38;

// Object at Bike+0x60c: the handlebar part (samples/physics/tire/Tire.h MovingPart, 0x4c bytes).
// The loader builds it with `new(__FILE__, 0x660)` from the "Handlebars" node of the model and
// the steer state; ~Bike deletes it through an out-of-line empty destructor (0x00464e90).
#include "BikeHandlebarView.h"

// 24-byte block the loader's sixth argument points to; copied into Bike+0x5c8 when present
// (tier 3: contents unknown, only the whole-block copy is decoded).
#include "BikeLoaderRecord.h"

// Particle emitters the loader hangs on the bike (samples/physics/effects/ParticleEmitters.h:
// Dust 0x80 bytes, DirtChunk 0x608, Steam 0x47c).  Provisional views: the constructor, the
// GameObject-shaped virtual slot 27 that returns the child to register, and Steam's +0x60 flag.
struct BikeParticleEmitterView {
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
};
struct BikeDustEmitter : BikeParticleEmitterView {
    explicit BikeDustEmitter(int flags);        // 0x004b8a00
    char pad_0x04[0x7c];                        // 0x80 bytes (`new(__FILE__, 0x70f)`)
};
struct BikeDirtChunkEmitter : BikeParticleEmitterView {
    explicit BikeDirtChunkEmitter(int flags);   // 0x004b8df0
    char pad_0x04[0x604];                       // 0x608 bytes (`new(__FILE__, 0x710)`)
};
struct BikeSteamEmitter : BikeParticleEmitterView {
    explicit BikeSteamEmitter(int flags);       // 0x004b9f40
    char pad_0x04[0x5c];
    int emitting;  // +0x60  SteamParticleEmitter +0x60 emitting; the loader sets it to 1
    char pad_0x64[0x418];                       // 0x47c bytes (`new(__FILE__, 0x711)`)
};

// Objects reached through Bike fields.  Layout is only known where accessed.
// Frame object at BikeWheel+0x1c0 (provisional): only the vector at +0xc is read.
struct BikeWheelFrame {
    char pad_0x00[0xc];
    Vec3 axis;
};
// Objects at Bike+0x5f8 / +0x5fc (provisional, Method_0x0040a520): a point, an axis, an
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
    Vec3 normalLoadVector;                    // VehicleWheel +0x120 normalLoadVector; Method_0x0040a520 crosses the wheel offset with it while in contact
    char pad_0x12c[24];
    float contactLoad;
    float tangentSpeed;  // +0x148 CollisionPoint+0x90 tangentSpeed (VehicleWheel name)
    float spinSpeed;  // +0x14c CollisionPoint+0x94 spinSpeed (VehicleWheel name); KrustyBike 0x00413200 clears it
    float penetration;  // +0x150 CollisionPoint+0x98 penetration (VehicleWheel name)
    char pad_0x154[4];
    float loadWeight;
    char pad_0x15c[16];
    int w_0x16c;              // Method_0x0040a520 sets it to 2 while the wheel is in contact
    char pad_0x170[76];
    SoultreeObject* sceneNode;  // +0x1bc wheel scene node (VehicleWheel name)
    BikeWheelFrame* ownerRef;  // +0x1c0 MovingPart+0x44 ownerRef (Tire ctor arg c; sceneNode +0x1bc is MovingPart+0x40); Method_0x0040a520 projects on its +0xc axis
    char pad_0x1c4[60];
    Vec3 nodePosition;        // +0x200 (VehicleWheel name)
    Vec3 wheelUpAxis;  // +0x20c Tire.h wheelUpAxis; scaled by wheelRadius while airborne (Method_0x0040a520)
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
    float w_0x2a4;
    BikeWheelQ* aux;  // +0x2a8 VehicleWheel aux (Tire.h TireAux)
    BikeQ* secondaryAux;  // +0x2ac VehicleWheel secondaryAux (the fallback when primaryAux is null)
    BikeQ* primaryAux;  // +0x2b0 VehicleWheel primaryAux (attachment AddWheel stores)
    char pad_0x2b4[4];
    float driveShare;  // +0x2b8 UpdateDriveShare (0x00513f90) stores the drive share here (fst [esi+0x2b8]; 1.0 on the other path)
    float driveShareComplement;  // +0x2bc UpdateDriveShare (0x00513f90) stores 1 - driveShare here (0 on the other path)
    void UpdateDriveShare(void* owner);   // 0x00513f90 (Tire.h name: drive share from the vehicle speed)
};

// Object at BikeWheel+0x2a8: the gearbox (engineState) for driven wheels (Bike's loader).
// Opaque provisional view; force calls use UnknownEngineGearbox directly.
struct BikeWheelQ {};


struct BikeA34 {
    char pad_0x000[416];
    BikeA1A0* r_0x1a0;
};

struct BikeA5C4 {
    char pad_0x000[12];
    int c_0xc;
    char pad_0x010[400];
    BikeA1A0* c_0x1a0;
    void Method_0x004a8c50(int firstHandle, int secondHandle, float phase, float weight);
    // The rider is a D3DIMSoultreeCharacter (the loader builds it with 0x004455b0): Character's
    // FindMotion 0x004a6b30 and Method_0x004a8bf0 seen through this view.
    int FindMotion(const char* name, int unused);
    void Method_0x004a8bf0(int handle, float phase);
};

struct BikeA1A0 {
    char pad_0x000[320];
    SoultreeObject* d_0x140;           // SoultreeObject::firstChild
    int FindByName(const char* name);     // 0x004fdae0, SoultreeObject::FindByName (soultree.cpp)
    void GetSubtreeBounds(Vec3* center, Vec3* extents);   // 0x004fe850 (SoultreeObject::GetSubtreeBounds)
    void SetMatrixIn(int frame, Matrix4* m);
};

struct BikeA128 {
    char pad_0x000[752];
    int g_0x2f0;
};

struct BikeElem {
    char pad_0x000[4];
    int h_0x4;
    char pad_0x008[12];
    Vec3 h_0x14;
    char pad_0x020[36];
    Vec3 h_0x44;
    char pad_0x050[80];
    float h_0xa0;
    int h_0xa4;
};

struct BikeA1F4 {
    char pad_0x000[3048];
    int i_0xbe8;
};

// Smoother objects at Bike+0x640 / +0x644 (the Vehicle.h VehicleSmoother shape, 0x14 bytes each;
// the loader builds them with `new(__FILE__, 0x7df/0x7e0)` and inline constructors).
struct BikeA640 {
    BikeA640(float value, float tc)
    {
        steerValue = value;
        l_0x4 = tc;
        minValue = 3.402823466e+38f;
        maxValue = -3.402823466e+38f;
        l_0x8 = 1.0f;
    }
    float steerValue;
    float l_0x4;
    float l_0x8;
    float minValue;
    float maxValue;
};

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

// Shock objects at BikeWheel+0x2ac (RotatingShock) / +0x2b0 (InlineShock); the classes are
// in samples/physics/suspension/Suspension.h.
struct BikeQ {
    char pad_0x000[148];
    float impulseScale;                       // VehicleShock +0x94 impulseScale; slot 65 stores min(min(0.6 * smoothedVerticalAccel, 100) * rate * 5, 10000)
    float q_0x98;             // KrustyBike 0x00413200 stores the rear travel here
    char pad_0x09c[0x24];
    Vec3 q_0xc0;              // InlineShock::axis
    void ClampAndStepAlong(float maxDelta, float dt, Vec3& pos);   // 0x004fa090 InlineShock::ClampAndStep
    void ClampAndStep(float maxDelta, float dt);                   // 0x004fa8b0 RotatingShock::ClampAndStep
};

struct BikeA38 {
    char pad_0x000[100];
    int n_0x64;
    char pad_0x068[124];
    BikeA1F4* n_0xe4;
    void Method_0x00435fe0();
};

// The scene node (d3d_field_0x1a0, field_0x42c, BikeA1A0::d_0x140) is SoultreeObject
// (../common/SoultreeObject.h).

struct BikeGlobal_0056e26c { char pad_0x000[0x2f0]; float g_0x2f0; };

// Bike.cpp uses its Math3D header-local kVec3Zero, initialized at 0x00407890.

#include "BikeCore.h"

#endif
