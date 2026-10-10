#include "DlgProcs.h"

#include "DialogEventKind.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BackgroundImage.h"
#include "Camera.h"
#include "DebugAlloc.h"
#include "../krusty2/math/FastMath.h"
#include "InGameProcs.h"
#include "MatrixUtil.h"
#include "Net.h"
#include "NetProcs.h"
#include "OptionProcs.h"
#include "ProCircuitProcs.h"
#include "RenderTarget.h"
#include "SelectGamePicProcs.h"
#include "TrackGame.h"
#include "TrackRecordDlg.h"

// A truncating copy into a `size`-byte buffer. Retail evaluates `source`
// once for the length and again for the copy, so this is a macro.
#define COPY_TEXT(dest, source, size)                              \
    {                                                              \
        int length = strlen(source);                               \
        int copied = length > (size) - 1 ? (size) - 1 : length;    \
        strncpy(dest, source, copied);                             \
        (dest)[copied] = 0;                                        \
    }

// The four vector constants that open many retail files (see
// LightEmitter.cpp): 0x0059adc0, 0x0059add0, 0x0059adf0 and 0x0059adb0.
// Their initializers sit in the middle of this file's code
// (0x00453d50..0x00453e8b).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// d3drm.dll's D3DRMVectorRotate (declared as in FollowCamera.h): rotates
// `vector` about `axis` by `theta`.
extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

// Inline vector helpers of the bike views (D3D_OVERLOADS-style).
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

// dplay.h's service provider GUIDs (Net.cpp declares them the same way).
extern "C" const GUID DPSPGUID_IPX;                     // 0x005567d0
extern "C" const GUID DPSPGUID_TCPIP;                   // 0x005567e0
extern "C" const GUID DPSPGUID_SERIAL;                  // 0x005567f0
extern "C" const GUID DPSPGUID_MODEM;                   // 0x00556800

// 0x0059adbc: the profile row "ButRemoveProfile" asked to remove.
int g_UnknownGlobal59adbc;
int g_UnknownGlobal59adfc;
int g_UnknownGlobal59ae84;
int g_UnknownGlobal59ae88;
int g_UnknownGlobal59ae00;

// 0x00569e14: the characters a profile name may use.
static const char kProfileNameCharacters[] =
    "\xc7\xfc\xe9\xe2\xe4\xe0\xe5\xe7\xea\xeb\xe8\xef\xee\xec\xc4\xc5"
    "\xc9\xe6\xc6\xf4\xf6\xf2\xfb\xf9\xff\xd6\xdc\xe1\xed\xf3\xfa\xf1"
    "\xd1\xaa\xba\xbf\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d"
    "\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d"
    "\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xab\xac\xad\xae"
    "\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe"
    "\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce"
    "\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde"
    "\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xf0\xf5\xf7\xf8\xf9"
    "\xfd\xfe"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-()$#@";

// 0x0044b0a0
int CompareRowNumbersAscending(const void* a, const void* b) {
    int first = atoi(((const UnknownGameUiListRow*)a)->text);
    int second = atoi(((const UnknownGameUiListRow*)b)->text);
    if (first < second)
        return -1;
    return first != second;
}

// 0x0044b0e0
int CompareRowNumbersDescending(const void* a, const void* b) {
    int first = atoi(((const UnknownGameUiListRow*)a)->text);
    int second = atoi(((const UnknownGameUiListRow*)b)->text);
    if (first > second)
        return -1;
    return first != second;
}

// 0x0044b120
int UseSelectedProfile(UnknownDialogEvent* event) {
    UIListBox* list = static_cast<UIListBox*>(event->dialog->FindControl("LstProfiles", 0));
    short selection = list->GetSelectedRow();
    if (selection != -1) {
        char name[16];
        COPY_TEXT(name, list->GetRowText(selection), 16);
        COPY_TEXT(g_TrackGame->mode.field_0x00, name, 16);
        g_TrackGame->mode.UnknownFunction5231f0();
        return 1;
    }
    return 0;
}

// 0x0044b1f0
void CloseDialogCallback(UIDialog* dialog) {
    dialog->EndDialog(0);
}

// 0x0044c6b0
int MainDlg::UnknownVirtualSlot10(float frameTime) {
    if (field_0x7f6c) {
        FindControl("ScreenOverCtl", 0)->UnknownVirtualSlot50();
        if (field_0x7f6c && !menuSingleAnimation->field_0x68 && field_0x7f78) {
            if (field_0x7f7c < 4.0f) {
                field_0x7f7c = g_TrackGame->frameTime + field_0x7f7c;
            } else {
                field_0x7f78 = 0;
                if (movieControl) {
                    movieControl->field_0x1ec->Restart();
                    movieControl->field_0x1ec->UnknownVirtualSlot16(field_0x7f78);
                    movieControl->Show(1, 1);
                }
            }
        }
    }
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x0044c7a0
void MainDlg::FillProfileList() {
    char name[260];
    UIListBox* list = static_cast<UIListBox*>(FindControl("LstProfiles", 0));
    list->RemoveAllRows();
    g_TrackGame->profileDirectory->UnknownFunction44a1d0("ui\\profile");
    g_TrackGame->profileDirectory->UnknownFunction44a220("*", 0);
    g_TrackGame->profileDirectory->UnknownVirtualSlot1();
    g_TrackGame->profileDirectory->UnknownFunction44a550(name);
    if (name[0])
        list->AddRow(name, 0, 0);
    while (g_TrackGame->profileDirectory->UnknownFunction44a4c0(name)) {
        if (name[0])
            list->AddRow(name, 0, 0);
    }
    list->Sort(1);
    list->SetRowTextColor(0xfeb97a, -1);
    list->SelectRowByText(g_TrackGame->mode.field_0x00);
    list->SetSelectColor(0xffffff);
    list->SetSelectBoxColor(0xfeb97a);
}

// 0x0044c8c0
void MainDlg::ShowCurrentProfile() {
    char label[128];
    char text[128];
    UIControl* control = FindControl("TxtCurrentProfile", 12);
    g_TrackGame->LoadResourceString(0x1466, label, 128);
    sprintf(text, "%s %s", label, g_TrackGame->mode.field_0x00);
    control->SetText(text);
}

// 0x0044c930
void SetStatsButtonResources(UnknownDialogEvent* event, int a, int b, int c, int d) {
    UIControl* button;
    if (a) {
        button = event->dialog->FindControl("ButStats1", 0);
        button->SetTextFromResource(g_TrackGame->resourceInstance, a);
    }
    if (b) {
        button = event->dialog->FindControl("ButStats2", 0);
        button->SetTextFromResource(g_TrackGame->resourceInstance, b);
    }
    if (c) {
        button = event->dialog->FindControl("ButStats3", 0);
        button->SetTextFromResource(g_TrackGame->resourceInstance, c);
    }
    if (d) {
        button = event->dialog->FindControl("ButStats4", 0);
        button->SetTextFromResource(g_TrackGame->resourceInstance, d);
    }
}

// 0x0044c9f0
void SetStatsButtonTexts(UnknownDialogEvent* event, const char* a, const char* b, const char* c,
                         const char* d) {
    UIControl* button;
    if (a) {
        button = event->dialog->FindControl("ButStats1", 0);
        button->SetText(a);
    }
    if (b) {
        button = event->dialog->FindControl("ButStats2", 0);
        button->SetText(b);
    }
    if (c) {
        button = event->dialog->FindControl("ButStats3", 0);
        button->SetText(c);
    }
    if (d) {
        button = event->dialog->FindControl("ButStats4", 0);
        button->SetText(d);
    }
}

// 0x0044ca80
void ClearStatsLists(UnknownDialogEvent* event) {
    static_cast<UIListBox*>(event->dialog->FindControl("LstStats1", 0))->RemoveAllRows();
    static_cast<UIListBox*>(event->dialog->FindControl("LstStats2", 0))->RemoveAllRows();
    static_cast<UIListBox*>(event->dialog->FindControl("LstStats3", 0))->RemoveAllRows();
    static_cast<UIListBox*>(event->dialog->FindControl("LstStats4", 0))->RemoveAllRows();
}

// 0x0044cae0
void AddStatsRows(UnknownDialogEvent* event, const char* a, const char* b, const char* c,
                  const char* d) {
    UIListBox* list;
    if (a) {
        list = static_cast<UIListBox*>(event->dialog->FindControl("LstStats1", 0));
        list->AddRow(a, 0, 0);
    }
    if (b) {
        list = static_cast<UIListBox*>(event->dialog->FindControl("LstStats2", 0));
        list->AddRow(b, 0, 0);
    }
    if (c) {
        list = static_cast<UIListBox*>(event->dialog->FindControl("LstStats3", 0));
        list->AddRow(c, 0, 0);
    }
    if (d) {
        list = static_cast<UIListBox*>(event->dialog->FindControl("LstStats4", 0));
        list->AddRow(d, 0, 0);
    }
}

// 0x0044cb80
void ChooseTCPMethodDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogInit: {
        UIControl* control = FindControl("ButLeft", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e7);
        control = FindControl("ButMiddle", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e6);
        control = FindControl("ButRight", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13dc);
        control = FindControl("TitleText", 12);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e8);
        UIControl* prompt = FindControl("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->SetTextFromResource(g_TrackGame->resourceInstance, 0x13fe);
        break;
    }
    case kDialogCommand:
        if (!_stricmp("ButLeft", event->controlName)) {
            event->dialog->EndDialog(0x3c);
            event->handled = 1;
        } else if (!_stricmp("ButMiddle", event->controlName)) {
            event->dialog->EndDialog(0x3d);
            event->handled = 1;
        } else if (!_stricmp("ButRight", event->controlName)) {
            event->dialog->EndDialog(0);
            event->handled = 1;
        }
        break;
    }
}

// 0x0044ccf0
void NewbieDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogInit: {
        UIControl* control = FindControl("ButLeft", 1);
        control->Show(0, 1);
        control->keyBind = 0;
        FindControl("ButRight", 1)->Show(0, 1);
        control = FindControl("ButMiddle", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e9);
        control->keyBind = 0x1c;
        control = FindControl("TitleText", 12);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13ea);
        UIControl* prompt = FindControl("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->SetTextFromResource(g_TrackGame->resourceInstance, 0x1400);
        break;
    }
    case kDialogCommand:
        if (!_stricmp("ButMiddle", event->controlName)) {
            event->dialog->EndDialog(0x33);
            event->handled = 1;
        }
        break;
    }
}

// 0x0044ce10
void NoDelCurProfileDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogInit: {
        UIControl* control = FindControl("ButLeft", 1);
        control->Show(0, 1);
        control->keyBind = 0;
        FindControl("ButRight", 1)->Show(0, 1);
        control = FindControl("ButMiddle", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e9);
        control->keyBind = 0x1c;
        control = FindControl("TitleText", 12);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x146d);
        UIControl* prompt = FindControl("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->SetTextFromResource(g_TrackGame->resourceInstance, 0x146e);
        break;
    }
    case kDialogCommand:
        if (!_stricmp("ButMiddle", event->controlName))
            EndDialog(0);
        break;
    }
}

