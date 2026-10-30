// Group2_SPRaceInfoDlg.cpp -- link bring-up body for SPRaceInfoDlg::HandleEvent
// (0x004507a0, vtable SPRaceInfoDlg slot 29), referenced by DlgProcs.cpp but
// defined nowhere yet. Best-guess admission (docs/GAME_BUILD.md), not
// byte-matched: extracted from samples/ui/DlgProcsNearMisses.cpp (same calls,
// strings and list columns as retail; register reloads inside the per-racer
// loops differ). The headers are DlgProcs.cpp's, so the class layouts agree.
#include "BikeRaceCore.h"
#include "GUIManagerCore.h"
#include "DlgProcs.h"

#include "DialogEventKind.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BackgroundImage.h"
#include "Camera.h"
#include "D3DIMSoulTree.h"
#include "DebugAlloc.h"
#include "../krusty2/math/FastMath.h"
#include "InGameProcs.h"
#include "MatrixUtil.h"
#include "MenuId.h"
#include "Net.h"
#include "NetProcs.h"
#include "OptionProcs.h"
#include "ProCircuitProcs.h"
#include "RenderTarget.h"
#include "SelectGamePicProcs.h"
#include "TrackGame.h"
#include "EventManager.h"
#include "BaseQuarryEvent.h"
#include "TrackRecordDlg.h"

// A truncating copy into a `size`-byte buffer (as in DlgProcs.cpp).
#define COPY_TEXT(dest, source, size)                              \
    {                                                              \
        int length = strlen(source);                               \
        int copied = length > (size) - 1 ? (size) - 1 : length;    \
        strncpy(dest, source, copied);                             \
        (dest)[copied] = 0;                                        \
    }

// The replay tape's header ("VCRtape.dat", 0xeb0 bytes): the race settings
// at +0x34 and the garage engine size and stroke at +0x33c.
struct UnknownReplayTapeHeader {
    unsigned char field_0x00[0x34];
    UnknownTrackGameModeSettings raceSettings;      // +0x34
    unsigned char field_0x220[0x33c - 0x220];
    int engineSize;
    int fourStroke;
    unsigned char field_0x344[0xeb0 - 0x344];
};

// 0x004507a0: fills the result lists (by event type) when the page opens;
// "ButViewReplay" plays the last race's tape.
void SPRaceInfoDlg::HandleEvent(UnknownDialogEvent* event) {
    char position[128];
    char score[128];
    char points[128];
    char title[128];
    char best[128];
    char suffix[128];
    char label[128];
    char time[128];
    switch (event->kind) {
    case kDialogInit: {
        if (g_TrackGame->ui->replayAvailable)
            FindControl("ButViewReplay", 1)->Show(1, 1);
        else
            FindControl("ButViewReplay", 1)->Show(0, 1);
        if (g_TrackGame->mode.raceSettings.raceMode == 2) {
            UIControl* text = FindControl("TitleText", 0xc);
            g_TrackGame->LoadResourceString(0x93b, position, 0x80);
            sprintf(title, "%s (%d/%d)", position, g_TrackGame->eventManager->racesRun,
                    g_TrackGame->mode.raceSettings.races + 1);
            text->SetText(title);
            if (g_TrackGame->eventManager->racesRun > g_TrackGame->mode.raceSettings.races) {
                text->SetTextFromResource(g_TrackGame->resourceInstance, 0x93a);
                g_TrackGame->mode.championshipInProgress = 0;
                g_TrackGame->ui->currentMenu = 0x10e;
            }
        }
        UIControl* first = FindControl("LstStats1", 0);
        RequestHover(first, 0, 0);
        ClearStatsLists(event);
        int i;
        UIListBox* list;
        switch (g_TrackGame->mode.raceSettings.eventType) {
        case 1:
        case 2:
        case 3:
        case 5: {
            if (g_TrackGame->mode.raceSettings.raceMode == 2)
                SetStatsButtonResources(event, 0x934, 0x935, 0, 0x938);
            else
                SetStatsButtonResources(event, 0x934, 0x935, 0, 0);
            g_TrackGame->LoadResourceString(0x936, label, 0x80);
            g_TrackGame->LoadResourceString(0x13b8, suffix, 0x80);
            sprintf(title, "%s (%s)", label, suffix);
            UIControl* button = FindControl("ButStats3", 0);
            button->SetText(title);
            for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
                sprintf(position, "%d", g_TrackGame->eventManager->results[i].position);
                FormatLapTime(time, g_TrackGame->eventManager->results[i].fastestLap);
                if (g_TrackGame->eventManager->results[i].finished)
                    FormatLapTime(best, g_TrackGame->eventManager->results[i].totalRunningTime);
                else
                    g_TrackGame->LoadResourceString(0xff0, best, 0x80);
                sprintf(score, "%s (%s)", time, best);
                if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                    sprintf(points, "%d", g_TrackGame->eventManager->results[i].points);
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position,
                                 score, points);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position,
                                 score, 0);
                }
            }
            UIListBox* stats = static_cast<UIListBox*>(FindControl("LstStats4", 0));
            static_cast<UIButton*>(FindControl("ButStats4", 1))->SetSortList(stats);
            stats->SetSortCompare(CompareRowNumbersDescending);
            list = static_cast<UIListBox*>(FindControl("LstStats2", 0));
            static_cast<UIButton*>(FindControl("ButStats2", 1))->SetSortList(list);
            list->SetSortCompare(CompareRowNumbersAscending);
            list->Sort(1);
            break;
        }
        case 0: {
            if (g_TrackGame->mode.raceSettings.raceMode == 2)
                SetStatsButtonResources(event, 0x934, 0, 0x937, 0x938);
            else
                SetStatsButtonResources(event, 0x934, 0, 0x937, 0);
            for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
                sprintf(position, "%d", (int)g_TrackGame->eventManager->results[i].score);
                if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                    sprintf(score, "%d", g_TrackGame->eventManager->results[i].points);
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, score);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, 0);
                }
            }
            UIListBox* stats = static_cast<UIListBox*>(FindControl("LstStats4", 0));
            static_cast<UIButton*>(FindControl("ButStats4", 1))->SetSortList(stats);
            stats->SetSortCompare(CompareRowNumbersDescending);
            list = static_cast<UIListBox*>(FindControl("LstStats3", 0));
            list->SetSortCompare(CompareRowNumbersDescending);
            list->Sort(1);
            break;
        }
        }
        break;
    }
    case kDialogCommand:
        if (!_stricmp("ButViewReplay", event->controlName)) {
            UnknownReplayTapeHeader header;
            FILE* file = fopen("VCRtape.dat", "rb");
            if (file) {
                fread(&header, sizeof(header), 1, file);
                fclose(file);
                g_TrackGame->mode.raceSettings = header.raceSettings;
                g_TrackGame->mode.SelectSeries(g_TrackGame->mode.raceSettings.eventType);
                g_TrackGame->playingReplay = 1;
                g_TrackGame->useReplayTape = 1;
                UNKNOWN_GARAGE_SETTINGS->engineSize = header.engineSize;
                UNKNOWN_GARAGE_SETTINGS->fourStroke = header.fourStroke;
                COPY_TEXT(g_TrackGame->mode.replayFile, "VCRtape.dat", 0x104);
                LeaveMenusForRace();
                parentDialog->MarkPendingRelease();
            } else {
                event->control->Show(0, 1);
            }
        }
        break;
    }
}
