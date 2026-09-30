// KrustyBikeTypes.h -- PROVISIONAL supporting types for the KrustyBike reconstruction.
// GENERATED from work/e_krustybike/spec.py by gen.py; edit the spec, not this file.
//
// Everything here is tier-3 (provisional): member offsets are confirmed by decoded
// instructions in the KrustyBike overrides, but the names and the boundaries between
// Bike and KrustyBike members are inferred.  Once Bike.h (agent D) and common/Math3D.h
// exist, replace Bike below with the real Bike and KbVec3 with the common vector type.
#ifndef KRUSTYBIKE_TYPES_H
#define KRUSTYBIKE_TYPES_H

struct KbRacer;
struct KbRace;
struct KbRaceHandler;
struct KbSink;
struct KbLink;
struct KbEngine;
struct KbInput;
struct KbTrackA;
struct KbTrackB;
struct KbSensor;
struct KbXform;
struct KbChild;
struct KbObj128;

// KbVec3 is the shared Math3D Vec3 (12 bytes: x,y,z at +0,+4,+8; tier 1 layout).
#include "../common/Math3D.h"
typedef Vec3 KbVec3;

// 0x0067C348: a global zero vector copied by several overrides (three dword loads).
extern KbVec3 g_kbZeroVec;

// Object reached through the global at 0x0056E26C (game/session singleton).
struct KbGame {
    char pad_0x0000[0x18];
    int field_0x18; // 0x18
    char pad_0x001C[0x54C];
    KbRacer* field_0x568; // 0x568
    char pad_0x056C[0xA0];
    int field_0x60c; // 0x60C
    char pad_0x0610[0x9D0];
    int field_0xfe0; // 0xFE0
    int field_0xfe4; // 0xFE4
    int field_0xfe8; // 0xFE8
    int field_0xfec; // 0xFEC
    int field_0xff0; // 0xFF0
    int field_0xff4; // 0xFF4
    int field_0xff8; // 0xFF8
    char pad_0x0FFC[0x1D74];
    int field_0x2d70; // 0x2D70
    int field_0x2d74; // 0x2D74
    char pad_0x2D78[0xC];
    int field_0x2d84; // 0x2D84
    int Fn_00524100();
};
extern KbGame* g_kbGame;

// Object at Bike+0x604 (audio/engine-sound style helper; only its methods are called here).
struct KbEngine {
    char pad_0x0000[0x52C];
    int field_0x52c; // 0x52C
    void Fn_00532220(int a);
    void Fn_00532310();
    void Fn_005327C0();
};

struct KbInput {
    char pad_0x0000[0x150];
    float field_0x150; // 0x150
    char pad_0x0154[0xF4];
    KbVec3 field_0x248; // 0x248
    char pad_0x0254[0xC];
    int field_0x260; // 0x260
    char pad_0x0264[0x18];
    float field_0x27c; // 0x27C
    float field_0x280; // 0x280
    char pad_0x0284[0x18];
    float field_0x29c; // 0x29C
};
struct KbTrackA {
    char pad_0x0000[0xA4];
    KbTrackB* field_0xa4; // 0xA4
};
struct KbTrackB {
    char pad_0x0000[0x394];
    float field_0x394; // 0x394
    float field_0x398; // 0x398
};
struct KbSensor {
    char pad_0x0000[0x3DC];
    int field_0x3dc; // 0x3DC
};

struct KbRacer {
    char pad_0x0000[0xA8];
    void* field_0xa8; // 0xA8
    char pad_0x00AC[0x198];
    int field_0x244; // 0x244
    char pad_0x0248[0x16C];
    void* field_0x3b4; // 0x3B4
};

struct KbRaceHandler {
    void Fn_004DE580(void* who, int a, int b);
};

// Object reached through KrustyBike+0x740 (event/race context).
struct KbRace {
    char pad_0x0000[0x38];
    void* field_0x38; // 0x38
    char pad_0x003C[0x8];
    KbRaceHandler* handler; // 0x44
    char pad_0x0048[0x8];
    KbRacer* field_0x50; // 0x50
    char pad_0x0054[0x136];
    char field_0x18a; // 0x18A
    char pad_0x018B[0x3];
    char field_0x18e; // 0x18E
    char pad_0x018F[0xD];
    KbSensor* field_0x19c; // 0x19C
};

