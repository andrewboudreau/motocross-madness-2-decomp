#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TrackOverlay.h"
#include "DebugAlloc.h"
#include "BikeCamera.h"
#include "Camera.h"
#include "ControlInterface.h"
#include "FollowCamera.h"
#include "KeyboardDevice.h"
#include "KrustyUI.h"
#include "PCRenderTarget.h"
#include "PCTextureMap.h"
#include "TrackGame.h"
#include "Track.h"
#include "D3DConstants.h"

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
// A GDI LOGFONTA (0x3c bytes).
struct UnknownLogFont {
    long lfHeight;
    long lfWidth;
    long lfEscapement;
    long lfOrientation;
    long lfWeight;
    unsigned char lfItalic;
    unsigned char lfUnderline;
    unsigned char lfStrikeOut;
    unsigned char lfCharSet;
    unsigned char lfOutPrecision;
    unsigned char lfClipPrecision;
    unsigned char lfQuality;
    unsigned char lfPitchAndFamily;
    char lfFaceName[32];
};
extern "C" __declspec(dllimport) void* __stdcall CreateFontIndirectA(const UnknownLogFont* font);
extern "C" __declspec(dllimport) void* __stdcall SelectObject(void* dc, void* object);
extern "C" __declspec(dllimport) unsigned long __stdcall SetBkColor(void* dc, unsigned long color);
extern "C" __declspec(dllimport) int __stdcall SetBkMode(void* dc, int mode);
extern "C" __declspec(dllimport) unsigned long __stdcall SetTextColor(void* dc, unsigned long color);
extern "C" __declspec(dllimport) int __stdcall TextOutA(void* dc, int x, int y, const char* text, int length);
extern "C" __declspec(dllimport) int __stdcall GetTextExtentPoint32A(void* dc, const char* text, int length,
                                                                     UnknownTextExtent* size);
extern "C" __declspec(dllimport) int __stdcall DrawTextA(void* dc, const char* text, int length,
                                                         UnknownOverlayRect* rect, unsigned int format);

// 0x0057513c: the radar zoom level, 1..5.
static int s_UnknownGlobal57513c = 2;

// 0x00575140, 0x0068a448: the previous state flag and the decaying value.
static int s_UnknownGlobal575140 = 1;
static float s_UnknownGlobal68a448;

// 0x0068a440: never set; when set, StatsOverlay shows the end timer.
static int s_UnknownGlobal68a440;

// 0x0068a444: when set, ChatOverlay redraws all of its lines.
static int s_UnknownGlobal68a444;

// 0x00518720
InstrumentOverlay::InstrumentOverlay(int flags) : Overlay(flags, 1)
{
    alphaBlended = 1;
    instrumentSource = 0;
}

// 0x00518770: loads the dial texture and places the needle for the
// render target's size (the layout is for 640x480).
InstrumentOverlay* InstrumentOverlay::UnknownFunction518770(RenderTarget* target, TextureMapManager* manager, UnknownInstrumentSource* a3)
{
    UnknownOverlayRect rect;
    char name[260];
    instrumentSource = a3;
    strcpy(name, "instr1.tga");
    rect.left = 511;
    rect.right = 639;
    rect.top = 351;
    rect.bottom = 479;
    TextureMap* texture = UnknownFunction50a590(manager, name, 4444, 0, 0, 5, 6, 0, 0x80,
                                                0xff00ff, 1, 1);
    if (!texture) {
        Release();
        return 0;
    }
    texture->UnknownVirtualSlot8(1, 0, 0);
    Attach(target, texture, &rect, 1, 0, 0.00001f, 0, 0, 0, 0, 1555, 0);
    RenderTarget* owner = Target();
    dial.field_0x00 = (int)(rect.left * owner->field_0x0c / 640.0f);
    dial.field_0x04 = (int)(rect.top * owner->field_0x10 / 480.0f);
    dial.centerX = (int)((owner->field_0x0c / 640.0f) * 64.0f + dial.field_0x00);
    dial.centerY = (int)((owner->field_0x10 / 480.0f) * 85.0f + dial.field_0x04);
    dial.length = (owner->field_0x0c / 640.0f) * 35.0f;
    dial.value = 0.0f;
    dial.maximum = 100.0f;
    dial.firstSweepValue = 50.0f;
    dial.startAngle = 230.0f;
    dial.degreesPerUnit = 130.0f;
    PlaceNeedle(&dial, dialNeedle);
    return this;
}

// 0x00518940: draws the dial and its needle.
int InstrumentOverlay::UnknownVirtualSlot14()
{
    if (g_TrackGame->uiInteractionBlocked) {
        Target()->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        Target()->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        Target()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGENABLE, 0, 0);
        Target()->UnknownVirtualSlot8(D3DRENDERSTATE_COLORKEYENABLE, 0, 0);
        Target()->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
    } else {
        Overlay::UnknownVirtualSlot14();
        Target()->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        Target()->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        Target()->UnknownVirtualSlot8(D3DRENDERSTATE_COLORKEYENABLE, 0, 0);
        Target()->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
        RenderTarget* owner = Target();
        if (owner->field_0x08->field_0x1cc) {
            dial.field_0x00 = (int)((unsigned int)owner->field_0x08->field_0x1a0[2] * screenRect.left / 640.0f
                                           + (unsigned int)owner->field_0x08->field_0x1a0[0]);
            dial.field_0x04 = (int)((unsigned int)owner->field_0x08->field_0x1a0[3] * screenRect.top / 480.0f
                                           + (unsigned int)owner->field_0x08->field_0x1a0[1]);
            dial.centerX = (int)(((unsigned int)owner->field_0x08->field_0x1a0[2] / 640.0f) * 64.0f + dial.field_0x00);
            dial.centerY = (int)(((unsigned int)owner->field_0x08->field_0x1a0[3] / 480.0f) * 85.0f + dial.field_0x04);
            dial.length = ((unsigned int)owner->field_0x08->field_0x1a0[2] / 640.0f) * 45.0f;
            dial.value = 0.0f;
            dial.maximum = 100.0f;
            dial.firstSweepValue = 50.0f;
            dial.startAngle = 230.0f;
            dial.degreesPerUnit = 130.0f;
            PlaceNeedle(&dial, dialNeedle);
        }
        UnknownInstrumentState* state = instrumentSource->state;
        if (state->field_0x108) {
            s_UnknownGlobal68a448 = (!s_UnknownGlobal575140 ? state->gaugeValue * 0.68182f : s_UnknownGlobal68a448) - 0.5f;
            if (s_UnknownGlobal68a448 < 0.0f)
                s_UnknownGlobal68a448 = 0.0f;
            dial.value = s_UnknownGlobal68a448;
        } else {
            dial.value = state->gaugeValue * 0.68182f;
        }
        s_UnknownGlobal575140 = instrumentSource->state->field_0x108;
        PlaceNeedleTip(&dial, dialNeedle);
        if (!Target()->UnknownVirtualSlot16(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)dialNeedle, 2, 0))
            return 0;
    }
    return 1;
}