// 0x0044cf20
void SinglePlayerDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogInit: {
        field_0x7f58 = 0;
        field_0x7f5c = 0;
        field_0x7f60 = 0;
        UIRadioButton* tab = static_cast<UIRadioButton*>(FindControl("EventTab", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x13e3);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x13e3);
        tab->SetTextAlign(0x12);
        tab->SelectInGroup(0);
        tab = static_cast<UIRadioButton*>(FindControl("BikeRiderTab", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x13e4);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x13e4);
        tab->SetTextAlign(0x12);
        tab = static_cast<UIRadioButton*>(FindControl("RaceInfoTab", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x13e5);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x13e5);
        tab->SetTextAlign(0x12);
        memcpy(&g_TrackGame->mode.field_0x27f8, &g_TrackGame->mode.field_0x29e4,
               sizeof(g_TrackGame->mode.field_0x29e4));
        memcpy(g_TrackGame->mode.field_0xfd8, g_TrackGame->mode.field_0x1034,
               sizeof(g_TrackGame->mode.field_0x1034));
        memcpy(&g_TrackGame->mode.field_0x1974, &g_TrackGame->mode.field_0x1a3c,
               sizeof(g_TrackGame->mode.field_0x1a3c));
        g_TrackGame->mode.field_0x94 = g_TrackGame->mode.field_0x98;
        g_TrackGame->mode.UnknownFunction5240e0(g_TrackGame->mode.field_0x27f8.field_0x04);
        if (openingMenu == 0x88e) {
            static_cast<UIRadioButton*>(FindControl("RaceInfoTab", 4))->SelectInGroup(0);
            OpenPage(2);
        } else {
            static_cast<UIRadioButton*>(FindControl("EventTab", 4))->SelectInGroup(0);
            OpenPage(0);
        }
        break;
    }
    case kDialogCommand:
        if (!_stricmp("Back", event->controlName)) {
            if (field_0x7f58)
                field_0x7f58->UpdateBoundValues(1);
            if (field_0x7f5c)
                field_0x7f5c->UpdateBoundValues(1);
            UpdateBoundValues(1);
            memcpy(&g_TrackGame->mode.field_0x29e4, &g_TrackGame->mode.field_0x27f8,
                   sizeof(g_TrackGame->mode.field_0x29e4));
            memcpy(g_TrackGame->mode.field_0x1034, g_TrackGame->mode.field_0xfd8,
                   sizeof(g_TrackGame->mode.field_0x1034));
            memcpy(&g_TrackGame->mode.field_0x1a3c, &g_TrackGame->mode.field_0x1974,
                   sizeof(g_TrackGame->mode.field_0x1a3c));
            g_TrackGame->mode.field_0x98 = g_TrackGame->mode.field_0x94;
            g_TrackGame->ui->OpenMenu(100);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("Start", event->controlName)) {
            if (field_0x7f58)
                field_0x7f58->UpdateBoundValues(1);
            if (g_TrackGame->mode.field_0x27f8.field_0x08 == -1) {
                ChoiceDlg* dialog = new(__FILE__, 1274) ChoiceDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0,
                                                  (UIDialog*)this, 0, 0, 1);
                char text[1024];
                strcpy(text, "You must choose one of the following trial version tracks:\n"
                             "Stunt Event: Donner Pass, or\n"
                             "Nationals Race: A Voodoo Basin\n");
                dialog->SetTextsOrResources("Trial Version", 0, text, 0, 0, 0, 0, 0x13e9, 0, 0);
                dialog->FindControl("TxtPrompt", 0)->field_0x1e8 = 1;
            } else if (g_TrackGame->mode.field_0x9c) {
                NewbieDlg* dialog = new(__FILE__, 1292) NewbieDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0,
                                                  (UIDialog*)this, 0, 0, 1);
            } else {
                g_TrackGame->ui->HideScene();
                g_TrackGame->mode.field_0x27f8.field_0x28 = 0;
                if (field_0x7f58)
                    field_0x7f58->UpdateBoundValues(1);
                if (field_0x7f5c)
                    field_0x7f5c->UpdateBoundValues(1);
                if (field_0x7f60)
                    field_0x7f60->UpdateBoundValues(1);
                if (g_TrackGame->mode.field_0x2dbc && g_TrackGame->mode.field_0x27f8.field_0x00 != 4) {
                    g_TrackGame->field_0x342c = 1;
                    g_TrackGame->field_0x3428 = 0;
                } else {
                    g_TrackGame->field_0x342c = 0;
                    g_TrackGame->field_0x3428 = 0;
                }
                memcpy(&g_TrackGame->mode.field_0x29e4, &g_TrackGame->mode.field_0x27f8,
                       sizeof(g_TrackGame->mode.field_0x29e4));
                memcpy(g_TrackGame->mode.field_0x1034, g_TrackGame->mode.field_0xfd8,
                       sizeof(g_TrackGame->mode.field_0x1034));
                memcpy(&g_TrackGame->mode.field_0x1a3c, &g_TrackGame->mode.field_0x1974,
                       sizeof(g_TrackGame->mode.field_0x1a3c));
                g_TrackGame->mode.field_0x98 = g_TrackGame->mode.field_0x94;
                if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4 && g_TrackGame->mode.field_0x25ec[0]) {
                    // KrustyUI+0x4e8 holds a copy of the race settings.
                    memcpy(&g_TrackGame->mode.field_0x27f8, (unsigned char*)g_TrackGame->ui + 0x4e8,
                           sizeof(g_TrackGame->mode.field_0x29e4));
                } else {
                    g_TrackGame->mode.field_0x25ec[0] = 0;
                }
                UnknownFunction4536e0();
                UnknownVirtualSlot26();
            }
        } else if (!_stricmp("EventTab", event->controlName)) {
            OpenPage(0);
        } else if (!_stricmp("BikeRiderTab", event->controlName)) {
            OpenPage(1);
        } else if (!_stricmp("RaceInfoTab", event->controlName)) {
            OpenPage(2);
        } else if (!_stricmp("Options", event->controlName)) {
            OptionsDlg* dialog = new(__FILE__, 1347) OptionsDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0,
                                              (UIDialog*)this, 0, 0, 1);
        } else if (!_stricmp("Joystick", event->controlName)) {
            OptionsDlg* dialog = new(__FILE__, 1350) OptionsDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0,
                                              (UIDialog*)this, 1, 0, 1);
        } else if (!_stricmp("Help", event->controlName)) {
            g_TrackGame->mode.OpenHelp("MCM2HELP", 0);
        }
        break;
    case 9: {
        int code = event->code;
        if (code == 0x33) {
            static_cast<UIRadioButton*>(FindControl("BikeRiderTab", 4))->SelectInGroup(0);
            OpenPage(1);
        } else if (code == 0x51) {
            if (field_0x7f5c) {
                field_0x7f5c->UpdateBoundValues(0);
                field_0x7f5c->FillBikeRiderLists();
                field_0x7f5c->ApplyChosenRider();
                field_0x7f5c->UnknownFunction4500e0();
            }
        } else if (code == 0x66) {
            static_cast<UIRadioButton*>(FindControl("EventTab", 4))->SelectInGroup(0);
            OpenPage(0);
        }
        break;
    }
    case kDialogClose:
        if (parentDialog)
            parentDialog->UnknownFunction46ea60(1);
        break;
    }
}

