#include <math.h>
#include <string.h>

#include "TrackOverlay.h"
#include "DebugAlloc.h"
#include "BikeCamera.h"
#include "Camera.h"
#include "KrustyUI.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"

// GDI32 and USER32 imports (0x00550054..0x00550088, 0x005502e8).
extern "C" __declspec(dllimport) void* __stdcall CreateFontA(int height, int width, int escapement,
                                                             int orientation, int weight,
                                                             unsigned long italic,
                                                             unsigned long underline,
                                                             unsigned long strikeOut,
                                                             unsigned long charSet,
                                                             unsigned long outPrecision,
                                                             unsigned long clipPrecision,
                                                             unsigned long quality,
                                                             unsigned long pitchAndFamily,
                                                             const char* face);
extern "C" __declspec(dllimport) int __stdcall DeleteObject(void* object);
extern "C" __declspec(dllimport) void* __stdcall SelectObject(void* dc, void* object);
extern "C" __declspec(dllimport) unsigned long __stdcall SetBkColor(void* dc, unsigned long color);
extern "C" __declspec(dllimport) int __stdcall SetBkMode(void* dc, int mode);
extern "C" __declspec(dllimport) unsigned long __stdcall SetTextColor(void* dc, unsigned long color);
extern "C" __declspec(dllimport) int __stdcall TextOutA(void* dc, int x, int y, const char* text, int length);
extern "C" __declspec(dllimport) int __stdcall DrawTextA(void* dc, const char* text, int length,
                                                         UnknownOverlayRect* rect, unsigned int format);

// 0x00575140, 0x0068a448: the previous state flag and the decaying value.
static int s_UnknownGlobal575140 = 1;
static float s_UnknownGlobal68a448;

// 0x00518720
InstrumentOverlay::InstrumentOverlay(int flags) : Overlay(flags, 1)
{
    field_0x110 = 1;
    field_0x184 = 0;
}

// 0x00518770: loads the dial texture and places the needle for the
// render target's size (the layout is for 640x480).
InstrumentOverlay* InstrumentOverlay::UnknownFunction518770(RenderTarget* target, TextureMapManager* manager, UnknownInstrumentSource* a3)
{
    UnknownOverlayRect rect;
    char name[260];
    field_0x184 = a3;
    strcpy(name, "instr1.tga");
    rect.left = 511;
    rect.right = 639;
    rect.top = 351;
    rect.bottom = 479;
    TextureMap* texture = UnknownFunction50a590(manager, name, 0x115c, 0, 0, 5, 6, 0, 0x80,
                                                0xff00ff, 1, 1);
    if (!texture) {
        Release();
        return 0;
    }
    texture->UnknownVirtualSlot8(1, 0, 0);
    UnknownFunction4b5f50(target, texture, &rect, 1, 0, 0.00001f, 0, 0, 0, 0, 0x613, 0);
    RenderTarget* owner = Target();
    field_0x11c.field_0x00 = (int)(rect.left * owner->field_0x0c / 640.0f);
    field_0x11c.field_0x04 = (int)(rect.top * owner->field_0x10 / 480.0f);
    field_0x11c.field_0x08 = (int)((owner->field_0x0c / 640.0f) * 64.0f + field_0x11c.field_0x00);
    field_0x11c.field_0x0c = (int)((owner->field_0x10 / 480.0f) * 85.0f + field_0x11c.field_0x04);
    field_0x11c.field_0x10 = (owner->field_0x0c / 640.0f) * 35.0f;
    field_0x11c.field_0x14 = 0.0f;
    field_0x11c.field_0x18 = 100.0f;
    field_0x11c.field_0x1c = 50.0f;
    field_0x11c.field_0x20 = 230.0f;
    field_0x11c.field_0x24 = 130.0f;
    UnknownFunction518c60(&field_0x11c, field_0x144);
    return this;
}

