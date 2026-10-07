// GameCursorNearMisses.cpp -- near miss for cursor.cpp's GameCursor slot 15
// (0x0043ed40). The canonical TU is src/reconstructed/GameCursor.cpp.
//
// Status: 871/943 positions. Everything up to the final blits matches; the
// two IDirectDrawSurface Blt branches (+0x38 frame / +0x34 image) differ in
// register allocation and in which branch's `return 1` epilogue is inlined
// (retail keeps both Blt calls with identical register roles; every source
// form tried either hoists GameObject+0x18 or tail-merges the two calls).

#include "../../src/reconstructed/GameCursor.cpp"
#include "../../src/reconstructed/TrackGame.h"

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
        GetWindowRect((HWND)g_TrackGame->field_0x31c, &window);
        GetClientRect((HWND)g_TrackGame->field_0x31c, &client);
        area = client;
        frame.left = 0;
        frame.top = 0;
        frame.right = 10;
        frame.bottom = 10;
        AdjustWindowRectEx(&frame, GetWindowLongA((HWND)g_TrackGame->field_0x31c, GWL_STYLE),
                           GetMenu((HWND)g_TrackGame->field_0x31c) != 0,
                           GetWindowLongA((HWND)g_TrackGame->field_0x31c, GWL_EXSTYLE));
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
        if (((PCRenderTarget*)field_0x18)->field_0x48->Blt(&s_destination, ((PCTextureMap*)field_0x38)->field_0x70, &s_source,
                (((PCTextureMap*)field_0x38)->field_0x30 ? 0x8000 : 0) + 0x1000000, 0))
            return 0;
    } else {
        PCRenderTarget* screen = (PCRenderTarget*)field_0x18;
        if (screen->field_0x48->Blt(&s_destination, field_0x34->field_0x70, &s_source,
                (field_0x34->field_0x30 ? 0x8000 : 0) + 0x1000000, 0))
            return 0;
    }
    return 1;
}
