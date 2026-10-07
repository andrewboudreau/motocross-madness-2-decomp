// Near-miss TrackOverlay.cpp candidates, kept out of src/reconstructed until
// they match. See docs/TRACKOVERLAY.md. They compile against
// src/reconstructed/TrackOverlay.h.
//
// RadarOverlay::UnknownVirtualSlot23 (0x0051bb60, 246 bytes, 89.69%): the
// zoom keys. Both arms of the field_0x178 test are identical, so retail keeps
// only a dead `test`; it schedules fld/fmul/fidiv before that test and VC6
// here after it. A scale local, a ternary, assigning before the test and an
// empty or return-only test do not reproduce it.

// RadarOverlay::IntersectLineCircle (0x0051c4f0, 560 bytes, 64%): the
// candidate is 550 bytes. The arithmetic and branch structure match; VC6
// here assigns the scale, discriminant and intersection temporaries to
// other frame slots (retail shares one slot for the scale and the
// discriminant and reuses the parameter slots for the direction and x1/x2),
// and computes proj + q with fxch/fadd instead of fld/faddp st(2).
// Declaration order, operand order of the distance sums and the store order
// of the result do not change it.

// ChatOverlay::UnknownFunction51d730 (0x0051d730, 586 bytes, 44%): the
// candidate is 587 bytes with the same instructions. Retail keeps the zero
// in ebx and the incremented count in ebp; VC6 here swaps them, which
// shifts later register choices. The racer loop written as in RadarOverlay
// 0x0051baf0, a post-increment index, a for loop and every order of the
// three initial assignments do not change it.

// StatsOverlay::UnknownFunction519a20 (0x00519a20, 1002 bytes): written
// like the exact 0x0051aa40, the candidate is 1004 bytes. Retail keeps the
// row-comparison index in edx and `this` in its spill slot; VC6 here does so
// only when the index is read after the loop (the `row < i` test below),
// which leaves a redundant compare on the mismatch path and swaps edi and
// ebp for `this` and the view. A goto out of the loop, a flag, drawing
// inside the loop and separate loop variables spill the index instead.

// StatsOverlay::UnknownFunction5194b0 (0x005194b0, 976 bytes): the
// candidate is 956 bytes with the same instruction sequence. Six identical
// cases give retail's jump table. Retail places the LOGFONT (0x3c bytes)
// below the two rectangles in the frame; VC6 here puts the rectangles first
// (with a LOGFONT of 0x38 bytes or less it orders them as retail), so the
// rectangle stores use different displacements. Declaration order, names,
// aggregate initializers and UnknownMakeOverlayRect do not move them.

// ChatOverlay::DrawNameTag (0x0051e3f0, 974 bytes, 966 match):
// the frame and code match except in the racer-tag branch. A racer local in
// the position branch gives retail's eax there; in the "%.3f" branch retail
// loads the racer into eax and the buffer address into ecx (VC6 here edx and
// eax), and loads the game pointer into edx (here ecx). Inverted branches, a
// racer local in that branch, a game local, a float or double local and a
// buffer pointer do not change it.

// RadarOverlay::DrawTrackOutline (0x0051c720, 1016 bytes; candidate
// 1002): draws the track outline through a TrackListItem work list. The
// control flow, the calls and the stores match once the node is read
// through a reference to the list entry (`TrackNode*& node`, retail keeps
// &list->field_0x04 in edi), the visited/unvisited branches are an if/else
// and the segment walk is `if (segment) do ... while (segment)`. Left: the
// frame (retail 0x8c bytes, here 0x74; retail groups the clip arrays as
// both circles, both line ends, both hits, and keeps the work list and the
// node reference in frame slots below the point), the store order of the
// entering clip's hit point, and the child loop, where retail reloads the
// node from edi in the body instead of reusing the loop test's eax.

