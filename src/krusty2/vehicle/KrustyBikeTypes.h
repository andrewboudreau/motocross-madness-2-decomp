// KrustyBikeTypes.h -- PROVISIONAL supporting types for the KrustyBike reconstruction.
//
// Everything here is tier-3 (provisional): member offsets are confirmed by decoded
// instructions in the KrustyBike overrides, but the names are inferred.  The base
// class is the canonical Bike (vehicle/Bike.h).  The Kb* structs below are area-local
// views of objects reached through inherited members whose canonical types are still
// generic; the overrides cast to them at the use site (samples/physics/hierarchy/MIGRATION.md rule 6).
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
struct KbBonusTable;
class KrustyBike;

// Vectors use the shared Math3D Vec3 (12 bytes: x,y,z at +0,+4,+8; tier 1 layout).
#include "../../reconstructed/TrackGameGlobal.h"
#include "vehicle/Bike.h"
#include "contact/ObjectPlacement.h"

class Terrain;

// A position on the race track (node, segment, t along the segment; the TrackPos of
// src/reconstructed/Track.h), copied as three dwords.  t is a float (0x00414370 adds
// 0.01 to it); the race reset writes the raw dwords.
struct KbRaw3 {
    int a;
    int b;
    union {
        int c;
        float t;
    };
};

// Object at KbGhost+0x34 / KbRace+0xc8 (tier 3 view): a point with a direction and its
// track position, which 0x00414370 places on the track when it is still unset.
struct KbGate {
    Vec3 position;          // +0x00
    Vec3 direction;         // +0x0c
    char pad_0x18[0x14];
    KbRaw3 trackPos;        // +0x2c
};

// AI racing line: up to 200 points.  Offsets tier 1 (0x0040e510, 0x00414370), names
// tier 3.  KrustyBike+0x838 holds one; 0x00414370 rebuilds it every frame.
#include "KrustyBikeRecords.h"

// 0x0067C348: a global zero vector copied by several overrides (three dword loads).
extern Vec3 g_kbZeroVec;

