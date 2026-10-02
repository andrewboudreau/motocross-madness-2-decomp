#pragma once

#include "GameObject.h"

class RenderTarget;

// Object at KrustyUI+0x2c (its methods sit among GUIManager.cpp's literals).
class UnknownTrackGameObject56cItem;
class UnknownKrustyUIGui {
public:
    void UnknownFunction486630(int value);                   // 0x00486630
    UnknownTrackGameObject56cItem* UnknownFunction485df0();  // 0x00485df0
};

// RTTI: KrustyUI : GameObject (vtable 0x0055488c; 0x1364 bytes, the size
// TrackGame slot 4 allocates). Its constructor 0x004987f0 writes the vtable.
// Only the members TrackGame calls are declared.
class KrustyUI : public GameObject {
public:
    explicit KrustyUI(int flags);                                  // 0x004987f0
    GameObject* UnknownFunction4988a0(RenderTarget* target, int value); // 0x004988a0
    void UnknownFunction498cf0(int value);                         // 0x00498cf0
    void UnknownFunction499b20(int menu);                          // 0x00499b20: opens a menu

    UnknownKrustyUIGui* field_0x2c;
    unsigned char field_0x30[0x3c - 0x30];
    int field_0x3c;                           // current menu
    unsigned char field_0x40[0x4a8 - 0x40];
    int field_0x4a8;
    unsigned char field_0x4ac[0x1364 - 0x4ac];
};
