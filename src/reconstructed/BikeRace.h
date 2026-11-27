#pragma once

#include <float.h>

#include "FollowCamera.h"
#include "KrustyBikeCamera.h"
#include "D3DIMSoulTree.h"
#include "DebugAlloc.h"
#include "DebugOverlay.h"
#include "ObjectPicker.h"
#include "InGameProcs.h"
#include "JoystickDevice.h"
#include "KeyboardDevice.h"
#include "MatrixUtil.h"
#include "RaceStatus.h"
#include "Recorder.h"
#include "SceneManager.h"
#include "Track.h"
#include "TrackGame.h"
#include "Wrecker.h"

// Reconstruction of part of D:\aardvark\VC\krusty2\bikerace.cpp (literal
// __FILE__ at 0x00567c74; bikerace.h's at 0x00567de0).
//
// Confirmed (RTTI): BikeRace : GraphicsTest : GameObject : BaseObject, all
// single non-virtual inheritance; vtable 0x00550d24 (COL 0x0055ac68). It
// overrides slot 0 (deleting destructor 0x00417eb0, core 0x0041cf30) and
// slots 10, 14, 16, 20, 22, 23 and 24. The constructor 0x00417c30 is the
// only writer of the vtable besides the destructor.
//
// Strong inference: RaceView.h's EventBikeRaceView is a view of the same
// object (EventManager 0x0045d2f0 returns it; its fields +0x38..+0x3f9 are
// the ones BikeRace's constructor initialises, and its non-virtual methods
// 0x0041f1d0..0x00423790 lie in this file). It keeps its own symbols because
// many reconstructed callers bind them.
//
// Extent (strong inference): the BikeRace code runs from the constructor's
// helpers at 0x00417b00 to slot 22 (0x00423880..0x004238bb); GR_BitString's
// code (0x004238c0..0x00423f6d) and BlockAllocator.cpp follow. Whether the
// plate number painter (0x00417500..0x00417aff, between BikeCamera's code
// and 0x00417b00) belongs to this file is open. Member names are
// provisional.

class CollisionObject;  // collision/CollisionObject.h (src/krusty2)
class KrustyBike;  // vehicle/KrustyBikeCore.h (src/krusty2)
class ProjectedShadow;  // shadow/ProjectedShadow.h (src/krusty2)
class Terrain;  // broadphase/Terrain.h (src/krusty2)
class LightManager;
struct TextureContext;
struct BikeEngineSetup;
struct LodBikeRaceView;
struct ReceivedRacerState;
struct RacerDeltaMessage;

// One of KrustyVCR's eight 8-byte records at +0x25c (cleared by its inline
// constructor).
struct KrustyVcrGridEntry {
    KrustyVcrGridEntry() {
        networkPlayerId = 0;
        points = 0;
    }
    int networkPlayerId;  // +0x0  same 8-byte grid entry as TrackGame.h GridEntry (+0 compared with racers' networkPlayerId)
    char points;  // +0x4  same grid entry: TrackGame.h GridEntry::points (championship points)
};

// One racer of a recording (KrustyVCR+0x300, 0x124 bytes; 0x0049c070 writes
// one and 0x0049c1e0 reads it back). The strings are copied with their
// lengths clamped to the array size less one.
struct KrustyVcrRacerRecord {
    int networkId;                            // +0x000 network id
    char aiIndex;                             // +0x004 AI index
    char name[0x10];                          // +0x005 name
    char bikeFile[0x40];  // +0x15  SetRacer model argument (Res\Ghost.mcf); the loader reads it back into bikeFile
    char riderFile[0x40];  // +0x55  SetRacer rider argument (Res\GhostRider.mcf); the loader reads it back into riderFile
    char bikeName[0x40];  // +0x95  bike name; the loader passes it back to KrustyUI::ResolveBikeFile
    char riderName[0x40];  // +0xD5  rider name; the loader passes it back to KrustyUI::ResolveRiderFile
    int engineSize;                           // +0x118 engine size
    int fourStroke;                           // +0x11c four-stroke flag (KrustyUI "BikeFourStroke_%02d")
    int plateNumber;  // +0x120  SetRacer's last argument is TrackGameMode plateNumber / slots[j].plateNumber; read back into plate
};

