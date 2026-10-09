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
#include "TrackGame.h"
#include "UnknownResourceManager.h"

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

// A branching absolute value (retail does not use the abs intrinsic here).
#define UNKNOWN_ABS(x) ((x) < 0 ? -(x) : (x))

// Slot 15's drawing state (0x0057eea0..0x0057eed7).
static RECT s_destination;                    // 0x0057eea0
static int s_bottom;                          // 0x0057eeb0
static RECT s_source;                         // 0x0057eeb8
static int s_right;                           // 0x0057eec8
static POINT s_mouse;                         // 0x0057eed0

// 0x0043ed40
int GameCursor::UnknownVirtualSlot15() {
    GameObject::UnknownVirtualSlot15();

    int outside = 0;
    if (field_0x3c && field_0x40) {
        field_0x54 = (int)field_0x3c->field_0x24;
        field_0x58 = (int)field_0x40->field_0x24;
    } else {
        RECT window;
        RECT client;
        RECT area;
        RECT frame;

        GetCursorPos(&s_mouse);
        GetWindowRect((HWND)g_TrackGame->windowHandle, &window);
        GetClientRect((HWND)g_TrackGame->windowHandle, &client);
        area = client;
        frame.left = 0;
        frame.top = 0;
        frame.right = 10;
        frame.bottom = 10;
        AdjustWindowRectEx(&frame, GetWindowLongA((HWND)g_TrackGame->windowHandle, GWL_STYLE),
                           GetMenu((HWND)g_TrackGame->windowHandle) != 0,
                           GetWindowLongA((HWND)g_TrackGame->windowHandle, GWL_EXSTYLE));
        client.left = UNKNOWN_ABS(frame.left) + window.left;
        client.right += UNKNOWN_ABS(frame.left);
        client.top = UNKNOWN_ABS(frame.top) + window.top;
        client.bottom += UNKNOWN_ABS(frame.top);
        field_0x54 = s_mouse.x - client.left;
        field_0x58 = s_mouse.y - client.top;

        if (g_TrackGame->field_0x2d4_bit1) {
            field_0x54 = max(0, min(field_0x54, client.right));
            field_0x58 = max(0, min(field_0x58, client.bottom));
        } else if (field_0x54 < 0 || field_0x58 < 0 || field_0x54 > area.right || field_0x58 > area.bottom) {
            field_0x58 = 0;
            field_0x54 = 0;
            outside = 1;
        }

        RenderTarget* target = (RenderTarget*)field_0x18;
        field_0x54 = (int)((float)target->field_0x0c / area.right * field_0x54);
        field_0x58 = (int)((float)target->field_0x10 / area.bottom * field_0x58);
        if (outside)
            return 1;
    }

    if (field_0x54 < 0)
        field_0x54 = 0;
    if (field_0x58 < 0)
        field_0x58 = 0;
    RenderTarget* target = (RenderTarget*)field_0x18;
    if (field_0x54 >= target->field_0x0c)
        field_0x54 = target->field_0x0c - 1;
    if (field_0x58 >= target->field_0x10)
        field_0x58 = target->field_0x10 - 1;

    s_right = (field_0x38 ? ((PCTextureMap*)field_0x38)->field_0x14 : field_0x34->field_0x14) + field_0x54;
    s_bottom = (field_0x38 ? ((PCTextureMap*)field_0x38)->field_0x18 : field_0x34->field_0x18) + field_0x58;
    if (s_right >= ((RenderTarget*)field_0x18)->field_0x0c)
        s_right = ((RenderTarget*)field_0x18)->field_0x0c;
    if (s_bottom >= ((RenderTarget*)field_0x18)->field_0x10)
        s_bottom = ((RenderTarget*)field_0x18)->field_0x10;

    s_destination.left = field_0x54;
    s_destination.top = field_0x58;
    s_destination.right = (int)(s_right * field_0x50);
    s_destination.bottom = (int)(s_bottom * field_0x50);
    s_source.top = 0;
    s_source.left = 0;
    s_source.right = s_right - field_0x54;
    s_source.bottom = s_bottom - field_0x58;

    if (field_0x44) {
        PCTextureMap* frame = field_0x38 ? (PCTextureMap*)field_0x38 : field_0x34;
        field_0x44->UnknownFunction404700(frame, s_destination.left, s_destination.top, (CameraRect*)&s_source,
                                          (frame->field_0x30 != 0) + 0x10, field_0x48, 0, &field_0x4c, 0);
    } else if (field_0x38) {
        if (((PCRenderTarget*)field_0x18)->renderSurface->Blt(&s_destination, ((PCTextureMap*)field_0x38)->systemSurface, &s_source,
                (((PCTextureMap*)field_0x38)->field_0x30 ? 0x8000 : 0) + 0x1000000, 0))
            goto failed;
    } else {
        PCTextureMap* image = field_0x34;
        if (((PCRenderTarget*)field_0x18)->renderSurface->Blt(&s_destination, image->systemSurface, &s_source,
                (image->field_0x30 ? 0x8000 : 0) + 0x1000000, 0))
            goto failed;
    }
    return 1;
failed:
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