// 0x0044d740
void SinglePlayerDlg::OpenPage(int page) {
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
            field_0x7f58 = new(__FILE__, 1401) SPEventDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(field_0x7f58, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 1:
        if (!field_0x7f5c) {
            field_0x7f5c = new(__FILE__, 1407) SPBikeRiderDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(field_0x7f5c, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 2:
        if (!field_0x7f60) {
            field_0x7f60 = new(__FILE__, 1413) SPRaceInfoDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(field_0x7f60, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    }
}

// 0x0044d950
void SPEventDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_TrackGame->mode.field_0x27f8.field_0x00;
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->listPart;
        g_TrackGame->LoadResourceString(0x13ed, text, 0x80);
        list->AddRow(text, 1, 0);
        g_TrackGame->LoadResourceString(0x13ee, text, 0x80);
        list->AddRow(text, 0, 0);
        g_TrackGame->LoadResourceString(0x13f0, text, 0x80);
        list->AddRow(text, 5, 0);
        g_TrackGame->LoadResourceString(0x13ef, text, 0x80);
        list->AddRow(text, 2, 0);
        g_TrackGame->LoadResourceString(0x13f2, text, 0x80);
        list->AddRow(text, 3, 0);
        list->SelectRowByData(settings->eventType);
        list = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
        g_TrackGame->LoadResourceString(0x13f3, text, 0x80);
        list->AddRow(text, 0, 0);
        g_TrackGame->LoadResourceString(0x13f4, text, 0x80);
        list->AddRow(text, 1, 0);
        g_TrackGame->LoadResourceString(0x13f5, text, 0x80);
        list->AddRow(text, 2, 1);
        g_TrackGame->LoadResourceString(0x13f6, text, 0x80);
        list->AddRow(text, 4, 1);
        list->SelectRow(settings->raceMode);
        FindControl("OpponentsListBox", 0)->UnknownVirtualSlot54(&settings->opponents);
        FindControl("ChkRecordRace", 0)->Show(0, 1);
        FindControl("RadLODEasy", 0)->UnknownVirtualSlot54(&g_TrackGame->mode.field_0x94);
        UIListBox* gates = static_cast<UIListBox*>(FindControl("LstNumGates", 3));
        gates->RemoveAllRows();
        for (int i = 10; i <= 30; i++) {
            _itoa(i, text, 10);
            gates->AddRow(text, i, 0);
        }
        static_cast<UIEditBox*>(FindControl("EditSeed", 0xb))->SetAcceptedCharacters("0123456789");
        UpdateBoundValues(0);
        ApplyEventType();
        ShowRaceModeControls();
        ghostHeaderCount = 0;
        ghostHeaders = 0;
        ghostPathCount = 0;
        ghostPaths = 0;
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4)
            ListGhosts();
        break;
    }
    case kDialogCommand:
        if (!_stricmp("ChkRandomGates", event->controlName))
            ShowRaceModeControls();
        if (!_stricmp("TrackLeft", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("TrackRight", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("GhostLeft", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLGhostRaces", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("GhostRight", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLGhostRaces", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("SeedLeft", event->controlName)) {
            char seed[32];
            UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditSeed", 0xb));
            edit->GetEditText(seed, 32);
            int value = atoi(seed) - 1;
            if (value < 0)
                value = 999;
            _itoa(value, seed, 10);
            edit->SetEditText(seed);
            guiUser->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
        } else if (!_stricmp("SeedRight", event->controlName)) {
            char seed[32];
            UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditSeed", 0xb));
            edit->GetEditText(seed, 32);
            int value = atoi(seed) + 1;
            if (value > 999)
                value = 0;
            _itoa(value, seed, 10);
            edit->SetEditText(seed);
            guiUser->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
        } else if (!_stricmp("OpponentsLeftArrow", event->controlName) ||
                   !_stricmp("OpponentsRightArrow", event->controlName)) {
            ShowRaceModeControls();
        }
        break;
    case kDialogListSelect:
        if (!_stricmp("RaceModeDropDown", event->controlName)) {
            if (static_cast<UIListBox*>(event->control)->GetRowData(-1) == 4)
                ListGhosts();
            if (static_cast<UIListBox*>(event->control)->GetRowData(-1) != 2) {
                g_TrackGame->mode.field_0x10ec = 0;
                g_TrackGame->eventManager->field_0x48 = 0;
            }
            ShowRaceModeControls();
        } else if (!_stricmp("EventTypeDropDown", event->controlName)) {
            UpdateBoundValues(1);
            ApplyEventType();
            ShowRaceModeControls();
            ListGhosts();
        } else if (!_stricmp("DDLTextBox", event->controlName)) {
            ListGhosts();
            strcpy(g_TrackGame->mode.field_0x6f4[g_TrackGame->mode.field_0x27f8.field_0x04],
                   static_cast<UIListBox*>(event->control)->GetRowText(-1));
        }
        break;
    }
}

// 0x0044dfc0: shows the controls that apply to the chosen race mode.
void SPEventDlg::ShowRaceModeControls() {
    char text[32];
    UIControl* record = FindControl("ChkRecordRace", 0);
    UIListBox* modes = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
    int mode = modes->GetRowData(-1);
    UIControl* lapsBox = FindControl("LapsControlBox", 5);
    UIControl* racesBox = FindControl("RacesControlBox", 5);
    UIControl* ghostBox = FindControl("GhostBox", 5);
    UIControl* opponentsBox = FindControl("OpponentsControlBox", 5);
    opponentsBox->Show(1, 1);
    record->Show(0, 1);
    ghostBox->Show(0, 1);
    ShowGroup(0x3f3, 1);
    switch (mode) {
    case 0:
        lapsBox->Show(0, 1);
        racesBox->Show(0, 1);
        break;
    case 1:
        lapsBox->Show(1, 1);
        racesBox->Show(0, 1);
        break;
    case 2:
        lapsBox->Show(1, 1);
        racesBox->Show(1, 1);
        break;
    case 4:
        opponentsBox->Show(0, 1);
        lapsBox->Show(0, 1);
        racesBox->Show(0, 1);
        record->Show(0, 1);
        ghostBox->Show(1, 1);
        ShowGroup(0x3f3, 0);
        break;
    }
    UIControl* waypoints = FindControl("WaypointBox", 5);
    UnknownTrackGameModeSettings& settings = g_TrackGame->mode.field_0x27f8;
    if (settings.field_0x04 == 1 && modes->GetRowData(-1) != 4) {
        waypoints->Show(1, 1);
        if (static_cast<UIMultiState*>(FindControl("ChkRandomGates", 2))->UnknownFunction4755c0()) {
            EnableGroup(0x6f, 1);
            FindControl("LstNumGates", 0)->Show(1, 1);
            FindControl("EditSeed", 0)->Show(1, 1);
        } else {
            EnableGroup(0x6f, 0);
            FindControl("LstNumGates", 0)->Show(0, 1);
            FindControl("EditSeed", 0)->Show(0, 1);
        }
    } else {
        waypoints->Show(0, 1);
    }
    UIControl* racesLeft = FindControl("RacesLeftArrow", 9);
    UIControl* racesRight = FindControl("RacesRightArrow", 0xa);
    UIControl* opponentsLeft = FindControl("OpponentsLeftArrow", 0);
    UIControl* opponentsRight = FindControl("OpponentsRightArrow", 0);
    UIControl* racesLabel = FindControl("RacesLabel", 0xc);
    UIListBox* races = static_cast<UIListBox*>(FindControl("RacesListBox", 3));
    UIControl* opponents = FindControl("OpponentsListBox", 3);
    races->RemoveAllRows();
    for (int i = 3; i <= 7; i += 2) {
        sprintf(text, "%d", i);
        races->AddRow(text, i - 1, 0);
    }
    races->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x0c);
    if (settings.field_0x00 == 2 && g_TrackGame->mode.field_0x10ec) {
        opponentsLeft->Show(0, 1);
        opponentsRight->Show(0, 1);
        opponents->UnknownVirtualSlot49(0);
        racesLeft->Show(0, 1);
        racesRight->Show(0, 1);
        racesLabel->SetTextFromResource(g_TrackGame->resourceInstance, 0x92b);
        sprintf(text, "%d/%d", g_TrackGame->eventManager->field_0x48 + 1,
                g_TrackGame->mode.field_0x27f8.field_0x0c + 1);
        races->RemoveAllRows();
        races->AddRow(text, 0, 0);
    } else {
        opponentsLeft->Show(1, 1);
        opponentsRight->Show(1, 1);
        opponents->UnknownVirtualSlot49(1);
        racesLeft->Show(1, 1);
        racesRight->Show(1, 1);
        racesLabel->SetTextFromResource(g_TrackGame->resourceInstance, 0x92a);
    }
}

// 0x0044e3e0: fills the race mode, opponent and lap lists for the chosen
// event type and lists its tracks.
void SPEventDlg::ApplyEventType() {
    char label[64];
    char text[256];
    UIListBox* types = static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->listPart;
    int type = types->GetRowData(-1);
    UnknownTrackGameModeSettings& settings = g_TrackGame->mode.field_0x27f8;
    settings.field_0x04 = type;
    g_TrackGame->mode.UnknownFunction5240e0(type);
    UIListBox* modes = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
    int row = ((UIListBox*)modes)->FindRowByData(4);
    if (type == 0) {
        if (row != -1) {
            if (modes->GetSelectedRow() == row) {
                modes->SelectRowByData(0);
                settings.field_0x00 = 0;
            }
            modes->RemoveRow(row);
        }
    } else if (row == -1) {
        g_TrackGame->LoadResourceString(0x13f6, label, 128);
        modes->AddRow(label, 4, 1);
    }
    UIControl* lapsLabel = FindControl("LapsLabel", 0xc);
    UIListBox* opponents = static_cast<UIListBox*>(FindControl("OpponentsListBox", 3));
    UIListBox* laps = static_cast<UIListBox*>(FindControl("LapsListBox", 3));
    FindControl("RacesListBox", 3);
    lapsLabel->SetTextFromResource(g_TrackGame->resourceInstance, 0x929);
    opponents->RemoveAllRows();
    int i;
    for (i = 0; i <= 4; i++) {
        sprintf(text, "%d", i);
        opponents->AddRow(text, i, 0);
    }
    laps->RemoveAllRows();
    for (i = type == 2 || type == 3 ? 2 : 1; i <= 5; i++) {
        sprintf(text, "%d", i);
        laps->AddRow(text, i, 0);
    }
    laps->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x20);
    if (type == 0) {
        lapsLabel->SetTextFromResource(g_TrackGame->resourceInstance, 0x92f);
        laps->RemoveAllRows();
        for (i = 2; i <= 5; i++) {
            sprintf(text, "%dm", i);
            laps->AddRow(text, i, 0);
        }
        laps->SelectRowByData((int)g_TrackGame->mode.field_0x27f8.field_0x140);
    }
    if (settings.field_0x04 == 0) {
        g_TrackGame->mode.UnknownFunction5240e0(0);
        FillTrackList(1, "PictureBox", "DDLTextBox", 0, this, 0);
        g_TrackGame->mode.UnknownFunction5240e0(5);
        FillTrackList(1, "PictureBox", "DDLTextBox", 0, this, 1);
    } else {
        FillTrackList(1, "PictureBox", "DDLTextBox", 0, this, 0);
    }
    UpdateBoundValues(0);
    g_TrackGame->mode.UnknownFunction5240e0(type);
    UIListBox* tracks = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
    tracks->SelectRowByText(g_TrackGame->mode.field_0x6f4[type]);
}

// 0x0044e6c0
void SPEventDlg::UnknownVirtualSlot31(int apply) {
    char seed[128];
    UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_TrackGame->mode.field_0x27f8.field_0x00;
    if (apply) {
        if (settings->eventType != 0 && settings->eventType != 4)
            settings->laps = static_cast<UIListBox*>(FindControl("LapsListBox", 3))->GetRowData(-1);
        else
            settings->minutes = (float)static_cast<UIListBox*>(FindControl("LapsListBox", 3))->GetRowData(-1);
        if (!g_TrackGame->mode.field_0x10ec)
            settings->races = static_cast<UIListBox*>(FindControl("RacesListBox", 3))->GetRowData(-1);
        settings->treeCollision = static_cast<UIMultiState*>(FindControl("ChkTreeCollision", 2))->UnknownFunction4755c0();
        strcpy(settings->trackName, "");
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
        int index = list->GetRowData(-1);
        {
            char* name = g_TrackGame->ui->field_0x60[index].field_0x14;
            int length = strlen(name);
            int count = length > 0xff ? 0xff : length;
            strncpy(settings->trackName, name, count);
            settings->trackName[count] = 0;
        }
        g_TrackGame->mode.UnknownFunction5240e0(g_TrackGame->ui->field_0x60[index].field_0x08);
        settings->trackNumber = g_TrackGame->ui->field_0x60[index].field_0x04;
        strcpy(g_TrackGame->mode.field_0x6f4[g_TrackGame->mode.field_0x27f8.field_0x04], list->GetRowText(-1));
        g_TrackGame->mode.field_0x9f4[g_TrackGame->mode.field_0x27f8.field_0x04] = g_TrackGame->mode.field_0x27f8.field_0x34;
        if (static_cast<UIMultiState*>(FindControl("ChkRandomGates", 2))->UnknownFunction4755c0())
            settings->randomGates = 1;
        else
            settings->randomGates = 0;
        settings->gateCount = static_cast<UIListBox*>(FindControl("LstNumGates", 3))->GetRowData(-1);
        static_cast<UIEditBox*>(FindControl("EditSeed", 0xb))->GetEditText(seed, 128);
        settings->gateSeed = atoi(seed);
        list = static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->listPart;
        settings->eventType = list->GetRowData(-1);
        list = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
        settings->raceMode = list->GetRowData(-1);
        list = static_cast<UIDropDownList*>(FindControl("DDLGhostRaces", 6))->listPart;
        int ghost = list->GetRowData(-1);
        if (settings->raceMode == 4 && ghost != -1) {
            COPY_TEXT(g_TrackGame->mode.field_0x25ec, ghostPaths[ghost], 0x104);
            // KrustyUI+0x4b4 holds the header of the ghost raced against.
            *(UnknownRecordFileHeader*)((unsigned char*)g_TrackGame->ui + 0x4b4) = ghostHeaders[ghost];
        } else {
            g_TrackGame->mode.field_0x25ec[0] = 0;
        }
    } else {
        static_cast<UIListBox*>(FindControl("RacesListBox", 3))->SelectRowByData(settings->races);
        static_cast<UIMultiState*>(FindControl("ChkRandomGates", 2))->SetCurrentState(settings->randomGates);
        static_cast<UIListBox*>(FindControl("LstNumGates", 3))->SelectRowByData(settings->gateCount);
        _itoa(settings->gateSeed, seed, 10);
        static_cast<UIEditBox*>(FindControl("EditSeed", 0xb))->SetEditText(seed);
        UIListBox* laps = static_cast<UIListBox*>(FindControl("LapsListBox", 3));
        if (settings->eventType != 0 && settings->eventType != 4)
            laps->SelectRowByData(settings->laps);
        else
            laps->SelectRowByData((int)settings->minutes);
        static_cast<UIMultiState*>(FindControl("ChkTreeCollision", 2))->SetCurrentState(settings->treeCollision);
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->listPart;
        list->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x04);
        list = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
        list->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x00);
    }
}

// 0x0044eb40
void SPEventDlg::ListGhosts() {
    char path[260];
    char label[128];
    WIN32_FIND_DATAA data;
    ForgetGhosts();
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLGhostRaces", 6))->listPart;
    list->RemoveAllRows();
    g_TrackGame->LoadResourceString(0x14c7, label, 128);
    list->AddRow(label, -1, 0);
    list->SelectRow(0);
    HANDLE find = FindFirstFileA("Record\\*.gho", &data);
    if (find != INVALID_HANDLE_VALUE) {
        sprintf(path, "%s\\%s", "Record", data.cFileName);
        AddGhost(path);
        while (FindNextFileA(find, &data)) {
            sprintf(path, "%s\\%s", "Record", data.cFileName);
            AddGhost(path);
        }
        FindClose(find);
    }
}

