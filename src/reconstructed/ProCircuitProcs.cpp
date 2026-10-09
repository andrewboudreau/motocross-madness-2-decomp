#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ProCircuitProcs.h"

#include "DialogEventKind.h"
#include "SelectGamePicProcs.h"
#include "KrustyUI.h"
#include "TrackGame.h"
#include "MatrixUtil.h"
#include "DebugAlloc.h"
#include "../krusty2/math/FastMath.h"
#include "OptionProcs.h"
#include "BackgroundImage.h"
#include "Camera.h"

// d3drm.dll's D3DRMVectorRotate (declared as in FollowCamera.h): rotates
// `vector` about `axis` by `theta`.
extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

// Inline vector helpers of the bike views (as in DlgProcs.cpp).
static inline Vector3 UnknownVectorDifference(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 UnknownVectorSum(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline float UnknownVectorLength(const Vector3& v) {
    float lengthSquared = (v.x * v.x + v.y * v.y) + v.z * v.z;
    if (lengthSquared == 1.0f)
        return 1.0f;
    return FastSqrt(lengthSquared);
}

static inline Vector3& operator*=(Vector3& v, float scale) {
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
    return v;
}

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
static inline void UnknownSetText(UIControl* control, int id)
{
    control->SetTextFromResource(g_TrackGame->resourceInstance, id);
}

// 0x004d4b20
int CompareCashAscending(const void* a, const void* b)
{
    int first = atoi(((const UnknownGameUiListRow*)a)->text + 1);
    int second = atoi(((const UnknownGameUiListRow*)b)->text + 1);
    if (first < second)
        return -1;
    return first != second;
}

// 0x004d4b60
int CompareCashDescending(const void* a, const void* b)
{
    int first = atoi(((const UnknownGameUiListRow*)a)->text + 1);
    int second = atoi(((const UnknownGameUiListRow*)b)->text + 1);
    if (first > second)
        return -1;
    return first != second;
}

// 0x004d4ba0
void OpenCareerDialog()
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    if (circuit->field_0x464 & 8) {
        PCCompleteDlg* dialog = new(__FILE__, 78) PCCompleteDlg;
        g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
    } else if (circuit->field_0x44 == 1) {
        PCNewEventDlg* dialog = new(__FILE__, 80) PCNewEventDlg;
        g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
    } else {
        PCCentralDlg* dialog = new(__FILE__, 82) PCCentralDlg;
        g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
    }
}

// 0x004d4d20
void PCStartupDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->kind) {
    case kDialogInit:
        field_0x7f5c = 0;
        field_0x7f58 = 0;
        FillLists();
        EnableButtons();
        field_0x806c = 0;
        field_0x8070 = 0;
        static_cast<UIButton*>(FindControl("ButName", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstName", 3)));
        static_cast<UIButton*>(FindControl("ButRank", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstRank", 3)));
        static_cast<UIListBox*>(FindControl("LstRank", 3))->SetSortCompare(CompareRowNumbersAscending);
        static_cast<UIButton*>(FindControl("ButClass", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstClass", 3)));
        static_cast<UIButton*>(FindControl("ButDiff", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstDiff", 3)));
        static_cast<UIButton*>(FindControl("ButPoints", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstPoints", 3)));
        static_cast<UIListBox*>(FindControl("LstPoints", 3))->SetSortCompare(CompareRowNumbersDescending);
        static_cast<UIButton*>(FindControl("ButCash", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstCash", 3)));
        static_cast<UIListBox*>(FindControl("LstCash", 3))->SetSortCompare(CompareCashDescending);
        static_cast<UIButton*>(FindControl("ButDoneName", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstDoneName", 3)));
        static_cast<UIButton*>(FindControl("ButDoneRank", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstDoneRank", 3)));
        static_cast<UIListBox*>(FindControl("LstDoneRank", 3))->SetSortCompare(CompareRowNumbersAscending);
        static_cast<UIButton*>(FindControl("ButDoneClass", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstDoneClass", 3)));
        static_cast<UIButton*>(FindControl("ButDoneDiff", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstDoneDiff", 3)));
        static_cast<UIButton*>(FindControl("ButDonePoints", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstDonePoints", 3)));
        static_cast<UIListBox*>(FindControl("LstDonePoints", 3))->SetSortCompare(CompareRowNumbersDescending);
        static_cast<UIButton*>(FindControl("ButDoneCash", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstDoneCash", 3)));
        static_cast<UIListBox*>(FindControl("LstDoneCash", 3))->SetSortCompare(CompareCashDescending);
        static_cast<UIButton*>(FindControl("ButDoneStatus", 1))->SetSortList(static_cast<UIListBox*>(FindControl("LstDoneStatus", 3)));
        break;
    case kDialogCommand:
        if (_stricmp("ButNew", event->controlName) == 0) {
            PCNewDlg* dialog = new(__FILE__, 135) PCNewDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
            dialog->SetAnswerTargets(field_0x7f60, &field_0x8060, &field_0x8064, &field_0x8068);
        } else if (_stricmp("ButContinue", event->controlName) == 0) {
            int row = static_cast<UIListBox*>(FindControl("LstRank", 3))->GetRowData(-1);
            g_TrackGame->field_0x3444 = new(__FILE__, 142) UnknownTrackGameObject3444;
            strcpy((char*)g_TrackGame->field_0x3448, field_0x7f58[row]);
            if (g_TrackGame->field_0x3444->LoadSaved((char*)g_TrackGame->field_0x3448)) {
                PCCentralDlg* dialog = new(__FILE__, 146) PCCentralDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            }
            EndDialog(0);
        } else if (_stricmp("ButDelete", event->controlName) == 0) {
            field_0x8070 = 0;
            field_0x806c = new(__FILE__, 152) ChoiceDlg;
            guiManager->ShowDialog((UIDialog*)field_0x806c, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
            field_0x806c->SetTextsOrResources(0, 0x143c, 0, 0, 0, 0x143e, 0, 0, 0, 0x143d);
        } else if (_stricmp("ButDoneDelete", event->controlName) == 0) {
            field_0x806c = 0;
            field_0x8070 = new(__FILE__, 162) ChoiceDlg;
            guiManager->ShowDialog((UIDialog*)field_0x8070, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
            field_0x8070->SetTextsOrResources(0, 0x143c, 0, 0, 0, 0x143e, 0, 0, 0, 0x143d);
        } else if (_stricmp("Back", event->controlName) == 0) {
            g_TrackGame->ui->OpenMenu(100);
            EndDialog(0);
        }
        break;
    case 9:
        if (event->code == 0x3e9) {
            g_TrackGame->field_0x3444 = new(__FILE__, 182) UnknownTrackGameObject3444;
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
            g_TrackGame->field_0x3444->UnknownFunction4d3b00(field_0x7f60, difficulty, field_0x8064,
                                                                        field_0x8068 + 1);
            char path[128];
            // The first free CircuitNNN.pc.
            int number = 1;
            sprintf(path, "%s\\%s\\Circuit%03d.pc", "ui\\profile", g_TrackGame->mode.field_0x00, number);
            while (UnknownFileExists(path))
                sprintf(path, "%s\\%s\\Circuit%03d.pc", "ui\\profile", g_TrackGame->mode.field_0x00, ++number);
            strcpy((char*)g_TrackGame->field_0x3448, path);
            g_TrackGame->field_0x3444->Save((char*)g_TrackGame->field_0x3448);
            EndDialog(0);
            PCNewEventDlg* dialog = new(__FILE__, 214) PCNewEventDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
        } else if (event->code == 0x65) {
            if (field_0x806c)
                DeleteFileA(field_0x7f58[static_cast<UIListBox*>(FindControl("LstRank", 3))->GetRowData(-1)]);
            else
                DeleteFileA(field_0x7f58[static_cast<UIListBox*>(FindControl("LstDoneRank", 3))->GetRowData(-1)]);
            FillLists();
            EnableButtons();
        }
        break;
    case kDialogListSelect:
        EnableButtons();
        break;
    case kDialogClose:
        FreeFileNames();
        break;
    }
}

// 0x004d5600
void PCStartupDlg::EnableButtons()
{
    UIListBox* list = static_cast<UIListBox*>(FindControl("LstRank", 3));
    if (list->rowCount && list->GetSelectedRow() >= 0) {
        FindControl("ButContinue", 0)->Show(1, 1);
        FindControl("ButDelete", 0)->Show(1, 1);
    } else {
        FindControl("ButContinue", 0)->Show(0, 1);
        FindControl("ButDelete", 0)->Show(0, 1);
    }
    list = static_cast<UIListBox*>(FindControl("LstDoneRank", 3));
    if (list->rowCount && list->GetSelectedRow() >= 0)
        FindControl("ButDoneDelete", 0)->Show(1, 1);
    else
        FindControl("ButDoneDelete", 0)->Show(0, 1);
}

// 0x004d56c0
void PCStartupDlg::FillLists()
{
    UnknownTrackGameObject3444 circuit;
    static_cast<UIListBox*>(FindControl("LstName", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstRank", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstClass", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDiff", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstPoints", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstCash", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDoneName", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDoneRank", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDoneClass", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDoneDiff", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDonePoints", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDoneCash", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDoneStatus", 3))->RemoveAllRows();

    char path[128];
    char directory[128];
    char pattern[128];
    WIN32_FIND_DATA data;
    sprintf(directory, "%s\\%s\\", "ui\\profile", g_TrackGame->mode.field_0x00);
    sprintf(pattern, "%s*.pc", directory);
    HANDLE find = FindFirstFileA(pattern, &data);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            sprintf(path, "%s%s", directory, data.cFileName);
            if (circuit.LoadSaved(path)) {
                field_0x7f58 = (char**)UnknownFunction47b570(field_0x7f58, (field_0x7f5c + 1) * 4);
                field_0x7f58[field_0x7f5c] = (char*)DebugMalloc(strlen(path) + 1, __FILE__, 297);
                strcpy(field_0x7f58[field_0x7f5c], path);
                AddCircuitRow(&circuit, field_0x7f5c);
                field_0x7f5c++;
            }
        } while (FindNextFileA(find, &data));
        FindClose(find);
    }
}

// 0x004d5ca0
void PCStartupDlg::FreeFileNames()
{
    if (field_0x7f58) {
        for (int i = 0; i < field_0x7f5c; i++) {
            if (field_0x7f58[i])
                DebugFree(field_0x7f58[i], __FILE__, 379);
        }
        DebugFree(field_0x7f58, __FILE__, 382);
    }
    field_0x7f5c = 0;
    field_0x7f58 = 0;
}

// 0x004d5d20
void PCNewDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    char number[32];
    char text[128];
    switch (event->kind) {
    case kDialogEditChange:
    {
        UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditBox", 11));
        edit->GetEditText(text, 127);
        if (!text[0])
            FindControl("OkButton", 0)->UnknownVirtualSlot49(0);
        else
            FindControl("OkButton", 0)->UnknownVirtualSlot49(1);
        break;
    }
    case kDialogInit: {
        static_cast<UIRadioButton*>(FindControl("RadClass250", 4))->SelectInGroup(0);
        static_cast<UIRadioButton*>(FindControl("RadLODMedium", 4))->SelectInGroup(0);
        static_cast<UIEditBox*>(FindControl("EditBox", 11))->SetEditText(g_TrackGame->mode.field_0x00);
        UIListBox* list = static_cast<UIListBox*>(FindControl("OpponentsListBox", 3));
        list->RemoveAllRows();
        for (int i = 3; i < 11; i++) {
            sprintf(number, "%d", i);
            list->AddRow(number, i, 0);
        }
        list->SelectRowByData(7);
        guiUser->UnknownFunction487790((UnknownGuiControl*)FindControl("EditBox", 11), 0, 0);
        break;
    }
    case kDialogCommand:
        if (_stricmp("OkButton", event->controlName) == 0) {
            static_cast<UIEditBox*>(FindControl("EditBox", 11))->GetEditText(field_0x7f58, 64);
            *field_0x7f60 = static_cast<UIRadioButton*>(FindControl("RadClass125", 4))->GetGroupSelection();
            *field_0x7f5c = static_cast<UIRadioButton*>(FindControl("RadLODEasy", 4))->GetGroupSelection();
            *field_0x7f64 = static_cast<UIListBox*>(FindControl("OpponentsListBox", 3))->GetRowData(-1);
            EndDialog(0x3e9);
        } else if (_stricmp("CancelButton", event->controlName) == 0) {
            EndDialog(0);
        }
        break;
    }
}

// 0x004d5f40
void PCNewDlg::SetAnswerTargets(char* name, int* difficulty, int* bikeClass, int* opponents)
{
    field_0x7f58 = name;
    field_0x7f5c = difficulty;
    field_0x7f60 = bikeClass;
    field_0x7f64 = opponents;
}

// 0x004d5f70
void PCCentralDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->kind) {
    case kDialogClose:
        if (parentDialog)
            parentDialog->UnknownFunction46ea60(1);
        break;
    case kDialogInit: {
        field_0x7f58 = 0;
        field_0x7f5c = 0;
        field_0x7f60 = 0;
        UIRadioButton* tab = static_cast<UIRadioButton*>(FindControl("TabNext", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1446);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1446);
        tab->SetTextAlign(0x22);
        tab->SelectInGroup(0);
        tab = static_cast<UIRadioButton*>(FindControl("TabBikeRider", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1447);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1447);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIRadioButton*>(FindControl("TabStandings", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1448);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1448);
        tab->SetTextAlign(0x22);
        static_cast<UIRadioButton*>(FindControl("TabNext", 4))->SelectInGroup(0);
        OpenPage(0);
        break;
    }
    case kDialogCommand:
        if (_stricmp("Back", event->controlName) == 0) {
            g_TrackGame->field_0x3444->Save((char*)g_TrackGame->field_0x3448);
            PCStartupDlg* dialog = new(__FILE__, 488) PCStartupDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (_stricmp("Start", event->controlName) == 0 || _stricmp("Practice", event->controlName) == 0) {
            UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
            g_TrackGame->ui->HideScene();
            g_TrackGame->mode.field_0x25ec[0] = 0;
            g_TrackGame->mode.field_0x27f8.field_0x28 = 0;
            if (field_0x7f58)
                field_0x7f58->UpdateBoundValues(1);
            if (field_0x7f5c)
                field_0x7f5c->UpdateBoundValues(1);
            if (field_0x7f60)
                field_0x7f60->UpdateBoundValues(1);
            g_TrackGame->eventManager->ResetEntries();
            if (_stricmp("Start", event->controlName) == 0) {
                g_TrackGame->mode.field_0x27f8.field_0x04 = circuit->field_0x40;
                g_TrackGame->mode.field_0x27f8.field_0x00 = 1;
                g_TrackGame->mode.field_0x27f8.field_0x20 = circuit->field_0x1285[circuit->field_0x40].field_0x0c;
                g_TrackGame->mode.field_0x27f8.field_0x24 = circuit->field_0x460 - 1;
            } else {
                g_TrackGame->mode.field_0x27f8.field_0x04 = circuit->field_0x40;
                g_TrackGame->mode.field_0x27f8.field_0x00 = 0;
                g_TrackGame->mode.field_0x27f8.field_0x24 = 0;
            }
            g_TrackGame->mode.UnknownFunction5240e0(g_TrackGame->mode.field_0x27f8.field_0x04);
            char name[128];
            strcpy(name, circuit->field_0x1285[circuit->field_0x40].field_0x00[circuit->field_0x44 - 1].field_0x00);
            char* extension = strrchr(name, '.');
            if (extension)
                *extension = 0;
            strcpy(g_TrackGame->mode.field_0x27f8.field_0x36, name);
            if (circuit->field_0x40 == 1 || circuit->field_0x40 == 5)
                g_TrackGame->mode.field_0x27f8.field_0x34 = 1;
            else
                g_TrackGame->mode.field_0x27f8.field_0x34 = 0;
            g_TrackGame->mode.field_0x27f8.field_0x10 = 1;
            g_TrackGame->mode.field_0x94 = circuit->field_0x50;
            UnknownFunction4536e0();
            UnknownVirtualSlot26();
        } else if (_stricmp("TabNext", event->controlName) == 0) {
            OpenPage(0);
        } else if (_stricmp("TabBikeRider", event->controlName) == 0) {
            OpenPage(1);
        } else if (_stricmp("TabStandings", event->controlName) == 0) {
            OpenPage(2);
        } else if (_stricmp("Options", event->controlName) == 0) {
            OptionsDlg* dialog = new(__FILE__, 554) OptionsDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, (UIDialog*)this, 0, 0, 1);
        } else if (_stricmp("Controls", event->controlName) == 0) {
            OptionsDlg* dialog = new(__FILE__, 557) OptionsDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, (UIDialog*)this, 1, 0, 1);
        } else if (_stricmp("Help", event->controlName) == 0) {
            g_TrackGame->mode.OpenHelp("MCM2HELP", 0);
        }
        break;
    }
}

// 0x004d6570
void PCCentralDlg::OpenPage(int page)
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
            guiManager->ShowDialog((UIDialog*)field_0x7f58, 0, 1, (int)&area, (UIDialog*)this, 0, 0, 1);
        }
        break;
    case 1:
        if (!field_0x7f5c) {
            field_0x7f5c = new(__FILE__, 590) PCCentralBikeRiderDlg;
            guiManager->ShowDialog((UIDialog*)field_0x7f5c, 0, 1, (int)&area, (UIDialog*)this, 0, 0, 1);
        }
        break;
    case 2:
        if (!field_0x7f60) {
            field_0x7f60 = new(__FILE__, 596) PCCentralStandingsDlg;
            guiManager->ShowDialog((UIDialog*)field_0x7f60, 0, 1, (int)&area, (UIDialog*)this, 0, 0, 1);
        }
        break;
    }
}

// 0x004d7f40
void PCCentralNextDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    if (event->kind != kDialogInit)
        return;
    const char* directory;
    UIControl* picture;
    char text[128];
    char image[128];
    char path[260];
    int purse = (int)(circuit->field_0x1229[circuit->field_0x40] * circuit->field_0x1241);
    sprintf(text, "$%d", circuit->field_0x1229[circuit->field_0x40]);
    FindControl("TxtNextFee", 12)->SetText(text);
    sprintf(text, "$%d", purse);
    FindControl("TxtNextPurse", 12)->SetText(text);
    FindControl("TxtNextTrack", 12)
        ->SetText(circuit->field_0x1285[circuit->field_0x40].field_0x00[circuit->field_0x44 - 1].field_0x04);
    switch (circuit->field_0x40) {
    case 5:
        g_TrackGame->LoadResourceString(0x13f0, text, 128);
        break;
    case 1:
        g_TrackGame->LoadResourceString(0x13ed, text, 128);
        break;
    case 2:
        g_TrackGame->LoadResourceString(0x13ef, text, 128);
        break;
    case 3:
        g_TrackGame->LoadResourceString(0x13f2, text, 128);
        break;
    }
    FindControl("TxtNextType", 12)->SetText(text);
    sprintf(text, "%d/%d", circuit->field_0x44,
            circuit->field_0x1285[circuit->field_0x40].field_0x08 ? circuit->field_0x1285[circuit->field_0x40].field_0x08 - 1
                                                                  : circuit->field_0x1285[circuit->field_0x40].field_0x04);
    FindControl("TxtNextRace", 12)->SetText(text);
    sprintf(text, "$%d", circuit->field_0x465[0].field_0x30);
    FindControl("TxtCash", 12)->SetText(text);
    sprintf(text, "$%d", circuit->field_0x454);
    FindControl("TxtFees", 12)->SetText(text);
    sprintf(text, "$%d", circuit->field_0x465[0].field_0x18.field_0x08);
    FindControl("TxtRepairs", 12)->SetText(text);
    sprintf(text, "$%d", circuit->field_0x465[0].field_0x18.field_0x0c);
    FindControl("TxtMedical", 12)->SetText(text);
    UIControl* announcement = FindControl("TxtAnnouncement", 12);
    announcement->field_0x1e8 = 1;
    announcement->SetText(circuit->field_0x54);

    picture = FindControl("NextTrackPic", 5);
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
    g_TrackGame->mode.FindFileDirectory(
        (int)directory, circuit->field_0x1285[series].field_0x00[circuit->field_0x44 - 1].field_0x00, "env", path);
    g_TrackGame->sceneObject->UnknownFunction4e9b80(path);
    g_TrackGame->sceneObject->UnknownFunction4e9e30(
        circuit->field_0x1285[circuit->field_0x40].field_0x00[circuit->field_0x44 - 1].field_0x00, "scn", 0);
    picture->SetImageFile(0, image);
}

// 0x004d8380
void PCCentralStandingsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    if (event->kind != kDialogInit)
        return;
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    UIListBox* rank = static_cast<UIListBox*>(FindControl("LstRank", 3));
    UIListBox* name = static_cast<UIListBox*>(FindControl("LstName", 3));
    UIListBox* points = static_cast<UIListBox*>(FindControl("LstPoints", 3));
    UIListBox* bike = static_cast<UIListBox*>(FindControl("LstBike", 3));
    UIListBox* cash = static_cast<UIListBox*>(FindControl("LstCash", 3));
    UIListBox* winnings = static_cast<UIListBox*>(FindControl("LstWinnings", 3));
    UIListBox* expenses = static_cast<UIListBox*>(FindControl("LstExpenses", 3));
    char text[128];
    for (int i = 0; i < circuit->field_0x460; i++) {
        _itoa(circuit->field_0x465[i].field_0x18.field_0x10, text, 10);
        rank->AddRow(text, i, 0);
        name->AddRow(circuit->field_0x465[i].field_0xf8, i, 0);
        _itoa(circuit->field_0x465[i].field_0x18.field_0x00, text, 10);
        points->AddRow(text, i, 0);
        bike->AddRow(circuit->field_0x465[i].field_0x118, i, 0);
        sprintf(text, "$%d", circuit->field_0x465[i].field_0x30);
        cash->AddRow(text, i, 0);
        sprintf(text, "$%d", circuit->field_0x465[i].field_0x18.field_0x04 + circuit->field_0x465[i].field_0x18.field_0x14);
        winnings->AddRow(text, i, 0);
        sprintf(text, "$%d", circuit->field_0x465[i].field_0x18.field_0x08 + circuit->field_0x465[i].field_0x18.field_0x0c +
                                 circuit->field_0x454);
        expenses->AddRow(text, i, 0);
    }
    static_cast<UIButton*>(FindControl("ButRank", 1))->SetSortList(rank);
    rank->SetSortCompare(CompareRowNumbersAscending);
    static_cast<UIButton*>(FindControl("ButName", 1))->SetSortList(name);
    static_cast<UIButton*>(FindControl("ButPoints", 1))->SetSortList(points);
    points->SetSortCompare(CompareRowNumbersDescending);
    static_cast<UIButton*>(FindControl("ButBike", 1))->SetSortList(bike);
    static_cast<UIButton*>(FindControl("ButCash", 1))->SetSortList(cash);
    cash->SetSortCompare(CompareCashDescending);
    static_cast<UIButton*>(FindControl("ButWinnings", 1))->SetSortList(winnings);
    winnings->SetSortCompare(CompareCashDescending);
    static_cast<UIButton*>(FindControl("ButExpenses", 1))->SetSortList(expenses);
    expenses->SetSortCompare(CompareCashAscending);
    rank->Sort(1);
}

// 0x004d8660
void PCLastRaceDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    switch (event->kind) {
    case kDialogInit: {
        UIListBox* rank = static_cast<UIListBox*>(FindControl("LstRank", 3));
        UIListBox* name = static_cast<UIListBox*>(FindControl("LstName", 3));
        UIListBox* points = static_cast<UIListBox*>(FindControl("LstPoints", 3));
        UIListBox* bike = static_cast<UIListBox*>(FindControl("LstBike", 3));
        UIListBox* winnings = static_cast<UIListBox*>(FindControl("LstWinnings", 3));
        UIListBox* repairs = static_cast<UIListBox*>(FindControl("LstRepairs", 3));
        UIListBox* medical = static_cast<UIListBox*>(FindControl("LstMedical", 3));
        PayOutRace();
        char text[128];
        for (int i = 0; i < g_TrackGame->field_0x3444->field_0x460; i++) {
            _itoa(g_TrackGame->field_0x3444->field_0x465[i].field_0x00.field_0x10, text, 10);
            rank->AddRow(text, i, 0);
            name->AddRow(g_TrackGame->field_0x3444->field_0x465[i].field_0xf8, i, 0);
            _itoa(g_TrackGame->field_0x3444->field_0x465[i].field_0x00.field_0x00, text, 10);
            points->AddRow(text, i, 0);
            bike->AddRow(g_TrackGame->field_0x3444->field_0x465[i].field_0x118, i, 0);
            sprintf(text, "$%d", g_TrackGame->field_0x3444->field_0x465[i].field_0x00.field_0x04);
            winnings->AddRow(text, i, 0);
            sprintf(text, "$%d", g_TrackGame->field_0x3444->field_0x465[i].field_0x00.field_0x08);
            repairs->AddRow(text, i, 0);
            sprintf(text, "$%d", g_TrackGame->field_0x3444->field_0x465[i].field_0x00.field_0x0c);
            medical->AddRow(text, i, 0);
        }
        static_cast<UIButton*>(FindControl("ButRank", 1))->SetSortList(rank);
        rank->SetSortCompare(CompareRowNumbersAscending);
        static_cast<UIButton*>(FindControl("ButName", 1))->SetSortList(name);
        static_cast<UIButton*>(FindControl("ButPoints", 1))->SetSortList(points);
        points->SetSortCompare(CompareRowNumbersDescending);
        static_cast<UIButton*>(FindControl("ButBike", 1))->SetSortList(bike);
        static_cast<UIButton*>(FindControl("ButWinnings", 1))->SetSortList(winnings);
        winnings->SetSortCompare(CompareCashDescending);
        static_cast<UIButton*>(FindControl("ButRepairs", 1))->SetSortList(repairs);
        repairs->SetSortCompare(CompareCashAscending);
        static_cast<UIButton*>(FindControl("ButMedical", 1))->SetSortList(medical);
        medical->SetSortCompare(CompareCashAscending);
        rank->Sort(1);
        g_TrackGame->field_0x3444->AdvanceAfterRace();
        g_TrackGame->field_0x3444->Save((char*)g_TrackGame->field_0x3448);
        break;
    }
    case kDialogCommand:
        if (_stricmp("Back", event->controlName) == 0) {
            int bonus = circuit->field_0x44 == circuit->field_0x1285[circuit->field_0x40].field_0x08;
            if (bonus && circuit->field_0x465[0].field_0x18.field_0x10 == 1) {
                PCBonusTrackDlg* dialog = new(__FILE__, 1368) PCBonusTrackDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            } else if (circuit->field_0x465[0].field_0x30 <
                       circuit->field_0x1229[g_TrackGame->field_0x3444->field_0x40]) {
                if (!(circuit->field_0x464 & 1)) {
                    PCBailoutDlg* dialog = new(__FILE__, 1374) PCBailoutDlg;
                    guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
                    circuit->field_0x464 |= 1;
                } else if (!(circuit->field_0x464 & 2)) {
                    PCBunnyDlg* dialog = new(__FILE__, 1378) PCBunnyDlg;
                    guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
                    circuit->field_0x464 |= 2;
                } else {
                    circuit->field_0x464 |= 4;
                    PCFailedDlg* dialog = new(__FILE__, 1383) PCFailedDlg;
                    guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
                }
            } else {
                OpenCareerDialog();
            }
            EndDialog(0);
        }
        break;
    }
}

// 0x004d8c20
int CompareRankPoints(const void* a, const void* b)
{
    int first = ((const UnknownProCircuitRank*)a)->points;
    int second = ((const UnknownProCircuitRank*)b)->points;
    if (first > second)
        return -1;
    return first != second;
}

// 0x004d9340
void PCFailedDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UIControl* label;
    switch (event->kind) {
    case kDialogInit: {
        g_TrackGame->field_0x3444->Save((char*)g_TrackGame->field_0x3448);
        UIControl* description = FindControl("TxtDescription", 12);
        description->field_0x1e8 = 1;
        description->SetTextFromResource(g_TrackGame->resourceInstance, 0x1464);
        label = FindControl("TitleText", 12);
        label->SetTextFromResource(g_TrackGame->resourceInstance, 0x1467);
        label = FindControl("ButDecline", 1);
        label->Show(0, 1);
        label = FindControl("ButAccept", 1);
        label->SetTextFromResource(g_TrackGame->resourceInstance, 0x144e);
        UIControl* picture = FindControl("Pic", 5);
        picture->SetImage(0, FindSectionObject("FailScreen"));
        break;
    }
    case kDialogCommand: {
        PCFinishedDlg* dialog = new(__FILE__, 1593) PCFinishedDlg;
        guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
        EndDialog(0);
        break;
    }
    }
}

// 0x004d94e0
void PCFinishedDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    switch (event->kind) {
    case kDialogInit: {
        UIListBox* rank = static_cast<UIListBox*>(FindControl("LstRank", 3));
        UIListBox* name = static_cast<UIListBox*>(FindControl("LstName", 3));
        UIListBox* points = static_cast<UIListBox*>(FindControl("LstPoints", 3));
        UIListBox* bike = static_cast<UIListBox*>(FindControl("LstBike", 3));
        UIListBox* cash = static_cast<UIListBox*>(FindControl("LstCash", 3));
        UIListBox* repairs = static_cast<UIListBox*>(FindControl("LstRepairs", 3));
        UIListBox* medical = static_cast<UIListBox*>(FindControl("LstMedical", 3));
        char text[128];
        for (int i = 0; i < circuit->field_0x460; i++) {
            _itoa(circuit->field_0x465[i].field_0x18.field_0x10, text, 10);
            rank->AddRow(text, i, 0);
            name->AddRow(circuit->field_0x465[i].field_0xf8, i, 0);
            _itoa(circuit->field_0x465[i].field_0x18.field_0x00, text, 10);
            points->AddRow(text, i, 0);
            bike->AddRow(circuit->field_0x465[i].field_0x118, i, 0);
            sprintf(text, "$%d", circuit->field_0x465[i].field_0x30);
            cash->AddRow(text, i, 0);
            sprintf(text, "$%d", circuit->field_0x465[i].field_0x18.field_0x08);
            repairs->AddRow(text, i, 0);
            sprintf(text, "$%d", circuit->field_0x465[i].field_0x18.field_0x0c);
            medical->AddRow(text, i, 0);
        }
        static_cast<UIButton*>(FindControl("ButRank", 1))->SetSortList(rank);
        rank->SetSortCompare(CompareRowNumbersAscending);
        static_cast<UIButton*>(FindControl("ButName", 1))->SetSortList(name);
        static_cast<UIButton*>(FindControl("ButPoints", 1))->SetSortList(points);
        points->SetSortCompare(CompareRowNumbersDescending);
        static_cast<UIButton*>(FindControl("ButBike", 1))->SetSortList(bike);
        static_cast<UIButton*>(FindControl("ButCash", 1))->SetSortList(cash);
        cash->SetSortCompare(CompareCashDescending);
        static_cast<UIButton*>(FindControl("ButRepairs", 1))->SetSortList(repairs);
        repairs->SetSortCompare(CompareCashAscending);
        static_cast<UIButton*>(FindControl("ButMedical", 1))->SetSortList(medical);
        medical->SetSortCompare(CompareCashAscending);
        rank->Sort(1);
        break;
    }
    case kDialogCommand:
        if (_stricmp("Back", event->controlName) == 0) {
            PCStartupDlg* dialog = new(__FILE__, 1659) PCStartupDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        }
        break;
    }
}

// 0x004d9fd0
void PCNewEventDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    switch (event->kind) {
    case kDialogInit: {
        int description;
        const char* image;
        switch (circuit->field_0x40) {
        case 5:
            description = 0x1465;
            image = "WinScreen";
            break;
        case 1:
            description = 0x1454;
            image = "BajaScreen";
            break;
        case 2:
            description = 0x1455;
            image = "NatScreen";
            break;
        case 3:
            description = 0x1456;
            image = "SuperScreen";
            break;
        }
        UIControl* control = FindControl("TxtDescription", 12);
        control->field_0x1e8 = 1;
        UnknownSetText(control, description);
        UnknownSetText(FindControl("TitleText", 12), 0x1467);
        control = FindControl("ButDecline", 1);
        control->Show(0, 1);
        UnknownSetText(FindControl("ButAccept", 1), 0x916);
        control = FindControl("Pic", 5);
        control->SetImage(0, FindSectionObject(image));
        break;
    }
    case kDialogCommand: {
        PCCentralDlg* dialog = new(__FILE__, 1896) PCCentralDlg;
        guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
        EndDialog(0);
        break;
    }
    }
}

// 0x004da1b0
void PCCompleteDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    switch (event->kind) {
    case kDialogInit: {
        char rank[1024];
        char text[1024];
        char message[2048];
        g_TrackGame->field_0x3444->Save((char*)g_TrackGame->field_0x3448);
        UnknownSetText(FindControl("TitleText", 12), 0x145d);
        UIControl* description = FindControl("TxtDescription", 12);
        g_TrackGame->LoadResourceString(0x145e, text, 1023);
        strcpy(rank, "");
        if (circuit->field_0x465[0].field_0x18.field_0x10 == 1)
            g_TrackGame->LoadResourceString(0x145f, rank, 1023);
        sprintf(message, "%s\n%s", text, rank);
        description->field_0x1e8 = 1;
        description->SetText(message);
        UIControl* control = FindControl("ButDecline", 1);
        control->Show(0, 1);
        UnknownSetText(FindControl("ButAccept", 1), 0x144e);
        control = FindControl("Pic", 5);
        control->SetImage(0, FindSectionObject("WinScreen"));
        break;
    }
    case kDialogCommand: {
        PCFinishedDlg* dialog = new(__FILE__, 1941) PCFinishedDlg;
        guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
        EndDialog(0);
        break;
    }
    }
}

// 0x004d71f0
void PCCentralBikeRiderDlg::PaintPlateNumber(int number)
{
    UnknownBikeNumberPainter painter(g_TrackGame->field_0x1c);
    for (int i = 0; i < g_TrackGame->ui->field_0x4c; i++)
        painter.UnknownFunction417670(
            ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[i].field_0xc0->plateTexture, number);
}

// 0x004d72a0
void PCCentralBikeRiderDlg::UnknownVirtualSlot26()
{
    g_TrackGame->ui->HideScene();
    UIDialog::UnknownVirtualSlot26();
}

// 0x004d78c0
void PCCentralBikeRiderDlg::ApplyChosenRider()
{
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
    field_0x7f88 = 1;
    g_TrackGame->field_0x3444->field_0x458 = list->GetRowData(-1);
}

// 0x004d7900: shows the chosen bike and loads its class's garage defaults.
void PCCentralBikeRiderDlg::ApplyChosenBike()
{
    char text[12];
    UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    KrustyUI* ui = g_TrackGame->ui;
    UnknownKrustyUIBike* bike = &((UnknownKrustyUIBike*)ui->field_0x50)[bikes->GetRowData(-1)];
    UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)ui->field_0x48)[bike->model];
    int i;
    // Retail re-reads TrackGame's KrustyUI after each model is hidden.
    for (i = 0; i < ui->field_0x4c; i++) {
        ((UnknownKrustyUIModel*)ui->field_0x48)[i].field_0xc0->UnknownVirtualSlot4();
        ui = g_TrackGame->ui;
    }
    model->field_0xc0->UnknownVirtualSlot5();
    field_0x7f84 = 1;
    if (bike->field_0x88) {
        UIControl* engine = FindControl("DDLEngineSize", 0);
        if (!engine->field_0x70) {
            engine->Show(1, 1);
            FindControl("TxtEngineSize", 0)->Show(1, 1);
            if (dialogBackground)
                dialogBackground->UnknownFunction404da0();
        }
    } else {
        UIControl* engine = FindControl("DDLEngineSize", 0);
        if (engine->field_0x70) {
            engine->Show(0, 1);
            FindControl("TxtEngineSize", 0)->Show(0, 1);
            if (dialogBackground)
                dialogBackground->UnknownFunction404da0();
        }
    }
    g_TrackGame->field_0x3444->field_0x45c = bikes->GetRowData(-1);
    if (bike->field_0x88) {
        UIListBox* sizes = static_cast<UIDropDownList*>(FindControl("DDLEngineSize", 6))->listPart;
        int size = sizes->GetRowData(-1);
        g_TrackGame->field_0x3444->field_0x465[0].field_0x138 = g_UnknownGlobal56cb6c[size];
        g_TrackGame->field_0x3444->field_0x465[0].field_0x13c = size == 2 || size == 4 ? 1 : 0;
        UNKNOWN_APPLY_BIKE_CLASS(size, i);
    } else {
        int row = bikes->GetRowData(-1);
        g_TrackGame->field_0x3444->field_0x465[0].field_0x138 =
            ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[row].engineSize;
        g_TrackGame->field_0x3444->field_0x465[0].field_0x13c =
            ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[row].field_0x90;
        int bikeClass = UnknownBikeClassOf(g_TrackGame->field_0x3444->field_0x465[0].field_0x138);
        UNKNOWN_APPLY_BIKE_CLASS(bikeClass, i);
    }
    UNKNOWN_GARAGE_SETTINGS->engineSize = g_TrackGame->field_0x3444->field_0x465[0].field_0x138;
    UNKNOWN_GARAGE_SETTINGS->field_0x04 = g_TrackGame->field_0x3444->field_0x465[0].field_0x13c;
    UIEditBox* plate = static_cast<UIEditBox*>(FindControl("EditPlateNumber", 0xb));
    _itoa(g_TrackGame->mode.field_0x1bcc, text, 10);
    plate->SetEditText(text);
}

// 0x004d7d70
void PCCentralBikeRiderDlg::UnknownVirtualSlot31(int apply)
{
    if (!apply)
        return;
    strcpy(g_TrackGame->field_0x3444->field_0x465[0].field_0x38, "");
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    int bike = list->GetRowData(-1);
    strcpy(g_TrackGame->field_0x3444->field_0x465[0].field_0x38,
           ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)
               [((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[bike].model].modelName);
    strcpy(g_TrackGame->field_0x3444->field_0x465[0].field_0x78,
           ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[bike].field_0x48);
    strcpy(g_TrackGame->field_0x3444->field_0x465[0].field_0x118,
           ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[bike].displayName);
    strcpy(g_TrackGame->field_0x3444->field_0x465[0].field_0xb8, "");
    list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
    int rider = list->GetRowData(-1);
    strcpy(g_TrackGame->field_0x3444->field_0x465[0].field_0xb8,
           ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[rider].modelName);
}

// 0x004d6770: as SPBikeRiderDlg slot 29 (dlgprocs.cpp 0x0044ef70); the
// engine sizes offered follow the circuit's bike class rule.
void PCCentralBikeRiderDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    char plate[12];
    char typed[12];
    switch (event->kind) {
    case kDialogInit: {
        field_0x7f78 = 0;
        static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->SetCurrentState(1);
        field_0x7f88 = 1;
        field_0x7f84 = 1;
        FillBikeRiderLists();
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLEngineSize", 6))->listPart;
        list->RemoveAllRows();
        if (g_TrackGame->field_0x3444->field_0x4c == 1 || g_TrackGame->field_0x3444->field_0x4c == 3)
            list->AddRow("125cc 2-stroke", UnknownBikeClassOf(125), 0);
        if (g_TrackGame->field_0x3444->field_0x4c == 2 || g_TrackGame->field_0x3444->field_0x4c == 3) {
            list->AddRow("250cc 2-stroke", UnknownBikeClassOf(250), 0);
            list->AddRow("400cc 4-stroke", UnknownBikeClassOf(400), 0);
        }
        if (g_TrackGame->field_0x3444->field_0x4c == 3) {
            list->AddRow("500cc 2-stroke", UnknownBikeClassOf(500), 0);
            list->AddRow("600cc 4-stroke", UnknownBikeClassOf(600), 0);
        }
        list->SelectRowByData(UnknownBikeClassOf(g_TrackGame->field_0x3444->field_0x465[0].field_0x138));
        UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditPlateNumber", 0xb));
        edit->SetAcceptedCharacters("0123456789");
        g_TrackGame->ui->ShowScene(this);
        Vector3* eye = &field_0x7f58;
        field_0x7f8c.left = 0x52;
        field_0x7f8c.right = 0x216;
        field_0x7f8c.top = 0x74;
        field_0x7f8c.bottom = 0x150;
        *eye = kVec3Zero;
        float fov = 50.0f;
        Vector3* target = &field_0x7f64;
        field_0x7f58.z = 15.0f;
        field_0x7f58.y = 1.0f;
        *target = g_TrackGame->ui->field_0x474;
        field_0x7f64.y += 3.0f;
        g_TrackGame->ui->field_0x468->UnknownFunction42e9b0(eye, 0, 0, 0, &fov);
        g_TrackGame->ui->field_0x468->UnknownVirtualSlot29(*target);
        g_TrackGame->ui->field_0x468->UnknownFunction42f190(
            field_0x7f8c.left, field_0x7f8c.top, field_0x7f8c.right - field_0x7f8c.left,
            field_0x7f8c.bottom - field_0x7f8c.top);
        field_0x7f70 = UnknownVectorLength(UnknownVectorDifference(*eye, g_TrackGame->ui->field_0x474));
        ApplyChosenRider();
        ApplyChosenBike();
        PaintPlateNumber(g_TrackGame->mode.field_0x1bcc);
        srand(ReadClock());
        if (dialogBackground)
            field_0x7f74 = dialogBackground->UnknownFunction4040f0(0);
        break;
    }
    case kDialogListSelect:
        if (!_stricmp("DDLBikes", event->controlName))
            ApplyChosenBike();
        else if (!_stricmp("DDLRiders", event->controlName))
            ApplyChosenRider();
        else if (!_stricmp("DDLEngineSize", event->controlName))
            ApplyChosenBike();
        break;
    case kDialogCommand:
        if (!_stricmp("BikeLeft", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("BikeRight", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RiderLeft", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RiderRight", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("ChkAutoRotate", event->controlName)) {
            field_0x7f78 = !static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->UnknownFunction4755c0();
        } else if (!_stricmp("ButWrench", event->controlName)) {
            OptionsDlg* dialog = new(__FILE__, 719) OptionsDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, parentDialog, 2, 0, 1);
        }
        break;
    case kDialogEditDone:
        if (!_stricmp("EditPlateNumber", event->controlName)) {
            static_cast<UIEditBox*>(event->control)->GetEditText(plate, 9);
            int number = atoi(plate);
            if (number < 100)
                number += 100;
            if (number >= 101) {
                if (number > 999)
                    number = 999;
            } else {
                number = 101;
            }
            g_TrackGame->mode.field_0x1bcc = number;
            _itoa(number, plate, 10);
            static_cast<UIEditBox*>(event->control)->SetEditText(plate);
            PaintPlateNumber(number);
        }
        break;
    case kDialogEditChange:
        if (!_stricmp("EditPlateNumber", event->controlName)) {
            static_cast<UIEditBox*>(event->control)->GetEditText(typed, 9);
            int number = atoi(typed);
            if (number >= 100 && number <= 999) {
                PaintPlateNumber(number);
                g_TrackGame->mode.field_0x1bcc = number;
            }
        }
        break;
    case kDialogClose:
        g_TrackGame->ui->HideScene();
        if (dialogBackground)
            dialogBackground->UnknownFunction404200(field_0x7f74);
        ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(0);
        break;
    }
}


// 0x004d77d0
int PCCentralBikeRiderDlg::UnknownVirtualSlot13()
{
    if (field_0x7f84) {
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        int bike = list->GetRowData(-1);
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)ui->field_0x50)[bike];
        ((UnknownProCircuitSkinned*)((UnknownKrustyUIModel*)ui->field_0x48)[entry->model].field_0xc0->plateTexture)
            ->UnknownFunction444c70(0, entry->field_0x48, &g_TrackGame->field_0x1c);
        field_0x7f84 = 0;
    }
    if (field_0x7f88) {
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        UnknownKrustyUIModel* rider = &((UnknownKrustyUIModel*)ui->field_0x58)[list->GetRowData(-1)];
        ((UnknownProCircuitSkinned*)ui->field_0x46c->plateTexture)
            ->UnknownFunction444c70(0, rider->modelName, &g_TrackGame->field_0x1c);
        field_0x7f88 = 0;
    }
    return UIDialog::UnknownVirtualSlot13();
}