// 0x00518940: draws the dial and its needle.
int InstrumentOverlay::UnknownVirtualSlot14()
{
    if (g_UnknownGlobal56e26c->uiInteractionBlocked) {
        Target()->UnknownVirtualSlot7(0, 1, 1);
        Target()->UnknownVirtualSlot7(0, 4, 1);
        Target()->UnknownVirtualSlot8(0x1c, 0, 0);
        Target()->UnknownVirtualSlot8(0x29, 0, 0);
        Target()->UnknownVirtualSlot8(0x1b, 0, 0);
    } else {
        Overlay::UnknownVirtualSlot14();
        Target()->UnknownVirtualSlot7(0, 1, 1);
        Target()->UnknownVirtualSlot7(0, 4, 1);
        Target()->UnknownVirtualSlot8(0x29, 0, 0);
        Target()->UnknownVirtualSlot8(0x1b, 0, 0);
        RenderTarget* owner = Target();
        if (owner->field_0x08->field_0x1cc) {
            field_0x11c.field_0x00 = (int)((unsigned int)owner->field_0x08->field_0x1a0[2] * field_0xe8.left / 640.0f
                                           + (unsigned int)owner->field_0x08->field_0x1a0[0]);
            field_0x11c.field_0x04 = (int)((unsigned int)owner->field_0x08->field_0x1a0[3] * field_0xe8.top / 480.0f
                                           + (unsigned int)owner->field_0x08->field_0x1a0[1]);
            field_0x11c.field_0x08 = (int)(((unsigned int)owner->field_0x08->field_0x1a0[2] / 640.0f) * 64.0f + field_0x11c.field_0x00);
            field_0x11c.field_0x0c = (int)(((unsigned int)owner->field_0x08->field_0x1a0[3] / 480.0f) * 85.0f + field_0x11c.field_0x04);
            field_0x11c.field_0x10 = ((unsigned int)owner->field_0x08->field_0x1a0[2] / 640.0f) * 45.0f;
            field_0x11c.field_0x14 = 0.0f;
            field_0x11c.field_0x18 = 100.0f;
            field_0x11c.field_0x1c = 50.0f;
            field_0x11c.field_0x20 = 230.0f;
            field_0x11c.field_0x24 = 130.0f;
            UnknownFunction518c60(&field_0x11c, field_0x144);
        }
        UnknownInstrumentState* state = field_0x184->field_0x3b4;
        if (state->field_0x108) {
            s_UnknownGlobal68a448 = (!s_UnknownGlobal575140 ? state->field_0x0bc * 0.68182f : s_UnknownGlobal68a448) - 0.5f;
            if (s_UnknownGlobal68a448 < 0.0f)
                s_UnknownGlobal68a448 = 0.0f;
            field_0x11c.field_0x14 = s_UnknownGlobal68a448;
        } else {
            field_0x11c.field_0x14 = state->field_0x0bc * 0.68182f;
        }
        s_UnknownGlobal575140 = field_0x184->field_0x3b4->field_0x108;
        UnknownFunction518cc0(&field_0x11c, field_0x144);
        if (!Target()->UnknownVirtualSlot16(3, 0x1c4, (int)field_0x144, 2, 0))
            return 0;
    }
    return 1;
}

// 0x00518c00
InstrumentOverlay::~InstrumentOverlay()
{
    if (field_0x2c)
        field_0x2c->Release();
}

// 0x00518c60: the needle from its tip to the dial centre.
void InstrumentOverlay::UnknownFunction518c60(UnknownGauge* gauge, UnknownOverlayVertex* vertices)
{
    vertices[0].sz = 0.00001f;
    vertices[0].rhw = 1.0f;
    vertices[0].color = 0xff0000;
    vertices[0].specular = 0;
    vertices[0].tu = 0;
    vertices[0].tv = 0;
    vertices[1].sx = (float)gauge->field_0x08;
    vertices[1].sy = (float)gauge->field_0x0c;
    vertices[1].sz = 0.00001f;
    vertices[1].rhw = 1.0f;
    vertices[1].color = 0xff0000;
    vertices[1].specular = 0;
    vertices[1].tu = 0;
    vertices[1].tv = 0;
    UnknownFunction518cc0(gauge, vertices);
}

// 0x00518cc0: the needle tip for the clamped value.
void InstrumentOverlay::UnknownFunction518cc0(UnknownGauge* gauge, UnknownOverlayVertex* vertex)
{
    if (gauge->field_0x14 > gauge->field_0x18)
        gauge->field_0x14 = gauge->field_0x18;
    if (gauge->field_0x14 < 0.0f)
        gauge->field_0x14 = 0.0f;
    float angle;
    if (gauge->field_0x14 < gauge->field_0x1c)
        angle = (360.0f - gauge->field_0x20) * (gauge->field_0x14 / gauge->field_0x1c) + gauge->field_0x20;
    else
        angle = (gauge->field_0x14 - gauge->field_0x1c) / gauge->field_0x1c * gauge->field_0x24;
    angle *= 0.0174532905f;
    vertex->sx = sin(angle) * gauge->field_0x10 + gauge->field_0x08;
    vertex->sy = gauge->field_0x0c - (float)cos(angle) * gauge->field_0x10;
}

// 0x00518d50
UnknownTrackOverlayRect::UnknownTrackOverlayRect()
{
    left = 0;
    right = 0;
    top = 0;
    bottom = 0;
}

