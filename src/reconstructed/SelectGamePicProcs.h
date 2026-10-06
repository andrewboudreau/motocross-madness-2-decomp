#pragma once

#include "DialogProc.h"
#include "MatrixUtil.h"
#include "OptionProcs.h"

// SelectGamePicProcs.cpp (__FILE__ 0x00573c6c): the multiplayer lobby
// dialogs. MultiPlayerDlg ("MPBase.dtm", built by KrustyUI 0x0049a0b0)
// hosts four pages: MPEventDlg, MPBikeRiderDlg, MPRaceInfoDlg and
// MPOptionsDlg. Names are provisional; the dialog classes are RTTI names.

class BackgroundImage;
class DirectoryList;
class TextureMapManager;
class MultiPlayerDlg;
class PlayerInfoType;

// The seven remote player records at 0x00689d08 (+0x04 the DirectPlay id,
// +0x08 the name). SceneManager.cpp currently defines them with the
// initializers 0x004f1740..0x004f1793, but only this file's code reads the
// array (0x004f212e onwards), so the array and those initializers are
// probably this file's (strong inference; not moved here).
extern PlayerInfoType g_UnknownGlobal689d08[7];

// 0x00689df4: set while the local player is ready.
extern int g_UnknownGlobal689df4;

// 0x00689df8 and 0x00689dfc (SelectGamePicProcs.cpp's .bss).
extern int g_UnknownGlobal689df8;
extern int g_UnknownGlobal689dfc;

// 0x004bfa80: the millisecond clock.
int ReadClock();

// dlgprocs.cpp's code (0x004536e0).
void UnknownFunction4536e0();
// 0x00451380: formats `value` into `text` (near dlgprocs.cpp's code).
void UnknownFunction451380(int value, char* text);
// 0x004f17a0: fills the list `listName` of `dialog` with the `pattern`
// files under `directory` and shows the first one's picture.
int UnknownFunction4f17a0(DirectoryList* directories, const char* directory, const char* pattern, const char* kind,
                          int a, const char* picture, const char* listName, char* name, int* value,
                          UIDialog* dialog, int b, int append);
// 0x004f3080: the same for the track directories of the current game type.
void UnknownFunction4f3080(int a, const char* picture, const char* listName, int* value, UIDialog* dialog, int append);

// KrustyUI+0x50's 0x94-byte bike records and the 0xc8-byte model records
// at KrustyUI+0x48 and +0x58 (bikes and riders) they index.
struct UnknownKrustyUIModelObject {
    unsigned char field_0x000[0x1a0];
    void* field_0x1a0;                        // the texture the plate number goes on
};

struct UnknownKrustyUIModel {
    char field_0x00[0x40];                    // display name
    char field_0x40[0x40];                    // name
    unsigned char field_0x80[0xc0 - 0x80];
    UnknownKrustyUIModelObject* field_0xc0;
    int field_0xc4;                           // kind
};

// The plate number painter (0x2c bytes; constructor 0x00417500 loads
// "BikeNumbers128.tga", destructor 0x00417570 releases it; code among
// BikeAI.cpp's and bikerace.cpp's literals).
class UnknownBikeNumberPainter {
public:
    UnknownBikeNumberPainter(TextureMapManager* textures); // 0x00417500
    ~UnknownBikeNumberPainter();              // 0x00417570
    void UnknownFunction417670(void* texture, int number); // 0x00417670: draws `number`

    unsigned char field_0x00[0x2c];
};

struct UnknownKrustyUIBike {
    int field_0x00;                           // model (KrustyUI+0x48)
    char field_0x04[0x44];                    // display name
    char field_0x48[0x40];
    int field_0x88;
    int field_0x8c;                           // engine size
    int field_0x90;
};

