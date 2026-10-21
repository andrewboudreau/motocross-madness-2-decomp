// Near-miss TrackOverlay.cpp candidates, kept out of src/reconstructed until
// they match. See docs/TRACKOVERLAY.md. They compile against
// src/reconstructed/TrackOverlay.h.
//
// RadarOverlay::IntersectLineCircle (0x0051c4f0, 560 bytes, 334/520 masked):
// the candidate is 552 bytes; `(ex * dx) + ey * dy` and the final
// `... < (by * by) + bx * bx` (best of 3328 parenthesisations of the sums and
// products) gain 13 bytes. The arithmetic and branch structure match; VC6
// here assigns the scale, discriminant and intersection temporaries to
// other frame slots (retail shares one slot for the scale and the
// discriminant and reuses the parameter slots for the direction and x1/x2),
// and computes proj + q with fxch/fadd instead of fld/faddp st(2).
// Declaration order, operand order of the distance sums and the store order
// of the result do not change it.

// ChatOverlay::CollectRacers (0x0051d730, 586 bytes, 44%): the
// candidate is 587 bytes with the same instructions. Retail keeps the zero
// in ebx and the incremented count in ebp; VC6 here swaps them, which
// shifts later register choices. The racer loop written as in RadarOverlay
// 0x0051baf0, a post-increment index, a for loop and every order of the
// three initial assignments do not change it.

// StatsOverlay::DrawStandings (0x00519a20, 1002 bytes): written
// like the exact 0x0051aa40, the candidate is 1004 bytes. Retail keeps the
// row-comparison index in edx and `this` in its spill slot; VC6 here does so
// only when the index is read after the loop (the `row < i` test below),
// which leaves a redundant compare on the mismatch path and swaps edi and
// ebp for `this` and the view. A goto out of the loop, a flag, drawing
// inside the loop and separate loop variables spill the index instead.

// ChatOverlay::DrawNameTag (0x0051e3f0, 974 bytes, 966 match):
// the frame and code match except in the racer-tag branch. A racer local in
// the position branch gives retail's eax there; in the "%.3f" branch retail
// loads the racer into eax and the buffer address into ecx (VC6 here edx and
// eax), and loads the game pointer into edx (here ecx). Inverted branches, a
// racer local in that branch, a game local, a float or double local and a
// buffer pointer do not change it; nor do an inline float or double
// accessor for +0x4a4, an explicit (double) cast or `&text[0]`. Also tried:
// no inner shadowing `size` (853), a racer local at the top of the else
// (worse), a racer local in the "%.3f" branch only, `!= 0` tests, no racer
// local in the 0x6c0 branch (884) and unbraced if/else (unchanged).
// Session 14: all 24 declaration orders of text, name, length and size
// keep 966.

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

// ChatOverlay::Setup (0x0051cf80, 1953 bytes; candidate 1953, 1768 of the
// 1785 non-relocation bytes): the loader. Frame layout, calls, the font
// set-up and the rectangles match once the side-panel rectangle is declared
// inside the wide-screen block. The left panel's source rectangle is written
// before its screen rectangle (left edge first, from chatRect), and the
// LOGFONT stores lfItalic and lfHeight last, as in StatsOverlay::LoadPanel.
// Left: in the right panel's block retail loads largeLayout into eax and
// forms the panel width in ecx; VC6 here swaps the two registers. All 720
// orders of that block's six statements, the four spellings of each
// ternary, a width local, `rect.right = rect.left + ...` and
// `rect.left = rect.right + 256` leave the swap (or are worse).
// Inline width/bottom helpers (by value or const&, in any of 15
// placements) compile identically.


