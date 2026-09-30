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
// PROVISIONAL (until the owners of the real bases publish their headers):
//   * SoultreePhysicsCharacter is declared here as a plain polymorphic class with
//     placeholder virtuals for slots 0..42 and padding up to the start of Vehicle's
//     own members (0x430, tier 3 - the real boundary is not yet proven).
//   * The virtual GameObject base and the 540 secondary subobject are NOT modelled
//     as C++ bases; their bytes are padding. The offsets that matter to the byte
//     matches (this+0x4.. fields) are exact.
//   * VehVec3 is a stand-in for the shared vector type in common/Math3D.h.
#ifndef MCM2_PHYSICS_VEHICLE_H
#define MCM2_PHYSICS_VEHICLE_H

#include <stddef.h>

#define VEH_CHECK_OFFSET(cls, member, off) \
    typedef char veh_assert_##member##_##off[(offsetof(cls, member) == (off)) ? 1 : -1]

struct VehVec3 { float x, y, z; };

// Global vector at 0x0068a6e8 (three floats, all zero at rest): copied whole into
// vector members when they are reset.
extern VehVec3 g_VehZeroVec3;

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
    char pad_0x08[0x98];
    float field_0xa0;
    int  field_0xa4;
};
struct VehicleContactSet;            // object at SoultreePhysicsCharacter+0x128
// Object at Vehicle+0x47c (provisional layout: only what Vehicle touches).
struct VehicleSteerState {
    char pad_0x00[4];
    float field_0x04;
    float field_0x08;
    void Method_00504EC0(float value, int arg);   // 0x00504ec0, purpose unknown
};
struct VehicleAxis;                  // objects at Vehicle+0x4f8/0x4fc/0x500
struct VehicleCamera;                // object at Vehicle+0x5ac
class Vehicle;
struct VehicleWheelAux {             // object at VehicleWheel+0x2a8 (provisional)
    void Method_004D31B0(float a, VehVec3* b, float c, int d, float e, VehVec3* f, VehVec3* g);
};
// Elements of Vehicle+0x53c (provisional: only touched offsets are named).
struct VehicleWheel {
    char pad_0x00[0x230];
    VehVec3 field_0x230;
    char pad_0x23C[0xC];
    VehVec3 field_0x248;
    char pad_0x254[0xC];
    int field_0x260;
    char pad_0x264[0x1C];
    VehVec3 field_0x280;
    char pad_0x28C[0x1C];
    VehicleWheelAux* field_0x2a8;
    void Method_005143D0(float a, float b, int c, int d, int e, int f);   // 0x005143d0
    void Method_00513F90(Vehicle* owner);                                 // 0x00513f90
};

class SoultreePhysicsCharacter {
public:
    virtual void UnknownVirtualSlot0(float dt);
    virtual void UnknownVirtualSlot1(int arg);
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3(int a, void* b, void* c, int d, void* e, void* f, void* g);
    virtual void UnknownVirtualSlot4(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
                                     int a8, int a9, int a10, int a11, int a12, int a13, int a14);
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7(void* arg);
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16();
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18(void* arg);
    virtual void UnknownVirtualSlot19(void* arg);
    virtual void UnknownVirtualSlot20(void* arg);
    virtual void UnknownVirtualSlot21();
    virtual void UnknownVirtualSlot22();
    virtual int  UnknownVirtualSlot23();
    virtual int  UnknownVirtualSlot24();
    virtual int  UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
    virtual void UnknownVirtualSlot27();
    virtual int  UnknownVirtualSlot28(int arg);
    virtual void UnknownVirtualSlot29();
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual void UnknownVirtualSlot32();
    virtual int  UnknownVirtualSlot33(void* a, void* b, void* c, int d, int e, int f);
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35(int a, int b);
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot38(int a, int b, void* c);
    virtual int  UnknownVirtualSlot39(void* arg);
    virtual void UnknownVirtualSlot40();
    virtual void UnknownVirtualSlot41();
    virtual int  UnknownVirtualSlot42();

