#pragma once

#include "ControlInterface.h"
#include "DirectoryList.h"
#include "EventManager.h"
#include "KrustyUI.h"
#include "MemTag.h"
#include "PCGame.h"
#include "UnknownResourceManager.h"
#include "RaceView.h"
#include "TrackRecord.h"

struct UnknownInputEntry;

// RTTI: TrackGame : PCGame (vtable 0x00558938; trkgame.cpp). The global
// pointer at 0x0056e26c is statically initialised to the object at
// 0x006851a0, which a dynamic initializer (0x004aa7c0) constructs with
// 0x00520870 (it writes TrackGame's vtable) and destroys at exit with
// 0x00521ae0. The camera and input code reads it throughout. Only members
// that reconstructed functions touch are declared; names are provisional.

// The race-mode objects at TrackGame+0x558..+0x568 (EventManager uses the
// first one present). They are GameObjects (slots 4 and 5, the +0x25 flag
// bits); their classes are not established.
struct TrackGameViewOwner : public GameObject {
    unsigned char field_0x2c[0x34 - 0x2c];
    UnknownKrustyBikeView* field_0x34;
    unsigned char field_0x38[0x5c - 0x38];
    GameObject* field_0x5c;                   // EventManager 0x0045cdc0 calls slot 4 on 0x5c..0x68
    GameObject* field_0x60;
    GameObject* field_0x64;
    GameObject* field_0x68;
    TextQueueOverlay* field_0x6c;
    float field_0x70;                         // compared with TrackGame+0x2eb0 (EventManager 0x0045eef0)
    unsigned char field_0x74[0xa8 - 0x74];
    UnknownEventRacer* field_0xa8;
};

// Object embedded at +0x578; KrustyBikeCamera slot 52 reads its mode.
class TrackGameMode {
public:
    TrackGameMode();             // 0x00522060
    int UnknownFunction523bf0(); // 0x00523bf0 (TrackGame slot 1 retries until it passes)
    int UnknownFunction523c90(); // 0x00523c90 (TrackGame slot 1)
    ~TrackGameMode();            // 0x005225f0
    int UnknownFunction524100(); // 0x00524100
    void UnknownFunction5240e0(int series); // 0x005240e0 (TrackRecord.cpp 0x00520390)
    void UnknownFunction523580();                           // 0x00523580 (TrackGame slot 15)
    int UnknownFunction5238f0(const char* name, char* path); // 0x005238f0 (TrackGame slot 18)
    void UnknownFunction522680();             // 0x00522680 (TrackGame slot 4)
    void UnknownFunction522d00();             // 0x00522d00 (TrackGame slot 4)
    int UnknownFunction5231f0();              // 0x005231f0 (TrackGame slot 4)
    void UnknownFunction523e50();             // 0x00523e50 (KrustyUI 0x004988a0)
    // 0x00523a60 (near uiinfo.cpp's literals): builds the path of `name`'s
    // `kind` file (EventManager 0x0045cb70 asks for "env").
    void UnknownFunction523a60(int value, char* name, const char* kind, char* path);

    char field_0x00[16];                      // name; slot 4 sets it from the network object
    unsigned char field_0x10[0x90 - 0x10];
    int field_0x90;                           // cycles 0..5 (TrackOverlay 0x0051e7c0)
    unsigned char field_0x94[0xa0 - 0x94];
    char field_0xa0[6][0x100];                // directories (TrackRecord.cpp 0x0051f0b0)
    int field_0x6a0;                          // a directory name pointer for TrackRecord.cpp 0x0051f2c0                          // EventManager 0x0045cb70 passes it to 0x00523a60
    unsigned char field_0x6a4[0x6b0 - 0x6a4];
    int field_0x6b0;                          // shows the race statistics (TrackOverlay 0x00519980)
    unsigned char field_0x6b4[0x6b8 - 0x6b4];
    int field_0x6b8;                          // shows the racers' name tags (TrackOverlay 0x005190e0)
    int field_0x6bc;                          // TrackOverlay 0x0051e390
    int field_0x6c0;                          // TrackOverlay 0x0051e390
    unsigned char field_0x6c4[0x6c8 - 0x6c4];
    int field_0x6c8;                          // copied to the GUI's +0xec (KrustyUI 0x004988a0)
    unsigned char field_0x6cc[0x6d4 - 0x6cc];
    int field_0x6d4;                          // copied into GUI layer 0 (EventManager 0x0045e710)
    unsigned char field_0x6d8[0xa20 - 0x6d8];
    int field_0xa20;                          // EventManager 0x0045e710 calls TrackGame 0x00521a40 when clear
    int field_0xa24;                          // selects +0xa3c for the GUI's +0x34c (KrustyUI 0x004988a0)
    unsigned char field_0xa28[0xa34 - 0xa28];
    int field_0xa34;                          // TrackGame slot 4 audio argument
    unsigned char field_0xa38[0xa3c - 0xa38];
    int field_0xa3c;
    unsigned char field_0xa40[0xa48 - 0xa40];
    int field_0xa48;                          // selects 16 (else 8) in TrackGame slot 4
};

