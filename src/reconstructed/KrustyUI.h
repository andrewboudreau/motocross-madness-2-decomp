#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"

class Camera;
class RenderTarget;
struct UnknownKrustyUIModelObject;

// Object at KrustyUI+0x2c (its methods sit among GUIManager.cpp's literals).
class UnknownTrackGameObject56cItem;
// Object at a GUI layer's +0xc0.
struct UnknownKrustyUIGuiLayerItem {
    unsigned char field_0x00[0x5c];
    int field_0x5c;                           // set from TrackGame+0xc4c (EventManager 0x0045e710)
};

// One of the GUI's four layers (+0x330).
struct UnknownKrustyUIGuiLayer {
    void AcceptDevice(int value);    // 0x00487c30
    void ForgetDevices();             // 0x00487d60

    unsigned char field_0x00[0x30];
    GameObject* field_0x30;                   // the cursor (bikerace.cpp slot 23 tests its +0x25 bit 0)
    unsigned char field_0x34[0xc0 - 0x34];
    UnknownKrustyUIGuiLayerItem* field_0xc0;
};

class TextureMapManager;
class BackgroundImage;

class UnknownKrustyUIGui {
public:
    explicit UnknownKrustyUIGui(int flags);                  // 0x00485190 (0x3e0 bytes; GUIManager.cpp)
    // 0x004853b0: sets the GUI up (font, cursor, progress callback); returns
    // the object KrustyUI adds as its child.
    GameObject* SetUp(void* owner, int a, TextureMapManager* textures, int b, int c,
                      int d, const char* font, int fontSize, const char* cursor,
                      void (*progress)(), int e);
    void UnknownFunction485d70(const char* directory);       // 0x00485d70 ("ui")
    void UnknownFunction486560(const char* image);           // 0x00486560 ("ui\\wait.tga")
    void UnknownFunction4866c0(char* font);                  // 0x004866c0
    void ShowCursors(int value);                             // 0x00486630
    void CloseDialogResource();                            // 0x00485d50 (KrustyUI 0x004999b0)
    UnknownTrackGameObject56cItem* FindInputDialog();        // 0x00485df0
    void CreateBackground();                                 // 0x00485ef0
    UnknownKrustyUIGuiLayer* GetUser(int index); // 0x00486540: layer `index` (0 past 3)
    void UnknownFunction486590(const char* image, int visible); // 0x00486590 (bikerace.cpp: "ui\\cursor.tga")
    void EnableWindowClipper(int value);                     // 0x004868b0
    // 0x00485a70: shows `dialog` (EventManager 0x0045e710 passes 0, 2, 0, 0,
    // 0, 0, 1).
    void ShowDialog(GameObject* dialog, int a, int b, int c, int d, int e, int f, int g);
    void ReleaseBackground();                            // 0x00485fc0: releases the background

    unsigned char field_0x000[0x3c];
    BackgroundImage* field_0x03c;             // the background (dlgprocs.cpp 0x004536e0)
    unsigned char field_0x040[0xec - 0x40];
    int field_0x0ec;                          // from TrackGame+0xc40 (KrustyUI 0x004988a0)
    unsigned char field_0x0f0[0x30c - 0xf0];
    int field_0x30c;
    unsigned char field_0x310[0x34c - 0x310];
    int field_0x34c;
    char field_0x350[0x3d0 - 0x350];          // font face (TrackOverlay 0x0051b320)
    float field_0x3d0;                        // string 0xfed's value when nonzero
    int field_0x3d4;                          // string 0x1469's value; bold font
    int field_0x3d8;                          // italic font (TrackOverlay 0x0051b320)
    unsigned char field_0x3dc[0x3e0 - 0x3dc];
};


// Page object at +0x490 (see samples/game/EventManagerNearMisses.cpp).
class UnknownGameUiPage;

// 8-byte record; KrustyUI keeps eight at +0x634 (the constructor's loop
// clears both fields of each).
struct UnknownKrustyUISlot {
    UnknownKrustyUISlot() : field_0x00(0), field_0x04(0) {}

    int field_0x00;
    char field_0x04;
};

// 0x54-byte record in KrustyUI's +0x60 list (appended by 0x0049ba70).
struct UnknownKrustyUIEntry {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    char field_0x14[64];
};