// 0x0044ec50
int SPEventDlg::AddGhost(const char* path) {
    char label[128];
    char text[128];
    int added = 0;
    UnknownRecordFileHeader header;
    FILE* file = fopen(path, "rb");
    if (file) {
        if (fread(&header, sizeof(header), 1, file)) {
            UIListBox* tracks = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
            int row = tracks->GetRowData(-1);
            if (!_stricmp(header.trackName, g_TrackGame->ui->field_0x60[row].field_0x14) &&
                header.lengthSeconds > 0.0f &&
                g_TrackGame->ui->field_0x60[row].field_0x04 == header.trackIndex) {
                ghostHeaders = (UnknownRecordFileHeader*)UnknownFunction47b570(
                    ghostHeaders, (ghostHeaderCount + 1) * sizeof(UnknownRecordFileHeader));
                ghostHeaders[ghostHeaderCount] = header;
                ghostHeaderCount++;
                ghostPaths = (char**)UnknownFunction47b570(ghostPaths, (ghostPathCount + 1) * sizeof(char*));
                ghostPaths[ghostPathCount] = (char*)DebugMalloc(strlen(path) + 1, __FILE__, 1986);
                strcpy(ghostPaths[ghostPathCount], path);
                ghostPathCount++;
                UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLGhostRaces", 6))->listPart;
                g_TrackGame->LoadResourceString(0x14c8, label, 128);
                sprintf(text, "%s: %s", label, header.description);
                list->AddRow(text, ghostHeaderCount - 1, 0);
                added = 1;
            }
        }
        fclose(file);
    }
    return added;
}

// 0x0044eed0
void SPEventDlg::ForgetGhosts() {
    if (ghostHeaders)
        DebugFree(ghostHeaders, __FILE__, 2009);
    if (ghostPaths) {
        for (int i = 0; i < ghostPathCount; i++)
            DebugFree(ghostPaths[i], __FILE__, 2013);
        DebugFree(ghostPaths, __FILE__, 2015);
    }
    ghostHeaders = 0;
    ghostPaths = 0;
    ghostPathCount = 0;
    ghostHeaderCount = 0;
}

// 0x0044ef70
void SPBikeRiderDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char plate[12];
    char typed[12];
    switch (event->kind) {
    case kDialogInit: {
        previewDragged = 0;
        static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->SetCurrentState(1);
        g_TrackGame->mode.field_0x9c = 0;
        riderChanged = 1;
        bikeChanged = 1;
        FillBikeRiderLists();
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLEngineSize", 6))->listPart;
        list->RemoveAllRows();
        list->AddRow("125cc 2-stroke", UnknownBikeClassOf(125), 0);
        list->AddRow("250cc 2-stroke", UnknownBikeClassOf(250), 0);
        list->AddRow("400cc 4-stroke", UnknownBikeClassOf(400), 0);
        list->AddRow("500cc 2-stroke", UnknownBikeClassOf(500), 0);
        list->AddRow("600cc 4-stroke", UnknownBikeClassOf(600), 0);
        list->SelectRowByData(UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->engineSize));
        UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditPlateNumber", 0xb));
        edit->SetAcceptedCharacters("0123456789");
        g_TrackGame->ui->ShowScene(this);
        Vector3* eye = &viewEye;
        previewArea.left = 0x5b;
        previewArea.right = 0x21f;
        previewArea.top = 0x6c;
        previewArea.bottom = 0x148;
        *eye = kVec3Zero;
        float fov = 50.0f;
        Vector3* target = &viewTarget;
        viewEye.z = 15.0f;
        viewEye.y = 1.0f;
        *target = g_TrackGame->ui->field_0x474;
        viewTarget.y += 3.0f;
        g_TrackGame->ui->field_0x468->UnknownFunction42e9b0(eye, 0, 0, 0, &fov);
        g_TrackGame->ui->field_0x468->UnknownVirtualSlot29(*target);
        g_TrackGame->ui->field_0x468->UnknownFunction42f190(
            previewArea.left, previewArea.top, previewArea.right - previewArea.left,
            previewArea.bottom - previewArea.top);
        viewDistance = UnknownVectorLength(UnknownVectorDifference(*eye, g_TrackGame->ui->field_0x474));
        ApplyChosenRider();
        UnknownFunction4500e0();
        PaintPlateNumber(g_TrackGame->mode.field_0x1bcc);
        srand(ReadClock());
        if (dialogBackground)
            previewRegion = dialogBackground->UnknownFunction4040f0(1);
        break;
    }
    case kDialogListSelect:
        if (!_stricmp("DDLBikes", event->controlName))
            UnknownFunction4500e0();
        else if (!_stricmp("DDLRiders", event->controlName))
            ApplyChosenRider();
        else if (!_stricmp("DDLEngineSize", event->controlName))
            UnknownFunction4500e0();
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
            previewDragged = !static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->UnknownFunction4755c0();
        } else if (!_stricmp("ButWrench", event->controlName)) {
            OptionsDlg* dialog = new(__FILE__, 2135) OptionsDlg;
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
            dialogBackground->UnknownFunction404200(previewRegion);
        ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(0);
        break;
    }
}

// 0x0044f750
void SPBikeRiderDlg::FillBikeRiderLists() {
    int bike = 0;
    int rider = 0;
    char text[0x80];
    SPBikeRiderDlg* self = this;
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    list->RemoveAllRows();
    for (int i = 0; i < g_TrackGame->ui->field_0x54; i++) {
        if (!((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[i].field_0x88) {
            int kind = ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[i].model].modelKind;
            if (kind != 2 && kind != 3 && (kind != 10 || !(g_TrackGame->mode.field_0x1970 & 1)))
                continue;
        }
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[i];
        UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[entry->model];
        sprintf(text, "%s %s", model->displayName, entry->displayName);
        list->AddRow(text, i, 0);
        if (g_TrackGame->mode.field_0x1974.field_0xc4 == i)
            bike = i;
    }
    if (bike)
        list->SelectRowByData(bike);
    else
        list->SelectRow(0);
    list->Sort(1);
    list = static_cast<UIDropDownList*>(self->FindControl("DDLRiders", 6))->listPart;
    list->RemoveAllRows();
    for (int j = 0; j < g_TrackGame->ui->field_0x5c; j++) {
        UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[j];
        if (model->modelKind == 2 || model->modelKind == 3 || model->field_0xc0) {
            list->AddRow(model->displayName, j, 0);
            if (!strcmp(((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[j].modelName,
                        g_TrackGame->mode.field_0x1974.field_0x80))
                rider = j;
        }
    }
    list->SelectRowByData(rider);
}


// 0x0044f950
void SPBikeRiderDlg::PaintPlateNumber(int number) {
    UnknownBikeNumberPainter painter(g_TrackGame->field_0x1c);
    for (int i = 0; i < g_TrackGame->ui->field_0x4c; i++)
        painter.UnknownFunction417670(
            ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[i].field_0xc0->plateTexture, number);
}

// 0x0044fa00: a click inside the bike preview (inset by 50 and 30 pixels)
// starts dragging it and turns automatic rotation off.
int SPBikeRiderDlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind == 1 && event->control == 0) {
        CameraRect area = previewArea;
        area.left += 50;
        area.top += 30;
        area.right -= 50;
        area.bottom -= 30;
        if (PtInRect((RECT*)&area, guiUser->pointerDevice->pointerPosition)) {
            previewDragged = 1;
            previewDragging = 1;
            static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->SetCurrentState(0);
        }
    }
    return UIDialog::UnknownVirtualSlot23(event, entry);
}

// 0x0044fab0
int SPBikeRiderDlg::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind == 1 && event->control == 0)
        previewDragging = 0;
    return UIDialog::UnknownVirtualSlot22(event, entry);
}

// 0x0044ffc0
int SPBikeRiderDlg::UnknownVirtualSlot13() {
    int result = UIDialog::UnknownVirtualSlot13();
    if (dialogBackground)
        dialogBackground->UnknownFunction404c80();
    if (bikeChanged) {
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        UnknownKrustyUIBike* bike = &((UnknownKrustyUIBike*)ui->field_0x50)[list->GetRowData(-1)];
        ((UnknownModelTexture*)((UnknownKrustyUIModel*)ui->field_0x48)[bike->model].field_0xc0->plateTexture)
            ->UnknownFunction444c70(0, bike->field_0x48, &g_TrackGame->field_0x1c);
        bikeChanged = 0;
    }
    if (riderChanged) {
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        UnknownKrustyUIModel* rider = &((UnknownKrustyUIModel*)ui->field_0x58)[list->GetRowData(-1)];
        ((UnknownModelTexture*)ui->field_0x46c->plateTexture)
            ->UnknownFunction444c70(0, rider->modelName, &g_TrackGame->field_0x1c);
        riderChanged = 0;
    }
    return result;
}

// 0x004500d0
void SPBikeRiderDlg::ApplyChosenRider() {
    riderChanged = 1;
}

// 0x004500e0: shows the chosen bike and loads its class's garage defaults.
void SPBikeRiderDlg::UnknownFunction4500e0() {
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
    bikeChanged = 1;
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
    g_TrackGame->mode.field_0x1974.field_0xc4 = bikes->GetRowData(-1);
    if (bike->field_0x88) {
        UIListBox* sizes = static_cast<UIDropDownList*>(FindControl("DDLEngineSize", 6))->listPart;
        int size = sizes->GetRowData(-1);
        int previous = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->engineSize);
        UNKNOWN_GARAGE_SETTINGS->engineSize = g_UnknownGlobal56cb6c[size];
        UNKNOWN_GARAGE_SETTINGS->field_0x04 = size == 2 || size == 4 ? 1 : 0;
        int current = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->engineSize);
        if (current != previous)
            UNKNOWN_APPLY_BIKE_CLASS(current, i);
    } else {
        int row = bikes->GetRowData(-1);
        int previous = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->engineSize);
        UNKNOWN_GARAGE_SETTINGS->engineSize =
            ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[row].engineSize;
        UNKNOWN_GARAGE_SETTINGS->field_0x04 =
            ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[row].field_0x90;
        int current = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->engineSize);
        if (current != previous)
            UNKNOWN_APPLY_BIKE_CLASS(current, i);
    }
    UIEditBox* plate = static_cast<UIEditBox*>(FindControl("EditPlateNumber", 0xb));
    _itoa(g_TrackGame->mode.field_0x1bcc, text, 10);
    plate->SetEditText(text);
}

// 0x004505d0
void SPBikeRiderDlg::UnknownVirtualSlot31(int apply) {
    if (!apply)
        return;
    strcpy(g_TrackGame->mode.field_0x1974.field_0x00, "");
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    int bike = list->GetRowData(-1);
    {
        UnknownKrustyUIBike* bikes = (UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50;
        char* name = ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[bikes[bike].model].modelName;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_TrackGame->mode.field_0x1974.field_0x00, name, count);
        g_TrackGame->mode.field_0x1974.field_0x00[count] = 0;
    }
    {
        char* name = ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[bike].field_0x48;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_TrackGame->mode.field_0x1974.field_0x40, name, count);
        g_TrackGame->mode.field_0x1974.field_0x40[count] = 0;
    }
    {
        int length = strlen("");
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_TrackGame->mode.field_0x1974.field_0x80, "", count);
        g_TrackGame->mode.field_0x1974.field_0x80[count] = 0;
    }
    list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
    int rider = list->GetRowData(-1);
    {
        char* name = ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[rider].modelName;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_TrackGame->mode.field_0x1974.field_0x80, name, count);
        g_TrackGame->mode.field_0x1974.field_0x80[count] = 0;
    }
    g_TrackGame->mode.UnknownFunction523580();
}

// 0x00450e30
LoadSupercrossDlg::LoadSupercrossDlg(int flags) : LoadingDlg(flags) {
    strcpy(resourceName, "LoadSX.dtm");
}

// 0x00450e80
LoadEnduroDlg::LoadEnduroDlg(int flags) : LoadingDlg(flags) {
    strcpy(resourceName, "LoadEnd.dtm");
}

// 0x00450ed0
LoadQuarryDlg::LoadQuarryDlg(int flags) : LoadingDlg(flags) {
    strcpy(resourceName, "LoadQuar.dtm");
}