// Object reached through the global at 0x0056E26C (game/session singleton).
struct KbTrackRec { Vec3 field_0x00; char pad_0x0C[0xC]; };
struct KbMode { char pad_0x00[0x94]; int GetSelectedSeries(); };
struct KbGameCfg {
    char pad_0x0000[0x10]; int field_0x10;
    // 0x004AC830 (Net.cpp): send a message (type, data, size, destination id, flags); returns 1 on success.
    int Send(int type, void* data, int size, int dest, int flags);
};
// Object behind KrustyBike+0x13fc (tier 3); 0x004E8720 appends a record (type, id, data, flag).
struct KbRecorder { int QueueRecord(int type, int id, void* data, int flag); };
// Entity-level view used for the id at +0x11bc of any bike (tier 3, offset confirmed by decoded loads).
struct KbCell { char pad_0x00[8]; float field_0x8; char pad_0xc[4]; float field_0x10; };
struct KbNetBike {
    char pad_0x0000[0x11bc];
    int netId;  // +0x11BC  KrustyBike::netId seen through the entity view
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

// Received copy of message 1 (0x60 bytes, four at KrustyBike+0x11d8) that 0x00493660 reads.

// Message type 10 (8 bytes): the bike's +0x11c0 flag and +0x768.
struct KbBikePing {
    char field_0x00;
    unsigned char field_0x01;
    char pad_0x02[2];
    float field_0x04;
};
// The state 0x00492670 keeps of the last message (caller-owned; tier 3).

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
    KbGameCfg* network;  // +0x8  Game +0x8 network (NetworkInterface); set in network games, KrustyBike sends its packets through it
    char pad_0x000C[0xC];
    int playerCount;  // +0x18  Game +0x18 playerCount
    char pad_0x001C[0x1c];
    KbOverlay* debugOverlay;  // +0x38  Game +0x38 debugOverlay (0 when off)
    char pad_0x003C[0x2f0 - 0x3c];
    float frameTime;  // +0x2F0  Game +0x2f0 frameTime (seconds)
    char pad_0x02F4[0x560 - 0x2f4];
    KbTrackRec* bajaEnduroEvent;  // +0x560  TrackGame +0x560 bajaEnduroEvent (TrackGameViewOwner): [lapGate + 9] is gates[lapGate] at +0xd8, +0xac the gate count
    char pad_0x0564[0x4];
    KbRacer* tagEvent;  // +0x568  TrackGame +0x568 tagEvent (event type 4); its +0xa8 is compared with the bike
    char pad_0x056C[0x4];
    KbSession* eventManager;  // +0x570  TrackGame +0x570 eventManager (FindRaceMode/FindTextQueue)
    char pad_0x0574[0x4];
    KbMode mode;  // +0x578  TrackGame +0x578 mode (TrackGameMode; only the first 0x94 bytes are modelled here)
    int difficulty;  // +0x60C  TrackGame mode.difficulty (+0x578 + 0x94)
    char pad_0x0610[0x640];
    int field_0xc50; // 0xC50
    char pad_0x0C54[0x38C];
    int steeringSlider;                       // +0xFE0  OptionProcs.h UnknownOptControlSettings (TrackGame+0xfe0) +0x00: SteeringSlider
    int crossupSlider;                        // +0xFE4  UnknownOptControlSettings +0x04: CrossupSlider
    int pitchSlider;                          // +0xFE8  UnknownOptControlSettings +0x08: PitchSlider
    int pressThrottleSlider;                  // +0xFEC  UnknownOptControlSettings +0x0c: PressThrottleSlider
    int pressBrakesSlider;                    // +0xFF0  UnknownOptControlSettings +0x10: PressBrakesSlider
    int releaseThrottleSlider;                // +0xFF4  UnknownOptControlSettings +0x14: ReleaseThrottleSlider
    int releaseBrakesSlider;                  // +0xFF8  UnknownOptControlSettings +0x18: ReleaseBrakesSlider
    int feedbackSlider;                       // +0xFFC  UnknownOptControlSettings +0x1c: FeedbackSlider
    char pad_0x1000[0x1D70];
    int raceMode;  // +0x2D70  TrackGame mode.raceSettings.raceMode (+0x578 + 0x27f8)
    int eventType;  // +0x2D74  TrackGame mode.raceSettings.eventType: 3 SX, 4 Tag
    char pad_0x2D78[0xC];
    int riderCollision;  // +0x2D84  TrackGame mode.raceSettings.riderCollision
    char pad_0x2D88[0x2eb8 - 0x2d88];
    int stuntMode;  // +0x2EB8  TrackGame mode.raceSettings.stuntMode (ChkStuntMode)
    char pad_0x2EBC[0x3334 - 0x2ebc];
    int saveReplay;  // +0x3334  TrackGame mode.saveReplay
    char pad_0x3338[0xC];
    char steerBinding[0x3c];  // +0x3344  TrackGame +0x3344 steerBinding (LoadBike's steer)
    char leanBinding[0x3c];  // +0x3380  TrackGame +0x3380 leanBinding (LoadBike's lean)
    char throttleBinding[0x3c];  // +0x33BC  TrackGame +0x33bc throttleBinding (LoadBike's throttle)
    char pad_0x33F8[0x4];
    KbTrackEntry* controlSetup;  // +0x33FC  TrackGame +0x33fc controlSetup
    char pad_0x3400[0x14];
    float fullNetPacketIntervalSec; // 0x3414
    float shortNetPacketIntervalSec; // 0x3418
    float fullRecordPacketIntervalSec; // 0x341C
    char pad_0x3420[0x8];
    int playingReplay;  // +0x3428  TrackGame +0x3428 playingReplay
    char pad_0x342C[0x3444 - 0x342c];
    KbBonusTable* proCircuit;  // +0x3444  TrackGame +0x3444 proCircuit (ProCircuit.h)
};
// 0x004BFA80: millisecond clock (timeGetTime-based; tier 3).
unsigned int ReadClock();
// 0x004B5D00 (cdecl): forward/up axes from yaw, pitch and roll (tier 3 names; 0x00493660
// pushes the message +0x14 angle last).
void OrientationAxesFromAngles(Vec3* forward, Vec3* up, float yaw, float pitch, float roll);
// 0x00460C70: 1/sqrt estimate (name from src/reconstructed/FastMath.bindings.json).
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
    void SelectLapGates(int a, int b, KrustyBike* bike);   // 0x00404DF0 (TrackGameViewOwner: makes gates a and b the current pair)
};
// BikeWheel view for 0x00513560 (tier 3 name).
struct KbWheel { void SetRollDistance(float d); };

