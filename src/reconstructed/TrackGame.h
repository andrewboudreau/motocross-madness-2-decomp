#pragma once

#include "ControlInterface.h"
#include "MemTag.h"
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
    GameObject* UnknownFunction45d2b0();           // 0x0045d2b0
    GameObject* UnknownFunction45d2f0();           // 0x0045d2f0
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
    int UnknownFunction523bf0(); // 0x00523bf0 (TrackGame slot 1 retries until it passes)
    int UnknownFunction523c90(); // 0x00523c90 (TrackGame slot 1)
    ~TrackGameMode();            // 0x005225f0
    int UnknownFunction524100(); // 0x00524100
    void UnknownFunction523580();                           // 0x00523580 (TrackGame slot 15)
    int UnknownFunction5238f0(const char* name, char* path); // 0x005238f0 (TrackGame slot 18)
};

// Object returned by (TrackGame+0x56c)->+0x2c->0x00485df0.
class UnknownTrackGameObject56cItem {
public:
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
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual void UnknownVirtualSlot22();
    virtual void UnknownVirtualSlot23();
    virtual void UnknownVirtualSlot24();
    virtual void UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
};

class UnknownTrackGameObject56cOwner {
public:
    UnknownTrackGameObject56cItem* UnknownFunction485df0();  // 0x00485df0
};

// Object at TrackGame+0x56c; 0x00521cd0 tests its +0x4a8.
class UnknownTrackGameObject56c {
public:
    void UnknownFunction499b20(int value);    // 0x00499b20

    unsigned char field_0x00[0x2c];
    UnknownTrackGameObject56cOwner* field_0x2c;
    unsigned char field_0x30[0x3c - 0x30];
    int field_0x3c;
    unsigned char field_0x40[0x4a8 - 0x40];
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

// Global at 0x00572b44; its code is among ResourceManager.cpp's literals.
// No RTTI names it.
class UnknownResourceManager {
public:
    void UnknownFunction4e9030(const char* path, int flags);  // 0x004e9030: adds an archive
};
extern UnknownResourceManager* g_UnknownResourceManager572b44;

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
    virtual int UnknownVirtualSlot1();        // 0x00520ab0: start-up checks and the EULA
    virtual int UnknownVirtualSlot2();        // 0x00520d00: PCGame's slot 2
    virtual int UnknownVirtualSlot3();        // 0x00520d10: opens the resource archives
    virtual int UnknownVirtualSlot5();        // 0x00521a50
    virtual int UnknownVirtualSlot10();       // 0x00521660: Game's slot 10 as a flag
    virtual int UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00521840
    virtual int UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00521670
    virtual int UnknownVirtualSlot15();       // 0x00521a70: shutdown
    virtual int UnknownVirtualSlot18(const char* name, char* path); // 0x00521cb0

    void UnknownFunction521a30();             // 0x00521a30
    void UnknownFunction521a40();             // 0x00521a40
    int UnknownFunction521cd0();              // 0x00521cd0

    // 0x00521970: loads string resource `id` into buffer (size bytes), or
    // "Resource String Unavailable"; 1 when loaded.
    int UnknownFunction521970(int id, char* buffer, int size);
    // 0x00521860: opens (`open`) or closes the in-game menu `id`.
    void UnknownFunction521860(int open, int id, int sound);

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
