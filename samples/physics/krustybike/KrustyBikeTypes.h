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
struct KbOverlay;
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
struct KbCollider;
struct KbBonusTable;
class KrustyBike;

// Vectors use the shared Math3D Vec3 (12 bytes: x,y,z at +0,+4,+8; tier 1 layout).
#include "vehicle/Bike.h"
#include "contact/ObjectPlacement.h"

// 0x0067C348: a global zero vector copied by several overrides (three dword loads).
extern Vec3 g_kbZeroVec;

// Object reached through the global at 0x0056E26C (game/session singleton).
struct KbTrackRec { Vec3 field_0x00; char pad_0x0C[0xC]; };
struct KbMode { char pad_0x00[0x94]; int Fn_00524100(); };
struct KbGameCfg {
    char pad_0x0000[0x10]; int field_0x10;
    // 0x004AC830 (Net.cpp): send a message (type, data, size, destination id, flags); returns 1 on success.
    int Send(int type, void* data, int size, int dest, int flags);
};
// Object behind KrustyBike+0x13fc (tier 3); 0x004E8720 appends a record (type, id, data, flag).
struct KbRecorder { int Fn_004E8720(int type, int id, void* data, int flag); };
// Entity-level view used for the id at +0x11bc of any bike (tier 3, offset confirmed by decoded loads).
struct KbCell { char pad_0x00[8]; float field_0x8; char pad_0xc[4]; float field_0x10; };
struct KbNetBike {
    char pad_0x0000[0x11bc];
    int field_0x11bc;
    char pad_0x11c0[8];
    KbCell* field_0x11c8;
    KbCell* field_0x11cc;
};
// 12-byte message built by KrustyBike::Fn_004925A0 (tier 3: the leading dword is never written).
// Network message 13 (bikerace.cpp's 0x0041fb.. handlers pass it to 0x004933e0):
// signed byte deltas of the four state vectors plus two raw ints at the unaligned
// offsets +0x0e/+0x12 (tier 2 layout, tier 3 names).
#pragma pack(push, 1)
struct KbNetDelta {
    char field_0x00;
    signed char delta1[3];       // +0x01 x 15/64 -> KrustyBike+0x1364
    unsigned char step;          // +0x04 bit 0: units of 8, bits 1-7: count
    signed char delta3[3];       // +0x05 x pi/64 -> +0x137c..+0x1384
    signed char delta2[3];       // +0x08 x pi/64 -> +0x1370
    signed char delta0[3];       // +0x0b x 15/64 -> +0x1358
    int field_0x0e;
    int field_0x12;
};
#pragma pack(pop)

// The state a message is decoded into (BikeRace.h's UnknownBikeRaceNetState).
struct KbNetState {
    short field_0x00;
    unsigned short step;         // +0x02
    int field_0x04;
    Vec3 field_0x08;             // KrustyBike+0x1364
    float field_0x14;            // +0x137c
    float field_0x18;            // +0x1380
    float field_0x1c;            // +0x1384
    char pad_0x20[0x10];
    Vec3 field_0x30;             // +0x1370
    Vec3 field_0x3c;             // +0x1358
    int field_0x48;
    int field_0x4c;              // running step total (+0x1388)
    int field_0x50;
};

