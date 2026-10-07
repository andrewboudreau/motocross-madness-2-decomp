#include "DlgProcs.h"

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
int UnknownFunction44b0a0(const void* a, const void* b) {
    int first = atoi(((const UnknownGameUiListRow*)a)->field_0x14);
    int second = atoi(((const UnknownGameUiListRow*)b)->field_0x14);
    if (first < second)
        return -1;
    return first != second;
}

// 0x0044b0e0
int UnknownFunction44b0e0(const void* a, const void* b) {
    int first = atoi(((const UnknownGameUiListRow*)a)->field_0x14);
    int second = atoi(((const UnknownGameUiListRow*)b)->field_0x14);
    if (first > second)
        return -1;
    return first != second;
}

// 0x0044b120
int UnknownFunction44b120(UnknownDialogEvent* event) {
    UIListBox* list = static_cast<UIListBox*>(event->field_0x0c->UnknownFunction46ebf0("LstProfiles", 0));
    short selection = list->UnknownFunction476950();
    if (selection != -1) {
        char name[16];
        COPY_TEXT(name, list->UnknownFunction476d20(selection), 16);
        COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x00, name, 16);
        g_UnknownGlobal56e26c->mode.UnknownFunction5231f0();
        return 1;
    }
    return 0;
}

// 0x0044b1f0
void UnknownFunction44b1f0(UIDialog* dialog) {
    dialog->UnknownFunction46ff30(0);
}

// 0x0044c6b0
int MainDlg::UnknownVirtualSlot10(float frameTime) {
    if (field_0x7f6c) {
        UnknownFunction46ebf0("ScreenOverCtl", 0)->UnknownVirtualSlot50();
        if (field_0x7f6c && !field_0x7f60->field_0x68 && field_0x7f78) {
            if (field_0x7f7c < 4.0f) {
                field_0x7f7c = g_UnknownGlobal56e26c->field_0x2f0 + field_0x7f7c;
            } else {
                field_0x7f78 = 0;
                if (field_0x7f58) {
                    field_0x7f58->field_0x1ec->UnknownFunction4a2900();
                    field_0x7f58->field_0x1ec->UnknownVirtualSlot16(field_0x7f78);
                    field_0x7f58->UnknownFunction470660(1, 1);
                }
            }
        }
    }
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x0044c7a0
void MainDlg::UnknownFunction44c7a0() {
    char name[260];
    UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstProfiles", 0));
    list->UnknownFunction4775f0();
    g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a1d0("ui\\profile");
    g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a220("*", 0);
    g_UnknownGlobal56e26c->profileDirectory->UnknownVirtualSlot1();
    g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a550(name);
    if (name[0])
        list->UnknownFunction476d80(name, 0, 0);
    while (g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a4c0(name)) {
        if (name[0])
            list->UnknownFunction476d80(name, 0, 0);
    }
    list->UnknownFunction477900(1);
    list->UnknownFunction476c70(0xfeb97a, -1);
    list->UnknownFunction476ad0(g_UnknownGlobal56e26c->mode.field_0x00);
    list->UnknownFunction476b80(0xffffff);
    list->UnknownFunction476cd0(0xfeb97a);
}

// 0x0044c8c0
void MainDlg::UnknownFunction44c8c0() {
    char label[128];
    char text[128];
    UnknownGameUiControl* control = UnknownFunction46ebf0("TxtCurrentProfile", 12);
    g_UnknownGlobal56e26c->UnknownFunction521970(0x1466, label, 128);
    sprintf(text, "%s %s", label, g_UnknownGlobal56e26c->mode.field_0x00);
    control->UnknownFunction470b20(text);
}

// 0x0044c930
void UnknownFunction44c930(UnknownDialogEvent* event, int a, int b, int c, int d) {
    UnknownGameUiControl* button;
    if (a) {
        button = event->field_0x0c->UnknownFunction46ebf0("ButStats1", 0);
        button->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, a);
    }
    if (b) {
        button = event->field_0x0c->UnknownFunction46ebf0("ButStats2", 0);
        button->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, b);
    }
    if (c) {
        button = event->field_0x0c->UnknownFunction46ebf0("ButStats3", 0);
        button->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, c);
    }
    if (d) {
        button = event->field_0x0c->UnknownFunction46ebf0("ButStats4", 0);
        button->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, d);
    }
}

// 0x0044c9f0
void UnknownFunction44c9f0(UnknownDialogEvent* event, const char* a, const char* b, const char* c,
                           const char* d) {
    UnknownGameUiControl* button;
    if (a) {
        button = event->field_0x0c->UnknownFunction46ebf0("ButStats1", 0);
        button->UnknownFunction470b20(a);
    }
    if (b) {
        button = event->field_0x0c->UnknownFunction46ebf0("ButStats2", 0);
        button->UnknownFunction470b20(b);
    }
    if (c) {
        button = event->field_0x0c->UnknownFunction46ebf0("ButStats3", 0);
        button->UnknownFunction470b20(c);
    }
    if (d) {
        button = event->field_0x0c->UnknownFunction46ebf0("ButStats4", 0);
        button->UnknownFunction470b20(d);
    }
}

// 0x0044ca80
void UnknownFunction44ca80(UnknownDialogEvent* event) {
    static_cast<UIListBox*>(event->field_0x0c->UnknownFunction46ebf0("LstStats1", 0))->UnknownFunction4775f0();
    static_cast<UIListBox*>(event->field_0x0c->UnknownFunction46ebf0("LstStats2", 0))->UnknownFunction4775f0();
    static_cast<UIListBox*>(event->field_0x0c->UnknownFunction46ebf0("LstStats3", 0))->UnknownFunction4775f0();
    static_cast<UIListBox*>(event->field_0x0c->UnknownFunction46ebf0("LstStats4", 0))->UnknownFunction4775f0();
}

// 0x0044cae0
void UnknownFunction44cae0(UnknownDialogEvent* event, const char* a, const char* b, const char* c,
                           const char* d) {
    UIListBox* list;
    if (a) {
        list = static_cast<UIListBox*>(event->field_0x0c->UnknownFunction46ebf0("LstStats1", 0));
        list->UnknownFunction476d80(a, 0, 0);
    }
    if (b) {
        list = static_cast<UIListBox*>(event->field_0x0c->UnknownFunction46ebf0("LstStats2", 0));
        list->UnknownFunction476d80(b, 0, 0);
    }
    if (c) {
        list = static_cast<UIListBox*>(event->field_0x0c->UnknownFunction46ebf0("LstStats3", 0));
        list->UnknownFunction476d80(c, 0, 0);
    }
    if (d) {
        list = static_cast<UIListBox*>(event->field_0x0c->UnknownFunction46ebf0("LstStats4", 0));
        list->UnknownFunction476d80(d, 0, 0);
    }
}

// 0x0044cb80
void ChooseTCPMethodDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("ButLeft", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e7);
        control = UnknownFunction46ebf0("ButMiddle", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e6);
        control = UnknownFunction46ebf0("ButRight", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13dc);
        control = UnknownFunction46ebf0("TitleText", 12);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e8);
        UnknownGameUiControl* prompt = UnknownFunction46ebf0("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13fe);
        break;
    }
    case 1:
        if (!_stricmp("ButLeft", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0x3c);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButMiddle", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0x3d);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButRight", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        }
        break;
    }
}

// 0x0044ccf0
void NewbieDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("ButLeft", 1);
        control->UnknownFunction470660(0, 1);
        control->field_0x1d8 = 0;
        UnknownFunction46ebf0("ButRight", 1)->UnknownFunction470660(0, 1);
        control = UnknownFunction46ebf0("ButMiddle", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e9);
        control->field_0x1d8 = 0x1c;
        control = UnknownFunction46ebf0("TitleText", 12);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13ea);
        UnknownGameUiControl* prompt = UnknownFunction46ebf0("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x1400);
        break;
    }
    case 1:
        if (!_stricmp("ButMiddle", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0x33);
            event->field_0x20 = 1;
        }
        break;
    }
}

// 0x0044ce10
void NoDelCurProfileDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("ButLeft", 1);
        control->UnknownFunction470660(0, 1);
        control->field_0x1d8 = 0;
        UnknownFunction46ebf0("ButRight", 1)->UnknownFunction470660(0, 1);
        control = UnknownFunction46ebf0("ButMiddle", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e9);
        control->field_0x1d8 = 0x1c;
        control = UnknownFunction46ebf0("TitleText", 12);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x146d);
        UnknownGameUiControl* prompt = UnknownFunction46ebf0("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x146e);
        break;
    }
    case 1:
        if (!_stricmp("ButMiddle", event->field_0x04))
            UnknownFunction46ff30(0);
        break;
    }
}

