#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"
#include "OverlayRect.h"
#include "Track.h"
#include "VCRfile.h"

// A queued text line (0x8c bytes, TrackOverlay.cpp). Callers build one on
// the stack (constructor 0x0051b200, no destructor) and hand it to
// TextQueueOverlay, which keeps heap copies (copy constructor 0x0051b250).
class TextQueueMessage {
public:
    TextQueueMessage(const char* text, float duration); // 0x0051b200
    TextQueueMessage(const TextQueueMessage& other);       // 0x0051b250

    char line[0x80];                          // +0x00 text
    float showSeconds;                        // +0x80 seconds to show it
    float shownSeconds;                       // +0x84 seconds shown so far
    TextQueueMessage* next;                     // +0x88 next queued line
};

// RTTI: TextQueueOverlay : GameObject (vtable 0x00558760), 0x58 bytes
// (TrackOverlay.cpp): draws the head of a queue of TextQueueMessage lines with
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
    TextQueueOverlay* Setup(void* target, OverlayRect rect);
    void ReplaceHeadMessage(TextQueueMessage* message); // 0x0051b540: replaces the head
    void AppendMessage(TextQueueMessage* message); // 0x0051b5e0: appends
    void DropHeadMessage();             // 0x0051b670: drops the head

    void* textFont;                           // +0x2c font (DeleteObject)
    OverlayRect textRect;              // +0x30 text rectangle (+0x40 less one)
    OverlayRect shadowRect;            // +0x40 shadow rectangle
    TextQueueMessage* queueHead;                // +0x50 queue head
    unsigned int textColor;                   // +0x54 text colour, 0xff00
};

class KrustyBike;
struct EventBikeRaceView;
struct RaceGate;
class ChatOverlay;

class D3DIMSoultreeCharacter;
class Wrecker;
class SoultreeObject;
// Keep the local race-view name while using the actual scene-node type.
typedef SoultreeObject UnknownVehiclePart;

// 0x54-byte race status node (RaceStatus.cpp, DebugCalloc'd): one per
// racer, chained through +0x50; the racer points back at it from +0x744.
// +0x1c/+0x28 are the racer's previous and current (leading) positions and
// +0x34 the next gate (a view's +0xc8 list, BikeRace.h) they are tested
// against; +0x38/+0x44 are track positions.
struct RaceStatusNode {
    int creationOrder;                        // +0x00 1-based order of creation
    KrustyBike* racer;                 // +0x04 racer
    float goalDistance;                       // +0x08 progressPos to the finish probe (track races) or to the next gate's centre (gate races), 0x004e6a50
    float gateDistance;                       // +0x0c distance to the next gate (sort key)
    float progressStep;                       // +0x10 progressPos..placedPos distance this frame (0x004e6a50)
    float progressTotal;                      // +0x14 sum of accepted progress steps (gate races: of gate approach)
    int lapOffset;                            // +0x18 added to the lap count when ordering (0x004e5f40): -1 when the placedPos..progressPos distance passes the probe gate
    Vector3 previousPosition;                 // +0x1c previous position
    Vector3 currentPosition;                  // +0x28 current position
    RaceGate* nextGate;                       // +0x34 next gate
    TrackPos placedPos;                       // +0x38 Track::PlaceNear's (or ClosestPointOnTrack's) placement of previousPosition
    TrackPos progressPos;                     // +0x44 PlaceNear's search start; advances only forward to placedPos (0x004e6a50)
    RaceStatusNode* next;              // +0x50 next
};

// The score board at EventBikeRaceView+0xbc (BonusObjectManager, the
// object SetupCounter returns); only 0x0048d1e0 is declared: shows `score` with `multiplier`
// (FollowRacer.cpp passes 2500 and 1 for taking over the followed racer).
struct EventKbScoreBoardView {
    void ShowValue(float score, float multiplier);  // 0x0048d1e0
};

// The replay object at EventBikeRaceView+0x1a0; +0x10c is its length.
struct EventKrustyVCRView {
    unsigned char field_0x000[0x10c];
    float replayLength;                       // +0x10c
};