// 0x00450f20
LoadNationalsDlg::LoadNationalsDlg(int flags) : LoadingDlg(flags) {
    strcpy(resourceName, "LoadNat.dtm");
}

// 0x00450f70
LoadTagDlg::LoadTagDlg(int flags) : LoadingDlg(flags) {
    strcpy(resourceName, "LoadTag.dtm");
}

// 0x00450fd0
LoadBajaDlg::LoadBajaDlg(int flags) : LoadingDlg(flags) {
    strcpy(resourceName, "LoadBaja.dtm");
}

// 0x00451020
void LoadingDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogTimer:
        if (g_TrackGame->network) {
            if (g_UnknownGlobal59adfc >= 10) {
                g_TrackGame->mode.field_0x6a4 = 0;
                event->dialog->EndDialog(0);
                event->handled = 1;
                g_TrackGame->eventManager->UnknownFunction45d270();
            }
            g_UnknownGlobal59adfc++;
        } else {
            if (g_UnknownGlobal59adfc >= 10) {
                event->dialog->EndDialog(0);
                event->handled = 1;
                g_TrackGame->UnknownFunction521a40();
                g_TrackGame->UnknownFunction468880();
                g_TrackGame->eventManager->FindRaceView()->UnknownFunction421050();
                if (g_TrackGame->field_0x55c)
                    g_TrackGame->field_0x55c->UnknownFunction4e1f00();
                if (g_TrackGame->field_0x568)
                    g_TrackGame->field_0x568->UnknownFunction4e1f00();
                g_TrackGame->eventManager->UnknownFunction45d270();
            }
            g_UnknownGlobal59adfc++;
        }
        break;
    case kDialogClose:
        RemoveTimers(0);
        guiManager->ShowCursors(0);
        g_TrackGame->ui->field_0x490 = 0;
        break;
    case kDialogInit: {
        UIControl* area = FindControl("StaBarRect", 5);
        UIProgressBar* bar = new(__FILE__, 2878) UIProgressBar(0, (CameraRect*)area->field_0x3c, this);
        AddControl(bar, 0, 0);
        bar->SetName("ProgressBar");
        UnknownDialogImage* image = (UnknownDialogImage*)FindSectionObject("BlueBar");
        bar->UnknownFunction47b3d0(image->GetCurrentTexture(), 0);
        bar->field_0x1f0 = 20;
        bar->showPercentage = 0;
        area->Show(0, 1);
        g_TrackGame->ui->field_0x490 = (UnknownGameUiPage*)this;
        g_TrackGame->ui->UnknownFunction499b10();
        g_UnknownGlobal59ae84 = 0;
        g_TrackGame->mode.UnknownFunction523580();
        if (g_TrackGame->network)
            g_TrackGame->network->StartKeepAlive(
                g_TrackGame->eventManager->keepAliveTimeout,
                g_TrackGame->eventManager->keepAliveInterval);
        g_UnknownGlobal59adfc = 0;
        g_UnknownGlobal59ae88 = 0;
        framesShown = 0;
        break;
    }
    }
}

// 0x00451270
int LoadingDlg::UnknownVirtualSlot10(float frameTime) {
    if (framesShown == 2 && !g_UnknownGlobal59ae84) {
        if (!g_TrackGame->eventManager->UnknownFunction45cb70()) {
            EndDialog(0);
            if (g_TrackGame->network) {
                UnknownLoadFailedMessage message;
                message.field_0x04 = g_TrackGame->network->localPlayer;
                g_TrackGame->network->Send(0xcc, &message, sizeof(message),
                                                                        message.field_0x04, 0);
                EndNetworkGame();
            }
            g_TrackGame->ui->UnknownFunction499b00();
            g_TrackGame->ui->OpenMenu(100);
            return 1;
        }
        if (g_TrackGame->network) {
            for (int i = 0; i < g_TrackGame->mode.field_0x1be0; i++)
                g_TrackGame->mode.field_0x1be4[i].UnknownFunction522050();
        }
        g_UnknownGlobal59ae84 = 1;
        AddTimer(0, 100, 0);
    }
    framesShown++;
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00451380
void UnknownFunction451380(int value, char* text) {
    _itoa(value, text, 10);
}

// 0x00451b80
void CreateProfile(UnknownDialogEvent* event) {
    char name[128];
    char preset[260];
    char control[260];
    UIEditBox* edit = static_cast<UIEditBox*>(event->dialog->FindControl("EditBox", 0));
    edit->GetEditText(name, 16);
    if (!strcmp(name, ""))
        return;
    if (g_TrackGame->profileDirectory->UnknownFunction44a910(name)) {
        ProfileExistsDlg* dialog = new(__FILE__, 3416) ProfileExistsDlg;
        g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 4, 0, (int)event->dialog,
                                                                     (int)name, 0, 1);
        return;
    }
    sprintf(preset, "%s\\%s", "ui\\profile", name);
    if (!CreateDirectoryA(preset, 0))
        return;
    g_TrackGame->mode.UnknownFunction522440();
    COPY_TEXT(g_TrackGame->mode.field_0x00, name, 16);
    g_TrackGame->mode.field_0x9c = 1;
    for (int i = 0; i < g_TrackGame->ui->field_0x54; i++) {
        if (((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[i].engineSize == 250) {
            g_TrackGame->mode.field_0x1974.field_0xc4 = i;
            g_TrackGame->mode.field_0x1974.field_0xc0 =
                ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[i].model;
            strcpy(g_TrackGame->mode.field_0x1974.field_0x00,
                   ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)
                       [((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[i].model].modelName);
            strcpy(g_TrackGame->mode.field_0x1974.field_0x40,
                   ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[i].field_0x48);
            break;
        }
    }
    strcpy(g_TrackGame->mode.field_0x1974.field_0x80,
           ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[0].modelName);
    memcpy(g_TrackGame->mode.field_0x1b04, &g_TrackGame->mode.field_0x1974,
           sizeof(g_TrackGame->mode.field_0x1b04));
    memcpy(&g_TrackGame->mode.field_0x1a3c, g_TrackGame->mode.field_0x1b04,
           sizeof(g_TrackGame->mode.field_0x1a3c));
    for (int type = 0; type < 6; type++) {
        g_TrackGame->mode.field_0x6f4[type][0] = 0;
        g_TrackGame->mode.field_0x9f4[type] = 0;
    }
    strcpy(g_TrackGame->mode.field_0x10, "");
    strcpy(g_TrackGame->mode.field_0x27f8.field_0x36, "");
    g_TrackGame->mode.field_0x27f8.field_0x04 = 0;
    g_TrackGame->mode.field_0x27f8.field_0x34 = 0;
    g_TrackGame->mode.field_0x27f8.field_0x08 = 0;
    sprintf(preset, "%s\\%s", "ui", "preset.ctl");
    sprintf(control, "%s\\%s\\%s", "ui\\profile", name, "control.ctl");
    CopyFileA(preset, control, 0);
    g_TrackGame->mode.UnknownFunction523000();
    g_TrackGame->profileDirectory->UnknownVirtualSlot1();
    g_TrackGame->mode.UnknownFunction523580();
    g_TrackGame->field_0x33fc->UnknownFunction448e90(control, -1);
    g_TrackGame->UnknownFunction521a30();
    event->dialog->EndDialog(0x1e);
    event->handled = 1;
}

// 0x00451ff0
void UserNameDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char playerName[16];
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        g_TrackGame->ui->field_0x4b0 = 1;
        UIControl* control = FindControl("TitleText", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x926);
        control = FindControl("OKButton", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e9);
        control = FindControl("CancelButton", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13dc);
        int menu = g_TrackGame->ui->field_0x3c;
        if (menu == 0xbba || menu == 0xbbb)
            ShowGroup(0x65, 0);
        UIControl* prompt = FindControl("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->SetTextFromResource(g_TrackGame->resourceInstance, 0x13ae);
        UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditBox", 0));
        edit->SetCapacity(15);
        edit->SetAcceptedCharacters(kProfileNameCharacters);
        if (g_TrackGame->ui->field_0x3c == 0xbbb) {
            g_TrackGame->network->GetPlayerName(g_TrackGame->network->localPlayer,
                                                                    playerName);
            edit->SetEditText(playerName);
            edit->controlType = 12;
            CreateProfile(event);
            EndDialog(0);
            FindControl("OkButton", 0)->UnknownVirtualSlot49(0);
        } else {
            guiUser->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
            FindControl("OkButton", 0)->UnknownVirtualSlot49(0);
        }
        break;
    }
    case kDialogCommand:
        if (!_stricmp("CancelButton", event->controlName))
            UnknownFunction4526b0(0xbb9, event);
        else if (!_stricmp("OKButton", event->controlName))
            UnknownFunction452930(0xbb9, event);
        break;
    case 9:
        if (event->code == 0x1f) {
            UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditBox", 0));
            edit->GetEditText(text, 128);
            COPY_TEXT(g_TrackGame->mode.field_0x00, text, 16);
            g_TrackGame->mode.UnknownFunction5231f0();
            EndDialog(0x1f);
        } else {
            UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditBox", 0));
            edit->SetEditText("");
            guiUser->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
        }
        break;
    case kDialogEditChange: {
        UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditBox", 11));
        edit->GetEditText(text, 127);
        if (!text[0])
            FindControl("OkButton", 0)->UnknownVirtualSlot49(0);
        else
            FindControl("OkButton", 0)->UnknownVirtualSlot49(1);
        break;
    }
    case kDialogClose:
        g_TrackGame->ui->field_0x4b0 = 0;
        break;
    }
}

// 0x00452370
void ProfileExistsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char label[128];
    char text[256];
    switch (event->kind) {
    case kDialogInit: {
        UIControl* title = FindControl("TitleText", 12);
        UIControl* button = FindControl("ButLeft", 1);
        button->SetTextFromResource(g_TrackGame->resourceInstance, 0x13ec);
        button->keyBind = 0;
        button = FindControl("ButRight", 1);
        button->SetTextFromResource(g_TrackGame->resourceInstance, 0x13eb);
        button->keyBind = 0;
        button = FindControl("ButMiddle", 1);
        button->Show(0, 1);
        g_TrackGame->LoadResourceString(0x13cf, label, 128);
        sprintf(text, "'%s' %s", (const char*)event->field_0x18, label);
        title->SetText(text);
        UIControl* prompt = FindControl("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->SetTextFromResource(g_TrackGame->resourceInstance, 0x1403);
        break;
    }
    case kDialogCommand:
        if (!_stricmp("ButLeft", event->controlName)) {
            event->dialog->EndDialog(0x1f);
            event->handled = 1;
        } else if (!_stricmp("ButRight", event->controlName)) {
            event->dialog->EndDialog(0);
            event->handled = 1;
        }
        break;
    }
}

// 0x004524f0
void RemoveProfileDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[128];
    switch (event->kind) {
    case 11:
        if (event->key == 0x1b)
            UnknownFunction4526b0(0x12f, event);
        else if (event->key == 0xd)
            UnknownFunction452930(0x12f, event);
        break;
    case kDialogInit: {
        UIControl* title = FindControl("TitleText", 0);
        g_TrackGame->LoadResourceString(0xfef, text, 128);
        title->SetTextAlign(10);
        title->SetText(text);
        sprintf(text, "\"%s\"", (const char*)event->field_0x18);
        UIControl* prompt = FindControl("TxtPrompt", 12);
        prompt->SetText(text);
        UIControl* button = FindControl("ButLeft", 0);
        g_TrackGame->LoadResourceString(0x13e9, text, 128);
        button->SetText(text);
        button = FindControl("ButRight", 0);
        g_TrackGame->LoadResourceString(0x13dc, text, 128);
        button->SetText(text);
        button = FindControl("ButMiddle", 1);
        button->Show(0, 1);
        break;
    }
    case kDialogCommand:
        if (!_stricmp("ButRight", event->controlName))
            UnknownFunction4526b0(0x12f, event);
        else if (!_stricmp("ButLeft", event->controlName))
            UnknownFunction452930(0x12f, event);
        break;
    }
}