// 0x00518c00
InstrumentOverlay::~InstrumentOverlay()
{
    if (overlayTexture)
        overlayTexture->Release();
}

// 0x00518c60: the needle from its tip to the dial centre.
void InstrumentOverlay::PlaceNeedle(UnknownGauge* gauge, UnknownOverlayVertex* vertices)
{
    vertices[0].sz = 0.00001f;
    vertices[0].rhw = 1.0f;
    vertices[0].color = 0xff0000;
    vertices[0].specular = 0;
    vertices[0].tu = 0;
    vertices[0].tv = 0;
    vertices[1].sx = (float)gauge->centerX;
    vertices[1].sy = (float)gauge->centerY;
    vertices[1].sz = 0.00001f;
    vertices[1].rhw = 1.0f;
    vertices[1].color = 0xff0000;
    vertices[1].specular = 0;
    vertices[1].tu = 0;
    vertices[1].tv = 0;
    PlaceNeedleTip(gauge, vertices);
}

// 0x00518cc0: the needle tip for the clamped value.
void InstrumentOverlay::PlaceNeedleTip(UnknownGauge* gauge, UnknownOverlayVertex* vertex)
{
    if (gauge->value > gauge->maximum)
        gauge->value = gauge->maximum;
    if (gauge->value < 0.0f)
        gauge->value = 0.0f;
    float angle;
    if (gauge->value < gauge->firstSweepValue)
        angle = (360.0f - gauge->startAngle) * (gauge->value / gauge->firstSweepValue) + gauge->startAngle;
    else
        angle = (gauge->value - gauge->firstSweepValue) / gauge->firstSweepValue * gauge->degreesPerUnit;
    angle *= 0.0174532905f;
    vertex->sx = sin(angle) * gauge->length + gauge->centerX;
    vertex->sy = gauge->centerY - (float)cos(angle) * gauge->length;
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

// The four per-TU vector constants (see src/krusty2/math/Math3D.h). Their
// dynamic initializers sit at 0x0051dba0..0x0051dcdc, mid-way through this
// TU's code, and only this TU's code reads the zero vector.
static const Vector3 s_UnknownVector68a410 = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector68a420 = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector68a430 = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 s_UnknownVector68a400 = Vector3(0.0f, 0.0f, 1.0f);

// 0x00518d80
NameOverlay::NameOverlay(int flags) : Overlay(flags, 1)
{
    trackedRacer = 0;
    world = 0;
    field_0x13c = 0;
    field_0x140 = 0;
    field_0x148 = 0;
    field_0x14c = 0;
    field_0x124 = 0;
    field_0x170 = 0;
    lastVisible = 1;
    alphaBlended = 1;
    overlayAlpha = 0xff;
    fadeTime = 0.25f;
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
    if (!Attach(target, texture, rect, a4, source, 0.00001f, 0, 0, 0, 0, 1555, 0)) {
        Release();
        return 0;
    }
    world = a7;
    trackedRacer = a6;
    sourceWidth = source->right - source->left + 1;
    sourceHeight = source->bottom - source->top + 1;
    testPhase = a8 % 4;
    frameCounter = 0;
    field_0x168 = 0;
    field_0x16c = 1;
    field_0x150 = s_UnknownVector68a410;
    field_0x15c = s_UnknownVector68a410;
    return this;
}

// 0x00519000: every fourth frame, tests whether `point` can be seen from the
// camera (no terrain in between).
int NameOverlay::IsPointVisible(const Vector3* point)
{
    Vector3 hit;
    if (frameCounter == testPhase) {
        int hidden = world->terrain->UnknownFunction506e90(&((RenderTarget*)field_0x18)->field_0x08->field_0x170, point, &hit, 0, 0, 0);
        lastVisible = !hidden;
    }
    frameCounter++;
    if (frameCounter > 3)
        frameCounter = 0;
    return lastVisible;
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
    if (!trackedRacer) {
        int x = (int)field_0x150.x;
        int y = (int)field_0x150.y;
        screenRect = UnknownMakeOverlayRect(x, y, field_0x124 + x - 1, sourceHeight + y);
        Target()->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        Target()->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        Overlay::UnknownVirtualSlot14();
        return 1;
    }
    if (trackedRacer->field_0x4a0)
        return 1;
    if (!trackedRacer->field_0x25_bit0)
        return 1;
    if ((field_0x15c.x != -1.0f || field_0x15c.y != -1.0f) && g_TrackGame->mode.field_0x6b8) {
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
        screenRect = UnknownMakeOverlayRect(left, top, right, sourceHeight + top);
        Target()->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        Target()->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        int alpha = overlayAlpha;
        overlayAlpha = field_0x16c ? 0xb4 : 0xff;
        Overlay::UnknownVirtualSlot14();
        overlayAlpha = alpha;
    }
    Vector3 head;
    trackedRacer->field_0x5c4->field_0x1a0->UnknownFunction4fdae0("Head")->UnknownFunction4fc9a0(0, &head);
    head.y += 1.62f;
    int hidden = !IsPointVisible(&head);
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
    if (overlayAlpha) {
        Camera* camera = Target()->field_0x08;
        Vector3 screen;
        float depth;
        if (g_UnknownGlobal575a98->UnknownFunction52f340(camera, camera->field_0xec, &head, &screen, &depth)) {
            int x = (int)screen.x;
            int y = (int)screen.y;
            screenRect = UnknownMakeOverlayRect(x, y, field_0x124 + x, sourceHeight + y);
            Target()->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
            Target()->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_POINT);
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
    instrumentSource = 0;
    statsSource = 0;
    timeSinceRedraw = 0;
    overlayTexture = 0;
    backingTexture = 0;
    memset(rowText, 0, sizeof(rowText));
    memset(field_0x1b0, 0, sizeof(field_0x1b0));
    rowExtent.cy = 0;
    rowExtent.cx = 0;
}

// 0x00519420
StatsOverlay::~StatsOverlay()
{
    if (field_0x11c)
        DeleteObject(field_0x11c);
    if (field_0x120)
        DeleteObject(field_0x120);
    if (overlayTexture)
        overlayTexture->Release();
    if (backingTexture)
        backingTexture->Release();
}

// 0x00519880
int StatsOverlay::UnknownFunction519880(int value)
{
    statsSource = (UnknownStatsSource*)value;
    return 1;
}

// 0x005198a0: redraws the panel for the view mode; 0 when that failed.
int StatsOverlay::Redraw()
{
    switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
    case 0:
        if (!UnknownFunction519a20())
            goto fail;
        break;
    case 1:
    case 5:
        if (!DrawRacePanel())
            goto fail;
        break;
    case 2:
    case 3:
        if (!UnknownFunction519ef0())
            goto fail;
        break;
    case 4:
        if (!DrawTagStandings())
            goto fail;
        break;
    }
    return 1;
fail:
    return 0;
}

// 0x00519900
int StatsOverlay::UnknownVirtualSlot10(float frameTime)
{
    Overlay::UnknownVirtualSlot10(g_TrackGame->frameTime);
    timeSinceRedraw += g_TrackGame->frameTime;
    return 1;
}

// 0x00519940
int StatsOverlay::UnknownVirtualSlot13()
{
    if (timeSinceRedraw >= 1.0f) {
        if (g_TrackGame->mode.field_0x6b0)
            Redraw();
        timeSinceRedraw = 0;
    }
    return 1;
}

// 0x00519980
int StatsOverlay::UnknownVirtualSlot14()
{
    if (g_TrackGame->mode.field_0x6b0) {
        Target()->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        Target()->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        Overlay::UnknownVirtualSlot14();
        switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
        case 0:
            DrawTimeLeft();
            break;
        case 2:
        case 3:
            DrawLapTime();
            break;
        }
    }
    return 1;
}