// Object at KrustyBikeCamera+0x3b8 (chosen by slot 10 from the global's
// +0x558..+0x568 objects); slot 48 tests two flags. TrackGame and
// EventManager use it as a GameObject (slot 5, the +0x25 flag bits), which
// fits KrustyBike's primary base chain; its class is not established.
struct EventBikeRaceView : public GameObject {
    void ToggleReplayPause(int value);   // 0x0041f550 (InGameProcs.cpp)
    void RestartRace(int mode, int path, int description); // 0x00420650 (InGameProcs.cpp ExitDlg)
    void ResetStartCountdown();             // 0x00421050 (dlgprocs.cpp LoadingDlg)
    void HandlePlayerLeft(int player);  // 0x00420590 (EventManager slot 24)
    void SetDetailLevel(int level);   // 0x00423790: detail level (QuarryStuntEvent.cpp)
    // 0x00420b00 (near bikerace.cpp's literals): saves the replay to `path`
    // with `description`.
    void SaveReplay(char* path, char* description);
    // 0x004204e0: the next racer after `*iterator` (advancing it), or 0.
    KrustyBike* NextBike(int* iterator);

    unsigned char field_0x02c[0x38 - 0x2c];
    KrustyBike* localRacer;            // +0x38 its own racer
    KrustyBike** racerSlots;           // +0x3c all racers, by racer slot
    KrustyBike** aiRacers;             // +0x40 AI racers (TrackGame+0x2d94 of them)
    GameObject* raceSound;                    // +0x44 BikeRace: the RaceSound (FollowRacer plays the "it" sound through it)
    unsigned char field_0x048[0x50 - 0x48];
    GameObject* raceCamera;                   // +0x50 BikeRace: the race camera
    unsigned char field_0x054[0x60 - 0x54];
    GameObject* youIndicator;                 // +0x60 BikeRace: the "youind.slt" marker
    unsigned char field_0x064[0xbc - 0x64];
    EventKbScoreBoardView* scoreBoard;            // +0x0bc score board (FollowRacer.cpp 0x004a9e80)
    unsigned char field_0x0c0[0xc8 - 0xc0];
    RaceGate* gateList;                       // +0x0c8 gate list (BikeRace.h)
    unsigned char field_0x0cc[0x158 - 0xcc];
    int racerCount;                           // +0x158 entries in +0x3c
    unsigned char field_0x15c[0x18a - 0x15c];
    bool isRacing;                            // +0x18a racing (EventManager 0x0045eef0)
    unsigned char field_0x18b[0x18e - 0x18b];
    bool networkStartPending;                 // +0x18e BikeRace: the multiplayer start check is pending (the race clock waits, the camera shows the grid)
    unsigned char field_0x18f[0x19c - 0x18f];
    ChatOverlay* chatOverlay;                 // +0x19c the name tags (TrackOverlay 0x0051bed0 updates them)
    EventKrustyVCRView* replayVcr;   // +0x1a0 replay being played (InGameProcs.cpp VCRDlg)
    unsigned char ghostVcr[0x1a8 - 0x1a4];  // +0x1a4 BikeRace ghostVcr (bytes kept)
    VCRFile* vcrFile;             // +0x1a8 VCR file the recordings open; EventManager 0x0045cdc0 saves it as Record\*.gho
    unsigned char field_0x1ac[0x1b8 - 0x1ac];
    float replayTime;                         // +0x1b8 replay time (InGameProcs.cpp VCRDlg)
    unsigned char field_0x1bc[0x1c4 - 0x1bc];
    float replaySeekTarget;                   // +0x1c4 replay seek target, -1 when none
    unsigned char field_0x1c8[0x1dc - 0x1c8];
    int replayMode;                           // +0x1dc replay mode (4 play ... 14)
    unsigned char appliedReplayMode[0x1e4 - 0x1e0];  // +0x1e0 BikeRace appliedReplayMode (bytes kept)
    int lastRecordKind;  // +0x1E4  BikeRace +0x1e4 lastRecordKind: RecordFrame's last record kind (-2: no replay / tape end)
    int ghostLap;                             // +0x1e8 lap count when the ghost lap restarted
    unsigned char field_0x1ec[0x3f8 - 0x1ec];
    bool replayPaused;                       // +0x3f8
    bool replayStepFrame;                    // +0x3f9
};