// ChatOverlay::UnknownFunction51cf80 (0x0051cf80, 1953 bytes; candidate
// 1947): the loader. Frame layout, calls, the font set-up and the
// rectangles match once the side-panel rectangle is declared inside the
// wide-screen block. Left: scheduling in that block (retail keeps the chat
// rectangle's left edge in ebx and later -1 in ebx for both the EH state and
// the 0x00469190 argument), the order of the LOGFONT field stores, and the
// thirteenth name-tag rectangle, whose constructor temporary retail places
// in a separate frame slot (0x74) where VC6 here reuses the first one.


// RadarOverlay::DrawRacers (0x0051bed0, 1160 bytes; 1129 match):
// the map update. Every call, loop and store matches with the racer read
// through `field_0x134[i]` each time, the own position through a reference,
// the rim point as a Vector3 (its third slot is unused) and the chosen
// screen point as the ternaries below. Left: retail allocates the
// int-to-float slot of `outside` right above `x` (here it is the last
// scalar slot, so y and the loop temporaries shift by one), and schedules
// the distance products before the name address and the pushes of the
// 0x0051d9c0 call. Declaration order and scope, an int copy, a float copy
// and a distance local do not move either.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/TrackOverlay.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/Camera.h"
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/Track.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/ControlInterface.h"
#include "../../src/reconstructed/D3DConstants.h"

// GDI32 imports, as in TrackOverlay.cpp.
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
extern "C" __declspec(dllimport) int __stdcall GetTextExtentPoint32A(void* dc, const char* text, int length,
                                                                     UnknownTextExtent* size);
extern "C" __declspec(dllimport) int __stdcall DrawTextA(void* dc, const char* text, int length,
                                                         UnknownOverlayRect* rect, unsigned int format);

// TrackOverlay.cpp statics: the radar zoom (0x0057513c) and the ChatOverlay
// redraw flag (0x0068a444).
static int s_UnknownGlobal57513c = 2;
static int s_UnknownGlobal68a444;

// 0x0051bb60: control 0x21 toggles the frame rate, 0x34 and 0x33 zoom.
// Retail tests +0x178 before each scale update although both arms compute
// the same value; the identical if/else keeps that test.
int RadarOverlay::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    if (UnknownFunction43caa0(0x21, 0, event, 0x80000000)) {
        showFrameRate = 1 - showFrameRate;
        return 1;
    }
    if (UnknownFunction43caa0(0x34, 0, event, 0x80000000)) {
        if (++s_UnknownGlobal57513c > 5)
            s_UnknownGlobal57513c = 5;
        if (largeMap)
            mapScale = mapRadius / 180.0f / s_UnknownGlobal57513c;
        else
            mapScale = mapRadius / 180.0f / s_UnknownGlobal57513c;
        return 1;
    }
    if (UnknownFunction43caa0(0x33, 0, event, 0x80000000)) {
        if (--s_UnknownGlobal57513c < 1)
            s_UnknownGlobal57513c = 1;
        if (largeMap)
            mapScale = mapRadius / 180.0f / s_UnknownGlobal57513c;
        else
            mapScale = mapRadius / 180.0f / s_UnknownGlobal57513c;
        return 1;
    }
    return 0;
}

// 0x0051c4f0
int RadarOverlay::IntersectLineCircle(const float* a, const float* b, const float* circle, float* a4, float* point)
{
    float nx;
    float ny;
    float d;
    if (LineThrough(a, b, &nx, &ny, &d))
        return -1;
    float scale = 1.0f / (nx * nx + ny * ny);
    float t = -(scale * d);
    float px = t * nx;
    float py = t * ny;
    float root = UnknownFunction460b50(scale);
    float dx = root * ny;
    float dy = -(root * nx);
    float lengthSquared = dy * dy + dx * dx;
    if (lengthSquared < 1e-14f)
        return 3;
    float ex = circle[0] - px;
    float ey = circle[1] - py;
    float cross = ey * dx - ex * dy;
    float r = circle[2];
    float disc = r * r * lengthSquared - cross * cross;
    if (disc < -1e-14f)
        return -1;
    float proj = ex * dx + ey * dy;
    if (disc < 1e-14f) {
        float k = proj / lengthSquared;
        point[0] = k * dx + px;
        point[1] = k * dy + py;
        return 1;
    }
    float q = UnknownFunction460b50(disc);
    float inverse = 1.0f / lengthSquared;
    float k1 = (proj - q) * inverse;
    float k2 = (proj + q) * inverse;
    float x1 = k1 * dx + px;
    float y1 = k1 * dy + py;
    float x2 = k2 * dx + px;
    float y2 = k2 * dy + py;
    float ay = point[1] - y1;
    float ax = point[0] - x1;
    float by = point[1] - y2;
    float bx = point[0] - x2;
    if (ax * ax + ay * ay < by * by + bx * bx) {
        point[0] = x1;
        point[1] = y1;
    } else {
        point[0] = x2;
        point[1] = y2;
    }
    return 2;
}

