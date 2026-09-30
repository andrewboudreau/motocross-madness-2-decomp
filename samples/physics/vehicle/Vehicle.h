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
//   * VehVec3 is the shared Math3D Vec3.
#ifndef MCM2_PHYSICS_VEHICLE_H
#define MCM2_PHYSICS_VEHICLE_H

#include <stddef.h>
#include "../hierarchy/SoultreePhysicsCharacter.h"

#define VEH_CHECK_OFFSET(cls, member, off) \
    typedef char veh_assert_##member##_##off[(offsetof(cls, member) == (off)) ? 1 : -1]

typedef Vec3 VehVec3;
struct VehBlock7 { float f[7]; };   // 28-byte state block (pose/orientation values, provisional)

// Global vector at 0x0068a6e8 (three floats, all zero at rest): copied whole into
// vector members when they are reset.
extern VehVec3 g_VehZeroVec3;

// 0x00460b50: table-driven sqrt (returns 0 for 0); 0x00460c00: table-driven 1/sqrt.
extern float __cdecl VehFastSqrt(float x);
extern float __cdecl VehFastInvSqrt(float x);

// ---- provisional collaborator types (layout only what the code touches) ----

// Object at Vehicle+0x3bc: transform/matrix helper. Methods are non-virtual thiscall
// (callee pops); names are the retail addresses.
struct VehicleXform {
    void  Method_004FC540(int a, VehVec3* b, VehVec3* c);
    void  Method_004FC050(int a, VehVec3* b, VehVec3* c, int d, int e);
    void  Method_004FC630(float x, float y, float z);
    void  Method_004FC660(VehVec3* v);
    void  Method_004FC970(VehVec3* v);
    void  Method_004FC9A0(int a, VehVec3* v);
    VehVec3* Method_004FD5C0(VehVec3* out, VehVec3* in);
    VehVec3* Method_004FD710(VehVec3* out, VehVec3* in);
    void  Method_004FD1F0(VehVec3 a, VehVec3 b, float c);
    void  Method_004444E0();
};
// Free helpers (cdecl) used by the vehicle code. Retail addresses; names are tier 3.
extern float    __cdecl VehDot(const VehVec3* a, const VehVec3* b);                 // 0x0040ae30 dot product
extern VehVec3* __cdecl VehScaleVec(VehVec3* out, const VehVec3* v, float k);       // 0x005015b0 out = v * k
extern VehVec3* __cdecl VehSubVec(VehVec3* out, const VehVec3* a, const VehVec3* b);// 0x00421d00 out = a - b
extern VehVec3* __cdecl VehNormalize(VehVec3* out, const VehVec3* v);              // 0x005087b0 out = v / |v|
// 0x004b5a60: builds seven scalars from two basis vectors (orientation block); provisional.
// 0x005004a0: 17-argument cdecl helper used by slot 4 (provisional; arg roles unknown).
// Same function as SoultreePhysicsCallees.h Fn_5004a0; parameter types follow slot 4.
extern void __cdecl VehSlot4Helper(float a, const VehVec3* b, float c, VehicleXform* xf, VehVec3* p,
                                   const VehVec3* q, VehVec3* normal, VehVec3* out, VehVec3* d,
                                   int e, int f, VehVec3* g, const VehVec3* h, VehVec3* i,
                                   VehVec3* j, int k, float* l);
struct VehicleContact;
// 0x0043ad80 / 0x0043aa30 / 0x0043aff0: contact-array helpers used by slot 49 (provisional).
extern int  __cdecl VehContactsA(int a, int* count, int n, VehicleContact** arr, int b, VehVec3* pos, int c, float d);
extern void __cdecl VehContactsB(int n, VehicleContact** arr, VehVec3* scale, VehVec3* p, VehVec3* v,
                                 VehVec3* pos18, VehVec3* pos0c, VehVec3* o1, VehVec3* o2, VehVec3* o3);
