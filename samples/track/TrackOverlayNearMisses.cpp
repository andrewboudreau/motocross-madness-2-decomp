// Near-miss TrackOverlay.cpp candidates, kept out of src/reconstructed until
// they match. See docs/TRACKOVERLAY.md. They compile against
// src/reconstructed/TrackOverlay.h.
//
// RadarOverlay::UnknownVirtualSlot23 (0x0051bb60, 246 bytes, 89.69%): the
// zoom keys. Both arms of the field_0x178 test are identical, so retail keeps
// only a dead `test`; it schedules fld/fmul/fidiv before that test and VC6
// here after it. A scale local, a ternary, assigning before the test and an
// empty or return-only test do not reproduce it.

// RadarOverlay::UnknownFunction51c4f0 (0x0051c4f0, 560 bytes, 64%): the
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

// ChatOverlay::UnknownFunction51e3f0 (0x0051e3f0, 974 bytes, 964 match):
// the frame and code match except in the racer-tag branch, where retail
// loads the racer pointer into eax and the buffer address into ecx (and the
// game pointer into edx); VC6 here uses edx, eax and ecx. Inverted
// branches, a racer local, a game local and an explicit double do not
// change it.

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
        field_0x164 = 1 - field_0x164;
        return 1;
    }
    if (UnknownFunction43caa0(0x34, 0, event, 0x80000000)) {
        if (++s_UnknownGlobal57513c > 5)
            s_UnknownGlobal57513c = 5;
        if (field_0x178)
            field_0x198 = field_0x19c / 180.0f / s_UnknownGlobal57513c;
        else
            field_0x198 = field_0x19c / 180.0f / s_UnknownGlobal57513c;
        return 1;
    }
    if (UnknownFunction43caa0(0x33, 0, event, 0x80000000)) {
        if (--s_UnknownGlobal57513c < 1)
            s_UnknownGlobal57513c = 1;
        if (field_0x178)
            field_0x198 = field_0x19c / 180.0f / s_UnknownGlobal57513c;
        else
            field_0x198 = field_0x19c / 180.0f / s_UnknownGlobal57513c;
        return 1;
    }
    return 0;
}

// 0x0051c4f0
int RadarOverlay::UnknownFunction51c4f0(const float* a, const float* b, const float* circle, float* a4, float* point)
{
    float nx;
    float ny;
    float d;
    if (UnknownFunction51c460(a, b, &nx, &ny, &d))
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
    field_0x12c = view;
    while ((racer = field_0x12c->UnknownFunction4204e0(&iterator)) != 0) {
        field_0x16c[field_0x198] = racer;
        field_0x198++;
    }
    for (int i = 0; i < field_0x198; i++) {
        field_0x2d8[i] = (new(__FILE__, 2369) NameOverlay(1))->UnknownFunction518e30(
            Target(), field_0x2c, &field_0x30c[i], 0, &field_0x30c[i], (UnknownEventRacer*)field_0x16c[i],
            (UnknownNameOverlayWorld*)field_0x12c, i);
        UnknownFunction469190(field_0x2d8[i], -1);
    }
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 0) {
        field_0x2d8[field_0x198] = (new(__FILE__, 2376) NameOverlay(1))->UnknownFunction518e30(
            Target(), field_0x2c, &field_0x30c[field_0x198], 0, &field_0x30c[field_0x198], 0,
            (UnknownNameOverlayWorld*)field_0x12c, i);
        UnknownFunction469190(field_0x2d8[field_0x198], -1);
        if (field_0x2d8[field_0x198]) {
            field_0x2d8[field_0x198]->UnknownFunction519080(Vector3(field_0x150, field_0x154, 0));
            field_0x2d8[field_0x198]->UnknownVirtualSlot4();
            field_0x160 = field_0x30c[field_0x198].right;
            field_0x2d8[field_0x198]->field_0x124 = 0;
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
        sprintf(field_0x5b0[0], "%s", label);
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
        sprintf(field_0x5b0[row], "%d) %s : %.0f", shown + 1, scores[shown].racer->field_0x5e0,
                scores[shown].racer->field_0x768);
        shown++;
    }
    if (own >= shown)
        sprintf(field_0x5b0[row], "%d) %s : %.0f", own + 1, scores[own].racer->field_0x5e0,
                scores[own].racer->field_0x768);
    else
        sprintf(field_0x5b0[row], "%d) %s : %.0f", shown + 1, scores[shown].racer->field_0x5e0,
                scores[shown].racer->field_0x768);
    for (i = 0; i <= row; i++) {
        if (strcmp(field_0x5b0[i], field_0x1b0[i]) != 0)
            break;
    }
    if (row < i)
        return 1;
    UnknownFunction4b6710(&field_0xf8);
    PCTextureMap* texture = (PCTextureMap*)field_0x34;
    if (texture->field_0x70->UnknownMethod17(&dc) != 0)
        goto fail;
    SetBkColor(dc, 1);
    SetBkMode(dc, 1);
    SetTextColor(dc, 0xffffff);
    font = SelectObject(dc, field_0x11c);
    for (i = 0; i <= row; i++) {
        if (i == 0)
            GetTextExtentPoint32A(dc, field_0x5b0[0], strlen(field_0x5b0[0]), &field_0x9b0);
        DrawTextA(dc, field_0x5b0[i], strlen(field_0x5b0[i]), &field_0x130[i], 0x120);
        strcpy(field_0x1b0[i], field_0x5b0[i]);
    }
    SelectObject(dc, font);
    if (texture->field_0x70->UnknownMethod26(dc) != 0) {
fail:
        return 0;
    }
    UnknownFunction4b6880(0, 0x80, 0xffff);
    field_0x2c->UnknownVirtualSlot9(0, -1);
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
    field_0x128 = (UnknownInstrumentSource*)camera;
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
    field_0x30 = UnknownFunction50a590(manager, name, 0x115c, 0, 8, 5, 6, 0, 0x10, 0xff00ff, 1, 1);
    if (!field_0x30) {
        Release();
        return 0;
    }
    field_0x2c = field_0x30->UnknownVirtualSlot6();
    if (field_0x2c->field_0x20 == 0x115c)
        field_0x2c->UnknownFunction50abd0(5, 6);
    field_0x2c->UnknownVirtualSlot8(1, 0, 0);
    source.left = 0;
    source.right = 128;
    source.top = 0;
    source.bottom = 128;
    font = "arialsm";
    sprintf(path, "%s\\%s", "Res", "Fonts.res");
    UnknownFunction4b5f50(target, field_0x2c, &rect, 0, &source, 0.00001f, 1, path, &font, 1, 0x115c, 0);
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
    field_0x130[0] = UnknownTrackOverlayRect(14, 128, 20, 30);
    field_0x130[1] = UnknownTrackOverlayRect(14, 128, 36, 46);
    field_0x130[2] = UnknownTrackOverlayRect(14, 128, 53, 63);
    field_0x130[3] = UnknownTrackOverlayRect(14, 128, 69, 79);
    field_0x130[4] = UnknownTrackOverlayRect(14, 128, 87, 97);
    field_0x130[5] = UnknownTrackOverlayRect(14, 100, 105, 115);
    field_0x130[6] = UnknownTrackOverlayRect(5, 86, 34, 49);
    return this;
}