// 0x004526b0
void UnknownFunction4526b0(int menu, UnknownDialogEvent* event) {
    switch (menu) {
    case 0xc9:
        g_TrackGame->ui->OpenMenu(0x66);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xca:
        g_TrackGame->ui->OpenMenu(0x66);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xcb:
        g_TrackGame->ui->OpenMenu(0x66);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xcd:
        g_TrackGame->ui->OpenMenu(g_TrackGame->ui->field_0x30);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xd0:
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xd8:
        event->dialog->EndDialog(0xe);
        event->handled = 1;
        break;
    case 0xd9:
        event->dialog->EndDialog(0xe);
        event->handled = 1;
        break;
    case 0x104:
    case 0x105:
        g_TrackGame->ui->OpenMenu(0x65);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0x12c:
        event->dialog->EndDialog(0x15);
        event->handled = 1;
        break;
    case 0x12f:
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0x191: {
        g_TrackGame->SetMenuOpen(0, 0x191, 0);
        TrackGameViewOwner* owner = g_TrackGame->field_0x568;
        if (owner && g_TrackGame->network && g_TrackGame->network->isHost &&
            owner->field_0xa8 && owner->field_0x34 && owner->field_0xa8 == owner->field_0x34->field_0x38) {
            UnknownEventRacer* racer = owner->field_0x34->field_0x38;
            if (racer)
                racer->field_0x4a0 = 1;
            g_TrackGame->field_0x568->UnknownFunction4a9d20();
        }
        g_TrackGame->eventManager->UnknownFunction45cdc0(2);
        if (g_TrackGame->network) {
            g_TrackGame->network->SetSessionJoinable(1);
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
                EndNetworkGame();
                g_TrackGame->eventManager->UnknownFunction45e710(100);
            } else {
                g_TrackGame->eventManager->UnknownFunction45e710(0x867);
            }
        } else if (!g_TrackGame->field_0x3444) {
            g_TrackGame->eventManager->UnknownFunction45e710(0x65);
        } else {
            g_TrackGame->eventManager->UnknownFunction45e710(0x88f);
        }
        break;
    }
    case 0x1f9:
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xbb9:
    case 0xbba:
    case 0xbbb:
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    }
}

// 0x00453090
void UnknownFunction453090(int menu, UnknownDialogEvent* event) {
    switch (menu) {
    case 0xfc:
        event->dialog->EndDialog(0xe);
        event->handled = 1;
        break;
    case 0x66:
        g_TrackGame->ui->OpenMenu(g_TrackGame->ui->field_0x30);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0x65:
        g_TrackGame->ui->OpenMenu(100);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0x910:
    case 0x911: {
        KrustyUI* ui = g_TrackGame->ui;
        if (ui->field_0x3c == 0x910)
            ui->OpenMenu(0x65);
        else if (ui->field_0x3c == 0x911)
            ui->OpenMenu(ui->field_0x38);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    }
    case 0x10e:
        g_TrackGame->eventManager->ResetEntries();
        if (g_TrackGame->network)
            g_TrackGame->ui->OpenMenu(0x867);
        else
            g_TrackGame->ui->OpenMenu(0x65);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0x190:
        g_TrackGame->SetMenuOpen(0, 0x190, 1);
        break;
    }
}

// 0x00453170
void CreditsVidDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogClose:
        if (!g_TrackGame->field_0x2d5_bit1)
            g_TrackGame->ui->OpenMenu(100);
        break;
    case kDialogInit: {
        CameraRect area;
        area.left = 0;
        area.top = 0;
        area.right = 640;
        area.bottom = 480;
        field_0x7f58 = new(__FILE__, 4108) UIVideoStatic(0, &area, this);
        AddControl(field_0x7f58, 0, 1);
        field_0x7f58->UnknownFunction4691f0();
        field_0x7f58->UnknownFunction469260(FindControl("Back", 0), -1);
        if (!field_0x7f58->UnknownFunction47ae90("ui\\Credits.avi", 1, CloseDialogCallback, this))
            EndDialog(0);
        break;
    }
    case kDialogCommand:
        if (!_stricmp("Back", event->controlName))
            EndDialog(0);
        break;
    }
}

// 0x004532c0
void Intro1Dlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogClose:
        if (!g_TrackGame->field_0x2d5_bit1) {
            Intro2Dlg* dialog = new(__FILE__, 4217) Intro2Dlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 0);
        }
        break;
    case kDialogInit:
        g_TrackGame->ui->field_0x2c->ShowCursors(0);
        framesShown = 0;
        g_UnknownGlobal59ae00 = !g_TrackGame->mode.field_0x6cc;
        break;
    }
}

// 0x004533d0
int Intro1Dlg::UnknownVirtualSlot10(float frameTime) {
    if (++framesShown == 3) {
        g_TrackGame->ui->UnknownFunction498cf0(1);
        EndDialog(0);
    }
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00453420
int Intro1Dlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    return g_UnknownGlobal59ae00 = 1;
}

// 0x00453430
void Intro2Dlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogClose:
        if (!g_TrackGame->field_0x2d5_bit1) {
            if (!g_UnknownGlobal59ae00) {
                Intro3Dlg* dialog = new(__FILE__, 4256) Intro3Dlg;
                g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 0);
            } else {
                g_TrackGame->ui->OpenMenu(100);
                g_TrackGame->ui->field_0x2c->ShowCursors(1);
            }
        }
        break;
    case kDialogInit:
        frameRan = 0;
        secondsShown = 0.0f;
        framesShown = 0;
        g_TrackGame->UnknownFunction468880();
        break;
    }
}

// 0x00453570
int Intro2Dlg::UnknownVirtualSlot10(float frameTime) {
    if (++framesShown == 3)
        g_TrackGame->ui->UnknownFunction498cf0(2);
    secondsShown = frameTime + secondsShown;
    frameRan = 1;
    if ((g_UnknownGlobal59ae00 || secondsShown > 3.0f) && framesShown >= 3)
        EndDialog(0);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x004535f0
int Intro2Dlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (frameRan == 1)
        g_UnknownGlobal59ae00 = 1;
    return 1;
}

// 0x00453610
void Intro3Dlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogClose:
        if (!g_TrackGame->field_0x2d5_bit1) {
            g_TrackGame->ui->OpenMenu(100);
            g_TrackGame->ui->field_0x2c->ShowCursors(1);
        }
        break;
    case kDialogInit:
        frameRan = 0;
        secondsShown = 0.0f;
        g_TrackGame->UnknownFunction468880();
        break;
    }
}

// 0x00453670
int Intro3Dlg::UnknownVirtualSlot10(float frameTime) {
    secondsShown = frameTime + secondsShown;
    frameRan = 1;
    if (g_UnknownGlobal59ae00 || secondsShown > 3.0f)
        EndDialog(0);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x004536c0
int Intro3Dlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (frameRan == 1)
        g_UnknownGlobal59ae00 = 1;
    return 1;
}