// Part table at Vehicle+0x1f0 -> +0xb4 (tier 3): count at +0, 0x44-byte records at +4.  A record
// with flag bit 3 clear owns a collision object at record+8 -> +0x3c.
struct KbPartObj { char pad_0x00[0x3C]; CollisionObject* collider; };  // +0x3c: AddIgnoredOwner 0x00439410 is called on it
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
    int typing;  // +0x3DC  BikeRace chatOverlay +0x3dc typing (BikeRace.h); blocks the bike's input (slot 84)
};

struct KbRacer {
    char pad_0x0000[0xA8];
    void* field_0xa8; // 0xA8
    char pad_0x00AC[0x198];
    int cameraState;  // +0x244  BikeRace raceCamera +0x244 cameraState (BikeRace.h)
    char pad_0x0248[0x168];
    void* followedBike;  // +0x3B0  BikeRace raceCamera +0x3b0 followedBike (BikeCamera's bike)
    void* followedRacer;  // +0x3B4  BikeRace raceCamera +0x3b4 followedRacer (KrustyBikeCamera's racer)
};

struct KbRaceHandler {
    void Fn_004DE580(void* who, float a, int b);
    void ReportTrickScore(KrustyBike* who, float score);   // 0x004e57d0: reports a trick score
};
// Object at KbRace+0xbc: 0x00495ff0 hands it the trick angle and multiplier.
struct KbScoreBoard { void ShowValue(float points, float multiplier); };
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
// Work-list entry of the track's graph walks (src/reconstructed/Track.h TrackListItem,
// calloc(1, 0x10)); 0x00414370 builds a two-entry list starting at the track's first node.
struct KbTrackItem {
    int field_0x00;
    int* field_0x04;               // node
    KbTrackItem* field_0x08;
    KbTrackItem* field_0x0c;       // next
};
// The race's Track (src/reconstructed/Track.h; tier 3 view): field_0x0 is the start node.
// Positions on the track are KbRaw3 {node, segment, t} passed by value.
struct KbRaceSub {
    int* startNode;  // +0x0  Track+0x00 startNode (KbRaceSub is the Track at BikeRace+0x48); slot 43 seeds the ghost block from it
    int ClosestPointOnNode(Vec3 p, int* node, KbRaw3* out, float* outDistance);
    int PlaceNear(Vec3 p, KbRaw3 from, float distance, float range, int path,
                              KbRaw3* out);
    int FreeWorkList(KbTrackItem** list, int all);
    int ShortestPath(KbRaw3 a, KbRaw3 b, KbTrackItem** path, float* distance);
    float DistanceAlong(KbRaw3 a, KbRaw3 b);
    int MoveAlongPath(KbRaw3 pos, KbRaw3* out, KbTrackItem** path, float distance,
                      unsigned char flags);
    int PointAt(KbRaw3 pos, Vec3* out);
    int SegmentDirection(int segment, Vec3* out);
    int DistanceToStripEdge(Vec3 p, int segment, float* out, int mode);
};
struct KbRace {
    KrustyBike* NextBike(int* cursor);   // 0x004204E0: next bike of the race (cursor starts at 0)
    char pad_0x0000[0x38];
    void* localRacer;  // +0x38  BikeRace+0x38 localRacer; KrustyBike compares it with this
    char pad_0x003C[0x8];
    KbRaceHandler* handler; // 0x44
    KbRaceSub* raceTrack;  // +0x48  BikeRace+0x48 raceTrack (Track methods 0x00516ca0.. are called on it)
    Terrain* raceTerrain;  // +0x4C  BikeRace+0x4c raceTerrain; terrain passed to the look-ahead simulation 0x0040eca0
    KbRacer* raceCamera;  // +0x50  BikeRace+0x50 raceCamera (+0x244 cameraState, +0x3b0/+0x3b4 followed bike/racer)
    char pad_0x0054[0x48];
    float aiStuntChance1;  // +0x9C  BikeRace aiSettings[8] (10): AI KrustyBike+0x15dc at difficulty 1; BikeAI rolls rand*100 <= it
    float aiStuntChance2;  // +0xA0  BikeRace aiSettings[9] (50): AI KrustyBike+0x15dc at difficulty 2
    float aiStuntChance3;  // +0xA4  BikeRace aiSettings[10] (100): AI KrustyBike+0x15dc at difficulty 3 and 4
    char pad_0x00A8[0x10];
    int forceAiStep;  // +0xB8  BikeRace+0xb8; slot 49 takes the BikeAI step 0x00413200 when aiKind or this is set
    KbScoreBoard* scoreBoard;  // +0xBC  BikeRace+0xbc scoreBoard (BonusObjectManager counter)
    char pad_0x00C0[0x8];
    KbGate* gateList;  // +0xC8  BikeRace+0xc8 gateList; slot 43 points the ghost block at the first gate
    char pad_0x00CC[0x2C];
    int startProbeNode;  // +0xF8  BikeRace startProbe.trackPos.node (+0xcc+0x2c); slot 43 copies it when hasStartProbe
    int startProbeSegment;  // +0xFC  BikeRace startProbe.trackPos.segment
    int startProbeT;  // +0x100  BikeRace startProbe.trackPos.t
    char pad_0x0104[0x30];
    KbRaw3 finishProbePos;  // +0x134  BikeRace finishProbe.trackPos (+0x108+0x2c); the race-kind 2/3 path search runs to it (0x00414370)
    char pad_0x0140[0x4];
    int hasStartProbe;  // +0x144  BikeRace+0x144 hasStartProbe; gates the copy of the start probe position
    char pad_0x0148[0x41];
    char startBeforeFinish;  // +0x189  BikeRace+0x189; picks string 0x14d6/0x14d7 at lap 1
    char isRacing;  // +0x18A  BikeRace+0x18a isRacing
    char pad_0x018B[0x1];
    char forceHighLod;  // +0x18C  BikeRace+0x18c "ForceHighLOD"; 0x0048fc80 keeps full detail when set
    char pad_0x018D[0x1];
    char networkStartPending;  // +0x18E  BikeRace+0x18e networkStartPending
    char pad_0x018F[0xD];
    KbSensor* chatOverlay;  // +0x19C  BikeRace+0x19c chatOverlay; its +0x3dc typing blocks the bike's input (slot 84)
    char pad_0x01A0[0x1b8 - 0x1a0];
    float replayTime;  // +0x1B8  BikeRace+0x1b8 replayTime (seconds); 0x00493660 replays against it
    char pad_0x01BC[0x1dc - 0x1bc];
    int replayMode;  // +0x1DC  BikeRace+0x1dc replayMode; slot 10 runs the base update when it and +0x1e0 are 4
    int appliedReplayMode;  // +0x1E0  BikeRace+0x1e0 appliedReplayMode
    int lastRecordKind;  // +0x1E4  BikeRace+0x1e4: RecordFrame's last record kind (-2 after the tape end)
    char pad_0x01E8[0x3f8 - 0x1e8];
    char replayPaused;  // +0x3F8  BikeRace+0x3f8 replayPaused; 0x00493660 skips the wheel roll and the +0x478 flag
    char pad_0x03F9[0x1];
    char recorderTickDue;  // +0x3FA  BikeRace+0x3fa recorderTickDue; slot 10 calls 0x00492ad0 when set
    char scoreRecordDue;  // +0x3FB  BikeRace+0x3fb; 0x00492670 records message 10 (score) when set
};
// Record at KrustyBike+0x744 (tier 3): per-bike state block reset by slot 43.
struct KbGhost {
    char pad_0x00[0x8];
    float field_0x08;   // 0x00414370 compares it with 300.0f
    int field_0x0c, field_0x10, field_0x14, field_0x18;
    Vec3 field_0x1c;
    Vec3 field_0x28;
    KbGate* field_0x34;
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
    Vec3 WorldToLocalDirection(const Vec3* v);
    // D3DIMSoultreeObject setters (names from src/reconstructed/D3DIMSoulTree.h): 0x0048fc80 calls
    // these with 0 or 1 on the bike and rider nodes.
};
// Element of the array at Bike+0x12c (count at +0x130).
struct KbChild {
    int pad_0[1];
    int field_0x4;
    char pad_8[0x9c];
    int field_0xa4;
};
// Opaque storage view of the ghost modifier; allocation/calls use GhostMod1.
struct KbObj128 {
};


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


// Session object at KbGame+0x570; 0x0045D2B0 returns the local player record (tier 3 names).
struct KbMessage;
struct KbMsgSink { void ReplaceHeadMessage(KbMessage* m); };
struct KbSession { KbPlayer* FindRaceMode(); KbMsgSink* FindTextQueue(); };
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
// 0x58-byte rider morph (src/reconstructed MorphBastardModifier: ctor 0x004a3150, loader 0x004a33b0).
struct KbMorphMod {
    char pad_0x00[0x58];
    KbMorphMod* LoadParameterFile(void* a, const char* path, SoultreeObject* node);
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
