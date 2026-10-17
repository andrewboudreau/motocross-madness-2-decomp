#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"
#include "OverlayRect.h"
#include "Track.h"
#include "VCRfile.h"

// A queued text line (0x8c bytes, TrackOverlay.cpp). Callers build one on
// the stack (constructor 0x0051b200, no destructor) and hand it to
// TextQueueOverlay, which keeps heap copies (copy constructor 0x0051b250).
class UnknownMessage {
public:
    UnknownMessage(const char* text, float duration); // 0x0051b200
    UnknownMessage(const UnknownMessage& other);       // 0x0051b250

    char line[0x80];                          // +0x00 text
    float showSeconds;                        // +0x80 seconds to show it
    float shownSeconds;                       // +0x84 seconds shown so far
    UnknownMessage* next;                     // +0x88 next queued line
};

// RTTI: TextQueueOverlay : GameObject (vtable 0x00558760), 0x58 bytes
// (TrackOverlay.cpp): draws the head of a queue of UnknownMessage lines with
// GDI, then drops it after its duration. EventManager 0x0045d270 calls its
// slot 5. Member names are provisional.
class TextQueueOverlay : public GameObject {
public:
    explicit TextQueueOverlay(int flags);     // 0x0051b2a0
    virtual ~TextQueueOverlay();              // 0x0051b300 (deleting wrapper 0x0051b2e0)
    virtual int Tick(float frameTime);                 // 0x0051b3f0
    virtual int RenderPost3D();               // 0x0051b450

    // 0x0051b320: binds the render target and the text rectangle and creates
    // the font from the GUI's face name (Arial when it has none).
    TextQueueOverlay* Setup(void* target, UnknownOverlayRect rect);
    void ReplaceHeadMessage(UnknownMessage* message); // 0x0051b540: replaces the head
    void AppendMessage(UnknownMessage* message); // 0x0051b5e0: appends
    void DropHeadMessage();             // 0x0051b670: drops the head

    void* textFont;                           // +0x2c font (DeleteObject)
    UnknownOverlayRect textRect;              // +0x30 text rectangle (+0x40 less one)
    UnknownOverlayRect shadowRect;            // +0x40 shadow rectangle
    UnknownMessage* queueHead;                // +0x50 queue head
    unsigned int textColor;                   // +0x54 text colour, 0xff00
};

struct UnknownEventRacer;
struct UnknownKrustyBikeView;
struct UnknownBikeRaceNode;
class ChatOverlay;

// The object at a racer's +0x5f0; RaceStatus.cpp 0x004e6a50 reads the
// position at +0x200 for gate races.
struct UnknownEventRacerModel {
    unsigned char field_0x000[0x200];
    Vector3 nodePosition;  // +0x200  racer +0x5f0 is Bike::frontWheel (BikeWheel); VehicleWheel +0x200 nodePosition; RaceStatus 0x004e6a50 reads it
};
struct UnknownCameraBikeRider;
struct UnknownCameraBikeState;
class UnknownVehiclePart;

// 0x54-byte race status node (RaceStatus.cpp, DebugCalloc'd): one per
// racer, chained through +0x50; the racer points back at it from +0x744.
// +0x1c/+0x28 are the racer's previous and current (leading) positions and
// +0x34 the next gate (a view's +0xc8 list, BikeRace.h) they are tested
// against; +0x38/+0x44 are track positions.
struct UnknownEventRacerPart {
    int creationOrder;                        // +0x00 1-based order of creation
    UnknownEventRacer* racer;                 // +0x04 racer
    float goalDistance;                       // +0x08 progressPos to the finish probe (track races) or to the next gate's centre (gate races), 0x004e6a50
    float gateDistance;                       // +0x0c distance to the next gate (sort key)
    float progressStep;                       // +0x10 progressPos..placedPos distance this frame (0x004e6a50)
    float progressTotal;                      // +0x14 sum of accepted progress steps (gate races: of gate approach)
    int lapOffset;                            // +0x18 added to the lap count when ordering (0x004e5f40): -1 when the placedPos..progressPos distance passes the probe gate
    Vector3 previousPosition;                 // +0x1c previous position
    Vector3 currentPosition;                  // +0x28 current position
    UnknownBikeRaceNode* nextGate;            // +0x34 next gate
    TrackPos placedPos;                       // +0x38 Track::PlaceNear's (or ClosestPointOnTrack's) placement of previousPosition
    TrackPos progressPos;                     // +0x44 PlaceNear's search start; advances only forward to placedPos (0x004e6a50)
    UnknownEventRacerPart* next;              // +0x50 next
};

