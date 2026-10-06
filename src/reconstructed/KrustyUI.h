#pragma once

#include "GameObject.h"

class Camera;
class RenderTarget;

// Object at KrustyUI+0x2c (its methods sit among GUIManager.cpp's literals).
class UnknownTrackGameObject56cItem;
// Object at a GUI layer's +0xc0.
struct UnknownKrustyUIGuiLayerItem {
    unsigned char field_0x00[0x5c];
    int field_0x5c;                           // set from TrackGame+0xc4c (EventManager 0x0045e710)
};

// One of the GUI's four layers (+0x330).
struct UnknownKrustyUIGuiLayer {
    void UnknownFunction487c30(int value);    // 0x00487c30
    void UnknownFunction487d60();             // 0x00487d60

    unsigned char field_0x00[0xc0];
    UnknownKrustyUIGuiLayerItem* field_0xc0;
};

class TextureMapManager;

class UnknownKrustyUIGui {
public:
    explicit UnknownKrustyUIGui(int flags);                  // 0x00485190 (0x3e0 bytes; GUIManager.cpp)
    // 0x004853b0: sets the GUI up (font, cursor, progress callback); returns
    // the object KrustyUI adds as its child.
    GameObject* UnknownFunction4853b0(void* owner, int a, TextureMapManager* textures, int b, int c,
                                      int d, const char* font, int fontSize, const char* cursor,
                                      void (*progress)(), int e);
    void UnknownFunction485d70(const char* directory);       // 0x00485d70 ("ui")
    void UnknownFunction486560(const char* image);           // 0x00486560 ("ui\\wait.tga")
    void UnknownFunction4866c0(char* font);                  // 0x004866c0
    void UnknownFunction486630(int value);                   // 0x00486630
    void UnknownFunction485d50();                            // 0x00485d50 (KrustyUI 0x004999b0)
    UnknownTrackGameObject56cItem* UnknownFunction485df0();  // 0x00485df0
    void UnknownFunction485ef0();                            // 0x00485ef0
    UnknownKrustyUIGuiLayer* UnknownFunction486540(int index); // 0x00486540: layer `index` (0 past 3)
    void UnknownFunction4868b0(int value);                   // 0x004868b0
    // 0x00485a70: shows `dialog` (EventManager 0x0045e710 passes 0, 2, 0, 0,
    // 0, 0, 1).
    void UnknownFunction485a70(GameObject* dialog, int a, int b, int c, int d, int e, int f, int g);

    unsigned char field_0x000[0xec];
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
    void UnknownFunction4999f0(GameObject* parent); // 0x004999f0: shows the +0x464 scene
    void UnknownFunction499a20();             // 0x00499a20: hides it
    void UnknownFunction499b00();             // 0x00499b00
    void UnknownFunction499b10();             // 0x00499b10
    void UnknownFunction49a4a0();             // 0x0049a4a0: opens the exit dialog
    // 0x0049b020: fills `names` with `count` distinct random short strings.
    void UnknownFunction49b020(const char** names, int count);
    // 0x0049ba70: appends an entry to +0x60; returns its index.
    int UnknownFunction49ba70(int a, int b, int c, const char* name, int d, int e);
    void UnknownFunction4999b0();             // 0x004999b0: shutdown (the destructor's first step)
    void UnknownFunction49bb80();             // 0x0049bb80: frees +0x60
    void UnknownFunction49a540();             // 0x0049a540
    void UnknownFunction49a8b0();             // 0x0049a8b0
    GameObject* UnknownFunction4988a0(RenderTarget* target, int value); // 0x004988a0
    void UnknownFunction498cf0(int value);                         // 0x00498cf0
    void UnknownFunction499b20(int menu);                          // 0x00499b20: opens a menu
    void UnknownFunction49b530();                                  // 0x0049b530
    void UnknownFunction49bbb0();                                  // 0x0049bbb0

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
    // Garage tables per bike class (OptionProcs.cpp's OptGarageDlg).
    int field_0x68[5][3][11];                 // standard curves
    unsigned char field_0x2fc[0x324 - 0x2fc];
    int field_0x324[5][11];                   // band maxima
    int field_0x400[5];                       // total of the bands
    unsigned char field_0x414[0x43c - 0x414];
    int field_0x43c[5];                       // slider minimum
    int field_0x450[5];                       // slider maximum
    GameObject* field_0x464;                  // released by 0x004999b0
    Camera* field_0x468;                      // made current while +0x464 is shown
    unsigned char field_0x46c[0x48c - 0x46c];
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
