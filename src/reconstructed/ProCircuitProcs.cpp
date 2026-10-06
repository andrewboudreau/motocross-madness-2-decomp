#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ProCircuitProcs.h"
#include "SelectGamePicProcs.h"
#include "KrustyUI.h"
#include "TrackGame.h"
#include "MatrixUtil.h"
#include "DebugAlloc.h"

// The four vector constants of many retail files (see Cube.cpp):
// 0x00689b08, 0x00689b18, 0x00689b28 and 0x00689af8, initialised by
// 0x004da3e0..0x004da51b, after this file's last function. 0x004d6abb reads
// the first and 0x004d73df passes the third. The set before the first
// function (0x004d49e0) closes ProCircuit.cpp.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// Whether `path` can be opened.
static inline int UnknownFileExists(const char* path)
{
    FILE* file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

// Sets `control`'s text from string resource `id`.
static inline void UnknownSetText(UnknownGameUiControl* control, int id)
{
    control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, id);
}

// 0x004d4b20
int UnknownFunction4d4b20(const void* a, const void* b)
{
    int first = atoi(((const UnknownGameUiListRow*)a)->field_0x14 + 1);
    int second = atoi(((const UnknownGameUiListRow*)b)->field_0x14 + 1);
    if (first < second)
        return -1;
    return first != second;
}

// 0x004d4b60
int UnknownFunction4d4b60(const void* a, const void* b)
{
    int first = atoi(((const UnknownGameUiListRow*)a)->field_0x14 + 1);
    int second = atoi(((const UnknownGameUiListRow*)b)->field_0x14 + 1);
    if (first > second)
        return -1;
    return first != second;
}

// 0x004d4ba0
void UnknownFunction4d4ba0()
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    if (circuit->field_0x464 & 8) {
        PCCompleteDlg* dialog = new(__FILE__, 78) PCCompleteDlg;
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
    } else if (circuit->field_0x44 == 1) {
        PCNewEventDlg* dialog = new(__FILE__, 80) PCNewEventDlg;
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
    } else {
        PCCentralDlg* dialog = new(__FILE__, 82) PCCentralDlg;
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
    }
}