// The race settings at TrackGame+0x2d70 (0x1ec bytes; InGameProcs.cpp
// saves and restores the block). Declared as a view: TrackGame.h names the
// same fields individually.
struct UnknownRaceSettings {
    int field_0x00;                           // race mode (+0x2d70)
    int field_0x04;                           // event type (+0x2d74)
    unsigned char field_0x08[0x0c - 0x08];
    int field_0x0c;                           // races (+0x2d7c)
    unsigned char field_0x10[0x20 - 0x10];
    int field_0x20;                           // laps (+0x2d90)
    unsigned char field_0x24[0x34 - 0x24];
    unsigned char field_0x34;                 // track number (+0x2da4)
    unsigned char field_0x35;
    char field_0x36[0x100];                   // track name (+0x2da6)
    unsigned char field_0x136[0x138 - 0x136];
    int field_0x138;
    int field_0x13c;
    float field_0x140;                        // minutes (+0x2eb0)
    int field_0x144;                          // tag ball (+0x2eb4)
    int field_0x148;                          // stunt mode (+0x2eb8)
    unsigned char field_0x14c[0x1ec - 0x14c];
};

// One lobby slot (MultiPlayerDlg+0x7f68 holds eight).
struct UnknownLobbySlot {
    int field_0x00;                           // player id (0 when free)
    int field_0x04;                           // requested racers
    int field_0x08;                           // granted racers
};

// MultiPlayerDlg+0x7f68. Its constructor 0x004f2f00 is out-of-line
// (KrustyUI 0x0049a0c9 and 0x0049a21e call it).
class UnknownLobbySlotTable {
public:
    UnknownLobbySlotTable();                  // 0x004f2f00
    void UnknownFunction4f2f20(int id, int value); // 0x004f2f20: adds or updates `id`
    void UnknownFunction4f2f80(int id);       // 0x004f2f80: frees `id`'s slot
    int UnknownFunction4f2fe0(int count, int limit); // 0x004f2fe0

    UnknownLobbySlot field_0x00[8];
};

// The lobby settings message (type 0xf, 0xb4 bytes) the host sends.
struct UnknownLobbySettingsMessage {
    int field_0x00;
    unsigned char minutes : 5;                // TrackGame+0x2eb0
    unsigned char eventType : 3;              // +0x2d74
    unsigned char raceMode : 3;               // +0x2d70
    unsigned char laps : 5;                   // +0x2d90
    unsigned char opponents : 4;              // +0x2d98
    unsigned char players : 4;                // +0x2da5
    unsigned char races : 3;                  // +0x2d7c
    unsigned char field_0x2d78 : 3;           // +0x2d78
    unsigned char treeCollision : 1;          // +0x2d80
    unsigned char riderCollision : 1;         // +0x2d84
    unsigned char fastFinishes : 1;           // +0x2d88
    unsigned char bikeClass : 3;              // +0x2d8c
    unsigned char tagBall : 1;                // +0x2eb4
    unsigned char stuntMode : 1;              // +0x2eb8
    unsigned char detail : 2;                 // +0x60c
    unsigned char field_0x09;                 // track number (+0x2da4)
    int field_0x0c;                           // +0x2ea8
    int field_0x10;                           // +0x2eac
    char field_0x14[0x40];                    // track name (+0x2da6)
    UnknownLobbySlot field_0x54[8];           // MultiPlayerDlg+0x7f68
};

// RTTI: MPEventDlg : UIDialog (vtable 0x00557a88; 0x7f5c bytes).
class MPEventDlg : public UIDialog {
public:
    MPEventDlg() : UIDialog(1, "MPEvent.dtm") {}
    virtual int UnknownVirtualSlot24(int type, void* data, int from, int to, int flags); // 0x004f5f30
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004f6370
    void UnknownFunction4f6070();             // 0x004f6070: shows the host's settings
    void UnknownFunction4f69d0();             // 0x004f69d0
    void UnknownFunction4f6e60();             // 0x004f6e60: applies the event type
    void UnknownVirtualSlot31(int apply);     // 0x004f7210: stores (or shows) the event settings
    void UnknownFunction4f75d0();             // 0x004f75d0: shows the track picture
    void UnknownFunction4f7640(UnknownGameUiControl* picture, const char* directory, const char* name); // 0x004f7640

    MultiPlayerDlg* field_0x2c;               // parent dialog
    unsigned char field_0x30[0x7f3c - 0x30];
    GameObject* field_0x7f3c;                 // the controls
    unsigned char field_0x7f40[0x7f58 - 0x7f40];
    int field_0x7f58;
};