    int field_0x04;  // stand-in for the vbptr of the virtual GameObject base (provisional)
    char pad_0x008[0x4];
    VehVec3 field_0x0c;
    VehVec3 field_0x18;
    int field_0x24;
    char pad_0x028[0x4];
    float field_0x2c[7];  // current state block, copied to field_0x48 in slot 58
    float field_0x48[7];
    VehVec3 field_0x64;
    char pad_0x070[0xC];
    VehVec3 field_0x7c;
    VehVec3 field_0x88;
    VehVec3 field_0x94;
    VehVec3 field_0xa0;
    VehVec3 field_0xac;
    float field_0xb8;
    float field_0xbc;
    char pad_0x0C0[0xC];
    VehVec3 field_0xcc;
    VehVec3 field_0xd8;
    VehVec3 field_0xe4;
    VehVec3 field_0xf0;
    char pad_0x0FC[0xC];
    bool field_0x108;
    bool field_0x109;
    char pad_0x10A[0x2];
    VehVec3 field_0x10c;
    VehVec3 field_0x118;
    void* field_0x124;
    VehicleContactSet* field_0x128;
    VehicleContact** field_0x12c;
    int field_0x130;
    char pad_0x134[0x4];
    bool field_0x138;
    char pad_0x139[0x3];
    float field_0x13c;
    float field_0x140;
    char pad_0x144[0x8];
    int field_0x14c;
    float field_0x150;
    char pad_0x154[0x40];
    VehVec3 field_0x194;
    char pad_0x1A0[0xC];
    VehVec3 field_0x1ac;
    VehVec3 field_0x1b8;
    char pad_0x1C4[0x8];
    int field_0x1cc;
    bool field_0x1d0;
    char pad_0x1D1[0xF];
    float field_0x1e0;
    char pad_0x1E4[0xC];
    void* field_0x1f0;
    char pad_0x1F4[0x18];
    bool field_0x20c;
    bool field_0x20d;
    char pad_0x20E[0xA];
    void* field_0x218;
    char pad_0x21C[0x1A0];
    VehicleXform* field_0x3bc;
    char pad_0x3C0[0x6C];
    int field_0x42c;
};

class Vehicle : public SoultreePhysicsCharacter {
public:
    // ---- overrides of inherited slots (order as in the vtable) ----
    virtual void UnknownVirtualSlot0(float dt);
    virtual void UnknownVirtualSlot1(int arg);
    virtual void UnknownVirtualSlot3(int a, void* b, void* c, int d, void* e, void* f, void* g);
    virtual void UnknownVirtualSlot4(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
                                     int a8, int a9, int a10, int a11, int a12, int a13, int a14);
    virtual void UnknownVirtualSlot7(void* arg);
    virtual void UnknownVirtualSlot18(void* arg);
    virtual void UnknownVirtualSlot19(void* arg);
    virtual void UnknownVirtualSlot20(void* arg);
    virtual void UnknownVirtualSlot21();
    virtual int  UnknownVirtualSlot23();
    virtual int  UnknownVirtualSlot24();
    virtual int  UnknownVirtualSlot25();
    virtual int  UnknownVirtualSlot28(int arg);
    virtual int  UnknownVirtualSlot33(void* a, void* b, void* c, int d, int e, int f);
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35(int a, int b);
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot38(int a, int b, void* c);
    virtual int  UnknownVirtualSlot39(void* arg);
    virtual int  UnknownVirtualSlot42();

    // ---- slots introduced by Vehicle (43..96) ----
    virtual void UnknownVirtualSlot43();
    virtual void UnknownVirtualSlot44();
    virtual float UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot46(VehVec3* out, int arg);
    virtual void UnknownVirtualSlot47() = 0;  // pure virtual: entry is _purecall (0x00534cfe)
    virtual void UnknownVirtualSlot48();
    virtual void UnknownVirtualSlot49(int a, int b, int c);
    virtual void UnknownVirtualSlot50(int a, int b, int c);
    virtual int UnknownVirtualSlot51();
    virtual int UnknownVirtualSlot52();
    virtual float UnknownVirtualSlot53();
    virtual void UnknownVirtualSlot54(VehVec3* out);
    virtual void UnknownVirtualSlot55(int arg, VehVec3* out);
    virtual void UnknownVirtualSlot56(int a, void* b, void* c);
    virtual float UnknownVirtualSlot57();
    virtual void UnknownVirtualSlot58(void* a, int b);
    virtual float UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60(float a, float b, int c);
    virtual float UnknownVirtualSlot61(int arg);
    virtual int UnknownVirtualSlot62(int arg);
    virtual int UnknownVirtualSlot63(int arg);
    virtual int UnknownVirtualSlot64(int arg);
    virtual void UnknownVirtualSlot65();  // entry is the shared empty stub 0x00464e80
    virtual int UnknownVirtualSlot66() = 0;  // pure virtual: entry is _purecall (0x00534cfe)
    virtual void UnknownVirtualSlot67();
    virtual int UnknownVirtualSlot68(int* out);
    virtual void UnknownVirtualSlot69();
    virtual int UnknownVirtualSlot70(float arg);
    virtual void UnknownVirtualSlot71(int arg);
    virtual void UnknownVirtualSlot72(VehVec3* out, int arg);
    virtual float UnknownVirtualSlot73(int a, int b);
    virtual float UnknownVirtualSlot74(void* a, void* b, float c, float d);
    virtual float UnknownVirtualSlot75();
    virtual void UnknownVirtualSlot76(VehVec3* out, VehVec3* a, VehVec3* b);
    virtual int UnknownVirtualSlot77(int arg);
    virtual int UnknownVirtualSlot78();
    virtual int UnknownVirtualSlot79();
    virtual int UnknownVirtualSlot80();
    virtual int UnknownVirtualSlot81();
    virtual int UnknownVirtualSlot82();
    virtual int UnknownVirtualSlot83(VehicleWheel* wheel);
    virtual void UnknownVirtualSlot84(int a, int b);
    virtual void UnknownVirtualSlot85();
    virtual void UnknownVirtualSlot86();
    virtual void UnknownVirtualSlot87();  // entry is the shared empty stub 0x00464e90
    virtual int UnknownVirtualSlot88();
    virtual int UnknownVirtualSlot89(int arg);
    virtual void UnknownVirtualSlot90(int* a, int b);
    virtual void UnknownVirtualSlot91();  // entry is the shared empty stub 0x00464e90
    virtual void UnknownVirtualSlot92(int a, int b);
    virtual void UnknownVirtualSlot93();
    virtual void UnknownVirtualSlot94();  // entry is the shared empty stub 0x00464e90
    virtual void UnknownVirtualSlot95();
    virtual void UnknownVirtualSlot96();