// RTTI: KrustyVCR : VCRInterface (vtable 0x00550e9c, 0xf8c bytes). Its
// constructor is inline at bikerace.cpp's `new` sites (lines 0x2b9, 0x10e7,
// 0x1103 and others), which also emit its implicit destructor 0x00419960
// and the deleting wrapper 0x00419940. BikeRace keeps the replay being
// recorded or played at +0x1a0 and the ghost at +0x1a4. Its methods sit
// after krustyui.cpp's code; only the members used here are declared.
class KrustyVCR : public VCRInterface {
public:
    KrustyVCR() {}

    // 0x0049bf10: opens `name` in `file` through `callback` for recording
    // (mode 0) or playback (mode 1); 0 on failure.
    int OpenRecording(RecorderCallbackProc callback, int mode, char* name,
                              VCRFile* file);
    float RecordedLength();            // 0x0049c000: the recorded length
    void SetReplayLength(float time, int flag); // 0x0049bff0
    // 0x0049c010: saves the recording with its length and description.
    void SaveRecording(float length, char* description, int flag);
    // 0x0049c070: describes a racer of the recording: its network id, AI
    // index (a char), record slot, name, model files, bike and rider names,
    // engine size, four-stroke flag and plate number. GetRacer (0x0049c1e0) copies
    // record `index`'s fields back out.
    void SetRacer(int id, char ai, int slot, char* name, char* model,
                               char* rider, char* bikeName, char* riderName, int engineSize,
                               int fourStroke, int value);
    void GetRacer(int* id, char* ai, int index, char* name, char* model, char* rider,
                               char* bikeName, char* riderName, int* engineSize, int* fourStroke,
                               int* value);

    float replayLength;                       // +0x0d8 SetReplayLength (BikeRace slot 10 each frame)
    // +0xdc..+0xf8c (0xeb0 bytes) is the block 0x0049bf10 and 0x0049c010 hand
    // to the recorder.
    char magic[8];                            // +0x0dc "MCMVCR" (0x0049bf10, mode 0)
    char version[8];                          // +0x0e4 version "2.0" plus 0x1a (the string at 0x0056da64)
    char description[0x20];                   // +0x0ec description (0x0049c010)
    float timePlayed;                         // +0x10c the time played (0x00421d50)
    // +0x110 TrackGame+0x2d70's 0x1ec bytes, the race settings from
    // raceMode on (copied by 0x0049bf10, mode 0).
    unsigned char raceSettings[0x25c - 0x110];
    KrustyVcrGridEntry gridEntries[8];  // +0x25C  raceSettings+0x14c (TrackGame+0x2ebc) copied with the settings: per grid slot player id (TrackGame.h GridEntry)
    unsigned char field_0x29c[0x2fc - 0x29c];
    int racerCount;                           // +0x2fc racers recorded (the loader)
    KrustyVcrRacerRecord racers[11];         // +0x300 0x0049c070 / 0x0049c1e0
};

// Shared chat overlay stored at BikeRace+0x19c and by the race events.
class ChatOverlay;

struct BikeRaceRiderView {
    unsigned char field_0x000[0x1a0];
    D3DIMSoultreeObject* modelNode;            // +0x1a0 the rider's model (SetDetailLevel 0x00423790)
};

class BikeRace;

// A bike's engine setup (0x5c bytes): the loader copies TrackGameMode
// +0xfd8 (TrackGame+0x1550) and fills it from KrustyUI's garage tables for
// the bike class of the engine size.
struct BikeEngineSetup {
    int engineSize;                           // +0x00 engine size (racer +0x738)
    int fourStroke;                           // +0x04 four-stroke flag (KrustyUI "BikeFourStroke_%02d") (racer +0x737)
    int weight;                               // +0x08 KrustyUI "Weight"
    unsigned char field_0x0c[0x24 - 0x0c];
    int presetCurve[11];                      // +0x24 KrustyUI's preset 0 curve
    int rpmLowerLimit;                        // +0x50 "RPMLowerLimit"
    int rpmUpperLimit;                        // +0x54 "RPMUpperLimit"
    int rpmStep;                              // +0x58 a tenth of their range
};