// Message type 1 (0x58 bytes) that 0x00492670 sends or records: this bike's motion state,
// pose and flags (tier 2 layout from the stores, tier 3 names).
struct KbBikeMessage {
    char field_0x00;
    unsigned char field_0x01;            // KrustyBike+0x784
    short field_0x02;                    // ms since the state's last send
    short field_0x04;                    // +0x7a0
    short field_0x06;                    // +0x790
    Vec3 position;                       // +0x08
    float roll, pitch, yaw;              // +0x14
    int field_0x20;                      // +0x74c
    int field_0x24;
    int field_0x28;                      // +0x750
    int field_0x2c;                      // +0x770
    Vec3 angularVelocity;                // +0x30 (y plus the turn rate)
    Vec3 velocity;                       // +0x3c
    unsigned char poseIndex : 4;         // +0x48
    unsigned char poseState : 4;
    unsigned char poseParam;             // +0x49 x 100
    unsigned char poseBlend;             // +0x4a x 100
    unsigned char poseLeanBlend;         // +0x4b x 100
    unsigned int time;                   // +0x4c
    unsigned char motion : 6;            // +0x50 crash motion (17 without one)
    unsigned char pad_0x50 : 2;
    unsigned char crashDirection : 3;    // +0x51
    unsigned char crashed : 1;
    unsigned char flag4 : 1;             // vehicle slot 51
    unsigned char flag5 : 1;             // +0x7a4
    unsigned char flag6 : 1;             // +0x78c
    unsigned char flag7 : 1;             // +0x478
    unsigned char flag8 : 1;             // +0x52 +0x479
    unsigned char flag9 : 1;             //       +0x153c
    unsigned char field_0x53;            // +0x7b8 (+0x7a0 in modes 2 and 3)
    unsigned char field_0x54;            // +0x11c0
};
// Received copy of message 1 (0x60 bytes, four at KrustyBike+0x11d8) that 0x00493660 reads.
struct KbBikeNetState : KbBikeMessage {
    unsigned int timeReceived;           // +0x58 local clock when it arrived
    int field_0x5c;                      // +0x5c set when the lap/race fields are valid
};
// Message type 10 (8 bytes): the bike's +0x11c0 flag and +0x768.
struct KbBikePing {
    char field_0x00;
    unsigned char field_0x01;
    char pad_0x02[2];
    float field_0x04;
};
// The state 0x00492670 keeps of the last message (caller-owned; tier 3).
struct KbBikeState {
    Vec3 position;                       // +0x00
    float roll, pitch, yaw;              // +0x0c
    Vec3 angularVelocity;                // +0x18
    Vec3 velocity;                       // +0x24
    int field_0x30;
    unsigned int time;                   // +0x34
    // What message 13's byte deltas could not carry (0x00492ad0), per value.
    Vec3 velocityError;                  // +0x38
    Vec3 positionError;                  // +0x44
    Vec3 angularVelocityError;           // +0x50
    float rollError;                     // +0x5c
    float pitchError;                    // +0x60
    float yawError;                      // +0x64
    float timer;                         // +0x68 seconds since the last full message
};
// Message type 13 (0x17 bytes, packed) that 0x00492ad0 sends or records: byte deltas
// against the last state (the decoder 0x004933e0 reads them as KbNetDelta), the time step
// and message 1's pose and flag bytes.
#pragma pack(push, 1)
struct KbBikeDeltaMessage {
    char field_0x00;
    signed char position[3];             // +0x01 x 64/15
    unsigned char coarse : 1;            // +0x04 the count is in units of 8 ms
    unsigned char count : 7;             //       ms since the last state
    signed char roll, pitch, yaw;        // +0x05 x 64/pi
    signed char angularVelocity[3];      // +0x08 x 64/pi
    signed char velocity[3];             // +0x0b x 64/15
    unsigned char motion : 6;            // +0x0e
    unsigned char pad_0x0e : 2;
    unsigned char crashDirection : 3;    // +0x0f
    unsigned char crashed : 1;
    unsigned char flag4 : 1;
    unsigned char flag5 : 1;
    unsigned char flag6 : 1;
    unsigned char flag7 : 1;
    unsigned char flag8 : 1;             // +0x10
    unsigned char flag9 : 1;
    unsigned char field_0x11;            // +0x11
    unsigned char poseIndex : 4;         // +0x12
    unsigned char poseState : 4;
    unsigned char poseParam;             // +0x13 x 100
    unsigned char poseBlend;             // +0x14 x 100
    unsigned char poseLeanBlend;         // +0x15 x 100
    unsigned char field_0x16;            // +0x16 +0x11c0
};
#pragma pack(pop)

