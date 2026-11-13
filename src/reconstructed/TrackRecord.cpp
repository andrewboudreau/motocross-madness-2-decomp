#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Track.h"
#include "TrackGame.h"
#include "EventManager.h"
#include "TrackRecordDlg.h"
#include "DebugAlloc.h"
#include "MenuId.h"

// 0x0051ee90: an empty name and no value.
TrackRecordEntry::TrackRecordEntry()
{
    int sourceLength = strlen("");
    int length = sourceLength > 15 ? 15 : sourceLength;
    strncpy(racerName, "", length);
    racerName[length] = 0;
    score = 0.0f;
}

// 0x0051eed0
int CompareRecordsAscending(const void* a, const void* b)
{
    if (((TrackRecordEntry*)a)->score < ((TrackRecordEntry*)b)->score)
        return -1;
    if (((TrackRecordEntry*)a)->score == ((TrackRecordEntry*)b)->score)
        return 0;
    return 1;
}

// 0x0051ef00
int CompareRecordsDescending(const void* a, const void* b)
{
    if (((TrackRecordEntry*)a)->score < ((TrackRecordEntry*)b)->score)
        return 1;
    if (((TrackRecordEntry*)a)->score == ((TrackRecordEntry*)b)->score)
        return 0;
    return -1;
}

// 0x0051ef30
TrackRecord::TrackRecord()
{
    directoryList = new(__FILE__, 47) DirectoryList;
    extensionIndex = 0;
    Clear();
}

// 0x0051efc0
TrackRecord::~TrackRecord()
{
    if (directoryList) {
        delete directoryList;
        directoryList = 0;
    }
}

// 0x0051efe0: empties the table and restores the extensions.
void TrackRecord::Clear()
{
    recordCount = 0;
    for (int i = 0; i < 10; i++)
        records[i] = TrackRecordEntry();
    strcpy(extensions[0], ".hs1");
    strcpy(extensions[1], ".hs2");
    strcpy(extensions[2], ".hs3");
    series = 0;
}

// 0x0051f0b0: reads `name` from one of TrackGameMode's directories.
int TrackRecord::ReadRecord(short directory, const char* name)
{
    char path[260];
    sprintf(path, "%s\\%s", g_TrackGame->mode.seriesDirectories[directory], name);
    return Read(path);
}

// 0x0051f110: reads the table from `path` plus the current extension.
int TrackRecord::Read(const char* path)
{
    char name[260];
    Clear();
    sprintf(name, "%s%s", path, extensions[extensionIndex]);
    FILE* file = fopen(name, "rb");
    if (!file)
        return 0;
    fread(&series, 4, 1, file);
    fread(&recordCount, 4, 1, file);
    fread(records, sizeof(TrackRecordEntry), 10, file);
    fclose(file);
    return 1;
}

// 0x0051f1b0: writes the table to `path` plus the current extension.
int TrackRecord::Write(const char* path, int value)
{
    char name[260];
    series = value;
    sprintf(name, "%s%s", path, extensions[extensionIndex]);
    FILE* file = fopen(name, "wb");
    if (!file)
        return 0;
    fwrite(&series, 4, 1, file);
    fwrite(&recordCount, 4, 1, file);
    fwrite(records, sizeof(TrackRecordEntry), 10, file);
    fclose(file);
    return 1;
}

// 0x0051f260: writes `name` into one of TrackGameMode's directories.
int TrackRecord::WriteRecord(short directory, const char* name, int value)
{
    char path[260];
    sprintf(path, "%s\\%s", g_TrackGame->mode.seriesDirectories[directory], name);
    return Write(path, value);
}

// 0x0051f2c0: reads `name` when the directory scan lists it; the first
// argument is unused.
void TrackRecord::ReadListedRecord(int unused, const char* name)
{
    char path[260];
    char file[260];
    for (int i = 0; i < 10; i++)
        records[i] = TrackRecordEntry();
    directoryList->SetDirectory((const char*)g_TrackGame->mode.currentSeriesDirectory);
    sprintf(path, "*%s", extensions[extensionIndex]);
    directoryList->SetPattern(path, 1);
    directoryList->Scan();
    sprintf(path, "%s\\%s", (const char*)g_TrackGame->mode.currentSeriesDirectory, name);
    sprintf(file, "%s%s", name, extensions[extensionIndex]);
    if (directoryList->Contains(file))
        Read(path);
}

