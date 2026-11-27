// KrustyBikeTypes.h -- PROVISIONAL supporting types for the KrustyBike reconstruction.
//
// Everything here is tier-3 (provisional): member offsets are confirmed by decoded
// instructions in the KrustyBike overrides, but the names are inferred.  The base
// class is the canonical Bike (vehicle/Bike.h).  The Kb* structs below are area-local
// views of objects reached through inherited members whose canonical types are still
// generic; the overrides cast to them at the use site (samples/physics/hierarchy/MIGRATION.md rule 6).
#ifndef KRUSTYBIKE_TYPES_H
#define KRUSTYBIKE_TYPES_H

struct KbSession;
struct KbPlayer;
struct KbBody;
struct KbSink;
struct KbXform;
struct KbChild;
struct KbObj128;
struct KbBonusTable;
class KrustyBike;

// Vectors use the shared Math3D Vec3 (12 bytes: x,y,z at +0,+4,+8; tier 1 layout).
#include "../../reconstructed/TrackGameGlobal.h"
#include "vehicle/Bike.h"
#include "contact/ObjectPlacement.h"

class Terrain;

// AI racing line: up to 200 points.  Offsets tier 1 (0x0040e510, 0x00414370), names
// tier 3.  KrustyBike+0x838 holds one; 0x00414370 rebuilds it every frame.
#include "KrustyBikeRecords.h"

// 0x18-byte gate records of the Baja/Enduro owner (TrackGame::bajaEnduroEvent): [lapGate + 9]
// is gates[lapGate] at +0xd8 (tier 2 offsets, view name).
struct KbTrackRec { Vec3 position; char pad_0x0C[0xC]; };
// Object behind KrustyBike+0x13fc (tier 3); 0x004E8720 appends a record (type, id, data, flag).
struct KbRecorder { int QueueRecord(int type, int id, void* data, int flag); };
// Entity-level view used for the id at +0x11bc of any bike (tier 3, offset confirmed by decoded loads).
struct KbNetBike {
    char pad_0x0000[0x11bc];
    int netId;  // +0x11BC  KrustyBike::netId seen through the entity view
};
// 12-byte message built by KrustyBike::SendFollowedRacer (tier 3: the leading dword is never written).
// Network message 13 (bikerace.cpp's 0x0041fb.. handlers pass it to 0x004933e0):
// signed byte deltas of the velocity, position, angular velocity and angles (the
// fields KbBikeDeltaMessage encodes at the same offsets) plus two raw ints at the unaligned
// offsets +0x0e/+0x12 (tier 2 layout, tier 3 names; the raw ints are
// KbBikeDeltaMessage's motion/flag and pose bytes).
#pragma pack(push, 1)
struct KbNetDelta {
    char type;                   // +0x00 the message type byte (SendRaw 0x004ad280 writes it)
    signed char position[3];     // +0x01 x 15/64 -> KrustyBike+0x1364 deltaBasePosition
    unsigned char step;          // +0x04 bit 0: units of 8, bits 1-7: count
    signed char angles[3];       // +0x05 roll, pitch, yaw x pi/64 -> +0x137c..+0x1384
    signed char angularVelocity[3];  // +0x08 x pi/64 -> +0x1370 deltaBaseAngularVelocity
    signed char velocity[3];     // +0x0b x 15/64 -> +0x1358 deltaBaseVelocity
    int motionFlags;             // +0x0e motion, crash and flag bits (KbBikeDeltaMessage +0x0e..+0x11)
    int poseBytes;               // +0x12 pose index/state, param, blend, lean blend (+0x12..+0x15)
};
#pragma pack(pop)

// The state a message is decoded into (BikeRace.h's RacerStateMessage; the names
// follow that view and the KrustyBike delta bases 0x004933e0 copies in).
struct KbNetState {
    char type;                   // +0x00 KbBikeMessage type (the message's type byte)
    unsigned char finishPosition;  // +0x01 KbBikeMessage finishPosition
    unsigned short step;         // +0x02
    short lapCount;              // +0x04 KbBikeMessage lapCount
    short gatesPassed;           // +0x06 KbBikeMessage gatesPassed
    Vec3 position;               // +0x08 KrustyBike+0x1364 deltaBasePosition
    float roll;                  // +0x14 +0x137c deltaBaseRoll
    float pitch;                 // +0x18 +0x1380 deltaBasePitch
    float yaw;                   // +0x1c +0x1384 deltaBaseYaw
    char pad_0x20[0x10];
    Vec3 angularVelocity;        // +0x30 +0x1370 deltaBaseAngularVelocity
    Vec3 velocity;               // +0x3c +0x1358 deltaBaseVelocity
    int poseBytes;               // +0x48 KbBikeMessage +0x48 pose bytes (KbNetDelta::poseBytes)
    int time;                    // +0x4c running step total (+0x1388 deltaBaseTime)
    int motionFlags;             // +0x50 KbBikeMessage +0x50 motion/flag bits (KbNetDelta::motionFlags)
};