// 0x004d4d20
void PCStartupDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->field_0x08) {
    case 5:
        field_0x7f5c = 0;
        field_0x7f58 = 0;
        UnknownFunction4d56c0();
        UnknownFunction4d5600();
        field_0x806c = 0;
        field_0x8070 = 0;
        UnknownFunction46ebf0("ButName", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstName", 3));
        UnknownFunction46ebf0("ButRank", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstRank", 3));
        UnknownFunction46ebf0("LstRank", 3)->UnknownFunction4777f0(UnknownFunction44b0a0);
        UnknownFunction46ebf0("ButClass", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstClass", 3));
        UnknownFunction46ebf0("ButDiff", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstDiff", 3));
        UnknownFunction46ebf0("ButPoints", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstPoints", 3));
        UnknownFunction46ebf0("LstPoints", 3)->UnknownFunction4777f0(UnknownFunction44b0e0);
        UnknownFunction46ebf0("ButCash", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstCash", 3));
        UnknownFunction46ebf0("LstCash", 3)->UnknownFunction4777f0(UnknownFunction4d4b60);
        UnknownFunction46ebf0("ButDoneName", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstDoneName", 3));
        UnknownFunction46ebf0("ButDoneRank", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstDoneRank", 3));
        UnknownFunction46ebf0("LstDoneRank", 3)->UnknownFunction4777f0(UnknownFunction44b0a0);
        UnknownFunction46ebf0("ButDoneClass", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstDoneClass", 3));
        UnknownFunction46ebf0("ButDoneDiff", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstDoneDiff", 3));
        UnknownFunction46ebf0("ButDonePoints", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstDonePoints", 3));
        UnknownFunction46ebf0("LstDonePoints", 3)->UnknownFunction4777f0(UnknownFunction44b0e0);
        UnknownFunction46ebf0("ButDoneCash", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstDoneCash", 3));
        UnknownFunction46ebf0("LstDoneCash", 3)->UnknownFunction4777f0(UnknownFunction4d4b60);
        UnknownFunction46ebf0("ButDoneStatus", 1)->UnknownFunction473390(UnknownFunction46ebf0("LstDoneStatus", 3));
        break;
    case 1:
        if (_stricmp("ButNew", event->field_0x04) == 0) {
            PCNewDlg* dialog = new(__FILE__, 135) PCNewDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0, (UnknownGuiDialog*)this, 0, 0, 1);
            dialog->UnknownFunction4d5f40(field_0x7f60, &field_0x8060, &field_0x8064, &field_0x8068);
        } else if (_stricmp("ButContinue", event->field_0x04) == 0) {
            int row = UnknownFunction46ebf0("LstRank", 3)->UnknownFunction4768d0(-1);
            g_UnknownGlobal56e26c->field_0x3444 = new(__FILE__, 142) UnknownTrackGameObject3444;
            strcpy((char*)g_UnknownGlobal56e26c->field_0x3448, field_0x7f58[row]);
            if (g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d4100((char*)g_UnknownGlobal56e26c->field_0x3448)) {
                PCCentralDlg* dialog = new(__FILE__, 146) PCCentralDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            }
            UnknownFunction46ff30(0);
        } else if (_stricmp("ButDelete", event->field_0x04) == 0) {
            field_0x8070 = 0;
            field_0x806c = new(__FILE__, 152) ChoiceDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)field_0x806c, 0, 4, 0, (UnknownGuiDialog*)this, 0, 0, 1);
            field_0x806c->UnknownFunction455700(0, 0x143c, 0, 0, 0, 0x143e, 0, 0, 0, 0x143d);
        } else if (_stricmp("ButDoneDelete", event->field_0x04) == 0) {
            field_0x806c = 0;
            field_0x8070 = new(__FILE__, 162) ChoiceDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)field_0x8070, 0, 4, 0, (UnknownGuiDialog*)this, 0, 0, 1);
            field_0x8070->UnknownFunction455700(0, 0x143c, 0, 0, 0, 0x143e, 0, 0, 0, 0x143d);
        } else if (_stricmp("Back", event->field_0x04) == 0) {
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(100);
            UnknownFunction46ff30(0);
        }
        break;
    case 9:
        if (event->field_0x00 == 0x3e9) {
            g_UnknownGlobal56e26c->field_0x3444 = new(__FILE__, 182) UnknownTrackGameObject3444;
            int difficulty;
            switch (field_0x8060) {
            case 1:
                difficulty = 1;
                break;
            case 2:
                difficulty = 2;
                break;
            default:
                difficulty = 3;
                break;
            }
            g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d3b00(field_0x7f60, difficulty, field_0x8064,
                                                                        field_0x8068 + 1);
            char path[128];
            // The first free CircuitNNN.pc.
            int number = 1;
            sprintf(path, "%s\\%s\\Circuit%03d.pc", "ui\\profile", g_UnknownGlobal56e26c->mode.field_0x00, number);
            while (UnknownFileExists(path))
                sprintf(path, "%s\\%s\\Circuit%03d.pc", "ui\\profile", g_UnknownGlobal56e26c->mode.field_0x00, ++number);
            strcpy((char*)g_UnknownGlobal56e26c->field_0x3448, path);
            g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d4150((char*)g_UnknownGlobal56e26c->field_0x3448);
            UnknownFunction46ff30(0);
            PCNewEventDlg* dialog = new(__FILE__, 214) PCNewEventDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
        } else if (event->field_0x00 == 0x65) {
            if (field_0x806c)
                DeleteFileA(field_0x7f58[UnknownFunction46ebf0("LstRank", 3)->UnknownFunction4768d0(-1)]);
            else
                DeleteFileA(field_0x7f58[UnknownFunction46ebf0("LstDoneRank", 3)->UnknownFunction4768d0(-1)]);
            UnknownFunction4d56c0();
            UnknownFunction4d5600();
        }
        break;
    case 2:
        UnknownFunction4d5600();
        break;
    case 6:
        UnknownFunction4d5ca0();
        break;
    }
}

// 0x004d5600
void PCStartupDlg::UnknownFunction4d5600()
{
    UnknownGameUiControl* list = UnknownFunction46ebf0("LstRank", 3);
    if (list->field_0x1ec && list->UnknownFunction476950() >= 0) {
        UnknownFunction46ebf0("ButContinue", 0)->UnknownFunction470660(1, 1);
        UnknownFunction46ebf0("ButDelete", 0)->UnknownFunction470660(1, 1);
    } else {
        UnknownFunction46ebf0("ButContinue", 0)->UnknownFunction470660(0, 1);
        UnknownFunction46ebf0("ButDelete", 0)->UnknownFunction470660(0, 1);
    }
    list = UnknownFunction46ebf0("LstDoneRank", 3);
    if (list->field_0x1ec && list->UnknownFunction476950() >= 0)
        UnknownFunction46ebf0("ButDoneDelete", 0)->UnknownFunction470660(1, 1);
    else
        UnknownFunction46ebf0("ButDoneDelete", 0)->UnknownFunction470660(0, 1);
}

// 0x004d56c0
void PCStartupDlg::UnknownFunction4d56c0()
{
    UnknownTrackGameObject3444 circuit;
    UnknownFunction46ebf0("LstName", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstRank", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstClass", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstDiff", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstPoints", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstCash", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstDoneName", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstDoneRank", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstDoneClass", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstDoneDiff", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstDonePoints", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstDoneCash", 3)->UnknownFunction4775f0();
    UnknownFunction46ebf0("LstDoneStatus", 3)->UnknownFunction4775f0();

    char path[128];
    char directory[128];
    char pattern[128];
    WIN32_FIND_DATA data;
    sprintf(directory, "%s\\%s\\", "ui\\profile", g_UnknownGlobal56e26c->mode.field_0x00);
    sprintf(pattern, "%s*.pc", directory);
    HANDLE find = FindFirstFileA(pattern, &data);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            sprintf(path, "%s%s", directory, data.cFileName);
            if (circuit.UnknownFunction4d4100(path)) {
                field_0x7f58 = (char**)UnknownFunction47b570(field_0x7f58, (field_0x7f5c + 1) * 4);
                field_0x7f58[field_0x7f5c] = (char*)DebugMalloc(strlen(path) + 1, __FILE__, 297);
                strcpy(field_0x7f58[field_0x7f5c], path);
                UnknownFunction4d59a0(&circuit, field_0x7f5c);
                field_0x7f5c++;
            }
        } while (FindNextFileA(find, &data));
        FindClose(find);
    }
}

// 0x004d5ca0
void PCStartupDlg::UnknownFunction4d5ca0()
{
    if (field_0x7f58) {
        for (int i = 0; i < field_0x7f5c; i++) {
            if (field_0x7f58[i])
                operator delete(field_0x7f58[i], __FILE__, 379);
        }
        operator delete(field_0x7f58, __FILE__, 382);
    }
    field_0x7f5c = 0;
    field_0x7f58 = 0;
}

// 0x004d5d20
void PCNewDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    char number[32];
    char text[128];
    switch (event->field_0x08) {
    case 0x13:
    {
        UnknownGameUiControl* edit = UnknownFunction46ebf0("EditBox", 11);
        edit->UnknownFunction473ef0(text, 127);
        if (!text[0])
            UnknownFunction46ebf0("OkButton", 0)->UnknownVirtualSlot49(0);
        else
            UnknownFunction46ebf0("OkButton", 0)->UnknownVirtualSlot49(1);
        break;
    }
    case 5: {
        UnknownFunction46ebf0("RadClass250", 4)->UnknownFunction479310(0);
        UnknownFunction46ebf0("RadLODMedium", 4)->UnknownFunction479310(0);
        UnknownFunction46ebf0("EditBox", 11)->UnknownFunction473da0(g_UnknownGlobal56e26c->mode.field_0x00);
        UnknownGameUiControl* list = UnknownFunction46ebf0("OpponentsListBox", 3);
        list->UnknownFunction4775f0();
        for (int i = 3; i < 11; i++) {
            sprintf(number, "%d", i);
            list->UnknownFunction476d80(number, i, 0);
        }
        list->UnknownFunction476b30(7);
        field_0x34->UnknownFunction487790((UnknownGuiControl*)UnknownFunction46ebf0("EditBox", 11), 0, 0);
        break;
    }
    case 1:
        if (_stricmp("OkButton", event->field_0x04) == 0) {
            UnknownFunction46ebf0("EditBox", 11)->UnknownFunction473ef0(field_0x7f58, 64);
            *field_0x7f60 = UnknownFunction46ebf0("RadClass125", 4)->UnknownFunction4793f0();
            *field_0x7f5c = UnknownFunction46ebf0("RadLODEasy", 4)->UnknownFunction4793f0();
            *field_0x7f64 = UnknownFunction46ebf0("OpponentsListBox", 3)->UnknownFunction4768d0(-1);
            UnknownFunction46ff30(0x3e9);
        } else if (_stricmp("CancelButton", event->field_0x04) == 0) {
            UnknownFunction46ff30(0);
        }
        break;
    }
}

// 0x004d5f40
void PCNewDlg::UnknownFunction4d5f40(char* name, int* difficulty, int* bikeClass, int* opponents)
{
    field_0x7f58 = name;
    field_0x7f5c = difficulty;
    field_0x7f60 = bikeClass;
    field_0x7f64 = opponents;
}

// 0x004d5f70
void PCCentralDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->field_0x08) {
    case 6:
        if (field_0x2c)
            field_0x2c->UnknownFunction46ea60(1);
        break;
    case 5: {
        field_0x7f58 = 0;
        field_0x7f5c = 0;
        field_0x7f60 = 0;
        UnknownGameUiControl* tab = UnknownFunction46ebf0("TabNext", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x1446);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x1446);
        tab->UnknownFunction470da0(0x22);
        tab->UnknownFunction479310(0);
        tab = UnknownFunction46ebf0("TabBikeRider", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x1447);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x1447);
        tab->UnknownFunction470da0(0x22);
        tab = UnknownFunction46ebf0("TabStandings", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x1448);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x1448);
        tab->UnknownFunction470da0(0x22);
        UnknownFunction46ebf0("TabNext", 4)->UnknownFunction479310(0);
        UnknownFunction4d6570(0);
        break;
    }
    case 1:
        if (_stricmp("Back", event->field_0x04) == 0) {
            g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d4150((char*)g_UnknownGlobal56e26c->field_0x3448);
            PCStartupDlg* dialog = new(__FILE__, 488) PCStartupDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (_stricmp("Start", event->field_0x04) == 0 || _stricmp("Practice", event->field_0x04) == 0) {
            UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
            g_UnknownGlobal56e26c->ui->UnknownFunction499a20();
            g_UnknownGlobal56e26c->mode.field_0x25ec[0] = 0;
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = 0;
            if (field_0x7f58)
                field_0x7f58->UnknownFunction46ecc0(1);
            if (field_0x7f5c)
                field_0x7f5c->UnknownFunction46ecc0(1);
            if (field_0x7f60)
                field_0x7f60->UnknownFunction46ecc0(1);
            g_UnknownGlobal56e26c->eventManager->UnknownFunction45e520();
            if (_stricmp("Start", event->field_0x04) == 0) {
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 = circuit->field_0x40;
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 = 1;
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20 = circuit->field_0x1285[circuit->field_0x40].field_0x0c;
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x24 = circuit->field_0x460 - 1;
            } else {
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 = circuit->field_0x40;
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 = 0;
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x24 = 0;
            }
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04);
            char name[128];
            strcpy(name, circuit->field_0x1285[circuit->field_0x40].field_0x00[circuit->field_0x44 - 1].field_0x00);
            char* extension = strrchr(name, '.');
            if (extension)
                *extension = 0;
            strcpy(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, name);
            if (circuit->field_0x40 == 1 || circuit->field_0x40 == 5)
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34 = 1;
            else
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34 = 0;
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x10 = 1;
            g_UnknownGlobal56e26c->mode.field_0x94 = circuit->field_0x50;
            UnknownFunction4536e0();
            UnknownVirtualSlot26();
        } else if (_stricmp("TabNext", event->field_0x04) == 0) {
            UnknownFunction4d6570(0);
        } else if (_stricmp("TabBikeRider", event->field_0x04) == 0) {
            UnknownFunction4d6570(1);
        } else if (_stricmp("TabStandings", event->field_0x04) == 0) {
            UnknownFunction4d6570(2);
        } else if (_stricmp("Options", event->field_0x04) == 0) {
            OptionsDlg* dialog = new(__FILE__, 554) OptionsDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, (UnknownGuiDialog*)this, 0, 0, 1);
        } else if (_stricmp("Controls", event->field_0x04) == 0) {
            OptionsDlg* dialog = new(__FILE__, 557) OptionsDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, (UnknownGuiDialog*)this, 1, 0, 1);
        } else if (_stricmp("Help", event->field_0x04) == 0) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523d30("MCM2HELP", 0);
        }
        break;
    }
}