// A racer: a view's +0x38 (its own) and +0x3c (all, by racer slot), and the
// object EventManager ranks and updates from network messages. It reaches
// GameObject through a vbptr at +0x04 (EventManager 0x0045cdc0 calls slot 4
// through it), as KrustyBike's RTTI shows (virtual GameObject base, pdisp 4).
// The class is not established; only the vbase access path is modelled.
struct UnknownEventRacer : virtual public GameObject {
    virtual void UnknownVirtualSlot0();            // gives the racer its own vfptr at +0
    // Own vtable +0xc8 (slot 50) is called by FollowRacer.cpp 0x004a9e80 with
    // (1, 5.0f, 0); samples/race/FollowRacerNearMisses.cpp views the slot.

    // 0x004925a0 (KrustyBike.cpp's code): the view's own racer hears that
    // `racer` is followed (FollowRacer.cpp passes 0 as the flag).
    void UnknownFunction4925a0(UnknownEventRacer* racer, bool flag);
    // 0x00496f90 (KrustyBike.cpp's code): this racer hears about `other`.
    void UnknownFunction496f90(UnknownEventRacer* other);

    unsigned char field_0x008[0x0c - 0x08];
    Vector3 position;                         // +0x00c position (KrustyBikeCamera slot 10 adds 2 to y)
    unsigned char field_0x018[0x50 - 0x18];
    float heading;                            // +0x050 heading; RadarOverlay 0x0051bed0 negates it for the map
    unsigned char field_0x054[0x64 - 0x54];
    Vector3 velocity;                         // +0x064 velocity (racesnd.cpp 0x004e3730)
    unsigned char field_0x070[0x88 - 0x70];
    Vector3 bodyForward;  // +0x88  SoultreePhysicsBaseObject +0x88 bodyForward; RaceStatus leads position by 3.5x it, falls back to it for trackDirection
    unsigned char field_0x094[0xb8 - 0x94];
    float previousSpeed;                      // +0x0b8 racesnd.cpp scales it to an engine speed
    unsigned char field_0x0bc[0x108 - 0xbc];
    bool airborne;                            // +0x108 racesnd.cpp 0x004e39b0 (airborne sounds)
    unsigned char field_0x109[0x10c - 0x109];
    Vector3 trackPoint;                       // +0x10c track point (RaceStatus.cpp 0x004e63e0)
    Vector3 trackDirection;                   // +0x118 track direction there
    unsigned char field_0x124[0x3bc - 0x124];
    UnknownVehiclePart* vehicleModel;         // +0x3bc model; racesnd.cpp reads its position
    unsigned char field_0x3c0[0x444 - 0x3c0];
    int crashState;                           // +0x444 racesnd.cpp 0x004e39b0
    unsigned char field_0x448[0x460 - 0x448];
    int crashReason;                          // +0x460 racesnd.cpp tests 12
    unsigned char field_0x464[0x478 - 0x464];
    bool throttleOn;                          // +0x478 racesnd.cpp: throttle on
    bool driveDisengaged;                     // +0x479  Vehicle +0x479 driveDisengaged; racesnd.cpp 0x004e39b0 picks the engine sound off it
    unsigned char field_0x47a[0x484 - 0x47a];
    int justLanded;                           // +0x484 racesnd.cpp 0x004e39b0
    unsigned char field_0x488[0x4a0 - 0x488];
    int outOfRace;                            // +0x4a0 EventManager 0x0045eef0: counts as done when set
    unsigned char field_0x4a4[0x5c4 - 0x4a4];
    UnknownCameraBikeRider* riderCharacter;   // +0x5c4 as BikeCamera's bike +0x5c4 (TrackOverlay 0x005190e0)
    unsigned char field_0x5c8[0x5e0 - 0x5c8];
    char racerName[0x10];                     // +0x5e0 name
    UnknownEventRacerModel* frontWheel;       // +0x5f0 RaceStatus.cpp 0x004e6a50 takes its +0x200 position
    unsigned char field_0x5f4[0x604 - 0x5f4];
    UnknownCameraBikeState* wrecker;          // +0x604 the racer's Wrecker; racesnd.cpp 0x004e39b0 reads its contactThisFrame
    unsigned char field_0x608[0x734 - 0x608];
    char aiKind;  // +0x734  KrustyBike +0x734 aiKind; RaceStatus estimates finish times only for racers with it set
    unsigned char isRemote;  // +0x735  KrustyBike isRemote; passed as CountLap/AdvanceGate skipTest; selects ClosestPointOnTrack over PlaceNear
    unsigned char isGhost;  // +0x736  KrustyBike +0x736 isGhost (loader kind 0xff)
    char engineKind;                          // +0x737 selects engine 2 over 1 (racesnd.cpp 0x004e3430)
    int engineSize;                           // +0x738 engine size; below 250 uses engine 0
    unsigned char field_0x73c[0x740 - 0x73c];
    UnknownKrustyBikeView* bikeView;          // +0x740 the view (RaceStatus.cpp compares its +0x38 and +0x50)
    UnknownEventRacerPart* statusNode;        // +0x744 race status node (UnknownBikeRaceRacer)
    int finishTimeStamp;                      // +0x748 time stamp; 0x7ffffffe until finished
    float lastLapTime;                        // +0x74c last lap time (RaceStatus.cpp)
    float fastestLap;                         // +0x750 EventManager 0x0045c8b0 copies it to ZoneReport's "FastestLap" (TrackRecord reads it)
    float totalRunningTime;                   // +0x754 FLT_MAX until finished; ZoneReport "TotalrunningTime" (0x0045c8b0)
    int totalTrickTime;                       // +0x758  KrustyBike::totalTrickTime (an int view of the float)
    float tagRun;                             // +0x75c the current run as the followed racer in tag (mode 4); banked by
                                              // TrackGameViewOwner 0x004a9e80 and EventManager 0x0045eef0
    float bestTagRun;                         // +0x760 best banked run
    float totalTagRun;                        // +0x764 sum of the banked runs; EventManager 0x0045e9d0 ranks by it
    float score;                              // +0x768 a score (sorted with TrackOverlay's 0x005199f0)
    float offTrackTime;                       // +0x76c time off the track (RaceStatus.cpp 0x004e63e0)
    float timeBehindLeader;                   // +0x770 time behind the leader (RaceStatus.cpp 0x004e6210)
    float lapStartTime;                       // +0x774 lap start time
    unsigned char field_0x778[0x77c - 0x778];
    float* lapStartTimes;                     // +0x77c lap start times, by lap % 100
    float* gateTimes;                         // +0x780 gate times, by gate count % 600
    int finishPosition;                       // +0x784 finishing position (1-based)
    int largestStuntPoints;                   // +0x788 ZoneReport "LargestSingleStuntPoints" (0x0045c8b0)
    int onTrack;                              // +0x78c Track::PlaceNear placed the racer (RaceStatus.cpp 0x004e63e0); clears offTrackTime
    int gatesPassed;                          // +0x790 gates passed
    unsigned char field_0x794[0x7a0 - 0x794];
    unsigned short lapCount;                  // +0x7a0 laps
    unsigned char field_0x7a2[0x7a4 - 0x7a2];
    char hasFinished;                         // +0x7a4 finished
    unsigned char field_0x7a5[0x7ac - 0x7a5];
    int field_0x7ac[3];                            // copied by EventManager 0x0045c8b0
    int lapGate;                              // +0x7b8 gate of the lap (AdvanceGate's gateIndex); racesnd.cpp keeps a copy at +0x1294
    int nextLapGate;                          // +0x7bc lapGate + 1 wrapped by the owner's gate count (RaceStatus.cpp 0x004e6a50)
    int forceGatePass;  // +0x7C0  passed as CountLap/AdvanceGate `forced`: with skipTest the gate/lap counts only when set
    unsigned char field_0x7c4[0x11bc - 0x7c4];
    int networkPlayerId;                      // +0x11bc network player id
    char aiIndex;                             // +0x11c0 AI racer's index in its messages
};