// 0x0051d730
int ChatOverlay::UnknownFunction51d730(UnknownChatView* view)
{
    int iterator;
    UnknownChatRacer* racer;
    field_0x198 = 0;
    iterator = 0;
    chatView = view;
    while ((racer = chatView->UnknownFunction4204e0(&iterator)) != 0) {
        racers[field_0x198] = racer;
        field_0x198++;
    }
    for (int i = 0; i < field_0x198; i++) {
        nameTags[i] = (new(__FILE__, 2369) NameOverlay(1))->UnknownFunction518e30(
            Target(), overlayTexture, &nameTagRects[i], 0, &nameTagRects[i], (UnknownEventRacer*)racers[i],
            (UnknownNameOverlayWorld*)chatView, i);
        UnknownFunction469190(nameTags[i], -1);
    }
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 0) {
        nameTags[field_0x198] = (new(__FILE__, 2376) NameOverlay(1))->UnknownFunction518e30(
            Target(), overlayTexture, &nameTagRects[field_0x198], 0, &nameTagRects[field_0x198], 0,
            (UnknownNameOverlayWorld*)chatView, i);
        UnknownFunction469190(nameTags[field_0x198], -1);
        if (nameTags[field_0x198]) {
            nameTags[field_0x198]->UnknownFunction519080(Vector3(cueRect.left, cueRect.top, 0));
            nameTags[field_0x198]->UnknownVirtualSlot4();
            field_0x160 = nameTagRects[field_0x198].right;
            nameTags[field_0x198]->field_0x124 = 0;
        }
    }
    UnknownFunction51e910(-1);
    return 1;
}

// 0x00519a20: the standings, best score first: the time left (as a label),
// then a row per racer that is shown, ending with the player's own row.
// Draws only when a row changed.
int StatsOverlay::UnknownFunction519a20()
{
    UnknownEventScore scores[11];
    char time[0x80];
    char label[0x80];
    int iterator;
    int own;
    void* dc;
    void* font;
    int i;
    int count = 0;
    UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->field_0x55c->field_0x34;
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 0 && g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 4) {
        g_UnknownGlobal56e26c->UnknownFunction521970(0x943, label, 0x80);
        TrackGameViewOwner* owner = g_UnknownGlobal56e26c->field_0x55c;
        float limit = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140 * 60.0f;
        UnknownFunction518640(time, limit - (owner->field_0x70 * 60.0f + owner->field_0x74));
        sprintf(rowText[0], "%s", label);
    }
    iterator = 0;
    UnknownEventRacer* racer;
    while ((racer = view->UnknownFunction4204e0(&iterator)) != 0) {
        if (racer->field_0x25_bit0) {
            scores[count].value = racer->field_0x768;
            scores[count].racer = racer;
            count++;
        }
    }
    qsort(scores, count, sizeof(UnknownEventScore), UnknownFunction5199f0);
    for (i = 0; i < count; i++) {
        if (scores[i].racer == view->field_0x38) {
            own = i;
            break;
        }
    }
    int first = 1;
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 0 && g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 4)
        count++;
    else
        first = 0;
    if (count >= 6)
        count = 6;
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
    for (i = 0; i <= row; i++) {
        if (strcmp(rowText[i], field_0x1b0[i]) != 0)
            break;
    }
    if (row < i)
        return 1;
    RestoreRect(&sourceRect);
    PCTextureMap* texture = (PCTextureMap*)sharedTexture;
    if (texture->field_0x70->UnknownMethod17(&dc) != 0)
        goto fail;
    SetBkColor(dc, 1);
    SetBkMode(dc, 1);
    SetTextColor(dc, 0xffffff);
    font = SelectObject(dc, field_0x11c);
    for (i = 0; i <= row; i++) {
        if (i == 0)
            GetTextExtentPoint32A(dc, rowText[0], strlen(rowText[0]), &rowExtent);
        DrawTextA(dc, rowText[i], strlen(rowText[i]), &rowRects[i], 0x120);
        strcpy(field_0x1b0[i], rowText[i]);
    }
    SelectObject(dc, font);
    if (texture->field_0x70->UnknownMethod26(dc) != 0) {
fail:
        return 0;
    }
    TintRows(0, 0x80, 0xffff);
    overlayTexture->UnknownVirtualSlot9(0, -1);
    return 1;
}