// 0x0051f3c0: enters `racer` as the last record and re-sorts the table;
// 0 when the value would not place. Kinds 0 and 4 rank higher values
// first, the rest lower times first.
int TrackRecord::AddRacer(short kind, int racer)
{
    char name[16];
    int sourceLength = strlen(g_TrackGame->eventManager->results[racer].name);
    int length = sourceLength > 15 ? 15 : sourceLength;
    strncpy(name, g_TrackGame->eventManager->results[racer].name, length);
    name[length] = 0;
    float value = 0.0f;
    switch (kind) {
    case 1:
    case 2:
    case 3:
    case 5:
        if (extensionIndex > 0 && extensionIndex <= 2)
            value = g_TrackGame->eventManager->results[racer].totalRunningTime;
        else
            value = g_TrackGame->eventManager->results[racer].fastestLap;
        if (recordCount != 0 && value > records[recordCount - 1].score)
            return 0;
        break;
    case 4:
        if (extensionIndex > 0 && extensionIndex <= 2)
            value = g_TrackGame->eventManager->results[racer].totalTagRun;
        else
            value = g_TrackGame->eventManager->results[racer].bestTagRun;
        if (recordCount != 0 && value < records[recordCount - 1].score)
            return 0;
        break;
    case 0:
        if (extensionIndex > 0 && extensionIndex <= 2)
            value = g_TrackGame->eventManager->results[racer].score;
        else
            value = (float)g_TrackGame->eventManager->results[racer].largestStuntPoints;
        if (recordCount != 0 && value < records[recordCount - 1].score)
            return 0;
        break;
    }
    if (value == 0.0f)
        return 0;
    recordCount++;
    if (recordCount >= 10)
        recordCount = 9;
    sourceLength = strlen(name);
    length = sourceLength > 15 ? 15 : sourceLength;
    strncpy(records[9].racerName, name, length);
    records[9].racerName[length] = 0;
    records[9].score = value;
    if (kind != 0 && kind != 4) {
        for (int i = 0; i < 10; i++) {
            if (records[i].score == 0.0f)
                records[i].score = FLT_MAX;
        }
        qsort(records, 10, sizeof(TrackRecordEntry), CompareRecordsAscending);
    } else {
        qsort(records, 10, sizeof(TrackRecordEntry), CompareRecordsDescending);
    }
    return 1;
}