extern void __cdecl VehContactsC(int n, VehicleContact** arr);
extern void __cdecl VehBasisToBlock(VehVec3 a, VehVec3 b, float* p0, float* p1, float* p2,
                                    float* p3, float* p4, float* p5, float* p6);

// Object at Vehicle+0x468: control/input map with two virtual queries.
struct VehicleValueSource {          // object at VehicleInputMap+0x0c (provisional)
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual int  UnknownVirtualSlot3(int a, float* out);
};
struct VehicleInputMap {
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual int  UnknownVirtualSlot2(int a, int b);
    virtual int  UnknownVirtualSlot3(int a, int b, int c, int d);
    char pad_0x04[0x8];
    VehicleValueSource* field_0x0c;
    char pad_0x10[0x24];
    void* field_0x34;
};

struct VehicleContact {              // elements of Vehicle+0x12c
    char pad_0x00[4];
    int  field_0x04;
    char pad_0x08[0xC];
    VehVec3 field_0x14;              // contact position (distance source in slot 7)
    VehVec3 field_0x20;              // second contact point (impact position in slot 18)
    char pad_0x2C[0x18];
    VehVec3 field_0x44;              // per-contact share of the applied vector
    char pad_0x50[0x50];
    float field_0xa0;                // blend weight
    int  field_0xa4;                 // contact active flag
    int  field_0xa8;                 // impact-handled flag (slot 18)
    char pad_0xAC[0x10];
    unsigned char field_0xbc;        // surface material id
};
// Table object at Vehicle+0x1f0: field_0xa4 points at a byte table (+0x400) indexed by material id.
struct VehicleMaterialSet {
    char pad_0x00[0xA4];
    char* field_0xa4;
};
// Sink at VehicleImpactEvent+0x04 (provisional): fields 0x44/0x50/0x60 are touched by slot 18.
struct VehicleImpactSink {
    char pad_0x00[0x44];
    VehVec3 field_0x44;
    VehVec3 field_0x50;
    char pad_0x5C[4];
    int field_0x60;
    char pad_0x64[0x10];
    VehVec3 field_0x74;
    void Method_004B8D90(VehVec3 pos, float intensity);   // 0x004b8d90, purpose unknown (impact/sound post)
    void Method_004B9DC0(VehVec3 pos);                    // 0x004b9dc0, purpose unknown (impact/sound post)
};
struct VehicleImpactEvent {          // argument of slots 18..20 (provisional)
    char pad_0x00[4];
    VehicleImpactSink* field_0x04;
    VehicleImpactSink* field_0x08;
    VehicleImpactSink* field_0x0c;
    char pad_0x10[0x14];
    char field_0x24;
};
// Object at Vehicle+0x480 (provisional): lean/speed state; field_0x00 is read by slots 42/57 as a float.
struct VehicleSpeedState {
    float field_0x00;
    char pad_0x04[4];
    int field_0x08;
    float field_0x0c;                // countdown timer
    void Method_004D2F50(float dt, int a, int b);   // 0x004d2f50, purpose unknown
    void Method_004D3030(int a, float speed);       // 0x004d3030, purpose unknown
};
// Exponential smoother objects at Vehicle+0x58c / +0x598 (provisional): f0 is the smoothed value.
struct VehicleSmoother {
    float field_0x00;
    float field_0x04;                // time constant / cap
    float field_0x08;                // last blend factor
};
struct VehicleContactSet;            // object at SoultreePhysicsCharacter+0x128
// Object at Vehicle+0x47c (provisional layout: only what Vehicle touches).
struct VehicleSteerState {
    VehicleXform* field_0x00;
    float field_0x04;
    float field_0x08;
    void Method_00504EC0(float value, SoultreeNode* node);   // 0x00504ec0, purpose unknown
};
struct VehicleAxis;                  // objects at Vehicle+0x4f8/0x4fc/0x500
struct VehicleCamera;                // object at Vehicle+0x5ac
class Vehicle;
struct VehicleWheelAux {             // object at VehicleWheel+0x2a8 (provisional)
    void Method_004D31B0(float a, VehVec3* b, float c, int d, float e, VehVec3* f, VehVec3* g);
};
// Elements of Vehicle+0x53c (provisional: only touched offsets are named).
struct VehicleWheel {
    char pad_0x00[0xCC];
    VehVec3 field_0xcc;              // wheel contact position (slot 7 distance source)
    VehVec3 field_0xd8;              // (y at +0xdc is read as a height by slot 58)
    VehVec3 field_0xe4;              // contact normal (averaged by Method_00528400)
    char pad_0xF0[0xC];
    VehVec3 field_0xfc;              // per-wheel share of the applied vector
    char pad_0x108[0x3C];
    float field_0x144;
    char pad_0x148[8];
    float field_0x150;
    char pad_0x154[4];
    float field_0x158;               // blend weight
    char pad_0x15C[4];
    int field_0x160;                 // impact-handled flag (slot 18)
    int field_0x164;                 // impact-handled flag (slot 19)
    int field_0x168;                 // impact-handled flag (slot 20)
    char pad_0x16C[8];
    unsigned char field_0x174;       // surface material id
    char pad_0x175[0x4B];
    int field_0x1c0;
    char pad_0x1C4[0x64];
    float field_0x228;
    char pad_0x22C[4];
    VehVec3 field_0x230;
    VehVec3 field_0x23c;
    VehVec3 field_0x248;
    char pad_0x254[0xC];
    int field_0x260;
    char pad_0x264[0x1C];
    VehVec3 field_0x280;
    float field_0x28c;
    float field_0x290;
    float field_0x294;
    char pad_0x298[0x10];
    VehicleWheelAux* field_0x2a8;
    char pad_0x2AC[0xC];
    float field_0x2b8;
    float field_0x2bc;
    void Method_005143D0(float a, float b, int c, int d, int e, float f);   // 0x005143d0
    void Method_00513F90(Vehicle* owner);                                 // 0x00513f90
    void Method_00515660();                    // 0x00515660, called per wheel by Bike slot 41 (0x0040cbd0)
};

