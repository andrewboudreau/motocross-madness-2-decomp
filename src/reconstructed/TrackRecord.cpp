#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Track.h"
#include "TrackGame.h"
#include "TrackRecordDlg.h"
#include "DebugAlloc.h"

// 0x0051ee90: an empty name and no value.
UnknownTrackRecord::UnknownTrackRecord()
{
    int n = strlen("");
    int length = n > 15 ? 15 : n;
    strncpy(field_0x00, "", length);
    field_0x00[length] = 0;
    field_0x10 = 0.0f;
}

// 0x0051eed0
int UnknownFunction51eed0(const void* a, const void* b)
{
    if (((UnknownTrackRecord*)a)->field_0x10 < ((UnknownTrackRecord*)b)->field_0x10)
        return -1;
    if (((UnknownTrackRecord*)a)->field_0x10 == ((UnknownTrackRecord*)b)->field_0x10)
        return 0;
    return 1;
}

// 0x0051ef00
int UnknownFunction51ef00(const void* a, const void* b)
{
    if (((UnknownTrackRecord*)a)->field_0x10 < ((UnknownTrackRecord*)b)->field_0x10)
        return 1;
    if (((UnknownTrackRecord*)a)->field_0x10 == ((UnknownTrackRecord*)b)->field_0x10)
        return 0;
    return -1;
}

// 0x0051ef30
UnknownTrackGameObject3400::UnknownTrackGameObject3400()
{
    field_0xdc = new(__FILE__, 47) DirectoryList;
    field_0x00 = 0;
    Clear();
}

// 0x0051efc0
UnknownTrackGameObject3400::~UnknownTrackGameObject3400()
{
    if (field_0xdc) {
        delete field_0xdc;
        field_0xdc = 0;
    }
}

// 0x0051efe0: empties the table and restores the extensions.
void UnknownTrackGameObject3400::Clear()
{
    field_0xe0 = 0;
    for (int i = 0; i < 10; i++)
        field_0x14[i] = UnknownTrackRecord();
    strcpy(field_0x04[0], ".hs1");
    strcpy(field_0x04[1], ".hs2");
    strcpy(field_0x04[2], ".hs3");
    field_0xe4 = 0;
}

// 0x0051f0b0: reads `name` from one of TrackGameMode's directories.
int UnknownTrackGameObject3400::UnknownFunction51f0b0(short directory, const char* name)
{
    char path[260];
    sprintf(path, "%s\\%s", g_TrackGame->mode.field_0xa0[directory], name);
    return Read(path);
}

// 0x0051f110: reads the table from `path` plus the current extension.
int UnknownTrackGameObject3400::Read(const char* path)
{
    char name[260];
    Clear();
    sprintf(name, "%s%s", path, field_0x04[field_0x00]);
    FILE* file = fopen(name, "rb");
    if (!file)
        return 0;
    fread(&field_0xe4, 4, 1, file);
    fread(&field_0xe0, 4, 1, file);
    fread(field_0x14, sizeof(UnknownTrackRecord), 10, file);
    fclose(file);
    return 1;
}

// 0x0051f1b0: writes the table to `path` plus the current extension.
int UnknownTrackGameObject3400::Write(const char* path, int value)
{
    char name[260];
    field_0xe4 = value;
    sprintf(name, "%s%s", path, field_0x04[field_0x00]);
    FILE* file = fopen(name, "wb");
    if (!file)
        return 0;
    fwrite(&field_0xe4, 4, 1, file);
    fwrite(&field_0xe0, 4, 1, file);
    fwrite(field_0x14, sizeof(UnknownTrackRecord), 10, file);
    fclose(file);
    return 1;
}

// 0x0051f260: writes `name` into one of TrackGameMode's directories.
int UnknownTrackGameObject3400::UnknownFunction51f260(short directory, const char* name, int value)
{
    char path[260];
    sprintf(path, "%s\\%s", g_TrackGame->mode.field_0xa0[directory], name);
    return Write(path, value);
}