// 0x0051f600: the dialog's messages: 5 sets it up, 1 handles the buttons
// and tabs, 2 a track chosen in the list, 6 frees the rows.
void TrackRecordDlg::HandleEvent(DialogEvent* event)
{
    char name[128];
    char path[260];
    UIMultiState* control;
    switch (event->kind) {
    case 5:
        rows = 0;
        rowCount = 0;
        control = static_cast<UIMultiState*>(FindControl("TxtTitle", 0));
        control->SetTextAlign(10);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x958);
        selectedSeries = 0;
        g_TrackGame->mode.SelectSeries(0);
        ShowTab(event, selectedSeries);
        FillLists(event, selectedSeries);
        selectedSubTab = 0;
        g_TrackGame->highScores->extensionIndex = 0;
        control = static_cast<UIMultiState*>(FindControl("TabQuarry", 4));
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1423);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1423);
        control = static_cast<UIMultiState*>(FindControl("TabTag", 4));
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1426);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1426);
        control = static_cast<UIMultiState*>(FindControl("TabSupercross", 4));
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1424);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1424);
        control = static_cast<UIMultiState*>(FindControl("TabNationals", 4));
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1425);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1425);
        control = static_cast<UIMultiState*>(FindControl("TabBaja", 4));
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1421);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1421);
        control = static_cast<UIMultiState*>(FindControl("TabEnduro", 4));
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1422);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1422);
        LabelTabs(selectedSeries);
        break;
    case 1:
        if (!_stricmp("Back", event->controlName)) {
            g_TrackGame->ui->OpenMenu(kMenuMain);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("TabQuarry", event->controlName)) {
            selectedSeries = 0;
            FillLists(event, 0);
        } else if (!_stricmp("TabBaja", event->controlName)) {
            selectedSeries = 1;
            FillLists(event, 1);
        } else if (!_stricmp("TabNationals", event->controlName)) {
            selectedSeries = 2;
            FillLists(event, 2);
        } else if (!_stricmp("TabSupercross", event->controlName)) {
            selectedSeries = 3;
            FillLists(event, 3);
        } else if (!_stricmp("TabEnduro", event->controlName)) {
            selectedSeries = 5;
            FillLists(event, 5);
        } else if (!_stricmp("TabTag", event->controlName)) {
            selectedSeries = 4;
            FillLists(event, 4);
        } else if (!_stricmp("TabLeft", event->controlName)) {
            selectedSubTab = 0;
            g_TrackGame->highScores->extensionIndex = 0;
            FillLists(event, selectedSeries);
        } else if (!_stricmp("TabMiddle", event->controlName)) {
            selectedSubTab = 1;
            g_TrackGame->highScores->extensionIndex = 1;
            FillLists(event, selectedSeries);
        } else if (!_stricmp("TabRight", event->controlName)) {
            selectedSubTab = 2;
            g_TrackGame->highScores->extensionIndex = 2;
            FillLists(event, selectedSeries);
        } else if (!_stricmp("Help", event->controlName)) {
            g_TrackGame->mode.OpenHelp("MCM2HELP", 0);
        }
        break;
    case 2:
        if (!_stricmp("LstTrack", event->controlName)) {
            g_TrackGame->highScores->Clear();
            if (static_cast<UIListBox*>(event->control)->GetSelectedRow() != -1 && rows) {
                int sourceLength = strlen(static_cast<UIListBox*>(event->control)->GetRowText(-1));
                int length = sourceLength > 0x7f ? 0x7f : sourceLength;
                strncpy(name, static_cast<UIListBox*>(event->control)->GetRowText(-1), length);
                name[length] = '\0';
                static_cast<UIListBox*>(FindControl("LstStats1", 3))->RemoveAllRows();
                static_cast<UIListBox*>(FindControl("LstStats2", 3))->RemoveAllRows();
                g_TrackGame->mode.CopySeriesDirectory((short)selectedSeries, 0, path);
                ShowTrackScores(event, selectedSeries,
                                      rows[static_cast<UIListBox*>(event->control)->GetRowData(-1)]->fileName);
            }
        }
        break;
    case 6:
        FreeRows();
        break;
    }
}

void TrackRecordDlg::LabelTabs(int series)
{
    char text[128];
    char format[128];
    UIMultiState* tab;
    if (series != 0 && series != 4) {
        tab = static_cast<UIMultiState*>(FindControl("TabLeft", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1427);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1427);
        tab = static_cast<UIMultiState*>(FindControl("TabMiddle", 4));
        g_TrackGame->LoadResourceString(0x142f, format, 128);
        sprintf(text, format, 5);
        tab->SetStateText(0, text);
        tab->SetStateText(1, text);
        tab = static_cast<UIMultiState*>(FindControl("TabRight", 4));
        sprintf(text, format, 10);
        tab->SetStateText(0, text);
        tab->SetStateText(1, text);
    } else {
        tab = static_cast<UIMultiState*>(FindControl("TabLeft", 4));
        int id = (series == 4) + 0x142a;
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, id);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, id);
        tab = static_cast<UIMultiState*>(FindControl("TabMiddle", 4));
        g_TrackGame->LoadResourceString(0x1430, format, 128);
        sprintf(text, format, 5);
        tab->SetStateText(0, text);
        tab->SetStateText(1, text);
        tab = static_cast<UIMultiState*>(FindControl("TabRight", 4));
        sprintf(text, format, 10);
        tab->SetStateText(0, text);
        tab->SetStateText(1, text);
    }
    switch (selectedSubTab) {
    case 1:
        static_cast<UIRadioButton*>(FindControl("TabMiddle", 4))->SelectInGroup(0);
        break;
    case 2:
        static_cast<UIRadioButton*>(FindControl("TabRight", 4))->SelectInGroup(0);
        break;
    default:
        static_cast<UIRadioButton*>(FindControl("TabLeft", 4))->SelectInGroup(0);
        break;
    }
}