// 0x0044cf20
void SinglePlayerDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 5: {
        field_0x7f58 = 0;
        field_0x7f5c = 0;
        field_0x7f60 = 0;
        UIRadioButton* tab = static_cast<UIRadioButton*>(UnknownFunction46ebf0("EventTab", 4));
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x13e3);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x13e3);
        tab->UnknownFunction470da0(0x12);
        tab->UnknownFunction479310(0);
        tab = static_cast<UIRadioButton*>(UnknownFunction46ebf0("BikeRiderTab", 4));
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x13e4);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x13e4);
        tab->UnknownFunction470da0(0x12);
        tab = static_cast<UIRadioButton*>(UnknownFunction46ebf0("RaceInfoTab", 4));
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x13e5);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x13e5);
        tab->UnknownFunction470da0(0x12);
        memcpy(&g_UnknownGlobal56e26c->mode.field_0x27f8, &g_UnknownGlobal56e26c->mode.field_0x29e4,
               sizeof(g_UnknownGlobal56e26c->mode.field_0x29e4));
        memcpy(g_UnknownGlobal56e26c->mode.field_0xfd8, g_UnknownGlobal56e26c->mode.field_0x1034,
               sizeof(g_UnknownGlobal56e26c->mode.field_0x1034));
        memcpy(&g_UnknownGlobal56e26c->mode.field_0x1974, &g_UnknownGlobal56e26c->mode.field_0x1a3c,
               sizeof(g_UnknownGlobal56e26c->mode.field_0x1a3c));
        g_UnknownGlobal56e26c->mode.field_0x94 = g_UnknownGlobal56e26c->mode.field_0x98;
        g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04);
        if (field_0xc4 == 0x88e) {
            static_cast<UIRadioButton*>(UnknownFunction46ebf0("RaceInfoTab", 4))->UnknownFunction479310(0);
            UnknownFunction44d740(2);
        } else {
            static_cast<UIRadioButton*>(UnknownFunction46ebf0("EventTab", 4))->UnknownFunction479310(0);
            UnknownFunction44d740(0);
        }
        break;
    }
    case 1:
        if (!_stricmp("Back", event->field_0x04)) {
            if (field_0x7f58)
                field_0x7f58->UnknownFunction46ecc0(1);
            if (field_0x7f5c)
                field_0x7f5c->UnknownFunction46ecc0(1);
            UnknownFunction46ecc0(1);
            memcpy(&g_UnknownGlobal56e26c->mode.field_0x29e4, &g_UnknownGlobal56e26c->mode.field_0x27f8,
                   sizeof(g_UnknownGlobal56e26c->mode.field_0x29e4));
            memcpy(g_UnknownGlobal56e26c->mode.field_0x1034, g_UnknownGlobal56e26c->mode.field_0xfd8,
                   sizeof(g_UnknownGlobal56e26c->mode.field_0x1034));
            memcpy(&g_UnknownGlobal56e26c->mode.field_0x1a3c, &g_UnknownGlobal56e26c->mode.field_0x1974,
                   sizeof(g_UnknownGlobal56e26c->mode.field_0x1a3c));
            g_UnknownGlobal56e26c->mode.field_0x98 = g_UnknownGlobal56e26c->mode.field_0x94;
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(100);
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (!_stricmp("Start", event->field_0x04)) {
            if (field_0x7f58)
                field_0x7f58->UnknownFunction46ecc0(1);
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x08 == -1) {
                ChoiceDlg* dialog = new(__FILE__, 1274) ChoiceDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0,
                                                  (UnknownGuiDialog*)this, 0, 0, 1);
                char text[1024];
                strcpy(text, "You must choose one of the following trial version tracks:\n"
                             "Stunt Event: Donner Pass, or\n"
                             "Nationals Race: A Voodoo Basin\n");
                dialog->UnknownFunction455700("Trial Version", 0, text, 0, 0, 0, 0, 0x13e9, 0, 0);
                dialog->UnknownFunction46ebf0("TxtPrompt", 0)->field_0x1e8 = 1;
            } else if (g_UnknownGlobal56e26c->mode.field_0x9c) {
                NewbieDlg* dialog = new(__FILE__, 1292) NewbieDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0,
                                                  (UnknownGuiDialog*)this, 0, 0, 1);
            } else {
                g_UnknownGlobal56e26c->ui->UnknownFunction499a20();
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = 0;
                if (field_0x7f58)
                    field_0x7f58->UnknownFunction46ecc0(1);
                if (field_0x7f5c)
                    field_0x7f5c->UnknownFunction46ecc0(1);
                if (field_0x7f60)
                    field_0x7f60->UnknownFunction46ecc0(1);
                if (g_UnknownGlobal56e26c->mode.field_0x2dbc && g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 4) {
                    g_UnknownGlobal56e26c->field_0x342c = 1;
                    g_UnknownGlobal56e26c->field_0x3428 = 0;
                } else {
                    g_UnknownGlobal56e26c->field_0x342c = 0;
                    g_UnknownGlobal56e26c->field_0x3428 = 0;
                }
                memcpy(&g_UnknownGlobal56e26c->mode.field_0x29e4, &g_UnknownGlobal56e26c->mode.field_0x27f8,
                       sizeof(g_UnknownGlobal56e26c->mode.field_0x29e4));
                memcpy(g_UnknownGlobal56e26c->mode.field_0x1034, g_UnknownGlobal56e26c->mode.field_0xfd8,
                       sizeof(g_UnknownGlobal56e26c->mode.field_0x1034));
                memcpy(&g_UnknownGlobal56e26c->mode.field_0x1a3c, &g_UnknownGlobal56e26c->mode.field_0x1974,
                       sizeof(g_UnknownGlobal56e26c->mode.field_0x1a3c));
                g_UnknownGlobal56e26c->mode.field_0x98 = g_UnknownGlobal56e26c->mode.field_0x94;
                if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 4 && g_UnknownGlobal56e26c->mode.field_0x25ec[0]) {
                    // KrustyUI+0x4e8 holds a copy of the race settings.
                    memcpy(&g_UnknownGlobal56e26c->mode.field_0x27f8, (unsigned char*)g_UnknownGlobal56e26c->ui + 0x4e8,
                           sizeof(g_UnknownGlobal56e26c->mode.field_0x29e4));
                } else {
                    g_UnknownGlobal56e26c->mode.field_0x25ec[0] = 0;
                }
                UnknownFunction4536e0();
                UnknownVirtualSlot26();
            }
        } else if (!_stricmp("EventTab", event->field_0x04)) {
            UnknownFunction44d740(0);
        } else if (!_stricmp("BikeRiderTab", event->field_0x04)) {
            UnknownFunction44d740(1);
        } else if (!_stricmp("RaceInfoTab", event->field_0x04)) {
            UnknownFunction44d740(2);
        } else if (!_stricmp("Options", event->field_0x04)) {
            OptionsDlg* dialog = new(__FILE__, 1347) OptionsDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0,
                                              (UnknownGuiDialog*)this, 0, 0, 1);
        } else if (!_stricmp("Joystick", event->field_0x04)) {
            OptionsDlg* dialog = new(__FILE__, 1350) OptionsDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0,
                                              (UnknownGuiDialog*)this, 1, 0, 1);
        } else if (!_stricmp("Help", event->field_0x04)) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523d30("MCM2HELP", 0);
        }
        break;
    case 9: {
        int code = event->field_0x00;
        if (code == 0x33) {
            static_cast<UIRadioButton*>(UnknownFunction46ebf0("BikeRiderTab", 4))->UnknownFunction479310(0);
            UnknownFunction44d740(1);
        } else if (code == 0x51) {
            if (field_0x7f5c) {
                field_0x7f5c->UnknownFunction46ecc0(0);
                field_0x7f5c->UnknownFunction44f750();
                field_0x7f5c->UnknownFunction4500d0();
                field_0x7f5c->UnknownFunction4500e0();
            }
        } else if (code == 0x66) {
            static_cast<UIRadioButton*>(UnknownFunction46ebf0("EventTab", 4))->UnknownFunction479310(0);
            UnknownFunction44d740(0);
        }
        break;
    }
    case 6:
        if (field_0x2c)
            field_0x2c->UnknownFunction46ea60(1);
        break;
    }
}

// 0x0044d740
void SinglePlayerDlg::UnknownFunction44d740(int page) {
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
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f58, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 1:
        if (!field_0x7f5c) {
            field_0x7f5c = new(__FILE__, 1407) SPBikeRiderDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f5c, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 2:
        if (!field_0x7f60) {
            field_0x7f60 = new(__FILE__, 1413) SPRaceInfoDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f60, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    }
}

// 0x0044d950
void SPEventDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[128];
    switch (event->field_0x08) {
    case 5: {
        UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00;
        UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("EventTypeDropDown", 6))->field_0x1fc;
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13ed, text, 0x80);
        list->UnknownFunction476d80(text, 1, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13ee, text, 0x80);
        list->UnknownFunction476d80(text, 0, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f0, text, 0x80);
        list->UnknownFunction476d80(text, 5, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13ef, text, 0x80);
        list->UnknownFunction476d80(text, 2, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f2, text, 0x80);
        list->UnknownFunction476d80(text, 3, 0);
        list->UnknownFunction476b30(settings->field_0x04);
        list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("RaceModeDropDown", 6))->field_0x1fc;
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f3, text, 0x80);
        list->UnknownFunction476d80(text, 0, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f4, text, 0x80);
        list->UnknownFunction476d80(text, 1, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f5, text, 0x80);
        list->UnknownFunction476d80(text, 2, 1);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f6, text, 0x80);
        list->UnknownFunction476d80(text, 4, 1);
        list->UnknownFunction476a60(settings->field_0x00);
        UnknownFunction46ebf0("OpponentsListBox", 0)->UnknownVirtualSlot54(&settings->field_0x24);
        UnknownFunction46ebf0("ChkRecordRace", 0)->UnknownFunction470660(0, 1);
        UnknownFunction46ebf0("RadLODEasy", 0)->UnknownVirtualSlot54(&g_UnknownGlobal56e26c->mode.field_0x94);
        UIListBox* gates = static_cast<UIListBox*>(UnknownFunction46ebf0("LstNumGates", 3));
        gates->UnknownFunction4775f0();
        for (int i = 10; i <= 30; i++) {
            _itoa(i, text, 10);
            gates->UnknownFunction476d80(text, i, 0);
        }
        static_cast<UIEditBox*>(UnknownFunction46ebf0("EditSeed", 0xb))->UnknownFunction473f30("0123456789");
        UnknownFunction46ecc0(0);
        UnknownFunction44e3e0();
        UnknownFunction44dfc0();
        field_0x7f5c = 0;
        field_0x7f58 = 0;
        field_0x7f64 = 0;
        field_0x7f60 = 0;
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 4)
            UnknownFunction44eb40();
        break;
    }
    case 1:
        if (!_stricmp("ChkRandomGates", event->field_0x04))
            UnknownFunction44dfc0();
        if (!_stricmp("TrackLeft", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLTextBox", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("TrackRight", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLTextBox", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("GhostLeft", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLGhostRaces", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("GhostRight", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLGhostRaces", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("SeedLeft", event->field_0x04)) {
            char seed[32];
            UIEditBox* edit = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditSeed", 0xb));
            edit->UnknownFunction473ef0(seed, 32);
            int value = atoi(seed) - 1;
            if (value < 0)
                value = 999;
            _itoa(value, seed, 10);
            edit->UnknownFunction473da0(seed);
            field_0x34->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
        } else if (!_stricmp("SeedRight", event->field_0x04)) {
            char seed[32];
            UIEditBox* edit = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditSeed", 0xb));
            edit->UnknownFunction473ef0(seed, 32);
            int value = atoi(seed) + 1;
            if (value > 999)
                value = 0;
            _itoa(value, seed, 10);
            edit->UnknownFunction473da0(seed);
            field_0x34->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
        } else if (!_stricmp("OpponentsLeftArrow", event->field_0x04) ||
                   !_stricmp("OpponentsRightArrow", event->field_0x04)) {
            UnknownFunction44dfc0();
        }
        break;
    case 2:
        if (!_stricmp("RaceModeDropDown", event->field_0x04)) {
            if (static_cast<UIListBox*>(event->field_0x14)->UnknownFunction4768d0(-1) == 4)
                UnknownFunction44eb40();
            if (static_cast<UIListBox*>(event->field_0x14)->UnknownFunction4768d0(-1) != 2) {
                g_UnknownGlobal56e26c->mode.field_0x10ec = 0;
                g_UnknownGlobal56e26c->eventManager->field_0x48 = 0;
            }
            UnknownFunction44dfc0();
        } else if (!_stricmp("EventTypeDropDown", event->field_0x04)) {
            UnknownFunction46ecc0(1);
            UnknownFunction44e3e0();
            UnknownFunction44dfc0();
            UnknownFunction44eb40();
        } else if (!_stricmp("DDLTextBox", event->field_0x04)) {
            UnknownFunction44eb40();
            strcpy(g_UnknownGlobal56e26c->mode.field_0x6f4[g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04],
                   static_cast<UIListBox*>(event->field_0x14)->UnknownFunction476d20(-1));
        }
        break;
    }
}