// 0x0051f2c0: reads `name` when the directory scan lists it; the first
// argument is unused.
void UnknownTrackGameObject3400::UnknownFunction51f2c0(int unused, const char* name)
{
    char path[260];
    char file[260];
    for (int i = 0; i < 10; i++)
        field_0x14[i] = UnknownTrackRecord();
    field_0xdc->UnknownFunction44a1d0((const char*)g_TrackGame->mode.field_0x6a0);
    sprintf(path, "*%s", field_0x04[field_0x00]);
    field_0xdc->UnknownFunction44a220(path, 1);
    field_0xdc->UnknownVirtualSlot1();
    sprintf(path, "%s\\%s", (const char*)g_TrackGame->mode.field_0x6a0, name);
    sprintf(file, "%s%s", name, field_0x04[field_0x00]);
    if (field_0xdc->UnknownFunction44a910(file))
        Read(path);
}

// 0x0051f3c0: enters `racer` as the last record and re-sorts the table;
// 0 when the value would not place. Kinds 0 and 4 rank higher values
// first, the rest lower times first.
int UnknownTrackGameObject3400::AddRacer(short kind, int racer)
{
    char name[16];
    int n = strlen(g_TrackGame->eventManager->field_0x50[racer].field_0x40);
    int length = n > 15 ? 15 : n;
    strncpy(name, g_TrackGame->eventManager->field_0x50[racer].field_0x40, length);
    name[length] = 0;
    float value = 0.0f;
    switch (kind) {
    case 1:
    case 2:
    case 3:
    case 5:
        if (field_0x00 > 0 && field_0x00 <= 2)
            value = g_TrackGame->eventManager->field_0x50[racer].field_0x14;
        else
            value = g_TrackGame->eventManager->field_0x50[racer].field_0x08;
        if (field_0xe0 != 0 && value > field_0x14[field_0xe0 - 1].field_0x10)
            return 0;
        break;
    case 4:
        if (field_0x00 > 0 && field_0x00 <= 2)
            value = g_TrackGame->eventManager->field_0x50[racer].field_0x1c;
        else
            value = g_TrackGame->eventManager->field_0x50[racer].field_0x18;
        if (field_0xe0 != 0 && value < field_0x14[field_0xe0 - 1].field_0x10)
            return 0;
        break;
    case 0:
        if (field_0x00 > 0 && field_0x00 <= 2)
            value = g_TrackGame->eventManager->field_0x50[racer].field_0x2c;
        else
            value = (float)g_TrackGame->eventManager->field_0x50[racer].field_0x0c;
        if (field_0xe0 != 0 && value < field_0x14[field_0xe0 - 1].field_0x10)
            return 0;
        break;
    }
    if (value == 0.0f)
        return 0;
    field_0xe0++;
    if (field_0xe0 >= 10)
        field_0xe0 = 9;
    n = strlen(name);
    length = n > 15 ? 15 : n;
    strncpy(field_0x14[9].field_0x00, name, length);
    field_0x14[9].field_0x00[length] = 0;
    field_0x14[9].field_0x10 = value;
    if (kind != 0 && kind != 4) {
        for (int i = 0; i < 10; i++) {
            if (field_0x14[i].field_0x10 == 0.0f)
                field_0x14[i].field_0x10 = FLT_MAX;
        }
        qsort(field_0x14, 10, sizeof(UnknownTrackRecord), UnknownFunction51eed0);
    } else {
        qsort(field_0x14, 10, sizeof(UnknownTrackRecord), UnknownFunction51ef00);
    }
    return 1;
}

