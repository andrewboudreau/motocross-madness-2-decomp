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

#include "vehicle/Vehicle.h"
#include "core/DebugAlloc.h"


struct BikeWheel;
struct BikeWheelQ;
struct BikeA604;
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
struct BikeA60C {
    BikeA60C(const char* name, SoultreeObject* root, VehicleSteerState* owner);   // 0x004a23a0 (ret 0xc)
    ~BikeA60C();
    char pad_0x00[0x40];
    SoultreeObject* node;          // +0x40 root->FindByName(name); the shocks and contacts are anchored on it
    VehicleSteerState* owner;      // +0x44 ctor argument
    int field_0x48;
};

// 24-byte block the loader's sixth argument points to; copied into Bike+0x5c8 when present
// (tier 3: contents unknown, only the whole-block copy is decoded).
struct BikeA5C8 {
    int field_0x00[6];
};

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
    int field_0x60;                             // the loader sets it to 1
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
    Vec3 w_0x0d8;             // KrustyBike 0x00413200 moves it with the body's step displacement
    Vec3 groundNormal;
    char pad_0x0f0[12];
    Vec3 appliedShare;
    Vec3 w_0x108;
    char pad_0x114[12];
    Vec3 w_0x120;             // Method_0x0040a520 crosses the wheel offset with it while in contact
    char pad_0x12c[24];
    float contactLoad;
    float w_0x148;
    float w_0x14c;            // KrustyBike 0x00413200 clears it
    float w_0x150;
    char pad_0x154[4];
    float loadWeight;
    char pad_0x15c[16];
    int w_0x16c;              // Method_0x0040a520 sets it to 2 while the wheel is in contact
    char pad_0x170[76];
    SoultreeObject* sceneNode;  // +0x1bc wheel scene node (VehicleWheel name)
    BikeWheelFrame* w_0x1c0;  // frame whose +0xc axis Method_0x0040a520 projects on
    char pad_0x1c4[60];
    Vec3 nodePosition;        // +0x200 (VehicleWheel name)
    Vec3 w_0x20c;             // scaled by w_0x274 while airborne (Method_0x0040a520)
    char pad_0x218[24];
    Vec3 w_0x230;
    Vec3 w_0x23c;
    Vec3 w_0x248;             // KrustyBike slot 48 (0x004964e0) zeroes it
    char pad_0x254[12];
    int inContact;
    char pad_0x264[4];
    int w_0x268;
    char pad_0x26c[8];
    float w_0x274;
    char pad_0x278[4];
    float w_0x27c;
    float w_0x280;
    char pad_0x284[4];
    float w_0x288;
    char pad_0x28c[8];
    float w_0x294;
    char pad_0x298[4];
    float w_0x29c;
    char pad_0x2a0[4];
    float w_0x2a4;
    BikeWheelQ* w_0x2a8;
    BikeQ* w_0x2ac;
    BikeQ* w_0x2b0;
    char pad_0x2b4[4];
    float w_0x2b8;
    float w_0x2bc;
    void UpdateDriveShare(void* owner);   // 0x00513f90 (Tire.h name: drive share from the vehicle speed)
};

// Query object at BikeWheel+0x2a8.
struct BikeWheelQ {
    // 0x004D31B0: per-frame update fed with the wheel vectors (purpose unknown).
    void Fn_004D31B0(float a, Vec3* b, float c, bool d, float e, Vec3* f, float* g);
};

struct BikeA604 {
    char pad_0x000[52];
    BikeA34* a_0x34;
    BikeA38* a_0x38;
    char pad_0x03c[8];
    int a_0x44;
    char pad_0x048[100];
    int a_0xac;
    char pad_0x0b0[40];
    int a_0xd8;
    Vec3 a_0xdc;
    char pad_0x0e8[52];
    void* a_0x11c;           // receives SoultreePhysicsBaseObject::field_0x124
    BikeA604(int a);
    char pad_0x120[0x40c];
    int a_0x52c;                  // KrustyBike slot 33 (0x0048e130) sets it to 1
    char pad_0x530[0xc];          // allocation size 0x53c (Bike slot 97)
    // Called through Bike+0x604 by KrustyBike slots 27 and 67 (purpose unknown).
    void Method_0x00532220(int a);
    void Method_0x00532310();
    void Method_0x005327c0();
    int Method_0x00530190(void* a, BikeA5C4* b, const char* name);
    void Method_0x00532900(Vec3 offset, int bone);
};