// 0x005194b0
StatsOverlay* StatsOverlay::UnknownFunction5194b0(RenderTarget* target, TextureMapManager* manager, void* camera,
                                                  UnknownOverlayRect screen)
{
    const char* font;
    UnknownLogFont logFont;
    UnknownOverlayRect rect;
    UnknownOverlayRect source;
    char name[260];
    char path[260];
    instrumentSource = (UnknownInstrumentSource*)camera;
    rect.left = 0;
    rect.top = 0;
    rect.right = 128;
    rect.bottom = 128;
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 0:
        strcpy(name, "BajaScore.tga");
        break;
    case 1:
        strcpy(name, "BajaScore.tga");
        break;
    case 2:
        strcpy(name, "BajaScore.tga");
        break;
    case 3:
        strcpy(name, "BajaScore.tga");
        break;
    case 4:
        strcpy(name, "BajaScore.tga");
        break;
    case 5:
        strcpy(name, "BajaScore.tga");
        break;
    default:
        Release();
        return 0;
    }
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
    source.right = 128;
    source.top = 0;
    source.bottom = 128;
    font = "arialsm";
    sprintf(path, "%s\\%s", "Res", "Fonts.res");
    Attach(target, overlayTexture, &rect, 0, &source, 0.00001f, 1, path, &font, 1, 4444, 0);
    logFont.lfHeight = 16;
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfItalic = 0;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfCharSet = 1;
    logFont.lfOutPrecision = 0;
    logFont.lfClipPrecision = 0;
    logFont.lfQuality = 2;
    logFont.lfPitchAndFamily = 2;
    UnknownKrustyUIGui* gui = g_UnknownGlobal56e26c->ui->field_0x2c;
    const char* face = gui ? gui->field_0x350 : "";
    if (*face != '\0') {
        logFont.lfWeight = gui->field_0x3d4 ? 700 : 500;
    } else {
        logFont.lfWeight = 400;
        face = "Small Fonts";
    }
    strcpy(logFont.lfFaceName, face);
    field_0x11c = CreateFontIndirectA(&logFont);
    logFont.lfHeight = 12;
    field_0x120 = CreateFontIndirectA(&logFont);
    rowRects[0] = UnknownTrackOverlayRect(14, 128, 20, 30);
    rowRects[1] = UnknownTrackOverlayRect(14, 128, 36, 46);
    rowRects[2] = UnknownTrackOverlayRect(14, 128, 53, 63);
    rowRects[3] = UnknownTrackOverlayRect(14, 128, 69, 79);
    rowRects[4] = UnknownTrackOverlayRect(14, 128, 87, 97);
    rowRects[5] = UnknownTrackOverlayRect(14, 100, 105, 115);
    rowRects[6] = UnknownTrackOverlayRect(5, 86, 34, 49);
    return this;
}

