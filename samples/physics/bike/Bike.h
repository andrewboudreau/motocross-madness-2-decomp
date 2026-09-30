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

typedef Vec3 BikeVec3;   // the shared Math3D vector (same as VehVec3/SoultreeVec3)

struct BikeWheel;
struct BikeA604;
struct BikeA34;
struct BikeA5C4;
struct BikeA1A0;
struct BikeA128;
struct BikeElem;
struct BikeA1F4;
struct BikeA5AC;
struct BikeA640;
struct BikeA644;
struct BikeQ;
struct BikeA38;
struct BikeXform;

// Objects reached through Bike fields.  Layout is only known where accessed.
struct BikeWheel {
    char pad_0x000[204];
    BikeVec3 w_0xcc;
    char pad_0x0d8[12];
    BikeVec3 w_0xe4;
    char pad_0x0f0[12];
    BikeVec3 w_0xfc;
    BikeVec3 w_0x108;
    char pad_0x114[48];
    float w_0x144;
    float w_0x148;
    char pad_0x14c[4];
    float w_0x150;
    char pad_0x154[4];
    float w_0x158;
    char pad_0x15c[212];
    BikeVec3 w_0x230;
    BikeVec3 w_0x23c;
    BikeVec3 w_0x248;             // KrustyBike slot 48 (0x004964e0) zeroes it
    char pad_0x254[12];
    int w_0x260;
    char pad_0x264[4];
    int w_0x268;
    char pad_0x26c[20];
    float w_0x280;
    char pad_0x284[4];
    float w_0x288;
    char pad_0x28c[8];
    float w_0x294;
    char pad_0x298[4];
    float w_0x29c;
    char pad_0x2a0[4];
    int w_0x2a4;
    char pad_0x2a8[4];
    BikeQ* w_0x2ac;
    BikeQ* w_0x2b0;
    char pad_0x2b4[4];
    float w_0x2b8;
    float w_0x2bc;
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
    BikeVec3 a_0xdc;
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
    void Method_0x00532900(BikeVec3 offset, int bone);
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
    BikeXform* d_0x140;
    int Method_0x004fdae0(const char* name);
    void Method_0x004444e0();
    void Method_0x004fb8c0(int a, float* b);
};

struct BikeA128 {
    char pad_0x000[752];
    int g_0x2f0;
};

struct BikeElem {
    char pad_0x000[4];
    int h_0x4;
    char pad_0x008[12];
    BikeVec3 h_0x14;
    char pad_0x020[36];
    BikeVec3 h_0x44;
    char pad_0x050[80];
    float h_0xa0;
    int h_0xa4;
};

struct BikeA1F4 {
    char pad_0x000[3048];
    int i_0xbe8;
};

struct BikeA5AC {

};

struct BikeA640 {
    float l_0x0;
    float l_0x4;
    float l_0x8;
};

struct BikeA644 {
    int m_0x0;
    float m_0x4;
    float m_0x8;
    float m_0xc;
    float m_0x10;
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

struct BikeXform {
    void Method_0x004fc050(int a, BikeVec3* b, BikeVec3* c, int d, int e);
    void Method_0x004fc540(int a, BikeVec3* b, BikeVec3* c);
    BikeVec3* Method_0x004fd7f0(BikeVec3* out, const BikeVec3* in);
    BikeVec3* Method_0x004fd660(BikeVec3* out, const BikeVec3* in);
    void Method_0x004fca80(int a, float* b);
    BikeVec3* Method_0x004fd710(BikeVec3* out, const BikeVec3* in);
    void Method_0x004fd1f0(BikeVec3 pos, BikeVec3 dir, float mag);
};

float BikeMath_0x00460b50(float x);
void BikeFunc_0x004b5a60(BikeVec3 a, BikeVec3 b, float* p1, float* p2, float* p3, float* p4, float* p5, float* p6, float* p7);   // cdecl
float BikeMath_0x00460c00(float x);            // cdecl; used to scale a vector by 1/length (provisional)
struct BikeGlobal_0056e26c { char pad_0x000[0x2f0]; float g_0x2f0; };
extern BikeGlobal_0056e26c* g_Bike_0056e26c;    // pointer read at 0x0056e26c (frame delta / time step; provisional)
            // cdecl, returns float in st0
extern BikeVec3 g_BikeVec3_005778a8;
extern BikeVec3 g_BikeVec3_005778c8;
BikeVec3* BikeVecAdd_0x00421cb0(BikeVec3* out, const BikeVec3* a, const BikeVec3* b);   // cdecl
BikeVec3* BikeVecSub_0x00421d00(BikeVec3* out, const BikeVec3* a, const BikeVec3* b);   // cdecl
void* operator new(unsigned int size, const char* file, int line);   // debug allocator at 0x004a3010

class Bike : public Vehicle {
public:
    Bike();
    virtual ~Bike();              // vbase vtable 0x0055085c slot 0 -> 0x0040ca40

