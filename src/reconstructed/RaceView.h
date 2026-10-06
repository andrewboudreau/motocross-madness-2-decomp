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

    char field_0x00[0x80];                    // text
    float field_0x80;                         // seconds to show it
    float field_0x84;                         // seconds shown so far
    UnknownMessage* field_0x88;               // next queued line
};

// RTTI: TextQueueOverlay : GameObject (vtable 0x00558760), 0x58 bytes
// (TrackOverlay.cpp): draws the head of a queue of UnknownMessage lines with
// GDI, then drops it after its duration. EventManager 0x0045d270 calls its
// slot 5. Member names are provisional.
class TextQueueOverlay : public GameObject {
public:
    explicit TextQueueOverlay(int flags);     // 0x0051b2a0
    virtual ~TextQueueOverlay();              // 0x0051b300 (deleting wrapper 0x0051b2e0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0051b3f0
    virtual int UnknownVirtualSlot15();       // 0x0051b450

    // 0x0051b320: binds the render target and the text rectangle and creates
    // the font from the GUI's face name (Arial when it has none).
    TextQueueOverlay* UnknownFunction51b320(void* target, UnknownOverlayRect rect);
    void UnknownFunction51b540(UnknownMessage* message); // 0x0051b540: replaces the head
    void UnknownFunction51b5e0(UnknownMessage* message); // 0x0051b5e0: appends
    void UnknownFunction51b670();             // 0x0051b670: drops the head

    void* field_0x2c;                         // font (DeleteObject)
    UnknownOverlayRect field_0x30;            // text rectangle (+0x40 less one)
    UnknownOverlayRect field_0x40;            // shadow rectangle
    UnknownMessage* field_0x50;               // queue head
    unsigned int field_0x54;                  // text colour, 0xff00
};

struct UnknownEventRacer;
struct UnknownKrustyBikeView;
struct UnknownBikeRaceNode;

// The object at a racer's +0x5f0; RaceStatus.cpp 0x004e6a50 reads the
// position at +0x200 for gate races.
struct UnknownEventRacerModel {
    unsigned char field_0x000[0x200];
    Vector3 field_0x200;
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
    int field_0x00;                         // 1-based order of creation
    UnknownEventRacer* field_0x04;          // racer
    float field_0x08;
    float field_0x0c;                       // distance to the next gate (sort key)
    float field_0x10;
    float field_0x14;
    int field_0x18;                         // -1 when behind the leader's lap (0x004e5f40)
    Vector3 field_0x1c;                     // previous position
    Vector3 field_0x28;                     // current position
    UnknownBikeRaceNode* field_0x34;        // next gate
    TrackPos field_0x38;
    TrackPos field_0x44;
    UnknownEventRacerPart* field_0x50;      // next
};