// The object at a racer's +0x740 (its view, BikeRace): +0x18c set keeps the
// racer's model at full detail when the camera follows another one.
struct LodBikeRaceView {
    unsigned char field_0x000[0x18c];
    char forceHighLod;                        // +0x18c BikeRace's "ForceHighLOD"
};

// A racer's network state (0x58 bytes, network message 1).
struct RacerStateMessage {
    char type;                                // +0x00 SendRaw (0x004ad280) writes the type byte here
    char finishPosition;  // +0x1  KbBikeMessage+0x01 finishPosition (the sender's message 1 layout)
    short elapsedMs;                          // +0x02 KbBikeMessage+0x02 elapsedMs
    short lapCount;                           // +0x04 KbBikeMessage+0x04 lapCount
    short gatesPassed;                        // +0x06 KbBikeMessage+0x06 gatesPassed
    Vector3 position;  // +0x8  KbBikeMessage+0x08 position; copied to racer deltaBasePosition (+0x1364)
    int roll;  // +0x14  KbBikeMessage+0x14 roll; copied raw to racer +0x137c
    int pitch;  // +0x18  KbBikeMessage+0x18 pitch; copied raw to racer +0x1380
    int yaw;  // +0x1C  KbBikeMessage+0x1c yaw; copied raw to racer +0x1384
    float lastLapTime;                        // +0x20 KbBikeMessage+0x20 lastLapTime
    unsigned char field_0x24[0x28 - 0x24];
    float fastestLap;  // +0x28  RecordFrame copies it to racer fastestLap (+0x750, as KbBikeMessage+0x28)
    int timeBehindLeader;  // +0x2C  KbBikeMessage+0x2c timeBehindLeader; replayed into racer timeBehindLeader (+0x770)
    Vector3 angularVelocity;  // +0x30  KbBikeMessage+0x30 angularVelocity; copied to racer +0x1370
    Vector3 velocity;  // +0x3C  KbBikeMessage+0x3c velocity; copied to racer +0x1358
    unsigned char field_0x48[0x4c - 0x48];
    int time;  // +0x4C  KbBikeMessage+0x4c time (ms); RecordFrame overwrites it with replay seconds * 1000
    unsigned char field_0x50[0x54 - 0x50];
    char racerIndex;                          // +0x54 racer index
    unsigned char field_0x55[0x58 - 0x55];
};

// Network message 13 (KrustyBike.cpp's 0x004933e0 applies it).
struct RacerDeltaMessage {
    unsigned char field_0x00[0x16];
    char racerIndex;                          // +0x16 racer index
};

// Network message 10: a racer's score.
struct RacerScoreMessage {
    char type;                                // +0x00 SendRaw (0x004ad280) writes the type byte here
    char racerIndex;                          // +0x01 racer index
    unsigned char field_0x02[0x04 - 0x02];
    float racerScore;  // +0x4  message 10 (KbBikePing+0x04 = KrustyBike+0x768 score); copied to racer score
};

// Network messages 0x84 (a time stamp at +4) and 0x85 (text at +4).
struct TimeStampMessage {
    int header;                               // +0x00 SendRaw (0x004ad280) writes the type byte here and the sequence at +0x02
    unsigned int timeStamp;                   // +0x04 sender's time stamp
};

// One of a racer's three received states (+0x11c8, newest first).
struct ReceivedRacerState {
    RacerStateMessage state;            // +0x00 the received state
    int messageFlags;                         // +0x58 the message's flags
    int fresh;                                // +0x5c fresh
};



#include "BikeRaceNode.h"

#include "RaceGateEvent.h"