struct BikeA34 {
    char pad_0x000[416];
    BikeA1A0* r_0x1a0;
};

struct BikeA5C4 {
    char pad_0x000[12];
    int c_0xc;
    char pad_0x010[400];
    BikeA1A0* c_0x1a0;
    void Method_0x004a8c50(int a, int b, float c, float d);
    // The rider is a D3DIMSoultreeCharacter (the loader builds it with 0x004455b0): Character's
    // FindMotion 0x004a6b30 and Method_0x004a8bf0 seen through this view.
    int FindMotion(const char* name, int a);
    void Method_0x004a8bf0(int a, float b);
};

struct BikeA1A0 {
    char pad_0x000[320];
    SoultreeObject* d_0x140;           // SoultreeObject::firstChild
    int FindByName(const char* name);     // 0x004fdae0, SoultreeObject::FindByName (soultree.cpp)
    void Method_0x004444e0();
    void Fn_4444c0(int a);                // 0x004444c0 (SoultreeD3DNode::Fn_4444c0)
    void GetSubtreeBounds(Vec3* center, Vec3* extents);   // 0x004fe850 (SoultreeObject::GetSubtreeBounds)
    void Method_0x004fb8c0(int a, Matrix4* b);
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
    float q_0x94;
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
extern BikeGlobal_0056e26c* g_Bike_0056e26c;    // pointer read at 0x0056e26c (frame delta / time step; provisional)
extern char g_BikeString_00577738[];
extern Vec3 g_BikeVec3_005778a8;
extern Vec3 g_BikeVec3_005778c8;

class Bike : public Vehicle {
public:
    explicit Bike(int flags);       // 0x00407700, ret 8 = flags + hidden most-derived flag
    virtual ~Bike();              // vbase vtable 0x0055085c slot 0 -> 0x0040ca40

    // overrides of Vehicle slots (signatures: SoultreePhysicsBaseObject.h, Vehicle.h)
    virtual void UnknownVirtualSlot1(float value);
    virtual int UnknownVirtualSlot5(int arg);
    virtual void UnknownVirtualSlot7(const Vec3* dir);
    virtual void UnknownVirtualSlot8();
    virtual Vec3 UnknownVirtualSlot16(const Vec3* v);
    virtual void UnknownVirtualSlot29(int arg);
    virtual float UnknownVirtualSlot32();
    virtual int UnknownVirtualSlot33(const Vec3* a, const Vec3* b, const Vec3* c,
                                     const Vec3* d, int e, float f);
    virtual void UnknownVirtualSlot38(int a, int b, void* c);
    virtual int UnknownVirtualSlot39(float dt);
    virtual void UnknownVirtualSlot41();
    virtual int UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot44();
    virtual Vec3* UnknownVirtualSlot46(Vec3* out, float t);
    virtual float UnknownVirtualSlot47(float arg);
    virtual void UnknownVirtualSlot49(float arg);
    virtual float UnknownVirtualSlot53();
    virtual Vec3* UnknownVirtualSlot54(Vec3* out);
    virtual Vec3* UnknownVirtualSlot55(Vec3* out, Vec3* pos);
    virtual void UnknownVirtualSlot56(Vec3* a, int b, Vec3* c);
    virtual float UnknownVirtualSlot57();
    virtual float UnknownVirtualSlot59();
    virtual float UnknownVirtualSlot61(float arg);
    virtual int UnknownVirtualSlot62(float arg);
    virtual void UnknownVirtualSlot65(float arg);
    virtual int UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual void UnknownVirtualSlot71(int arg);
    virtual void UnknownVirtualSlot72(Vec3* out, VehicleWheel* wheel);
    virtual float UnknownVirtualSlot73(const Vec3* a, const Vec3* b);
    virtual float UnknownVirtualSlot75();
    virtual Vec3 UnknownVirtualSlot76(const Vec3* a, const Vec3* b);
    virtual int UnknownVirtualSlot89(float arg);
    virtual void UnknownVirtualSlot90(int* flag, float arg);
    virtual void UnknownVirtualSlot91();
    virtual void UnknownVirtualSlot92(const Vec3* a, const Vec3* b);
    virtual void UnknownVirtualSlot96();
    