// 0x0051e3f0: draws name tag `index` into `dc`: the name line for the
// player's own tag, otherwise the position and the racer's name (or the
// value at +0x4a4), centred, and sizes the tag to the text.
void ChatOverlay::UnknownFunction51e3f0(void* dc, int index)
{
    char text[0x80];
    char name[0x80];
    int length;
    UnknownTextExtent size;
    void* font = SelectObject(dc, field_0x124);
    if (index == field_0x198) {
        sprintf(text, "%s", field_0x1d4);
        length = strlen(text);
        UnknownOverlayRect rect = field_0x30c[index];
        rect.left += field_0x2d8[index]->field_0x124;
        DrawTextA(dc, text, length, &rect, 0x8124);
        UnknownTextExtent size;
        GetTextExtentPoint32A(dc, text, length, &size);
        field_0x2d8[index]->field_0x124 += size.cx;
        field_0x2d8[index]->field_0xf8 = field_0x30c[index];
        if (field_0x2d8[index]->field_0x124 + 4 < field_0x2d8[index]->field_0x120) {
            field_0x2d8[index]->field_0x124 += 4;
            field_0x2d8[index]->field_0xf8.right = field_0x2d8[index]->field_0x124 + field_0x2d8[index]->field_0xf8.left - 1;
        } else {
            field_0x2d8[index]->field_0x124 = field_0x2d8[index]->field_0x120;
        }
    } else {
        if (s_UnknownGlobal68a444) {
            sprintf(text, "%.3f", field_0x16c[index]->field_0x4a4);
        } else {
            if (g_UnknownGlobal56e26c->mode.field_0x6c0) {
                if (field_0x16c[index]->field_0x784 > 0)
                    sprintf(text, "%d", field_0x16c[index]->field_0x784);
                else
                    strcpy(text, "--");
            } else {
                strcpy(text, "");
            }
            if (g_UnknownGlobal56e26c->mode.field_0x6bc)
                strcpy(name, field_0x16c[index]->field_0x5e0);
            else
                strcpy(name, "");
            if (g_UnknownGlobal56e26c->mode.field_0x6c0 && g_UnknownGlobal56e26c->mode.field_0x6bc)
                strcat(text, " : ");
            strcat(text, name);
        }
        length = strlen(text);
        GetTextExtentPoint32A(dc, text, length, &size);
        field_0x2d8[index]->field_0x124 = size.cx;
        DrawTextA(dc, text, length, &field_0x30c[index], 0x8125);
        field_0x2d8[index]->field_0xf8 = field_0x30c[index];
        if (field_0x2d8[index]->field_0x124 + 4 < field_0x2d8[index]->field_0x120) {
            field_0x2d8[index]->field_0x124 += 4;
            int margin = (field_0x2d8[index]->field_0x120 - field_0x2d8[index]->field_0x124) >> 1;
            field_0x2d8[index]->field_0xf8.left += margin;
            field_0x2d8[index]->field_0xf8.right += 1 - margin;
        } else {
            field_0x2d8[index]->field_0x124 = field_0x2d8[index]->field_0x120;
        }
    }
    SelectObject(dc, font);
}