// BikeRace+0x64/+0x6c (StartGate.slb, 30SecMan.slt) and the camera's
// character target are scene CollisionCharacters (Scene::FindEntryByFileName,
// SceneAnimation::character); their GameObject part, whose +0x25 isEnabled
// bit this file tests, is a virtual base reached through the vbptr at +4. The
// colliders at +0x210 are CollisionObjects (GameObject part at +0xc).

// The members of BikeRace+0x50's KrustyBikeCamera that bikerace.cpp
// reads and writes (FollowCamera's +0x244 and +0x274, VehicleCamera's
// +0x384..+0x394, BikeCamera's +0x3b0 and KrustyBikeCamera's +0x3b4, which
// those headers keep protected and type by their own readers). The racer
// pointers are the racers this file follows.
struct BikeRaceKrustyBikeCameraView {
    unsigned char field_0x000[0x244];
    int cameraState;                          // +0x244 camera state
    unsigned char field_0x248[0x268 - 0x248];
    int overrideActive;                       // +0x268 override active
    unsigned char field_0x26c[0x274 - 0x26c];
    unsigned char isSettled;  // +0x274  FollowCamera isSettled (the comment above names FollowCamera's +0x274)
    unsigned char field_0x275[0x384 - 0x275];
    CollisionCharacter* targetCharacter;  // +0x384  VehicleCamera+0x384 target; TargetSceneCharacter stores its scene character
    CarProcedural* targetCar;  // +0x388  VehicleCamera+0x388 targetCar; TargetSceneObject stores its scene procedural object
    void* shadowCaster;                       // +0x38c a scene shadow caster
    unsigned char followingRacer;             // +0x390 following a racer
    unsigned char targetPointCached;          // +0x391 VehicleCamera 0x0052bc50; cleared on a new target
    unsigned char targetHeadingCached;        // +0x392 VehicleCamera 0x0052bac0; cleared on a new target
    KrustyBike* vehicle;            // +0x394 VehicleCamera's followed vehicle
    Vector3 cachedTargetHeading;              // +0x398 VehicleCamera cachedTargetHeading (targetHeadingCached)
    Vector3 cachedTargetPoint;                // +0x3a4 VehicleCamera cachedTargetPoint (targetPointCached)
    KrustyBike* followedBike;       // +0x3b0 BikeCamera's bike (RaceSound.h)
    KrustyBike* followedRacer;      // +0x3b4 KrustyBikeCamera's racer (RaceSound.h)

    // Inline (three times in 0x0041eb20): follows `racer`, showing the
    // previous racer's model again and hiding the new one's (BikeRace.cpp).
    void StartFollowing(KrustyBike* racer);

    // Inline (0x0041eb20): the camera targets one scene object instead.
    // Each of the three stores its own target first (the store order is
    // what makes AimCamera 0x0041eb20 exact).
    void TargetShadowCaster(void* caster) {
        shadowCaster = caster;
        isSettled = 0;
        targetPointCached = 0;
        targetHeadingCached = 0;
        followingRacer = false;
        targetCharacter = 0;
        targetCar = 0;
    }
    void TargetSceneCharacter(CollisionCharacter* character) {
        targetCharacter = character;
        isSettled = 0;
        targetPointCached = 0;
        targetHeadingCached = 0;
        followingRacer = false;
        targetCar = 0;
        shadowCaster = 0;
    }
    void TargetSceneObject(CarProcedural* object) {
        targetCar = object;
        isSettled = 0;
        targetPointCached = 0;
        targetHeadingCached = 0;
        followingRacer = false;
        targetCharacter = 0;
        shadowCaster = 0;
    }
};

// The replay recorder's record kinds -1 (a tick) carry the time played and
// the tick count.
struct VcrTickRecord {
    float time;                               // +0x00 recorded length at the tick
    int tickCount;                            // +0x04 ticks recorded so far
};

