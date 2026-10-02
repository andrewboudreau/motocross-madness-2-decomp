#pragma once

#include "ControlInterface.h"
#include "PCGame.h"

struct UnknownInputEntry;

// RTTI: TrackGame : PCGame (vtable 0x00558938; trkgame.cpp). The global
// pointer at 0x0056e26c is statically initialised to the object at
// 0x006851a0, which a dynamic initializer (0x004aa7c0) constructs with
// 0x00520870 (it writes TrackGame's vtable) and destroys at exit with
// 0x00521ae0. The camera and input code reads it throughout. Only members
// that reconstructed functions touch are declared; names are provisional.

// Object returned by (+0x570)->0x0045d340; KrustyBikeCamera slot 58 hands it
// a message.
class UnknownMessageTarget;

class TrackGameList {
public:
    UnknownMessageTarget* UnknownFunction45d340(); // 0x0045d340
};

struct UnknownKrustyBikeView;

struct TrackGameViewOwner {
    unsigned char field_0x00[0x34];
    UnknownKrustyBikeView* field_0x34;
};

// Object embedded at +0x578; KrustyBikeCamera slot 52 reads its mode.
class TrackGameMode {
public:
    TrackGameMode();             // 0x00522060
    ~TrackGameMode();            // 0x005225f0
    int UnknownFunction524100(); // 0x00524100
    void UnknownFunction523580();                           // 0x00523580 (TrackGame slot 15)
    int UnknownFunction5238f0(const char* name, char* path); // 0x005238f0 (TrackGame slot 18)
};

// Object at TrackGame+0x56c; 0x00521cd0 tests its +0x4a8.
struct UnknownTrackGameObject56c {
    unsigned char field_0x00[0x4a8];
    int field_0x4a8;
};

// Object at TrackGame+0x574, deleted by slot 15. Its destructor is
// out-of-line and empty (the shared body 0x00464e90).
class UnknownTrackGameObject574 {
public:
    ~UnknownTrackGameObject574();             // 0x00464e90
};

// Object at TrackGame+0x3338, deleted through its virtual destructor.
class UnknownTrackGameObject3338 {
public:
    virtual ~UnknownTrackGameObject3338();
};

// Object at TrackGame+0x3340 (0x00521a40 calls 0x004310e0 on it).
class UnknownTrackGameObject3340 {
public:
    ~UnknownTrackGameObject3340();            // 0x004310b0
    void UnknownFunction4310e0();             // 0x004310e0
};

// Object at TrackGame+0x3400.
class UnknownTrackGameObject3400 {
public:
    ~UnknownTrackGameObject3400();            // 0x0051efc0
};

// Object at TrackGame+0x3444 (0x00521cd0 tests that it exists).
class UnknownTrackGameObject3444 {
public:
    ~UnknownTrackGameObject3444();            // 0x004d39f0
};

// Global object at 0x0068a48c, deleted by TrackGame's destructor.
class UnknownTrackGameGlobal68a48c {
public:
    ~UnknownTrackGameGlobal68a48c();          // 0x004ad3d0
};
extern UnknownTrackGameGlobal68a48c* g_UnknownGlobal68a48c;

class TrackGame : public PCGame {
public:
    TrackGame();                              // 0x00520870
    virtual ~TrackGame();                     // 0x00521ae0 (deleting wrapper 0x00520a90)
    virtual int UnknownVirtualSlot2();        // 0x00520d00: PCGame's slot 2
    virtual int UnknownVirtualSlot5();        // 0x00521a50
    virtual int UnknownVirtualSlot10();       // 0x00521660: Game's slot 10 as a flag
    virtual int UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00521840
    virtual int UnknownVirtualSlot15();       // 0x00521a70: shutdown
    virtual int UnknownVirtualSlot18(const char* name, char* path); // 0x00521cb0

    void UnknownFunction521a30();             // 0x00521a30
    void UnknownFunction521a40();             // 0x00521a40
    int UnknownFunction521cd0();              // 0x00521cd0

    // 0x00521970: copies string `id` into buffer (size bytes).
    void UnknownFunction521970(int id, char* buffer, int size);

    // Objects whose +0x34 KrustyBikeCamera slot 10 takes as its view, by
    // field_0x2d74.
    TrackGameViewOwner* field_0x558;
    TrackGameViewOwner* field_0x55c;
    TrackGameViewOwner* field_0x560;
    TrackGameViewOwner* field_0x564;
    TrackGameViewOwner* field_0x568;
    UnknownTrackGameObject56c* field_0x56c;
    TrackGameList* field_0x570;
    UnknownTrackGameObject574* field_0x574;
    TrackGameMode field_0x578;
    unsigned char field_0x0579[0x2930 - 0x579];
    int field_0x2930;    // saved KrustyBikeCamera state (slots 61, 62)
    float field_0x2934;  // saved KrustyBikeCamera presets (slots 59, 60)
    float field_0x2938;
    float field_0x293c;
    float field_0x2940;
    unsigned char field_0x2944[0x2d74 - 0x2944];
    int field_0x2d74;    // selects KrustyBikeCamera's view (slot 10)
    unsigned char field_0x2d78[0x3338 - 0x2d78];
    UnknownTrackGameObject3338* field_0x3338;
    int field_0x333c;
    UnknownTrackGameObject3340* field_0x3340;
    UnknownControlBinding field_0x3344;
    UnknownControlBinding field_0x3380;
    UnknownControlBinding field_0x33bc;
    char* field_0x33f8;
    char* field_0x33fc;
    UnknownTrackGameObject3400* field_0x3400;
    char field_0x3404[8];      // saved decimal separator, restored on shutdown
    void* field_0x340c;        // "lang.dll", the string resource instance when present
    int field_0x3410;
    float field_0x3414;        // "IntervalBetweenFullNetPacketsMS", in seconds
    float field_0x3418;        // "IntervalBetweenShortNetPacketsMS"
    float field_0x341c;        // "IntervalBetweenFullRecordPacketsMS"
    float field_0x3420;        // "IntervalBetweenShortRecordPacketsMS"
    int field_0x3424;
    int field_0x3428;
    int field_0x342c;
    int field_0x3430;    // blocks KrustyBikeCamera slot 23
    int field_0x3434;
    int field_0x3438;
    int field_0x343c;          // opens a web page (string 0x14df) on destruction
    int field_0x3440;          // opens the store page on destruction
    UnknownTrackGameObject3444* field_0x3444;
    unsigned char field_0x3448[0x34c8 - 0x3448];
    int field_0x34c8;          // re-enables the screen saver on destruction (NT)
};

extern TrackGame* g_UnknownGlobal56e26c;