// RadarOverlay::DrawRacers (0x0051bed0, 1160 bytes; 1129 match):
// the map update. Every call, loop and store matches with the racer read
// through `field_0x134[i]` each time, the own position through a reference,
// the rim point as a Vector3 (its third slot is unused) and the chosen
// screen point as the ternaries below. Left: retail allocates the
// int-to-float slot of `outside` right above `x` (here it is the last
// scalar slot, so y and the loop temporaries shift by one), and schedules
// the distance products before the name address and the pushes of the
// 0x0051d9c0 call. Declaration order and scope, an int copy, a float copy
// and a distance local do not move either; nor do the distance written
// inline in the call, a name or overlay local, or the sum written x first.
// Bracketing one product (`dz * dz + (dx * dx)`) moves the products ahead
// of the name address and the pushes (1143, one hunk) but computes dx*dx
// first where retail does dz*dz first, and aligns one instruction fewer;
// not kept. Square helpers, casts and other bracket placements do not move.

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
#include "../../src/reconstructed/EventManager.h"
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
    float root = FastSqrt(scale);
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
    float proj = (ex * dx) + ey * dy;
    if (disc < 1e-14f) {
        float k = proj / lengthSquared;
        point[0] = k * dx + px;
        point[1] = k * dy + py;
        return 1;
    }
    float q = FastSqrt(disc);
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
    if (ax * ax + ay * ay < (by * by) + bx * bx) {
        point[0] = x1;
        point[1] = y1;
    } else {
        point[0] = x2;
        point[1] = y2;
    }
    return 2;
}

// 0x0051d730
int ChatOverlay::CollectRacers(UnknownChatView* view)
{
    int iterator;
    UnknownChatRacer* racer;
    racerCount = 0;
    iterator = 0;
    chatView = view;
    while ((racer = chatView->NextBike(&iterator)) != 0) {
        racers[racerCount] = racer;
        racerCount++;
    }
    for (int i = 0; i < racerCount; i++) {
        nameTags[i] = (new(__FILE__, 2369) NameOverlay(1))->Create(
            Target(), overlayTexture, &nameTagRects[i], 0, &nameTagRects[i], (UnknownEventRacer*)racers[i],
            (UnknownNameOverlayWorld*)chatView, i);
        AppendChild(nameTags[i], -1);
    }
    if (g_TrackGame->mode.raceSettings.eventType == 0) {
        nameTags[racerCount] = (new(__FILE__, 2376) NameOverlay(1))->Create(
            Target(), overlayTexture, &nameTagRects[racerCount], 0, &nameTagRects[racerCount], 0,
            (UnknownNameOverlayWorld*)chatView, i);
        AppendChild(nameTags[racerCount], -1);
        if (nameTags[racerCount]) {
            nameTags[racerCount]->SetFixedPosition(Vector3(cueRect.left, cueRect.top, 0));
            nameTags[racerCount]->Disable();
            nameLineRight = nameTagRects[racerCount].right;
            nameTags[racerCount]->drawWidth = 0;
        }
    }
    RedrawNameTags(-1);
    return 1;
}

