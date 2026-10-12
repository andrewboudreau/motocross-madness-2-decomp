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
    bindingX = 0;
    bindingY = 0;
    cursorTexture = 0;
    currentFrame = 0;
    hotSpotX = 1;
    hotSpotY = 1;
    backgroundImage = 0;
    backgroundRegion = -1;
    field_0x4c = 0;
    field_0x50 = 1.0f;
}

// 0x0043ea70
GameObject* GameCursor::CreateBound(void* target, UnknownControlBinding* x, UnknownControlBinding* y,
                                              const char* image, TextureMapManager* textures,
                                              BackgroundImage* background, void* palette, Palette8* palette8) {
    GameObject::UnknownVirtualSlot8(target);
    hotSpotX = 1;
    hotSpotY = 1;
    cursorTexture = 0;
    backgroundImage = background;
    if (background)
        backgroundRegion = background->AllocRegion(0);
    bindingX = x;
    bindingY = y;
    if (LoadCursorTexture(image, textures, palette, palette8))
        return this;
    Release();
    return 0;
}

// 0x0043eaf0
GameObject* GameCursor::Create(void* target, const char* image, TextureMapManager* textures,
                                              BackgroundImage* background, void* palette, Palette8* palette8) {
    GameObject::UnknownVirtualSlot8(target);
    bindingX = 0;
    bindingY = 0;
    hotSpotX = 1;
    hotSpotY = 1;
    cursorTexture = 0;
    backgroundImage = background;
    if (background)
        backgroundRegion = background->AllocRegion(0);
    if (LoadCursorTexture(image, textures, palette, palette8))
        return this;
    Release();
    return 0;
}

// 0x0043eb60
GameCursor::~GameCursor() {
    if (cursorTexture)
        cursorTexture->Release();
    if (backgroundImage)
        backgroundImage->ReleaseRegion(backgroundRegion);
}

// 0x0043ebd0
int GameCursor::LoadCursorTexture(const char* image, TextureMapManager* textures, void* palette,
                                      Palette8* palette8) {
    UnknownTgaFile* file = LoadFile(image, 0, (int)g_UnknownResourceManager572b44);
    if (!file)
        return 0;

    int height = file->height;
    int width = file->width;
    if (cursorTexture)
        cursorTexture->Release();
    cursorTexture = new(__FILE__, 128) PCTextureMap(textures, 1);

    if (file->bitsPerPixel == 32) {
        if (!cursorTexture->UnknownVirtualSlot4(file->bits, width, height, width, width, 0x22b8, 0x115c, 0, 4, 0,
                                             0, 0, 2, 1, 0, 0x80, 0xff00ff))
            goto failed;
    } else {
        if (!cursorTexture->UnknownVirtualSlot4(file->bits, width, height, width, width,
                                             file->bitsPerPixel == 16 ? 0x22b : 0x378,
                                             ((PCRenderTarget*)field_0x18)->surfacePixelFormat,
                                             (UnknownTexturePalette*)palette, 4,
                                             ((PCRenderTarget*)field_0x18)->targetDisplay->field_0x198, 0, 0, 2, 1, 0,
                                             0x80, 0xff00ff))
            goto failed;
        cursorTexture->UnknownVirtualSlot18(0xff00ff);
    }
    FreeFile(file);
    return 1;

failed:
    FreeFile(file);
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
    if (bindingX && bindingY) {
        field_0x54 = (int)bindingX->currentValue;
        field_0x58 = (int)bindingY->currentValue;
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

        if (g_TrackGame->fullScreen) {
            field_0x54 = max(0, min(field_0x54, client.right));
            field_0x58 = max(0, min(field_0x58, client.bottom));
        } else if (field_0x54 < 0 || field_0x58 < 0 || field_0x54 > area.right || field_0x58 > area.bottom) {
            field_0x58 = 0;
            field_0x54 = 0;
            outside = 1;
        }

        RenderTarget* target = (RenderTarget*)field_0x18;
        field_0x54 = (int)((float)target->targetWidth / area.right * field_0x54);
        field_0x58 = (int)((float)target->targetHeight / area.bottom * field_0x58);
        if (outside)
            return 1;
    }

    if (field_0x54 < 0)
        field_0x54 = 0;
    if (field_0x58 < 0)
        field_0x58 = 0;
    RenderTarget* target = (RenderTarget*)field_0x18;
    if (field_0x54 >= target->targetWidth)
        field_0x54 = target->targetWidth - 1;
    if (field_0x58 >= target->targetHeight)
        field_0x58 = target->targetHeight - 1;

    s_right = (currentFrame ? ((PCTextureMap*)currentFrame)->textureWidth : cursorTexture->textureWidth) + field_0x54;
    s_bottom = (currentFrame ? ((PCTextureMap*)currentFrame)->textureHeight : cursorTexture->textureHeight) + field_0x58;
    if (s_right >= ((RenderTarget*)field_0x18)->targetWidth)
        s_right = ((RenderTarget*)field_0x18)->targetWidth;
    if (s_bottom >= ((RenderTarget*)field_0x18)->targetHeight)
        s_bottom = ((RenderTarget*)field_0x18)->targetHeight;

    s_destination.left = field_0x54;
    s_destination.top = field_0x58;
    s_destination.right = (int)(s_right * field_0x50);
    s_destination.bottom = (int)(s_bottom * field_0x50);
    s_source.top = 0;
    s_source.left = 0;
    s_source.right = s_right - field_0x54;
    s_source.bottom = s_bottom - field_0x58;

    if (backgroundImage) {
        PCTextureMap* frame = currentFrame ? (PCTextureMap*)currentFrame : cursorTexture;
        backgroundImage->DrawImageRect(frame, s_destination.left, s_destination.top, (CameraRect*)&s_source,
                                          (frame->usesColorKey != 0) + 0x10, backgroundRegion, 0, &field_0x4c, 0);
    } else if (currentFrame) {
        if (((PCRenderTarget*)field_0x18)->renderSurface->Blt(&s_destination, ((PCTextureMap*)currentFrame)->systemSurface, &s_source,
                (((PCTextureMap*)currentFrame)->usesColorKey ? 0x8000 : 0) + 0x1000000, 0))
            goto failed;
    } else {
        PCTextureMap* image = cursorTexture;
        if (((PCRenderTarget*)field_0x18)->renderSurface->Blt(&s_destination, image->systemSurface, &s_source,
                (image->usesColorKey ? 0x8000 : 0) + 0x1000000, 0))
            goto failed;
    }
    return 1;
failed:
    return 0;
}

// 0x0043f100
UnknownCursorPosition GameCursor::CursorPosition() {
    POINT point;
    UnknownCursorPosition position;

    if (bindingX && bindingY) {
        point.x = (int)bindingX->currentValue;
        point.y = (int)bindingY->currentValue;
    } else {
        GetCursorPos(&point);
    }
    point.x += hotSpotX;
    point.y += hotSpotY;
    position.x = point.x;
    position.y = point.y;
    return position;
}
