// Complete nonexact candidate: retail 974 bytes, candidate 974, 966 matching positions.
// Member/field names and original translation-unit ownership remain provisional.
// Left: retail holds the racer in eax and the buffer in ecx in the "%.3f" branch and
// g_TrackGame in edx in the positions block. Session 15, unchanged (966): rect.left =
// rect.left + ..., a per-branch length, a racer local in the "%.3f" branch, a (double)
// cast, `!= 0`, one or two dead int locals in the cue or the else branch (so not a
// round-robin register state); dropping the inner `size` shadow is worse (933).
// s22, also 966: the scale through an inline accessor (pointer, reference or double
// result), a float or double local, `(*racers[index]).`, `(*(racers + index))->`,
// `this->racers`, a `char*` alias of `text`, `&text[0]`, the format in a local and a
// TrackGame local in the positions block; without the position branch's racer local 964.
#include <stdio.h>
#include <string.h>
#include "../../src/reconstructed/TrackOverlay.h"
#include "../../src/reconstructed/TrackGame.h"
// Shared mutable DWORD at retail 0x0068a444, defined by TrackOverlay.cpp.
// A preceding internal declaration in the unadmitted aggregate retains its
// existing local view; standalone builds import the canonical storage.
extern int s_UnknownGlobal68a444;
extern "C" __declspec(dllimport) void* __stdcall SelectObject(void*,void*);
extern "C" __declspec(dllimport) int __stdcall DrawTextA(void*,const char*,int,OverlayRect*,unsigned int);
extern "C" __declspec(dllimport) int __stdcall GetTextExtentPoint32A(void*,const char*,int,Win32Size*);

// 0x0051e3f0: draw one racer/cue name and update the tag rectangle.
// Unchanged at 966/974 (2026-11): inline accessors for s_UnknownGlobal68a444
// (int or bool, `!= 0`), racers[index], the racer scale, nameTagFont,
// racerCount and the showRiderPositions/showRiderNames tests, and the game
// through an inline `Game()`. Folded accessors do not move the register
// naming that is left.
void ChatOverlay::DrawNameTag(void* dc, int index)
{
    char text[0x80];
    char name[0x80];
    int length;
    Win32Size size;
    void* font = SelectObject(dc, nameTagFont);
    if (index == racerCount) {
        sprintf(text, "%s", cueRacerName);
        length = strlen(text);
        OverlayRect rect = nameTagRects[index];
        rect.left += nameTags[index]->drawWidth;
        DrawTextA(dc, text, length, &rect, 0x8124);
        Win32Size size;
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
