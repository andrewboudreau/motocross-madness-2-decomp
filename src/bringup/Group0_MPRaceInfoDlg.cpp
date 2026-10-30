// Group0_MPRaceInfoDlg.cpp -- bring-up body for MPRaceInfoDlg::HandleEvent
// (UIDialog slot 28 override), extracted from samples/ui/DlgProcsNearMisses.cpp
// (that sample reports the same calls, strings and list columns as retail;
// only register reloads inside the per-racer loops differ). Only this
// function is taken from the sample; the sample itself includes the whole
// DlgProcs.cpp, so the includes here are that file's own.
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
#include "Track.h"

// 0x004513a0: fills the result lists (by event type) when the page opens.
void MPRaceInfoDlg::HandleEvent(UnknownDialogEvent* event) {
    char position[128];
    char score[128];
    char points[128];
    char title[128];
    char best[128];
    char suffix[128];
    char label[128];
    char time[128];
    if (event->kind != kDialogInit)
        return;
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
            if (!g_TrackGame->eventManager->results[i].outOfRace) {
                sprintf(position, "%d", g_TrackGame->eventManager->results[i].position);
                FormatLapTime(time, g_TrackGame->eventManager->results[i].fastestLap);
                if (g_TrackGame->eventManager->results[i].finished)
                    FormatLapTime(best, g_TrackGame->eventManager->results[i].totalRunningTime);
                else
                    g_TrackGame->LoadResourceString(0xff0, best, 0x80);
                sprintf(score, "%s (%s)", time, best);
                if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                    sprintf(points, "%d", g_TrackGame->eventManager->results[i].points);
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position, score,
                                 points);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position, score, 0);
                }
            }
        }
        UIListBox* stats = static_cast<UIListBox*>(FindControl("LstStats4", 0));
        static_cast<UIButton*>(FindControl("ButStats4", 1))->SetSortList(stats);
        stats->SetSortCompare(CompareRowNumbersDescending);
        list = static_cast<UIListBox*>(FindControl("LstStats2", 0));
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
            if (!g_TrackGame->eventManager->results[i].outOfRace) {
                sprintf(position, "%d", (int)g_TrackGame->eventManager->results[i].score);
                if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                    sprintf(score, "%d", g_TrackGame->eventManager->results[i].points);
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, score);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, 0);
                }
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
    case 4:
        if (g_TrackGame->mode.raceSettings.stuntMode) {
            if (g_TrackGame->mode.raceSettings.raceMode == 2)
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0x938);
        else
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0);
            for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
                if (!g_TrackGame->eventManager->results[i].outOfRace) {
                    sprintf(position, "%d", (int)g_TrackGame->eventManager->results[i].score);
                    if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                        sprintf(score, "%d", g_TrackGame->eventManager->results[i].points);
                        AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, score);
                    } else {
                        AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, 0);
                    }
                }
            }
            UIListBox* stats = static_cast<UIListBox*>(FindControl("LstStats4", 0));
            static_cast<UIButton*>(FindControl("ButStats4", 1))->SetSortList(stats);
            stats->SetSortCompare(CompareRowNumbersDescending);
            list = static_cast<UIListBox*>(FindControl("LstStats3", 0));
            list->SetSortCompare(CompareRowNumbersDescending);
            list->Sort(1);
        } else {
            if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                SetStatsButtonResources(event, 0x934, 0x939, 0x938, 0);
            } else {
                g_TrackGame->LoadResourceString(0x934, position, 0x80);
                g_TrackGame->LoadResourceString(0x935, score, 0x80);
                g_TrackGame->LoadResourceString(0x939, points, 0x80);
                g_TrackGame->LoadResourceString(0x142b, title, 0x80);
                SetStatsButtonTexts(event, position, score, points, title);
            }
            for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
                if (!g_TrackGame->eventManager->results[i].outOfRace) {
                    sprintf(position, "%d", g_TrackGame->eventManager->results[i].position);
                    FormatLapTime(score, g_TrackGame->eventManager->results[i].totalTagRun);
                    FormatLapTime(points, g_TrackGame->eventManager->results[i].bestTagRun);
                    if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                        sprintf(points, "%d", g_TrackGame->eventManager->results[i].points);
                        AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position, score,
                                     points);
                    } else {
                        AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position, score,
                                     points);
                    }
                }
            }
            list = static_cast<UIListBox*>(FindControl("LstStats2", 0));
            list->SetSortCompare(CompareRowNumbersAscending);
            list->Sort(1);
        }
        break;
    }
}
