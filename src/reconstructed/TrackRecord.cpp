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
    UnknownFunction51efe0();
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
void UnknownTrackGameObject3400::UnknownFunction51efe0()
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
    sprintf(path, "%s\\%s", g_UnknownGlobal56e26c->mode.field_0xa0[directory], name);
    return UnknownFunction51f110(path);
}

// 0x0051f110: reads the table from `path` plus the current extension.
int UnknownTrackGameObject3400::UnknownFunction51f110(const char* path)
{
    char name[260];
    UnknownFunction51efe0();
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
int UnknownTrackGameObject3400::UnknownFunction51f1b0(const char* path, int value)
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
    sprintf(path, "%s\\%s", g_UnknownGlobal56e26c->mode.field_0xa0[directory], name);
    return UnknownFunction51f1b0(path, value);
}

// 0x0051f2c0: reads `name` when the directory scan lists it; the first
// argument is unused.
void UnknownTrackGameObject3400::UnknownFunction51f2c0(int unused, const char* name)
{
    char path[260];
    char file[260];
    for (int i = 0; i < 10; i++)
        field_0x14[i] = UnknownTrackRecord();
    field_0xdc->UnknownFunction44a1d0((const char*)g_UnknownGlobal56e26c->mode.field_0x6a0);
    sprintf(path, "*%s", field_0x04[field_0x00]);
    field_0xdc->UnknownFunction44a220(path, 1);
    field_0xdc->UnknownVirtualSlot1();
    sprintf(path, "%s\\%s", (const char*)g_UnknownGlobal56e26c->mode.field_0x6a0, name);
    sprintf(file, "%s%s", name, field_0x04[field_0x00]);
    if (field_0xdc->UnknownFunction44a910(file))
        UnknownFunction51f110(path);
}

// 0x0051f3c0: enters `racer` as the last record and re-sorts the table;
// 0 when the value would not place. Kinds 0 and 4 rank higher values
// first, the rest lower times first.
int UnknownTrackGameObject3400::UnknownFunction51f3c0(short kind, int racer)
{
    char name[16];
    int n = strlen(g_UnknownGlobal56e26c->eventManager->field_0x50[racer].field_0x40);
    int length = n > 15 ? 15 : n;
    strncpy(name, g_UnknownGlobal56e26c->eventManager->field_0x50[racer].field_0x40, length);
    name[length] = 0;
    float value = 0.0f;
    switch (kind) {
    case 1:
    case 2:
    case 3:
    case 5:
        if (field_0x00 > 0 && field_0x00 <= 2)
            value = g_UnknownGlobal56e26c->eventManager->field_0x50[racer].field_0x14;
        else
            value = g_UnknownGlobal56e26c->eventManager->field_0x50[racer].field_0x08;
        if (field_0xe0 != 0 && value > field_0x14[field_0xe0 - 1].field_0x10)
            return 0;
        break;
    case 4:
        if (field_0x00 > 0 && field_0x00 <= 2)
            value = g_UnknownGlobal56e26c->eventManager->field_0x50[racer].field_0x1c;
        else
            value = g_UnknownGlobal56e26c->eventManager->field_0x50[racer].field_0x18;
        if (field_0xe0 != 0 && value < field_0x14[field_0xe0 - 1].field_0x10)
            return 0;
        break;
    case 0:
        if (field_0x00 > 0 && field_0x00 <= 2)
            value = g_UnknownGlobal56e26c->eventManager->field_0x50[racer].field_0x2c;
        else
            value = (float)g_UnknownGlobal56e26c->eventManager->field_0x50[racer].field_0x0c;
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

void TrackRecordDlg::UnknownFunction51fc40(int series)
{
    char text[128];
    char format[128];
    UnknownGameUiControl* tab;
    if (series != 0 && series != 4) {
        tab = UnknownFunction46ebf0("TabLeft", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x1427);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x1427);
        tab = UnknownFunction46ebf0("TabMiddle", 4);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x142f, format, 128);
        sprintf(text, format, 5);
        tab->UnknownFunction478ad0(0, text);
        tab->UnknownFunction478ad0(1, text);
        tab = UnknownFunction46ebf0("TabRight", 4);
        sprintf(text, format, 10);
        tab->UnknownFunction478ad0(0, text);
        tab->UnknownFunction478ad0(1, text);
    } else {
        tab = UnknownFunction46ebf0("TabLeft", 4);
        int id = (series == 4) + 0x142a;
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, id);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, id);
        tab = UnknownFunction46ebf0("TabMiddle", 4);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x1430, format, 128);
        sprintf(text, format, 5);
        tab->UnknownFunction478ad0(0, text);
        tab->UnknownFunction478ad0(1, text);
        tab = UnknownFunction46ebf0("TabRight", 4);
        sprintf(text, format, 10);
        tab->UnknownFunction478ad0(0, text);
        tab->UnknownFunction478ad0(1, text);
    }
    switch (field_0x7f5c) {
    case 1:
        UnknownFunction46ebf0("TabMiddle", 4)->UnknownFunction479310(0);
        break;
    case 2:
        UnknownFunction46ebf0("TabRight", 4)->UnknownFunction479310(0);
        break;
    default:
        UnknownFunction46ebf0("TabLeft", 4)->UnknownFunction479310(0);
        break;
    }
}