// 0x0044dfc0: shows the controls that apply to the chosen race mode.
void SPEventDlg::UnknownFunction44dfc0() {
    char text[32];
    UnknownGameUiControl* record = UnknownFunction46ebf0("ChkRecordRace", 0);
    UIListBox* modes = static_cast<UIDropDownList*>(UnknownFunction46ebf0("RaceModeDropDown", 6))->field_0x1fc;
    int mode = modes->UnknownFunction4768d0(-1);
    UnknownGameUiControl* lapsBox = UnknownFunction46ebf0("LapsControlBox", 5);
    UnknownGameUiControl* racesBox = UnknownFunction46ebf0("RacesControlBox", 5);
    UnknownGameUiControl* ghostBox = UnknownFunction46ebf0("GhostBox", 5);
    UnknownGameUiControl* opponentsBox = UnknownFunction46ebf0("OpponentsControlBox", 5);
    opponentsBox->UnknownFunction470660(1, 1);
    record->UnknownFunction470660(0, 1);
    ghostBox->UnknownFunction470660(0, 1);
    UnknownFunction46eb30(0x3f3, 1);
    switch (mode) {
    case 0:
        lapsBox->UnknownFunction470660(0, 1);
        racesBox->UnknownFunction470660(0, 1);
        break;
    case 1:
        lapsBox->UnknownFunction470660(1, 1);
        racesBox->UnknownFunction470660(0, 1);
        break;
    case 2:
        lapsBox->UnknownFunction470660(1, 1);
        racesBox->UnknownFunction470660(1, 1);
        break;
    case 4:
        opponentsBox->UnknownFunction470660(0, 1);
        lapsBox->UnknownFunction470660(0, 1);
        racesBox->UnknownFunction470660(0, 1);
        record->UnknownFunction470660(0, 1);
        ghostBox->UnknownFunction470660(1, 1);
        UnknownFunction46eb30(0x3f3, 0);
        break;
    }
    UnknownGameUiControl* waypoints = UnknownFunction46ebf0("WaypointBox", 5);
    UnknownTrackGameModeSettings& settings = g_UnknownGlobal56e26c->mode.field_0x27f8;
    if (settings.field_0x04 == 1 && modes->UnknownFunction4768d0(-1) != 4) {
        waypoints->UnknownFunction470660(1, 1);
        if (static_cast<UIMultiState*>(UnknownFunction46ebf0("ChkRandomGates", 2))->UnknownFunction4755c0()) {
            UnknownFunction46ea80(0x6f, 1);
            UnknownFunction46ebf0("LstNumGates", 0)->UnknownFunction470660(1, 1);
            UnknownFunction46ebf0("EditSeed", 0)->UnknownFunction470660(1, 1);
        } else {
            UnknownFunction46ea80(0x6f, 0);
            UnknownFunction46ebf0("LstNumGates", 0)->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("EditSeed", 0)->UnknownFunction470660(0, 1);
        }
    } else {
        waypoints->UnknownFunction470660(0, 1);
    }
    UnknownGameUiControl* racesLeft = UnknownFunction46ebf0("RacesLeftArrow", 9);
    UnknownGameUiControl* racesRight = UnknownFunction46ebf0("RacesRightArrow", 0xa);
    UnknownGameUiControl* opponentsLeft = UnknownFunction46ebf0("OpponentsLeftArrow", 0);
    UnknownGameUiControl* opponentsRight = UnknownFunction46ebf0("OpponentsRightArrow", 0);
    UnknownGameUiControl* racesLabel = UnknownFunction46ebf0("RacesLabel", 0xc);
    UIListBox* races = static_cast<UIListBox*>(UnknownFunction46ebf0("RacesListBox", 3));
    UnknownGameUiControl* opponents = UnknownFunction46ebf0("OpponentsListBox", 3);
    races->UnknownFunction4775f0();
    for (int i = 3; i <= 7; i += 2) {
        sprintf(text, "%d", i);
        races->UnknownFunction476d80(text, i - 1, 0);
    }
    races->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c);
    if (settings.field_0x00 == 2 && g_UnknownGlobal56e26c->mode.field_0x10ec) {
        opponentsLeft->UnknownFunction470660(0, 1);
        opponentsRight->UnknownFunction470660(0, 1);
        opponents->UnknownVirtualSlot49(0);
        racesLeft->UnknownFunction470660(0, 1);
        racesRight->UnknownFunction470660(0, 1);
        racesLabel->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x92b);
        sprintf(text, "%d/%d", g_UnknownGlobal56e26c->eventManager->field_0x48 + 1,
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c + 1);
        races->UnknownFunction4775f0();
        races->UnknownFunction476d80(text, 0, 0);
    } else {
        opponentsLeft->UnknownFunction470660(1, 1);
        opponentsRight->UnknownFunction470660(1, 1);
        opponents->UnknownVirtualSlot49(1);
        racesLeft->UnknownFunction470660(1, 1);
        racesRight->UnknownFunction470660(1, 1);
        racesLabel->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x92a);
    }
}

// 0x0044e3e0: fills the race mode, opponent and lap lists for the chosen
// event type and lists its tracks.
void SPEventDlg::UnknownFunction44e3e0() {
    char label[64];
    char text[256];
    UIListBox* types = static_cast<UIDropDownList*>(UnknownFunction46ebf0("EventTypeDropDown", 6))->field_0x1fc;
    int type = types->UnknownFunction4768d0(-1);
    UnknownTrackGameModeSettings& settings = g_UnknownGlobal56e26c->mode.field_0x27f8;
    settings.field_0x04 = type;
    g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(type);
    UIListBox* modes = static_cast<UIDropDownList*>(UnknownFunction46ebf0("RaceModeDropDown", 6))->field_0x1fc;
    int row = ((UIListBox*)modes)->UnknownFunction476900(4);
    if (type == 0) {
        if (row != -1) {
            if (modes->UnknownFunction476950() == row) {
                modes->UnknownFunction476b30(0);
                settings.field_0x00 = 0;
            }
            modes->UnknownFunction477490(row);
        }
    } else if (row == -1) {
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f6, label, 128);
        modes->UnknownFunction476d80(label, 4, 1);
    }
    UnknownGameUiControl* lapsLabel = UnknownFunction46ebf0("LapsLabel", 0xc);
    UIListBox* opponents = static_cast<UIListBox*>(UnknownFunction46ebf0("OpponentsListBox", 3));
    UIListBox* laps = static_cast<UIListBox*>(UnknownFunction46ebf0("LapsListBox", 3));
    UnknownFunction46ebf0("RacesListBox", 3);
    lapsLabel->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x929);
    opponents->UnknownFunction4775f0();
    int i;
    for (i = 0; i <= 4; i++) {
        sprintf(text, "%d", i);
        opponents->UnknownFunction476d80(text, i, 0);
    }
    laps->UnknownFunction4775f0();
    for (i = type == 2 || type == 3 ? 2 : 1; i <= 5; i++) {
        sprintf(text, "%d", i);
        laps->UnknownFunction476d80(text, i, 0);
    }
    laps->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20);
    if (type == 0) {
        lapsLabel->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x92f);
        laps->UnknownFunction4775f0();
        for (i = 2; i <= 5; i++) {
            sprintf(text, "%dm", i);
            laps->UnknownFunction476d80(text, i, 0);
        }
        laps->UnknownFunction476b30((int)g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140);
    }
    if (settings.field_0x04 == 0) {
        g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(0);
        UnknownFunction4f3080(1, "PictureBox", "DDLTextBox", 0, this, 0);
        g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(5);
        UnknownFunction4f3080(1, "PictureBox", "DDLTextBox", 0, this, 1);
    } else {
        UnknownFunction4f3080(1, "PictureBox", "DDLTextBox", 0, this, 0);
    }
    UnknownFunction46ecc0(0);
    g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(type);
    UIListBox* tracks = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLTextBox", 6))->field_0x1fc;
    tracks->UnknownFunction476ad0(g_UnknownGlobal56e26c->mode.field_0x6f4[type]);
}

// 0x0044e6c0
void SPEventDlg::UnknownVirtualSlot31(int apply) {
    char seed[128];
    UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00;
    if (apply) {
        if (settings->field_0x04 != 0 && settings->field_0x04 != 4)
            settings->field_0x20 = static_cast<UIListBox*>(UnknownFunction46ebf0("LapsListBox", 3))->UnknownFunction4768d0(-1);
        else
            settings->field_0x140 = (float)static_cast<UIListBox*>(UnknownFunction46ebf0("LapsListBox", 3))->UnknownFunction4768d0(-1);
        if (!g_UnknownGlobal56e26c->mode.field_0x10ec)
            settings->field_0x0c = static_cast<UIListBox*>(UnknownFunction46ebf0("RacesListBox", 3))->UnknownFunction4768d0(-1);
        settings->field_0x10 = static_cast<UIMultiState*>(UnknownFunction46ebf0("ChkTreeCollision", 2))->UnknownFunction4755c0();
        strcpy(settings->field_0x36, "");
        UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLTextBox", 6))->field_0x1fc;
        int index = list->UnknownFunction4768d0(-1);
        {
            char* name = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x14;
            int length = strlen(name);
            int count = length > 0xff ? 0xff : length;
            strncpy(settings->field_0x36, name, count);
            settings->field_0x36[count] = 0;
        }
        g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x08);
        settings->field_0x34 = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x04;
        strcpy(g_UnknownGlobal56e26c->mode.field_0x6f4[g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04], list->UnknownFunction476d20(-1));
        g_UnknownGlobal56e26c->mode.field_0x9f4[g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04] = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34;
        if (static_cast<UIMultiState*>(UnknownFunction46ebf0("ChkRandomGates", 2))->UnknownFunction4755c0())
            settings->field_0x2c = 1;
        else
            settings->field_0x2c = 0;
        settings->field_0x2d = static_cast<UIListBox*>(UnknownFunction46ebf0("LstNumGates", 3))->UnknownFunction4768d0(-1);
        static_cast<UIEditBox*>(UnknownFunction46ebf0("EditSeed", 0xb))->UnknownFunction473ef0(seed, 128);
        settings->field_0x30 = atoi(seed);
        list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("EventTypeDropDown", 6))->field_0x1fc;
        settings->field_0x04 = list->UnknownFunction4768d0(-1);
        list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("RaceModeDropDown", 6))->field_0x1fc;
        settings->field_0x00 = list->UnknownFunction4768d0(-1);
        list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLGhostRaces", 6))->field_0x1fc;
        int ghost = list->UnknownFunction4768d0(-1);
        if (settings->field_0x00 == 4 && ghost != -1) {
            COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x25ec, field_0x7f60[ghost], 0x104);
            // KrustyUI+0x4b4 holds the header of the ghost raced against.
            *(UnknownRecordFileHeader*)((unsigned char*)g_UnknownGlobal56e26c->ui + 0x4b4) = field_0x7f58[ghost];
        } else {
            g_UnknownGlobal56e26c->mode.field_0x25ec[0] = 0;
        }
    } else {
        static_cast<UIListBox*>(UnknownFunction46ebf0("RacesListBox", 3))->UnknownFunction476b30(settings->field_0x0c);
        static_cast<UIMultiState*>(UnknownFunction46ebf0("ChkRandomGates", 2))->UnknownFunction478cf0(settings->field_0x2c);
        static_cast<UIListBox*>(UnknownFunction46ebf0("LstNumGates", 3))->UnknownFunction476b30(settings->field_0x2d);
        _itoa(settings->field_0x30, seed, 10);
        static_cast<UIEditBox*>(UnknownFunction46ebf0("EditSeed", 0xb))->UnknownFunction473da0(seed);
        UIListBox* laps = static_cast<UIListBox*>(UnknownFunction46ebf0("LapsListBox", 3));
        if (settings->field_0x04 != 0 && settings->field_0x04 != 4)
            laps->UnknownFunction476b30(settings->field_0x20);
        else
            laps->UnknownFunction476b30((int)settings->field_0x140);
        static_cast<UIMultiState*>(UnknownFunction46ebf0("ChkTreeCollision", 2))->UnknownFunction478cf0(settings->field_0x10);
        UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("EventTypeDropDown", 6))->field_0x1fc;
        list->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04);
        list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("RaceModeDropDown", 6))->field_0x1fc;
        list->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00);
    }
}

// 0x0044eb40
void SPEventDlg::UnknownFunction44eb40() {
    char path[260];
    char label[128];
    WIN32_FIND_DATAA data;
    UnknownFunction44eed0();
    UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLGhostRaces", 6))->field_0x1fc;
    list->UnknownFunction4775f0();
    g_UnknownGlobal56e26c->UnknownFunction521970(0x14c7, label, 128);
    list->UnknownFunction476d80(label, -1, 0);
    list->UnknownFunction476a60(0);
    HANDLE find = FindFirstFileA("Record\\*.gho", &data);
    if (find != INVALID_HANDLE_VALUE) {
        sprintf(path, "%s\\%s", "Record", data.cFileName);
        UnknownFunction44ec50(path);
        while (FindNextFileA(find, &data)) {
            sprintf(path, "%s\\%s", "Record", data.cFileName);
            UnknownFunction44ec50(path);
        }
        FindClose(find);
    }
}

// 0x0044ec50
int SPEventDlg::UnknownFunction44ec50(const char* path) {
    char label[128];
    char text[128];
    int added = 0;
    UnknownRecordFileHeader header;
    FILE* file = fopen(path, "rb");
    if (file) {
        if (fread(&header, sizeof(header), 1, file)) {
            UIListBox* tracks = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLTextBox", 6))->field_0x1fc;
            int row = tracks->UnknownFunction4768d0(-1);
            if (!_stricmp(header.field_0x6a, g_UnknownGlobal56e26c->ui->field_0x60[row].field_0x14) &&
                header.field_0x30 > 0.0f &&
                g_UnknownGlobal56e26c->ui->field_0x60[row].field_0x04 == header.field_0x68) {
                field_0x7f58 = (UnknownRecordFileHeader*)UnknownFunction47b570(
                    field_0x7f58, (field_0x7f5c + 1) * sizeof(UnknownRecordFileHeader));
                field_0x7f58[field_0x7f5c] = header;
                field_0x7f5c++;
                field_0x7f60 = (char**)UnknownFunction47b570(field_0x7f60, (field_0x7f64 + 1) * sizeof(char*));
                field_0x7f60[field_0x7f64] = (char*)DebugMalloc(strlen(path) + 1, __FILE__, 1986);
                strcpy(field_0x7f60[field_0x7f64], path);
                field_0x7f64++;
                UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLGhostRaces", 6))->field_0x1fc;
                g_UnknownGlobal56e26c->UnknownFunction521970(0x14c8, label, 128);
                sprintf(text, "%s: %s", label, header.field_0x10);
                list->UnknownFunction476d80(text, field_0x7f5c - 1, 0);
                added = 1;
            }
        }
        fclose(file);
    }
    return added;
}

