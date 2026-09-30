// KrustyBikeTypes.h -- PROVISIONAL supporting types for the KrustyBike reconstruction.
//
// Everything here is tier-3 (provisional): member offsets are confirmed by decoded
// instructions in the KrustyBike overrides, but the names are inferred.  The base
// class is the canonical Bike (bike/Bike.h).  The Kb* structs below are area-local
// views of objects reached through inherited members whose canonical types are still
// generic; the overrides cast to them at the use site (MIGRATION.md rule 6).
#ifndef KRUSTYBIKE_TYPES_H
#define KRUSTYBIKE_TYPES_H

struct KbRacer;
struct KbSession;
struct KbPlayer;
struct KbBody;
struct KbRace;
struct KbRaceHandler;
struct KbSink;
struct KbTrackA;
struct KbTrackB;
struct KbSensor;
struct KbXform;
struct KbChild;
struct KbObj128;

// KbVec3 is the shared Math3D Vec3 (12 bytes: x,y,z at +0,+4,+8; tier 1 layout).
#include "../bike/Bike.h"
typedef Vec3 KbVec3;

// 0x0067C348: a global zero vector copied by several overrides (three dword loads).
extern KbVec3 g_kbZeroVec;

// Object reached through the global at 0x0056E26C (game/session singleton).
struct KbTrackRec { KbVec3 field_0x00; char pad_0x0C[0xC]; };
struct KbMode { char pad_0x00[0x94]; int Fn_00524100(); };
struct KbGameCfg { char pad_0x0000[0x10]; int field_0x10; };
struct KbGame {
    char pad_0x0000[0x8];
    KbGameCfg* field_0x8; // 0x8
    char pad_0x000C[0xC];
    int field_0x18; // 0x18
    char pad_0x001C[0x544];
    KbTrackRec* field_0x560; // 0x560: array of 24-byte records (first member is a Vec3)
    char pad_0x0564[0x4];
    KbRacer* field_0x568; // 0x568
    char pad_0x056C[0x4];
    KbSession* field_0x570; // 0x570
    char pad_0x0574[0x4];
    KbMode field_0x578; // 0x578: game-mode state object (0x00524100 returns the mode number)
    int field_0x60c; // 0x60C
    char pad_0x0610[0x640];
    int field_0xc50; // 0xC50
    char pad_0x0C54[0x38C];
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
};
extern KbGame* g_kbGame;

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
    void Fn_004DE580(void* who, float a, int b);
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

// Matrix-like helper at +0x3bc (D3DIMSoultreeCharacter::d3d_field_0x1a0); 0x004FD710
// transforms a vector.
struct KbXform {
    KbVec3 Fn_004FD710(const KbVec3* v);
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


// Session object at KbGame+0x570; 0x0045D2B0 returns the local player record (tier 3 names).
struct KbSession { KbPlayer* Fn_0045D2B0(); };
class Vehicle;
struct KbPlayer {
    char pad_0x0000[0xA8];
    Vehicle* field_0xa8;   // 0xA8: vehicle this player is bound to
    char pad_0x00AC[0x30];
    Vehicle* field_0xdc;   // 0xDC
    void Fn_004A9E80(Vehicle* who, int a, int b);
};
// Bounds/collision body at Vehicle::field_0x128 (canonical: CollisionObject::Fn_004392c0).
struct KbBody { int Fn_004392C0(KbBody* other); };

// Non-virtual callees of slot 11 (cdecl; call targets are relocation-masked).  Tier 3 names.
// 0x004B0AC0: probe at a position with three radii and a scale; returns a byte flag.
unsigned char Kb_004B0AC0(KbVec3* pos, float r0, float r1, float r2, float scale,
                          int a, int b, int c, int d);
// 0x004B0DF0: the query the base slot 11 issues (SoulTreePhysics.cpp Fn_4b0df0).  This
// override passes different constants, so k and l are declared as nullable Vec3 pointers.
int Kb_004B0DF0(void* body, SoultreeProbe* obj, KbVec3* pos, float scale, int e, int f, int g, int h,
                float i, int j, const KbVec3* k, const KbVec3* l, int m, KbVec3* n, KbVec3* o,
                KbVec3* p, int* q);

#endif