// RTTI: MPBikeRiderDlg : UIDialog (vtable 0x00557a04; 0x7f9c bytes).
class MPBikeRiderDlg : public UIDialog {
public:
    MPBikeRiderDlg() : UIDialog(1, "MPBikeR.dtm") {}
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004f8780
    virtual int UnknownVirtualSlot24(int type, void* data, int from, int to, int flags); // 0x004f8620
    void UnknownVirtualSlot31(int apply);     // 0x004f91e0: stores the chosen bike and rider
    void UnknownFunction4f8650();             // 0x004f8650
    void UnknownFunction4f8700();             // 0x004f8700
    void UnknownFunction4f8570(int number);   // 0x004f8570: paints the plate number on every bike
    void UnknownFunction4f8220();             // 0x004f8220: fills the bike and rider lists
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004f78a0
    void UnknownFunction4f8d20();             // 0x004f8d20: applies the chosen bike
    void UnknownFunction4500d0();             // 0x004500d0 (in dlgprocs.cpp's code): applies the chosen rider

    MultiPlayerDlg* field_0x2c;               // parent dialog
    GUIManager* field_0x30;
    GUIUser* field_0x34;
    unsigned char field_0x38[0x110 - 0x38];
    BackgroundImage* field_0x110;
    unsigned char field_0x114[0x7f58 - 0x114];
    Vector3 field_0x7f58;                     // the bike view's eye
    Vector3 field_0x7f64;                     // and target
    float field_0x7f70;                       // their distance
    int field_0x7f74;                         // background region
    int field_0x7f78;
    int field_0x7f7c;
    int field_0x7f80;
    int field_0x7f84;
    int field_0x7f88;
    RECT field_0x7f8c;                        // the bike view
};

// RTTI: MPRaceInfoDlg : UIDialog (vtable 0x00557980; 0x7f58 bytes; its
// procedure 0x004513a0 is not in this file).
class MPRaceInfoDlg : public UIDialog {
public:
    MPRaceInfoDlg() : UIDialog(1, "MPRInfo.dtm") {}

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: MPOptionsDlg : UIDialog (vtable 0x005578fc; 0x7f58 bytes).
class MPOptionsDlg : public UIDialog {
public:
    MPOptionsDlg() : UIDialog(1, "MPOpt.dtm") {}
    virtual int UnknownVirtualSlot24(int type, void* data, int from, int to, int flags); // 0x004f9570
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004f93c0
    void UnknownFunction4f95a0();             // 0x004f95a0
    void UnknownFunction4f95b0();             // 0x004f95b0

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: MultiPlayerDlg : UIDialog (vtable 0x00554b14; 0x8110 bytes, the
// size KrustyUI 0x0049a08b allocates).
class MultiPlayerDlg : public UIDialog {
public:
    void UnknownFunction4f20d0();             // 0x004f20d0: fills the racer slots for the race
    void UnknownFunction4f2b90();             // 0x004f2b90: sends the lobby settings
    int UnknownFunction4f2ec0();              // 0x004f2ec0
    void UnknownFunction4f3260();             // 0x004f3260
    void UnknownFunction4f3620();             // 0x004f3620
    void UnknownFunction4f3720(int player, const char* text); // 0x004f3720: adds a chat line
    void UnknownFunction4f38a0();             // 0x004f38a0: sends the typed chat line
    void UnknownFunction4f3980(const char* text); // 0x004f3980: sends a system chat line
    void UnknownFunction4f3a10(int index);    // 0x004f3a10: removes player `index`
    void UnknownFunction4f57c0(int reason);   // 0x004f57c0
    void UnknownFunction4f5a00(int page);     // 0x004f5a00: shows page `page`
    void UnknownFunction4f5ca0();             // 0x004f5ca0: follows a track change

    unsigned char field_0x2c[0x34 - 0x2c];
    GUIUser* field_0x34;
    unsigned char field_0x38[0x7f58 - 0x38];
    MPEventDlg* field_0x7f58;
    MPBikeRiderDlg* field_0x7f5c;
    MPRaceInfoDlg* field_0x7f60;
    MPOptionsDlg* field_0x7f64;
    UnknownLobbySlotTable field_0x7f68;
    char field_0x7fc8[0x104];                 // track name the page was built for
    int field_0x80cc;
    char field_0x80d0[0x40];
};