// 0x0044eed0
void SPEventDlg::UnknownFunction44eed0() {
    if (field_0x7f58)
        DebugFree(field_0x7f58, __FILE__, 2009);
    if (field_0x7f60) {
        for (int i = 0; i < field_0x7f64; i++)
            DebugFree(field_0x7f60[i], __FILE__, 2013);
        DebugFree(field_0x7f60, __FILE__, 2015);
    }
    field_0x7f58 = 0;
    field_0x7f60 = 0;
    field_0x7f64 = 0;
    field_0x7f5c = 0;
}

// 0x0044ef70
void SPBikeRiderDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char plate[12];
    char typed[12];
    switch (event->field_0x08) {
    case 5: {
        field_0x7f78 = 0;
        static_cast<UIMultiState*>(UnknownFunction46ebf0("ChkAutoRotate", 2))->UnknownFunction478cf0(1);
        g_UnknownGlobal56e26c->mode.field_0x9c = 0;
        field_0x7f88 = 1;
        field_0x7f84 = 1;
        UnknownFunction44f750();
        UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLEngineSize", 6))->field_0x1fc;
        list->UnknownFunction4775f0();
        list->UnknownFunction476d80("125cc 2-stroke", UnknownBikeClassOf(125), 0);
        list->UnknownFunction476d80("250cc 2-stroke", UnknownBikeClassOf(250), 0);
        list->UnknownFunction476d80("400cc 4-stroke", UnknownBikeClassOf(400), 0);
        list->UnknownFunction476d80("500cc 2-stroke", UnknownBikeClassOf(500), 0);
        list->UnknownFunction476d80("600cc 4-stroke", UnknownBikeClassOf(600), 0);
        list->UnknownFunction476b30(UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->field_0x00));
        UIEditBox* edit = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditPlateNumber", 0xb));
        edit->UnknownFunction473f30("0123456789");
        g_UnknownGlobal56e26c->ui->UnknownFunction4999f0(this);
        Vector3* eye = &field_0x7f58;
        field_0x7f8c.left = 0x5b;
        field_0x7f8c.right = 0x21f;
        field_0x7f8c.top = 0x6c;
        field_0x7f8c.bottom = 0x148;
        *eye = kVec3Zero;
        float fov = 50.0f;
        Vector3* target = &field_0x7f64;
        field_0x7f58.z = 15.0f;
        field_0x7f58.y = 1.0f;
        *target = g_UnknownGlobal56e26c->ui->field_0x474;
        field_0x7f64.y += 3.0f;
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownFunction42e9b0(eye, 0, 0, 0, (int)&fov);
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownVirtualSlot29(*target);
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownFunction42f190(
            field_0x7f8c.left, field_0x7f8c.top, field_0x7f8c.right - field_0x7f8c.left,
            field_0x7f8c.bottom - field_0x7f8c.top);
        field_0x7f70 = UnknownVectorLength(UnknownVectorDifference(*eye, g_UnknownGlobal56e26c->ui->field_0x474));
        UnknownFunction4500d0();
        UnknownFunction4500e0();
        UnknownFunction44f950(g_UnknownGlobal56e26c->mode.field_0x1bcc);
        srand(ReadClock());
        if (field_0x110)
            field_0x7f74 = field_0x110->UnknownFunction4040f0(1);
        break;
    }
    case 2:
        if (!_stricmp("DDLBikes", event->field_0x04))
            UnknownFunction4500e0();
        else if (!_stricmp("DDLRiders", event->field_0x04))
            UnknownFunction4500d0();
        else if (!_stricmp("DDLEngineSize", event->field_0x04))
            UnknownFunction4500e0();
        break;
    case 1:
        if (!_stricmp("BikeLeft", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("BikeRight", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RiderLeft", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLRiders", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RiderRight", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLRiders", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("ChkAutoRotate", event->field_0x04)) {
            field_0x7f78 = !static_cast<UIMultiState*>(UnknownFunction46ebf0("ChkAutoRotate", 2))->UnknownFunction4755c0();
        } else if (!_stricmp("ButWrench", event->field_0x04)) {
            OptionsDlg* dialog = new(__FILE__, 2135) OptionsDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, field_0x2c, 2, 0, 1);
        }
        break;
    case 10:
        if (!_stricmp("EditPlateNumber", event->field_0x04)) {
            static_cast<UIEditBox*>(event->field_0x14)->UnknownFunction473ef0(plate, 9);
            int number = atoi(plate);
            if (number < 100)
                number += 100;
            if (number >= 101) {
                if (number > 999)
                    number = 999;
            } else {
                number = 101;
            }
            g_UnknownGlobal56e26c->mode.field_0x1bcc = number;
            _itoa(number, plate, 10);
            static_cast<UIEditBox*>(event->field_0x14)->UnknownFunction473da0(plate);
            UnknownFunction44f950(number);
        }
        break;
    case 19:
        if (!_stricmp("EditPlateNumber", event->field_0x04)) {
            static_cast<UIEditBox*>(event->field_0x14)->UnknownFunction473ef0(typed, 9);
            int number = atoi(typed);
            if (number >= 100 && number <= 999) {
                UnknownFunction44f950(number);
                g_UnknownGlobal56e26c->mode.field_0x1bcc = number;
            }
        }
        break;
    case 6:
        g_UnknownGlobal56e26c->ui->UnknownFunction499a20();
        if (field_0x110)
            field_0x110->UnknownFunction404200(field_0x7f74);
        ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(0);
        break;
    }
}

// 0x0044f950
void SPBikeRiderDlg::UnknownFunction44f950(int number) {
    UnknownBikeNumberPainter painter(g_UnknownGlobal56e26c->field_0x1c);
    for (int i = 0; i < g_UnknownGlobal56e26c->ui->field_0x4c; i++)
        painter.UnknownFunction417670(
            ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[i].field_0xc0->field_0x1a0, number);
}

// 0x0044fab0
int SPBikeRiderDlg::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind == 1 && event->control == 0)
        field_0x7f7c = 0;
    return UIDialog::UnknownVirtualSlot22(event, entry);
}

// 0x0044ffc0
int SPBikeRiderDlg::UnknownVirtualSlot13() {
    int result = UIDialog::UnknownVirtualSlot13();
    if (field_0x110)
        field_0x110->UnknownFunction404c80();
    if (field_0x7f84) {
        UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
        KrustyUI* ui = g_UnknownGlobal56e26c->ui;
        UnknownKrustyUIBike* bike = &((UnknownKrustyUIBike*)ui->field_0x50)[list->UnknownFunction4768d0(-1)];
        ((UnknownModelTexture*)((UnknownKrustyUIModel*)ui->field_0x48)[bike->field_0x00].field_0xc0->field_0x1a0)
            ->UnknownFunction444c70(0, bike->field_0x48, &g_UnknownGlobal56e26c->field_0x1c);
        field_0x7f84 = 0;
    }
    if (field_0x7f88) {
        UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLRiders", 6))->field_0x1fc;
        KrustyUI* ui = g_UnknownGlobal56e26c->ui;
        UnknownKrustyUIModel* rider = &((UnknownKrustyUIModel*)ui->field_0x58)[list->UnknownFunction4768d0(-1)];
        ((UnknownModelTexture*)ui->field_0x46c->field_0x1a0)
            ->UnknownFunction444c70(0, rider->field_0x40, &g_UnknownGlobal56e26c->field_0x1c);
        field_0x7f88 = 0;
    }
    return result;
}

// 0x004500d0
void SPBikeRiderDlg::UnknownFunction4500d0() {
    field_0x7f88 = 1;
}

// 0x004500e0: shows the chosen bike and loads its class's garage defaults.
void SPBikeRiderDlg::UnknownFunction4500e0() {
    char text[12];
    UIListBox* bikes = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
    KrustyUI* ui = g_UnknownGlobal56e26c->ui;
    UnknownKrustyUIBike* bike = &((UnknownKrustyUIBike*)ui->field_0x50)[bikes->UnknownFunction4768d0(-1)];
    UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)ui->field_0x48)[bike->field_0x00];
    int i;
    // Retail re-reads TrackGame's KrustyUI after each model is hidden.
    for (i = 0; i < ui->field_0x4c; i++) {
        ((UnknownKrustyUIModel*)ui->field_0x48)[i].field_0xc0->UnknownVirtualSlot4();
        ui = g_UnknownGlobal56e26c->ui;
    }
    model->field_0xc0->UnknownVirtualSlot5();
    field_0x7f84 = 1;
    if (bike->field_0x88) {
        UnknownGameUiControl* engine = UnknownFunction46ebf0("DDLEngineSize", 0);
        if (!engine->field_0x70) {
            engine->UnknownFunction470660(1, 1);
            UnknownFunction46ebf0("TxtEngineSize", 0)->UnknownFunction470660(1, 1);
            if (field_0x110)
                field_0x110->UnknownFunction404da0();
        }
    } else {
        UnknownGameUiControl* engine = UnknownFunction46ebf0("DDLEngineSize", 0);
        if (engine->field_0x70) {
            engine->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("TxtEngineSize", 0)->UnknownFunction470660(0, 1);
            if (field_0x110)
                field_0x110->UnknownFunction404da0();
        }
    }
    g_UnknownGlobal56e26c->mode.field_0x1974.field_0xc4 = bikes->UnknownFunction4768d0(-1);
    if (bike->field_0x88) {
        UIListBox* sizes = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLEngineSize", 6))->field_0x1fc;
        int size = sizes->UnknownFunction4768d0(-1);
        int previous = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->field_0x00);
        UNKNOWN_GARAGE_SETTINGS->field_0x00 = g_UnknownGlobal56cb6c[size];
        UNKNOWN_GARAGE_SETTINGS->field_0x04 = size == 2 || size == 4 ? 1 : 0;
        int current = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->field_0x00);
        if (current != previous)
            UNKNOWN_APPLY_BIKE_CLASS(current, i);
    } else {
        int row = bikes->UnknownFunction4768d0(-1);
        int previous = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->field_0x00);
        UNKNOWN_GARAGE_SETTINGS->field_0x00 =
            ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[row].field_0x8c;
        UNKNOWN_GARAGE_SETTINGS->field_0x04 =
            ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[row].field_0x90;
        int current = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->field_0x00);
        if (current != previous)
            UNKNOWN_APPLY_BIKE_CLASS(current, i);
    }
    UIEditBox* plate = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditPlateNumber", 0xb));
    _itoa(g_UnknownGlobal56e26c->mode.field_0x1bcc, text, 10);
    plate->UnknownFunction473da0(text);
}

// 0x004505d0
void SPBikeRiderDlg::UnknownVirtualSlot31(int apply) {
    if (!apply)
        return;
    strcpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x00, "");
    UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
    int bike = list->UnknownFunction4768d0(-1);
    {
        UnknownKrustyUIBike* bikes = (UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50;
        char* name = ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[bikes[bike].field_0x00].field_0x40;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x00, name, count);
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x00[count] = 0;
    }
    {
        char* name = ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bike].field_0x48;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x40, name, count);
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x40[count] = 0;
    }
    {
        int length = strlen("");
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80, "", count);
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80[count] = 0;
    }
    list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLRiders", 6))->field_0x1fc;
    int rider = list->UnknownFunction4768d0(-1);
    {
        char* name = ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x58)[rider].field_0x40;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80, name, count);
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80[count] = 0;
    }
    g_UnknownGlobal56e26c->mode.UnknownFunction523580();
}

// 0x00450e30
LoadSupercrossDlg::LoadSupercrossDlg(int flags) : LoadingDlg(flags) {
    strcpy(field_0x38, "LoadSX.dtm");
}

// 0x00450e80
LoadEnduroDlg::LoadEnduroDlg(int flags) : LoadingDlg(flags) {
    strcpy(field_0x38, "LoadEnd.dtm");
}