// 0x004d6570
void PCCentralDlg::UnknownFunction4d6570(int page)
{
    CameraRect area;
    area.left = 16;
    area.top = 41;
    area.right = 626;
    area.bottom = 396;
    if (field_0x7f58 && page != 0) {
        field_0x7f58->Release();
        field_0x7f58 = 0;
    }
    if (field_0x7f5c && page != 1) {
        field_0x7f5c->Release();
        field_0x7f5c = 0;
    }
    if (field_0x7f60 && page != 2) {
        field_0x7f60->Release();
        field_0x7f60 = 0;
    }
    switch (page) {
    case 0:
        if (!field_0x7f58) {
            field_0x7f58 = new(__FILE__, 584) PCCentralNextDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)field_0x7f58, 0, 1, (int)&area, (UnknownGuiDialog*)this, 0, 0, 1);
        }
        break;
    case 1:
        if (!field_0x7f5c) {
            field_0x7f5c = new(__FILE__, 590) PCCentralBikeRiderDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)field_0x7f5c, 0, 1, (int)&area, (UnknownGuiDialog*)this, 0, 0, 1);
        }
        break;
    case 2:
        if (!field_0x7f60) {
            field_0x7f60 = new(__FILE__, 596) PCCentralStandingsDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)field_0x7f60, 0, 1, (int)&area, (UnknownGuiDialog*)this, 0, 0, 1);
        }
        break;
    }
}

