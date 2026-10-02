#pragma once

#include "GameObject.h"

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
    unsigned char field_0x00[0xc0];
    UnknownKrustyUIGuiLayerItem* field_0xc0;
};

class UnknownKrustyUIGui {
public:
    void UnknownFunction486630(int value);                   // 0x00486630
    void UnknownFunction485d50();                            // 0x00485d50 (KrustyUI 0x004999b0)
    UnknownTrackGameObject56cItem* UnknownFunction485df0();  // 0x00485df0
    void UnknownFunction485ef0();                            // 0x00485ef0
    UnknownKrustyUIGuiLayer* UnknownFunction486540(int index); // 0x00486540: layer `index` (0 past 3)
    void UnknownFunction4868b0(int value);                   // 0x004868b0
    // 0x00485a70: shows `dialog` (EventManager 0x0045e710 passes 0, 2, 0, 0,
    // 0, 0, 1).
    void UnknownFunction485a70(GameObject* dialog, int a, int b, int c, int d, int e, int f, int g);
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

// RTTI: KrustyUI : GameObject (vtable 0x0055488c; 0x1364 bytes, the size
// TrackGame slot 4 allocates). Its constructor 0x004987f0 writes the vtable.
// Only the members TrackGame calls are declared.
class KrustyUI : public GameObject {
public:
    explicit KrustyUI(int flags);                                  // 0x004987f0
    virtual ~KrustyUI();                      // 0x0049b470 (deleting wrapper 0x00498880)
    void UnknownFunction4999b0();             // 0x004999b0: shutdown (the destructor's first step)
    void UnknownFunction49bb80();             // 0x0049bb80: frees +0x60
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
    void* field_0x60;                         // also freed by 0x0049bb80
    int field_0x64;
    unsigned char field_0x68[0x464 - 0x68];
    GameObject* field_0x464;                  // released by 0x004999b0
    unsigned char field_0x468[0x48c - 0x468];
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