// 0x00450ed0
LoadQuarryDlg::LoadQuarryDlg(int flags) : LoadingDlg(flags) {
    strcpy(field_0x38, "LoadQuar.dtm");
}

// 0x00450f20
LoadNationalsDlg::LoadNationalsDlg(int flags) : LoadingDlg(flags) {
    strcpy(field_0x38, "LoadNat.dtm");
}

// 0x00450f70
LoadTagDlg::LoadTagDlg(int flags) : LoadingDlg(flags) {
    strcpy(field_0x38, "LoadTag.dtm");
}

// 0x00450fd0
LoadBajaDlg::LoadBajaDlg(int flags) : LoadingDlg(flags) {
    strcpy(field_0x38, "LoadBaja.dtm");
}

// 0x00451020
void LoadingDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 7:
        if (g_UnknownGlobal56e26c->field_0x08) {
            if (g_UnknownGlobal59adfc >= 10) {
                g_UnknownGlobal56e26c->mode.field_0x6a4 = 0;
                event->field_0x0c->UnknownFunction46ff30(0);
                event->field_0x20 = 1;
                g_UnknownGlobal56e26c->eventManager->UnknownFunction45d270();
            }
            g_UnknownGlobal59adfc++;
        } else {
            if (g_UnknownGlobal59adfc >= 10) {
                event->field_0x0c->UnknownFunction46ff30(0);
                event->field_0x20 = 1;
                g_UnknownGlobal56e26c->UnknownFunction521a40();
                g_UnknownGlobal56e26c->UnknownFunction468880();
                g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0()->UnknownFunction421050();
                if (g_UnknownGlobal56e26c->field_0x55c)
                    g_UnknownGlobal56e26c->field_0x55c->UnknownFunction4e1f00();
                if (g_UnknownGlobal56e26c->field_0x568)
                    g_UnknownGlobal56e26c->field_0x568->UnknownFunction4e1f00();
                g_UnknownGlobal56e26c->eventManager->UnknownFunction45d270();
            }
            g_UnknownGlobal59adfc++;
        }
        break;
    case 6:
        UnknownFunction46fe40(0);
        field_0x30->UnknownFunction486630(0);
        g_UnknownGlobal56e26c->ui->field_0x490 = 0;
        break;
    case 5: {
        UnknownGameUiControl* area = UnknownFunction46ebf0("StaBarRect", 5);
        UIProgressBar* bar = new(__FILE__, 2878) UIProgressBar(0, (CameraRect*)area->field_0x3c, this);
        UnknownFunction46a8a0(bar, 0, 0);
        bar->UnknownFunction470dc0("ProgressBar");
        UnknownDialogImage* image = (UnknownDialogImage*)UnknownFunction46e9a0("BlueBar");
        bar->UnknownFunction47b3d0(image->UnknownFunction472f90(), 0);
        bar->field_0x1f0 = 20;
        bar->field_0x1fc = 0;
        area->UnknownFunction470660(0, 1);
        g_UnknownGlobal56e26c->ui->field_0x490 = (UnknownGameUiPage*)this;
        g_UnknownGlobal56e26c->ui->UnknownFunction499b10();
        g_UnknownGlobal59ae84 = 0;
        g_UnknownGlobal56e26c->mode.UnknownFunction523580();
        if (g_UnknownGlobal56e26c->field_0x08)
            g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac8d0(
                g_UnknownGlobal56e26c->eventManager->keepAliveTimeout,
                g_UnknownGlobal56e26c->eventManager->keepAliveInterval);
        g_UnknownGlobal59adfc = 0;
        g_UnknownGlobal59ae88 = 0;
        field_0x7f58 = 0;
        break;
    }
    }
}

// 0x00451270
int LoadingDlg::UnknownVirtualSlot10(float frameTime) {
    if (field_0x7f58 == 2 && !g_UnknownGlobal59ae84) {
        if (!g_UnknownGlobal56e26c->eventManager->UnknownFunction45cb70()) {
            UnknownFunction46ff30(0);
            if (g_UnknownGlobal56e26c->field_0x08) {
                UnknownLoadFailedMessage message;
                message.field_0x04 = g_UnknownGlobal56e26c->field_0x08->field_0x0c;
                g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac830(0xcc, &message, sizeof(message),
                                                                        message.field_0x04, 0);
                UnknownFunction4aef40();
            }
            g_UnknownGlobal56e26c->ui->UnknownFunction499b00();
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(100);
            return 1;
        }
        if (g_UnknownGlobal56e26c->field_0x08) {
            for (int i = 0; i < g_UnknownGlobal56e26c->mode.field_0x1be0; i++)
                g_UnknownGlobal56e26c->mode.field_0x1be4[i].UnknownFunction522050();
        }
        g_UnknownGlobal59ae84 = 1;
        UnknownFunction46fce0(0, 100, 0);
    }
    field_0x7f58++;
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00451380
void UnknownFunction451380(int value, char* text) {
    _itoa(value, text, 10);
}

// 0x00451b80
void UnknownFunction451b80(UnknownDialogEvent* event) {
    char name[128];
    char preset[260];
    char control[260];
    UIEditBox* edit = static_cast<UIEditBox*>(event->field_0x0c->UnknownFunction46ebf0("EditBox", 0));
    edit->UnknownFunction473ef0(name, 16);
    if (!strcmp(name, ""))
        return;
    if (g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a910(name)) {
        ProfileExistsDlg* dialog = new(__FILE__, 3416) ProfileExistsDlg;
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 4, 0, (int)event->field_0x0c,
                                                                     (int)name, 0, 1);
        return;
    }
    sprintf(preset, "%s\\%s", "ui\\profile", name);
    if (!CreateDirectoryA(preset, 0))
        return;
    g_UnknownGlobal56e26c->mode.UnknownFunction522440();
    COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x00, name, 16);
    g_UnknownGlobal56e26c->mode.field_0x9c = 1;
    for (int i = 0; i < g_UnknownGlobal56e26c->ui->field_0x54; i++) {
        if (((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[i].field_0x8c == 250) {
            g_UnknownGlobal56e26c->mode.field_0x1974.field_0xc4 = i;
            g_UnknownGlobal56e26c->mode.field_0x1974.field_0xc0 =
                ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[i].field_0x00;
            strcpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x00,
                   ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)
                       [((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[i].field_0x00].field_0x40);
            strcpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x40,
                   ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[i].field_0x48);
            break;
        }
    }
    strcpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80,
           ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x58)[0].field_0x40);
    memcpy(g_UnknownGlobal56e26c->mode.field_0x1b04, &g_UnknownGlobal56e26c->mode.field_0x1974,
           sizeof(g_UnknownGlobal56e26c->mode.field_0x1b04));
    memcpy(&g_UnknownGlobal56e26c->mode.field_0x1a3c, g_UnknownGlobal56e26c->mode.field_0x1b04,
           sizeof(g_UnknownGlobal56e26c->mode.field_0x1a3c));
    for (int type = 0; type < 6; type++) {
        g_UnknownGlobal56e26c->mode.field_0x6f4[type][0] = 0;
        g_UnknownGlobal56e26c->mode.field_0x9f4[type] = 0;
    }
    strcpy(g_UnknownGlobal56e26c->mode.field_0x10, "");
    strcpy(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, "");
    g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 = 0;
    g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34 = 0;
    g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x08 = 0;
    sprintf(preset, "%s\\%s", "ui", "preset.ctl");
    sprintf(control, "%s\\%s\\%s", "ui\\profile", name, "control.ctl");
    CopyFileA(preset, control, 0);
    g_UnknownGlobal56e26c->mode.UnknownFunction523000();
    g_UnknownGlobal56e26c->profileDirectory->UnknownVirtualSlot1();
    g_UnknownGlobal56e26c->mode.UnknownFunction523580();
    g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448e90(control, -1);
    g_UnknownGlobal56e26c->UnknownFunction521a30();
    event->field_0x0c->UnknownFunction46ff30(0x1e);
    event->field_0x20 = 1;
}

// 0x00451ff0
void UserNameDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char playerName[16];
    char text[128];
    switch (event->field_0x08) {
    case 5: {
        g_UnknownGlobal56e26c->ui->field_0x4b0 = 1;
        UnknownGameUiControl* control = UnknownFunction46ebf0("TitleText", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x926);
        control = UnknownFunction46ebf0("OKButton", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e9);
        control = UnknownFunction46ebf0("CancelButton", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13dc);
        int menu = g_UnknownGlobal56e26c->ui->field_0x3c;
        if (menu == 0xbba || menu == 0xbbb)
            UnknownFunction46eb30(0x65, 0);
        UnknownGameUiControl* prompt = UnknownFunction46ebf0("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13ae);
        UIEditBox* edit = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditBox", 0));
        edit->UnknownFunction473c70(15);
        edit->UnknownFunction473f30(kProfileNameCharacters);
        if (g_UnknownGlobal56e26c->ui->field_0x3c == 0xbbb) {
            g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac720(g_UnknownGlobal56e26c->field_0x08->field_0x0c,
                                                                    playerName);
            edit->UnknownFunction473da0(playerName);
            edit->field_0x5c = 12;
            UnknownFunction451b80(event);
            UnknownFunction46ff30(0);
            UnknownFunction46ebf0("OkButton", 0)->UnknownVirtualSlot49(0);
        } else {
            field_0x34->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
            UnknownFunction46ebf0("OkButton", 0)->UnknownVirtualSlot49(0);
        }
        break;
    }
    case 1:
        if (!_stricmp("CancelButton", event->field_0x04))
            UnknownFunction4526b0(0xbb9, event);
        else if (!_stricmp("OKButton", event->field_0x04))
            UnknownFunction452930(0xbb9, event);
        break;
    case 9:
        if (event->field_0x00 == 0x1f) {
            UIEditBox* edit = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditBox", 0));
            edit->UnknownFunction473ef0(text, 128);
            COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x00, text, 16);
            g_UnknownGlobal56e26c->mode.UnknownFunction5231f0();
            UnknownFunction46ff30(0x1f);
        } else {
            UIEditBox* edit = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditBox", 0));
            edit->UnknownFunction473da0("");
            field_0x34->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
        }
        break;
    case 19: {
        UIEditBox* edit = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditBox", 11));
        edit->UnknownFunction473ef0(text, 127);
        if (!text[0])
            UnknownFunction46ebf0("OkButton", 0)->UnknownVirtualSlot49(0);
        else
            UnknownFunction46ebf0("OkButton", 0)->UnknownVirtualSlot49(1);
        break;
    }
    case 6:
        g_UnknownGlobal56e26c->ui->field_0x4b0 = 0;
        break;
    }
}

// 0x00452370
void ProfileExistsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char label[128];
    char text[256];
    switch (event->field_0x08) {
    case 5: {
        UnknownGameUiControl* title = UnknownFunction46ebf0("TitleText", 12);
        UnknownGameUiControl* button = UnknownFunction46ebf0("ButLeft", 1);
        button->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13ec);
        button->field_0x1d8 = 0;
        button = UnknownFunction46ebf0("ButRight", 1);
        button->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13eb);
        button->field_0x1d8 = 0;
        button = UnknownFunction46ebf0("ButMiddle", 1);
        button->UnknownFunction470660(0, 1);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13cf, label, 128);
        sprintf(text, "'%s' %s", (const char*)event->field_0x18, label);
        title->UnknownFunction470b20(text);
        UnknownGameUiControl* prompt = UnknownFunction46ebf0("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x1403);
        break;
    }
    case 1:
        if (!_stricmp("ButLeft", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0x1f);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButRight", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        }
        break;
    }
}