// Record kind 0x11: the tag ball's state (0x3c bytes). Kind 0x10's deltas
// (TagBallStateDelta) accumulate into the ball's copy of it.
struct TagBallState {
    short header;                             // +0x00 the message header: SendRaw (0x004ad280) writes the type byte here
    unsigned short timeStep;                  // +0x02 time step (0x004a9aa0)
    Vector3 position;                         // +0x04  ball +0x5e0; the delta's position, which KrustyBike 0x004933e0 adds to deltaBasePosition. Tier 3
    float roll;                               // +0x10  ball +0x5f8; angles[0] x pi/64, as KrustyBike's deltaBaseRoll (0x004933e0). Tier 3
    float pitch;                              // +0x14  ball +0x5fc; angles[1], as KrustyBike's deltaBasePitch. Tier 3
    float yaw;                                // +0x18  ball +0x600; angles[2], as KrustyBike's deltaBaseYaw. Tier 3
    Vector3 angularVelocity;                  // +0x1C  ball +0x5ec; angularVelocity x pi/64, as KrustyBike's deltaBaseAngularVelocity. Tier 3
    Vector3 velocity;                         // +0x28  ball +0x5d4; the delta's velocity, which KrustyBike 0x004933e0 adds to deltaBaseVelocity. Tier 3
    int runningTime;                          // +0x34 ball +0x604
    int field_0x38;
};

// Record kind 0x10: byte deltas of the ball's state, 15/64 per unit for the
// vectors at +0x01 and +0x0b and pi/64 for the rest (the same layout as the
// network message 13 that KrustyBike.cpp's 0x004933e0 applies).
struct TagBallStateDelta {
    char type;                                // +0x00 the message type byte (as KbNetDelta::type)
    signed char position[3];                  // +0x01 x 15/64 -> ball +0x5e0
    unsigned char step;                       // +0x04 bit 0: units of 8, bits 1-7: count
    signed char angles[3];                    // +0x05 roll, pitch, yaw x pi/64 -> ball +0x5f8..+0x600
    signed char angularVelocity[3];           // +0x08 x pi/64 -> ball +0x5ec
    signed char velocity[3];                  // +0x0b x 15/64 -> ball +0x5d4
};

// One of the ball's four received states (newest first).
struct TagBallReceivedState {
    TagBallState record;                      // +0x00 the kind 0x11 record
    int recordTime;                           // +0x3c the record's time
    int fresh;                                // +0x40 fresh
};

// Record kind 0x87: the racer the view follows (by its +0x11bc id).
struct VcrFollowRecord {
    int header;                               // +0x00 SendRaw (0x004ad280) writes the type byte here and the sequence at +0x02
    char request;                             // +0x04 KbNetPacket::request: 0 for message 0x87 (0x004a9d20 clears it)
    unsigned char field_0x05[0x08 - 0x05];
    int followedRacerId;  // +0x8  FollowRacer sends followedRacer->networkPlayerId; RecordFrame matches it to racer recorderId
};

// TrackGame+0x568's +0xdc object as the replay recorder callback reaches it
// (TrackGame.h keeps the type for 0x004a9d10's symbol): the Tag event's ball.
class TagBall {
public:
    // 0x004a9aa0 (FollowRacer.cpp): applies record kind 0x10 to a state (as
    // 0x004933e0).
    void ApplyReplayDelta(const TagBallStateDelta* delta, TagBallReceivedState* part);

    unsigned char field_0x000[0x4f8];
    int hasState;                             // +0x4f8 has a state
    TagBallReceivedState* receivedStates[4];  // +0x4fc newest first (0x00421d50 rotates them)
    unsigned char field_0x50c[0x5d4 - 0x50c];
    Vector3 velocity;                         // +0x5D4  record +0x28 (see TagBallState). Tier 3
    Vector3 position;                         // +0x5E0  record +0x04. Tier 3
    Vector3 angularVelocity;                  // +0x5EC  record +0x1c (see TagBallState). Tier 3
    float roll;                               // +0x5F8  record +0x10. Tier 3
    float pitch;                              // +0x5FC  record +0x14. Tier 3
    float yaw;                                // +0x600  record +0x18. Tier 3
    int runningTime;                          // +0x604 running time
};