// 0x004d7f40
void PCCentralNextDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    if (event->field_0x08 != 5)
        return;
    const char* directory;
    UnknownGameUiControl* picture;
    char text[128];
    char image[128];
    char path[260];
    int purse = (int)(circuit->field_0x1229[circuit->field_0x40] * circuit->field_0x1241);
    sprintf(text, "$%d", circuit->field_0x1229[circuit->field_0x40]);
    UnknownFunction46ebf0("TxtNextFee", 12)->UnknownFunction470b20(text);
    sprintf(text, "$%d", purse);
    UnknownFunction46ebf0("TxtNextPurse", 12)->UnknownFunction470b20(text);
    UnknownFunction46ebf0("TxtNextTrack", 12)
        ->UnknownFunction470b20(circuit->field_0x1285[circuit->field_0x40].field_0x00[circuit->field_0x44 - 1].field_0x04);
    switch (circuit->field_0x40) {
    case 5:
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f0, text, 128);
        break;
    case 1:
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13ed, text, 128);
        break;
    case 2:
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13ef, text, 128);
        break;
    case 3:
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f2, text, 128);
        break;
    }
    UnknownFunction46ebf0("TxtNextType", 12)->UnknownFunction470b20(text);
    sprintf(text, "%d/%d", circuit->field_0x44,
            circuit->field_0x1285[circuit->field_0x40].field_0x08 ? circuit->field_0x1285[circuit->field_0x40].field_0x08 - 1
                                                                  : circuit->field_0x1285[circuit->field_0x40].field_0x04);
    UnknownFunction46ebf0("TxtNextRace", 12)->UnknownFunction470b20(text);
    sprintf(text, "$%d", circuit->field_0x465[0].field_0x30);
    UnknownFunction46ebf0("TxtCash", 12)->UnknownFunction470b20(text);
    sprintf(text, "$%d", circuit->field_0x454);
    UnknownFunction46ebf0("TxtFees", 12)->UnknownFunction470b20(text);
    sprintf(text, "$%d", circuit->field_0x465[0].field_0x18.field_0x08);
    UnknownFunction46ebf0("TxtRepairs", 12)->UnknownFunction470b20(text);
    sprintf(text, "$%d", circuit->field_0x465[0].field_0x18.field_0x0c);
    UnknownFunction46ebf0("TxtMedical", 12)->UnknownFunction470b20(text);
    UnknownGameUiControl* announcement = UnknownFunction46ebf0("TxtAnnouncement", 12);
    announcement->field_0x1e8 = 1;
    announcement->UnknownFunction470b20(circuit->field_0x54);

    picture = UnknownFunction46ebf0("NextTrackPic", 5);
    strcpy(image, circuit->field_0x1285[circuit->field_0x40].field_0x00[circuit->field_0x44 - 1].field_0x08);
    char* extension = strrchr(image, '.');
    if (extension)
        *extension = 0;
    int series = circuit->field_0x40;
    switch (series) {
    case 5:
        directory = "Teraform\\Enduro";
        strcat(image, "01s.tga");
        break;
    case 1:
        directory = "Teraform\\Baja";
        strcat(image, "01s.tga");
        break;
    case 2:
        directory = "Teraform\\National";
        strcat(image, "s.tga");
        break;
    case 3:
        directory = "Teraform\\SX";
        strcat(image, "s.tga");
        break;
    }
    g_UnknownGlobal56e26c->mode.UnknownFunction523a60(
        (int)directory, circuit->field_0x1285[series].field_0x00[circuit->field_0x44 - 1].field_0x00, "env", path);
    g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9b80(path);
    g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9e30(
        circuit->field_0x1285[circuit->field_0x40].field_0x00[circuit->field_0x44 - 1].field_0x00, "scn", 0);
    picture->UnknownFunction470760(0, image);
}