// 0x004524f0
void RemoveProfileDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[128];
    switch (event->field_0x08) {
    case 11:
        if (event->field_0x1c == 0x1b)
            UnknownFunction4526b0(0x12f, event);
        else if (event->field_0x1c == 0xd)
            UnknownFunction452930(0x12f, event);
        break;
    case 5: {
        UnknownGameUiControl* title = UnknownFunction46ebf0("TitleText", 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0xfef, text, 128);
        title->UnknownFunction470da0(10);
        title->UnknownFunction470b20(text);
        sprintf(text, "\"%s\"", (const char*)event->field_0x18);
        UnknownGameUiControl* prompt = UnknownFunction46ebf0("TxtPrompt", 12);
        prompt->UnknownFunction470b20(text);
        UnknownGameUiControl* button = UnknownFunction46ebf0("ButLeft", 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13e9, text, 128);
        button->UnknownFunction470b20(text);
        button = UnknownFunction46ebf0("ButRight", 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13dc, text, 128);
        button->UnknownFunction470b20(text);
        button = UnknownFunction46ebf0("ButMiddle", 1);
        button->UnknownFunction470660(0, 1);
        break;
    }
    case 1:
        if (!_stricmp("ButRight", event->field_0x04))
            UnknownFunction4526b0(0x12f, event);
        else if (!_stricmp("ButLeft", event->field_0x04))
            UnknownFunction452930(0x12f, event);
        break;
    }
}

// 0x004526b0
void UnknownFunction4526b0(int menu, UnknownDialogEvent* event) {
    switch (menu) {
    case 0xc9:
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x66);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xca:
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x66);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xcb:
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x66);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xcd:
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(g_UnknownGlobal56e26c->ui->field_0x30);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xd0:
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xd8:
        event->field_0x0c->UnknownFunction46ff30(0xe);
        event->field_0x20 = 1;
        break;
    case 0xd9:
        event->field_0x0c->UnknownFunction46ff30(0xe);
        event->field_0x20 = 1;
        break;
    case 0x104:
    case 0x105:
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x65);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0x12c:
        event->field_0x0c->UnknownFunction46ff30(0x15);
        event->field_0x20 = 1;
        break;
    case 0x12f:
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0x191: {
        g_UnknownGlobal56e26c->UnknownFunction521860(0, 0x191, 0);
        TrackGameViewOwner* owner = g_UnknownGlobal56e26c->field_0x568;
        if (owner && g_UnknownGlobal56e26c->field_0x08 && g_UnknownGlobal56e26c->field_0x08->isHost &&
            owner->field_0xa8 && owner->field_0x34 && owner->field_0xa8 == owner->field_0x34->field_0x38) {
            UnknownEventRacer* racer = owner->field_0x34->field_0x38;
            if (racer)
                racer->field_0x4a0 = 1;
            g_UnknownGlobal56e26c->field_0x568->UnknownFunction4a9d20();
        }
        g_UnknownGlobal56e26c->eventManager->UnknownFunction45cdc0(2);
        if (g_UnknownGlobal56e26c->field_0x08) {
            g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac510(1);
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
                UnknownFunction4aef40();
                g_UnknownGlobal56e26c->eventManager->UnknownFunction45e710(100);
            } else {
                g_UnknownGlobal56e26c->eventManager->UnknownFunction45e710(0x867);
            }
        } else if (!g_UnknownGlobal56e26c->field_0x3444) {
            g_UnknownGlobal56e26c->eventManager->UnknownFunction45e710(0x65);
        } else {
            g_UnknownGlobal56e26c->eventManager->UnknownFunction45e710(0x88f);
        }
        break;
    }
    case 0x1f9:
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xbb9:
    case 0xbba:
    case 0xbbb:
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    }
}

// 0x00453090
void UnknownFunction453090(int menu, UnknownDialogEvent* event) {
    switch (menu) {
    case 0xfc:
        event->field_0x0c->UnknownFunction46ff30(0xe);
        event->field_0x20 = 1;
        break;
    case 0x66:
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(g_UnknownGlobal56e26c->ui->field_0x30);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0x65:
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(100);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0x910:
    case 0x911: {
        KrustyUI* ui = g_UnknownGlobal56e26c->ui;
        if (ui->field_0x3c == 0x910)
            ui->UnknownFunction499b20(0x65);
        else if (ui->field_0x3c == 0x911)
            ui->UnknownFunction499b20(ui->field_0x38);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    }
    case 0x10e:
        g_UnknownGlobal56e26c->eventManager->UnknownFunction45e520();
        if (g_UnknownGlobal56e26c->field_0x08)
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x867);
        else
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x65);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0x190:
        g_UnknownGlobal56e26c->UnknownFunction521860(0, 0x190, 1);
        break;
    }
}

// 0x00453170
void CreditsVidDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 6:
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1)
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(100);
        break;
    case 5: {
        CameraRect area;
        area.left = 0;
        area.top = 0;
        area.right = 640;
        area.bottom = 480;
        field_0x7f58 = new(__FILE__, 4108) UIVideoStatic(0, &area, this);
        UnknownFunction46a8a0(field_0x7f58, 0, 1);
        field_0x7f58->UnknownFunction4691f0();
        field_0x7f58->UnknownFunction469260(UnknownFunction46ebf0("Back", 0), -1);
        if (!field_0x7f58->UnknownFunction47ae90("ui\\Credits.avi", 1, UnknownFunction44b1f0, this))
            UnknownFunction46ff30(0);
        break;
    }
    case 1:
        if (!_stricmp("Back", event->field_0x04))
            UnknownFunction46ff30(0);
        break;
    }
}

// 0x004532c0
void Intro1Dlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 6:
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1) {
            Intro2Dlg* dialog = new(__FILE__, 4217) Intro2Dlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 0);
        }
        break;
    case 5:
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction486630(0);
        field_0x7f58 = 0;
        g_UnknownGlobal59ae00 = !g_UnknownGlobal56e26c->mode.field_0x6cc;
        break;
    }
}

// 0x004533d0
int Intro1Dlg::UnknownVirtualSlot10(float frameTime) {
    if (++field_0x7f58 == 3) {
        g_UnknownGlobal56e26c->ui->UnknownFunction498cf0(1);
        UnknownFunction46ff30(0);
    }
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00453420
int Intro1Dlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    return g_UnknownGlobal59ae00 = 1;
}

// 0x00453430
void Intro2Dlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 6:
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1) {
            if (!g_UnknownGlobal59ae00) {
                Intro3Dlg* dialog = new(__FILE__, 4256) Intro3Dlg;
                g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 0);
            } else {
                g_UnknownGlobal56e26c->ui->UnknownFunction499b20(100);
                g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction486630(1);
            }
        }
        break;
    case 5:
        field_0x7f60 = 0;
        field_0x7f5c = 0.0f;
        field_0x7f58 = 0;
        g_UnknownGlobal56e26c->UnknownFunction468880();
        break;
    }
}

// 0x00453570
int Intro2Dlg::UnknownVirtualSlot10(float frameTime) {
    if (++field_0x7f58 == 3)
        g_UnknownGlobal56e26c->ui->UnknownFunction498cf0(2);
    field_0x7f5c = frameTime + field_0x7f5c;
    field_0x7f60 = 1;
    if ((g_UnknownGlobal59ae00 || field_0x7f5c > 3.0f) && field_0x7f58 >= 3)
        UnknownFunction46ff30(0);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x004535f0
int Intro2Dlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (field_0x7f60 == 1)
        g_UnknownGlobal59ae00 = 1;
    return 1;
}

// 0x00453610
void Intro3Dlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 6:
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1) {
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(100);
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction486630(1);
        }
        break;
    case 5:
        field_0x7f5c = 0;
        field_0x7f58 = 0.0f;
        g_UnknownGlobal56e26c->UnknownFunction468880();
        break;
    }
}

// 0x00453670
int Intro3Dlg::UnknownVirtualSlot10(float frameTime) {
    field_0x7f58 = frameTime + field_0x7f58;
    field_0x7f5c = 1;
    if (g_UnknownGlobal59ae00 || field_0x7f58 > 3.0f)
        UnknownFunction46ff30(0);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x004536c0
int Intro3Dlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (field_0x7f5c == 1)
        g_UnknownGlobal59ae00 = 1;
    return 1;
}

// 0x004536e0: leaves the menus for the race: keeps a copy of the settings and
// opens the loading dialog of the event type.
void UnknownFunction4536e0() {
    g_UnknownGlobal56e26c->mode.field_0x2dbc = 0;
    g_UnknownGlobal56e26c->ui->field_0x4ac = 1;
    g_UnknownGlobal56e26c->ui->field_0x4a8 = 0;
    g_UnknownGlobal56e26c->ui->UnknownFunction4999b0();
    g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485fc0();
    if (g_UnknownGlobal56e26c->field_0x0c->field_0x6c)
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction4868b0(0);
    if (g_UnknownGlobal56e26c->field_0x18 == 1 && !g_UnknownGlobal56e26c->UnknownFunction521cd0() &&
        !g_UnknownGlobal56e26c->ui->field_0x4a8 && !g_UnknownGlobal56e26c->field_0x3428) {
        memcpy(&g_UnknownGlobal56e26c->mode.field_0x29e4, &g_UnknownGlobal56e26c->mode.field_0x27f8,
               sizeof(g_UnknownGlobal56e26c->mode.field_0x29e4));
        memcpy(g_UnknownGlobal56e26c->mode.field_0x1034, g_UnknownGlobal56e26c->mode.field_0xfd8,
               sizeof(g_UnknownGlobal56e26c->mode.field_0x1034));
        memcpy(&g_UnknownGlobal56e26c->mode.field_0x1a3c, &g_UnknownGlobal56e26c->mode.field_0x1974,
               sizeof(g_UnknownGlobal56e26c->mode.field_0x1a3c));
        g_UnknownGlobal56e26c->mode.field_0x98 = g_UnknownGlobal56e26c->mode.field_0x94;
    }
    if (g_UnknownGlobal56e26c->field_0x18 > 1 && !g_UnknownGlobal56e26c->ui->field_0x4a8 &&
        !g_UnknownGlobal56e26c->field_0x3428) {
        memcpy(&g_UnknownGlobal56e26c->mode.field_0x2bd0, &g_UnknownGlobal56e26c->mode.field_0x27f8,
               sizeof(g_UnknownGlobal56e26c->mode.field_0x2bd0));
        memcpy(g_UnknownGlobal56e26c->mode.field_0x1090, g_UnknownGlobal56e26c->mode.field_0xfd8,
               sizeof(g_UnknownGlobal56e26c->mode.field_0x1090));
        memcpy(g_UnknownGlobal56e26c->mode.field_0x1b04, &g_UnknownGlobal56e26c->mode.field_0x1974,
               sizeof(g_UnknownGlobal56e26c->mode.field_0x1b04));
    }
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2 && !g_UnknownGlobal56e26c->mode.field_0x10ec) {
        g_UnknownGlobal56e26c->mode.field_0x10ec = 1;
        g_UnknownGlobal56e26c->eventManager->field_0x48 = 0;
        for (int i = 0; i < 11; i++)
            g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x28 = 0;
    }
    g_UnknownGlobal56e26c->ui->field_0x494 = (void*)ImmAssociateContext((HWND)g_UnknownGlobal56e26c->field_0x31c, 0);
    if (g_UnknownGlobal56e26c->ui->field_0x2c->field_0x03c)
        g_UnknownGlobal56e26c->ui->field_0x2c->field_0x03c->UnknownFunction404c80();
    if (g_UnknownGlobal56e26c->mode.field_0xa4c != g_UnknownGlobal56e26c->field_0x0c->field_0x0c)
        g_UnknownGlobal56e26c->UnknownVirtualSlot19(g_UnknownGlobal56e26c->mode.field_0xa4c);
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 4) {
        g_UnknownGlobal56e26c->mode.field_0x26f0 = 0;
        g_UnknownGlobal56e26c->mode.field_0x2dbc = 0;
        g_UnknownGlobal56e26c->mode.field_0x94 = 2;
    }
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 3:
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(new(__FILE__, 4405) LoadSupercrossDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 5:
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(new(__FILE__, 4408) LoadEnduroDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 0:
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(new(__FILE__, 4411) LoadQuarryDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 4:
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(new(__FILE__, 4414) LoadTagDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 1:
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(new(__FILE__, 4417) LoadBajaDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    case 2:
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(new(__FILE__, 4420) LoadNationalsDlg(1), 0, 2, 0, 0, 0, 0, 0);
        break;
    }
}

// 0x00453b20
void GhostReplayDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 5: {
        field_0x7f58 = 0;
        field_0x7f5c = 0;
        UIMultiState* tab = static_cast<UIMultiState*>(UnknownFunction46ebf0("TabLeft", 4));
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x143a);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x143a);
        tab->UnknownFunction470da0(0x22);
        tab = static_cast<UIMultiState*>(UnknownFunction46ebf0("TabRight", 4));
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x1439);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x1439);
        tab->UnknownFunction470da0(0x22);
        UnknownFunction453e90(1);
        static_cast<UIRadioButton*>(UnknownFunction46ebf0("TabRight", 4))->UnknownFunction479310(0);
        break;
    }
    case 1:
        if (!_stricmp("Back", event->field_0x04)) {
            if (field_0x7f58) {
                field_0x7f58->Release();
                field_0x7f58 = 0;
            }
            if (field_0x7f5c) {
                field_0x7f5c->Release();
                field_0x7f5c = 0;
            }
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(100);
        } else if (!_stricmp("TabLeft", event->field_0x04)) {
            UnknownFunction453e90(0);
        } else if (!_stricmp("TabRight", event->field_0x04)) {
            UnknownFunction453e90(1);
        } else if (!_stricmp("Help", event->field_0x04)) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523d30("MCM2HELP", 0);
        } else if (!_stricmp("Start", event->field_0x04)) {
            if (field_0x7f58)
                field_0x7f58->UnknownFunction454970();
            else if (field_0x7f5c)
                field_0x7f5c->UnknownFunction455490();
            UnknownFunction46ff30(0);
        }
        break;
    }
}