// 0x004536e0: leaves the menus for the race: keeps a copy of the settings and
// opens the loading dialog of the event type.
void UnknownFunction4536e0() {
    g_TrackGame->mode.field_0x2dbc = 0;
    g_TrackGame->ui->field_0x4ac = 1;
    g_TrackGame->ui->field_0x4a8 = 0;
    g_TrackGame->ui->Shutdown();
    g_TrackGame->ui->field_0x2c->ReleaseBackground();
    if (g_TrackGame->display->freezeFrameIndex)
        g_TrackGame->ui->field_0x2c->EnableWindowClipper(0);
    if (g_TrackGame->field_0x18 == 1 && !g_TrackGame->UnknownFunction521cd0() &&
        !g_TrackGame->ui->field_0x4a8 && !g_TrackGame->field_0x3428) {
        memcpy(&g_TrackGame->mode.field_0x29e4, &g_TrackGame->mode.field_0x27f8,
               sizeof(g_TrackGame->mode.field_0x29e4));
        memcpy(g_TrackGame->mode.field_0x1034, g_TrackGame->mode.field_0xfd8,
               sizeof(g_TrackGame->mode.field_0x1034));
        memcpy(&g_TrackGame->mode.field_0x1a3c, &g_TrackGame->mode.field_0x1974,
               sizeof(g_TrackGame->mode.field_0x1a3c));
        g_TrackGame->mode.field_0x98 = g_TrackGame->mode.field_0x94;
    }
    if (g_TrackGame->field_0x18 > 1 && !g_TrackGame->ui->field_0x4a8 &&
        !g_TrackGame->field_0x3428) {
        memcpy(&g_TrackGame->mode.field_0x2bd0, &g_TrackGame->mode.field_0x27f8,
               sizeof(g_TrackGame->mode.field_0x2bd0));
        memcpy(g_TrackGame->mode.field_0x1090, g_TrackGame->mode.field_0xfd8,
               sizeof(g_TrackGame->mode.field_0x1090));
        memcpy(g_TrackGame->mode.field_0x1b04, &g_TrackGame->mode.field_0x1974,
               sizeof(g_TrackGame->mode.field_0x1b04));
    }
    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2 && !g_TrackGame->mode.field_0x10ec) {
        g_TrackGame->mode.field_0x10ec = 1;
        g_TrackGame->eventManager->field_0x48 = 0;
        for (int i = 0; i < 11; i++)
            g_TrackGame->eventManager->field_0x50[i].field_0x28 = 0;
    }
    g_TrackGame->ui->field_0x494 = (void*)ImmAssociateContext((HWND)g_TrackGame->windowHandle, 0);
    if (g_TrackGame->ui->field_0x2c->field_0x03c)
        g_TrackGame->ui->field_0x2c->field_0x03c->UnknownFunction404c80();
    if (g_TrackGame->mode.field_0xa4c != g_TrackGame->display->currentDisplayMode)
        g_TrackGame->UnknownVirtualSlot19(g_TrackGame->mode.field_0xa4c);
    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
        g_TrackGame->mode.field_0x26f0 = 0;
        g_TrackGame->mode.field_0x2dbc = 0;
        g_TrackGame->mode.field_0x94 = 2;
    }
    switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
    case 3:
        g_TrackGame->ui->field_0x2c->ShowDialog(new(__FILE__, 4405) LoadSupercrossDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 5:
        g_TrackGame->ui->field_0x2c->ShowDialog(new(__FILE__, 4408) LoadEnduroDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 0:
        g_TrackGame->ui->field_0x2c->ShowDialog(new(__FILE__, 4411) LoadQuarryDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 4:
        g_TrackGame->ui->field_0x2c->ShowDialog(new(__FILE__, 4414) LoadTagDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 1:
        g_TrackGame->ui->field_0x2c->ShowDialog(new(__FILE__, 4417) LoadBajaDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 2:
        g_TrackGame->ui->field_0x2c->ShowDialog(new(__FILE__, 4420) LoadNationalsDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    }
}

// 0x00453b20
void GhostReplayDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogInit: {
        field_0x7f58 = 0;
        field_0x7f5c = 0;
        UIMultiState* tab = static_cast<UIMultiState*>(FindControl("TabLeft", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x143a);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x143a);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIMultiState*>(FindControl("TabRight", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1439);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1439);
        tab->SetTextAlign(0x22);
        OpenPage(1);
        static_cast<UIRadioButton*>(FindControl("TabRight", 4))->SelectInGroup(0);
        break;
    }
    case kDialogCommand:
        if (!_stricmp("Back", event->controlName)) {
            if (field_0x7f58) {
                field_0x7f58->Release();
                field_0x7f58 = 0;
            }
            if (field_0x7f5c) {
                field_0x7f5c->Release();
                field_0x7f5c = 0;
            }
            event->dialog->EndDialog(0);
            event->handled = 1;
            g_TrackGame->ui->OpenMenu(100);
        } else if (!_stricmp("TabLeft", event->controlName)) {
            OpenPage(0);
        } else if (!_stricmp("TabRight", event->controlName)) {
            OpenPage(1);
        } else if (!_stricmp("Help", event->controlName)) {
            g_TrackGame->mode.OpenHelp("MCM2HELP", 0);
        } else if (!_stricmp("Start", event->controlName)) {
            if (field_0x7f58)
                field_0x7f58->RaceSelectedGhost();
            else if (field_0x7f5c)
                field_0x7f5c->PlaySelectedReplay();
            EndDialog(0);
        }
        break;
    }
}

// 0x00453e90
void GhostReplayDlg::OpenPage(int page) {
    CameraRect area;
    area.left = 20;
    area.top = 35;
    area.right = 620;
    area.bottom = 395;
    if (field_0x7f58 && page != 0) {
        field_0x7f58->Release();
        field_0x7f58 = 0;
    }
    if (field_0x7f5c && page != 1) {
        field_0x7f5c->Release();
        field_0x7f5c = 0;
    }
    switch (page) {
    case 0:
        if (!field_0x7f58) {
            field_0x7f58 = new(__FILE__, 4498) GhostFilesDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(field_0x7f58, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 1:
        if (!field_0x7f5c) {
            field_0x7f5c = new(__FILE__, 4504) ReplayFilesDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(field_0x7f5c, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    }
}

// 0x00454030
void GhostFilesDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[256];
    switch (event->kind) {
    case kDialogInit:
        fileHeaderCount = 0;
        fileHeaders = 0;
        filePathCount = 0;
        filePaths = 0;
        ListGhostFiles();
        FindControl("LstLaps", 0)->Show(0, 1);
        FindControl("TxtLaps", 0)->Show(0, 1);
        break;
    case kDialogCommand:
        if (!_stricmp("ButDelete", event->controlName)) {
            if (fileHeaderCount) {
                UIListBox* list = static_cast<UIListBox*>(FindControl("LstTrack", 3));
                sprintf(text, "%s, %s", list->GetRowText(-1),
                        fileHeaders[list->GetRowData(-1)].description);
                ChoiceDlg* dialog = new(__FILE__, 4533) ChoiceDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0,
                                                  (UIDialog*)this, 0, 0, 1);
                dialog->SetTextsOrResources(0, 0x143c, text, 0, 0, 0x143e, 0, 0, 0, 0x143d);
            }
        } else if (!_stricmp("ButDescription", event->controlName)) {
            if (fileHeaderCount) {
                COPY_TEXT(editedDescription, static_cast<UIListBox*>(FindControl("LstDesc", 3))->GetRowText(-1), 32);
                EditBoxDlg* dialog = new(__FILE__, 4545) EditBoxDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0,
                                                  (UIDialog*)this, 0, 0, 1);
                dialog->EditWithResources(0, 0x143f, 0, 0, editedDescription, 32);
            }
        }
        break;
    case 9: {
        int code = event->code;
        if (code == 0x65) {
            UIListBox* list = static_cast<UIListBox*>(FindControl("LstTrack", 3));
            DeleteFileA(filePaths[list->GetRowData(-1)]);
            ListGhostFiles();
        } else if (code == 0xc9) {
            UIListBox* list = static_cast<UIListBox*>(FindControl("LstTrack", 3));
            UnknownRecordFileHeader* header = &fileHeaders[list->GetRowData(-1)];
            strcpy(header->description, editedDescription);
            char* path = filePaths[list->GetRowData(-1)];
            FILE* file = fopen(path, "rb+");
            if (file) {
                fseek(file, 0, SEEK_SET);
                fwrite(header, sizeof(UnknownRecordFileHeader), 1, file);
                fclose(file);
            }
            ListGhostFiles();
        }
        break;
    }
    case kDialogClose:
        if (fileHeaders)
            DebugFree(fileHeaders, __FILE__, 4580);
        if (filePaths) {
            for (int i = 0; i < filePathCount; i++)
                DebugFree(filePaths[i], __FILE__, 4584);
            DebugFree(filePaths, __FILE__, 4586);
        }
        break;
    }
}

// 0x00454470
void GhostFilesDlg::ListGhostFiles() {
    char path[260];
    WIN32_FIND_DATAA data;
    int count = 0;
    if (fileHeaders)
        DebugFree(fileHeaders, __FILE__, 4599);
    if (filePaths) {
        for (int i = 0; i < filePathCount; i++)
            DebugFree(filePaths[i], __FILE__, 4603);
        DebugFree(filePaths, __FILE__, 4605);
    }
    fileHeaders = 0;
    filePaths = 0;
    filePathCount = 0;
    fileHeaderCount = 0;
    static_cast<UIListBox*>(FindControl("LstTrack", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstRider", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstLength", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDesc", 3))->RemoveAllRows();
    HANDLE find = FindFirstFileA("Record\\*.gho", &data);
    if (find != INVALID_HANDLE_VALUE) {
        sprintf(path, "%s\\%s", "Record", data.cFileName);
        if (AddGhostFile(path))
            count++;
        while (FindNextFileA(find, &data)) {
            sprintf(path, "%s\\%s", "Record", data.cFileName);
            if (AddGhostFile(path))
                count++;
        }
        FindClose(find);
    }
    parentDialog->FindControl("Start", 0)->Show(count != 0, 1);
}

// 0x00454640
int GhostFilesDlg::AddGhostFile(const char* path) {
    char name[128];
    char text[64];
    char env[260];
    int added = 0;
    UnknownRecordFileHeader header;
    FILE* file = fopen(path, "rb");
    if (file) {
        if (fread(&header, sizeof(header), 1, file) && header.lengthSeconds > 0.0f) {
            fileHeaders = (UnknownRecordFileHeader*)UnknownFunction47b570(
                fileHeaders, (fileHeaderCount + 1) * sizeof(UnknownRecordFileHeader));
            fileHeaders[fileHeaderCount] = header;
            fileHeaderCount++;
            filePaths = (char**)UnknownFunction47b570(filePaths, (filePathCount + 1) * sizeof(char*));
            filePaths[filePathCount] = (char*)DebugMalloc(strlen(path) + 1, __FILE__, 4657);
            strcpy(filePaths[filePathCount], path);
            filePathCount++;
            UIListBox* track = static_cast<UIListBox*>(FindControl("LstTrack", 3));
            UIListBox* rider = static_cast<UIListBox*>(FindControl("LstRider", 3));
            UIListBox* length = static_cast<UIListBox*>(FindControl("LstLength", 3));
            UIListBox* description = static_cast<UIListBox*>(FindControl("LstDesc", 3));
            g_TrackGame->mode.UnknownFunction5240e0(header.series);
            g_TrackGame->mode.FindFileDirectory(g_TrackGame->mode.field_0x6a0,
                                                              header.trackName, "env", env);
            g_TrackGame->sceneObject->UnknownFunction4e9b80(env);
            rider->AddRow(header.riderName, 0, 0);
            g_TrackGame->sceneObject->UnknownFunction4ea010(text, header.trackName, 0, "scn", 0, 0);
            COPY_TEXT(name, text, 128);
            if (!_stricmp(name, "no name"))
                g_TrackGame->LoadResourceString(0x143b, name, 128);
            track->AddRow(name, fileHeaderCount - 1, 0);
            FormatTime(name, header.lengthSeconds);
            length->AddRow(name, 0, 0);
            description->AddRow(header.description, 0, 0);
            added = 1;
        }
        fclose(file);
    }
    return added;
}

// 0x00454970
void GhostFilesDlg::RaceSelectedGhost() {
    UIListBox* list = static_cast<UIListBox*>(FindControl("LstTrack", 3));
    UnknownRecordFileHeader* header = &fileHeaders[list->GetRowData(-1)];
    memcpy(&g_TrackGame->mode.field_0x27f8, &header->eventType, sizeof(g_TrackGame->mode.field_0x29e4));
    COPY_TEXT(g_TrackGame->mode.field_0x25ec, filePaths[list->GetRowData(-1)], 0x104);
    parentDialog->UnknownVirtualSlot26();
    g_TrackGame->mode.UnknownFunction5240e0(header->series);
    UnknownFunction4536e0();
}

// 0x00454a50
void ReplayFilesDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[256];
    switch (event->kind) {
    case kDialogInit:
        field_0x7f5c = 0;
        field_0x7f58 = 0;
        field_0x7f64 = 0;
        field_0x7f60 = 0;
        ListReplayFiles();
        break;
    case kDialogCommand:
        if (!_stricmp("ButDelete", event->controlName)) {
            if (field_0x7f5c) {
                UIListBox* list = static_cast<UIListBox*>(FindControl("LstTrack", 3));
                sprintf(text, "%s, %s", list->GetRowText(-1),
                        field_0x7f58[list->GetRowData(-1)].description);
                ChoiceDlg* dialog = new(__FILE__, 4744) ChoiceDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0,
                                                  (UIDialog*)this, 0, 0, 1);
                dialog->SetTextsOrResources(0, 0x143c, text, 0, 0, 0x143e, 0, 0, 0, 0x143d);
            }
        } else if (!_stricmp("ButDescription", event->controlName)) {
            if (field_0x7f5c) {
                COPY_TEXT(field_0x7f68, static_cast<UIListBox*>(FindControl("LstDesc", 3))->GetRowText(-1), 32);
                EditBoxDlg* dialog = new(__FILE__, 4756) EditBoxDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0,
                                                  (UIDialog*)this, 0, 0, 1);
                dialog->EditWithResources(0, 0x143f, 0, 0, field_0x7f68, 32);
            }
        }
        break;
    case 9: {
        int code = event->code;
        if (code == 0x65) {
            UIListBox* list = static_cast<UIListBox*>(FindControl("LstTrack", 3));
            DeleteFileA(field_0x7f60[list->GetRowData(-1)]);
            ListReplayFiles();
        } else if (code == 0xc9) {
            UIListBox* list = static_cast<UIListBox*>(FindControl("LstTrack", 3));
            UnknownRecordFileHeader* header = &field_0x7f58[list->GetRowData(-1)];
            COPY_TEXT(header->description, field_0x7f68, 32);
            char* path = field_0x7f60[list->GetRowData(-1)];
            FILE* file = fopen(path, "rb+");
            if (file) {
                fseek(file, 0, SEEK_SET);
                fwrite(header, sizeof(UnknownRecordFileHeader), 1, file);
                fclose(file);
            }
            ListReplayFiles();
        }
        break;
    }
    case kDialogClose:
        if (field_0x7f58)
            DebugFree(field_0x7f58, __FILE__, 4791);
        if (field_0x7f60) {
            for (int i = 0; i < field_0x7f64; i++)
                DebugFree(field_0x7f60[i], __FILE__, 4795);
            DebugFree(field_0x7f60, __FILE__, 4797);
        }
        break;
    }
}

// 0x00454e60
void ReplayFilesDlg::ListReplayFiles() {
    char path[260];
    WIN32_FIND_DATAA data;
    int count = 0;
    if (field_0x7f58)
        DebugFree(field_0x7f58, __FILE__, 4810);
    if (field_0x7f60) {
        for (int i = 0; i < field_0x7f64; i++)
            DebugFree(field_0x7f60[i], __FILE__, 4814);
        DebugFree(field_0x7f60, __FILE__, 4816);
    }
    field_0x7f58 = 0;
    field_0x7f60 = 0;
    field_0x7f64 = 0;
    field_0x7f5c = 0;
    static_cast<UIListBox*>(FindControl("LstTrack", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstRider", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstLaps", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstLength", 3))->RemoveAllRows();
    static_cast<UIListBox*>(FindControl("LstDesc", 3))->RemoveAllRows();
    HANDLE find = FindFirstFileA("Record\\*.vcr", &data);
    if (find != INVALID_HANDLE_VALUE) {
        sprintf(path, "%s\\%s", "Record", data.cFileName);
        if (AddReplayFile(path))
            count++;
        while (FindNextFileA(find, &data)) {
            sprintf(path, "%s\\%s", "Record", data.cFileName);
            if (AddReplayFile(path))
                count++;
        }
        FindClose(find);
    }
    parentDialog->FindControl("Start", 0)->Show(count != 0, 1);
}

// 0x00455040
int ReplayFilesDlg::AddReplayFile(const char* path) {
    char name[128];
    char text[64];
    char env[260];
    int added = 0;
    UnknownRecordFileHeader header;
    FILE* file = fopen(path, "rb");
    if (file) {
        if (fread(&header, sizeof(header), 1, file) && header.lengthSeconds > 0.0f) {
            field_0x7f58 = (UnknownRecordFileHeader*)UnknownFunction47b570(
                field_0x7f58, (field_0x7f5c + 1) * sizeof(UnknownRecordFileHeader));
            field_0x7f58[field_0x7f5c] = header;
            field_0x7f5c++;
            field_0x7f60 = (char**)UnknownFunction47b570(field_0x7f60, (field_0x7f64 + 1) * sizeof(char*));
            field_0x7f60[field_0x7f64] = (char*)DebugMalloc(strlen(path) + 1, __FILE__, 4870);
            strcpy(field_0x7f60[field_0x7f64], path);
            field_0x7f64++;
            UIListBox* track = static_cast<UIListBox*>(FindControl("LstTrack", 3));
            UIListBox* rider = static_cast<UIListBox*>(FindControl("LstRider", 3));
            UIListBox* laps = static_cast<UIListBox*>(FindControl("LstLaps", 3));
            UIListBox* length = static_cast<UIListBox*>(FindControl("LstLength", 3));
            UIListBox* description = static_cast<UIListBox*>(FindControl("LstDesc", 3));
            g_TrackGame->mode.UnknownFunction5240e0(header.series);
            g_TrackGame->mode.FindFileDirectory(g_TrackGame->mode.field_0x6a0,
                                                              header.trackName, "env", env);
            g_TrackGame->sceneObject->UnknownFunction4e9b80(env);
            strcpy(name, "--");
            switch (header.raceKind) {
            case 3:
                _itoa(header.laps, name, 10);
                break;
            case 2:
                _itoa(header.laps, name, 10);
                break;
            case 1:
                _itoa(header.laps, name, 10);
                break;
            case 5:
                _itoa(header.laps, name, 10);
                break;
            }
            laps->AddRow(name, 0, 0);
            rider->AddRow(header.riderName, 0, 0);
            g_TrackGame->sceneObject->UnknownFunction4ea010(text, header.trackName, header.trackIndex,
                                                                      "scn", 0, 0);
            if (!_stricmp(text, "no name")) {
                g_TrackGame->LoadResourceString(0x143b, name, 128);
            } else if ((header.raceKind == 1 || header.raceKind == 5) && header.field_0x60) {
                g_TrackGame->sceneObject->UnknownFunction4ea010(text, header.trackName, 0, "scn", 0, 0);
                sprintf(name, "%s, #%d", text, header.eventNumber);
            } else {
                strcpy(name, text);
            }
            track->AddRow(name, field_0x7f5c - 1, 0);
            FormatTime(name, header.lengthSeconds);
            length->AddRow(name, 0, 0);
            description->AddRow(header.description, 0, 0);
            added = 1;
        }
        fclose(file);
    }
    return added;
}

// 0x00455490
void ReplayFilesDlg::PlaySelectedReplay() {
    UIListBox* list = static_cast<UIListBox*>(FindControl("LstTrack", 3));
    UnknownRecordFileHeader* header = &field_0x7f58[list->GetRowData(-1)];
    memcpy(&g_TrackGame->mode.field_0x27f8, &header->eventType, sizeof(g_TrackGame->mode.field_0x29e4));
    g_TrackGame->mode.UnknownFunction5240e0(header->series);
    g_TrackGame->field_0x3428 = 1;
    g_TrackGame->field_0x342c = 1;
    *(int*)&g_TrackGame->mode.field_0xfd8[0] = header->field_0x33c;
    *(int*)&g_TrackGame->mode.field_0xfd8[4] = header->field_0x340;
    COPY_TEXT(g_TrackGame->mode.field_0x26f4, field_0x7f60[list->GetRowData(-1)], 0x104);
    parentDialog->UnknownVirtualSlot26();
    UnknownFunction4536e0();
}

// 0x004555b0
void ChoiceDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ButLeft", event->controlName))
            EndDialog(0x65);
        else if (!_stricmp("ButMiddle", event->controlName))
            EndDialog(0x66);
        else if (!_stricmp("ButRight", event->controlName))
            EndDialog(0x67);
        break;
    }
}

// 0x00455630
void ChoiceDlg::SetTexts(const char* title, const char* prompt, const char* left,
                         const char* middle, const char* right) {
    FindControl("TitleText", 0)->SetText(title);
    FindControl("TxtPrompt", 0)->SetText(prompt);
    UIControl* button = FindControl("ButLeft", 0);
    if (left && *left)
        button->SetText(left);
    else
        button->Show(0, 1);
    button = FindControl("ButMiddle", 0);
    if (middle && *middle)
        button->SetText(middle);
    else
        button->Show(0, 1);
    button = FindControl("ButRight", 0);
    if (right && *right)
        button->SetText(right);
    else
        button->Show(0, 1);
}

// 0x00455700
void ChoiceDlg::SetTextsOrResources(const char* title, int titleId, const char* prompt, int promptId,
                                    const char* left, int leftId, const char* middle, int middleId,
                                    const char* right, int rightId) {
    char middleText[128];
    char leftText[128];
    char titleText[128];
    char rightText[128];
    char promptText[1024];
    if (titleId)
        g_TrackGame->LoadResourceString(titleId, titleText, 128);
    if (promptId)
        g_TrackGame->LoadResourceString(promptId, promptText, 128);
    if (leftId)
        g_TrackGame->LoadResourceString(leftId, leftText, 128);
    if (middleId)
        g_TrackGame->LoadResourceString(middleId, middleText, 128);
    if (rightId)
        g_TrackGame->LoadResourceString(rightId, rightText, 128);
    SetTexts(titleId ? titleText : title, promptId ? promptText : prompt,
             leftId ? leftText : left, middleId ? middleText : middle,
             rightId ? rightText : right);
}

// 0x00455840
void EditBoxDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogInit:
        editBuffer = 0;
        editBufferSize = 0;
        guiUser->UnknownFunction487790((UnknownGuiControl*)FindControl("EditBox", 0), 0, 0);
        break;
    case kDialogCommand:
        if (!_stricmp("OkButton", event->controlName)) {
            static_cast<UIEditBox*>(FindControl("EditBox", 11))->GetEditText(editBuffer, editBufferSize);
            EndDialog(0xc9);
        } else if (!_stricmp("CancelButton", event->controlName)) {
            EndDialog(0xca);
        }
        break;
    }
}