// Object returned by (TrackGame+0x56c)->+0x2c->0x00485df0.
class UnknownTrackGameObject56cItem {
public:
    void UnknownFunction46ff30(int value);    // 0x0046ff30 (EventManager 0x0045e710)
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

// Object at TrackGame+0x574, deleted by slot 15. Its destructor is
// out-of-line and empty (the shared body 0x00464e90).
class UnknownTrackGameObject574 {
public:
    UnknownTrackGameObject574();              // 0x004e99d0 (defined in SceneManager.cpp)
    ~UnknownTrackGameObject574();             // 0x00464e90
    // 0x004e9a10 / 0x004e9ac0 (SceneManager.cpp): run a temporary Scene.
    int UnknownFunction4e9a10(int* flag);
    int UnknownFunction4e9ac0(unsigned long* info, char* a, char* b, int c);
    // 0x004e9ba0: opens the scene archive `name` (default field_0x24c).
    UnknownTextureStream* UnknownFunction4e9ba0(const char* name);
    void UnknownFunction4e9f70(const char* name); // 0x004e9f70: derived file names
    void UnknownFunction4e9b80(char* path);   // 0x004e9b80 (EventManager 0x0045cb70)
    void UnknownFunction4e9e30(char* name, const char* kind, int value); // 0x004e9e30
    // 0x004ea390: writes the scene name for `field_0x24c` into `name`.
    void UnknownFunction4ea390(char* name, char* scene, int value);
    // 0x004ea010 (TrackRecord.cpp 0x0051ffe0): a track's display name.
    // Clears *a and *b when given; the archive's 0x00461cb0 fills them.
    void UnknownFunction4ea010(char* out, char* name, int index, const char* kind, int* a, int* b);

    // Layout from the constructor's stores; array lengths are inferred
    // from the gaps between them (and the 0x103-byte copy into +0x44).
    UnknownTextureStream* field_0x00;         // open scene archive
    char field_0x04[0x40];                    // its name
    char field_0x44[0x104];                   // path
    char field_0x148[0x104];
    char field_0x24c[0x40];                   // scene file name
    char field_0x28c[0x40];                   // "%s.est"
    char field_0x2cc[0x40];                   // "%s.trn"
    char field_0x30c[0x40];
    char field_0x34c[0x40];                   // "%s01.wpt"
    int field_0x38c;
    int field_0x390;
};

// Object at TrackGame+0x3338, deleted through its virtual destructor.
// Object at TrackGame+0x3340 (0x00521a40 calls 0x004310e0 on it).
class UnknownTrackGameObject3340 {
public:
    UnknownTrackGameObject3340();             // 0x00431050
    ~UnknownTrackGameObject3340();            // 0x004310b0
    void UnknownFunction4310c0();             // 0x004310c0
    void UnknownFunction4310e0();             // 0x004310e0

    unsigned char field_0x00[0x10];
};

// UnknownTrackGameObject3400, the object at TrackGame+0x3400, is in TrackRecord.h.

// Object at TrackGame+0x33fc (constructor near DebugOverlay.cpp's literals;
// no destructor).
class UnknownTrackGameObject33fc {
public:
    UnknownTrackGameObject33fc();             // 0x00448960

    unsigned char field_0x00[0x384];
};

// Per-racer record in TrackGame (0xf8 bytes from +0x215c, after the count at
// +0x2158; EventManager 0x0045fbd0 copies whole records). The array's
// length is not established.
struct UnknownTrackGameRacerSlot {
    unsigned char field_0x00[0xc8];
    int field_0xc8;                           // set when the player leaves (EventManager slot 24)
    int field_0xcc;                           // ready
    int field_0xd0;
    int field_0xd4;                           // network player id
    int field_0xd8;                           // player's racer index (EventManager slot 24)
    unsigned char field_0xdc[0xf8 - 0xdc];
};

// Base of the +0x3410 object; its constructor sits among
// FontTextureManager.cpp's literals.
class UnknownTrackGameObject3410Base {
public:
    UnknownTrackGameObject3410Base();         // 0x004676a0

    unsigned char field_0x00[0x32c];
};

// Object at TrackGame+0x3410, created for network games. It has no
// constructor of its own: `new` calls the base constructor directly and uses
// the allocation (not the constructor's result), and its method sits among
// MSZoneInterface.cpp's literals.
class UnknownTrackGameObject3410 : public UnknownTrackGameObject3410Base {
public:
    void UnknownFunction4aa350(UnknownDirectPlay4A* a, UnknownDirectPlayLobby3A* b); // 0x004aa350
    void UnknownFunction49c770();             // 0x0049c770 (EventManager 0x0045e550)
};

// Object at TrackGame+0x3444 (0x00521cd0 tests that it exists).
class UnknownTrackGameObject3444 {
public:
    ~UnknownTrackGameObject3444();            // 0x004d39f0
};

// cdecl 0x00520820 (near TrackRecord.cpp's literals): logs a message.
void UnknownFunction520820(const char* message);

// Global object at 0x0068a48c, deleted by TrackGame's destructor.
class UnknownTrackGameGlobal68a48c {
public:
    UnknownTrackGameGlobal68a48c();           // 0x004ad3b0 (defined in Net.cpp)
    ~UnknownTrackGameGlobal68a48c();          // 0x004ad3d0
    int UnknownFunction4ad3e0(const char* address, int port); // 0x004ad3e0: 1 on failure
    int UnknownFunction4ad530(const char* data, int length);  // 0x004ad530
    int UnknownFunction4ad570(const char* text);              // 0x004ad570

