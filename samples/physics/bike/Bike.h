// Bike: motorcycle-specific layer over Vehicle (RTTI ".?AVBike@@", vtables
// 0x00550900 (primary, 103 slots), 0x005508cc @+540 (12 slots),
// 0x0055085c @+1848 (27 slots, virtual base)).
//
// Layout facts (tier 1 unless noted):
//  * Bike : Vehicle : SoultreePhysicsCharacter (+ D3IMSoultreeCharacter at
//    object offset 540); GameObject/BaseObject is a virtual base placed at
//    +1848 in a complete Bike and +1472 in a complete Vehicle, so the fields
//    Bike adds occupy [1472, 1848).  (RTTI hierarchy + vtable_write_xrefs.)
//  * Slots 0..96 come from Vehicle (Bike overrides some); Bike introduces
//    97..102.  (vtable_overrides.json)
//
// PROVISIONAL: agent C owns the real Vehicle.h.  Until it lands, the base
// classes below are stand-ins with padding so that offsets and vtable slot
// numbers are right.  Every field named field_0xNN is tier 3 (offset is
// tier 1/2 from decoded instructions, the type and any role are guesses).
#ifndef MCM2_PHYSICS_BIKE_H
#define MCM2_PHYSICS_BIKE_H

struct BikeVec3 {
    float x, y, z;
    BikeVec3() {}
    BikeVec3(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
};

struct BikeWheel;
struct BikeA604;
struct BikeA34;
struct BikeA5C4;
struct BikeA1A0;
struct BikeA124;
struct BikeA128;
struct BikeElem;
struct BikeA1F4;
struct BikeA47C;
struct BikeA5AC;
struct BikeA640;
struct BikeA644;
struct BikeObj;
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
    char pad_0x248[24];
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
    BikeA124* a_0x11c;
    BikeA604(int a);
    char pad_tail[0x41c];
    int Method_0x00530190(int a, BikeA5C4* b, const char* name);
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

struct BikeA124 {

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

struct BikeA47C {
    char pad_0x000[4];
    float j_0x4;
    float j_0x8;
    void Method_0x00504ec0(float a, int b);
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

struct BikeObj {
    void Method_0x00515660();
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

struct BikeGOSub {   // sub-object at GameObject+4 (provisional)
    char pad_0x00[0x18];
    void Method_0x00469190(BikeA604* a, int b);
};

class BikeGameObject {   // stand-in for the virtual base (27 slots)
public:
    virtual void GOVirtualSlot0();
    virtual void GOVirtualSlot1();
    virtual void GOVirtualSlot2();
    virtual void GOVirtualSlot3();
    virtual void GOVirtualSlot4();
    virtual void GOVirtualSlot5();
    virtual void GOVirtualSlot6();
    virtual void GOVirtualSlot7();
    virtual void GOVirtualSlot8();
    virtual void GOVirtualSlot9();
    virtual void GOVirtualSlot10();
    virtual void GOVirtualSlot11();
    virtual void GOVirtualSlot12();
    virtual void GOVirtualSlot13();
    virtual void GOVirtualSlot14();
    virtual void GOVirtualSlot15();
    virtual void GOVirtualSlot16();
    virtual void GOVirtualSlot17();
    virtual void GOVirtualSlot18();
    virtual void GOVirtualSlot19();
    virtual void GOVirtualSlot20();
    virtual void GOVirtualSlot21();
    virtual void GOVirtualSlot22();
    virtual void GOVirtualSlot23();
    virtual void GOVirtualSlot24();
    virtual void GOVirtualSlot25();
    virtual void GOVirtualSlot26();
    BikeGOSub field_0x4;
    int go_0x1c;
};

class SoultreePhysicsCharacter : public virtual BikeGameObject {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1(int arg);
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual int UnknownVirtualSlot5(int arg);
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7(const BikeVec3* dir);
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual BikeVec3 UnknownVirtualSlot16(const BikeVec3& v);
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual void UnknownVirtualSlot22();
    virtual void UnknownVirtualSlot23();
    virtual void UnknownVirtualSlot24();
    virtual void UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29(int arg);
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual float UnknownVirtualSlot32();
    virtual int UnknownVirtualSlot33(int a, int b, int c, int d, int e, int f);
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35();
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot38();
    virtual int UnknownVirtualSlot39(int arg);
    virtual void UnknownVirtualSlot40();
    virtual void UnknownVirtualSlot41();
    virtual int UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot43();
    virtual void UnknownVirtualSlot44();
    virtual void UnknownVirtualSlot45();
    virtual BikeVec3 UnknownVirtualSlot46(float t);
    virtual float UnknownVirtualSlot47(float arg);
    virtual void UnknownVirtualSlot48();
    virtual void UnknownVirtualSlot49(float arg);
    virtual void UnknownVirtualSlot50();
    virtual int UnknownVirtualSlot51();
    virtual void UnknownVirtualSlot52();
    virtual float UnknownVirtualSlot53();
    virtual BikeVec3 UnknownVirtualSlot54();
    virtual BikeVec3 UnknownVirtualSlot55(BikeVec3* pos);
    virtual void UnknownVirtualSlot56(int a, int b, int c);
    virtual float UnknownVirtualSlot57();
    virtual void UnknownVirtualSlot58();
    virtual float UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60();
    virtual float UnknownVirtualSlot61(float arg);
    virtual int UnknownVirtualSlot62(float arg);
    virtual void UnknownVirtualSlot63();
    virtual void UnknownVirtualSlot64();
    virtual void UnknownVirtualSlot65(float arg);
    virtual int UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual void UnknownVirtualSlot68();
    virtual void UnknownVirtualSlot69();
    virtual int UnknownVirtualSlot70(float arg);
    virtual void UnknownVirtualSlot71(int arg);
    virtual void UnknownVirtualSlot72(BikeVec3* out, int unused);
    virtual float UnknownVirtualSlot73(const BikeVec3* a, const BikeVec3* b);
    virtual float UnknownVirtualSlot74(const BikeVec3* a, const BikeVec3* b, float c, float d);
    virtual float UnknownVirtualSlot75();
    virtual BikeVec3 UnknownVirtualSlot76(int a, int b);
    virtual void UnknownVirtualSlot77();
    virtual void UnknownVirtualSlot78();
    virtual void UnknownVirtualSlot79();
    virtual void UnknownVirtualSlot80();
    virtual void UnknownVirtualSlot81();
    virtual void UnknownVirtualSlot82();
    virtual void UnknownVirtualSlot83();
    virtual void UnknownVirtualSlot84();
    virtual void UnknownVirtualSlot85();
    virtual void UnknownVirtualSlot86();
    virtual void UnknownVirtualSlot87();
    virtual void UnknownVirtualSlot88();
    virtual void UnknownVirtualSlot89(float arg);
    virtual void UnknownVirtualSlot90(int* flag, float arg);
    virtual void UnknownVirtualSlot91();
    virtual void UnknownVirtualSlot92(const BikeVec3* a, const BikeVec3* b);
    virtual void UnknownVirtualSlot93();
    virtual void UnknownVirtualSlot94();
    virtual void UnknownVirtualSlot95();
    virtual void UnknownVirtualSlot96();
    char pad_0x008[4];
    float field_0xc;
    float field_0x10;
    float field_0x14;
    BikeVec3 field_0x18;
    float field_0x24;
    char pad_0x028[4];
    float field_0x2c;
    float field_0x30;
    float field_0x34;
    float field_0x38;
    float field_0x3c;
    float field_0x40;
    float field_0x44;
    float field_0x48;
    float field_0x4c;
    float field_0x50;
    float field_0x54;
    float field_0x58;
    float field_0x5c;
    float field_0x60;
    float field_0x64;
    float field_0x68;
    float field_0x6c;
    char pad_0x070[24];
    BikeVec3 field_0x88;
    BikeVec3 field_0x94;
    BikeVec3 field_0xa0;
    BikeVec3 field_0xac;
    float field_0xb8;
    float field_0xbc;
    char pad_0x0c0[24];
    float field_0xd8;
    float field_0xdc;
    float field_0xe0;
    BikeVec3 field_0xe4;
    BikeVec3 field_0xf0;
    char pad_0x0fc[12];
    char field_0x108;
    char pad_0x109[27];
    BikeA124* field_0x124;
    char pad_0x128[4];
    BikeElem** field_0x12c;
    int field_0x130;
    char pad_0x134[4];
    char field_0x138;
    char pad_0x139[3];
    float field_0x13c;
    float field_0x140;
    char pad_0x144[24];
    float field_0x15c;
    char pad_0x160[52];
    BikeVec3 field_0x194;
    BikeVec3 field_0x1a0;
    BikeVec3 field_0x1ac;
    BikeVec3 field_0x1b8;
    char pad_0x1c4[8];
    int field_0x1cc;
    char field_0x1d0;
    char pad_0x1d1[15];
    float field_0x1e0;
    float field_0x1e4;
    char pad_0x1e8[12];
    BikeA1F4* field_0x1f4;
    char pad_0x1f8[36];
};

class D3IMSoultreeCharacter : public virtual BikeGameObject {
public:
    virtual void D3IMVirtualSlot0();
    virtual void D3IMVirtualSlot1();
    virtual void D3IMVirtualSlot2();
    virtual void D3IMVirtualSlot3();
    virtual void D3IMVirtualSlot4();
    virtual void D3IMVirtualSlot5();
    virtual void D3IMVirtualSlot6();
    virtual void D3IMVirtualSlot7();
    virtual void D3IMVirtualSlot8();
    virtual void D3IMVirtualSlot9();
    virtual void D3IMVirtualSlot10();
    virtual void D3IMVirtualSlot11();
    void Method_0x004a8b00();
    void Method_0x004a8bf0(int a, float b);
    void Method_0x004a8c50(int a, int b, float c, float d);
};

class Vehicle : public SoultreePhysicsCharacter, public D3IMSoultreeCharacter {
public:
    char pad_0x224[408];
    BikeXform* field_0x3bc;
    char pad_0x3c0[108];
    BikeXform* field_0x42c;
    char field_0x430;
    char field_0x431;
    char pad_0x432[1];
    signed char field_0x433;
    char pad_0x434[4];
    float field_0x438;
    float field_0x43c;
    char pad_0x440[4];
    int field_0x444;
    int field_0x448;
    float field_0x44c;
    char pad_0x450[4];
    float field_0x454;
    char pad_0x458[4];
    float field_0x45c;
    int field_0x460;
    int field_0x464;
    char pad_0x468[16];
    char field_0x478;
    char field_0x479;
    char pad_0x47a[2];
    BikeA47C* field_0x47c;
    float* field_0x480;
    char pad_0x484[36];
    int field_0x4a8;
    float field_0x4ac;
    float field_0x4b0;
    float field_0x4b4;
    float field_0x4b8;
    float field_0x4bc;
    char pad_0x4c0[32];
    float field_0x4e0;
    float field_0x4e4;
    char pad_0x4e8[4];
    float field_0x4ec;
    char pad_0x4f0[20];
    float field_0x504;
    float field_0x508;
    char pad_0x50c[8];
    float field_0x514;
    char pad_0x518[8];
    int field_0x520;
    float field_0x524;
    float field_0x528;
    float field_0x52c;
    float field_0x530;
    float field_0x534;
    float field_0x538;
    BikeObj** field_0x53c;
    char pad_0x540[4];
    int field_0x544;
    char pad_0x548[40];
    int field_0x570;
    BikeVec3 field_0x574;
    float field_0x580;
    float field_0x584;
    float field_0x588;
    char pad_0x58c[16];
    int field_0x59c;
    char pad_0x5a0[4];
    int field_0x5a4;
    int field_0x5a8;
    char pad_0x5ac[8];
    int field_0x5b4;
    char pad_0x5b8[4];
    int field_0x5bc;
};

class Bike : public Vehicle {
public:
    // overrides of Vehicle slots
    virtual void UnknownVirtualSlot1(int arg);
    virtual int UnknownVirtualSlot5(int arg);
    virtual void UnknownVirtualSlot7(const BikeVec3* dir);
    virtual void UnknownVirtualSlot8();
    virtual BikeVec3 UnknownVirtualSlot16(const BikeVec3& v);
    virtual void UnknownVirtualSlot29(int arg);
    virtual float UnknownVirtualSlot32();
    virtual int UnknownVirtualSlot33(int a, int b, int c, int d, int e, int f);
    virtual int UnknownVirtualSlot39(int arg);
    virtual void UnknownVirtualSlot41();
    virtual int UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot44();
    virtual BikeVec3 UnknownVirtualSlot46(float t);
    virtual float UnknownVirtualSlot47(float arg);
    virtual void UnknownVirtualSlot49(float arg);
    virtual float UnknownVirtualSlot53();
    virtual BikeVec3 UnknownVirtualSlot54();
    virtual BikeVec3 UnknownVirtualSlot55(BikeVec3* pos);
    virtual void UnknownVirtualSlot56(int a, int b, int c);
    virtual float UnknownVirtualSlot57();
    virtual float UnknownVirtualSlot59();
    virtual float UnknownVirtualSlot61(float arg);
    virtual int UnknownVirtualSlot62(float arg);
    virtual void UnknownVirtualSlot65(float arg);
    virtual int UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual void UnknownVirtualSlot71(int arg);
    virtual void UnknownVirtualSlot72(BikeVec3* out, int unused);
    virtual float UnknownVirtualSlot73(const BikeVec3* a, const BikeVec3* b);
    virtual float UnknownVirtualSlot75();
    virtual BikeVec3 UnknownVirtualSlot76(int a, int b);
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
    void Method_0x00525c60();
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
    char pad_0x728[16];
};

#endif