// 0x0051e3f0: draws name tag `index` into `dc`: the name line for the
// player's own tag, otherwise the position and the racer's name (or the
// value at +0x4a4), centred, and sizes the tag to the text.
void ChatOverlay::DrawNameTag(void* dc, int index)
{
    char text[0x80];
    char name[0x80];
    int length;
    UnknownTextExtent size;
    void* font = SelectObject(dc, field_0x124);
    if (index == field_0x198) {
        sprintf(text, "%s", playerName);
        length = strlen(text);
        UnknownOverlayRect rect = nameTagRects[index];
        rect.left += nameTags[index]->field_0x124;
        DrawTextA(dc, text, length, &rect, 0x8124);
        UnknownTextExtent size;
        GetTextExtentPoint32A(dc, text, length, &size);
        nameTags[index]->field_0x124 += size.cx;
        nameTags[index]->sourceRect = nameTagRects[index];
        if (nameTags[index]->field_0x124 + 4 < nameTags[index]->sourceWidth) {
            nameTags[index]->field_0x124 += 4;
            nameTags[index]->sourceRect.right = nameTags[index]->field_0x124 + nameTags[index]->sourceRect.left - 1;
        } else {
            nameTags[index]->field_0x124 = nameTags[index]->sourceWidth;
        }
    } else {
        if (s_UnknownGlobal68a444) {
            sprintf(text, "%.3f", racers[index]->field_0x4a4);
        } else {
            if (g_UnknownGlobal56e26c->mode.field_0x6c0) {
                UnknownChatRacer* racer = racers[index];
                if (racer->field_0x784 > 0)
                    sprintf(text, "%d", racer->field_0x784);
                else
                    strcpy(text, "--");
            } else {
                strcpy(text, "");
            }
            if (g_UnknownGlobal56e26c->mode.field_0x6bc)
                strcpy(name, racers[index]->name);
            else
                strcpy(name, "");
            if (g_UnknownGlobal56e26c->mode.field_0x6c0 && g_UnknownGlobal56e26c->mode.field_0x6bc)
                strcat(text, " : ");
            strcat(text, name);
        }
        length = strlen(text);
        GetTextExtentPoint32A(dc, text, length, &size);
        nameTags[index]->field_0x124 = size.cx;
        DrawTextA(dc, text, length, &nameTagRects[index], 0x8125);
        nameTags[index]->sourceRect = nameTagRects[index];
        if (nameTags[index]->field_0x124 + 4 < nameTags[index]->sourceWidth) {
            nameTags[index]->field_0x124 += 4;
            int margin = (nameTags[index]->sourceWidth - nameTags[index]->field_0x124) >> 1;
            nameTags[index]->sourceRect.left += margin;
            nameTags[index]->sourceRect.right += 1 - margin;
        } else {
            nameTags[index]->field_0x124 = nameTags[index]->sourceWidth;
        }
    }
    SelectObject(dc, font);
}

// The view RadarOverlay draws (+0x12c) holds its track at +0x48.
struct UnknownRadarView {
    unsigned char field_0x00[0x48];
    Track* field_0x48;
};