    unsigned int field_0x00;                  // WinSock socket
    int field_0x04;                           // last WinSock result
    int field_0x08;
    int field_0x0c;                           // connected
    int field_0x10;                           // port
};
extern UnknownTrackGameGlobal68a48c* g_UnknownGlobal68a48c;

class TrackGame : public PCGame {
public:
    TrackGame();                              // 0x00520870
    virtual ~TrackGame();                     // 0x00521ae0 (deleting wrapper 0x00520a90)
    virtual int UnknownVirtualSlot1();        // 0x00520ab0: start-up checks and the EULA
    virtual int UnknownVirtualSlot2();        // 0x00520d00: PCGame's slot 2
    virtual int UnknownVirtualSlot3();        // 0x00520d10: opens the resource archives
    virtual int UnknownVirtualSlot4();        // 0x00521050: creates the game objects
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
    KrustyUI* ui;
    EventManager* eventManager;
    UnknownTrackGameObject574* sceneObject;
    TrackGameMode mode;
    int field_0xfc4;                          // display mode index (EventManager 0x0045e710)
    unsigned char field_0xfc8[0x2158 - 0xfc8];
    int field_0x2158;                         // racer count (EventManager 0x0045e550)
    UnknownTrackGameRacerSlot field_0x215c[8];
    unsigned char field_0x291c[0x2930 - 0x291c];
    int field_0x2930;    // saved KrustyBikeCamera state (slots 61, 62)
    float field_0x2934;  // saved KrustyBikeCamera presets (slots 59, 60)
    float field_0x2938;
    float field_0x293c;
    float field_0x2940;
    unsigned char field_0x2944[0x2b58 - 0x2944];
    DirectoryList* field_0x2b58;               // scanned by TrackRecord.cpp 0x00520390
    unsigned char field_0x2b5c[0x2c68 - 0x2b5c];
    int field_0x2c68;                          // mode 4 saves a ghost (EventManager 0x0045cdc0)
    unsigned char field_0x2c6c[0x2d70 - 0x2c6c];
    int field_0x2d70;    // EventManager 0x0045e600 compares it with 2
    int field_0x2d74;    // selects KrustyBikeCamera's view (slot 10)
    unsigned char field_0x2d78[0x2d7c - 0x2d78];
    int field_0x2d7c;                         // cleared by EventManager 0x0045e520
    unsigned char field_0x2d80[0x2d88 - 0x2d80];
    int field_0x2d88;                          // selects a 30 (else 120) second limit (EventManager 0x0045eef0)
    unsigned char field_0x2d8c[0x2d94 - 0x2d8c];
    int field_0x2d94;                          // AI racer count (EventManager 0x0045f9a0)
    unsigned char field_0x2d98[0x2da5 - 0x2d98];
    unsigned char field_0x2da5;                // decremented when a player leaves (EventManager 0x0045fbd0)
    char field_0x2da6[0x20];                   // track name (length not established)
    unsigned char field_0x2dc6[0x2eb0 - 0x2dc6];
    float field_0x2eb0;
    unsigned char field_0x2eb4[0x2eb8 - 0x2eb4];
    int field_0x2eb8;                          // mode 4 ranks by +0x768 (else +0x764) (EventManager 0x0045e9d0)
    unsigned char field_0x2ebc[0x3334 - 0x2ebc];
    int field_0x3334;                          // saves a replay (EventManager 0x0045cdc0)
    DirectoryList* profileDirectory;
    int menuIsOpen;
    UnknownTrackGameObject3340* field_0x3340;
    UnknownControlBinding field_0x3344;
    UnknownControlBinding field_0x3380;
    UnknownControlBinding field_0x33bc;
    UnknownControlMapping* controlMapping;
    UnknownTrackGameObject33fc* field_0x33fc;
    UnknownTrackGameObject3400* field_0x3400;
    char savedDecimalSeparator[8];
    void* languageModule;
    UnknownTrackGameObject3410* networkGameObject;
    float fullNetPacketIntervalSeconds;
    float shortNetPacketIntervalSeconds;
    float fullRecordPacketIntervalSeconds;
    float shortRecordPacketIntervalSeconds;
    int field_0x3424;
    int field_0x3428;
    int field_0x342c;
    int uiInteractionBlocked;
    float field_0x3434;        // seconds UI interaction has been blocked (EventManager slot 10)
    int field_0x3438;
    int openLocalizedWebPageOnExit;
    int openStorePageOnExit;
    UnknownTrackGameObject3444* field_0x3444;
    unsigned char field_0x3448[0x34c8 - 0x3448];
    int screenSaverWasActive;
};

extern TrackGame* g_UnknownGlobal56e26c;