class Vehicle : public SoultreePhysicsCharacter {
public:
    Vehicle();
    virtual ~Vehicle();                          // vbase deleting dtor 0x0052b630
    virtual int GameObjectVirtualSlot10(float dt);  // 0x0052a830 via vtordisp thunk 0x0040cab0

    // ---- overrides of inherited slots (order as in the vtable) ----
    virtual void UnknownVirtualSlot0(float dt);
    virtual void UnknownVirtualSlot1(float value);                               // 0x00525550
    virtual void UnknownVirtualSlot3(const VehVec3* a, const VehVec3* b, const VehVec3* c,
                                     const VehVec3* d, int e, int f, float* g); // 0x005264c0
    virtual void UnknownVirtualSlot4(const VehVec3* a0, VehVec3* a1, const VehVec3* a2,
                                     const VehVec3* a3, float a4, int a5, int a6,
                                     const VehVec3* a7, const VehVec3* a8, const VehVec3* a9,
                                     VehVec3* a10, VehVec3* a11, int a12, float* a13, float a14);
    virtual void UnknownVirtualSlot7(const VehVec3* arg);
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
    virtual int  UnknownVirtualSlot33(const VehVec3* a, const VehVec3* b, const VehVec3* c,
                                      const VehVec3* d, int e, float f);   // 0x00527430
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
    virtual VehVec3* UnknownVirtualSlot46(VehVec3* out, float arg);
    // pure virtual: entry is _purecall (0x00534cfe).  Bike's override 0x0040cbb0 is
    // `ret 4` and returns a float (tier 1 via its byte match), hence float(float).
    virtual float UnknownVirtualSlot47(float arg) = 0;
    virtual void UnknownVirtualSlot48();
    // 0x0052a940 forwards its argument unchanged to slots 9 (float dt), 39, 64 and 65,
    // so it and they take the float time step (tier 2).
    virtual void UnknownVirtualSlot49(float dt);
    virtual void UnknownVirtualSlot50(int a, int b, int c);
    virtual int UnknownVirtualSlot51();
    virtual int UnknownVirtualSlot52();
    virtual float UnknownVirtualSlot53();
    virtual VehVec3* UnknownVirtualSlot54(VehVec3* out);
    virtual VehVec3* UnknownVirtualSlot55(VehVec3* unused, VehVec3* out);
    virtual void UnknownVirtualSlot56(VehVec3* a, int b, VehVec3* c);
    virtual float UnknownVirtualSlot57();
    virtual void UnknownVirtualSlot58(VehVec3* a, int b);
    virtual float UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60(float a, float b, int c);
    virtual float UnknownVirtualSlot61(float arg);
    virtual int UnknownVirtualSlot62(float arg);
    virtual int UnknownVirtualSlot63(float dt);
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
    virtual void UnknownVirtualSlot72(VehVec3* out, VehicleWheel* wheel);
    virtual float UnknownVirtualSlot73(const VehVec3* a, const VehVec3* b);  // only read (Bike 0x00406090)
    virtual float UnknownVirtualSlot74(VehVec3* point, VehVec3* dir, float c, float d);
    virtual float UnknownVirtualSlot75();
    // ret 0xc = hidden result pointer + two arguments; KrustyBike 0x00491ca0 byte-matches
    // as a by-value Vec3 return (tier 1 for the ABI shape, tier 2 for the Vec3 pointers).
    virtual VehVec3 UnknownVirtualSlot76(const VehVec3* a, const VehVec3* b);
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
    virtual void UnknownVirtualSlot92(const VehVec3* a, const VehVec3* b);
    virtual void UnknownVirtualSlot93();
    virtual void UnknownVirtualSlot94();  // entry is the shared empty stub 0x00464e90
    virtual void UnknownVirtualSlot95();
    virtual void UnknownVirtualSlot96();

