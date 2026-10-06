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
class FollowCamera;

// RTTI: TrackGame : PCGame (vtable 0x00558938; trkgame.cpp). The global
// pointer at 0x0056e26c is statically initialised to the object at
// 0x006851a0, which a dynamic initializer (0x004aa7c0) constructs with
// 0x00520870 (it writes TrackGame's vtable) and destroys at exit with
// 0x00521ae0. The camera and input code reads it throughout. Only members
// that reconstructed functions touch are declared; names are provisional.

// The race-mode objects at TrackGame+0x558..+0x568 (EventManager uses the
// first one present). They are GameObjects (slots 4 and 5, the +0x25 flag
// bits); their classes are not established.
struct UnknownTrackGameViewPart {
    unsigned char field_0x00[0xc4];
    GameObject* field_0xc4;                   // racesnd.cpp switches it with slot 16
};

struct TrackGameViewOwner : public GameObject {
    // 0x004e0c30 (among QuarryStuntEvent.cpp's code): shows the on/off
    // message for string `id` (racesnd.cpp passes 0x1429 and 0x14c3).
    void UnknownFunction4e0c30(int id, int value);
    void UnknownFunction4e1f00();             // 0x004e1f00 (dlgprocs.cpp LoadingDlg)
    void UnknownFunction4a9d20();             // 0x004a9d20 (dlgprocs.cpp 0x004526b0)

    UnknownTrackGameViewPart* field_0x2c;
    unsigned char field_0x30[0x34 - 0x30];
    UnknownKrustyBikeView* field_0x34;
    unsigned char field_0x38[0x3c - 0x38];
    FollowCamera* field_0x3c;                 // the race camera (InGameProcs.cpp VCRDlg)
    unsigned char field_0x40[0x5c - 0x40];
    GameObject* field_0x5c;                   // EventManager 0x0045cdc0 calls slot 4 on 0x5c..0x68
    GameObject* field_0x60;
    GameObject* field_0x64;
    GameObject* field_0x68;
    TextQueueOverlay* field_0x6c;
    float field_0x70;                         // compared with TrackGame+0x2eb0 (EventManager 0x0045eef0)
    unsigned char field_0x74[0x9c - 0x74];
    GameObject* field_0x9c;                   // racesnd.cpp calls slot 4 (sound off) or 5 (on)
    unsigned char field_0xa0[0xa8 - 0xa0];
    UnknownEventRacer* field_0xa8;
};

// A SessionInfoType record (Net.h) as TrackGame+0x1010 holds five of them
// (0x10c bytes each; TrackGameMode's constructor 0x00522060 builds them with
// SessionInfoType's constructor at its +0xa98, which is TrackGame+0x1010).
// Declared without the vtable so TrackGame's constructor does not build them.
struct UnknownTrackGameSession {
    void* field_0x000;                        // SessionInfoType vtable
    char field_0x004[0x104];                  // session name
    void* field_0x108;                        // session instance GUID
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
    void UnknownFunction523d30(const char* topic, int a); // 0x00523d30 (OptionProcs.cpp: "MCM2HELP")
    void UnknownFunction522440();             // 0x00522440 (OptionProcs.cpp)
    void UnknownFunction523000();             // 0x00523000 (OptionProcs.cpp)
    void UnknownFunction523580();                           // 0x00523580 (TrackGame slot 15)
    int UnknownFunction5238f0(const char* name, char* path); // 0x005238f0 (TrackGame slot 18)
    void UnknownFunction522680();             // 0x00522680 (TrackGame slot 4)
    void UnknownFunction522d00();             // 0x00522d00 (TrackGame slot 4)
    int UnknownFunction5231f0();              // 0x005231f0 (TrackGame slot 4)
    void UnknownFunction523e50();             // 0x00523e50 (KrustyUI 0x004988a0)
    void UnknownFunction523b70(char* name);   // 0x00523b70 (SelectGamePicProcs.cpp)
    // 0x00523a60 (near uiinfo.cpp's literals): builds the path of `name`'s
    // `kind` file (EventManager 0x0045cb70 asks for "env").
    void UnknownFunction523a60(int value, char* name, const char* kind, char* path);