// 0x004d8380
void PCCentralStandingsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    if (event->field_0x08 != 5)
        return;
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    UnknownGameUiControl* rank = UnknownFunction46ebf0("LstRank", 3);
    UnknownGameUiControl* name = UnknownFunction46ebf0("LstName", 3);
    UnknownGameUiControl* points = UnknownFunction46ebf0("LstPoints", 3);
    UnknownGameUiControl* bike = UnknownFunction46ebf0("LstBike", 3);
    UnknownGameUiControl* cash = UnknownFunction46ebf0("LstCash", 3);
    UnknownGameUiControl* winnings = UnknownFunction46ebf0("LstWinnings", 3);
    UnknownGameUiControl* expenses = UnknownFunction46ebf0("LstExpenses", 3);
    char text[128];
    for (int i = 0; i < circuit->field_0x460; i++) {
        _itoa(circuit->field_0x465[i].field_0x18.field_0x10, text, 10);
        rank->UnknownFunction476d80(text, i, 0);
        name->UnknownFunction476d80(circuit->field_0x465[i].field_0xf8, i, 0);
        _itoa(circuit->field_0x465[i].field_0x18.field_0x00, text, 10);
        points->UnknownFunction476d80(text, i, 0);
        bike->UnknownFunction476d80(circuit->field_0x465[i].field_0x118, i, 0);
        sprintf(text, "$%d", circuit->field_0x465[i].field_0x30);
        cash->UnknownFunction476d80(text, i, 0);
        sprintf(text, "$%d", circuit->field_0x465[i].field_0x18.field_0x04 + circuit->field_0x465[i].field_0x18.field_0x14);
        winnings->UnknownFunction476d80(text, i, 0);
        sprintf(text, "$%d", circuit->field_0x465[i].field_0x18.field_0x08 + circuit->field_0x465[i].field_0x18.field_0x0c +
                                 circuit->field_0x454);
        expenses->UnknownFunction476d80(text, i, 0);
    }
    UnknownFunction46ebf0("ButRank", 1)->UnknownFunction473390(rank);
    rank->UnknownFunction4777f0(UnknownFunction44b0a0);
    UnknownFunction46ebf0("ButName", 1)->UnknownFunction473390(name);
    UnknownFunction46ebf0("ButPoints", 1)->UnknownFunction473390(points);
    points->UnknownFunction4777f0(UnknownFunction44b0e0);
    UnknownFunction46ebf0("ButBike", 1)->UnknownFunction473390(bike);
    UnknownFunction46ebf0("ButCash", 1)->UnknownFunction473390(cash);
    cash->UnknownFunction4777f0(UnknownFunction4d4b60);
    UnknownFunction46ebf0("ButWinnings", 1)->UnknownFunction473390(winnings);
    winnings->UnknownFunction4777f0(UnknownFunction4d4b60);
    UnknownFunction46ebf0("ButExpenses", 1)->UnknownFunction473390(expenses);
    expenses->UnknownFunction4777f0(UnknownFunction4d4b20);
    rank->UnknownFunction477900(1);
}