struct KbSink {
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual int UnknownVirtualSlot2(int a, int b);
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
};

// Matrix-like helper at Bike+0x3bc; 0x004FD710 transforms a vector (Math3D owner: see report).
struct KbXform {
    KbVec3 Fn_004FD710(KbVec3* v);
};
// Element of the array at Bike+0x12c (count at +0x130).
struct KbChild {
    int pad_0[1];
    int field_0x4;
    char pad_8[0x9c];
    int field_0xa4;
};
struct KbObj128 {
    char pad_0[0xc];
    KbSink field_0xc;
    char pad_10[0x74];
    int field_0x84;
    void Fn_00435FE0();
};
struct KbDirector {
    void Fn_004DCF20(KbObj128* who, int a);
};
extern KbDirector* g_kbDirector; // pointer global at 0x0068ABA4

// 0x004CB6E0: cdecl helper applied to (Bike+0xd8, ptr, Vec3*, float dt, flag).
void Fn_004CB6E0(float* a, float* b, float* c, float d, char e);

struct KbLink {
    char pad_0x0000[0xC];
    int field_0xc; // 0xC
};

// Provisional stand-in for Bike (agent D owns the real one).  Bike's vtable has 103
// slots (KrustyBike vtable 0x00554548); secondary vptrs live at +540 and +1848 in the
// retail layout and are represented only by padding here.
class Bike {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1(int a);
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3(int a0, int a1, int a2, int a3, int a4, int a5, int* a6);
    virtual void UnknownVirtualSlot4(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int* a13, int a14);
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual int UnknownVirtualSlot12(int a);
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14(KbVec3* a, KbVec3* b, KbVec3* c);
    virtual void UnknownVirtualSlot15(KbVec3* v, float* out);
    virtual KbVec3 UnknownVirtualSlot16(KbVec3* v);
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual int UnknownVirtualSlot22();
    virtual int UnknownVirtualSlot23();
    virtual int UnknownVirtualSlot24();
    virtual int UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29();
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual void UnknownVirtualSlot32();
    virtual int UnknownVirtualSlot33(int a0, int a1, int a2, int a3, int a4, int a5);
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35();
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot38();
    virtual int UnknownVirtualSlot39(int a);
    virtual void UnknownVirtualSlot40();
    virtual void UnknownVirtualSlot41();
    virtual int UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot43();
    virtual void UnknownVirtualSlot44();
    virtual float UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot46();
    virtual void UnknownVirtualSlot47();
    virtual void UnknownVirtualSlot48();
    virtual void UnknownVirtualSlot49();
    virtual void UnknownVirtualSlot50();
    virtual void UnknownVirtualSlot51();
    virtual int UnknownVirtualSlot52();
    virtual void UnknownVirtualSlot53();
    virtual void UnknownVirtualSlot54();
    virtual void UnknownVirtualSlot55();
    virtual void UnknownVirtualSlot56();
    virtual void UnknownVirtualSlot57();
    virtual void UnknownVirtualSlot58();
    virtual void UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60();
    virtual void UnknownVirtualSlot61();
    virtual void UnknownVirtualSlot62();
    virtual void UnknownVirtualSlot63(int a);
    virtual void UnknownVirtualSlot64(int a);
    virtual void UnknownVirtualSlot65();
    virtual int UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual int UnknownVirtualSlot68(int* out);
    virtual void UnknownVirtualSlot69();
    virtual int UnknownVirtualSlot70(float a);
    virtual void UnknownVirtualSlot71(int a);
    virtual void UnknownVirtualSlot72();
    virtual void UnknownVirtualSlot73();
    virtual void UnknownVirtualSlot74();
    virtual void UnknownVirtualSlot75();
    virtual KbVec3 UnknownVirtualSlot76(int a, int b);
    virtual int UnknownVirtualSlot77(int a);
    virtual void UnknownVirtualSlot78();
    virtual void UnknownVirtualSlot79();
    virtual int UnknownVirtualSlot80();
    virtual int UnknownVirtualSlot81();
    virtual int UnknownVirtualSlot82();
    virtual int UnknownVirtualSlot83(KbInput* input);
    virtual int UnknownVirtualSlot84(int a, int b);
    virtual void UnknownVirtualSlot85();
    virtual void UnknownVirtualSlot86();
    virtual void UnknownVirtualSlot87();
    virtual void UnknownVirtualSlot88();
    virtual void UnknownVirtualSlot89();
    virtual void UnknownVirtualSlot90();
    virtual void UnknownVirtualSlot91();
    virtual void UnknownVirtualSlot92();
    virtual void UnknownVirtualSlot93();
    virtual void UnknownVirtualSlot94();
    virtual void UnknownVirtualSlot95();
    virtual void UnknownVirtualSlot96();
    virtual void UnknownVirtualSlot97();
    virtual void UnknownVirtualSlot98();
    virtual void UnknownVirtualSlot99();
    virtual void UnknownVirtualSlot100();
    virtual void UnknownVirtualSlot101();
    virtual void UnknownVirtualSlot102();
public:
    char pad_0x0004[0x8];
    KbVec3 position; // 0xC
    char pad_0x0018[0xC];
    float field_0x24; // 0x24
    char pad_0x0028[0x3C];
    KbVec3 velocity; // 0x64
    KbVec3 field_0x70; // 0x70
    KbVec3 field_0x7c; // 0x7C
    char pad_0x0088[0x30];
    float field_0xb8; // 0xB8
    float speed; // 0xBC
    KbVec3 field_0xc0; // 0xC0
    char pad_0x00CC[0xC];
    KbVec3 field_0xd8; // 0xD8
    KbVec3 field_0xe4; // 0xE4
    KbVec3 field_0xf0; // 0xF0
    char pad_0x00FC[0xC];
    char field_0x108; // 0x108
    char field_0x109; // 0x109
    char pad_0x010A[0x1E];
    KbObj128* field_0x128; // 0x128
    KbChild** field_0x12c; // 0x12C
    int field_0x130; // 0x130
    char pad_0x0134[0x8];
    float field_0x13c; // 0x13C
    char pad_0x0140[0xB0];
    KbTrackA* field_0x1f0; // 0x1F0
    char pad_0x01F4[0x4];
    float field_0x1f8; // 0x1F8
    char pad_0x01FC[0x8];
    int field_0x204; // 0x204
    int field_0x208; // 0x208
    char pad_0x020C[0x1B0];
    KbXform* field_0x3bc; // 0x3BC
    char pad_0x03C0[0x70];
    char field_0x430; // 0x430
    char field_0x431; // 0x431
    char pad_0x0432[0x1];
    char field_0x433; // 0x433
    float field_0x434; // 0x434
    char pad_0x0438[0xC];
    int field_0x444; // 0x444
    int field_0x448; // 0x448
    char pad_0x044C[0x4];
    float field_0x450; // 0x450
    float field_0x454; // 0x454
    float field_0x458; // 0x458
    char pad_0x045C[0x4];
    int field_0x460; // 0x460
    char pad_0x0464[0x4];
    KbSink* field_0x468; // 0x468
    int field_0x46c; // 0x46C
    float field_0x470; // 0x470
    char pad_0x0474[0x5];
    bool field_0x479; // 0x479
    char pad_0x047A[0x6];
    float* field_0x480; // 0x480
    char pad_0x0484[0xA0];
    float field_0x524; // 0x524
    float field_0x528; // 0x528
    float field_0x52c; // 0x52C
    float field_0x530; // 0x530
    float field_0x534; // 0x534
    float field_0x538; // 0x538
    char pad_0x053C[0x60];
    int field_0x59c; // 0x59C
    char pad_0x05A0[0x10];
    float field_0x5b0; // 0x5B0
    int field_0x5b4; // 0x5B4
    char pad_0x05B8[0xC];
    KbLink* field_0x5c4; // 0x5C4
    char pad_0x05C8[0x28];
    KbInput* field_0x5f0; // 0x5F0
    KbInput* field_0x5f4; // 0x5F4
    char pad_0x05F8[0xC];
    KbEngine* field_0x604; // 0x604
    int field_0x608; // 0x608
    char pad_0x060C[0x10];
    KbVec3 field_0x61c; // 0x61C
    char pad_0x0628[0x14];
    float field_0x63c; // 0x63C
    char pad_0x0640[0x24];
    float field_0x664; // 0x664
    char pad_0x0668[0x98];
    int field_0x700; // 0x700
    char pad_0x0704[0x30];
    char field_0x734; // 0x734
    char field_0x735; // 0x735
    char field_0x736; // 0x736
    char pad_0x0737[0x9];
};

#endif
