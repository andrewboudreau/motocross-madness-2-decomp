// Complete retail 0x00519a20..0x00519e09 (1002 bytes).
// Reviewed candidate remains nonexact; original TU ownership is unproven.
// See docs/STATS_STANDINGS_DRAW.md for finite native evidence.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/TrackOverlay.h"
#include "../../src/reconstructed/GUIManagerCore.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/BikeRaceCore.h"
extern "C" __declspec(dllimport) void* __stdcall SelectObject(void* dc, void* object);
extern "C" __declspec(dllimport) unsigned long __stdcall SetBkColor(void* dc, unsigned long color);
extern "C" __declspec(dllimport) int __stdcall SetBkMode(void* dc, int mode);
extern "C" __declspec(dllimport) unsigned long __stdcall SetTextColor(void* dc, unsigned long color);
extern "C" __declspec(dllimport) int __stdcall GetTextExtentPoint32A(void* dc, const char* text, int length,
                                                                     UnknownTextExtent* size);
extern "C" __declspec(dllimport) int __stdcall DrawTextA(void* dc, const char* text, int length,
                                                         UnknownOverlayRect* rect, unsigned int format);

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
    while ((racer = reinterpret_cast<UnknownEventRacer*>(reinterpret_cast<BikeRace*>(view)->NextBike(&iterator))) != 0) {
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
