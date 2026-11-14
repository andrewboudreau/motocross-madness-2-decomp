// Exact: retail 974 bytes, candidate 974, 974 matching positions.
// Member/field names and original translation-unit ownership remain provisional.
// Two source shapes carry the scratch-register naming (each alone leaves 964-966):
// - the growing arm of the width test ends with its own `SelectObject(dc, font);
//   return;` and the clamp store follows the `if`. VC6 tail-merges the duplicate
//   epilogue, so the branch bytes are the same as with if/else, but the growing
//   arm is no longer the last block before the "%.3f"/positions branch and the
//   scratch rotation enters that branch one register later (racer eax, buffer
//   ecx, game edx);
// - the positions block reads `racers[index]->position` twice instead of through
//   a block-local racer pointer.
// Forms that did not move it while the if/else was in place (966): rect.left =
// rect.left + ..., a per-branch length, a racer local in the "%.3f" branch, a (double)
// cast, `!= 0`, dead int locals in the cue or the else branch, the scale through an
// inline accessor, a float or double local, `(*racers[index]).`, `this->racers`, a
// `char*` alias of `text`, `&text[0]`, the format in a local, a TrackGame local in the
// positions block, a tag or width local in the width test and reordered operands of
// the sourceRect.right sum; dropping the inner `size` shadow is worse (933), swapping
// the arms of the s_UnknownGlobal68a444 test changes the block layout (503).
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
            SelectObject(dc, font);
            return;
        }
        nameTags[index]->drawWidth = nameTags[index]->sourceWidth;
    } else {
        if (s_UnknownGlobal68a444) {
            sprintf(text, "%.3f", racers[index]->driveForceScale);
        } else {
            if (g_TrackGame->mode.showRiderPositions) {
                if (racers[index]->position > 0)
                    sprintf(text, "%d", racers[index]->position);
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
