// GameCursor.cpp -- reconstruction of D:\aardvark\VC\krusty2\cursor.cpp.
// See GameCursor.h for the TU extent and the class. Slot 15 (0x0043ed40)
// is a near miss in samples/ui/GameCursorNearMisses.cpp.

#include <windows.h>

#include "GameCursor.h"

#include "BackgroundImage.h"
#include "ControlInterface.h"
#include "DebugAlloc.h"
#include "MatrixUtil.h"
#include "PCRenderTarget.h"
#include "PCTextureMap.h"
#include "Tgafile.h"
#include "UnknownResourceManager.h"

// The four vector constants that open many retail files: 0x00579920,
// 0x00579a60, 0x0057a200 and 0x005798a8, built by 0x0043e870..0x0043e9ab.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x0043ea00
GameCursor::GameCursor(int flags) : GameObject(flags) {
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x2c = 1;
    field_0x30 = 1;
    field_0x44 = 0;
    field_0x48 = -1;
    field_0x4c = 0;
    field_0x50 = 1.0f;
}

// 0x0043ea70
GameObject* GameCursor::UnknownFunction43ea70(void* target, UnknownControlBinding* x, UnknownControlBinding* y,
                                              const char* image, TextureMapManager* textures,
                                              BackgroundImage* background, void* palette, Palette8* palette8) {
    GameObject::UnknownVirtualSlot8(target);
    field_0x2c = 1;
    field_0x30 = 1;
    field_0x34 = 0;
    field_0x44 = background;
    if (background)
        field_0x48 = background->UnknownFunction4040f0(0);
    field_0x3c = x;
    field_0x40 = y;
    if (UnknownFunction43ebd0(image, textures, palette, palette8))
        return this;
    Release();
    return 0;
}

// 0x0043eaf0
GameObject* GameCursor::UnknownFunction43eaf0(void* target, const char* image, TextureMapManager* textures,
                                              BackgroundImage* background, void* palette, Palette8* palette8) {
    GameObject::UnknownVirtualSlot8(target);
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x2c = 1;
    field_0x30 = 1;
    field_0x34 = 0;
    field_0x44 = background;
    if (background)
        field_0x48 = background->UnknownFunction4040f0(0);
    if (UnknownFunction43ebd0(image, textures, palette, palette8))
        return this;
    Release();
    return 0;
}

// 0x0043eb60
GameCursor::~GameCursor() {
    if (field_0x34)
        field_0x34->Release();
    if (field_0x44)
        field_0x44->UnknownFunction404200(field_0x48);
}

// 0x0043ebd0
int GameCursor::UnknownFunction43ebd0(const char* image, TextureMapManager* textures, void* palette,
                                      Palette8* palette8) {
    UnknownTgaFile* file = UnknownFunction5125c0(image, 0, (int)g_UnknownResourceManager572b44);
    if (!file)
        return 0;

    int height = file->height;
    int width = file->width;
    if (field_0x34)
        field_0x34->Release();
    field_0x34 = new(__FILE__, 128) PCTextureMap(textures, 1);

    if (file->bitsPerPixel == 32) {
        if (!field_0x34->UnknownVirtualSlot4(file->bits, width, height, width, width, 0x22b8, 0x115c, 0, 4, 0,
                                             0, 0, 2, 1, 0, 0x80, 0xff00ff))
            goto failed;
    } else {
        if (!field_0x34->UnknownVirtualSlot4(file->bits, width, height, width, width,
                                             file->bitsPerPixel == 16 ? 0x22b : 0x378,
                                             ((PCRenderTarget*)field_0x18)->field_0x28,
                                             (UnknownTexturePalette*)palette, 4,
                                             ((PCRenderTarget*)field_0x18)->field_0x04->field_0x198, 0, 0, 2, 1, 0,
                                             0x80, 0xff00ff))
            goto failed;
        field_0x34->UnknownVirtualSlot18(0xff00ff);
    }
    UnknownFunction512dd0(file);
    return 1;

failed:
    UnknownFunction512dd0(file);
    return 0;
}

// 0x0043f100
UnknownCursorPosition GameCursor::UnknownFunction43f100() {
    POINT point;
    UnknownCursorPosition position;

    if (field_0x3c && field_0x40) {
        point.x = (int)field_0x3c->field_0x24;
        point.y = (int)field_0x40->field_0x24;
    } else {
        GetCursorPos(&point);
    }
    point.x += field_0x2c;
    point.y += field_0x30;
    position.x = point.x;
    position.y = point.y;
    return position;
}
