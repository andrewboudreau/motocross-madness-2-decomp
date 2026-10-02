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

#include "../vehicle/Vehicle.h"
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

// Objects reached through Bike fields.  Layout is only known where accessed.
struct BikeWheel {
    char pad_0x000[204];
    Vec3 wheelPosition;
    char pad_0x0d8[12];
    Vec3 groundNormal;
    char pad_0x0f0[12];
    Vec3 appliedShare;
    Vec3 w_0x108;
    char pad_0x114[48];
    float contactLoad;
    float w_0x148;
    char pad_0x14c[4];
    float w_0x150;
    char pad_0x154[4];
    float loadWeight;
    char pad_0x15c[212];
    Vec3 w_0x230;
    Vec3 w_0x23c;
    Vec3 w_0x248;             // KrustyBike slot 48 (0x004964e0) zeroes it
    char pad_0x254[12];
    int inContact;
    char pad_0x264[4];
    int w_0x268;
    char pad_0x26c[16];
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
    void Fn_00513F90(void* owner);   // 0x00513F90 (KrustyBike; purpose unknown)
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
};

struct BikeA1A0 {
    char pad_0x000[320];
    SoultreeObject* d_0x140;           // SoultreeObject::firstChild
    int Method_0x004fdae0(const char* name);
    void Method_0x004444e0();
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

struct BikeA640 {
    float steerValue;
    float l_0x4;
    float l_0x8;
};

struct BikeA644 {
    int smoothedValue;
    float smoothTime;
    float smoothRatio;
    float maxRise;
    float maxFall;
};

struct BikeQ {
    char pad_0x000[148];
    float q_0x94;
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
extern Vec3 g_BikeVec3_005778a8;
extern Vec3 g_BikeVec3_005778c8;

class Bike : public Vehicle {
public:
    Bike();
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
    
    float Method_0x0040a520();

    // Inherited members whose canonical types are still generic are viewed through
    // casts at the use site (MIGRATION.md rule 6): field_0x12c as BikeElem**, field_0x1f4
    // as BikeA1F4*.  Inline accessor
    // functions were tried and cost the exact match of slot 8 (0x00406750): VC6 then
    // loads field_0x42c before the float arguments instead of after them.

    int field_0x5bc;              // first Bike field (Vehicle's own data ends at 0x5bc)
    int field_0x5c0;
    BikeA5C4* riderCharacter;  // +0x5c4 slot 8/89 (0x406ae0): calls Method_0x004a8c50(poseA, poseB, poseParam, w) on it and riderCharacter->c_0x1a0 scene node; slot 97 passes it with "rider.col" to the 0x604 object
    char pad_0x5c8[40];
    BikeWheel* frontWheel;       // +0x5f0 front wheel (largest config z, see 0x4079c0 loop at 0x408705); tier 3
    BikeWheel* rearWheel;       // +0x5f4 rear wheel (smallest config z, 0x408728); tier 3
    char pad_0x5f8[12];
    BikeA604* field_0x604;
    Vehicle* linkedVehicle;  // +0x608 cleared in ctor and reset (lines near linkedVehicle = 0), set reciprocally in the slot 38 pairing code: linkedVehicle = other; ((Bike*)other)->linkedVehicle = this
    char pad_0x60c[16];
    Vec3 field_0x61c;
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
    char pad_0x70c[4];
    float field_0x710;
    float wobbleTime;  // +0x714 slot 91 wobble: wobbleTime += field_0x13c each frame, cubed (t*t*t*0.3578) to cap amplitude, halved on sign flip, zeroed by slot 71
    float wobbleSign;  // +0x718 slot 91 wobble: sign of field_0x2c; compared with new sign to detect flip; set in slot 71 from sign of field_0x48
    float wobbleOffset;  // +0x71c slot 91 wobble: added into field_0xd8.z, decays *0.9 per frame, reloaded from field_0xd8.z on flip/slot 71
    int wobbleStage;  // +0x720 slot 91 wobble: cycles 1..2 on each sign flip, loop active only while 1..2; slot 71 sets to 1
    float field_0x724;
    char pad_0x728[12];           // own data ends at 0x734; the compiler places the vtordisp there
};

typedef char bike_assert_sizeof[(sizeof(Bike) == 0x738 + 0x2c) ? 1 : -1];

#endif