// 0x004d8660
void PCLastRaceDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    switch (event->field_0x08) {
    case 5: {
        UnknownGameUiControl* rank = UnknownFunction46ebf0("LstRank", 3);
        UnknownGameUiControl* name = UnknownFunction46ebf0("LstName", 3);
        UnknownGameUiControl* points = UnknownFunction46ebf0("LstPoints", 3);
        UnknownGameUiControl* bike = UnknownFunction46ebf0("LstBike", 3);
        UnknownGameUiControl* winnings = UnknownFunction46ebf0("LstWinnings", 3);
        UnknownGameUiControl* repairs = UnknownFunction46ebf0("LstRepairs", 3);
        UnknownGameUiControl* medical = UnknownFunction46ebf0("LstMedical", 3);
        UnknownFunction4d8c40();
        char text[128];
        for (int i = 0; i < g_UnknownGlobal56e26c->field_0x3444->field_0x460; i++) {
            _itoa(g_UnknownGlobal56e26c->field_0x3444->field_0x465[i].field_0x00.field_0x10, text, 10);
            rank->UnknownFunction476d80(text, i, 0);
            name->UnknownFunction476d80(g_UnknownGlobal56e26c->field_0x3444->field_0x465[i].field_0xf8, i, 0);
            _itoa(g_UnknownGlobal56e26c->field_0x3444->field_0x465[i].field_0x00.field_0x00, text, 10);
            points->UnknownFunction476d80(text, i, 0);
            bike->UnknownFunction476d80(g_UnknownGlobal56e26c->field_0x3444->field_0x465[i].field_0x118, i, 0);
            sprintf(text, "$%d", g_UnknownGlobal56e26c->field_0x3444->field_0x465[i].field_0x00.field_0x04);
            winnings->UnknownFunction476d80(text, i, 0);
            sprintf(text, "$%d", g_UnknownGlobal56e26c->field_0x3444->field_0x465[i].field_0x00.field_0x08);
            repairs->UnknownFunction476d80(text, i, 0);
            sprintf(text, "$%d", g_UnknownGlobal56e26c->field_0x3444->field_0x465[i].field_0x00.field_0x0c);
            medical->UnknownFunction476d80(text, i, 0);
        }
        UnknownFunction46ebf0("ButRank", 1)->UnknownFunction473390(rank);
        rank->UnknownFunction4777f0(UnknownFunction44b0a0);
        UnknownFunction46ebf0("ButName", 1)->UnknownFunction473390(name);
        UnknownFunction46ebf0("ButPoints", 1)->UnknownFunction473390(points);
        points->UnknownFunction4777f0(UnknownFunction44b0e0);
        UnknownFunction46ebf0("ButBike", 1)->UnknownFunction473390(bike);
        UnknownFunction46ebf0("ButWinnings", 1)->UnknownFunction473390(winnings);
        winnings->UnknownFunction4777f0(UnknownFunction4d4b60);
        UnknownFunction46ebf0("ButRepairs", 1)->UnknownFunction473390(repairs);
        repairs->UnknownFunction4777f0(UnknownFunction4d4b20);
        UnknownFunction46ebf0("ButMedical", 1)->UnknownFunction473390(medical);
        medical->UnknownFunction4777f0(UnknownFunction4d4b20);
        rank->UnknownFunction477900(1);
        g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d41a0();
        g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d4150((char*)g_UnknownGlobal56e26c->field_0x3448);
        break;
    }
    case 1:
        if (_stricmp("Back", event->field_0x04) == 0) {
            int bonus = circuit->field_0x44 == circuit->field_0x1285[circuit->field_0x40].field_0x08;
            if (bonus && circuit->field_0x465[0].field_0x18.field_0x10 == 1) {
                PCBonusTrackDlg* dialog = new(__FILE__, 1368) PCBonusTrackDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            } else if (circuit->field_0x465[0].field_0x30 <
                       circuit->field_0x1229[g_UnknownGlobal56e26c->field_0x3444->field_0x40]) {
                if (!(circuit->field_0x464 & 1)) {
                    PCBailoutDlg* dialog = new(__FILE__, 1374) PCBailoutDlg;
                    field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
                    circuit->field_0x464 |= 1;
                } else if (!(circuit->field_0x464 & 2)) {
                    PCBunnyDlg* dialog = new(__FILE__, 1378) PCBunnyDlg;
                    field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
                    circuit->field_0x464 |= 2;
                } else {
                    circuit->field_0x464 |= 4;
                    PCFailedDlg* dialog = new(__FILE__, 1383) PCFailedDlg;
                    field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
                }
            } else {
                UnknownFunction4d4ba0();
            }
            UnknownFunction46ff30(0);
        }
        break;
    }
}