struct KbNetPacket { int field_0x0; char field_0x4; char pad_0x5[3]; int field_0x8; };
// On-screen message object (0x8C bytes; ctor 0x0051B200 takes the text and a display time).
struct KbMessage { char data[0x8C]; KbMessage(const char* text, float seconds); };
// The debug overlay at KbGame+0x38 (src/reconstructed DebugOverlay: 0x00447fa0 names a page
// line block, 0x00447f40 prints a line; both cdecl varargs members).
struct KbOverlay {
    void Title(int line, const char* format, ...);   // 0x00447fa0
    void Print(int line, const char* format, ...);   // 0x00447f40
    char pad_0x0000[0x26c0];
    int lineCount;   // 0x26C0: next free line block
};
// 0x70-byte records behind KbGame+0x33fc; the first record's leading int selects the current one
// (0x0048fc80 reads records[records[0].field_0x0].field_0x28; tier 3).
struct KbTrackEntry {
    int field_0x0;
    char pad_0x04[0x24];
    int field_0x28;
    char pad_0x2C[0x70 - 0x2c];
};
struct KbGame {
    // The game object is polymorphic: 0x0048fc80 calls slot 20 (vtable +0x50) with a setting
    // name and a default and stores the int result (tier 1 slot, tier 3 names).
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
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
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual int UnknownVirtualSlot20(const char* name, int defaultValue);
    void GetStringText(int id, char* buffer, int size);   // 0x00521970: fetch text for a string id
    char pad_0x0004[0x4];
    KbGameCfg* field_0x8; // 0x8
    char pad_0x000C[0xC];
    int field_0x18; // 0x18
    char pad_0x001C[0x1c];
    KbOverlay* field_0x38; // 0x38: debug overlay (0 when off)
    char pad_0x003C[0x2f0 - 0x3c];
    float field_0x2f0; // 0x2F0: frame time (seconds)
    char pad_0x02F4[0x560 - 0x2f4];
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
    char pad_0x2D88[0x2eb8 - 0x2d88];
    int field_0x2eb8; // 0x2EB8
    char pad_0x2EBC[0x3334 - 0x2ebc];
    int field_0x3334; // 0x3334
    char pad_0x3338[0xC];
    char field_0x3344[0x3c]; // 0x3344  three 0x3c-byte blocks whose addresses 0x0048fc80 hands to the Bike loader
    char field_0x3380[0x3c]; // 0x3380
    char field_0x33bc[0x3c]; // 0x33BC
    char pad_0x33F8[0x4];
    KbTrackEntry* field_0x33fc; // 0x33FC
    char pad_0x3400[0x14];
    float fullNetPacketIntervalSec; // 0x3414
    float shortNetPacketIntervalSec; // 0x3418
    float fullRecordPacketIntervalSec; // 0x341C
    char pad_0x3420[0x8];
    int field_0x3428; // 0x3428
    char pad_0x342C[0x3444 - 0x342c];
    KbBonusTable* field_0x3444; // 0x3444
};
extern KbGame* g_kbGame;
// 0x004BFA80: millisecond clock (timeGetTime-based; tier 3).
unsigned int UnknownFunction4bfa80();
// 0x004B5D00 (cdecl): forward/up axes from roll, pitch and yaw (tier 3 names).
void UnknownFunction4b5d00(Vec3* forward, Vec3* up, float roll, float pitch, float yaw);
// 0x00460C70: 1/sqrt estimate (name from samples/physics/helpers/FastMath.bindings.json).
float FastInvSqrtEstimate(float v);
// Network smoothing settings 0x00493660 reads; 0x0048fc80 loads them from the game config.
extern int g_kbLatencyHiding;        // 0x0056CB3C
extern int g_kbRateLimiting;         // 0x0056CB40
extern int g_kbAllowWarping;         // 0x0056CB44
extern int g_kbUseLatencyThreshold;  // 0x0067C3A0
extern int g_kbUseExtrapLimit;       // 0x0056CB48
extern float g_kbExtrapLimit;        // 0x0056CB4C
extern float g_kbLatencyThreshold;   // 0x0056CB50
extern float g_kbWarpThreshold;      // 0x0056CB54
extern int g_kbUseTimeReceived;      // 0x0056CB58
extern int g_kbAllowNegative;        // 0x0056CB5C
extern int g_kbInterpolate;          // 0x0056CB60
extern float g_kbStallThreshold;     // 0x0056CB64  interval time below which slot 50 holds
extern float g_kbStallHoldSec;       // 0x0056CB68  length of that hold
// Track view behind KbGame+0x560 (tier 3): checkpoint count at +0xac.
struct KbTrack {
    char pad_0x00[0xac];
    int field_0xac;
    void UnknownFunction404df0(int a, int b, KrustyBike* bike);   // 0x00404DF0
};
// BikeWheel view for 0x00513560 (tier 3 name).
struct KbWheel { void SetRollDistance(float d); };