// 0x004558f0
void EditBoxDlg::Edit(const char* title, const char* prompt, char* buffer, int size) {
    editBufferSize = size;
    editBuffer = buffer;
    FindControl("TitleText", 0)->SetText(title);
    FindControl("TxtPrompt", 0)->SetText(prompt);
    UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditBox", 11));
    edit->SetCapacity(editBufferSize - 1);
    edit->SetEditText(editBuffer);
}

// 0x00455970
void EditBoxDlg::EditWithResources(const char* title, int titleId, const char* prompt, int promptId,
                                   char* buffer, int size) {
    char titleText[128];
    char promptText[1024];
    if (titleId)
        g_TrackGame->LoadResourceString(titleId, titleText, 128);
    if (promptId)
        g_TrackGame->LoadResourceString(promptId, promptText, 128);
    Edit(titleId ? titleText : title, promptId ? promptText : prompt, buffer, size);
}

// 0x00455a10
void DemoDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogClose:
        if (!g_TrackGame->field_0x2d5_bit1) {
            g_TrackGame->eventManager->UnknownFunction45cdc0(2);
            memcpy(&g_TrackGame->mode.field_0x27f8, &g_TrackGame->mode.field_0x29e4,
                   sizeof(g_TrackGame->mode.field_0x29e4));
            memcpy(g_TrackGame->mode.field_0xfd8, g_TrackGame->mode.field_0x1034,
                   sizeof(g_TrackGame->mode.field_0x1034));
            memcpy(&g_TrackGame->mode.field_0x1974, &g_TrackGame->mode.field_0x1a3c,
                   sizeof(g_TrackGame->mode.field_0x1a3c));
            g_TrackGame->eventManager->UnknownFunction45e710(100);
            guiManager->ShowCursors(1);
        }
        break;
    case kDialogCommand:
        if (!_stricmp("Back", event->controlName))
            EndDialog(0);
        break;
    }
}

// 0x00455ae0
int DemoDlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
    if (view && view->field_0x18a)
        EndDialog(0);
    return 1;
}

// 0x00455b20
int DemoDlg::UnknownVirtualSlot10(float frameTime) {
    KrustyUI* ui = g_TrackGame->ui;
    if (ui->field_0x44) {
        ui->field_0x44 = 0;
        EndDialog(0);
        return 0;
    }
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00455b60
void TransDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogClose:
        if (!g_TrackGame->field_0x2d5_bit1) {
            g_TrackGame->ui->OpenMenu(nextMenu);
            g_TrackGame->ui->field_0x2c->ShowCursors(1);
        }
        break;
    case kDialogInit:
        nextMenu = 100;
        framesShown = 0;
        secondsShown = 0.0f;
        g_TrackGame->UnknownFunction468880();
        break;
    }
}

// 0x00455bd0
int TransDlg::UnknownVirtualSlot10(float frameTime) {
    secondsShown = frameTime + secondsShown;
    framesShown++;
    if (secondsShown > 5.0f && framesShown >= 3)
        EndDialog(0);
    if (framesShown >= 3)
        g_TrackGame->ui->UnknownFunction498cf0(-1);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00455c40
void TransDlg::SetNextMenu(int menu) {
    nextMenu = menu;
}

// 0x00455c50
void Exit1Dlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogClose:
        if (!g_TrackGame->field_0x2d5_bit1)
            PostMessageA((HWND)g_TrackGame->display->windowHandle, WM_CLOSE, 0, 0);
        break;
    case kDialogInit:
        frameRan = 0;
        secondsShown = 0.0f;
        framesShown = 0;
        field_0x7f64 = 0;
        g_TrackGame->UnknownFunction468880();
        guiManager->ShowCursors(1);
        break;
    case kDialogCommand:
        if (!_stricmp("GoLink", event->controlName)) {
            g_TrackGame->openStorePageOnExit = 1;
            field_0x7f64 = 1;
        }
        break;
    }
}

// 0x00455d00
int Exit1Dlg::UnknownVirtualSlot10(float frameTime) {
    secondsShown = frameTime + secondsShown;
    framesShown++;
    frameRan = 1;
    if ((field_0x7f64 || secondsShown > 15.0f) && framesShown >= 3)
        EndDialog(0);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00455d70
int Exit1Dlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (frameRan == 1 && !event->kind)
        field_0x7f64 = 1;
    return UIDialog::UnknownVirtualSlot23(event, entry);
}