// Message type 1 (0x58 bytes) that 0x00492670 sends or records: this bike's motion state,
// pose and flags (tier 2 layout from the stores, tier 3 names).

// Received copy of message 1 (0x60 bytes, four at KrustyBike+0x11d8) that 0x00493660 reads.

// Message type 10 (8 bytes): the bike's +0x11c0 flag and +0x768.
struct KbBikePing {
    char type;                   // +0x00 SendRaw (0x004ad280) writes the type byte here
    unsigned char loaderKind;            // +0x01 KrustyBike::loaderKind (+0x11c0)
    char pad_0x02[2];
    float score;                 // +0x04 KrustyBike::score (+0x768)
};
// The state 0x00492670 keeps of the last message (caller-owned; tier 3).

// Message type 13 (0x17 bytes, packed) that 0x00492ad0 sends or records: byte deltas
// against the last state (the decoder 0x004933e0 reads them as KbNetDelta), the time step
// and message 1's pose and flag bytes.
#pragma pack(push, 1)
struct KbBikeDeltaMessage {
    char type;                           // +0x00 SendRaw (0x004ad280) writes the type byte here
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
    unsigned char spawnProtected : 1;
    unsigned char finished : 1;
    unsigned char onTrack : 1;
    unsigned char throttleOn : 1;
    unsigned char driveDisengaged : 1;   // +0x10
    unsigned char trickPoseHeld : 1;
    unsigned char lapGate;               // +0x11 lapGate (lapCount in event types 2 and 3)
    unsigned char poseIndex : 4;         // +0x12
    unsigned char poseState : 4;
    unsigned char poseParam;             // +0x13 x 100
    unsigned char poseBlend;             // +0x14 x 100
    unsigned char poseLeanBlend;         // +0x15 x 100
    unsigned char loaderKind;            // +0x16 +0x11c0
};
#pragma pack(pop)

// request: 1 for message 0x13 (this bike touched the followed racer), 0 for 0x87;
// followedRacerId: the racer's netId (BikeRace.h's view names +0x8 followedRacerId).
struct KbNetPacket { int header; char request; char pad_0x5[3]; int followedRacerId; };
// On-screen message object (0x8C bytes; ctor 0x0051B200 takes the text and a display time).
struct KbMessage { char data[0x8C]; KbMessage(const char* text, float seconds); };
// 0x004BFA80: millisecond clock (timeGetTime-based; tier 3).
unsigned int ReadClock();
// 0x004B5D00 (cdecl): forward/up axes from yaw, pitch and roll (tier 3 names; 0x00493660
// pushes the message +0x14 angle last).
int OrientationAxesFromAngles(Vector3* forward, Vector3* up, float yaw, float pitch, float roll);
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
extern float g_kbStallThreshold;     // 0x0056CB64  interval time at which slot 50 holds
extern float g_kbStallHoldSec;       // 0x0056CB68  length of that hold

// Object at TrackGame+0x3444 (proCircuit; bonus rules; packed: its ints sit at odd offsets).
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


struct KbSink {                    // view of the wheel Shock (vtable: 0 Reset, 1 ClearForces)
    virtual void Reset();          // 0x004f9ba0
    virtual void ClearForces();    // 0x004f9c70; slots 2-5 lie past the retail vtable
    virtual int UnknownVirtualSlot2(int a, int b);
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
};

// Matrix-like helper at +0x3bc (D3DIMSoultreeCharacter::d3d_field_0x1a0); 0x004FD710
// transforms a vector.
struct KbXform {
    Vec3 WorldToLocalDirection(const Vec3* v);
    // D3DIMSoultreeObject setters (names from src/reconstructed/D3DIMSoulTree.h): 0x0048fc80 calls
    // these with 0 or 1 on the bike and rider nodes.
};
// Element of the array at Bike+0x12c (count at +0x130).
struct KbChild {
    int pad_0[1];
    int ownerNode;   // +0x04 CollisionPoint ownerNode
    char pad_8[0x9c];
    int inContact;   // +0xa4 CollisionPoint inContact
};
// Opaque storage view of the ghost modifier; allocation/calls use GhostMod1.
struct KbObj128 {
};


