// Complete nonexact candidate: retail 974 bytes, candidate 974, 966 matching positions.
// Member/field names and original translation-unit ownership remain provisional.
#include <stdio.h>
#include <string.h>
#include "../../src/reconstructed/TrackOverlay.h"
#include "../../src/reconstructed/TrackGame.h"
// Shared mutable DWORD at retail 0x0068a444, defined by TrackOverlay.cpp.
// A preceding internal declaration in the unadmitted aggregate retains its
// existing local view; standalone builds import the canonical storage.
extern int s_UnknownGlobal68a444;
extern "C" __declspec(dllimport) void* __stdcall SelectObject(void*,void*);
extern "C" __declspec(dllimport) int __stdcall DrawTextA(void*,const char*,int,UnknownOverlayRect*,unsigned int);
extern "C" __declspec(dllimport) int __stdcall GetTextExtentPoint32A(void*,const char*,int,UnknownTextExtent*);

// 0x0051e3f0: draw one racer/cue name and update the tag rectangle.
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