// The score board at UnknownKrustyBikeView+0xbc (the krustybike sample's
// KbScoreBoard); only 0x0048d1e0 is declared: shows `score` with `multiplier`
// (FollowRacer.cpp passes 2500 and 1 for taking over the followed racer).
struct UnknownScoreBoard {
    void ShowValue(float score, float multiplier);  // 0x0048d1e0
};

// The replay object at UnknownKrustyBikeView+0x1a0; +0x10c is its length.
struct UnknownKrustyBikeViewReplay {
    unsigned char field_0x000[0x10c];
    float replayLength;                       // +0x10c
};

// Object at KrustyBikeCamera+0x3b8 (chosen by slot 10 from the global's
// +0x558..+0x568 objects); slot 48 tests two flags. TrackGame and
// EventManager use it as a GameObject (slot 5, the +0x25 flag bits), which
// fits KrustyBike's primary base chain; its class is not established.
struct UnknownKrustyBikeView : public GameObject {
    void NextCameraTarget(int a, int b, int c); // 0x0041f1d0 (InGameProcs.cpp)
    void ToggleReplayPause(int value);   // 0x0041f550 (InGameProcs.cpp)
    void RestartRace(int a, int b, int c); // 0x00420650 (InGameProcs.cpp ExitDlg)
    void ResetStartCountdown();             // 0x00421050 (dlgprocs.cpp LoadingDlg)
    void HandlePlayerLeft(int player);  // 0x00420590 (EventManager slot 24)
    void SetDetailLevel(int level);   // 0x00423790: detail level (QuarryStuntEvent.cpp)
    // 0x00420b00 (near bikerace.cpp's literals): saves the replay to `path`
    // with `description`.
    void SaveReplay(char* path, char* description);
    // 0x004204e0: the next racer after `*iterator` (advancing it), or 0.
    UnknownEventRacer* NextBike(int* iterator);
    // 0x004210f0: writes two positions relative to `reference`.
    void WriteRelativePositions(Vector3* a, Vector3* b, void* reference, int flags);