// Part table at Vehicle+0x1f0 -> +0xb4 (tier 3): count at +0, 0x44-byte records at +4.  A record
// with flag bit 3 clear owns a collision object at record+8 -> +0x3c.
struct KbPartObj { char pad_0x00[0x3C]; KbCollider* collider; };
struct KbPart { unsigned char flags; char pad_0x01[7]; KbPartObj* obj; char pad_0x0C[0x38]; };
struct KbPartList { int count; KbPart* items; };
struct KbTrackA {
    char pad_0x0000[0xA4];
    KbTrackB* field_0xa4; // 0xA4
    char pad_0x00A8[0xC];
    KbPartList* field_0xb4; // 0xB4
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
    void ReportTrickScore(KrustyBike* who, float score);   // 0x004e57d0: reports a trick score
};
// Object at KbRace+0xbc: 0x00495ff0 hands it the trick angle and multiplier.
struct KbScoreBoard { void Fn_0048D1E0(float angle, float multiplier); };
// Object at KbGame+0x3444 (bonus rules; packed: its ints sit at odd offsets).
#pragma pack(push, 1)
struct KbBonusTable {
    char pad_0x0000[0x40];
    int index;                   // +0x40 selects base[]
    char pad_0x0044[0x1229 - 0x44];
    int base[6];                 // +0x1229
    float baseScale;             // +0x1241
    char pad_0x1245[0x124d - 0x1245];
    float limitScale;            // +0x124d
};
#pragma pack(pop)