// 0x0051c720: draws the track outline (mode 1: the +0x18 edge of every
// segment, 2: the +0x0c edge), clipped to the map circle, walking every node
// once through a work list.
void RadarOverlay::DrawTrackOutline(int mode)
{
    TrackListItem* list = 0;
    TrackListItem* item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1992);
    item->field_0x04 = ((UnknownRadarView*)radarView)->field_0x48->field_0x00;
    item->field_0x0c = list;
    list = item;
    while (list) {
        TrackNode*& node = list->field_0x04;
        if (node->field_0x00 & 4) {
            node->field_0x00 &= ~4;
            item = list;
            list = list->field_0x0c;
            DebugFree(item, __FILE__, 2001);
        } else {
        node->field_0x00 |= 4;
        TrackSegment* first = node->field_0x08;
        Vector3 point;
        if (mode == 1)
            point = *(Vector3*)&first->field_0x18;
        else if (mode == 2)
            point = *(Vector3*)&first->field_0x0c;
        int startX;
        int startY;
        float rim[2];
        int outside = WorldToMap(point, &startX, &startY, &rim[0], &rim[1]);
        field_0x1b8.sx = (float)startX;
        field_0x1b8.sy = (float)startY;
        field_0x1b8.color = 0xedea5e;
        field_0x1d8.color = 0xedea5e;
        int x = startX;
        int y = startY;
        TrackSegment* segment = first->field_0x2c;
        if (segment) do {
            if (mode == 1)
                point = *(Vector3*)&segment->field_0x18;
            else if (mode == 2)
                point = *(Vector3*)&segment->field_0x0c;
            int wasOutside = outside;
            float from[2];
            from[0] = (float)x;
            from[1] = (float)y;
            outside = WorldToMap(point, &x, &y, &rim[0], &rim[1]);
            if (outside) {
                if (wasOutside) {
                    field_0x1b8.sx = (float)x;
                    field_0x1b8.sy = (float)y;
                } else {
                    float to[2];
                    float circle[3];
                    float hit[2];
                    to[0] = (float)x;
                    to[1] = (float)y;
                    circle[2] = mapRadius;
                    circle[0] = (float)mapCenterX;
                    circle[1] = (float)mapCenterY;
                    if (IntersectLineCircle(from, to, circle, rim, hit) != -1) {
                        field_0x1d8.sx = hit[0];
                        field_0x1d8.sy = hit[1];
                        Target()->UnknownVirtualSlot16(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&field_0x1b8, 2, 0);
                        field_0x1b8 = field_0x1d8;
                    }
                }
            } else if (wasOutside) {
                float to[2];
                float circle[3];
                float hit[2];
                to[0] = (float)x;
                to[1] = (float)y;
                circle[2] = mapRadius;
                circle[0] = (float)mapCenterX;
                circle[1] = (float)mapCenterY;
                if (IntersectLineCircle(from, to, circle, rim, hit) != -1) {
                    field_0x1b8.sy = hit[1];
                    field_0x1d8.sy = (float)y;
                    field_0x1d8.sx = (float)x;
                    field_0x1b8.sx = hit[0];
                    Target()->UnknownVirtualSlot16(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&field_0x1b8, 2, 0);
                    field_0x1b8 = field_0x1d8;
                }
            } else {
                field_0x1d8.sx = (float)x;
                field_0x1d8.sy = (float)y;
                Target()->UnknownVirtualSlot16(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&field_0x1b8, 2, 0);
                field_0x1b8 = field_0x1d8;
            }
            segment = segment->field_0x2c;
            if (segment == node->field_0x08) {
                field_0x1d8.sx = (float)startX;
                field_0x1d8.sy = (float)startY;
                break;
            }
        } while (segment);
        for (int i = 0; i < node->field_0x10; i++) {
            if (!(node->field_0x14[i]->field_0x00 & 4)) {
                item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 2094);
                item->field_0x04 = node->field_0x14[i];
                item->field_0x0c = list;
                list = item;
            }
        }
        }
    }
}

extern "C" __declspec(dllimport) void* __stdcall CreatePen(int style, int width, unsigned long color);