// 0x004d8c20
int UnknownFunction4d8c20(const void* a, const void* b)
{
    int first = ((const UnknownProCircuitRank*)a)->field_0x00;
    int second = ((const UnknownProCircuitRank*)b)->field_0x00;
    if (first > second)
        return -1;
    return first != second;
}

// 0x004d9340
void PCFailedDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownGameUiControl* label;
    switch (event->field_0x08) {
    case 5: {
        g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d4150((char*)g_UnknownGlobal56e26c->field_0x3448);
        UnknownGameUiControl* description = UnknownFunction46ebf0("TxtDescription", 12);
        description->field_0x1e8 = 1;
        description->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x1464);
        label = UnknownFunction46ebf0("TitleText", 12);
        label->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x1467);
        label = UnknownFunction46ebf0("ButDecline", 1);
        label->UnknownFunction470660(0, 1);
        label = UnknownFunction46ebf0("ButAccept", 1);
        label->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x144e);
        UnknownGameUiControl* picture = UnknownFunction46ebf0("Pic", 5);
        picture->UnknownFunction470730(0, UnknownFunction46e9a0("FailScreen"));
        break;
    }
    case 1: {
        PCFinishedDlg* dialog = new(__FILE__, 1593) PCFinishedDlg;
        field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
        UnknownFunction46ff30(0);
        break;
    }
    }
}

// 0x004d94e0
void PCFinishedDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    switch (event->field_0x08) {
    case 5: {
        UnknownGameUiControl* rank = UnknownFunction46ebf0("LstRank", 3);
        UnknownGameUiControl* name = UnknownFunction46ebf0("LstName", 3);
        UnknownGameUiControl* points = UnknownFunction46ebf0("LstPoints", 3);
        UnknownGameUiControl* bike = UnknownFunction46ebf0("LstBike", 3);
        UnknownGameUiControl* cash = UnknownFunction46ebf0("LstCash", 3);
        UnknownGameUiControl* repairs = UnknownFunction46ebf0("LstRepairs", 3);
        UnknownGameUiControl* medical = UnknownFunction46ebf0("LstMedical", 3);
        char text[128];
        for (int i = 0; i < circuit->field_0x460; i++) {
            _itoa(circuit->field_0x465[i].field_0x18.field_0x10, text, 10);
            rank->UnknownFunction476d80(text, i, 0);
            name->UnknownFunction476d80(circuit->field_0x465[i].field_0xf8, i, 0);
            _itoa(circuit->field_0x465[i].field_0x18.field_0x00, text, 10);
            points->UnknownFunction476d80(text, i, 0);
            bike->UnknownFunction476d80(circuit->field_0x465[i].field_0x118, i, 0);
            sprintf(text, "$%d", circuit->field_0x465[i].field_0x30);
            cash->UnknownFunction476d80(text, i, 0);
            sprintf(text, "$%d", circuit->field_0x465[i].field_0x18.field_0x08);
            repairs->UnknownFunction476d80(text, i, 0);
            sprintf(text, "$%d", circuit->field_0x465[i].field_0x18.field_0x0c);
            medical->UnknownFunction476d80(text, i, 0);
        }
        UnknownFunction46ebf0("ButRank", 1)->UnknownFunction473390(rank);
        rank->UnknownFunction4777f0(UnknownFunction44b0a0);
        UnknownFunction46ebf0("ButName", 1)->UnknownFunction473390(name);
        UnknownFunction46ebf0("ButPoints", 1)->UnknownFunction473390(points);
        points->UnknownFunction4777f0(UnknownFunction44b0e0);
        UnknownFunction46ebf0("ButBike", 1)->UnknownFunction473390(bike);
        UnknownFunction46ebf0("ButCash", 1)->UnknownFunction473390(cash);
        cash->UnknownFunction4777f0(UnknownFunction4d4b60);
        UnknownFunction46ebf0("ButRepairs", 1)->UnknownFunction473390(repairs);
        repairs->UnknownFunction4777f0(UnknownFunction4d4b20);
        UnknownFunction46ebf0("ButMedical", 1)->UnknownFunction473390(medical);
        medical->UnknownFunction4777f0(UnknownFunction4d4b20);
        rank->UnknownFunction477900(1);
        break;
    }
    case 1:
        if (_stricmp("Back", event->field_0x04) == 0) {
            PCStartupDlg* dialog = new(__FILE__, 1659) PCStartupDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            UnknownFunction46ff30(0);
        }
        break;
    }
}