// KrustyUI+0x48 records (0xc8 bytes; slot 10's debug page names the
// player's bike model from +0x40) and the KrustyUI+0x50 records (0x94
// bytes) whose +0 indexes them.
struct BikeRaceManufacturerRecord {
    unsigned char field_0x00[0x40];
    char modelFile[0xc8 - 0x40];  // +0x40  KrustyUI entry "ModelFile"/"File" (KrustyUIModelRecord::modelFile)
};

struct BikeRaceBikeRecord {
    int manufacturer;  // +0x0  KrustyUI bike entry +0: index into the manufacturers (+0x48)
    unsigned char displayName[0x48 - 0x04]; // +0x04: KrustyUIBikeRecord::displayName, "BikeName_%02d" (bytes kept)
    char texture[0x40];                       // +0x48 KrustyUIBikeRecord texture ("BikeTexture_%02d"); CreateRacers resolves it to the AI bike's texture
    int engineSizeSelectable;                 // +0x88 KrustyUIBikeRecord engineSizeSelectable
    int engineSize;                           // +0x8c engine size
    int fourStroke;                           // +0x90 four-stroke flag (KrustyUI "BikeFourStroke_%02d")
};

// The highlighted row of the "Soultree LOD (%s)" debug page (0x00567a88):
// SetRowHighlight marks row + 1, the arrow keys act on it and Enter (0x1c)
// advances it, wrapping back to 2.
extern int g_LodDebugSelectedRow;

#include "BikeRaceCore.h"

inline void BikeRace::FreeGateList() {
    if (gateList != 0) {
        while (gateList != 0) {
            RaceGate* node = gateList;
            gateList = node->nextGate;
            DebugFree(node, __FILE__, 0xd3);
        }
        gateList = 0;
    }
}

// 0x00417b00: the debug picker's hit callback (slot 23 passes it to
// 0x004b0210); stores what was hit in the current race's picker.
void PickerHitCallback(CollisionObject* self, CollisionObject* other);
// Debug line end points and their flags, drawn by slot 14 (red 0xff when
// the flag is set, else green 0xff). BikeAI.cpp's obstacle query 0x00415640 stores
// its segment and four copies of it (0x00415ac5..0x00415bdb), clears the
// five flags and sets each one when that segment touches an object
// (0x004161e8..0x00416659); RaceStatus.cpp (0x004e5b38..) writes the gate
// lines and BikeAI's path look-ahead 0x0040d200 (0x0040e30c..0x0040e347)
// the target line.
extern int g_ObstacleDebugUpperHit;         // 0x00577a00
extern int g_ObstacleDebugHit;              // 0x00578da8
extern int g_ObstacleDebugLowerHit;         // 0x00578dac
extern int g_ObstacleDebugLeftHit;          // 0x00578df8
extern int g_ObstacleDebugRightHit;         // 0x00578dd8
extern Vector3 g_ObstacleDebugUpperLine[2]; // 0x00578de0 the segment raised by 3
extern Vector3 g_ObstacleDebugLine[2];      // 0x00577a30 the segment
extern Vector3 g_ObstacleDebugLowerLine[2]; // 0x00577a48 the segment lowered by 1.5
extern Vector3 g_ObstacleDebugLeftLine[2];  // 0x005779d8 the segment moved 3 against its side vector
extern Vector3 g_ObstacleDebugRightLine[2]; // 0x00577a18 the segment moved 3 along it
extern Vector3 g_GateDebugBoxLine[2];
extern Vector3 g_GateDebugDirectionLine[2];
extern Vector3 g_PathTargetDebugLine[2];    // 0x00578dc0 position to path target, both raised by 3

// 0x004230e0 (cdecl; an RecorderCallbackProc, its address is passed at
// 0x0041918a and later): the replay time in milliseconds, then 0x00421d50.
int RecorderCallback(int kind, void* data, int flag, int recorderId, int* keep, int time,
                     int* milliseconds);
// 0x00417b30 (cdecl): sorts `keys` in descending order, moving `values`
// along with them.
void SortDescending(int* keys, int* values, int count);
