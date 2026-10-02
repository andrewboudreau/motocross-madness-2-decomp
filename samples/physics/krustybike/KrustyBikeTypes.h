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
struct KbGhost;

// Vectors use the shared Math3D Vec3 (12 bytes: x,y,z at +0,+4,+8; tier 1 layout).
#include "../bike/Bike.h"
#include "../contact/ObjectPlacement.h"

// 0x0067C348: a global zero vector copied by several overrides (three dword loads).
extern Vec3 g_kbZeroVec;

// Object reached through the global at 0x0056E26C (game/session singleton).
struct KbTrackRec { Vec3 field_0x00; char pad_0x0C[0xC]; };
struct KbMode { char pad_0x00[0x94]; int Fn_00524100(); };
struct KbGameCfg { char pad_0x0000[0x10]; int field_0x10; };
// On-screen message object (0x8C bytes; ctor 0x0051B200 takes the text and a display time).
struct KbMessage { char data[0x8C]; KbMessage(const char* text, float seconds); };
struct KbGame {
    void Fn_00521970(int id, char* buffer, int size);   // 0x00521970: fetch text for a string id
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
    int field_0xffc; // 0xFFC
    char pad_0x1000[0x1D70];
    int field_0x2d70; // 0x2D70
    int field_0x2d74; // 0x2D74
    char pad_0x2D78[0xC];
    int field_0x2d84; // 0x2D84
    char pad_0x2D88[0x68C];
    int field_0x3414; // 0x3414
    int field_0x3418; // 0x3418
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
    char pad_0x0248[0x168];
    void* field_0x3b0; // 0x3B0
    void* field_0x3b4; // 0x3B4
};

struct KbRaceHandler {
    void Fn_004DE580(void* who, float a, int b);
};

// Object reached through KrustyBike+0x740 (event/race context).
struct KbRaceSub { int* field_0x0; };
struct KbRace {
    char pad_0x0000[0x38];
    void* field_0x38; // 0x38
    char pad_0x003C[0x8];
    KbRaceHandler* handler; // 0x44
    KbRaceSub* field_0x48; // 0x48
    char pad_0x004C[0x4];
    KbRacer* field_0x50; // 0x50
    char pad_0x0054[0x64];
    int field_0xb8; // 0xB8
    char pad_0x00BC[0xC];
    int field_0xc8; // 0xC8
    char pad_0x00CC[0x2C];
    int field_0xf8; // 0xF8
    int field_0xfc; // 0xFC
    int field_0x100; // 0x100
    char pad_0x0104[0x40];
    int field_0x144; // 0x144
    char pad_0x0148[0x41];
    char field_0x189; // 0x189
    char field_0x18a; // 0x18A
    char pad_0x018B[0x3];
    char field_0x18e; // 0x18E
    char pad_0x018F[0xD];
    KbSensor* field_0x19c; // 0x19C
};
// Record at KrustyBike+0x744 (tier 3): per-bike state block reset by slot 43.
struct KbRaw3 { int a, b, c; };
struct KbGhost {
    char pad_0x00[0x8];
    int field_0x08, field_0x0c, field_0x10, field_0x14, field_0x18;
    Vec3 field_0x1c;
    Vec3 field_0x28;
    int field_0x34;
    KbRaw3 field_0x38;
    KbRaw3 field_0x44;
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
    void Fn_00444D80(KbObj128* who);   // 0x00444D80 (tier 3: attach/detach with a scene object)
    Vec3 Fn_004FD710(const Vec3* v);
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
    void Fn_0047BBF0(float a);   // 0x0047BBF0 (tier 3)
};
struct KbDirector {
    void Fn_004DCF20(KbObj128* who, int a);
};
extern KbDirector* g_kbDirector; // pointer global at 0x0068ABA4


// Pad/force-feedback-like device at VehicleInputMap+0x0c (tier 3 name).  Vtable slots used
// by KrustyBike slots 93/94: 6 (0x18), 7 (0x1c), 11 (0x2c), 12 (0x30); field +0x0c is a mode.
struct KbEffectParams { int field_0x0; int field_0x4; };
struct KbPad {
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6(int a, int b, int c);
    virtual void UnknownVirtualSlot7(int a, KbEffectParams* b, int c);
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11(int a);
    virtual void UnknownVirtualSlot12(int a, int b, int c);
    char pad_0x04[0x8];
    int field_0xc;
};
extern int g_kbPadActive;   // 0x0067C3AC

// Views of the rider-animation objects behind Bike+0x5c4 and Bike+0x604 (tier 3 names).
struct KbAnim;
struct KbA5C4 {
    char pad_0x00[0x10];
    int field_0x10; // 0x10
    KbAnim* Fn_004A6B30(const char* name, int a);
    void Fn_004A8B40(int a);
};
struct KbA604 {
    void Fn_005305B0(KbAnim* a);
    void Fn_005305F0(KbAnim* a);
    void Fn_00530630(KbAnim* a);
    void Fn_00530680(BikeWheel* w);
    void Fn_005328B0(SoultreeObject* node, Vec3 a, Vec3 b);
};

// Session object at KbGame+0x570; 0x0045D2B0 returns the local player record (tier 3 names).
struct KbMessage;
struct KbMsgSink { void Fn_0051B540(KbMessage* m); };
struct KbSession { KbPlayer* Fn_0045D2B0(); KbMsgSink* Fn_0045D340(); };
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
unsigned char Kb_004B0AC0(Vec3* pos, float r0, float r1, float r2, float scale,
                          int a, int b, int c, int d);
// 0x004B0DF0 (the query the base slot 11 issues) is Fn_4b0df0, declared in
// ../contact/ObjectPlacement.h.

#endif