// 0x00519a20: the standings, best score first: the time left (as a label),
// then a row per racer that is shown, ending with the player's own row.
// Draws only when a row changed.
int StatsOverlay::DrawStandings()
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
    UnknownKrustyBikeView* view = g_TrackGame->quarryEvent->raceView;
    if (g_TrackGame->mode.raceSettings.raceMode != 0 && g_TrackGame->mode.raceSettings.raceMode != 4) {
        g_TrackGame->LoadResourceString(0x943, label, 0x80);
        TrackGameViewOwner* owner = g_TrackGame->quarryEvent;
        float limit = g_TrackGame->mode.raceSettings.timeLimitMinutes * 60.0f;
        FormatMinutesSeconds(time, limit - (owner->elapsedMinutes * 60.0f + owner->elapsedSeconds));
        sprintf(rowText[0], "%s", label);
    }
    iterator = 0;
    UnknownEventRacer* racer;
    while ((racer = view->NextBike(&iterator)) != 0) {
        if (racer->isEnabled) {
            scores[count].value = racer->score;
            scores[count].racer = racer;
            count++;
        }
    }
    qsort(scores, count, sizeof(UnknownEventScore), CompareScoresDescending);
    for (i = 0; i < count; i++) {
        if (scores[i].racer == view->localRacer) {
            own = i;
            break;
        }
    }
    int first = 1;
    if (g_TrackGame->mode.raceSettings.raceMode != 0 && g_TrackGame->mode.raceSettings.raceMode != 4)
        count++;
    else
        first = 0;
    if (count >= 6)
        count = 6;
    int row;
    int shown = 0;
    for (row = first; row < count - 1; row++) {
        sprintf(rowText[row], "%d) %s : %.0f", shown + 1, scores[shown].racer->racerName,
                scores[shown].racer->score);
        shown++;
    }
    if (own >= shown)
        sprintf(rowText[row], "%d) %s : %.0f", own + 1, scores[own].racer->racerName,
                scores[own].racer->score);
    else
        sprintf(rowText[row], "%d) %s : %.0f", shown + 1, scores[shown].racer->racerName,
                scores[shown].racer->score);
    for (i = 0; i <= row; i++) {
        if (strcmp(rowText[i], drawnRowText[i]) != 0)
            break;
    }
    if (row < i)
        return 1;
    RestoreRect(&sourceRect);
    PCTextureMap* texture = (PCTextureMap*)sharedTexture;
    if (texture->systemSurface->GetDC(&dc) != 0)
        goto fail;
    SetBkColor(dc, 1);
    SetBkMode(dc, 1);
    SetTextColor(dc, 0xffffff);
    font = SelectObject(dc, largeFont);
    for (i = 0; i <= row; i++) {
        if (i == 0)
            GetTextExtentPoint32A(dc, rowText[0], strlen(rowText[0]), &rowExtent);
        DrawTextA(dc, rowText[i], strlen(rowText[i]), &rowRects[i], 0x120);
        strcpy(drawnRowText[i], rowText[i]);
    }
    SelectObject(dc, font);
    if (texture->systemSurface->ReleaseDC(dc) != 0) {
fail:
        return 0;
    }
    TintRows(0, 0x80, 0xffff);
    overlayTexture->Upload(0, -1);
    return 1;
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
    void* font = SelectObject(dc, nameTagFont);
    if (index == racerCount) {
        sprintf(text, "%s", cueRacerName);
        length = strlen(text);
        UnknownOverlayRect rect = nameTagRects[index];
        rect.left += nameTags[index]->drawWidth;
        DrawTextA(dc, text, length, &rect, 0x8124);
        UnknownTextExtent size;
        GetTextExtentPoint32A(dc, text, length, &size);
        nameTags[index]->drawWidth += size.cx;
        nameTags[index]->sourceRect = nameTagRects[index];
        if (nameTags[index]->drawWidth + 4 < nameTags[index]->sourceWidth) {
            nameTags[index]->drawWidth += 4;
            nameTags[index]->sourceRect.right = nameTags[index]->drawWidth + nameTags[index]->sourceRect.left - 1;
        } else {
            nameTags[index]->drawWidth = nameTags[index]->sourceWidth;
        }
    } else {
        if (s_UnknownGlobal68a444) {
            sprintf(text, "%.3f", racers[index]->driveForceScale);
        } else {
            if (g_TrackGame->mode.showRiderPositions) {
                UnknownChatRacer* racer = racers[index];
                if (racer->position > 0)
                    sprintf(text, "%d", racer->position);
                else
                    strcpy(text, "--");
            } else {
                strcpy(text, "");
            }
            if (g_TrackGame->mode.showRiderNames)
                strcpy(name, racers[index]->name);
            else
                strcpy(name, "");
            if (g_TrackGame->mode.showRiderPositions && g_TrackGame->mode.showRiderNames)
                strcat(text, " : ");
            strcat(text, name);
        }
        length = strlen(text);
        GetTextExtentPoint32A(dc, text, length, &size);
        nameTags[index]->drawWidth = size.cx;
        DrawTextA(dc, text, length, &nameTagRects[index], 0x8125);
        nameTags[index]->sourceRect = nameTagRects[index];
        if (nameTags[index]->drawWidth + 4 < nameTags[index]->sourceWidth) {
            nameTags[index]->drawWidth += 4;
            int margin = (nameTags[index]->sourceWidth - nameTags[index]->drawWidth) >> 1;
            nameTags[index]->sourceRect.left += margin;
            nameTags[index]->sourceRect.right += 1 - margin;
        } else {
            nameTags[index]->drawWidth = nameTags[index]->sourceWidth;
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
    item->node = ((UnknownRadarView*)radarView)->field_0x48->startNode;
    item->next = list;
    list = item;
    while (list) {
        TrackNode*& node = list->node;
        if (node->flags & 4) {
            node->flags &= ~4;
            item = list;
            list = list->next;
            DebugFree(item, __FILE__, 2001);
        } else {
        node->flags |= 4;
        TrackSegment* first = node->firstSegment;
        Vector3 point;
        if (mode == 1)
            point = *(Vector3*)&first->edgeBX;
        else if (mode == 2)
            point = *(Vector3*)&first->edgeAX;
        int startX;
        int startY;
        float rim[2];
        int outside = WorldToMap(point, &startX, &startY, &rim[0], &rim[1]);
        lineStart.sx = (float)startX;
        lineStart.sy = (float)startY;
        lineStart.color = 0xedea5e;
        lineEnd.color = 0xedea5e;
        int x = startX;
        int y = startY;
        TrackSegment* segment = first->next;
        if (segment) do {
            if (mode == 1)
                point = *(Vector3*)&segment->edgeBX;
            else if (mode == 2)
                point = *(Vector3*)&segment->edgeAX;
            int wasOutside = outside;
            float from[2];
            from[0] = (float)x;
            from[1] = (float)y;
            outside = WorldToMap(point, &x, &y, &rim[0], &rim[1]);
            if (outside) {
                if (wasOutside) {
                    lineStart.sx = (float)x;
                    lineStart.sy = (float)y;
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
                        lineEnd.sx = hit[0];
                        lineEnd.sy = hit[1];
                        Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
                        lineStart = lineEnd;
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
                    lineStart.sy = hit[1];
                    lineEnd.sy = (float)y;
                    lineEnd.sx = (float)x;
                    lineStart.sx = hit[0];
                    Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
                    lineStart = lineEnd;
                }
            } else {
                lineEnd.sx = (float)x;
                lineEnd.sy = (float)y;
                Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
                lineStart = lineEnd;
            }
            segment = segment->next;
            if (segment == node->firstSegment) {
                lineEnd.sx = (float)startX;
                lineEnd.sy = (float)startY;
                break;
            }
        } while (segment);
        for (int i = 0; i < node->linkCount; i++) {
            if (!(node->links[i]->flags & 4)) {
                item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 2094);
                item->node = node->links[i];
                item->next = list;
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
ChatOverlay* ChatOverlay::Setup(RenderTarget* target, TextureMapManager* manager, void* camera,
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
    backingTexture = TerrainLoadTexture(manager, name, 4444, 0, 8, 5, 6, 0, 0x10, 0xff00ff, 1, 1);
    if (!backingTexture) {
        Release();
        return 0;
    }
    overlayTexture = backingTexture->Clone();
    if (overlayTexture->mapFormat == 4444)
        overlayTexture->SetAddressModes(5, 6);
    overlayTexture->CreateTextureSurface(1, 0, 0);
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
        source.right = largeLayout ? 38 : 19;
        source.bottom = largeLayout ? 137 : 103;
        rect.left = chatRect.left - (largeLayout ? 38 : 19);
        rect.right = chatRect.left;
        leftPanel = new (__FILE__, 2284) Overlay(0, 1);
        leftPanel->Attach(target, overlayTexture, &rect, 0, &source, 0.00001f, 0, 0, 0, 0, 1555, 0);
        AppendChild(leftPanel, -1);
        source.left += largeLayout ? 38 : 19;
        source.top = 69;
        source.right += largeLayout ? 38 : 19;
        source.bottom = largeLayout ? 137 : 103;
        rect.left = chatRect.right;
        rect.right = chatRect.right + (largeLayout ? 38 : 19);
        rightPanel = new (__FILE__, 2294) Overlay(0, 1);
        rightPanel->Attach(target, overlayTexture, &rect, 0, &source, 0.00001f, 0, 0, 0, 0, 1555, 0);
        AppendChild(rightPanel, -1);
    }
    pen = CreatePen(0, 3, 0xc0c0c0);
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfCharSet = 1;
    logFont.lfOutPrecision = 0;
    logFont.lfClipPrecision = 0;
    logFont.lfQuality = 2;
    logFont.lfPitchAndFamily = 2;
    logFont.lfItalic = 0;
    logFont.lfHeight = 9;
    UnknownKrustyUIGui* gui = g_TrackGame->ui->gui;
    const char* face = gui ? gui->toolTipFontFace : "";
    if (*face != '\0') {
        logFont.lfWeight = gui->toolTipBold ? 700 : 500;
        strcpy(logFont.lfFaceName, face);
    } else {
        logFont.lfWeight = 400;
        strcpy(logFont.lfFaceName, "Lucida Console");
    }
    chatFont = CreateFontIndirectA(&logFont);
    if (*face == '\0')
        strcpy(logFont.lfFaceName, "Small Fonts");
    logFont.lfHeight = 10;
    nameTagFont = CreateFontIndirectA(&logFont);
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
            mapOrigin = racers[i]->position;
            mapAngle = -racers[i]->heading;
            if (racers[i]->crashState)
                mapAngle = heldMapAngle;
            else
                heldMapAngle = mapAngle;
            followedIndex = i;
            break;
        }
    }
    mapAngleCos = cos(mapAngle);
    mapAngleSin = sin(mapAngle);
    for (i = 0; i < racerCount; i++) {
        if (i != followedIndex && racers[i]->isEnabled) {
            outside = WorldToMap(racers[i]->position, &x, &y, &rim.x, &rim.y);
            if (cueShown && cueIndex == i) {
                if (radarView->chatOverlay) {
                    Vector3& own = racers[followedIndex]->position;
                    float dx = racers[i]->position.x - own.x;
                    float dz = racers[i]->position.z - own.z;
                    float distance = dz * dz + dx * dx;
                    radarView->chatOverlay->SetCueRacer(distance, racers[i]->racerName);
                }
                if (outside && (g_TrackGame->mode.raceSettings.eventType == 3 ||
                                g_TrackGame->mode.raceSettings.eventType == 2)) {
                    point.x = -1.0f;
                    point.y = -1.0f;
                } else {
                    point.x = (outside != 0.0f) ? rim.x : (float)x;
                    point.y = (float)(int)(((mapRect.top + outside) ? rim.y : (float)y) - 6.4f);
                }
                for (int k = 0; k < racerCount; k++) {
                    NameOverlay* tag = radarView->chatOverlay->nameTags[k];
                    if (tag->trackedRacer == racers[i])
                        tag->SetRadarAnchor(point, x - mapCenterX > 0, outside);
                }
            } else {
                point.x = -1.0f;
                point.y = -1.0f;
                if (radarView->chatOverlay)
                    radarView->chatOverlay->nameTags[i]->SetRadarAnchor(point, 0, outside);
            }
            if (outside) {
                if (g_TrackGame->mode.raceSettings.eventType == 3 ||
                    g_TrackGame->mode.raceSettings.eventType == 2)
                    continue;
                lineStart.color = 0x28aa28;
                lineEnd.color = 0x28aa28;
                lineStart.sx = rim.x;
                lineStart.sy = rim.y;
            } else {
                lineStart.color = 0x50e650;
                lineEnd.color = 0x50e650;
                lineStart.sx = (float)x;
                lineStart.sy = (float)y;
            }
            lineEnd = lineStart;
            lineEnd.sx += 3.0f;
            Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
            lineStart.sy += 1.0f;
            lineEnd.sy += 1.0f;
            Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
            lineStart.sy -= 2.0f;
            lineEnd.sy -= 2.0f;
            Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
        } else {
            point.x = -1.0f;
            point.y = -1.0f;
            if (radarView->chatOverlay)
                radarView->chatOverlay->nameTags[i]->SetRadarAnchor(point, 0, 1);
        }
    }
    if (mode == 2 || mode == 3) {
        DrawTrackOutline(1);
        DrawTrackOutline(2);
    }
    if (mode == 1 || mode == 5)
        DrawGates();
}