void TrackRecordDlg::ShowTab(DialogEvent* event, int series)
{
    switch (series) {
    case 0:
        static_cast<UIRadioButton*>(FindControl("TabQuarry", 4))->SelectInGroup(0);
        break;
    case 1:
        static_cast<UIRadioButton*>(FindControl("TabBaja", 4))->SelectInGroup(0);
        break;
    case 4:
        static_cast<UIRadioButton*>(FindControl("TabTag", 4))->SelectInGroup(0);
        break;
    case 5:
        static_cast<UIRadioButton*>(FindControl("TabEnduro", 4))->SelectInGroup(0);
        break;
    case 2:
        static_cast<UIRadioButton*>(FindControl("TabNationals", 4))->SelectInGroup(0);
        break;
    case 3:
        static_cast<UIRadioButton*>(FindControl("TabSupercross", 4))->SelectInGroup(0);
        break;
    }
}

void TrackRecordDlg::FreeRows()
{
    if (rows) {
        for (int i = 0; i < rowCount; i++) {
            DebugFree(rows[i]->fileName, __FILE__, 467);
            DebugFree(rows[i], __FILE__, 468);
        }
        DebugFree(rows, __FILE__, 470);
    }
    rows = 0;
    rowCount = 0;
}

// 0x0051ffe0: lists the tracks that have a high-score file. For series 1 and
// 5 the last character of the name is a digit that selects the variant. The
// scan result lives in `more`, which the loop condition reads again; that
// later use is what makes VC6 test it against the ebx zero (`cmp eax, ebx`)
// as retail does, where `if (CurrentName(...))` gives `test eax, eax`.
void TrackRecordDlg::ListHighScoreTracks(UIControl* list, DirectoryList* directory, int series)
{
    char found[128];
    char track[128];
    char display[64];
    char path[256];
    char pattern[260];
    int digit;
    int count = 0;
    int prefixed;
    int nameLength;
    if (series == 5 || series == 1)
        prefixed = 1;
    else
        prefixed = 0;
    FreeRows();
    directory->SetDirectory((const char*)g_TrackGame->mode.currentSeriesDirectory);
    sprintf(pattern, "*%s", g_TrackGame->highScores->extensions[g_TrackGame->highScores->extensionIndex]);
    directory->SetPattern(pattern, 1);
    directory->Scan();
    int more = directory->CurrentName(found);
    if (more) {
        if (prefixed) {
            nameLength = strlen(found) - 1;
            strncpy(track, found, nameLength);
            digit = found[nameLength] - '0';
        } else {
            digit = 0;
            int length = strlen(found);
            nameLength = length > 127 ? 127 : length;
            strncpy(track, found, nameLength);
        }
        track[nameLength] = 0;
    }
    while (more) {
        int directoryIndex = g_TrackGame->mode.GetSelectedSeries();
        sprintf(path, "%s\\%s%s", (const char*)g_TrackGame->mode.currentSeriesDirectory, found,
                g_TrackGame->highScores->extensions[g_TrackGame->highScores->extensionIndex]);
        FILE* file = fopen(path, "r");
        if (file) {
            fread(&directoryIndex, 4, 1, file);
            fclose(file);
        }
        g_TrackGame->mode.FindFileDirectory((int)g_TrackGame->mode.seriesDirectories[directoryIndex],
                                            track, "env", pattern);
        g_TrackGame->sceneObject->SetSceneDirectory(pattern);
        g_TrackGame->sceneObject->ReadSceneName(display, track, digit, "scn", 0, 0);
        static_cast<UIListBox*>(list)->AddRow(display, rowCount, 0);
        count++;
        rows = (UnknownTrackRecordRow**)DebugRealloc(rows, (rowCount + 1) * 4, __FILE__, 534);
        rows[rowCount] = new(__FILE__, 535) UnknownTrackRecordRow;
        rows[rowCount]->fileName = strdup(found);
        rows[rowCount]->field_0x04 = 0;
        rowCount++;
        more = directory->NextName(found);
        if (more) {
            if (prefixed) {
                nameLength = strlen(found) - 1;
                strncpy(track, found, nameLength);
                digit = found[nameLength] - '0';
                track[nameLength] = 0;
            } else {
                digit = 0;
                strcpy(track, found);
            }
        }
    }
    static_cast<UIListBox*>(list)->Sort(1);
    if (count == 0) {
        g_TrackGame->LoadResourceString(0x142e, path, 128);
        static_cast<UIListBox*>(list)->AddRow(path, 0, 0);
        static_cast<UIListBox*>(list)->SetSelectable(0);
    } else {
        static_cast<UIListBox*>(list)->SetSelectable(1);
    }
}