    unsigned char field_0x02c[0x38 - 0x2c];
    UnknownEventRacer* localRacer;            // +0x38 its own racer
    UnknownEventRacer** racerSlots;           // +0x3c all racers, by racer slot
    UnknownEventRacer** aiRacers;             // +0x40 AI racers (TrackGame+0x2d94 of them)
    GameObject* raceSound;                    // +0x44 BikeRace: the RaceSound (FollowRacer plays the "it" sound through it)
    unsigned char field_0x048[0x50 - 0x48];
    GameObject* raceCamera;                   // +0x50 BikeRace: the race camera
    unsigned char field_0x054[0x60 - 0x54];
    GameObject* youIndicator;                 // +0x60 BikeRace: the "youind.slt" marker
    unsigned char field_0x064[0xbc - 0x64];
    UnknownScoreBoard* scoreBoard;            // +0x0bc score board (FollowRacer.cpp 0x004a9e80)
    unsigned char field_0x0c0[0xc8 - 0xc0];
    UnknownBikeRaceNode* gateList;            // +0x0c8 gate list (BikeRace.h)
    unsigned char field_0x0cc[0x158 - 0xcc];
    int racerCount;                           // +0x158 entries in +0x3c
    unsigned char field_0x15c[0x18a - 0x15c];
    bool isRacing;                            // +0x18a racing (EventManager 0x0045eef0)
    unsigned char field_0x18b[0x18e - 0x18b];
    bool networkStartPending;                 // +0x18e BikeRace: the multiplayer start check is pending (the race clock waits, the camera shows the grid)
    unsigned char field_0x18f[0x19c - 0x18f];
    ChatOverlay* chatOverlay;                 // +0x19c the name tags (TrackOverlay 0x0051bed0 updates them)
    UnknownKrustyBikeViewReplay* replayVcr;   // +0x1a0 replay being played (InGameProcs.cpp VCRDlg)
    unsigned char field_0x1a4[0x1a8 - 0x1a4];
    UnknownVcrFile* vcrFile;             // +0x1a8 VCR file the recordings open; EventManager 0x0045cdc0 saves it as Record\*.gho
    unsigned char field_0x1ac[0x1b8 - 0x1ac];
    float replayTime;                         // +0x1b8 replay time (InGameProcs.cpp VCRDlg)
    unsigned char field_0x1bc[0x1c4 - 0x1bc];
    float replaySeekTarget;                   // +0x1c4 replay seek target, -1 when none
    unsigned char field_0x1c8[0x1dc - 0x1c8];
    int replayMode;                           // +0x1dc replay mode (4 play ... 14)
    unsigned char field_0x1e0[0x1e4 - 0x1e0];
    int lastRecordKind;  // +0x1E4  BikeRace +0x1e4 lastRecordKind: RecordFrame's last record kind (-2: no replay / tape end)
    int ghostLap;                             // +0x1e8 lap count when the ghost lap restarted
    unsigned char field_0x1ec[0x3f8 - 0x1ec];
    bool replayPaused;                       // +0x3f8
    bool replayStepFrame;                    // +0x3f9
};