// 0x00453e90
void GhostReplayDlg::UnknownFunction453e90(int page) {
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
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f58, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 1:
        if (!field_0x7f5c) {
            field_0x7f5c = new(__FILE__, 4504) ReplayFilesDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f5c, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    }
}

// 0x00454030
void GhostFilesDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[256];
    switch (event->field_0x08) {
    case 5:
        field_0x7f5c = 0;
        field_0x7f58 = 0;
        field_0x7f64 = 0;
        field_0x7f60 = 0;
        UnknownFunction454470();
        UnknownFunction46ebf0("LstLaps", 0)->UnknownFunction470660(0, 1);
        UnknownFunction46ebf0("TxtLaps", 0)->UnknownFunction470660(0, 1);
        break;
    case 1:
        if (!_stricmp("ButDelete", event->field_0x04)) {
            if (field_0x7f5c) {
                UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
                sprintf(text, "%s, %s", list->UnknownFunction476d20(-1),
                        field_0x7f58[list->UnknownFunction4768d0(-1)].field_0x10);
                ChoiceDlg* dialog = new(__FILE__, 4533) ChoiceDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0,
                                                  (UnknownGuiDialog*)this, 0, 0, 1);
                dialog->UnknownFunction455700(0, 0x143c, text, 0, 0, 0x143e, 0, 0, 0, 0x143d);
            }
        } else if (!_stricmp("ButDescription", event->field_0x04)) {
            if (field_0x7f5c) {
                COPY_TEXT(field_0x7f68, static_cast<UIListBox*>(UnknownFunction46ebf0("LstDesc", 3))->UnknownFunction476d20(-1), 32);
                EditBoxDlg* dialog = new(__FILE__, 4545) EditBoxDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0,
                                                  (UnknownGuiDialog*)this, 0, 0, 1);
                dialog->UnknownFunction455970(0, 0x143f, 0, 0, field_0x7f68, 32);
            }
        }
        break;
    case 9: {
        int code = event->field_0x00;
        if (code == 0x65) {
            UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
            DeleteFileA(field_0x7f60[list->UnknownFunction4768d0(-1)]);
            UnknownFunction454470();
        } else if (code == 0xc9) {
            UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
            UnknownRecordFileHeader* header = &field_0x7f58[list->UnknownFunction4768d0(-1)];
            strcpy(header->field_0x10, field_0x7f68);
            char* path = field_0x7f60[list->UnknownFunction4768d0(-1)];
            FILE* file = fopen(path, "rb+");
            if (file) {
                fseek(file, 0, SEEK_SET);
                fwrite(header, sizeof(UnknownRecordFileHeader), 1, file);
                fclose(file);
            }
            UnknownFunction454470();
        }
        break;
    }
    case 6:
        if (field_0x7f58)
            DebugFree(field_0x7f58, __FILE__, 4580);
        if (field_0x7f60) {
            for (int i = 0; i < field_0x7f64; i++)
                DebugFree(field_0x7f60[i], __FILE__, 4584);
            DebugFree(field_0x7f60, __FILE__, 4586);
        }
        break;
    }
}

// 0x00454470
void GhostFilesDlg::UnknownFunction454470() {
    char path[260];
    WIN32_FIND_DATAA data;
    int count = 0;
    if (field_0x7f58)
        DebugFree(field_0x7f58, __FILE__, 4599);
    if (field_0x7f60) {
        for (int i = 0; i < field_0x7f64; i++)
            DebugFree(field_0x7f60[i], __FILE__, 4603);
        DebugFree(field_0x7f60, __FILE__, 4605);
    }
    field_0x7f58 = 0;
    field_0x7f60 = 0;
    field_0x7f64 = 0;
    field_0x7f5c = 0;
    static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3))->UnknownFunction4775f0();
    static_cast<UIListBox*>(UnknownFunction46ebf0("LstRider", 3))->UnknownFunction4775f0();
    static_cast<UIListBox*>(UnknownFunction46ebf0("LstLength", 3))->UnknownFunction4775f0();
    static_cast<UIListBox*>(UnknownFunction46ebf0("LstDesc", 3))->UnknownFunction4775f0();
    HANDLE find = FindFirstFileA("Record\\*.gho", &data);
    if (find != INVALID_HANDLE_VALUE) {
        sprintf(path, "%s\\%s", "Record", data.cFileName);
        if (UnknownFunction454640(path))
            count++;
        while (FindNextFileA(find, &data)) {
            sprintf(path, "%s\\%s", "Record", data.cFileName);
            if (UnknownFunction454640(path))
                count++;
        }
        FindClose(find);
    }
    field_0x2c->UnknownFunction46ebf0("Start", 0)->UnknownFunction470660(count != 0, 1);
}

// 0x00454640
int GhostFilesDlg::UnknownFunction454640(const char* path) {
    char name[128];
    char text[64];
    char env[260];
    int added = 0;
    UnknownRecordFileHeader header;
    FILE* file = fopen(path, "rb");
    if (file) {
        if (fread(&header, sizeof(header), 1, file) && header.field_0x30 > 0.0f) {
            field_0x7f58 = (UnknownRecordFileHeader*)UnknownFunction47b570(
                field_0x7f58, (field_0x7f5c + 1) * sizeof(UnknownRecordFileHeader));
            field_0x7f58[field_0x7f5c] = header;
            field_0x7f5c++;
            field_0x7f60 = (char**)UnknownFunction47b570(field_0x7f60, (field_0x7f64 + 1) * sizeof(char*));
            field_0x7f60[field_0x7f64] = (char*)DebugMalloc(strlen(path) + 1, __FILE__, 4657);
            strcpy(field_0x7f60[field_0x7f64], path);
            field_0x7f64++;
            UIListBox* track = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
            UIListBox* rider = static_cast<UIListBox*>(UnknownFunction46ebf0("LstRider", 3));
            UIListBox* length = static_cast<UIListBox*>(UnknownFunction46ebf0("LstLength", 3));
            UIListBox* description = static_cast<UIListBox*>(UnknownFunction46ebf0("LstDesc", 3));
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(header.field_0x3c);
            g_UnknownGlobal56e26c->mode.UnknownFunction523a60(g_UnknownGlobal56e26c->mode.field_0x6a0,
                                                              header.field_0x6a, "env", env);
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9b80(env);
            rider->UnknownFunction476d80(header.field_0x229, 0, 0);
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(text, header.field_0x6a, 0, "scn", 0, 0);
            COPY_TEXT(name, text, 128);
            if (!_stricmp(name, "no name"))
                g_UnknownGlobal56e26c->UnknownFunction521970(0x143b, name, 128);
            track->UnknownFunction476d80(name, field_0x7f5c - 1, 0);
            UnknownFunction488bd0(name, header.field_0x30);
            length->UnknownFunction476d80(name, 0, 0);
            description->UnknownFunction476d80(header.field_0x10, 0, 0);
            added = 1;
        }
        fclose(file);
    }
    return added;
}

// 0x00454970
void GhostFilesDlg::UnknownFunction454970() {
    UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
    UnknownRecordFileHeader* header = &field_0x7f58[list->UnknownFunction4768d0(-1)];
    memcpy(&g_UnknownGlobal56e26c->mode.field_0x27f8, &header->field_0x34, sizeof(g_UnknownGlobal56e26c->mode.field_0x29e4));
    COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x25ec, field_0x7f60[list->UnknownFunction4768d0(-1)], 0x104);
    field_0x2c->UnknownVirtualSlot26();
    g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(header->field_0x3c);
    UnknownFunction4536e0();
}

// 0x00454a50
void ReplayFilesDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[256];
    switch (event->field_0x08) {
    case 5:
        field_0x7f5c = 0;
        field_0x7f58 = 0;
        field_0x7f64 = 0;
        field_0x7f60 = 0;
        UnknownFunction454e60();
        break;
    case 1:
        if (!_stricmp("ButDelete", event->field_0x04)) {
            if (field_0x7f5c) {
                UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
                sprintf(text, "%s, %s", list->UnknownFunction476d20(-1),
                        field_0x7f58[list->UnknownFunction4768d0(-1)].field_0x10);
                ChoiceDlg* dialog = new(__FILE__, 4744) ChoiceDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0,
                                                  (UnknownGuiDialog*)this, 0, 0, 1);
                dialog->UnknownFunction455700(0, 0x143c, text, 0, 0, 0x143e, 0, 0, 0, 0x143d);
            }
        } else if (!_stricmp("ButDescription", event->field_0x04)) {
            if (field_0x7f5c) {
                COPY_TEXT(field_0x7f68, static_cast<UIListBox*>(UnknownFunction46ebf0("LstDesc", 3))->UnknownFunction476d20(-1), 32);
                EditBoxDlg* dialog = new(__FILE__, 4756) EditBoxDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0,
                                                  (UnknownGuiDialog*)this, 0, 0, 1);
                dialog->UnknownFunction455970(0, 0x143f, 0, 0, field_0x7f68, 32);
            }
        }
        break;
    case 9: {
        int code = event->field_0x00;
        if (code == 0x65) {
            UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
            DeleteFileA(field_0x7f60[list->UnknownFunction4768d0(-1)]);
            UnknownFunction454e60();
        } else if (code == 0xc9) {
            UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
            UnknownRecordFileHeader* header = &field_0x7f58[list->UnknownFunction4768d0(-1)];
            COPY_TEXT(header->field_0x10, field_0x7f68, 32);
            char* path = field_0x7f60[list->UnknownFunction4768d0(-1)];
            FILE* file = fopen(path, "rb+");
            if (file) {
                fseek(file, 0, SEEK_SET);
                fwrite(header, sizeof(UnknownRecordFileHeader), 1, file);
                fclose(file);
            }
            UnknownFunction454e60();
        }
        break;
    }
    case 6:
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
void ReplayFilesDlg::UnknownFunction454e60() {
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
    static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3))->UnknownFunction4775f0();
    static_cast<UIListBox*>(UnknownFunction46ebf0("LstRider", 3))->UnknownFunction4775f0();
    static_cast<UIListBox*>(UnknownFunction46ebf0("LstLaps", 3))->UnknownFunction4775f0();
    static_cast<UIListBox*>(UnknownFunction46ebf0("LstLength", 3))->UnknownFunction4775f0();
    static_cast<UIListBox*>(UnknownFunction46ebf0("LstDesc", 3))->UnknownFunction4775f0();
    HANDLE find = FindFirstFileA("Record\\*.vcr", &data);
    if (find != INVALID_HANDLE_VALUE) {
        sprintf(path, "%s\\%s", "Record", data.cFileName);
        if (UnknownFunction455040(path))
            count++;
        while (FindNextFileA(find, &data)) {
            sprintf(path, "%s\\%s", "Record", data.cFileName);
            if (UnknownFunction455040(path))
                count++;
        }
        FindClose(find);
    }
    field_0x2c->UnknownFunction46ebf0("Start", 0)->UnknownFunction470660(count != 0, 1);
}