    char field_0x00[16];                      // name; slot 4 sets it from the network object
    char field_0x10[0x80];                    // player name (NetProcs.cpp opens sessions with it)
    int field_0x90;                           // cycles 0..5 (TrackOverlay 0x0051e7c0)
    int field_0x94;                           // "RadLODEasy" (SelectGamePicProcs.cpp)
    int field_0x98;                           // saved +0x94 (dlgprocs.cpp SinglePlayerDlg)
    int field_0x9c;                           // cleared by MPBikeRiderDlg (SelectGamePicProcs.cpp); opens NewbieDlg (dlgprocs.cpp)
    char field_0xa0[6][0x100];                // directories (TrackRecord.cpp 0x0051f0b0)
    int field_0x6a0;                          // a directory name pointer for TrackRecord.cpp 0x0051f2c0                          // EventManager 0x0045cb70 passes it to 0x00523a60
    int field_0x6a4;                          // cleared when a network race fails to load (dlgprocs.cpp LoadingDlg)
    int field_0x6a8;                          // visual cue and runway lights on (QuarryStuntEvent.cpp)
    int field_0x6ac;                          // instrument overlay on (QuarryStuntEvent.cpp)
    int field_0x6b0;                          // shows the race statistics (TrackOverlay 0x00519980)
    int field_0x6b4;                          // 1 shows the chat input line (QuarryStuntEvent.cpp)
    int field_0x6b8;                          // shows the racers' name tags (TrackOverlay 0x005190e0)
    int field_0x6bc;                          // TrackOverlay 0x0051e390
    int field_0x6c0;                          // TrackOverlay 0x0051e390
    unsigned char field_0x6c4[0x6c8 - 0x6c4];
    int field_0x6c8;                          // copied to the GUI's +0xec (KrustyUI 0x004988a0)
    int field_0x6cc;                          // intro movie on (dlgprocs.cpp Intro1Dlg)
    unsigned char field_0x6d0[0x6d4 - 0x6d0];
    int field_0x6d4;                          // copied into GUI layer 0 (EventManager 0x0045e710)
    unsigned char field_0x6d8[0x6dc - 0x6d8];
    int field_0x6dc;                          // most AI bikes ("EditMaxAIBikes", SelectGamePicProcs.cpp)
    unsigned char field_0x6e0[0x6f4 - 0x6e0];
    char field_0x6f4[6][0x80];                // a track name per game type (SelectGamePicProcs.cpp)
    int field_0x9f4[6];                       // a track index per game type (SelectGamePicProcs.cpp)
    unsigned char field_0xa0c[0xa20 - 0xa0c];
    int field_0xa20;                          // EventManager 0x0045e710 calls TrackGame 0x00521a40 when clear
    int field_0xa24;                          // selects +0xa3c for the GUI's +0x34c (KrustyUI 0x004988a0)
    int field_0xa28;                          // sound on (racesnd.cpp)
    unsigned char field_0xa2c[0xa34 - 0xa2c];
    int field_0xa34;                          // TrackGame slot 4 audio argument
    int field_0xa38;                          // position sounds on (racesnd.cpp 0x004e39b0)
    int field_0xa3c;
    int field_0xa40;                          // sound volume (racesnd.cpp 0x004e3430)
    unsigned char field_0xa44[0xa48 - 0xa44];
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
    // 0x004e9cd0: opens `name` from the archive in `stream` (racesnd.cpp
    // reads Audio.res samples through it); 0 when not found.
    int UnknownFunction4e9cd0(UnknownTextureStream* stream, const char* name, const char* mode, int a);
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
    // OptionProcs.cpp's control mapping dialog (provisional roles).
    void UnknownFunction448c90(int device, int row, int kind, int code); // 0x00448c90
    int UnknownFunction448cc0(int row, int kind, int code, int* other);  // 0x00448cc0
    void UnknownFunction448e90(const char* path, int a);                 // 0x00448e90
    void UnknownFunction449350(int device, int row, char* text);         // 0x00449350
    int UnknownFunction449380(int kind, int code, char* text);            // 0x00449380

    int field_0x00;                           // input device (OptionProcs.cpp "InputDeviceDDL")
    unsigned char field_0x04[0x384 - 0x04];
};

// Per-racer record in TrackGame (0xf8 bytes from +0x215c, after the count at
// +0x2158; EventManager 0x0045fbd0 copies whole records). The array's
// length is not established.
struct UnknownTrackGameRacerSlot {
    // 0x00521fb0 (SelectGamePicProcs.cpp passes TrackGame+0x1eec, +0x1f2c and +0x1f6c).
    void UnknownFunction521fb0(void* a, void* b, void* c);
    void UnknownFunction522050();             // 0x00522050 (dlgprocs.cpp LoadingDlg)