// 0x00518d60
UnknownTrackOverlayRect::UnknownTrackOverlayRect(int left, int right, int top, int bottom)
{
    this->left = left;
    this->right = right;
    this->top = top;
    this->bottom = bottom;
}

// 0x0068a410: a zero vector defined elsewhere.
extern Vector3 g_UnknownVector68a410;

// 0x00518d80
NameOverlay::NameOverlay(int flags) : Overlay(flags, 1)
{
    field_0x11c = 0;
    field_0x12c = 0;
    field_0x13c = 0;
    field_0x140 = 0;
    field_0x148 = 0;
    field_0x14c = 0;
    field_0x124 = 0;
    field_0x170 = 0;
    field_0x138 = 1;
    field_0x110 = 1;
    field_0x10c = 0xff;
    field_0x144 = 0.25f;
}

// 0x00518e20
NameOverlay::~NameOverlay()
{
}

// 0x00518e30
NameOverlay* NameOverlay::UnknownFunction518e30(RenderTarget* target, TextureMap* texture,
                                                const UnknownOverlayRect* rect, int a4,
                                                const UnknownOverlayRect* source, UnknownEventRacer* a6,
                                                UnknownNameOverlayWorld* a7, int a8)
{
    if (!UnknownFunction4b5f50(target, texture, rect, a4, source, 0.00001f, 0, 0, 0, 0, 0x613, 0)) {
        Release();
        return 0;
    }
    field_0x12c = a7;
    field_0x11c = a6;
    field_0x120 = source->right - source->left + 1;
    field_0x128 = source->bottom - source->top + 1;
    field_0x130 = a8 % 4;
    field_0x134 = 0;
    field_0x168 = 0;
    field_0x16c = 1;
    field_0x150 = g_UnknownVector68a410;
    field_0x15c = g_UnknownVector68a410;
    return this;
}

// 0x00519080
void NameOverlay::UnknownFunction519080(Vector3 point)
{
    field_0x150 = point;
}

// 0x005190a0
void NameOverlay::UnknownFunction5190a0(Vector3 point, int a4, int a5)
{
    field_0x15c = point;
    field_0x168 = a4;
    field_0x16c = a5;
}

// 0x005190e0: draws the tag at a fixed point (no racer), at the racer's
// screen-space anchor (+0x15c), and above the racer's head.
int NameOverlay::UnknownVirtualSlot14()
{
    if (!field_0x11c) {
        int x = (int)field_0x150.x;
        int y = (int)field_0x150.y;
        field_0xe8 = UnknownMakeOverlayRect(x, y, field_0x124 + x - 1, field_0x128 + y);
        Target()->UnknownVirtualSlot7(0, 0x10, 1);
        Target()->UnknownVirtualSlot7(0, 0x11, 1);
        Overlay::UnknownVirtualSlot14();
        return 1;
    }
    if (field_0x11c->field_0x4a0)
        return 1;
    if (!field_0x11c->field_0x25_bit0)
        return 1;
    if ((field_0x15c.x != -1.0f || field_0x15c.y != -1.0f) && g_UnknownGlobal56e26c->mode.field_0x6b8) {
        int left;
        int right;
        if (field_0x168) {
            right = (int)field_0x15c.x;
            left = right - field_0x124;
        } else {
            left = (int)field_0x15c.x;
            right = field_0x124 + left;
        }
        int top = (int)field_0x15c.y;
        field_0xe8 = UnknownMakeOverlayRect(left, top, right, field_0x128 + top);
        Target()->UnknownVirtualSlot7(0, 0x10, 1);
        Target()->UnknownVirtualSlot7(0, 0x11, 1);
        int alpha = field_0x10c;
        field_0x10c = field_0x16c ? 0xb4 : 0xff;
        Overlay::UnknownVirtualSlot14();
        field_0x10c = alpha;
    }
    Vector3 head;
    field_0x11c->field_0x5c4->field_0x1a0->UnknownFunction4fdae0("Head")->UnknownFunction4fc9a0(0, &head);
    head.y += 1.62f;
    int hidden = !UnknownFunction519000(&head);
    if (hidden) {
        if (!field_0x14c) {
            field_0x13c = 1;
            field_0x140 = 0;
        }
    } else if (field_0x14c) {
        field_0x140 = 1;
        field_0x13c = 0;
    }
    field_0x14c = hidden;
    if (field_0x10c) {
        Camera* camera = Target()->field_0x08;
        Vector3 screen;
        float depth;
        if (g_UnknownGlobal575a98->UnknownFunction52f340(camera, camera->field_0xec, &head, &screen, &depth)) {
            int x = (int)screen.x;
            int y = (int)screen.y;
            field_0xe8 = UnknownMakeOverlayRect(x, y, field_0x124 + x, field_0x128 + y);
            Target()->UnknownVirtualSlot7(0, 0x10, 1);
            Target()->UnknownVirtualSlot7(0, 0x11, 1);
            Overlay::UnknownVirtualSlot14();
        }
    }
    return 1;
}