// Object reached through KrustyBike+0x740 (event/race context).
struct KbRaceSub { int* field_0x0; };
struct KbRace {
    KrustyBike* NextBike(int* cursor);   // 0x004204E0: next bike of the race (cursor starts at 0)
    char pad_0x0000[0x38];
    void* field_0x38; // 0x38
    char pad_0x003C[0x8];
    KbRaceHandler* handler; // 0x44
    KbRaceSub* field_0x48; // 0x48
    char pad_0x004C[0x4];
    KbRacer* field_0x50; // 0x50
    char pad_0x0054[0x48];
    float field_0x9c; // 0x9C   0x0048fc80 copies one of the three into KrustyBike+0x15dc by game type
    float field_0xa0; // 0xA0
    float field_0xa4; // 0xA4
    char pad_0x00A8[0x10];
    int field_0xb8; // 0xB8
    KbScoreBoard* field_0xbc; // 0xBC
    char pad_0x00C0[0x8];
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
    char pad_0x018B[0x1];
    char field_0x18c; // 0x18C  0x0048fc80: scene-node flag passed as 0 when set, 1 otherwise
    char pad_0x018D[0x1];
    char field_0x18e; // 0x18E
    char pad_0x018F[0xD];
    KbSensor* field_0x19c; // 0x19C
    char pad_0x01A0[0x1b8 - 0x1a0];
    float field_0x1b8; // 0x1B8  race clock in seconds (0x00493660 replays against it)
    char pad_0x01BC[0x1dc - 0x1bc];
    int field_0x1dc; // 0x1DC  GameObject slot 10 runs the base update when both are 4
    int field_0x1e0; // 0x1E0
    int field_0x1e4; // 0x1E4  -2 while the race has not started (0x00493660)
    char pad_0x01E8[0x3f8 - 0x1e8];
    char field_0x3f8; // 0x3F8  0x00493660: skips the wheel roll and the +0x478 flag
    char pad_0x03F9[0x1];
    char field_0x3fa; // 0x3FA  GameObject slot 10 calls 0x00492ad0 when set
    char field_0x3fb; // 0x3FB  0x00492670 records message 10 when set
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
struct KbMorphMod;
struct KbXform {
    void Fn_00444D80(KbObj128* who);   // 0x00444D80 (tier 3: attach/detach with a scene object)
    void Fn_00444DE0(KbObj128* who);   // 0x00444DE0 (D3DIMSoultreeObject; GameObject slot 10)
    Vec3 WorldToLocalDirection(const Vec3* v);
    // D3DIMSoultreeObject setters (names from src/reconstructed/D3DIMSoulTree.h): 0x0048fc80 calls
    // the first two with 0 or 1 on the bike and rider nodes and the third with the rider morph.
    void UnknownFunction444d00(int value);         // 0x00444D00
    void UnknownFunction444d40(int value);         // 0x00444D40
    void UnknownFunction444eb0(KbMorphMod* mod);   // 0x00444EB0
};
// Element of the array at Bike+0x12c (count at +0x130).
struct KbChild {
    int pad_0[1];
    int field_0x4;
    char pad_8[0x9c];
    int field_0xa4;
};
// CollisionObject (collision/) viewed by address: ignore list at +0x78/+0x7c.
struct KbCollider {
    void AddIgnoredOwner(void* owner);      // 0x00439410
    void RemoveIgnoredOwner(void* owner);   // 0x00439490
    void SetIgnoreListMode(int mode);       // 0x00439400
    void SetUseBroadphase(int enable);      // 0x00432120
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
    void Remove(KbObj128* who, int a);
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
    KbAnim* FindMotion(const char* name, int a);
    void SetMotion(int a);
    void Fn_004A8BF0(int a, float b);   // 0x004A8BF0 (ret 8): same as D3DIMSoultreeCharacter::Method_0x004a8bf0
    void ApplyRestPose();               // 0x004A8B00 (Character::ApplyRestPose)
};
struct KbA604 {
    void Fn_005305B0(KbAnim* a);
    void Fn_005305F0(KbAnim* a);
    void Fn_00530630(KbAnim* a);
    void Fn_00530680(BikeWheel* w);
    void Fn_00530680(KbObj128* body);   // same 0x00530680 with one of the two collision bodies
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
    void Fn_004A9E80(Vehicle* a, Vehicle* b, int c);   // 0x004A9E80: b is a vehicle (tested and read at +0x75c)
};
// Bounds/collision body at Vehicle::field_0x128 (canonical: CollisionObject::TestMeshBounds).
struct KbBody { int TestMeshBounds(KbBody* other); };

// ---- loader (0x0048fc80) support ----
// 0x5c-byte setup record the loader copies and forwards field by field (tier 2 layout, tier 3
// names): +0 -> KrustyBike+0x738, +4 -> +0x737, +8 is converted to float, +0x24 is passed by
// address, the rest by value.
struct KbBikeSetup {
    int field_0x0;
    char field_0x4;
    char pad_0x05[0x3];
    int field_0x8;
    // Floats: the loader's `setup ? setup->f : 0` arguments zero their temporaries with
    // immediates (not the zero register) and VC6 threads their branches as one group.
    float field_0xc;
    float field_0x10;
    float field_0x14;
    float field_0x18;
    float field_0x1c;
    float field_0x20;
    char field_0x24[0x2c];
    int field_0x50;
    int field_0x54;
    int field_0x58;
};
// 0x48-byte object built with 0x0047bb20 (src/reconstructed GhostMod1) for a ghost bike.
struct KbGhostMod { char pad_0x00[0x48]; KbGhostMod(int flags); };
// 0x58-byte rider morph (src/reconstructed MorphBastardModifier: ctor 0x004a3150, loader 0x004a33b0).
struct KbMorphMod {
    char pad_0x00[0x58];
    KbMorphMod(int flags);
    KbMorphMod* UnknownFunction4a33b0(void* a, const char* path, SoultreeObject* node);
};
// Bike's own loader 0x004079c0 (`ret 0xa0`, 40 argument dwords; not a vtable entry).  It is a
// Bike member in truth; vehicle/Bike.h is canonical, so it is reached through this view.
struct KbBikeLoader {
    GameObject* Fn_004079C0(int a1, int a2, const char* name, const SoultreeLoadDesc* desc, int a5, int a6,
                            Vec3 a7, Vec3 a8, Vec3 a9, int a10, VehicleInputMap* map, int a12, int a13,
                            int noSetup, void* setupName, int s50, int s54, int s58, float s0c, float s10,
                            float s14, float s18, float s1c, float s20, int a25, int a26, int a27, int localFlag,
                            void* t0, void* t1, void* t2, int a32, float setupFloat, int a34);
};
// 0x004a2fc0 (cdecl): zeroed debug-heap allocation of count x size bytes tagged with the source line.
void* DebugCalloc(unsigned int count, unsigned int size, const char* file, int line);

// Non-virtual callees of slot 11 (cdecl; call targets are relocation-masked).  Tier 3 names.
// 0x004B0AC0: probe at a position with three radii and a scale; returns a byte flag.
unsigned char Kb_004B0AC0(Vec3* pos, float r0, float r1, float r2, float scale,
                          int a, int b, int c, int d);
// 0x004B0DF0 (the query the base slot 11 issues) is FindObjectPlacement, declared in
// ../contact/ObjectPlacement.h.

#endif