// 0x004da1b0
void PCCompleteDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    switch (event->field_0x08) {
    case 5: {
        char rank[1024];
        char text[1024];
        char message[2048];
        g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d4150((char*)g_UnknownGlobal56e26c->field_0x3448);
        UnknownSetText(UnknownFunction46ebf0("TitleText", 12), 0x145d);
        UnknownGameUiControl* description = UnknownFunction46ebf0("TxtDescription", 12);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x145e, text, 1023);
        strcpy(rank, "");
        if (circuit->field_0x465[0].field_0x18.field_0x10 == 1)
            g_UnknownGlobal56e26c->UnknownFunction521970(0x145f, rank, 1023);
        sprintf(message, "%s\n%s", text, rank);
        description->field_0x1e8 = 1;
        description->UnknownFunction470b20(message);
        UnknownGameUiControl* control = UnknownFunction46ebf0("ButDecline", 1);
        control->UnknownFunction470660(0, 1);
        UnknownSetText(UnknownFunction46ebf0("ButAccept", 1), 0x144e);
        control = UnknownFunction46ebf0("Pic", 5);
        control->UnknownFunction470730(0, UnknownFunction46e9a0("WinScreen"));
        break;
    }
    case 1: {
        PCFinishedDlg* dialog = new(__FILE__, 1941) PCFinishedDlg;
        field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
        UnknownFunction46ff30(0);
        break;
    }
    }
}

// 0x004d71f0
void PCCentralBikeRiderDlg::UnknownFunction4d71f0(int number)
{
    UnknownBikeNumberPainter painter(g_UnknownGlobal56e26c->field_0x1c);
    for (int i = 0; i < g_UnknownGlobal56e26c->ui->field_0x4c; i++)
        painter.UnknownFunction417670(
            ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[i].field_0xc0->field_0x1a0, number);
}

// 0x004d72a0
void PCCentralBikeRiderDlg::UnknownVirtualSlot26()
{
    g_UnknownGlobal56e26c->ui->UnknownFunction499a20();
    UIDialog::UnknownVirtualSlot26();
}

// 0x004d78c0
void PCCentralBikeRiderDlg::UnknownFunction4d78c0()
{
    UnknownGameUiControl* list = UnknownFunction46ebf0("DDLRiders", 6)->field_0x1fc;
    field_0x7f88 = 1;
    g_UnknownGlobal56e26c->field_0x3444->field_0x458 = list->UnknownFunction4768d0(-1);
}

// 0x004d7d70
void PCCentralBikeRiderDlg::UnknownVirtualSlot31(int apply)
{
    if (!apply)
        return;
    strcpy(g_UnknownGlobal56e26c->field_0x3444->field_0x465[0].field_0x38, "");
    UnknownGameUiControl* list = UnknownFunction46ebf0("DDLBikes", 6)->field_0x1fc;
    int bike = list->UnknownFunction4768d0(-1);
    strcpy(g_UnknownGlobal56e26c->field_0x3444->field_0x465[0].field_0x38,
           ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)
               [((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bike].field_0x00].field_0x40);
    strcpy(g_UnknownGlobal56e26c->field_0x3444->field_0x465[0].field_0x78,
           ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bike].field_0x48);
    strcpy(g_UnknownGlobal56e26c->field_0x3444->field_0x465[0].field_0x118,
           ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bike].field_0x04);
    strcpy(g_UnknownGlobal56e26c->field_0x3444->field_0x465[0].field_0xb8, "");
    list = UnknownFunction46ebf0("DDLRiders", 6)->field_0x1fc;
    int rider = list->UnknownFunction4768d0(-1);
    strcpy(g_UnknownGlobal56e26c->field_0x3444->field_0x465[0].field_0xb8,
           ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x58)[rider].field_0x40);
}

// 0x004d77d0
int PCCentralBikeRiderDlg::UnknownVirtualSlot13()
{
    if (field_0x7f84) {
        UnknownGameUiControl* list = UnknownFunction46ebf0("DDLBikes", 6)->field_0x1fc;
        KrustyUI* ui = g_UnknownGlobal56e26c->ui;
        int bike = list->UnknownFunction4768d0(-1);
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)ui->field_0x50)[bike];
        ((UnknownProCircuitSkinned*)((UnknownKrustyUIModel*)ui->field_0x48)[entry->field_0x00].field_0xc0->field_0x1a0)
            ->UnknownFunction444c70(0, entry->field_0x48, &g_UnknownGlobal56e26c->field_0x1c);
        field_0x7f84 = 0;
    }
    if (field_0x7f88) {
        UnknownGameUiControl* list = UnknownFunction46ebf0("DDLRiders", 6)->field_0x1fc;
        KrustyUI* ui = g_UnknownGlobal56e26c->ui;
        UnknownKrustyUIModel* rider = &((UnknownKrustyUIModel*)ui->field_0x58)[list->UnknownFunction4768d0(-1)];
        ((UnknownProCircuitSkinned*)ui->field_0x46c->field_0x1a0)
            ->UnknownFunction444c70(0, rider->field_0x40, &g_UnknownGlobal56e26c->field_0x1c);
        field_0x7f88 = 0;
    }
    return UIDialog::UnknownVirtualSlot13();
}