// 0x00519370
StatsOverlay::StatsOverlay(int flags) : Overlay(flags, 1)
{
    field_0x11c = 0;
    field_0x120 = 0;
    field_0x128 = 0;
    field_0x124 = 0;
    field_0x12c = 0;
    field_0x2c = 0;
    field_0x30 = 0;
    memset(field_0x5b0, 0, sizeof(field_0x5b0));
    memset(field_0x1b0, 0, sizeof(field_0x1b0));
    field_0x9b4 = 0;
    field_0x9b0 = 0;
}

// 0x00519420
StatsOverlay::~StatsOverlay()
{
    if (field_0x11c)
        DeleteObject(field_0x11c);
    if (field_0x120)
        DeleteObject(field_0x120);
    if (field_0x2c)
        field_0x2c->Release();
    if (field_0x30)
        field_0x30->Release();
}

// 0x00519880
int StatsOverlay::UnknownFunction519880(int value)
{
    field_0x124 = value;
    return 1;
}

// 0x00519900
int StatsOverlay::UnknownVirtualSlot10(float frameTime)
{
    Overlay::UnknownVirtualSlot10(g_UnknownGlobal56e26c->field_0x2f0);
    field_0x12c += g_UnknownGlobal56e26c->field_0x2f0;
    return 1;
}

// 0x00519940
int StatsOverlay::UnknownVirtualSlot13()
{
    if (field_0x12c >= 1.0f) {
        if (g_UnknownGlobal56e26c->mode.field_0x6b0)
            UnknownFunction5198a0();
        field_0x12c = 0;
    }
    return 1;
}

// 0x00519980
int StatsOverlay::UnknownVirtualSlot14()
{
    if (g_UnknownGlobal56e26c->mode.field_0x6b0) {
        Target()->UnknownVirtualSlot7(0, 0x10, 1);
        Target()->UnknownVirtualSlot7(0, 0x11, 1);
        Overlay::UnknownVirtualSlot14();
        switch (g_UnknownGlobal56e26c->field_0x2d74) {
        case 0:
            UnknownFunction519e10();
            break;
        case 2:
        case 3:
            UnknownFunction51a480();
            break;
        }
    }
    return 1;
}

// 0x005199f0
int UnknownFunction5199f0(const void* a, const void* b)
{
    const float* left = (const float*)a;
    const float* right = (const float*)b;
    if (*left > *right)
        return -1;
    if (*left == *right)
        return 0;
    return 1;
}

// 0x0051ae80
DropTextOverlay::DropTextOverlay(int flags) : GameObject(flags)
{
    field_0x2c = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x44 = 0;
    field_0x30 = 1.0f;
}

// 0x0051aee0
DropTextOverlay::~DropTextOverlay()
{
    if (field_0x2c)
        DeleteObject(field_0x2c);
}

// 0x0051b0f0: draws the text twice through the surface's device context,
// black one pixel down and right, then green on top.
int DropTextOverlay::UnknownVirtualSlot15()
{
    if (field_0x34) {
        GameObject::UnknownVirtualSlot15();
        void* dc;
        if (((PCRenderTarget*)field_0x18)->field_0x48->UnknownMethod17(&dc) == 0) {
            SetBkColor(dc, 1);
            SetBkMode(dc, 1);
            void* font = SelectObject(dc, field_0x2c);
            SetTextColor(dc, 0);
            TextOutA(dc, field_0x3c + 1, field_0x44 + 1, field_0x48, strlen(field_0x48));
            SetTextColor(dc, 0xff00);
            TextOutA(dc, field_0x3c, field_0x44, field_0x48, strlen(field_0x48));
            SelectObject(dc, font);
            if (((PCRenderTarget*)field_0x18)->field_0x48->UnknownMethod26(dc) != 0)
                return 0;
        } else {
            return 0;
        }
    }
    return 1;
}

// 0x0051b1f0
void DropTextOverlay::UnknownFunction51b1f0()
{
    field_0x30 = 0.0f;
    field_0x34 = 1;
}

// 0x0051b200
UnknownMessage::UnknownMessage(const char* text, float duration)
{
    strcpy(field_0x00, text);
    field_0x84 = 0.0f;
    field_0x88 = 0;
    field_0x80 = duration;
}