// A racer: a view's +0x38 (its own) and +0x3c (all, by racer slot), and the
// object EventManager ranks and updates from network messages. It reaches
// GameObject through a vbptr at +0x04 (EventManager 0x0045cdc0 calls slot 4
// through it), as KrustyBike's RTTI shows (virtual GameObject base, pdisp 4).
// The class is not established; only the vbase access path is modelled.
struct UnknownEventRacer : virtual public GameObject {
    virtual void UnknownVirtualSlot0();            // gives the racer its own vfptr at +0
    unsigned char field_0x008[0x0c - 0x08];
    Vector3 field_0x00c;                           // position (KrustyBikeCamera slot 10 adds 2 to y)
    unsigned char field_0x018[0x64 - 0x18];
    Vector3 field_0x064;                           // velocity (racesnd.cpp 0x004e3730)
    unsigned char field_0x070[0x88 - 0x70];
    Vector3 field_0x088;                           // RaceStatus.cpp 0x004e63e0 leads the position by 3.5 times it
    unsigned char field_0x094[0xb8 - 0x94];
    float field_0x0b8;                             // racesnd.cpp scales it to an engine speed
    unsigned char field_0x0bc[0x108 - 0xbc];
    bool field_0x108;                              // racesnd.cpp 0x004e39b0 (airborne sounds)
    unsigned char field_0x109[0x10c - 0x109];
    Vector3 field_0x10c;                           // track point (RaceStatus.cpp 0x004e63e0)
    Vector3 field_0x118;                           // track direction there
    unsigned char field_0x124[0x3bc - 0x124];
    UnknownVehiclePart* field_0x3bc;               // model; racesnd.cpp reads its position
    unsigned char field_0x3c0[0x444 - 0x3c0];
    int field_0x444;                               // racesnd.cpp 0x004e39b0
    unsigned char field_0x448[0x460 - 0x448];
    int field_0x460;                               // racesnd.cpp tests 12
    unsigned char field_0x464[0x478 - 0x464];
    bool field_0x478;                              // racesnd.cpp: throttle on
    bool field_0x479;
    unsigned char field_0x47a[0x484 - 0x47a];
    int field_0x484;                               // racesnd.cpp 0x004e39b0
    unsigned char field_0x488[0x4a0 - 0x488];
    int field_0x4a0;                               // EventManager 0x0045eef0: counts as done when set
    unsigned char field_0x4a4[0x5c4 - 0x4a4];
    UnknownCameraBikeRider* field_0x5c4;           // as BikeCamera's bike +0x5c4 (TrackOverlay 0x005190e0)
    unsigned char field_0x5c8[0x5e0 - 0x5c8];
    char field_0x5e0[0x10];                        // name
    UnknownEventRacerModel* field_0x5f0;           // RaceStatus.cpp 0x004e6a50 takes its +0x200 position
    unsigned char field_0x5f4[0x604 - 0x5f4];
    UnknownCameraBikeState* field_0x604;           // racesnd.cpp 0x004e39b0 reads its +0xb0
    unsigned char field_0x608[0x734 - 0x608];
    char field_0x734;                              // RaceStatus.cpp 0x004e6a50 finishes the race on its lap count
    unsigned char field_0x735;                     // RaceStatus.cpp: gates are passed without the crossing test
    unsigned char field_0x736;
    char field_0x737;                              // selects engine 2 over 1 (racesnd.cpp 0x004e3430)
    int field_0x738;                               // engine size; below 250 uses engine 0
    unsigned char field_0x73c[0x740 - 0x73c];
    UnknownKrustyBikeView* field_0x740;            // the view (RaceStatus.cpp compares its +0x38 and +0x50)
    UnknownEventRacerPart* field_0x744;
    int field_0x748;                               // time stamp; 0x7ffffffe until finished
    float field_0x74c;                             // last lap time (RaceStatus.cpp)
    float field_0x750;                             // EventManager 0x0045c8b0 copies it to a float TrackRecord reads
    float field_0x754;                             // FLT_MAX until finished
    int field_0x758;
    float field_0x75c;
    float field_0x760;                             // best of +0x75c
    float field_0x764;                             // sum of +0x75c
    float field_0x768;                             // a score (sorted with TrackOverlay's 0x005199f0)
    float field_0x76c;                             // time off the track (RaceStatus.cpp 0x004e63e0)
    float field_0x770;                             // time behind the leader (RaceStatus.cpp 0x004e6210)
    float field_0x774;                             // lap start time
    unsigned char field_0x778[0x77c - 0x778];
    float* field_0x77c;                            // lap start times, by lap % 100
    float* field_0x780;                            // gate times, by gate count % 600
    int field_0x784;                               // finishing position (1-based)
    int field_0x788;
    int field_0x78c;                               // RaceStatus.cpp 0x004e63e0: Track 0x00517340's result
    int field_0x790;                               // gates passed
    unsigned char field_0x794[0x7a0 - 0x794];
    unsigned short field_0x7a0;                    // laps
    unsigned char field_0x7a2[0x7a4 - 0x7a2];
    char field_0x7a4;                              // finished
    unsigned char field_0x7a5[0x7ac - 0x7a5];
    int field_0x7ac[3];                            // copied by EventManager 0x0045c8b0
    int field_0x7b8;                               // racesnd.cpp keeps a copy at +0x1294; gate of the lap
    int field_0x7bc;
    int field_0x7c0;                               // RaceStatus.cpp: passes the current gate without the test
    unsigned char field_0x7c4[0x11bc - 0x7c4];
    int field_0x11bc;                              // network player id
    char field_0x11c0;                             // AI racer's index in its messages
};