    // slots introduced by Bike
    virtual void UnknownVirtualSlot97();
    virtual int UnknownVirtualSlot98();
    virtual int UnknownVirtualSlot99(float a, float b, int c, float d);
    virtual int UnknownVirtualSlot100(float a, int b, float c);
    virtual void UnknownVirtualSlot101();
    virtual void UnknownVirtualSlot102(float arg);
    
    // 0x004079c0 (6745 bytes, `ret 0xa0` = 40 argument dwords): the bike loader.  Runs
    // Vehicle::LoadVehicle, builds the rider character, the handlebar part, the two wheels
    // with their shocks, the three contact points, the particle emitters and the pose handles.
    // Argument names are tier 3 except where a callee's parameter fixes them; a19 is unused.
    GameObject* LoadBike(int a1, const char* engineName, const char* a3, const char* riderModel,
                         const SoultreeLoadDesc* desc, const BikeA5C8* info, Vec3 position,
                         Vec3 forward, Vec3 up, void* a9, VehicleInputMap* map,
                         const char* name, int a19, int defaultEngine, int* torqueTable,
                         int rpmLow, int rpmHigh, int rpmStep, float frontSpring, float frontDamper,
                         float rearSpring, float rearDamper, float frontRatio, float rearRatio,
                         int a31, int a32, void* a10, void* device, VehicleAxis* steer,
                         VehicleAxis* lean, VehicleAxis* throttle, SoultreeSlot1f0* track,
                         float a11, int a21);

    // 0x0040a520 (2272 bytes): integrates a steering torque from the angular velocity, the
    // front wheel's contact and the two +0x5f8/+0x5fc contacts, turns it by the steer state
    // and returns the normalised projection (0 when nothing drives it).
    float Method_0x0040a520();

    // Inherited members whose canonical types are still generic are viewed through
    // casts at the use site (MIGRATION.md rule 6): field_0x12c as BikeElem**, field_0x1f4
    // as BikeA1F4*.  Inline accessor
    // functions were tried and cost the exact match of slot 8 (0x00406750): VC6 then
    // loads field_0x42c before the float arguments instead of after them.