    // non-virtual helper (retail 0x00526830, next to slot 70; tier 3 name)
    int Method_00526830();
    // 0x00528400: normalized average of the wheels' contact normals (+0xe4); returns out.
    VehVec3* Method_00528400(VehVec3* out);
    int   Method_00529280();                                     // 0x00529280
    void  Method_00528EB0();                                     // 0x00528eb0
    void  Method_00529A20();                                     // 0x00529a20
    void  Method_00527A20(float* speed, VehVec3* zero, VehVec3* up);   // 0x00527a20
    void  Method_005293E0(float* speed);                         // 0x005293e0
    void  Method_00529450(VehVec3* up, VehVec3* zero);           // 0x00529450
    void  Method_00529C20(VehVec3* up, VehVec3* zero, float d);  // 0x00529c20
    // 0x00525c60: calls virtual slot 0 of every object in the two owned arrays (+0x554/+0x55c, +0x560/+0x568).
    void Method_00525C60();

    // Inherited members whose canonical types are still separate classes are viewed
    // through casts at the use site (MIGRATION.md rule 6): d3d_field_0x1a0 and
    // field_0x218 as VehicleXform*, field_0x12c as VehicleContact**, field_0x128 as
    // VehicleContactSet*, field_0x1f0 as VehicleMaterialSet*.  Inline accessor functions
    // were tried; VC6 schedules their loads differently (slots 4, 19, 35 and 49 lost
    // bytes, Bike slot 8 lost its exact match), so the casts are written out.