// The replay object at UnknownKrustyBikeView+0x1a0; +0x10c is its length.
struct UnknownKrustyBikeViewReplay {
    unsigned char field_0x000[0x10c];
    float field_0x10c;
};

// Object at KrustyBikeCamera+0x3b8 (chosen by slot 10 from the global's
// +0x558..+0x568 objects); slot 48 tests two flags. TrackGame and
// EventManager use it as a GameObject (slot 5, the +0x25 flag bits), which
// fits KrustyBike's primary base chain; its class is not established.
struct UnknownKrustyBikeView : public GameObject {
    void UnknownFunction41f1d0(int a, int b, int c); // 0x0041f1d0 (InGameProcs.cpp)
    void UnknownFunction41f550(int value);   // 0x0041f550 (InGameProcs.cpp)
    void UnknownFunction420650(int a, int b, int c); // 0x00420650 (InGameProcs.cpp ExitDlg)
    void UnknownFunction421050();             // 0x00421050 (dlgprocs.cpp LoadingDlg)
    void UnknownFunction420590(int player);  // 0x00420590 (EventManager slot 24)
    void UnknownFunction423790(int level);   // 0x00423790: detail level (QuarryStuntEvent.cpp)
    // 0x00420b00 (near bikerace.cpp's literals): saves the replay to `path`
    // with `description`.
    void UnknownFunction420b00(char* path, char* description);
    // 0x004204e0: the next racer after `*iterator` (advancing it), or 0.
    UnknownEventRacer* UnknownFunction4204e0(int* iterator);
    // 0x004210f0: writes two positions relative to `reference`.
    void UnknownFunction4210f0(Vector3* a, Vector3* b, void* reference, int flags);

    unsigned char field_0x02c[0x38 - 0x2c];
    UnknownEventRacer* field_0x38;                // its own racer
    UnknownEventRacer** field_0x3c;               // all racers, by racer slot
    UnknownEventRacer** field_0x40;               // AI racers (TrackGame+0x2d94 of them)
    GameObject* field_0x44;
    unsigned char field_0x048[0x50 - 0x48];
    GameObject* field_0x50;
    unsigned char field_0x054[0x60 - 0x54];
    GameObject* field_0x60;
    unsigned char field_0x064[0xc8 - 0x64];
    UnknownBikeRaceNode* field_0x0c8;      // gate list (BikeRace.h)
    unsigned char field_0x0cc[0x158 - 0xcc];
    int field_0x158;                     // entries in +0x3c
    unsigned char field_0x15c[0x18a - 0x15c];
    bool field_0x18a;                    // racing (EventManager 0x0045eef0)
    unsigned char field_0x18b[0x18e - 0x18b];
    bool field_0x18e;                    // slot 10: view available
    unsigned char field_0x18f[0x1a0 - 0x18f];
    UnknownKrustyBikeViewReplay* field_0x1a0; // replay being played (InGameProcs.cpp VCRDlg)
    unsigned char field_0x1a4[0x1a8 - 0x1a4];
    UnknownVcrFile* field_0x1a8;         // ghost recording (EventManager 0x0045cdc0)
    unsigned char field_0x1ac[0x1b8 - 0x1ac];
    float field_0x1b8;                   // replay time (InGameProcs.cpp VCRDlg)
    unsigned char field_0x1bc[0x1c4 - 0x1bc];
    float field_0x1c4;                   // replay seek target, -1 when none
    unsigned char field_0x1c8[0x1dc - 0x1c8];
    int field_0x1dc;                     // replay mode (4 play ... 14)
    unsigned char field_0x1e0[0x1e4 - 0x1e0];
    int field_0x1e4;                     // -2: no replay
    int field_0x1e8;
    unsigned char field_0x1ec[0x3f8 - 0x1ec];
    bool field_0x3f8;
    bool field_0x3f9;
};