// 0x00519e10: the time left, in the first text row.
void StatsOverlay::DrawTimeLeft()
{
    char text[0x80];
    UnknownOverlayRect rect;
    int count;
    TrackGame* game = g_TrackGame;
    if (game->mode.field_0x27f8.field_0x00 != 0 && game->mode.field_0x27f8.field_0x00 != 4) {
        TrackGameViewOwner* owner = game->field_0x55c;
        float limit = game->mode.field_0x27f8.field_0x140 * 60.0f;
        UnknownFunction518640(text, limit - (owner->field_0x70 * 60.0f + owner->field_0x74));
        rect = rowRects[0];
        rect.left += rowExtent.cx + 4;
        sprintf(rowText[0], "%s", text);
        field_0x38->SelectFont("arialsm");
        void* laidOut = field_0x38->UnknownFunction50ba00(&rect, rowText[0], 0xffffff, &count);
        field_0x38->DrawVertices(Target(), laidOut, count);
    }
}

// 0x00519ef0: the race panel of the other view modes; in mode 4 the lap
// times come from the view's own racer.
int StatsOverlay::UnknownFunction519ef0()
{
    void* dc;
    int i;
    int iterator;
    void* font;
    char label[0x80];
    char time[0x80];
    g_TrackGame->LoadResourceString(0x835, label, 0x80);
    int finished = 0;
    iterator = 0;
    UnknownEventRacer* racer;
    while ((racer = statsSource->UnknownFunction4204e0(&iterator)) != 0) {
        if (racer->field_0x4a0)
            finished++;
    }
    int total;
    if (g_TrackGame->field_0x18 == 1) {
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4)
            total = 1;
        else
            total = g_TrackGame->mode.field_0x27f8.field_0x24 + 1;
    } else {
        total = g_TrackGame->mode.field_0x27f8.field_0x28 + g_TrackGame->field_0x18;
    }
    sprintf(rowText[0], "%s %d / %d", label, instrumentSource->state->position, total - finished);
    g_TrackGame->LoadResourceString(0x836, label, 0x80);
    int lap = instrumentSource->state->lapsDone + 1;
    if (g_TrackGame->mode.field_0x27f8.field_0x00 != 0 && g_TrackGame->mode.field_0x27f8.field_0x00 != 4) {
        if (lap > g_TrackGame->mode.field_0x27f8.field_0x20)
            lap = g_TrackGame->mode.field_0x27f8.field_0x20;
    }
    if (g_TrackGame->mode.field_0x27f8.field_0x00 != 0 && g_TrackGame->mode.field_0x27f8.field_0x00 != 4)
        sprintf(rowText[1], "%s %d / %d", label, lap, g_TrackGame->mode.field_0x27f8.field_0x20);
    else
        sprintf(rowText[1], "%s %d", label, lap);
    if (s_UnknownGlobal68a440) {
        UnknownFunction518690(time, g_TrackGame->eventManager->field_0x3c0);
        sprintf(rowText[2], "%s %s %d", "End Timer", time, instrumentSource->state->field_0x7a4);
    } else {
        g_TrackGame->LoadResourceString(0x837, label, 0x80);
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4 && instrumentSource->state->field_0x736)
            UnknownFunction518690(time, statsSource->ownRacer->field_0x750);
        else
            UnknownFunction518690(time, instrumentSource->state->lastLapTime);
        sprintf(rowText[2], "%s %s", label, time);
    }
    g_TrackGame->LoadResourceString(0x838, label, 0x80);
    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4)
        UnknownFunction518690(time, statsSource->ownRacer->field_0x750);
    else
        UnknownFunction518690(time, instrumentSource->state->field_0x750);
    sprintf(rowText[3], "%s %s", label, time);
    g_TrackGame->LoadResourceString(0x839, label, 0x80);
    UnknownFunction518690(time, instrumentSource->state->field_0x770);
    sprintf(rowText[4], "%s %s", label, time);
    g_TrackGame->LoadResourceString(0x13b8, label, 0x80);
    if (g_TrackGame->field_0x3428)
        UnknownFunction518690(time, statsSource->field_0x1b8);
    else
        UnknownFunction518690(time, instrumentSource->state->field_0x754);
    sprintf(rowText[5], "%s:", label);
    for (i = 0; i <= 5; i++) {
        if (strcmp(rowText[i], field_0x1b0[i]) != 0)
            goto draw;
    }
    return 1;
draw:
    RestoreRect(&sourceRect);
    PCTextureMap* texture = (PCTextureMap*)sharedTexture;
    if (texture->systemSurface->GetDC(&dc) != 0)
        goto fail;
    SetBkColor(dc, 1);
    SetBkMode(dc, 1);
    SetTextColor(dc, 0xffffff);
    font = SelectObject(dc, field_0x11c);
    for (i = 0; i <= 5; i++) {
        if (i == 5)
            GetTextExtentPoint32A(dc, rowText[5], strlen(rowText[5]), &rowExtent);
        if (i >= 2)
            SelectObject(dc, field_0x120);
        DrawTextA(dc, rowText[i], strlen(rowText[i]), &rowRects[i], 0x120);
        strcpy(field_0x1b0[i], rowText[i]);
    }
    SelectObject(dc, font);
    if (texture->systemSurface->ReleaseDC(dc) != 0) {
fail:
        return 0;
    }
    TintRows(0, 0x80, 0xffff);
    overlayTexture->UnknownVirtualSlot9(0, -1);
    return 1;
}

// 0x0051a480: the lap time, in the sixth text row.
void StatsOverlay::DrawLapTime()
{
    char text[0x80];
    UnknownOverlayRect rect;
    int count;
    if (g_TrackGame->field_0x3428)
        UnknownFunction518690(text, statsSource->field_0x1b8);
    else
        UnknownFunction518690(text, instrumentSource->state->field_0x754);
    rect = rowRects[5];
    rect.left += rowExtent.cx + 4;
    sprintf(rowText[5], "%s", text);
    field_0x38->SelectFont("arialsm");
    void* laidOut = field_0x38->UnknownFunction50ba00(&rect, rowText[5], 0xffffff, &count);
    field_0x38->DrawVertices(Target(), laidOut, count);
}