// RTTI: KrustyUI : GameObject (vtable 0x0055488c; 0x1364 bytes, the size
// TrackGame slot 4 allocates). Its constructor 0x004987f0 writes the vtable.
// Only the members TrackGame calls are declared.
class KrustyUI : public GameObject {
public:
    explicit KrustyUI(int flags);                                  // 0x004987f0
    virtual ~KrustyUI();                      // 0x0049b470 (deleting wrapper 0x00498880)
    virtual void UnknownVirtualSlot4();       // 0x00499ad0
    virtual void UnknownVirtualSlot5();       // 0x00499ac0
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00499ae0
    virtual int UnknownVirtualSlot18();       // 0x00499af0
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00499a40
    virtual int UnknownVirtualSlot24(int type, void* data, int from, int to, int flags); // 0x00499a70
    virtual int UnknownVirtualSlot25(void* value); // 0x00499980
    void ShowScene(GameObject* parent); // 0x004999f0: shows the +0x464 scene
    void HideScene();             // 0x00499a20: hides it
    // 0x0049b560 / 0x0049b7f0 (bikerace.cpp passes TrackGame+0x1f2c and
    // +0x1f6c): copy `name` to `text` (`size` bytes) and return 1 when it is
    // an archive entry or a file (in "Res" when it has no directory; then
    // `text` gets that path). Otherwise `text` gets a random bike (+0x50) or
    // rider (+0x58) name and they return 0.
    int UnknownFunction49b560(const char* name, char* text, int size);
    int UnknownFunction49b7f0(const char* name, char* text, int size);
    void UnknownFunction499b00();             // 0x00499b00
    void UnknownFunction499b10();             // 0x00499b10
    void OpenExitDialog();             // 0x0049a4a0: opens the exit dialog
    // 0x0049b020: fills `names` with `count` distinct random short strings.
    void UnknownFunction49b020(const char** names, int count);
    // 0x0049b0d0: picks `count` distinct random bikes and riders for the AI
    // racers (bikes near the player's class).
    void UnknownFunction49b0d0(int* bikes, int count, int* riders);
    // 0x0049ba70: appends an entry to +0x60; returns its index.
    int UnknownFunction49ba70(int a, int b, int c, const char* name, int d, int e);
    void Shutdown();             // 0x004999b0: shutdown (the destructor's first step)
    void UnknownFunction49bb80();             // 0x0049bb80: frees +0x60
    void LoadGarageTables();             // 0x0049a540: loads the garage tables
    void UnknownFunction49a8b0();             // 0x0049a8b0
    GameObject* UnknownFunction4988a0(RenderTarget* target, int value); // 0x004988a0
    void UnknownFunction498cf0(int value);                         // 0x00498cf0
    void OpenMenu(int menu);                                       // 0x00499b20: opens a menu
    void UnknownFunction49b530();                                  // 0x0049b530
    // 0x0049bbb0: records the finished race in the high-score tables
    // (TrackGame+0x3400): table 0, then 1 and 2 by laps (5, 10) or, in
    // modes 0 and 4, by the +0x140 setting (5.0, 10.0).
    void UnknownFunction49bbb0();
    void UnknownFunction49bc50(int table);                         // 0x0049bc50

    UnknownKrustyUIGui* field_0x2c;
    int field_0x30;
    int field_0x34;
    int field_0x38;
    int field_0x3c;                           // current menu
    int field_0x40;
    int field_0x44;                           // cleared by EventManager 0x0045cdc0
    void* field_0x48;                         // DebugMalloc'd; freed by the destructor
    int field_0x4c;
    void* field_0x50;
    int field_0x54;
    void* field_0x58;
    int field_0x5c;
    UnknownKrustyUIEntry* field_0x60;         // also freed by 0x0049bb80
    int field_0x64;                           // entry count
    // Garage tables per bike class (OptionProcs.cpp's OptGarageDlg), read
    // from presets.pb by 0x0049a540 (keys in the comments).
    int field_0x68[5][3][11];                 // standard curves ("Category_%dPreset_%d" RPM%05d)
    int field_0x2fc[5];                       // "RPMLowerLimit"
    int field_0x310[5];                       // "RPMUpperLimit"
    int field_0x324[5][11];                   // band maxima ("Category_%d" RPM%05d)
    int field_0x400[5];                       // total of the bands ("HPSum")
    int field_0x414[5];                       // RPM step: a tenth of the limits' range
    int field_0x428[5];                       // "Weight"
    int field_0x43c[5];                       // slider minimum ("MinRange")
    int field_0x450[5];                       // slider maximum (the largest band maximum)
    GameObject* field_0x464;                  // released by 0x004999b0
    Camera* field_0x468;                      // made current while +0x464 is shown
    UnknownKrustyUIModelObject* field_0x46c;  // the rider preview (dlgprocs.cpp SPBikeRiderDlg slot 13)
    UnknownKrustyUIModelObject* field_0x470;  // the garage character (dlgprocs.cpp SPBikeRiderDlg slot 10)
    Vector3 field_0x474;                      // the +0x464 scene's focus (SelectGamePicProcs.cpp)
    unsigned char field_0x480[0x48c - 0x480];
    int field_0x48c;
    UnknownGameUiPage* field_0x490;
    void* field_0x494;                        // input context, restored to the window (EventManager 0x0045e710)
    int field_0x498;
    int field_0x49c;
    int field_0x4a0;
    int field_0x4a4;
    int field_0x4a8;
    int field_0x4ac;
    int field_0x4b0;
    unsigned char field_0x4b4[0x634 - 0x4b4];
    UnknownKrustyUISlot field_0x634[8];
    unsigned char field_0x674[0x1364 - 0x674];
};
