#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ProCircuitProcs.h"

#include "DialogEventKind.h"
#include "SelectGamePicProcs.h"
#include "KrustyUI.h"
#include "MenuId.h"
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
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    if (circuit->careerFlags & 8) {
        PCCompleteDlg* dialog = new(__FILE__, 78) PCCompleteDlg;
        g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
    } else if (circuit->raceIndex == 1) {
        PCNewEventDlg* dialog = new(__FILE__, 80) PCNewEventDlg;
        g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
    } else {
        PCCentralDlg* dialog = new(__FILE__, 82) PCCentralDlg;
        g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
    }
}

// 0x004d4d20
void PCStartupDlg::HandleEvent(UnknownDialogEvent* event)
{
    switch (event->kind) {
    case kDialogInit:
        careerFileCount = 0;
        careerFiles = 0;
        FillLists();
        EnableButtons();
        deleteRunningChoice = 0;
        deleteFinishedChoice = 0;
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
            dialog->SetAnswerTargets(newCareerName, &chosenDifficulty, &chosenBikeClass, &chosenOpponentsRow);
        } else if (_stricmp("ButContinue", event->controlName) == 0) {
            int row = static_cast<UIListBox*>(FindControl("LstRank", 3))->GetRowData(-1);
            g_TrackGame->proCircuit = new(__FILE__, 142) UnknownTrackGameObject3444;
            strcpy(g_TrackGame->careerFile, careerFiles[row]);
            if (g_TrackGame->proCircuit->LoadSaved(g_TrackGame->careerFile)) {
                PCCentralDlg* dialog = new(__FILE__, 146) PCCentralDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            }
            EndDialog(0);
        } else if (_stricmp("ButDelete", event->controlName) == 0) {
            deleteFinishedChoice = 0;
            deleteRunningChoice = new(__FILE__, 152) ChoiceDlg;
            guiManager->ShowDialog((UIDialog*)deleteRunningChoice, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
            deleteRunningChoice->SetTextsOrResources(0, 0x143c, 0, 0, 0, 0x143e, 0, 0, 0, 0x143d);
        } else if (_stricmp("ButDoneDelete", event->controlName) == 0) {
            deleteRunningChoice = 0;
            deleteFinishedChoice = new(__FILE__, 162) ChoiceDlg;
            guiManager->ShowDialog((UIDialog*)deleteFinishedChoice, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
            deleteFinishedChoice->SetTextsOrResources(0, 0x143c, 0, 0, 0, 0x143e, 0, 0, 0, 0x143d);
        } else if (_stricmp("Back", event->controlName) == 0) {
            g_TrackGame->ui->OpenMenu(kMenuMain);
            EndDialog(0);
        }
        break;
    case 9:
        if (event->code == 0x3e9) {
            g_TrackGame->proCircuit = new(__FILE__, 182) UnknownTrackGameObject3444;
            int difficulty;
            switch (chosenDifficulty) {
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
            g_TrackGame->proCircuit->StartCareer(newCareerName, difficulty, chosenBikeClass,
                                                                        chosenOpponentsRow + 1);
            char path[128];
            // The first free CircuitNNN.pc.
            int number = 1;
            sprintf(path, "%s\\%s\\Circuit%03d.pc", "ui\\profile", g_TrackGame->mode.field_0x00, number);
            while (UnknownFileExists(path))
                sprintf(path, "%s\\%s\\Circuit%03d.pc", "ui\\profile", g_TrackGame->mode.field_0x00, ++number);
            strcpy(g_TrackGame->careerFile, path);
            g_TrackGame->proCircuit->Save(g_TrackGame->careerFile);
            EndDialog(0);
            PCNewEventDlg* dialog = new(__FILE__, 214) PCNewEventDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
        } else if (event->code == 0x65) {
            if (deleteRunningChoice)
                DeleteFileA(careerFiles[static_cast<UIListBox*>(FindControl("LstRank", 3))->GetRowData(-1)]);
            else
                DeleteFileA(careerFiles[static_cast<UIListBox*>(FindControl("LstDoneRank", 3))->GetRowData(-1)]);
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
                careerFiles = (char**)ResizeDebugBlock(careerFiles, (careerFileCount + 1) * 4);
                careerFiles[careerFileCount] = (char*)DebugMalloc(strlen(path) + 1, __FILE__, 297);
                strcpy(careerFiles[careerFileCount], path);
                AddCircuitRow(&circuit, careerFileCount);
                careerFileCount++;
            }
        } while (FindNextFileA(find, &data));
        FindClose(find);
    }
}

// 0x004d59a0
void PCStartupDlg::AddCircuitRow(UnknownTrackGameObject3444* circuit, int row)
{
    char name[64];
    char text[128];
    int finished = (circuit->careerFlags >> 3) & 1;
    int failed = (circuit->careerFlags >> 2) & 1;
    const char* prefix = finished || failed ? "LstDone" : "Lst";

    sprintf(name, "%s%s", prefix, "Name");
    static_cast<UIListBox*>(FindControl(name, 3))->AddRow(circuit->careerName, row, 0);

    sprintf(name, "%s%s", prefix, "Rank");
    sprintf(text, "%d / %d", circuit->racers[0].field_0x18.field_0x10, circuit->racerCount);
    static_cast<UIListBox*>(FindControl(name, 3))->AddRow(text, row, 0);

    sprintf(name, "%s%s", prefix, "Class");
    switch (circuit->bikeClassRule) {
    case 1:
        strcpy(text, "125");
        break;
    case 2:
        strcpy(text, "250");
        break;
    case 3:
        g_TrackGame->LoadResourceString(0x14cf, text, 127);
        break;
    }
    static_cast<UIListBox*>(FindControl(name, 3))->AddRow(text, row, 0);

    sprintf(name, "%s%s", prefix, "Diff");
    switch (circuit->field_0x50) {
    case 1:
        g_TrackGame->LoadResourceString(0x14cb, text, 127);
        break;
    case 3:
        g_TrackGame->LoadResourceString(0x14cd, text, 127);
        break;
    default:
        g_TrackGame->LoadResourceString(0x14cc, text, 127);
        break;
    }
    static_cast<UIListBox*>(FindControl(name, 3))->AddRow(text, row, 0);

    sprintf(name, "%s%s", prefix, "Points");
    _itoa(circuit->racers[0].field_0x18.field_0x00, text, 10);
    static_cast<UIListBox*>(FindControl(name, 3))->AddRow(text, row, 0);

    sprintf(name, "%s%s", prefix, "Cash");
    sprintf(text, "$%d", circuit->racers[0].money);
    static_cast<UIListBox*>(FindControl(name, 3))->AddRow(text, row, 0);

    if (finished || failed) {
        sprintf(name, "%s%s", prefix, "Status");
        if (failed)
            g_TrackGame->LoadResourceString(0x1442, text, 128);
        else if (finished)
            g_TrackGame->LoadResourceString(0x1443, text, 128);
        static_cast<UIListBox*>(FindControl(name, 3))->AddRow(text, row, 0);
    }
}


// 0x004d5ca0
void PCStartupDlg::FreeFileNames()
{
    if (careerFiles) {
        for (int i = 0; i < careerFileCount; i++) {
            if (careerFiles[i])
                DebugFree(careerFiles[i], __FILE__, 379);
        }
        DebugFree(careerFiles, __FILE__, 382);
    }
    careerFileCount = 0;
    careerFiles = 0;
}

// 0x004d5d20
void PCNewDlg::HandleEvent(UnknownDialogEvent* event)
{
    char number[32];
    char text[128];
    switch (event->kind) {
    case kDialogEditChange:
    {
        UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditBox", 11));
        edit->GetEditText(text, 127);
        if (!text[0])
            FindControl("OkButton", 0)->SetEnabled(0);
        else
            FindControl("OkButton", 0)->SetEnabled(1);
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
        guiUser->RequestFocus((UnknownGuiControl*)FindControl("EditBox", 11), 0, 0);
        break;
    }
    case kDialogCommand:
        if (_stricmp("OkButton", event->controlName) == 0) {
            static_cast<UIEditBox*>(FindControl("EditBox", 11))->GetEditText(careerName, 64);
            *bikeClassChoice = static_cast<UIRadioButton*>(FindControl("RadClass125", 4))->GetGroupSelection();
            *difficultyChoice = static_cast<UIRadioButton*>(FindControl("RadLODEasy", 4))->GetGroupSelection();
            *opponentsChoice = static_cast<UIListBox*>(FindControl("OpponentsListBox", 3))->GetRowData(-1);
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
    careerName = name;
    difficultyChoice = difficulty;
    bikeClassChoice = bikeClass;
    opponentsChoice = opponents;
}

// 0x004d5f70
void PCCentralDlg::HandleEvent(UnknownDialogEvent* event)
{
    switch (event->kind) {
    case kDialogClose:
        if (parentDialog)
            parentDialog->SetShown(1);
        break;
    case kDialogInit: {
        nextPage = 0;
        bikeRiderPage = 0;
        standingsPage = 0;
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
            g_TrackGame->proCircuit->Save(g_TrackGame->careerFile);
            PCStartupDlg* dialog = new(__FILE__, 488) PCStartupDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (_stricmp("Start", event->controlName) == 0 || _stricmp("Practice", event->controlName) == 0) {
            UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
            g_TrackGame->ui->HideScene();
            g_TrackGame->mode.ghostFile[0] = 0;
            g_TrackGame->mode.raceSettings.field_0x28 = 0;
            if (nextPage)
                nextPage->UpdateBoundValues(1);
            if (bikeRiderPage)
                bikeRiderPage->UpdateBoundValues(1);
            if (standingsPage)
                standingsPage->UpdateBoundValues(1);
            g_TrackGame->eventManager->ResetEntries();
            if (_stricmp("Start", event->controlName) == 0) {
                g_TrackGame->mode.raceSettings.eventType = circuit->series;
                g_TrackGame->mode.raceSettings.raceMode = 1;
                g_TrackGame->mode.raceSettings.lapCount = circuit->schedules[circuit->series].laps;
                g_TrackGame->mode.raceSettings.aiRacerCount = circuit->racerCount - 1;
            } else {
                g_TrackGame->mode.raceSettings.eventType = circuit->series;
                g_TrackGame->mode.raceSettings.raceMode = 0;
                g_TrackGame->mode.raceSettings.aiRacerCount = 0;
            }
            g_TrackGame->mode.SelectSeries(g_TrackGame->mode.raceSettings.eventType);
            char name[128];
            strcpy(name, circuit->schedules[circuit->series].tracks[circuit->raceIndex - 1].trackKey);
            char* extension = strrchr(name, '.');
            if (extension)
                *extension = 0;
            strcpy(g_TrackGame->mode.raceSettings.trackName, name);
            if (circuit->series == 1 || circuit->series == 5)
                g_TrackGame->mode.raceSettings.field_0x34 = 1;
            else
                g_TrackGame->mode.raceSettings.field_0x34 = 0;
            g_TrackGame->mode.raceSettings.treeCollision = 1;
            g_TrackGame->mode.field_0x94 = circuit->field_0x50;
            LeaveMenusForRace();
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
    if (nextPage && page != 0) {
        nextPage->Release();
        nextPage = 0;
    }
    if (bikeRiderPage && page != 1) {
        bikeRiderPage->Release();
        bikeRiderPage = 0;
    }
    if (standingsPage && page != 2) {
        standingsPage->Release();
        standingsPage = 0;
    }
    switch (page) {
    case 0:
        if (!nextPage) {
            nextPage = new(__FILE__, 584) PCCentralNextDlg;
            guiManager->ShowDialog((UIDialog*)nextPage, 0, 1, (int)&area, (UIDialog*)this, 0, 0, 1);
        }
        break;
    case 1:
        if (!bikeRiderPage) {
            bikeRiderPage = new(__FILE__, 590) PCCentralBikeRiderDlg;
            guiManager->ShowDialog((UIDialog*)bikeRiderPage, 0, 1, (int)&area, (UIDialog*)this, 0, 0, 1);
        }
        break;
    case 2:
        if (!standingsPage) {
            standingsPage = new(__FILE__, 596) PCCentralStandingsDlg;
            guiManager->ShowDialog((UIDialog*)standingsPage, 0, 1, (int)&area, (UIDialog*)this, 0, 0, 1);
        }
        break;
    }
}

// 0x004d7f40
void PCCentralNextDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    if (event->kind != kDialogInit)
        return;
    const char* directory;
    UIControl* picture;
    char text[128];
    char image[128];
    char path[260];
    int purse = (int)(circuit->entryFees[circuit->series] * circuit->purseMultiplier);
    sprintf(text, "$%d", circuit->entryFees[circuit->series]);
    FindControl("TxtNextFee", 12)->SetText(text);
    sprintf(text, "$%d", purse);
    FindControl("TxtNextPurse", 12)->SetText(text);
    FindControl("TxtNextTrack", 12)
        ->SetText(circuit->schedules[circuit->series].tracks[circuit->raceIndex - 1].displayName);
    switch (circuit->series) {
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
    sprintf(text, "%d/%d", circuit->raceIndex,
            circuit->schedules[circuit->series].bonusTrack ? circuit->schedules[circuit->series].bonusTrack - 1
                                                                  : circuit->schedules[circuit->series].trackCount);
    FindControl("TxtNextRace", 12)->SetText(text);
    sprintf(text, "$%d", circuit->racers[0].money);
    FindControl("TxtCash", 12)->SetText(text);
    sprintf(text, "$%d", circuit->field_0x454);
    FindControl("TxtFees", 12)->SetText(text);
    sprintf(text, "$%d", circuit->racers[0].field_0x18.field_0x08);
    FindControl("TxtRepairs", 12)->SetText(text);
    sprintf(text, "$%d", circuit->racers[0].field_0x18.field_0x0c);
    FindControl("TxtMedical", 12)->SetText(text);
    UIControl* announcement = FindControl("TxtAnnouncement", 12);
    announcement->wordWrap = 1;
    announcement->SetText(circuit->messageText);

    picture = FindControl("NextTrackPic", 5);
    strcpy(image, circuit->schedules[circuit->series].tracks[circuit->raceIndex - 1].keyCopy);
    char* extension = strrchr(image, '.');
    if (extension)
        *extension = 0;
    int series = circuit->series;
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
        (int)directory, circuit->schedules[series].tracks[circuit->raceIndex - 1].trackKey, "env", path);
    g_TrackGame->sceneObject->SetSceneDirectory(path);
    g_TrackGame->sceneObject->SelectScene(
        circuit->schedules[circuit->series].tracks[circuit->raceIndex - 1].trackKey, "scn", 0);
    picture->SetImageFile(0, image);
}

// 0x004d8380
void PCCentralStandingsDlg::HandleEvent(UnknownDialogEvent* event)
{
    if (event->kind != kDialogInit)
        return;
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    UIListBox* rank = static_cast<UIListBox*>(FindControl("LstRank", 3));
    UIListBox* name = static_cast<UIListBox*>(FindControl("LstName", 3));
    UIListBox* points = static_cast<UIListBox*>(FindControl("LstPoints", 3));
    UIListBox* bike = static_cast<UIListBox*>(FindControl("LstBike", 3));
    UIListBox* cash = static_cast<UIListBox*>(FindControl("LstCash", 3));
    UIListBox* winnings = static_cast<UIListBox*>(FindControl("LstWinnings", 3));
    UIListBox* expenses = static_cast<UIListBox*>(FindControl("LstExpenses", 3));
    char text[128];
    for (int i = 0; i < circuit->racerCount; i++) {
        _itoa(circuit->racers[i].field_0x18.field_0x10, text, 10);
        rank->AddRow(text, i, 0);
        name->AddRow(circuit->racers[i].racerName, i, 0);
        _itoa(circuit->racers[i].field_0x18.field_0x00, text, 10);
        points->AddRow(text, i, 0);
        bike->AddRow(circuit->racers[i].bikeDisplayName, i, 0);
        sprintf(text, "$%d", circuit->racers[i].money);
        cash->AddRow(text, i, 0);
        sprintf(text, "$%d", circuit->racers[i].field_0x18.field_0x04 + circuit->racers[i].field_0x18.field_0x14);
        winnings->AddRow(text, i, 0);
        sprintf(text, "$%d", circuit->racers[i].field_0x18.field_0x08 + circuit->racers[i].field_0x18.field_0x0c +
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
void PCLastRaceDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
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
        for (int i = 0; i < g_TrackGame->proCircuit->racerCount; i++) {
            _itoa(g_TrackGame->proCircuit->racers[i].field_0x00.field_0x10, text, 10);
            rank->AddRow(text, i, 0);
            name->AddRow(g_TrackGame->proCircuit->racers[i].racerName, i, 0);
            _itoa(g_TrackGame->proCircuit->racers[i].field_0x00.field_0x00, text, 10);
            points->AddRow(text, i, 0);
            bike->AddRow(g_TrackGame->proCircuit->racers[i].bikeDisplayName, i, 0);
            sprintf(text, "$%d", g_TrackGame->proCircuit->racers[i].field_0x00.field_0x04);
            winnings->AddRow(text, i, 0);
            sprintf(text, "$%d", g_TrackGame->proCircuit->racers[i].field_0x00.field_0x08);
            repairs->AddRow(text, i, 0);
            sprintf(text, "$%d", g_TrackGame->proCircuit->racers[i].field_0x00.field_0x0c);
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
        g_TrackGame->proCircuit->AdvanceAfterRace();
        g_TrackGame->proCircuit->Save(g_TrackGame->careerFile);
        break;
    }
    case kDialogCommand:
        if (_stricmp("Back", event->controlName) == 0) {
            int bonus = circuit->raceIndex == circuit->schedules[circuit->series].bonusTrack;
            if (bonus && circuit->racers[0].field_0x18.field_0x10 == 1) {
                PCBonusTrackDlg* dialog = new(__FILE__, 1368) PCBonusTrackDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            } else if (circuit->racers[0].money <
                       circuit->entryFees[g_TrackGame->proCircuit->series]) {
                if (!(circuit->careerFlags & 1)) {
                    PCBailoutDlg* dialog = new(__FILE__, 1374) PCBailoutDlg;
                    guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
                    circuit->careerFlags |= 1;
                } else if (!(circuit->careerFlags & 2)) {
                    PCBunnyDlg* dialog = new(__FILE__, 1378) PCBunnyDlg;
                    guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
                    circuit->careerFlags |= 2;
                } else {
                    circuit->careerFlags |= 4;
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
void PCFailedDlg::HandleEvent(UnknownDialogEvent* event)
{
    UIControl* label;
    switch (event->kind) {
    case kDialogInit: {
        g_TrackGame->proCircuit->Save(g_TrackGame->careerFile);
        UIControl* description = FindControl("TxtDescription", 12);
        description->wordWrap = 1;
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
void PCFinishedDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
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
        for (int i = 0; i < circuit->racerCount; i++) {
            _itoa(circuit->racers[i].field_0x18.field_0x10, text, 10);
            rank->AddRow(text, i, 0);
            name->AddRow(circuit->racers[i].racerName, i, 0);
            _itoa(circuit->racers[i].field_0x18.field_0x00, text, 10);
            points->AddRow(text, i, 0);
            bike->AddRow(circuit->racers[i].bikeDisplayName, i, 0);
            sprintf(text, "$%d", circuit->racers[i].money);
            cash->AddRow(text, i, 0);
            sprintf(text, "$%d", circuit->racers[i].field_0x18.field_0x08);
            repairs->AddRow(text, i, 0);
            sprintf(text, "$%d", circuit->racers[i].field_0x18.field_0x0c);
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
void PCNewEventDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    switch (event->kind) {
    case kDialogInit: {
        int description;
        const char* image;
        switch (circuit->series) {
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
        control->wordWrap = 1;
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
void PCCompleteDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    switch (event->kind) {
    case kDialogInit: {
        char rank[1024];
        char text[1024];
        char message[2048];
        g_TrackGame->proCircuit->Save(g_TrackGame->careerFile);
        UnknownSetText(FindControl("TitleText", 12), 0x145d);
        UIControl* description = FindControl("TxtDescription", 12);
        g_TrackGame->LoadResourceString(0x145e, text, 1023);
        strcpy(rank, "");
        if (circuit->racers[0].field_0x18.field_0x10 == 1)
            g_TrackGame->LoadResourceString(0x145f, rank, 1023);
        sprintf(message, "%s\n%s", text, rank);
        description->wordWrap = 1;
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
    for (int i = 0; i < g_TrackGame->ui->manufacturerCount; i++)
        painter.PaintNumber(
            ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)[i].field_0xc0->plateTexture, number);
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
    riderChanged = 1;
    g_TrackGame->proCircuit->riderRow = list->GetRowData(-1);
}

// 0x004d7900: shows the chosen bike and loads its class's garage defaults.
void PCCentralBikeRiderDlg::ApplyChosenBike()
{
    char text[12];
    UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    KrustyUI* ui = g_TrackGame->ui;
    UnknownKrustyUIBike* bike = &((UnknownKrustyUIBike*)ui->bikes)[bikes->GetRowData(-1)];
    UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)ui->manufacturers)[bike->model];
    int i;
    // Retail re-reads TrackGame's KrustyUI after each model is hidden.
    for (i = 0; i < ui->manufacturerCount; i++) {
        ((UnknownKrustyUIModel*)ui->manufacturers)[i].field_0xc0->UnknownVirtualSlot4();
        ui = g_TrackGame->ui;
    }
    model->field_0xc0->UnknownVirtualSlot5();
    bikeChanged = 1;
    if (bike->field_0x88) {
        UIControl* engine = FindControl("DDLEngineSize", 0);
        if (!engine->shown) {
            engine->Show(1, 1);
            FindControl("TxtEngineSize", 0)->Show(1, 1);
            if (dialogBackground)
                dialogBackground->MarkAllDirty();
        }
    } else {
        UIControl* engine = FindControl("DDLEngineSize", 0);
        if (engine->shown) {
            engine->Show(0, 1);
            FindControl("TxtEngineSize", 0)->Show(0, 1);
            if (dialogBackground)
                dialogBackground->MarkAllDirty();
        }
    }
    g_TrackGame->proCircuit->bikeRow = bikes->GetRowData(-1);
    if (bike->field_0x88) {
        UIListBox* sizes = static_cast<UIDropDownList*>(FindControl("DDLEngineSize", 6))->listPart;
        int size = sizes->GetRowData(-1);
        g_TrackGame->proCircuit->racers[0].engineSize = g_UnknownGlobal56cb6c[size];
        g_TrackGame->proCircuit->racers[0].field_0x13c = size == 2 || size == 4 ? 1 : 0;
        UNKNOWN_APPLY_BIKE_CLASS(size, i);
    } else {
        int row = bikes->GetRowData(-1);
        g_TrackGame->proCircuit->racers[0].engineSize =
            ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[row].engineSize;
        g_TrackGame->proCircuit->racers[0].field_0x13c =
            ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[row].field_0x90;
        int bikeClass = UnknownBikeClassOf(g_TrackGame->proCircuit->racers[0].engineSize);
        UNKNOWN_APPLY_BIKE_CLASS(bikeClass, i);
    }
    UNKNOWN_GARAGE_SETTINGS->engineSize = g_TrackGame->proCircuit->racers[0].engineSize;
    UNKNOWN_GARAGE_SETTINGS->fourStroke = g_TrackGame->proCircuit->racers[0].field_0x13c;
    UIEditBox* plate = static_cast<UIEditBox*>(FindControl("EditPlateNumber", 0xb));
    _itoa(g_TrackGame->mode.field_0x1bcc, text, 10);
    plate->SetEditText(text);
}

// 0x004d7d70
void PCCentralBikeRiderDlg::SyncValues(int apply)
{
    if (!apply)
        return;
    strcpy(g_TrackGame->proCircuit->racers[0].bikeModel, "");
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    int bike = list->GetRowData(-1);
    strcpy(g_TrackGame->proCircuit->racers[0].bikeModel,
           ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)
               [((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bike].model].modelName);
    strcpy(g_TrackGame->proCircuit->racers[0].bikeName,
           ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bike].field_0x48);
    strcpy(g_TrackGame->proCircuit->racers[0].bikeDisplayName,
           ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bike].displayName);
    strcpy(g_TrackGame->proCircuit->racers[0].riderModel, "");
    list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
    int rider = list->GetRowData(-1);
    strcpy(g_TrackGame->proCircuit->racers[0].riderModel,
           ((UnknownKrustyUIModel*)g_TrackGame->ui->riders)[rider].modelName);
}

// 0x004d6770: as SPBikeRiderDlg slot 29 (dlgprocs.cpp 0x0044ef70); the
// engine sizes offered follow the circuit's bike class rule.
void PCCentralBikeRiderDlg::HandleEvent(UnknownDialogEvent* event)
{
    char plate[12];
    char typed[12];
    switch (event->kind) {
    case kDialogInit: {
        previewDragged = 0;
        static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->SetCurrentState(1);
        riderChanged = 1;
        bikeChanged = 1;
        FillBikeRiderLists();
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLEngineSize", 6))->listPart;
        list->RemoveAllRows();
        if (g_TrackGame->proCircuit->bikeClassRule == 1 || g_TrackGame->proCircuit->bikeClassRule == 3)
            list->AddRow("125cc 2-stroke", UnknownBikeClassOf(125), 0);
        if (g_TrackGame->proCircuit->bikeClassRule == 2 || g_TrackGame->proCircuit->bikeClassRule == 3) {
            list->AddRow("250cc 2-stroke", UnknownBikeClassOf(250), 0);
            list->AddRow("400cc 4-stroke", UnknownBikeClassOf(400), 0);
        }
        if (g_TrackGame->proCircuit->bikeClassRule == 3) {
            list->AddRow("500cc 2-stroke", UnknownBikeClassOf(500), 0);
            list->AddRow("600cc 4-stroke", UnknownBikeClassOf(600), 0);
        }
        list->SelectRowByData(UnknownBikeClassOf(g_TrackGame->proCircuit->racers[0].engineSize));
        UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditPlateNumber", 0xb));
        edit->SetAcceptedCharacters("0123456789");
        g_TrackGame->ui->ShowScene(this);
        Vector3* eye = &viewEye;
        previewArea.left = 0x52;
        previewArea.right = 0x216;
        previewArea.top = 0x74;
        previewArea.bottom = 0x150;
        *eye = kVec3Zero;
        float fov = 50.0f;
        Vector3* target = &viewTarget;
        viewEye.z = 15.0f;
        viewEye.y = 1.0f;
        *target = g_TrackGame->ui->garageFocus;
        viewTarget.y += 3.0f;
        g_TrackGame->ui->garageCamera->SetFrame(eye, 0, 0, 0, &fov);
        g_TrackGame->ui->garageCamera->UnknownVirtualSlot29(*target);
        g_TrackGame->ui->garageCamera->SetViewport(
            previewArea.left, previewArea.top, previewArea.right - previewArea.left,
            previewArea.bottom - previewArea.top);
        viewDistance = UnknownVectorLength(UnknownVectorDifference(*eye, g_TrackGame->ui->garageFocus));
        ApplyChosenRider();
        ApplyChosenBike();
        PaintPlateNumber(g_TrackGame->mode.field_0x1bcc);
        srand(ReadClock());
        if (dialogBackground)
            previewRegion = dialogBackground->AllocRegion(0);
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
            list->ReportClick(0);
        } else if (!_stricmp("BikeRight", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + 1) % rows);
            list->ReportClick(0);
        } else if (!_stricmp("RiderLeft", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + rows - 1) % rows);
            list->ReportClick(0);
        } else if (!_stricmp("RiderRight", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + 1) % rows);
            list->ReportClick(0);
        } else if (!_stricmp("ChkAutoRotate", event->controlName)) {
            previewDragged = !static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->GetSelectedState();
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
            dialogBackground->ReleaseRegion(previewRegion);
        ((RenderTarget*)field_0x18)->SetCamera(0);
        break;
    }
}


// 0x004d6fc0
void PCCentralBikeRiderDlg::FillBikeRiderLists()
{
    int bike = 0;
    int rider = 0;
    char text[128];
    PCCentralBikeRiderDlg* self = this;
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    list->RemoveAllRows();
    for (int i = 0; i < g_TrackGame->ui->bikeCount; i++) {
        int rule = g_TrackGame->proCircuit->bikeClassRule;
        if (rule != 3 && !((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[i].field_0x88) {
            int bikeClass = UnknownBikeClassOf(((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[i].engineSize);
            if (bikeClass != 0) {
                if (bikeClass <= 0 || bikeClass > 2)
                    continue;
                if (rule != 2)
                    continue;
            } else if (rule != 1) {
                continue;
            }
        }
        UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)[((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[i].model];
        if (model->modelKind > g_TrackGame->proCircuit->maxModelKind)
            continue;
        sprintf(text, "%s %s", model->displayName, ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[i].displayName);
        list->AddRow(text, i, 0);
        if (g_TrackGame->proCircuit->bikeRow == i)
            bike = i;
    }
    if (bike)
        list->SelectRowByData(bike);
    else
        list->SelectRow(0);
    list->Sort(1);
    list = static_cast<UIDropDownList*>(self->FindControl("DDLRiders", 6))->listPart;
    list->RemoveAllRows();
    for (int j = 0; j < g_TrackGame->ui->riderCount; j++) {
        UnknownKrustyUIModel* entry = &((UnknownKrustyUIModel*)g_TrackGame->ui->riders)[j];
        if (entry->modelKind <= g_TrackGame->proCircuit->maxModelKind || entry->field_0xc0) {
            list->AddRow(entry->displayName, j, 0);
            if (g_TrackGame->proCircuit->riderRow == j)
                rider = j;
        }
    }
    if (rider)
        list->SelectRowByData(rider);
    else
        list->SelectRow(0);
}


// 0x004d77d0
int PCCentralBikeRiderDlg::UnknownVirtualSlot13()
{
    if (bikeChanged) {
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        int bike = list->GetRowData(-1);
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)ui->bikes)[bike];
        ((UnknownProCircuitSkinned*)((UnknownKrustyUIModel*)ui->manufacturers)[entry->model].field_0xc0->plateTexture)
            ->ReplaceMaterialTexture(0, entry->field_0x48, &g_TrackGame->field_0x1c);
        bikeChanged = 0;
    }
    if (riderChanged) {
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        UnknownKrustyUIModel* rider = &((UnknownKrustyUIModel*)ui->riders)[list->GetRowData(-1)];
        ((UnknownProCircuitSkinned*)ui->riderPreview->plateTexture)
            ->ReplaceMaterialTexture(0, rider->modelName, &g_TrackGame->field_0x1c);
        riderChanged = 0;
    }
    return UIDialog::UnknownVirtualSlot13();
}