// 0x0051a560: the race panel: position, lap, lap times and gate.
int StatsOverlay::DrawRacePanel()
{
    void* dc;
    int i;
    int iterator;
    void* font;
    char label[0x80];
    char time[0x80];
    int finished = 0;
    iterator = 0;
    UnknownEventRacer* racer;
    while ((racer = statsSource->UnknownFunction4204e0(&iterator)) != 0) {
        if (racer->field_0x4a0)
            finished++;
    }
    g_TrackGame->LoadResourceString(0x835, label, 0x80);
    int total;
    if (g_TrackGame->field_0x18 == 1) {
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4)
            total = 1;
        else
            total = g_TrackGame->mode.field_0x27f8.field_0x24 + 1;
    } else {
        total = g_TrackGame->mode.field_0x27f8.field_0x28 + g_TrackGame->field_0x18;
    }
    sprintf(rowText[0], "%s %d / %d", label, instrumentSource->state->position, total - finished);
    g_TrackGame->LoadResourceString(0x836, label, 0x80);
    int lap = instrumentSource->state->lapsDone + 1;
    if (g_TrackGame->mode.field_0x27f8.field_0x00 != 0 && g_TrackGame->mode.field_0x27f8.field_0x00 != 4) {
        if (lap > g_TrackGame->mode.field_0x27f8.field_0x20)
            lap = g_TrackGame->mode.field_0x27f8.field_0x20;
    }
    if (g_TrackGame->mode.field_0x27f8.field_0x00 != 0 && g_TrackGame->mode.field_0x27f8.field_0x00 != 4)
        sprintf(rowText[1], "%s %d / %d", label, lap, g_TrackGame->mode.field_0x27f8.field_0x20);
    else
        sprintf(rowText[1], "%s %d", label, lap);
    if (s_UnknownGlobal68a440) {
        UnknownFunction518690(time, g_TrackGame->eventManager->field_0x3c0);
        sprintf(rowText[2], "%s %s %d", "End Timer", time, instrumentSource->state->field_0x7a4);
    } else {
        g_TrackGame->LoadResourceString(0x837, label, 0x80);
        UnknownFunction518690(time, instrumentSource->state->lastLapTime);
        sprintf(rowText[2], "%s %s", label, time);
    }
    g_TrackGame->LoadResourceString(0x838, label, 0x80);
    UnknownFunction518690(time, instrumentSource->state->field_0x750);
    sprintf(rowText[3], "%s %s", label, time);
    g_TrackGame->LoadResourceString(0x839, label, 0x80);
    UnknownFunction518690(time, instrumentSource->state->field_0x770);
    sprintf(rowText[4], "%s %s", label, time);
    g_TrackGame->LoadResourceString(0x13ad, label, 0x80);
    sprintf(rowText[5], "%s %d / %d", label, instrumentSource->state->lapGate + 1,
            g_TrackGame->field_0x560->field_0xac);
    for (i = 0; i <= 5; i++) {
        if (strcmp(rowText[i], field_0x1b0[i]) != 0)
            goto draw;
    }
    return 1;
draw:
    RestoreRect(&sourceRect);
    PCTextureMap* texture = (PCTextureMap*)sharedTexture;
    if (texture->systemSurface->GetDC(&dc) != 0)
        goto fail;
    SetBkColor(dc, 1);
    SetBkMode(dc, 1);
    SetTextColor(dc, 0xffffff);
    font = SelectObject(dc, field_0x11c);
    for (i = 0; i <= 5; i++) {
        if (i >= 2)
            SelectObject(dc, field_0x120);
        DrawTextA(dc, rowText[i], strlen(rowText[i]), &rowRects[i], 0x120);
        strcpy(field_0x1b0[i], rowText[i]);
    }
    SelectObject(dc, font);
    if (texture->systemSurface->ReleaseDC(dc) != 0) {
fail:
        return 0;
    }
    TintRows(0, 0x80, 0xffff);
    overlayTexture->UnknownVirtualSlot9(0, -1);
    return 1;
}

// 0x0051aa40: the tag standings (view mode 4): the time left, the racers
// by score with the player's own row last, and in row 5 who holds the tag.
int StatsOverlay::DrawTagStandings()
{
    UnknownEventScore scores[11];
    char tag[0x40];
    char time[0x80];
    char label[0x80];
    void* dc;
    void* font;
    int i;
    int count = 0;
    if (g_TrackGame->mode.field_0x27f8.field_0x00 != 0) {
        g_TrackGame->LoadResourceString(0x943, label, 0x80);
        TrackGameViewOwner* owner = g_TrackGame->field_0x568;
        float limit = g_TrackGame->mode.field_0x27f8.field_0x140 * 60.0f;
        UnknownFunction518640(time, limit - (owner->field_0x70 * 60.0f + owner->field_0x74));
        sprintf(rowText[0], "%s %s", label, time);
    }
    if (g_TrackGame->mode.field_0x27f8.field_0x148) {
        int iterator;
        int own;
        UnknownKrustyBikeView* view = g_TrackGame->field_0x568->field_0x34;
        iterator = 0;
        UnknownEventRacer* racer;
        while ((racer = view->UnknownFunction4204e0(&iterator)) != 0) {
            scores[count].value = racer->field_0x768;
            scores[count].racer = racer;
            count++;
        }
        qsort(scores, count, sizeof(UnknownEventScore), UnknownFunction5199f0);
        for (i = 0; i < count; i++) {
            if (scores[i].racer == view->field_0x38) {
                own = i;
                break;
            }
        }
        int first = 1;
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 0)
            first = 0;
        else
            count++;
        if (count >= 5)
            count = 5;
        int row;
        int shown = 0;
        for (row = first; row < count - 1; row++) {
            sprintf(rowText[row], "%d) %s : %.0f", shown + 1, scores[shown].racer->field_0x5e0,
                    scores[shown].racer->field_0x768);
            shown++;
        }
        if (own >= shown)
            sprintf(rowText[row], "%d) %s : %.0f", own + 1, scores[own].racer->field_0x5e0,
                    scores[own].racer->field_0x768);
        else
            sprintf(rowText[row], "%d) %s : %.0f", shown + 1, scores[shown].racer->field_0x5e0,
                    scores[shown].racer->field_0x768);
    }
    if (g_TrackGame->field_0x568->field_0xa8) {
        g_TrackGame->LoadResourceString(0x14b8, tag, 0x40);
        sprintf(rowText[5], "  %s %s", g_TrackGame->field_0x568->field_0xa8->field_0x5e0, tag);
    } else if (g_TrackGame->field_0x568->field_0xdc) {
        g_TrackGame->LoadResourceString(0x14b7, tag, 0x40);
        sprintf(rowText[5], "  %s", tag);
    }
    for (i = 0; i <= 5; i++) {
        if (strcmp(rowText[i], field_0x1b0[i]) != 0)
            goto draw;
    }
    return 1;
