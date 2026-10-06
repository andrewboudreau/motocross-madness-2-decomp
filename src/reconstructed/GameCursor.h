#pragma once

#include "GameObject.h"

// cursor.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\cursor.cpp" at
// 0x0056890c, referenced by 0x0043ebd0). Strong inference, the TU is
// 0x0043e870..0x0043f15f: its .CRT$XCU entries 0x0043e870..0x0043e960 (the
// four per-TU vector initializers) sit between cubedraw.cpp's and the next
// file's, cubedraw.cpp's code ends at 0x0043e865 and D3DIMSoulTree.CPP's
// starts at 0x0043f160. The out-of-line inline copies 0x0043e9b0 and
// 0x0043e9e0 (Parameterblocks.h's stream accessors) also lie inside it.
// Member names are provisional.

class BackgroundImage;
class Palette8;
class PCTextureMap;
class TextureMapManager;
struct UnknownControlBinding;

// Cursor position in pixels (0x0043f100's result).
struct UnknownCursorPosition {
    int x;
    int y;
};

// RTTI: GameCursor : GameObject (vtable 0x0055130c; 0x5c bytes, GUICursor's
// first member is at +0x5c). Overrides only the destructor and slot 15.
class GameCursor : public GameObject {
public:
    explicit GameCursor(int flags);           // 0x0043ea00
    virtual ~GameCursor();                    // 0x0043eb60 (deleting wrapper 0x0043ea50)
    // 0x0043ed40: moves the cursor to the bindings' (or the mouse's)
    // position and draws it.
    virtual int UnknownVirtualSlot15();
    // 0x0043ea70: an input-driven cursor following the two bindings.
    GameObject* UnknownFunction43ea70(void* target, UnknownControlBinding* x, UnknownControlBinding* y,
                                      const char* image, TextureMapManager* textures,
                                      BackgroundImage* background, void* palette, Palette8* palette8);
    // 0x0043eaf0: a cursor without bindings.
    GameObject* UnknownFunction43eaf0(void* target, const char* image, TextureMapManager* textures,
                                      BackgroundImage* background, void* palette, Palette8* palette8);
    // 0x0043ebd0: loads `image` into a new texture (+0x34); 0 on failure.
    int UnknownFunction43ebd0(const char* image, TextureMapManager* textures, void* palette,
                              Palette8* palette8);
    // 0x0043f100: the bindings' values (or GetCursorPos's) plus the offset at
    // +0x2c/+0x30.
    UnknownCursorPosition UnknownFunction43f100();

    int field_0x2c;                           // hot spot offset
    int field_0x30;
    PCTextureMap* field_0x34;                 // image
    int field_0x38;                           // current frame (a PCTextureMap*), 0: the image
    UnknownControlBinding* field_0x3c;        // x
    UnknownControlBinding* field_0x40;        // y
    BackgroundImage* field_0x44;
    int field_0x48;                           // background region, -1 when none
    int field_0x4c;
    float field_0x50;                         // scale of the destination's far corner
    int field_0x54;                           // position
    int field_0x58;
};