// KrustyBike slots 93/94's view of the JoystickDevice at VehicleInputMap+0x0c (the
// ControlInterface's activeJoystick; see Vehicle.h).  The slots they call are its force
// feedback methods: 6 CreateConstantForce, 7 SetConstantForce, 11 StopEffect, 12 StartEffect;
// +0x0c is InputDevice's deviceKind.  KbEffectParams is SetConstantForce's direction array
// (0x4650 = 180 degrees in DirectInput hundredths; with DIEFF_POLAR the last element is reserved and must be 0).
struct KbEffectParams { int angle; int reserved; };
struct KbPad {
    virtual void RemoveBindings();
    virtual void UnknownVirtualSlot1();
    virtual void IsButtonDown();
    virtual void GetPovAngle();
    virtual void GetPovDirection();
    virtual void SetAutoCenter();
    virtual void CreateConstantForce(int effect, int magnitude, int duration);
    virtual void SetConstantForce(int effect, KbEffectParams* direction, int magnitude);
    virtual void SetDirection();
    virtual void CreateEnvelopedForce();
    virtual void CreateSquareWave();
    virtual void StopEffect(int effect);
    virtual void StartEffect(int effect, int iterations, int flags);
    char pad_0x04[0x8];
    int deviceKind;  // +0x0c; 3 when force feedback is driven
};
extern int g_kbPadActive;   // 0x0067C3AC

// Views of the rider-animation objects behind Bike+0x5c4 and Bike+0x604 (tier 3 names).
struct KbAnim;


// Session object at TrackGame+0x570 (eventManager); 0x0045D2B0 returns the local player record (tier 3 names).
struct KbMessage;
struct KbMsgSink { void ReplaceHeadMessage(KbMessage* m); };
struct KbSession { KbPlayer* FindRaceMode(); KbMsgSink* FindTextQueue(); };
class Vehicle;
// View of the race view owner (TrackGameViewOwner, src/reconstructed/TrackGame.h); the names
// follow that view.  In the tag event (type 4) followedRacer is the racer who is IT.
struct KbPlayer {
    char pad_0x0000[0xA8];
    Vehicle* followedRacer;  // +0xa8 TrackGameViewOwner::followedRacer
    char pad_0x00AC[0x30];
    Vehicle* ball;           // +0xdc TrackGameViewOwner::ball (the tag ball; "FREE BALL!" while set)
    // 0x004A9E80 (TrackGameViewOwner::SetFollowedRacer): follows racer after previous; previous
    // is a vehicle (tested and read at +0x75c); tell sends message 0x87 through SendFollowedRacer.
    void SetFollowedRacer(Vehicle* racer, Vehicle* previous, int tell);
};
// Bounds/collision body at Vehicle::collisionObject (canonical: CollisionObject::TestMeshBounds).
struct KbBody { int TestMeshBounds(KbBody* other); };

// ---- loader (0x0048fc80) support ----
// 0x5c-byte setup record the loader copies and forwards field by field: +0 -> KrustyBike+0x738,
// +4 -> +0x737, +8 is converted to float, +0x24 is passed by address, the rest by value.  The
// layout is the garage settings record (OptionProcs.h UnknownOptGarageSettings, 0x5c bytes,
// TrackGame+0x1550) and the names follow it (tier 2; the slider literals are tier 1 there).
struct KbBikeSetup {
    int engineSize;              // +0x00 -> KrustyBike::engineSize
    char fourStroke;             // +0x04 -> KrustyBike::fourStroke
    char pad_0x05[0x3];
    int weight;                  // +0x08 KrustyUI "Weight"; LoadBike's weight (as float)
    // Floats: the loader's `setup ? setup->f : 0` arguments zero their temporaries with
    // immediates (not the zero register) and VC6 threads their branches as one group.
    float frontComp;             // +0x0c SldFrontComp   -> LoadBike frontSpring
    float frontDecomp;           // +0x10 SldFrontDecomp -> frontDamper
    float rearComp;              // +0x14 SldRearComp    -> rearSpring
    float rearDecomp;            // +0x18 SldRearDecomp  -> rearDamper
    float frontDamp;             // +0x1c SldFrontDamp   -> frontRatio
    float rearDamp;              // +0x20 SldRearDamp    -> rearRatio
    int eqBands[11];             // +0x24 SldEQ1..11; passed by address as LoadBike's torqueTable
    int firstBand;               // +0x50 -> LoadBike rpmLow
    int lastBand;                // +0x54 KrustyUI "RPMUpperLimit" -> rpmHigh
    int bandStep;                // +0x58 -> rpmStep
};
// 0x004a2fc0 (cdecl): zeroed debug-heap allocation of count x size bytes tagged with the source line.
void* DebugCalloc(unsigned int count, unsigned int size, const char* file, int line);

// Non-virtual callees of slot 11: 0x004B0AC0 is PointInPlacementSquare (declared in
// KrustyBike.cpp; src/reconstructed/PlacementQueries.cpp has no header) and 0x004B0DF0 (the
// query the base slot 11 issues) is FindObjectPlacement, declared in ../contact/ObjectPlacement.h.

#endif