    int field_0x5bc;              // first Bike field (Vehicle's own data ends at 0x5bc)
    int field_0x5c0;
    BikeA5C4* riderCharacter;  // +0x5c4 slot 8/89 (0x406ae0): calls Method_0x004a8c50(poseA, poseB, poseParam, w) on it and riderCharacter->c_0x1a0 scene node; slot 97 passes it with "rider.col" to the 0x604 object
    BikeA5C8 field_0x5c8;         // +0x5c8 copy of the loader's info block (when given)
    char riderName[16];           // +0x5e0 ctor strcpy from global 0x00577738 (empty string in the image); tier 3 name
    BikeWheel* frontWheel;       // +0x5f0 front wheel (largest config z, see 0x4079c0 loop at 0x408705); tier 3
    BikeWheel* rearWheel;       // +0x5f4 rear wheel (smallest config z, 0x408728); tier 3
    BikeA5F8* field_0x5f8;         // +0x5f8 cleared in the ctor (0x00407700); Method_0x0040a520 reads it
    BikeA5F8* field_0x5fc;         // +0x5fc cleared in the ctor
    int field_0x600;               // +0x600 cleared in the ctor
    BikeA604* field_0x604;
    Vehicle* linkedVehicle;  // +0x608 cleared in ctor and reset (lines near linkedVehicle = 0), set reciprocally in the slot 38 pairing code: linkedVehicle = other; ((Bike*)other)->linkedVehicle = this
    BikeA60C* field_0x60c;         // +0x60c cleared in the ctor; ~Bike calls 0x00464e90 on it, then operator delete
    Vec3 field_0x610;              // per-axis gains of the steering torque integrator (Method_0x0040a520)
    Vec3 field_0x61c;              // integrated steering torque; decays by 0.99 (or -0.02/-0.1 at the limit) per step
    float field_0x628;
    float field_0x62c;
    int field_0x630;
    int field_0x634;
    int field_0x638;
    float field_0x63c;
    BikeA640* steerAxis;  // +0x640 slot 41 region: steer = (steerAxis->l_0x0 - 0.5f) * 4.0f; slot 89 maps (0.75 - l_0x0)*3.99 to a pose index
    BikeA644* poseSmoother;  // +0x644 slot 1 and slot 89: object with value/rate/ratio/limits that is advanced toward field_0x504 each frame with rate limiting (see BikeA644)
    float leanPoseMin;  // +0x648 slot 89: lean > leanPoseMin selects pose; poseLeanBlend = (lean - leanPoseMin) / (leanPoseMax - leanPoseMin)
    float leanPoseMax;  // +0x64c slot 89: upper lean bound; divisor (leanPoseMax - leanPoseMin) of the pose lean blend
    int poseIndex;  // +0x650 slot 89/61: current pose index (set to 10/11+idx, used to index riderPoseHandles[] and bikePoseHandles[], clamped to 13)
    float poseBlend;  // +0x654 slot 89: fractional part of the pose parameter ((0.75-l)*3.99 - idx); slot 61 weight w = 1 - poseBlend
    int poseState;  // +0x658 slot 89/61: paired with poseIndex (10 = lean pose); compared against 10 in slot 61 and set to idx*2+4
    float poseParam;  // +0x65c slot 61: passed as time/weight arg to Method_0x004a8c50 and Method_0x004a8bf0; slot 89 state machine writes it from the smoothed value (x = t * -0.25)
    float poseLeanBlend;  // +0x660 slot 89: (lean - min)/(max - min) pose lean blend; slot 61: (speed*0.0310559+1)*0.5
    float field_0x664;                // retail does fld/fcomp on it (KrustyBike request)
    int field_0x668;
    int riderPoseHandles[18];  // +0x66c slot 61: riderPoseHandles[idx], riderPoseHandles[next] passed to riderCharacter->Method_0x004a8c50 (rider-side pose ids, 18 entries)
    int bikePoseHandles[18];  // +0x6b4 slot 61: bikePoseHandles[idx], [next] passed to D3DIMSoultreeCharacter::Method_0x004a8c50 (bike-side pose ids, 18 entries)
    int field_0x6fc;
    int field_0x700;
    float field_0x704;
    float sideLieThreshold;  // +0x708 slot 38 tail: compared with |dot(g_BikeVec3_005778c8, field_0xa0)|; if smaller 'bike is lying on its side' branch
    float field_0x70c;             // +0x70c loader: 0.85f
    float field_0x710;
    float wobbleTime;  // +0x714 slot 91 wobble: wobbleTime += field_0x13c each frame, cubed (t*t*t*0.3578) to cap amplitude, halved on sign flip, zeroed by slot 71
    float wobbleSign;  // +0x718 slot 91 wobble: sign of field_0x2c; compared with new sign to detect flip; set in slot 71 from sign of field_0x48
    float wobbleOffset;  // +0x71c slot 91 wobble: added into field_0xd8.z, decays *0.9 per frame, reloaded from field_0xd8.z on flip/slot 71
    int wobbleStage;  // +0x720 slot 91 wobble: cycles 1..2 on each sign flip, loop active only while 1..2; slot 71 sets to 1
    float field_0x724;
    BikeDustEmitter* field_0x728;        // +0x728 cleared in the ctor; the loader's dust emitter (attachment type 1 on the rear wheel)
    BikeDirtChunkEmitter* field_0x72c;   // +0x72c cleared in the ctor; dirt chunks (attachment type 2 on the rear wheel)
    BikeSteamEmitter* field_0x730;       // +0x730 cleared in the ctor; steam (attachment type 4 at (0, 2.5, -2.75)); own data ends at 0x734, the compiler places the vtordisp there
};

typedef char bike_assert_sizeof[(sizeof(Bike) == 0x738 + 0x2c) ? 1 : -1];

#endif