    unsigned char field_0x00[0xc0];
    int field_0xc0;                           // SelectGamePicProcs.cpp
    int field_0xc4;                           // SelectGamePicProcs.cpp
    int field_0xc8;                           // set when the player leaves (EventManager slot 24)
    int field_0xcc;                           // ready
    int field_0xd0;
    int field_0xd4;                           // network player id
    int field_0xd8;                           // player's racer index (EventManager slot 24)
    char field_0xdc[0x10];                    // player name (SelectGamePicProcs.cpp)
    unsigned char field_0xec[0xf8 - 0xec];
};

// One player reported by 0x004aa670 (0x48 bytes).
struct UnknownZonePlayerRecord {
    char field_0x00[0x40];                    // name
    unsigned char field_0x40;                 // 1, other nonzero or zero: flags 2, 4 or 16
    int field_0x44;                           // score
};

// Base of the +0x3410 object; its constructor sits among
// FontTextureManager.cpp's literals.
class UnknownTrackGameObject3410Base {
public:
    UnknownTrackGameObject3410Base();         // 0x004676a0

    // The constructor clears +0 and +4; 0x004aa350 sets them
    // (MSZoneInterface.cpp).
    UnknownDirectPlay4A* field_0x00;
    UnknownDirectPlayLobby3A* field_0x04;
    UnknownZonePlayerRecord field_0x08[8];    // reported by 0x004aa670
    unsigned char field_0x248[0x298 - 0x248];
    int field_0x298;                          // the lobby's bike model (SelectGamePicProcs.cpp)
    unsigned char field_0x29c[0x32c - 0x29c];
};

// Object at TrackGame+0x3410, created for network games. It has no
// constructor of its own: `new` calls the base constructor directly and uses
// the allocation (not the constructor's result), and its method sits among
// MSZoneInterface.cpp's literals.
class UnknownTrackGameObject3410 : public UnknownTrackGameObject3410Base {
public:
    void UnknownFunction4aa350(UnknownDirectPlay4A* a, UnknownDirectPlayLobby3A* b); // 0x004aa350
    // MSZoneInterface.cpp: 0x004aa360 / 0x004aa4e0 ask the lobby for the
    // preset / rank property and copy it into `buffer` (at most `size`
    // bytes); 0x004aa670 reports `count` players through a COM object.
    int UnknownFunction4aa360(char* buffer, unsigned int size);
    int UnknownFunction4aa4e0(char* buffer, unsigned int size);
    int UnknownFunction4aa670(unsigned int count, void* a, void* b);
    void UnknownFunction49c770();             // 0x0049c770 (EventManager 0x0045e550)
};

// Object at TrackGame+0x3444 (0x00521cd0 tests that it exists): the pro
// circuit career, declared in ProCircuit.h.
#include "ProCircuit.h"