    // overrides of Vehicle slots (signatures: SoultreePhysicsBaseObject.h, Vehicle.h)
    virtual void UnknownVirtualSlot1(float value);
    virtual int UnknownVirtualSlot5(int arg);
    virtual void UnknownVirtualSlot7(const BikeVec3* dir);
    virtual void UnknownVirtualSlot8();
    virtual BikeVec3 UnknownVirtualSlot16(const BikeVec3* v);
    virtual void UnknownVirtualSlot29(int arg);
    virtual float UnknownVirtualSlot32();
    virtual int UnknownVirtualSlot33(const BikeVec3* a, const BikeVec3* b, const BikeVec3* c,
                                     const BikeVec3* d, int e, float f);
    virtual int UnknownVirtualSlot39(float dt);
    virtual void UnknownVirtualSlot41();
    virtual int UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot44();
    virtual BikeVec3* UnknownVirtualSlot46(BikeVec3* out, float t);
    virtual float UnknownVirtualSlot47(float arg);
    virtual void UnknownVirtualSlot49(float arg);
    virtual float UnknownVirtualSlot53();
    virtual BikeVec3* UnknownVirtualSlot54(BikeVec3* out);
    virtual BikeVec3* UnknownVirtualSlot55(BikeVec3* out, BikeVec3* pos);
    virtual void UnknownVirtualSlot56(BikeVec3* a, int b, BikeVec3* c);
    virtual float UnknownVirtualSlot57();
    virtual float UnknownVirtualSlot59();
    virtual float UnknownVirtualSlot61(float arg);
    virtual int UnknownVirtualSlot62(float arg);
    virtual void UnknownVirtualSlot65(float arg);
    virtual int UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual void UnknownVirtualSlot71(int arg);
    virtual void UnknownVirtualSlot72(BikeVec3* out, VehicleWheel* wheel);
    virtual float UnknownVirtualSlot73(const BikeVec3* a, const BikeVec3* b);
    virtual float UnknownVirtualSlot75();
    virtual BikeVec3 UnknownVirtualSlot76(const BikeVec3* a, const BikeVec3* b);
    virtual void UnknownVirtualSlot90(int* flag, float arg);
    virtual void UnknownVirtualSlot91();
    virtual void UnknownVirtualSlot92(const BikeVec3* a, const BikeVec3* b);
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
    // casts at the use site (MIGRATION.md rule 6): d3d_field_0x1a0 and field_0x42c as
    // BikeXform*, field_0x12c as BikeElem**, field_0x1f4 as BikeA1F4*.  Inline accessor
    // functions were tried and cost the exact match of slot 8 (0x00406750): VC6 then
    // loads field_0x42c before the float arguments instead of after them.

    int field_0x5bc;              // first Bike field (Vehicle's own data ends at 0x5bc)
    int field_0x5c0;
    BikeA5C4* field_0x5c4;
    char pad_0x5c8[40];
    BikeWheel* field_0x5f0;
    BikeWheel* field_0x5f4;
    char pad_0x5f8[12];
    BikeA604* field_0x604;
    Vehicle* field_0x608;
    char pad_0x60c[16];
    BikeVec3 field_0x61c;
    float field_0x628;
    float field_0x62c;
    int field_0x630;
    int field_0x634;
    int field_0x638;
    float field_0x63c;
    BikeA640* field_0x640;
    BikeA644* field_0x644;
    char pad_0x648[8];
    int field_0x650;
    float field_0x654;
    int field_0x658;
    float field_0x65c;
    float field_0x660;
    int field_0x664;
    int field_0x668;
    int field_0x66c[18];
    int field_0x6b4[18];
    int field_0x6fc;
    int field_0x700;
    float field_0x704;
    float field_0x708;
    char pad_0x70c[4];
    float field_0x710;
    float field_0x714;
    float field_0x718;
    float field_0x71c;
    int field_0x720;
    float field_0x724;
    char pad_0x728[12];           // own data ends at 0x734; the compiler places the vtordisp there
};

typedef char bike_assert_sizeof[(sizeof(Bike) == 0x738 + 0x2c) ? 1 : -1];

#endif