// 0x00520390: selects the series, then fills the track list; `event` is
// unused.
void TrackRecordDlg::FillLists(DialogEvent* event, int series)
{
    char text[128];
    g_TrackGame->mode.SelectSeries(selectedSeries);
    LabelTabs(selectedSeries);
    UIListBox* control = static_cast<UIListBox*>(FindControl("TxtTrack", 12));
    g_TrackGame->LoadResourceString(0xbc6, text, 128);
    control->SetText(text);
    control = static_cast<UIListBox*>(FindControl("LstStats1", 3));
    control->RemoveAllRows();
    control->SetSelectable(0);
    control = static_cast<UIListBox*>(FindControl("LstStats2", 3));
    control->RemoveAllRows();
    control->SetSelectable(0);
    control = static_cast<UIListBox*>(FindControl("LstTrack", 3));
    control->RemoveAllRows();
    ListHighScoreTracks(control, g_TrackGame->mode.highScoreDirectoryList, series);
    ((UnknownTrackRecordListBox*)control)->ReportClick(0);
}

void TrackRecordDlg::AddStatsRow(DialogEvent* event, const char* name, float value,
                                           const char* text)
{
    UIListBox* list;
    if (name) {
        list = static_cast<UIListBox*>(event->dialog->FindControl("LstStats1", 0));
        list->AddRow(name, (int)value, 0);
    }
    if (text) {
        list = static_cast<UIListBox*>(event->dialog->FindControl("LstStats2", 0));
        list->AddRow(text, 0, 0);
    }
}

void TrackRecordDlg::ShowTrackScores(DialogEvent* event, int times, const char* name)
{
    char text[128];
    UIListBox* control = static_cast<UIListBox*>(FindControl("TxtStats1", 12));
    g_TrackGame->LoadResourceString(0xbc4, text, 128);
    control->SetText(text);
    control = static_cast<UIListBox*>(FindControl("TxtStats2", 12));
    if (!times)
        g_TrackGame->LoadResourceString(0x938, text, 128);
    else
        g_TrackGame->LoadResourceString(0xbc5, text, 128);
    control->SetText(text);
    g_TrackGame->highScores->ReadListedRecord(times, name);
    control = static_cast<UIListBox*>(event->dialog->FindControl("LstStats1", 0));
    control->RemoveAllRows();
    control->SetSelectable(0);
    control = static_cast<UIListBox*>(event->dialog->FindControl("LstStats2", 0));
    control->RemoveAllRows();
    control->SetSelectable(0);
    for (int i = 0; i < 10; i++) {
        TrackRecord* table = g_TrackGame->highScores;
        if (strcmp(table->records[i].racerName, "") != 0) {
            if (!times)
                sprintf(text, "%.0f", table->records[i].score);
            else
                FormatLapTime(text, table->records[i].score);
            AddStatsRow(event, g_TrackGame->highScores->records[i].racerName, 0.0f, text);
        }
    }
}