// 0x0051cf80: sets the chat overlay up: the chat and cue rectangles, the
// texture, the side panels on wide screens, the pen, two fonts and the
// thirteen name-tag rectangles.
ChatOverlay* ChatOverlay::UnknownFunction51cf80(RenderTarget* target, TextureMapManager* manager, void* camera,
                                                UnknownOverlayRect screen, UnknownOverlayRect cue)
{
    const char* font;
    UnknownOverlayRect source;
    UnknownLogFont logFont;
    char name[260];
    char path[260];

    largeLayout = 1;
    chatRect.left = screen.right / 2 - 128;
    chatRect.top = 0;
    chatRect.right = chatRect.left + 256;
    chatRect.bottom = 68;
    strcpy(name, "chat256.tga");
    chatCamera = (UnknownChatCamera*)camera;
    cueRect = cue;
    timeSum = 0.08f;
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
    source.right = largeLayout ? 256 : 128;
    source.top = 0;
    source.bottom = largeLayout ? 68 : 37;
    font = "arialsm";
    sprintf(path, "%s\\%s", "Res", "Fonts.res");
    Attach(target, overlayTexture, &chatRect, 0, &source, 0.00001f, 1, path, &font, 1, 4444, 0);
    if (screen.right - screen.left > 512) {
        UnknownOverlayRect rect = chatRect;
        source.left = 3;
        source.top = 69;
        rect.right = rect.left;
        rect.left -= largeLayout ? 38 : 19;
        source.right = largeLayout ? 38 : 19;
        source.bottom = largeLayout ? 137 : 103;
        field_0x1c8 = new (__FILE__, 2284) Overlay(0, 1);
        field_0x1c8->Attach(target, overlayTexture, &rect, 0, &source, 0.00001f, 0, 0, 0, 0, 1555, 0);
        UnknownFunction469190(field_0x1c8, -1);
        source.left += largeLayout ? 38 : 19;
        source.top = 69;
        source.right += largeLayout ? 38 : 19;
        source.bottom = largeLayout ? 137 : 103;
        rect.left = chatRect.right;
        rect.right = chatRect.right + (largeLayout ? 38 : 19);
        field_0x1cc = new (__FILE__, 2294) Overlay(0, 1);
        field_0x1cc->Attach(target, overlayTexture, &rect, 0, &source, 0.00001f, 0, 0, 0, 0, 1555, 0);
        UnknownFunction469190(field_0x1cc, -1);
    }
    field_0x11c = CreatePen(0, 3, 0xc0c0c0);
    logFont.lfHeight = 9;
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfItalic = 0;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfCharSet = 1;
    logFont.lfOutPrecision = 0;
    logFont.lfClipPrecision = 0;
    logFont.lfQuality = 2;
    logFont.lfPitchAndFamily = 2;
    UnknownKrustyUIGui* gui = g_UnknownGlobal56e26c->ui->field_0x2c;
    const char* face = gui ? gui->field_0x350 : "";
    if (*face != '\0') {
        logFont.lfWeight = gui->field_0x3d4 ? 700 : 500;
        strcpy(logFont.lfFaceName, face);
    } else {
        logFont.lfWeight = 400;
        strcpy(logFont.lfFaceName, "Lucida Console");
    }
    field_0x120 = CreateFontIndirectA(&logFont);
    if (*face == '\0')
        strcpy(logFont.lfFaceName, "Small Fonts");
    logFont.lfHeight = 10;
    field_0x124 = CreateFontIndirectA(&logFont);
    nameTagRects[0] = UnknownTrackOverlayRect(0, 0x80, 0x89, 0x93);
    nameTagRects[1] = UnknownTrackOverlayRect(0x81, 0xff, 0x89, 0x93);
    nameTagRects[2] = UnknownTrackOverlayRect(0, 0x80, 0x94, 0x9e);
    nameTagRects[3] = UnknownTrackOverlayRect(0x81, 0xff, 0x94, 0x9e);
    nameTagRects[4] = UnknownTrackOverlayRect(0, 0x80, 0x9f, 0xa9);
    nameTagRects[5] = UnknownTrackOverlayRect(0x81, 0xff, 0x9f, 0xa9);
    nameTagRects[6] = UnknownTrackOverlayRect(0, 0x80, 0xaa, 0xb4);
    nameTagRects[7] = UnknownTrackOverlayRect(0x81, 0xff, 0xaa, 0xb4);
    nameTagRects[8] = UnknownTrackOverlayRect(0, 0x80, 0xb5, 0xbf);
    nameTagRects[9] = UnknownTrackOverlayRect(0x81, 0xff, 0xb5, 0xbf);
    nameTagRects[10] = UnknownTrackOverlayRect(0, 0x80, 0xc0, 0xca);
    nameTagRects[11] = UnknownTrackOverlayRect(0x81, 0xff, 0xc0, 0xca);
    nameTagRects[12] = UnknownTrackOverlayRect(0, 0x80, 0xcb, 0xd5);
    return this;
}