    // non-virtual helper (retail 0x00526830, next to slot 70; tier 3 name)
    int Method_00526830();

    bool field_0x430;
    char pad_0x431[0x2];
    char field_0x433;
    float field_0x434;
    char pad_0x438[0x4];
    float field_0x43c;
    int field_0x440;
    int field_0x444;
    char pad_0x448[0x8];
    float field_0x450;
    int field_0x454;
    int field_0x458;
    char pad_0x45C[0xC];
    VehicleInputMap* field_0x468;
    void* field_0x46c;
    float field_0x470;
    float field_0x474;
    bool field_0x478;
    bool field_0x479;
    bool field_0x47a;
    char pad_0x47B[0x1];
    VehicleSteerState* field_0x47c;
    float* field_0x480;
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
    int field_0x4e4;
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
    char pad_0x574[0x14];
    int field_0x588;
    char pad_0x58C[0x4];
    int field_0x590;
    int field_0x594;
    char pad_0x598[0x4];
    int field_0x59c;
    int field_0x5a0;
    int field_0x5a4;
    int field_0x5a8;
    VehicleCamera* field_0x5ac;
    float field_0x5b0;
    int field_0x5b4;
    char pad_0x5B8[0x8];
};

VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x04, 0x4);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x0c, 0xC);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x18, 0x18);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x24, 0x24);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x2c, 0x2C);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x48, 0x48);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x64, 0x64);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x7c, 0x7C);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x88, 0x88);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x94, 0x94);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0xa0, 0xA0);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0xac, 0xAC);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0xb8, 0xB8);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0xbc, 0xBC);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0xcc, 0xCC);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0xd8, 0xD8);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0xe4, 0xE4);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0xf0, 0xF0);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x108, 0x108);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x109, 0x109);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x10c, 0x10C);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x118, 0x118);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x124, 0x124);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x128, 0x128);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x12c, 0x12C);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x130, 0x130);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x138, 0x138);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x13c, 0x13C);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x140, 0x140);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x14c, 0x14C);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x150, 0x150);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x194, 0x194);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x1ac, 0x1AC);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x1b8, 0x1B8);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x1cc, 0x1CC);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x1d0, 0x1D0);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x1e0, 0x1E0);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x1f0, 0x1F0);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x20c, 0x20C);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x20d, 0x20D);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x218, 0x218);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x3bc, 0x3BC);
VEH_CHECK_OFFSET(SoultreePhysicsCharacter, field_0x42c, 0x42C);
VEH_CHECK_OFFSET(Vehicle, field_0x430, 0x430);
VEH_CHECK_OFFSET(Vehicle, field_0x433, 0x433);
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
VEH_CHECK_OFFSET(Vehicle, field_0x590, 0x590);
VEH_CHECK_OFFSET(Vehicle, field_0x594, 0x594);
VEH_CHECK_OFFSET(Vehicle, field_0x59c, 0x59C);
VEH_CHECK_OFFSET(Vehicle, field_0x5a0, 0x5A0);
VEH_CHECK_OFFSET(Vehicle, field_0x5a4, 0x5A4);
VEH_CHECK_OFFSET(Vehicle, field_0x5a8, 0x5A8);
VEH_CHECK_OFFSET(Vehicle, field_0x5ac, 0x5AC);
VEH_CHECK_OFFSET(Vehicle, field_0x5b0, 0x5B0);
VEH_CHECK_OFFSET(Vehicle, field_0x5b4, 0x5B4);

#endif