// 0x0051f600: the dialog's messages: 5 sets it up, 1 handles the buttons
// and tabs, 2 a track chosen in the list, 6 frees the rows.
void TrackRecordDlg::UnknownVirtualSlot29(UnknownTrackRecordEvent* event)
{
    char name[128];
    char path[260];
    UIMultiState* control;
    switch (event->kind) {
    case 5:
        field_0x7f60 = 0;
        field_0x7f64 = 0;
        control = static_cast<UIMultiState*>(FindControl("TxtTitle", 0));
        control->SetTextAlign(10);
        control->SetTextFromResource(g_TrackGame->field_0x420, 0x958);
        field_0x7f58 = 0;
        g_TrackGame->mode.UnknownFunction5240e0(0);
        ShowTab(event, field_0x7f58);
        FillLists(event, field_0x7f58);
        field_0x7f5c = 0;
        g_TrackGame->field_0x3400->field_0x00 = 0;
        control = static_cast<UIMultiState*>(FindControl("TabQuarry", 4));
        control->SetStateTextFromResource(0, g_TrackGame->field_0x420, 0x1423);
        control->SetStateTextFromResource(1, g_TrackGame->field_0x420, 0x1423);
        control = static_cast<UIMultiState*>(FindControl("TabTag", 4));
        control->SetStateTextFromResource(0, g_TrackGame->field_0x420, 0x1426);
        control->SetStateTextFromResource(1, g_TrackGame->field_0x420, 0x1426);
        control = static_cast<UIMultiState*>(FindControl("TabSupercross", 4));
        control->SetStateTextFromResource(0, g_TrackGame->field_0x420, 0x1424);
        control->SetStateTextFromResource(1, g_TrackGame->field_0x420, 0x1424);
        control = static_cast<UIMultiState*>(FindControl("TabNationals", 4));
        control->SetStateTextFromResource(0, g_TrackGame->field_0x420, 0x1425);
        control->SetStateTextFromResource(1, g_TrackGame->field_0x420, 0x1425);
        control = static_cast<UIMultiState*>(FindControl("TabBaja", 4));
        control->SetStateTextFromResource(0, g_TrackGame->field_0x420, 0x1421);
        control->SetStateTextFromResource(1, g_TrackGame->field_0x420, 0x1421);
        control = static_cast<UIMultiState*>(FindControl("TabEnduro", 4));
        control->SetStateTextFromResource(0, g_TrackGame->field_0x420, 0x1422);
        control->SetStateTextFromResource(1, g_TrackGame->field_0x420, 0x1422);
        LabelTabs(field_0x7f58);
        break;
    case 1:
        if (!_stricmp("Back", event->controlName)) {
            g_TrackGame->ui->OpenMenu(100);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("TabQuarry", event->controlName)) {
            field_0x7f58 = 0;
            FillLists(event, 0);
        } else if (!_stricmp("TabBaja", event->controlName)) {
            field_0x7f58 = 1;
            FillLists(event, 1);
        } else if (!_stricmp("TabNationals", event->controlName)) {
            field_0x7f58 = 2;
            FillLists(event, 2);
        } else if (!_stricmp("TabSupercross", event->controlName)) {
            field_0x7f58 = 3;
            FillLists(event, 3);
        } else if (!_stricmp("TabEnduro", event->controlName)) {
            field_0x7f58 = 5;
            FillLists(event, 5);
        } else if (!_stricmp("TabTag", event->controlName)) {
            field_0x7f58 = 4;
            FillLists(event, 4);
        } else if (!_stricmp("TabLeft", event->controlName)) {
            field_0x7f5c = 0;
            g_TrackGame->field_0x3400->field_0x00 = 0;
            FillLists(event, field_0x7f58);
        } else if (!_stricmp("TabMiddle", event->controlName)) {
            field_0x7f5c = 1;
            g_TrackGame->field_0x3400->field_0x00 = 1;
            FillLists(event, field_0x7f58);
        } else if (!_stricmp("TabRight", event->controlName)) {
            field_0x7f5c = 2;
            g_TrackGame->field_0x3400->field_0x00 = 2;
            FillLists(event, field_0x7f58);
        } else if (!_stricmp("Help", event->controlName)) {
            g_TrackGame->mode.OpenHelp("MCM2HELP", 0);
        }
        break;
    case 2:
        if (!_stricmp("LstTrack", event->controlName)) {
            g_TrackGame->field_0x3400->Clear();
            if (static_cast<UIListBox*>(event->control)->GetSelectedRow() != -1 && field_0x7f60) {
                int n = strlen(static_cast<UIListBox*>(event->control)->GetRowText(-1));
                int length = n > 0x7f ? 0x7f : n;
                strncpy(name, static_cast<UIListBox*>(event->control)->GetRowText(-1), length);
                name[length] = '\0';
                static_cast<UIListBox*>(FindControl("LstStats1", 3))->RemoveAllRows();
                static_cast<UIListBox*>(FindControl("LstStats2", 3))->RemoveAllRows();
                g_TrackGame->mode.CopySeriesDirectory((short)field_0x7f58, 0, path);
                UnknownFunction5204e0(event, field_0x7f58,
                                      field_0x7f60[static_cast<UIListBox*>(event->control)->GetRowData(-1)]->field_0x00);
            }
        }
        break;
    case 6:
        UnknownFunction51ff40();
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
        tab->SetStateTextFromResource(0, g_TrackGame->field_0x420, 0x1427);
        tab->SetStateTextFromResource(1, g_TrackGame->field_0x420, 0x1427);
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
        tab->SetStateTextFromResource(0, g_TrackGame->field_0x420, id);
        tab->SetStateTextFromResource(1, g_TrackGame->field_0x420, id);
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
    switch (field_0x7f5c) {
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

void TrackRecordDlg::ShowTab(UnknownTrackRecordEvent* event, int series)
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

void TrackRecordDlg::UnknownFunction51ff40()
{
    if (field_0x7f60) {
        for (int i = 0; i < field_0x7f64; i++) {
            DebugFree(field_0x7f60[i]->field_0x00, __FILE__, 467);
            DebugFree(field_0x7f60[i], __FILE__, 468);
        }
        DebugFree(field_0x7f60, __FILE__, 470);
    }
    field_0x7f60 = 0;
    field_0x7f64 = 0;
}

// 0x00520390: selects the series, then fills the track list; `event` is
// unused.
void TrackRecordDlg::FillLists(UnknownTrackRecordEvent* event, int series)
{
    char text[128];
    g_TrackGame->mode.UnknownFunction5240e0(field_0x7f58);
    LabelTabs(field_0x7f58);
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
    UnknownFunction51ffe0(control, g_TrackGame->mode.field_0x25e0, series);
    ((UnknownTrackRecordListBox*)control)->UnknownVirtualSlot66(0);
}

void TrackRecordDlg::UnknownFunction520480(UnknownTrackRecordEvent* event, const char* name, float value,
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

void TrackRecordDlg::UnknownFunction5204e0(UnknownTrackRecordEvent* event, int times, const char* name)
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
    g_TrackGame->field_0x3400->UnknownFunction51f2c0(times, name);
    control = static_cast<UIListBox*>(event->dialog->FindControl("LstStats1", 0));
    control->RemoveAllRows();
    control->SetSelectable(0);
    control = static_cast<UIListBox*>(event->dialog->FindControl("LstStats2", 0));
    control->RemoveAllRows();
    control->SetSelectable(0);
    for (int i = 0; i < 10; i++) {
        UnknownTrackGameObject3400* table = g_TrackGame->field_0x3400;
        if (strcmp(table->field_0x14[i].field_0x00, "") != 0) {
            if (!times)
                sprintf(text, "%.0f", table->field_0x14[i].field_0x10);
            else
                UnknownFunction518690(text, table->field_0x14[i].field_0x10);
            UnknownFunction520480(event, g_TrackGame->field_0x3400->field_0x14[i].field_0x00, 0.0f, text);
        }
    }
}