// 0x00455040
int ReplayFilesDlg::UnknownFunction455040(const char* path) {
    char name[128];
    char text[64];
    char env[260];
    int added = 0;
    UnknownRecordFileHeader header;
    FILE* file = fopen(path, "rb");
    if (file) {
        if (fread(&header, sizeof(header), 1, file) && header.field_0x30 > 0.0f) {
            field_0x7f58 = (UnknownRecordFileHeader*)UnknownFunction47b570(
                field_0x7f58, (field_0x7f5c + 1) * sizeof(UnknownRecordFileHeader));
            field_0x7f58[field_0x7f5c] = header;
            field_0x7f5c++;
            field_0x7f60 = (char**)UnknownFunction47b570(field_0x7f60, (field_0x7f64 + 1) * sizeof(char*));
            field_0x7f60[field_0x7f64] = (char*)DebugMalloc(strlen(path) + 1, __FILE__, 4870);
            strcpy(field_0x7f60[field_0x7f64], path);
            field_0x7f64++;
            UIListBox* track = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
            UIListBox* rider = static_cast<UIListBox*>(UnknownFunction46ebf0("LstRider", 3));
            UIListBox* laps = static_cast<UIListBox*>(UnknownFunction46ebf0("LstLaps", 3));
            UIListBox* length = static_cast<UIListBox*>(UnknownFunction46ebf0("LstLength", 3));
            UIListBox* description = static_cast<UIListBox*>(UnknownFunction46ebf0("LstDesc", 3));
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(header.field_0x3c);
            g_UnknownGlobal56e26c->mode.UnknownFunction523a60(g_UnknownGlobal56e26c->mode.field_0x6a0,
                                                              header.field_0x6a, "env", env);
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9b80(env);
            strcpy(name, "--");
            switch (header.field_0x38) {
            case 3:
                _itoa(header.field_0x54, name, 10);
                break;
            case 2:
                _itoa(header.field_0x54, name, 10);
                break;
            case 1:
                _itoa(header.field_0x54, name, 10);
                break;
            case 5:
                _itoa(header.field_0x54, name, 10);
                break;
            }
            laps->UnknownFunction476d80(name, 0, 0);
            rider->UnknownFunction476d80(header.field_0x229, 0, 0);
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(text, header.field_0x6a, header.field_0x68,
                                                                      "scn", 0, 0);
            if (!_stricmp(text, "no name")) {
                g_UnknownGlobal56e26c->UnknownFunction521970(0x143b, name, 128);
            } else if ((header.field_0x38 == 1 || header.field_0x38 == 5) && header.field_0x60) {
                g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(text, header.field_0x6a, 0, "scn", 0, 0);
                sprintf(name, "%s, #%d", text, header.field_0x64);
            } else {
                strcpy(name, text);
            }
            track->UnknownFunction476d80(name, field_0x7f5c - 1, 0);
            UnknownFunction488bd0(name, header.field_0x30);
            length->UnknownFunction476d80(name, 0, 0);
            description->UnknownFunction476d80(header.field_0x10, 0, 0);
            added = 1;
        }
        fclose(file);
    }
    return added;
}

// 0x00455490
void ReplayFilesDlg::UnknownFunction455490() {
    UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstTrack", 3));
    UnknownRecordFileHeader* header = &field_0x7f58[list->UnknownFunction4768d0(-1)];
    memcpy(&g_UnknownGlobal56e26c->mode.field_0x27f8, &header->field_0x34, sizeof(g_UnknownGlobal56e26c->mode.field_0x29e4));
    g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(header->field_0x3c);
    g_UnknownGlobal56e26c->field_0x3428 = 1;
    g_UnknownGlobal56e26c->field_0x342c = 1;
    *(int*)&g_UnknownGlobal56e26c->mode.field_0xfd8[0] = header->field_0x33c;
    *(int*)&g_UnknownGlobal56e26c->mode.field_0xfd8[4] = header->field_0x340;
    COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x26f4, field_0x7f60[list->UnknownFunction4768d0(-1)], 0x104);
    field_0x2c->UnknownVirtualSlot26();
    UnknownFunction4536e0();
}

// 0x004555b0
void ChoiceDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("ButLeft", event->field_0x04))
            UnknownFunction46ff30(0x65);
        else if (!_stricmp("ButMiddle", event->field_0x04))
            UnknownFunction46ff30(0x66);
        else if (!_stricmp("ButRight", event->field_0x04))
            UnknownFunction46ff30(0x67);
        break;
    }
}

// 0x00455630
void ChoiceDlg::UnknownFunction455630(const char* title, const char* prompt, const char* left,
                                      const char* middle, const char* right) {
    UnknownFunction46ebf0("TitleText", 0)->UnknownFunction470b20(title);
    UnknownFunction46ebf0("TxtPrompt", 0)->UnknownFunction470b20(prompt);
    UnknownGameUiControl* button = UnknownFunction46ebf0("ButLeft", 0);
    if (left && *left)
        button->UnknownFunction470b20(left);
    else
        button->UnknownFunction470660(0, 1);
    button = UnknownFunction46ebf0("ButMiddle", 0);
    if (middle && *middle)
        button->UnknownFunction470b20(middle);
    else
        button->UnknownFunction470660(0, 1);
    button = UnknownFunction46ebf0("ButRight", 0);
    if (right && *right)
        button->UnknownFunction470b20(right);
    else
        button->UnknownFunction470660(0, 1);
}

// 0x00455700
void ChoiceDlg::UnknownFunction455700(const char* title, int titleId, const char* prompt, int promptId,
                                      const char* left, int leftId, const char* middle, int middleId,
                                      const char* right, int rightId) {
    char middleText[128];
    char leftText[128];
    char titleText[128];
    char rightText[128];
    char promptText[1024];
    if (titleId)
        g_UnknownGlobal56e26c->UnknownFunction521970(titleId, titleText, 128);
    if (promptId)
        g_UnknownGlobal56e26c->UnknownFunction521970(promptId, promptText, 128);
    if (leftId)
        g_UnknownGlobal56e26c->UnknownFunction521970(leftId, leftText, 128);
    if (middleId)
        g_UnknownGlobal56e26c->UnknownFunction521970(middleId, middleText, 128);
    if (rightId)
        g_UnknownGlobal56e26c->UnknownFunction521970(rightId, rightText, 128);
    UnknownFunction455630(titleId ? titleText : title, promptId ? promptText : prompt,
                          leftId ? leftText : left, middleId ? middleText : middle,
                          rightId ? rightText : right);
}

// 0x00455840
void EditBoxDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 5:
        field_0x7f58 = 0;
        field_0x7f5c = 0;
        field_0x34->UnknownFunction487790((UnknownGuiControl*)UnknownFunction46ebf0("EditBox", 0), 0, 0);
        break;
    case 1:
        if (!_stricmp("OkButton", event->field_0x04)) {
            static_cast<UIEditBox*>(UnknownFunction46ebf0("EditBox", 11))->UnknownFunction473ef0(field_0x7f58, field_0x7f5c);
            UnknownFunction46ff30(0xc9);
        } else if (!_stricmp("CancelButton", event->field_0x04)) {
            UnknownFunction46ff30(0xca);
        }
        break;
    }
}

// 0x004558f0
void EditBoxDlg::UnknownFunction4558f0(const char* title, const char* prompt, char* buffer, int size) {
    field_0x7f5c = size;
    field_0x7f58 = buffer;
    UnknownFunction46ebf0("TitleText", 0)->UnknownFunction470b20(title);
    UnknownFunction46ebf0("TxtPrompt", 0)->UnknownFunction470b20(prompt);
    UIEditBox* edit = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditBox", 11));
    edit->UnknownFunction473c70(field_0x7f5c - 1);
    edit->UnknownFunction473da0(field_0x7f58);
}

// 0x00455970
void EditBoxDlg::UnknownFunction455970(const char* title, int titleId, const char* prompt, int promptId,
                                       char* buffer, int size) {
    char titleText[128];
    char promptText[1024];
    if (titleId)
        g_UnknownGlobal56e26c->UnknownFunction521970(titleId, titleText, 128);
    if (promptId)
        g_UnknownGlobal56e26c->UnknownFunction521970(promptId, promptText, 128);
    UnknownFunction4558f0(titleId ? titleText : title, promptId ? promptText : prompt, buffer, size);
}

// 0x00455a10
void DemoDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 6:
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1) {
            g_UnknownGlobal56e26c->eventManager->UnknownFunction45cdc0(2);
            memcpy(&g_UnknownGlobal56e26c->mode.field_0x27f8, &g_UnknownGlobal56e26c->mode.field_0x29e4,
                   sizeof(g_UnknownGlobal56e26c->mode.field_0x29e4));
            memcpy(g_UnknownGlobal56e26c->mode.field_0xfd8, g_UnknownGlobal56e26c->mode.field_0x1034,
                   sizeof(g_UnknownGlobal56e26c->mode.field_0x1034));
            memcpy(&g_UnknownGlobal56e26c->mode.field_0x1974, &g_UnknownGlobal56e26c->mode.field_0x1a3c,
                   sizeof(g_UnknownGlobal56e26c->mode.field_0x1a3c));
            g_UnknownGlobal56e26c->eventManager->UnknownFunction45e710(100);
            field_0x30->UnknownFunction486630(1);
        }
        break;
    case 1:
        if (!_stricmp("Back", event->field_0x04))
            UnknownFunction46ff30(0);
        break;
    }
}

// 0x00455ae0
int DemoDlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
    if (view && view->field_0x18a)
        UnknownFunction46ff30(0);
    return 1;
}

// 0x00455b20
int DemoDlg::UnknownVirtualSlot10(float frameTime) {
    KrustyUI* ui = g_UnknownGlobal56e26c->ui;
    if (ui->field_0x44) {
        ui->field_0x44 = 0;
        UnknownFunction46ff30(0);
        return 0;
    }
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00455b60
void TransDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 6:
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1) {
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(field_0x7f60);
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction486630(1);
        }
        break;
    case 5:
        field_0x7f60 = 100;
        field_0x7f58 = 0;
        field_0x7f5c = 0.0f;
        g_UnknownGlobal56e26c->UnknownFunction468880();
        break;
    }
}

// 0x00455bd0
int TransDlg::UnknownVirtualSlot10(float frameTime) {
    field_0x7f5c = frameTime + field_0x7f5c;
    field_0x7f58++;
    if (field_0x7f5c > 5.0f && field_0x7f58 >= 3)
        UnknownFunction46ff30(0);
    if (field_0x7f58 >= 3)
        g_UnknownGlobal56e26c->ui->UnknownFunction498cf0(-1);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00455c40
void TransDlg::UnknownFunction455c40(int menu) {
    field_0x7f60 = menu;
}

// 0x00455c50
void Exit1Dlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 6:
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1)
            PostMessageA((HWND)g_UnknownGlobal56e26c->field_0x0c->field_0x1b0, WM_CLOSE, 0, 0);
        break;
    case 5:
        field_0x7f60 = 0;
        field_0x7f5c = 0.0f;
        field_0x7f58 = 0;
        field_0x7f64 = 0;
        g_UnknownGlobal56e26c->UnknownFunction468880();
        field_0x30->UnknownFunction486630(1);
        break;
    case 1:
        if (!_stricmp("GoLink", event->field_0x04)) {
            g_UnknownGlobal56e26c->openStorePageOnExit = 1;
            field_0x7f64 = 1;
        }
        break;
    }
}

// 0x00455d00
int Exit1Dlg::UnknownVirtualSlot10(float frameTime) {
    field_0x7f5c = frameTime + field_0x7f5c;
    field_0x7f58++;
    field_0x7f60 = 1;
    if ((field_0x7f64 || field_0x7f5c > 15.0f) && field_0x7f58 >= 3)
        UnknownFunction46ff30(0);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x00455d70
int Exit1Dlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (field_0x7f60 == 1 && !event->kind)
        field_0x7f64 = 1;
    return UIDialog::UnknownVirtualSlot23(event, entry);
}