draw:
    RestoreRect(&sourceRect);
    PCTextureMap* texture = (PCTextureMap*)sharedTexture;
    if (texture->systemSurface->GetDC(&dc) != 0)
        goto fail;
    SetBkColor(dc, 1);
    SetBkMode(dc, 1);
    SetTextColor(dc, 0xffffff);
    font = SelectObject(dc, field_0x11c);
    for (i = 0; i < 6; i++) {
        DrawTextA(dc, rowText[i], strlen(rowText[i]), &rowRects[i], 0x120);
        strcpy(field_0x1b0[i], rowText[i]);
    }
    SelectObject(dc, font);
    if (texture->systemSurface->ReleaseDC(dc) != 0) {
fail:
        return 0;
    }
    TintRows(0, 0x80, 0xffff);
    overlayTexture->UnknownVirtualSlot9(0, -1);
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
    textFont = 0;
    visible = 0;
    field_0x38 = 0;
    textX = 0;
    field_0x40 = 0;
    textY = 0;
    field_0x30 = 1.0f;
}

// 0x0051aee0
DropTextOverlay::~DropTextOverlay()
{
    if (textFont)
        DeleteObject(textFont);
}

// 0x0051b0f0: draws the text twice through the surface's device context,
// black one pixel down and right, then green on top.
int DropTextOverlay::UnknownVirtualSlot15()
{
    if (visible) {
        GameObject::UnknownVirtualSlot15();
        void* dc;
        if (((PCRenderTarget*)field_0x18)->renderSurface->GetDC(&dc) == 0) {
            SetBkColor(dc, 1);
            SetBkMode(dc, 1);
            void* font = SelectObject(dc, textFont);
            SetTextColor(dc, 0);
            TextOutA(dc, textX + 1, textY + 1, caption, strlen(caption));
            SetTextColor(dc, 0xff00);
            TextOutA(dc, textX, textY, caption, strlen(caption));
            SelectObject(dc, font);
            if (((PCRenderTarget*)field_0x18)->renderSurface->ReleaseDC(dc) != 0)
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
    visible = 1;
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
    UnknownKrustyUIGui* gui = g_TrackGame->ui->field_0x2c;
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
    GameObject::UnknownVirtualSlot10(g_TrackGame->frameTime);
    if (field_0x50) {
        if (field_0x50->field_0x84 > field_0x50->field_0x80)
            UnknownFunction51b670();
        if (field_0x50)
            field_0x50->field_0x84 += g_TrackGame->frameTime;
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
        if (((PCRenderTarget*)field_0x18)->renderSurface->GetDC(&dc) == 0) {
            SetBkColor(dc, 1);
            SetBkMode(dc, 1);
            void* font = SelectObject(dc, field_0x2c);
            SetTextColor(dc, 0);
            DrawTextA(dc, field_0x50->field_0x00, strlen(field_0x50->field_0x00), &field_0x40, 0x124);
            SetTextColor(dc, field_0x54);
            DrawTextA(dc, field_0x50->field_0x00, strlen(field_0x50->field_0x00), &field_0x30, 0x124);
            SelectObject(dc, font);
            if (((PCRenderTarget*)field_0x18)->renderSurface->ReleaseDC(dc) != 0)
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

// 0x0051b690
RadarOverlay::RadarOverlay(int flags) : Overlay(flags, 1)
{
    radarCamera = 0;
    radarView = 0;
    frameTimeSum = 0.0f;
    overlayTexture = 0;
    backingTexture = 0;
    racerCount = 0;
    for (int i = 0; i < 11; i++)
        racers[i] = 0;
    field_0x170 = 0;
    showFrameRate = 0;
    readoutCount = 1;
    field_0x16c = 0;
    field_0x174 = 0;
    largeMap = 0;
    field_0x17c = s_UnknownVector68a410;
    field_0x1b8.sz = 0.00001f;
    field_0x1b8.rhw = 1.0f;
    field_0x1b8.color = 0xedea5e;
    field_0x1b8.specular = 0;
    field_0x1b8.tu = 0.0f;
    field_0x1b8.tv = 0.0f;
    field_0x1d8 = field_0x1b8;
    field_0x188 = 0.0f;
    field_0x18c = 0.0f;
    field_0x190 = 0.0f;
    field_0x194 = 0.0f;
    mapScale = 0.0f;
    mapRadius = 0.0f;
    mapCenterX = 0;
    mapCenterY = 0;
    frameCount = 0.0f;
    averageFrameTime = 0.0f;
    frameRateSum = 0.0f;
}

// 0x0051b7d0
RadarOverlay::~RadarOverlay()
{
    if (overlayTexture)
        overlayTexture->Release();
    if (backingTexture)
        backingTexture->Release();
}

// 0x0051b840
RadarOverlay* RadarOverlay::UnknownFunction51b840(RenderTarget* target, TextureMapManager* manager,
                                                  int a3, UnknownOverlayRect screen)
{
    const char* font;
    UnknownOverlayRect source;
    char name[260];
    char path[260];
    largeMap = screen.right > 800;
    if (largeMap) {
        mapRect.left = screen.right - 257;
        mapRect.top = screen.top;
        mapRect.right = screen.right - 1;
        mapRect.bottom = screen.top + 256;
        strcpy(name, "radar256.tga");
        mapRadius = 118.0f;
        mapScale = mapRadius / 180.0f / s_UnknownGlobal57513c;
        mapCenterX = mapRect.left + 126;
        mapCenterY = mapRect.top + 129;
        frameRateRect.left = mapRect.left + 200;
        frameRateRect.right = mapRect.left + 260;
        frameRateRect.top = mapRect.top + 238;
        frameRateRect.bottom = mapRect.top + 250;
    } else {
        mapRect.left = screen.right - 129;
        mapRect.top = screen.top;
        mapRect.right = screen.right - 1;
        mapRect.bottom = screen.top + 128;
        strcpy(name, "radar128.tga");
        mapRadius = 55.0f;
        mapScale = mapRadius / 180.0f / s_UnknownGlobal57513c;
        mapCenterX = mapRect.left + 61;
        mapCenterY = mapRect.top + 65;
        frameRateRect.left = mapRect.left + 86;
        frameRateRect.right = mapRect.left + 124;
        frameRateRect.top = mapRect.top + 112;
        frameRateRect.bottom = mapRect.top + 124;
    }
    radarCamera = (UnknownRadarCamera*)a3;
    backingTexture = UnknownFunction50a590(manager, name, 4444, 0, 8, 5, 6, 0, 0x10, 0xff00ff, 1, 1);
    if (!backingTexture) {
        Release();
        return 0;
    }
    overlayTexture = backingTexture->UnknownVirtualSlot6();
    if (overlayTexture->field_0x20 == 4444)
        overlayTexture->UnknownFunction50abd0(5, 6);
    overlayTexture->UnknownVirtualSlot8(1, 0, 0);
    source.left = 0;
    source.right = largeMap ? 256 : 128;
    source.top = 0;
    source.bottom = largeMap ? 256 : 128;
    font = "arialsm";
    sprintf(path, "%s\\%s", "Res", "Fonts.res");
    Attach(target, overlayTexture, &mapRect, 0, &source, 0.00001f, 1, path, &font,
                          1, 4444, 0);
    return this;
}

// 0x0051baf0: takes the racers from `view`.
int RadarOverlay::UnknownFunction51baf0(UnknownKrustyBikeView* view, int a2)
{
    int iterator;
    UnknownEventRacer* racer;
    racerCount = 0;
    iterator = 0;
    radarView = view;
    while ((racer = radarView->UnknownFunction4204e0(&iterator)) != 0) {
        racers[racerCount] = racer;
        racerCount++;
    }
    return 1;
}

// 0x0051bc60
int RadarOverlay::DrawForViewMode()
{
    switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
    case 0:
        DrawRacers(0);
        break;
    case 1:
    case 5:
        DrawRacers(1);
        break;
    case 2:
    case 3:
        DrawRacers(2);
        break;
    case 4:
        DrawRacers(4);
        break;
    }
    return 1;
}

// 0x0051bcd0
int RadarOverlay::UnknownVirtualSlot10(float frameTime)
{
    Overlay::UnknownVirtualSlot10(g_TrackGame->frameTime);
    frameTimeSum += g_TrackGame->frameTime;
    frameCount += 1.0f;
    return 1;
}

// 0x0051bd20
int RadarOverlay::UnknownVirtualSlot14()
{
    if (g_TrackGame->mode.field_0x6b8) {
        Target()->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        Target()->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        Overlay::UnknownVirtualSlot14();
        Target()->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        Target()->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        Target()->UnknownVirtualSlot8(D3DRENDERSTATE_COLORKEYENABLE, 0, 0);
        Target()->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
        if (g_TrackGame->mode.field_0x6b8) {
            DrawForViewMode();
            DrawFrameRate();
        }
        GameObject::UnknownVirtualSlot14();
    }
    return 1;
}

// 0x0051bdc0: averages the frame rate and draws it when enabled.
void RadarOverlay::DrawFrameRate()
{
    int count;
    char text[128];
    if (averageFrameTime != 0.0f && averageFrameTime < 1000.0f)
        frameRateSum += 1000.0f / averageFrameTime;
    float average = frameRateSum / readoutCount++;
    if (showFrameRate) {
        sprintf(text, "%.0f/%.0ffps", 1000.0f / averageFrameTime, average);
        field_0x38->SelectFont("arialsm");
        void* laidOut = field_0x38->UnknownFunction50ba00(&frameRateRect, text, 0xc8c8dc, &count);
        field_0x38->DrawVertices(Target(), laidOut, count);
    }
    averageFrameTime = frameTimeSum * 1000.0f / frameCount;
    frameTimeSum = 0.0f;
    frameCount = 0.0f;
}
// 0x0051c360
int RadarOverlay::WorldToMap(Vector3 point, int* x, int* y, float* rimX, float* rimY)
{
    float dx = point.x - field_0x17c.x;
    float dz = point.z - field_0x17c.z;
    *x = (int)((dz * field_0x194 + dx * field_0x190) * mapScale);
    *y = -(int)((dz * field_0x190 - dx * field_0x194) * mapScale);
    float angle = (float)atan2((float)*y, (float)*x);
    float distance = UnknownFunction460b50((float)(*x * *x + *y * *y));
    int outside = 0;
    if (distance >= mapRadius) {
        *rimX = cos(angle) * mapRadius + mapCenterX;
        *rimY = sin(angle) * mapRadius + mapCenterY;
        outside = 1;
    }
    *x += mapCenterX;
    *y += mapCenterY;
    return outside;
}

// 0x0051c460
int RadarOverlay::LineThrough(const float* a, const float* b, float* nx, float* ny, float* d)
{
    float dx = b[0] - a[0];
    float dy = b[1] - a[1];
    float lengthSquared = dx * dx + dy * dy;
    if (lengthSquared < 1e-14f)
        return -1;
    float scale = 1.0f / UnknownFunction460b50(lengthSquared);
    *nx = -(dy * scale);
    *ny = dx * scale;
    *d = (a[0] * b[1] - b[0] * a[1]) * scale;
    return 0;
}

// The race object RadarOverlay's gate drawing reads (see TrackOverlay.h).
#define RadarRace() ((UnknownRadarGateOwner*)g_TrackGame->field_0x560)

// Cross product as 0x0051cb20 inlines it. The doubled parentheses matter:
// with them VC6 keeps the folded zero term on the x87 stack and loads the
// other operand first, as retail does.
static inline Vector3 RadarCross(const Vector3& a, const Vector3& b)
{
    Vector3 r;
    r.x = ((a.y * b.z) - (a.z * b.y));
    r.y = ((a.z * b.x) - (a.x * b.z));
    r.z = ((a.x * b.y) - (a.y * b.x));
    return r;
}

// 0x0051cb20
void RadarOverlay::DrawGates()
{
    int x;
    int y;
    Vector3 left;
    Vector3 right;
    Vector3 rim;
    float width = RadarRace()->gateEntries[0]->halfWidth;
    for (int i = 0; i < RadarRace()->gateCount; i++) {
        if (RadarRace()->gateEntries[i]->drawEnabled == 0)
            continue;
        if (i != RadarRace()->currentGate && i != RadarRace()->nextGate)
            continue;
        if (WorldToMap(RadarRace()->gates[i].position, &x, &y, &rim.x, &rim.y) != 0)
            continue;
        left = RadarCross(RadarRace()->gates[i].direction, Vector3(0.0f, 1.0f, 0.0f));
        left.x *= width;
        left.z *= width;
        left += RadarRace()->gates[i].position;
        right = RadarCross(Vector3(0.0f, 1.0f, 0.0f), RadarRace()->gates[i].direction);
        right.x *= width;
        right.z *= width;
        right += RadarRace()->gates[i].position;
        if (i == RadarRace()->currentGate) {
            field_0x1b8.color = 0xffff5e;
            field_0x1d8.color = 0xffff5e;
        } else {
            field_0x1b8.color = 0xffffff;
            field_0x1d8.color = 0xffffff;
        }
        WorldToMap(left, &x, &y, &rim.x, &rim.y);
        field_0x1b8.sx = (float)x;
        field_0x1b8.sy = (float)y;
        WorldToMap(right, &x, &y, &rim.x, &rim.y);
        field_0x1d8.sx = (float)x;
        field_0x1d8.sy = (float)y;
        Target()->UnknownVirtualSlot16(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&field_0x1b8, 2, 0);
    }
}

// 0x0051cda0
ChatOverlay::ChatOverlay(int flags) : Overlay(flags, 1)
{
    chatCamera = 0;
    timeSum = 0.0f;
    overlayTexture = 0;
    backingTexture = 0;
    field_0x198 = 0;
    for (int i = 0; i < 11; i++)
        racers[i] = 0;
    field_0x120 = 0;
    field_0x124 = 0;
    field_0x160 = 0;
    field_0x11c = 0;
    inputShown = 0;
    chatInput = new(__FILE__, 2172) UnknownChatInput;
    for (i = 0; i < 11; i++)
        lastRacerValues[i] = 0;
    field_0x1c8 = 0;
    field_0x1cc = 0;
    field_0x1d0 = 0.0f;
    playerName[0] = '\0';
    nameChanged = 0;
    field_0x164 = 0;
    field_0x3dc = 0;
}

// 0x0051cee0
ChatOverlay::~ChatOverlay()
{
    if (field_0x120)
        DeleteObject(field_0x120);
    if (field_0x124)
        DeleteObject(field_0x124);
    if (field_0x11c)
        DeleteObject(field_0x11c);
    if (overlayTexture)
        overlayTexture->Release();
    if (backingTexture)
        backingTexture->Release();
}

// 0x0051d980
void ChatOverlay::UnknownFunction51d980(int show)
{
    if (g_TrackGame->mode.field_0x27f8.field_0x04 == 0) {
        field_0x164 = show;
        NameOverlay* tag = nameTags[field_0x198];
        if (tag) {
            if (show)
                tag->UnknownVirtualSlot5();
            else
                tag->UnknownVirtualSlot4();
        }
    }
}

// 0x0051d9c0
void ChatOverlay::UnknownFunction51d9c0(float value, const char* name)
{
    field_0x1d0 = value;
    if (_stricmp(name, playerName))
        nameChanged = 1;
    int n = strlen(name);
    int length = n > 0x103 ? 0x103 : n;
    strncpy(playerName, name, length);
    playerName[length] = '\0';
}

// 0x0051da30
int ChatOverlay::UnknownFunction51da30(int key, int* result)
{
    UnknownChatMessage message;
    if (inputShown) {
        *result = 0;
        if (key == 0xd || key == 0xa) {
            int n = strlen(chatInput->GetLine());
            int length = n > 0x49 ? 0x49 : n;
            strncpy(message.text, chatInput->GetLine(), length);
            message.text[length] = '\0';
            if (strlen(message.text)) {
                g_TrackGame->network->Send(
                    0x85, &message, 0x4e, g_TrackGame->network->localPlayer, 0);
                UnknownFunction51dd70(g_TrackGame->mode.field_0x00, (int)message.text, 0);
            }
            chatInput->ClearLine();
            field_0x3dc = 0;
            return 1;
        }
        if (key == 8) {
            chatInput->Backspace();
            return 1;
        }
        if (key == 0x1b) {
            *result = 1;
            return 1;
        }
        if (field_0x3dc && !g_TrackGame->controlInterface->keyboard->UnknownFunction48a240(0xc) && key) {
            chatInput->AppendChar(key);
            return 1;
        }
    }
    return 0;
}

// 0x0051dce0
int ChatOverlay::UnknownFunction51dce0(UnknownControlEvent* event, UnknownInputEntry* entry, int* result)
{
    if (inputShown && field_0x3dc && !event->kind)
        return 1;
    return 0;
}

// 0x0051dd10
void ChatOverlay::UnknownFunction51dd10()
{
    inputShown = 1;
    if (field_0x1c8) {
        field_0x1c8->UnknownVirtualSlot5();
        field_0x1cc->UnknownVirtualSlot5();
    }
}

// 0x0051dd40
void ChatOverlay::UnknownFunction51dd40()
{
    inputShown = 0;
    if (field_0x1c8) {
        field_0x1c8->UnknownVirtualSlot4();
        field_0x1cc->UnknownVirtualSlot4();
    }
}

// 0x0051dd70: names longer than ten characters are cut to eight and "..".
void ChatOverlay::UnknownFunction51dd70(const char* name, int a2, int a3)
{
    char text[32];
    if (strlen(name) > 10) {
        strncpy(text, name, 8);
        text[8] = '\0';
        strcat(text, "..");
    } else {
        strncpy(text, name, 10);
    }
    chatInput->AddHistory(text, (const char*)a2, a3);
}

// 0x0051e200
int ChatOverlay::UnknownVirtualSlot10(float frameTime)
{
    Overlay::UnknownVirtualSlot10(g_TrackGame->frameTime);
    timeSum += g_TrackGame->frameTime;
    return 1;
}

// 0x0051e240
int ChatOverlay::UnknownVirtualSlot13()
{
    UnknownOverlayRect rect;
    if (timeSum >= 0.1f) {
        if (inputShown)
            RedrawChat();
        timeSum = 0.0f;
    }
    if (s_UnknownGlobal68a444) {
        rect.left = 0;
        rect.top = 0;
        rect.right = 0xff;
        rect.bottom = 0xff;
        RestoreRect(&rect);
        UnknownFunction51e910(-1);
    } else {
        for (int i = 0; i < field_0x198; i++) {
            if (g_TrackGame->mode.field_0x6c0 && lastRacerValues[i] != racers[i]->field_0x784) {
                UnknownFunction51e910(i);
                lastRacerValues[i] = racers[i]->field_0x784;
            }
            if (racers[i]->racerState->hideNameTag)
                nameTags[i]->UnknownVirtualSlot4();
            else if (racers[i] == chatView->ownRacer && chatCamera->followedRacer == chatView->ownRacer)
                nameTags[i]->UnknownVirtualSlot4();
            else
                nameTags[i]->UnknownVirtualSlot5();
        }
    }
    if (g_TrackGame->mode.field_0x27f8.field_0x04 == 0)
        RedrawNameLine();
    return 1;
}

// 0x0051e390
int ChatOverlay::UnknownVirtualSlot14()
{
    if (inputShown) {
        Target()->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        Target()->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        Overlay::UnknownVirtualSlot14();
    }
    if (g_TrackGame->mode.field_0x6bc || g_TrackGame->mode.field_0x6c0)
        GameObject::UnknownVirtualSlot14();
    return 1;
}

// 0x0051de10
int ChatOverlay::RedrawChat()
{
    void* dc;
    int lineHeight;
    int value;
    void* font;
    int rows;
    UnknownOverlayRect rect;
    char text[0x80];
    RestoreRect(&sourceRect);
    PCTextureMap* texture = (PCTextureMap*)sharedTexture;
    if (texture->systemSurface->GetDC(&dc) != 0)
        goto fail;
    SetBkColor(dc, 1);
    SetBkMode(dc, 1);
    SetTextColor(dc, 0xffffff);
    font = SelectObject(dc, field_0x120);
    strcpy(text, ">");
    strcat(text, chatInput->GetLineTail());
    strcat(text, "_");
    rect.left = 1;
    if (largeLayout) {
        rect.right = 0xff;
        rect.top = 7;
        rect.bottom = 0x15;
        lineHeight = 0xe;
        rows = 0x3e;
    } else {
        rect.right = 0x7f;
        rect.top = 7;
        rect.bottom = 0xe;
        lineHeight = 7;
        rows = 0x1f;
    }
    DrawTextA(dc, text, strlen(text), &rect, 0x124);
    int i;
    for (i = 0; i <= chatInput->field_0x1c0; i++) {
        int n = strlen(chatInput->GetHistory(i, &value));
        int length = n > 0x7f ? 0x7f : n;
        strncpy(text, chatInput->GetHistory(i, &value), length);
        rect.top = (i + 1) * lineHeight + 7;
        rect.bottom = rect.top + lineHeight;
        text[length] = '\0';
        if (value == 0)
            DrawTextA(dc, text, strlen(text), &rect, 0x124);
    }
    SelectObject(dc, font);
    if (texture->systemSurface->ReleaseDC(dc) != 0)
        goto fail;
    TintRows(0, rows, 0xfff0);
    if (texture->systemSurface->GetDC(&dc) != 0)
        goto fail;
    SetBkColor(dc, 1);
    SetBkMode(dc, 1);
    SetTextColor(dc, 0xffffff);
    font = SelectObject(dc, field_0x120);
    for (i = 0; i <= chatInput->field_0x1c0; i++) {
        strcpy(text, chatInput->GetHistory(i, &value));
        rect.top = (i + 1) * lineHeight + 7;
        rect.bottom = rect.top + lineHeight;
        if (value != 0)
            DrawTextA(dc, text, strlen(text), &rect, 0x124);
    }
    SelectObject(dc, font);
    if (texture->systemSurface->ReleaseDC(dc) != 0) {
fail:
        return 0;
    }
    TintRows(0, rows, 0xf0ff);
    overlayTexture->UnknownVirtualSlot9(0, -1);
    return 1;
}

// 0x0051e7c0
void ChatOverlay::UnknownFunction51e7c0()
{
    g_TrackGame->mode.field_0x90++;
    if (g_TrackGame->mode.field_0x90 == 6)
        g_TrackGame->mode.field_0x90 = 0;
    UnknownFunction51e910(-1);
}

// 0x0051e800
void ChatOverlay::RedrawNameLine()
{
    UnknownOverlayRect rect;
    char text[128];
    rect = nameTagRects[field_0x198];
    rect.right = field_0x160;
    RestoreRect(&rect);
    sprintf(text, "%d : ", (int)sqrt(field_0x1d0));
    field_0x38->SelectFont("arialsm");
    int right = field_0x38->UnknownFunction50b080(overlayTexture, nameTagRects[field_0x198].left,
                                                   nameTagRects[field_0x198].top, text, 0);
    if (right != field_0x160 || nameChanged) {
        field_0x160 = right;
        nameChanged = 0;
        nameTags[field_0x198]->field_0x124 = right - nameTagRects[field_0x198].left;
        UnknownFunction51e910(field_0x198);
    }
}

// 0x0051eb40
void UnknownChatInput::AddHistory(const char* name, const char* text, int value)
{
    char separator[8];
    int nameLength = strlen(name);
    int textLength = strlen(text);
    strcpy(separator, ": ");
    int separatorLength = strlen(separator);
    if (field_0x1c0 < 3) {
        strcpy(history[field_0x1c0].text, name);
        strcat(history[field_0x1c0].text, separator);
        strncat(history[field_0x1c0].text, text, 0x31);
        history[field_0x1c0].text[0x31] = '\0';
        history[field_0x1c0].value = value;
        field_0x1c0++;
    } else {
        for (int i = 0; i < 2; i++) {
            strncpy(history[i].text, history[i + 1].text, 0x31);
            history[i].value = history[i + 1].value;
            history[i].text[0x31] = '\0';
        }
        strcpy(history[2].text, name);
        strcat(history[2].text, separator);
        strncat(history[2].text, text, 0x31);
        history[2].text[0x31] = '\0';
        history[2].value = value;
    }
    if (nameLength + separatorLength + textLength > 0x31) {
        if (field_0x1c0 < 3) {
            strcpy(history[field_0x1c0].text, "             ");
            history[field_0x1c0].text[separatorLength + nameLength] = '\0';
            strncat(history[field_0x1c0].text, text + 0x31 - separatorLength - nameLength, 0x31);
            history[field_0x1c0].value = value;
            field_0x1c0++;
        } else {
            for (int i = 0; i < 2; i++) {
                strncpy(history[i].text, history[i + 1].text, 0x31);
                history[i].text[0x31] = '\0';
                history[i].value = history[i + 1].value;
            }
            strcpy(history[2].text, "             ");
            history[2].text[separatorLength + nameLength] = '\0';
            strncat(history[2].text, text + 0x31 - separatorLength - nameLength, 0x31);
            history[2].value = value;
        }
    }
}

// 0x0051ea50
UnknownChatInput::UnknownChatInput()
{
    typedLine[0] = '\0';
    for (int i = 0; i < 4; i++) {
        history[i].value = 0;
        history[i].text[0] = '\0';
    }
    typedLength = 0;
    field_0x1c0 = 0;
}

// 0x0051ea80
void UnknownChatInput::AppendChar(char c)
{
    if (typedLength < 0x4a) {
        typedLine[typedLength] = c;
        typedLength++;
        typedLine[typedLength] = '\0';
    }
}

// 0x0051eab0
void UnknownChatInput::Backspace()
{
    if (typedLength > 0) {
        typedLength--;
        typedLine[typedLength] = '\0';
    }
}

// 0x0051ead0
void UnknownChatInput::ClearLine()
{
    typedLine[0] = '\0';
    typedLength = 0;
}

// 0x0051eae0: the 3-byte `mov eax, ecx; ret`. Its only callers are
// RedrawChat's two calls (0x0051dae5, 0x0051db09).
char* UnknownChatInput::GetLine()
{
    return typedLine;
}

// 0x0051eaf0
char* UnknownChatInput::GetLineTail()
{
    if (typedLength >= 0x31)
        return typedLine + typedLength - 0x31;
    return typedLine;
}

// 0x0051eb10
char* UnknownChatInput::GetHistory(int index, int* value)
{
    if (index < 4) {
        *value = history[index].value;
        return history[index].text;
    }
    return 0;
}

// 0x0051e910: redraws name tag `index` (all of them for -1) into the shared
// texture, then colours it with a grey that darkens with the cycle
// TrackGameMode+0x90.
void ChatOverlay::UnknownFunction51e910(int index)
{
    unsigned char level = 0xff - g_TrackGame->mode.field_0x90 * 0x33;
    if (index == -1) {
        for (int i = 0; i < field_0x198; i++)
            RestoreRect(&nameTagRects[i]);
    } else {
        RestoreRect(&nameTagRects[index]);
    }
    PCTextureMap* texture = (PCTextureMap*)sharedTexture;
    void* dc;
    if (texture->systemSurface->GetDC(&dc) == 0) {
        SetBkColor(dc, 1);
        SetBkMode(dc, 1);
        SetTextColor(dc, 0xffffff);
        if (index == -1) {
            for (int i = 0; i < field_0x198; i++)
                DrawNameTag(dc, i);
        } else {
            DrawNameTag(dc, index);
        }
        texture->systemSurface->ReleaseDC(dc);
    }
    TintRows(0x89, 0x46, 0xf000 | ((level & ~0xf) << 4) | (level & ~0xf) | (level >> 4));
    overlayTexture->UnknownVirtualSlot9(0, -1);
}