// cdecl 0x00520820 (near TrackRecord.cpp's literals): formats a message
// (_vsnprintf into 0x200 bytes) and sends it through 0x0068a48c.
void UnknownFunction520820(const char* format, ...);

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
    int field_0xfc8;                          // shadows on (QuarryStuntEvent.cpp)
    int field_0xfcc;                          // particles on (QuarryStuntEvent.cpp)
    unsigned char field_0xfd0[0xfd4 - 0xfd0];
    int field_0xfd4;                          // sky cube on (QuarryStuntEvent.cpp)
    unsigned char field_0xfd8[0xfdc - 0xfd8];
    int field_0xfdc;                          // detail level 0..9 (QuarryStuntEvent.cpp)
    unsigned char field_0xfe0[0x1010 - 0xfe0];
    UnknownTrackGameSession field_0x1010[5];   // enumerated sessions (NetProcs.cpp)
    int field_0x154c;                          // selected session (NetProcs.cpp)
    unsigned char field_0x1550[0x5c];          // restored from +0x15ac when a replay is left (InGameProcs.cpp)
    unsigned char field_0x15ac[0x5c];
    unsigned char field_0x1608[0x1664 - 0x1608];
    int field_0x1664;                          // SelectGamePicProcs.cpp
    int field_0x1668[5][3][11];                // custom garage curves per bike class (OptionProcs.cpp)
    unsigned char field_0x18fc[0x1ed4 - 0x18fc];
    int field_0x1ed4[5];                       // video settings: forced video memory, then four detail levels (OptionProcs.cpp)
    int field_0x1ee8;                          // bit 0 unlocks kind 10 bikes (SelectGamePicProcs.cpp)
    unsigned char field_0x1eec[0xc8];          // restored from +0x1fb4 when a replay is left (InGameProcs.cpp)
    unsigned char field_0x1fb4[0xc8];
    unsigned char field_0x207c[0x2144 - 0x207c];
    int field_0x2144;                         // SelectGamePicProcs.cpp
    int field_0x2148;                         // SelectGamePicProcs.cpp
    int field_0x214c;                         // SelectGamePicProcs.cpp
    int field_0x2150;                         // largest opponent count (SelectGamePicProcs.cpp)
    unsigned char field_0x2154[0x2158 - 0x2154];
    int field_0x2158;                         // racer count (EventManager 0x0045e550)
    UnknownTrackGameRacerSlot field_0x215c[8];
    int field_0x291c;                         // network game (racesnd.cpp 0x004e5780)
    unsigned char field_0x2920[0x2930 - 0x2920];
    int field_0x2930;    // saved KrustyBikeCamera state (slots 61, 62)
    float field_0x2934;  // saved KrustyBikeCamera presets (slots 59, 60)
    float field_0x2938;
    float field_0x293c;
    float field_0x2940;
    unsigned char field_0x2944[0x2b58 - 0x2944];
    DirectoryList* field_0x2b58;               // scanned by TrackRecord.cpp 0x00520390
    DirectoryList* field_0x2b5c;               // track directories (SelectGamePicProcs.cpp)
    unsigned char field_0x2b60[0x2b64 - 0x2b60];
    char field_0x2b64[0x104];                  // the ghost file raced against (dlgprocs.cpp GhostFilesDlg)
    int field_0x2c68;                          // mode 4 saves a ghost (EventManager 0x0045cdc0)
    char field_0x2c6c[0x104];                  // the replay file played (dlgprocs.cpp ReplayFilesDlg)
    int field_0x2d70;    // EventManager 0x0045e600 compares it with 2
    int field_0x2d74;    // selects KrustyBikeCamera's view (slot 10)
    int field_0x2d78;                          // SelectGamePicProcs.cpp
    int field_0x2d7c;                         // cleared by EventManager 0x0045e520
    int field_0x2d80;                          // tree collision (SelectGamePicProcs.cpp)
    int field_0x2d84;                          // rider collision (SelectGamePicProcs.cpp)
    int field_0x2d88;                          // selects a 30 (else 120) second limit (EventManager 0x0045eef0)
    int field_0x2d8c;                          // largest bike class allowed (SelectGamePicProcs.cpp)
    int field_0x2d90;                          // laps (SelectGamePicProcs.cpp)
    int field_0x2d94;                          // AI racer count (EventManager 0x0045f9a0)
    int field_0x2d98;                          // cleared by NetProcs.cpp 0x004aef40
    unsigned char field_0x2d9c[0x2da4 - 0x2d9c];
    unsigned char field_0x2da4;                // SelectGamePicProcs.cpp
    unsigned char field_0x2da5;                // decremented when a player leaves (EventManager 0x0045fbd0)
    char field_0x2da6[0x20];                   // track name (length not established)
    unsigned char field_0x2dc6[0x2ea8 - 0x2dc6];
    int field_0x2ea8;                          // SelectGamePicProcs.cpp
    int field_0x2eac;                          // SelectGamePicProcs.cpp
    float field_0x2eb0;
    int field_0x2eb4;                          // tag ball (SelectGamePicProcs.cpp)
    int field_0x2eb8;                          // mode 4 ranks by +0x768 (else +0x764) (EventManager 0x0045e9d0)
    unsigned char field_0x2ebc[0x2f5c - 0x2ebc];
    unsigned char field_0x2f5c[0x1ec];         // saved +0x2d70..+0x2f5c, restored when a replay is left (InGameProcs.cpp)
    unsigned char field_0x3148[0x3334 - 0x3148];
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
    int field_0x3424;                          // EventManager 0x0045fbd0 decrements it for a leaving player whose entry has racer +0x11c0 set (Game+0x18 otherwise)
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