    float field_0x434;
    float field_0x438;        // read by Bike slot 59 (0x0040a330) as a float divisor
    float field_0x43c;
    int field_0x440;
    int field_0x444;
    int field_0x448;          // Bike slot 99 (0x00409b30) stores small codes (1..5)
    float field_0x44c;        // Bike slot 99 (0x00409b30) multiplies it (float)
    float field_0x450;
    // 0x454/0x458: Bike slot 39 (0x0040a1f0) decrements 0x454 by the frame time and
    // compares it with 0.0f/-2.0f, so both are floats (tier 2; slot 69 copies 0x458).
    float field_0x454;
    float field_0x458;
    float field_0x45c;        // Bike slot 100 (0x0040a090) stores field_0x50 here
    int field_0x460;
    int field_0x464;
    VehicleInputMap* field_0x468;
    void* field_0x46c;
    float field_0x470;
    float field_0x474;
    bool field_0x478;
    bool field_0x479;
    bool field_0x47a;
    char pad_0x47B[0x1];
    VehicleSteerState* field_0x47c;
    VehicleSpeedState* field_0x480;
    int field_0x484;
    VehVec3 field_0x488;
    VehVec3 field_0x494;
    char pad_0x4A0[0x4];
    float field_0x4a4;
    int field_0x4a8;
    float field_0x4ac;
    float field_0x4b0;
    float field_0x4b4;
    float field_0x4b8;
    float field_0x4bc;
    VehVec3 field_0x4c0;
    VehVec3 field_0x4cc;
    float field_0x4d8;
    float field_0x4dc;
    float field_0x4e0;
    float field_0x4e4;        // Bike slot 92 (0x0040ba30): _finite()/fabs on it (float, tier 2)
    float field_0x4e8;
    float field_0x4ec;
    int field_0x4f0;
    int field_0x4f4;
    VehicleAxis* field_0x4f8;
    VehicleAxis* field_0x4fc;
    VehicleAxis* field_0x500;
    VehVec3 field_0x504;
    VehVec3 field_0x510;
    float field_0x51c;
    int field_0x520;
    float field_0x524[6];
    VehicleWheel** field_0x53c;
    char pad_0x540[0x4];
    int field_0x544;
    VehicleWheel* field_0x548;
    VehicleWheel* field_0x54c;
    char pad_0x550[0x20];
    int field_0x570;
    VehVec3 field_0x574;      // Bike slot 100 (0x0040a090) builds (a0.x, 0, a0.z) here
    float field_0x580;
    float field_0x584;
    float field_0x588;
    VehicleSmoother* field_0x58c;
    float field_0x590;
    float field_0x594;
    VehicleSmoother* field_0x598;
    int field_0x59c;
    int field_0x5a0;
    int field_0x5a4;
    int field_0x5a8;
    VehicleCamera* field_0x5ac;
    float field_0x5b0;
    int field_0x5b4;
    char pad_0x5B8[0x4];      // own data ends at 0x5bc; the compiler places the vtordisp there
};