void TrackRecordDlg::UnknownFunction51fe80(UnknownTrackRecordEvent* event, int series)
{
    switch (series) {
    case 0:
        UnknownFunction46ebf0("TabQuarry", 4)->UnknownFunction479310(0);
        break;
    case 1:
        UnknownFunction46ebf0("TabBaja", 4)->UnknownFunction479310(0);
        break;
    case 4:
        UnknownFunction46ebf0("TabTag", 4)->UnknownFunction479310(0);
        break;
    case 5:
        UnknownFunction46ebf0("TabEnduro", 4)->UnknownFunction479310(0);
        break;
    case 2:
        UnknownFunction46ebf0("TabNationals", 4)->UnknownFunction479310(0);
        break;
    case 3:
        UnknownFunction46ebf0("TabSupercross", 4)->UnknownFunction479310(0);
        break;
    }
}

void TrackRecordDlg::UnknownFunction51ff40()
{
    if (field_0x7f60) {
        for (int i = 0; i < field_0x7f64; i++) {
            operator delete(field_0x7f60[i]->field_0x00, __FILE__, 467);
            operator delete(field_0x7f60[i], __FILE__, 468);
        }
        operator delete(field_0x7f60, __FILE__, 470);
    }
    field_0x7f60 = 0;
    field_0x7f64 = 0;
}

// 0x00520390: selects the series, then fills the track list; `event` is
// unused.
void TrackRecordDlg::UnknownFunction520390(UnknownTrackRecordEvent* event, int series)
{
    char text[128];
    g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(field_0x7f58);
    UnknownFunction51fc40(field_0x7f58);
    UnknownGameUiControl* control = UnknownFunction46ebf0("TxtTrack", 12);
    g_UnknownGlobal56e26c->UnknownFunction521970(0xbc6, text, 128);
    control->UnknownFunction470b20(text);
    control = UnknownFunction46ebf0("LstStats1", 3);
    control->UnknownFunction4775f0();
    control->UnknownFunction477bb0(0);
    control = UnknownFunction46ebf0("LstStats2", 3);
    control->UnknownFunction4775f0();
    control->UnknownFunction477bb0(0);
    control = UnknownFunction46ebf0("LstTrack", 3);
    control->UnknownFunction4775f0();
    UnknownFunction51ffe0(control, g_UnknownGlobal56e26c->field_0x2b58, series);
    ((UnknownTrackRecordListBox*)control)->UnknownVirtualSlot66(0);
}

void TrackRecordDlg::UnknownFunction520480(UnknownTrackRecordEvent* event, const char* name, float value,
                                           const char* text)
{
    UnknownGameUiControl* list;
    if (name) {
        list = event->field_0x0c->UnknownFunction46ebf0("LstStats1", 0);
        list->UnknownFunction476d80(name, (int)value, 0);
    }
    if (text) {
        list = event->field_0x0c->UnknownFunction46ebf0("LstStats2", 0);
        list->UnknownFunction476d80(text, 0, 0);
    }
}

void TrackRecordDlg::UnknownFunction5204e0(UnknownTrackRecordEvent* event, int times, const char* name)
{
    char text[128];
    UnknownGameUiControl* control = UnknownFunction46ebf0("TxtStats1", 12);
    g_UnknownGlobal56e26c->UnknownFunction521970(0xbc4, text, 128);
    control->UnknownFunction470b20(text);
    control = UnknownFunction46ebf0("TxtStats2", 12);
    if (!times)
        g_UnknownGlobal56e26c->UnknownFunction521970(0x938, text, 128);
    else
        g_UnknownGlobal56e26c->UnknownFunction521970(0xbc5, text, 128);
    control->UnknownFunction470b20(text);
    g_UnknownGlobal56e26c->field_0x3400->UnknownFunction51f2c0(times, name);
    control = event->field_0x0c->UnknownFunction46ebf0("LstStats1", 0);
    control->UnknownFunction4775f0();
    control->UnknownFunction477bb0(0);
    control = event->field_0x0c->UnknownFunction46ebf0("LstStats2", 0);
    control->UnknownFunction4775f0();
    control->UnknownFunction477bb0(0);
    for (int i = 0; i < 10; i++) {
        UnknownTrackGameObject3400* table = g_UnknownGlobal56e26c->field_0x3400;
        if (strcmp(table->field_0x14[i].field_0x00, "") != 0) {
            if (!times)
                sprintf(text, "%.0f", table->field_0x14[i].field_0x10);
            else
                UnknownFunction518690(text, table->field_0x14[i].field_0x10);
            UnknownFunction520480(event, g_UnknownGlobal56e26c->field_0x3400->field_0x14[i].field_0x00, 0.0f, text);
        }
    }
}