// 0x0051b250
UnknownMessage::UnknownMessage(const UnknownMessage& other)
{
    strcpy(field_0x00, other.field_0x00);
    field_0x80 = other.field_0x80;
    field_0x84 = 0.0f;
    field_0x88 = 0;
}

// 0x0051b2a0
TextQueueOverlay::TextQueueOverlay(int flags) : GameObject(flags)
{
    field_0x2c = 0;
    field_0x30.left = 0;
    field_0x30.top = 0;
    field_0x30.right = 0;
    field_0x30.bottom = 0;
    field_0x50 = 0;
    field_0x54 = 0xff00;
}

// 0x0051b300
TextQueueOverlay::~TextQueueOverlay()
{
    if (field_0x2c)
        DeleteObject(field_0x2c);
}

// 0x0051b320
TextQueueOverlay* TextQueueOverlay::UnknownFunction51b320(void* target, UnknownOverlayRect rect)
{
    GameObject::UnknownVirtualSlot8(target);
    UnknownKrustyUIGui* gui = g_UnknownGlobal56e26c->ui->field_0x2c;
    const char* face = gui ? gui->field_0x350 : "";
    int weight = 700;
    int italic = 1;
    if (*face == '\0') {
        face = "Arial";
    } else {
        weight = gui->field_0x3d4 ? 700 : 500;
        italic = gui->field_0x3d8;
    }
    field_0x2c = CreateFontA(17, 0, 0, 0, weight, italic, 0, 0, 1, 0, 0, 2, 2, face);
    field_0x40 = rect;
    field_0x30.left = field_0x40.left - 1;
    field_0x30.right = field_0x40.right - 1;
    field_0x30.top = field_0x40.top - 1;
    field_0x30.bottom = field_0x40.bottom - 1;
    return this;
}

// 0x0051b3f0: ages the head line and drops it once its time is up.
int TextQueueOverlay::UnknownVirtualSlot10(float frameTime)
{
    GameObject::UnknownVirtualSlot10(g_UnknownGlobal56e26c->field_0x2f0);
    if (field_0x50) {
        if (field_0x50->field_0x84 > field_0x50->field_0x80)
            UnknownFunction51b670();
        if (field_0x50)
            field_0x50->field_0x84 += g_UnknownGlobal56e26c->field_0x2f0;
    }
    return 1;
}

// 0x0051b450: draws the head line, black in the shadow rectangle, then in
// the text colour one pixel up and left.
int TextQueueOverlay::UnknownVirtualSlot15()
{
    GameObject::UnknownVirtualSlot15();
    if (field_0x50) {
        void* dc;
        if (((PCRenderTarget*)field_0x18)->field_0x48->UnknownMethod17(&dc) == 0) {
            SetBkColor(dc, 1);
            SetBkMode(dc, 1);
            void* font = SelectObject(dc, field_0x2c);
            SetTextColor(dc, 0);
            DrawTextA(dc, field_0x50->field_0x00, strlen(field_0x50->field_0x00), &field_0x40, 0x124);
            SetTextColor(dc, field_0x54);
            DrawTextA(dc, field_0x50->field_0x00, strlen(field_0x50->field_0x00), &field_0x30, 0x124);
            SelectObject(dc, font);
            if (((PCRenderTarget*)field_0x18)->field_0x48->UnknownMethod26(dc) != 0)
                return 0;
        } else {
            return 0;
        }
    }
    return 1;
}

// 0x0051b540: replaces the head line with a copy of `message`.
void TextQueueOverlay::UnknownFunction51b540(UnknownMessage* message)
{
    if (!field_0x50) {
        field_0x50 = new(__FILE__, 1451) UnknownMessage(*message);
        return;
    }
    UnknownMessage* copy = new(__FILE__, 1453) UnknownMessage(*message);
    copy->field_0x88 = field_0x50->field_0x88;
    delete field_0x50;
    field_0x50 = copy;
}

// 0x0051b5e0: appends a copy of `message`.
void TextQueueOverlay::UnknownFunction51b5e0(UnknownMessage* message)
{
    if (!field_0x50) {
        field_0x50 = new(__FILE__, 1469) UnknownMessage(*message);
        return;
    }
    UnknownMessage* last;
    UnknownMessage* entry = field_0x50;
    while (entry) {
        last = entry;
        entry = entry->field_0x88;
    }
    last->field_0x88 = new(__FILE__, 1476) UnknownMessage(*message);
}

// 0x0051b670
void TextQueueOverlay::UnknownFunction51b670()
{
    UnknownMessage* head = field_0x50;
    if (head) {
        field_0x50 = head->field_0x88;
        delete head;
    }
}