VEH_CHECK_OFFSET(Vehicle, field_0x434, 0x434);
VEH_CHECK_OFFSET(Vehicle, field_0x43c, 0x43C);
VEH_CHECK_OFFSET(Vehicle, field_0x440, 0x440);
VEH_CHECK_OFFSET(Vehicle, field_0x444, 0x444);
VEH_CHECK_OFFSET(Vehicle, field_0x450, 0x450);
VEH_CHECK_OFFSET(Vehicle, field_0x454, 0x454);
VEH_CHECK_OFFSET(Vehicle, field_0x458, 0x458);
VEH_CHECK_OFFSET(Vehicle, field_0x468, 0x468);
VEH_CHECK_OFFSET(Vehicle, field_0x46c, 0x46C);
VEH_CHECK_OFFSET(Vehicle, field_0x470, 0x470);
VEH_CHECK_OFFSET(Vehicle, field_0x474, 0x474);
VEH_CHECK_OFFSET(Vehicle, field_0x478, 0x478);
VEH_CHECK_OFFSET(Vehicle, field_0x479, 0x479);
VEH_CHECK_OFFSET(Vehicle, field_0x47a, 0x47A);
VEH_CHECK_OFFSET(Vehicle, field_0x47c, 0x47C);
VEH_CHECK_OFFSET(Vehicle, field_0x480, 0x480);
VEH_CHECK_OFFSET(Vehicle, field_0x484, 0x484);
VEH_CHECK_OFFSET(Vehicle, field_0x488, 0x488);
VEH_CHECK_OFFSET(Vehicle, field_0x494, 0x494);
VEH_CHECK_OFFSET(Vehicle, field_0x4a4, 0x4A4);
VEH_CHECK_OFFSET(Vehicle, field_0x4a8, 0x4A8);
VEH_CHECK_OFFSET(Vehicle, field_0x4ac, 0x4AC);
VEH_CHECK_OFFSET(Vehicle, field_0x4b0, 0x4B0);
VEH_CHECK_OFFSET(Vehicle, field_0x4b4, 0x4B4);
VEH_CHECK_OFFSET(Vehicle, field_0x4b8, 0x4B8);
VEH_CHECK_OFFSET(Vehicle, field_0x4bc, 0x4BC);
VEH_CHECK_OFFSET(Vehicle, field_0x4c0, 0x4C0);
VEH_CHECK_OFFSET(Vehicle, field_0x4cc, 0x4CC);
VEH_CHECK_OFFSET(Vehicle, field_0x4d8, 0x4D8);
VEH_CHECK_OFFSET(Vehicle, field_0x4dc, 0x4DC);
VEH_CHECK_OFFSET(Vehicle, field_0x4e0, 0x4E0);
VEH_CHECK_OFFSET(Vehicle, field_0x4e4, 0x4E4);
VEH_CHECK_OFFSET(Vehicle, field_0x4e8, 0x4E8);
VEH_CHECK_OFFSET(Vehicle, field_0x4ec, 0x4EC);
VEH_CHECK_OFFSET(Vehicle, field_0x4f0, 0x4F0);
VEH_CHECK_OFFSET(Vehicle, field_0x4f4, 0x4F4);
VEH_CHECK_OFFSET(Vehicle, field_0x4f8, 0x4F8);
VEH_CHECK_OFFSET(Vehicle, field_0x4fc, 0x4FC);
VEH_CHECK_OFFSET(Vehicle, field_0x500, 0x500);
VEH_CHECK_OFFSET(Vehicle, field_0x504, 0x504);
VEH_CHECK_OFFSET(Vehicle, field_0x510, 0x510);
VEH_CHECK_OFFSET(Vehicle, field_0x51c, 0x51C);
VEH_CHECK_OFFSET(Vehicle, field_0x520, 0x520);
VEH_CHECK_OFFSET(Vehicle, field_0x524, 0x524);
VEH_CHECK_OFFSET(Vehicle, field_0x53c, 0x53C);
VEH_CHECK_OFFSET(Vehicle, field_0x544, 0x544);
VEH_CHECK_OFFSET(Vehicle, field_0x548, 0x548);
VEH_CHECK_OFFSET(Vehicle, field_0x54c, 0x54C);
VEH_CHECK_OFFSET(Vehicle, field_0x570, 0x570);
VEH_CHECK_OFFSET(Vehicle, field_0x588, 0x588);
VEH_CHECK_OFFSET(Vehicle, field_0x58c, 0x58C);
VEH_CHECK_OFFSET(Vehicle, field_0x590, 0x590);
VEH_CHECK_OFFSET(Vehicle, field_0x594, 0x594);
VEH_CHECK_OFFSET(Vehicle, field_0x598, 0x598);
VEH_CHECK_OFFSET(Vehicle, field_0x59c, 0x59C);
VEH_CHECK_OFFSET(Vehicle, field_0x5a0, 0x5A0);
VEH_CHECK_OFFSET(Vehicle, field_0x5a4, 0x5A4);
VEH_CHECK_OFFSET(Vehicle, field_0x5a8, 0x5A8);
VEH_CHECK_OFFSET(Vehicle, field_0x5ac, 0x5AC);
VEH_CHECK_OFFSET(Vehicle, field_0x5b0, 0x5B0);
VEH_CHECK_OFFSET(Vehicle, field_0x5b4, 0x5B4);
typedef char veh_assert_sizeof[(sizeof(Vehicle) == 0x5ec) ? 1 : -1];

#endif