// 0x0051bed0: places the name tags and draws the map dots (see the note at
// the top).
void RadarOverlay::DrawRacers(int mode)
{
    int i;
    int x;
    int y;
    int outside;
    Vector3 rim;
    Vector3 point;
    for (i = 0; i < racerCount; i++) {
        if (racers[i] == radarCamera->followedRacer) {
            field_0x17c = racers[i]->field_0x00c;
            field_0x188 = -racers[i]->field_0x050;
            if (racers[i]->field_0x444)
                field_0x188 = field_0x18c;
            else
                field_0x18c = field_0x188;
            field_0x170 = i;
            break;
        }
    }
    field_0x190 = cos(field_0x188);
    field_0x194 = sin(field_0x188);
    for (i = 0; i < racerCount; i++) {
        if (i != field_0x170 && racers[i]->field_0x25_bit0) {
            outside = WorldToMap(racers[i]->field_0x00c, &x, &y, &rim.x, &rim.y);
            if (field_0x174 && field_0x16c == i) {
                if (radarView->field_0x19c) {
                    Vector3& own = racers[field_0x170]->field_0x00c;
                    float dx = racers[i]->field_0x00c.x - own.x;
                    float dz = racers[i]->field_0x00c.z - own.z;
                    float distance = dz * dz + dx * dx;
                    radarView->field_0x19c->UnknownFunction51d9c0(distance, racers[i]->field_0x5e0);
                }
                if (outside && (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 3 ||
                                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 2)) {
                    point.x = -1.0f;
                    point.y = -1.0f;
                } else {
                    point.x = (outside != 0.0f) ? rim.x : (float)x;
                    point.y = (float)(int)(((mapRect.top + outside) ? rim.y : (float)y) - 6.4f);
                }
                for (int k = 0; k < racerCount; k++) {
                    NameOverlay* tag = radarView->field_0x19c->nameTags[k];
                    if (tag->trackedRacer == racers[i])
                        tag->UnknownFunction5190a0(point, x - mapCenterX > 0, outside);
                }
            } else {
                point.x = -1.0f;
                point.y = -1.0f;
                if (radarView->field_0x19c)
                    radarView->field_0x19c->nameTags[i]->UnknownFunction5190a0(point, 0, outside);
            }
            if (outside) {
                if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 3 ||
                    g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 2)
                    continue;
                field_0x1b8.color = 0x28aa28;
                field_0x1d8.color = 0x28aa28;
                field_0x1b8.sx = rim.x;
                field_0x1b8.sy = rim.y;
            } else {
                field_0x1b8.color = 0x50e650;
                field_0x1d8.color = 0x50e650;
                field_0x1b8.sx = (float)x;
                field_0x1b8.sy = (float)y;
            }
            field_0x1d8 = field_0x1b8;
            field_0x1d8.sx += 3.0f;
            Target()->UnknownVirtualSlot16(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&field_0x1b8, 2, 0);
            field_0x1b8.sy += 1.0f;
            field_0x1d8.sy += 1.0f;
            Target()->UnknownVirtualSlot16(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&field_0x1b8, 2, 0);
            field_0x1b8.sy -= 2.0f;
            field_0x1d8.sy -= 2.0f;
            Target()->UnknownVirtualSlot16(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&field_0x1b8, 2, 0);
        } else {
            point.x = -1.0f;
            point.y = -1.0f;
            if (radarView->field_0x19c)
                radarView->field_0x19c->nameTags[i]->UnknownFunction5190a0(point, 0, 1);
        }
    }
    if (mode == 2 || mode == 3) {
        DrawTrackOutline(1);
        DrawTrackOutline(2);
    }
    if (mode == 1 || mode == 5)
        DrawGates();
}
