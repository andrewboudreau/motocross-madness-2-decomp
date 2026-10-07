#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OptionProcs.h"
#include "PCAudio.h"
#include "MouseDevice.h"
#include "PCJoystickDevice.h"
#include "TrackGame.h"
#include "MatrixUtil.h"
#include "DebugAlloc.h"

// The four vector constants that open many retail files (see
// LightEmitter.cpp): 0x00688788, 0x006887c8, 0x006887f8 and 0x00688778,
// initialised by 0x004b1ec0..0x004b1ffb, just before this file's first
// procedure.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

UnknownOptGameSettings g_UnknownGlobal688898;
UnknownOptSoundSettings g_UnknownGlobal688798;
UnknownOptGraphicsSettings g_UnknownGlobal6887d8;
UnknownOptControlSettings g_UnknownGlobal688868;
UnknownOptGarageSettings g_UnknownGlobal688808;
char g_UnknownGlobal688c10[10][128];

// The list box of a drop-down list control.
static inline UnknownGameUiControl* UnknownDropDownList(UnknownGameUiControl* dropDown)
{
    return dropDown->field_0x1fc;
}


// 0x004b2000
void OptGameSettingsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->field_0x08) {
    case 5: {
        UnknownFunction46ebf0("RiderPositionCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x14);
        UnknownFunction46ebf0("RaceStatsCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x04);
        UnknownFunction46ebf0("GuagesCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x00);
        UnknownFunction46ebf0("ChkUIAnims", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x1c);
        UnknownFunction46ebf0("OverviewCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x0c);
        UnknownFunction46ebf0("RiderNamesCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x10);
        UnknownFunction46ebf0("ChkPodiums", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x18);
        UnknownFunction46ebf0("ChkIntroMovie", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x20);
        UnknownFunction46ebf0("ChkPlayerIndicator", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x24);
        UnknownFunction46ebf0("ChkToolTips", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x28);
        UnknownFunction46ebf0("ChkWreckResetOnTrack", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.field_0x2c);
        UnknownGameUiControl* chat = UnknownFunction46ebf0("MultChatMode", 2);
        chat->UnknownFunction478860(3);
        chat->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x140b);
        chat->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x140a);
        chat->UnknownFunction478a50(2, g_UnknownGlobal56e26c->field_0x420, 0x140c);
        UnknownGameUiControl* pid = UnknownFunction46ebf0("TxtPID", 12);
        unsigned long size;
        char value[128];
        char label[128];
        char text[256];
        size = sizeof(value);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13b7, label, 128);
        g_UnknownGlobal56e26c->UnknownVirtualSlot23("PID", "", value, &size);
        sprintf(text, "%s: %s", label, value);
        pid->UnknownFunction470b20(text);
        UnknownFunction46ecc0(0);
        break;
    }
    case 1:
        if (_stricmp("ChkToolTips", event->field_0x04) == 0) {
            if (event->field_0x14->UnknownFunction4755c0())
                field_0x30->UnknownFunction486540(0)->field_0xc0->field_0x5c = 1;
            else
                field_0x30->UnknownFunction486540(0)->field_0xc0->field_0x5c = 0;
        }
        break;
    }
}

// 0x004b22d0
void OptGameSettingsDlg::UnknownVirtualSlot31(int save)
{
    if (save) {
        switch (UnknownFunction46ebf0("MultChatMode", 2)->UnknownFunction4755c0()) {
        case 1:
            g_UnknownGlobal688898.field_0x08 = 1;
            break;
        case 2:
            g_UnknownGlobal688898.field_0x08 = 2;
            break;
        default:
            g_UnknownGlobal688898.field_0x08 = 0;
            break;
        }
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1) {
            if (UnknownFunction46ebf0("ChkToolTips", 2)->UnknownFunction4755c0())
                field_0x30->UnknownFunction486540(0)->field_0xc0->field_0x5c = 1;
            else
                field_0x30->UnknownFunction486540(0)->field_0xc0->field_0x5c = 0;
        }
    } else {
        UnknownGameUiControl* chat = UnknownFunction46ebf0("MultChatMode", 2);
        switch (g_UnknownGlobal688898.field_0x08) {
        case 1:
            chat->UnknownFunction478cf0(1);
            break;
        case 2:
            chat->UnknownFunction478cf0(2);
            break;
        default:
            chat->UnknownFunction478cf0(0);
            break;
        }
    }
}

// 0x004b23b0
void OptGraphicsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    char text[128];
    switch (event->field_0x08) {
    case 5: {
        UnknownFunction46ebf0("ShadowsCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal6887d8.field_0x04);
        UnknownFunction46ebf0("ParticlesCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal6887d8.field_0x08);
        UnknownFunction46ebf0("SkyCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal6887d8.field_0x10);
        UnknownFunction46ebf0("TerrainQualitySliderBar", 0)->UnknownVirtualSlot54(&g_UnknownGlobal6887d8.field_0x18);
        UnknownGameUiControl* dropDown = UnknownFunction46ebf0("DisplayResolutionDropDown", 0);
        dropDown->UnknownFunction470d40(0xffffff);
        dropDown->UnknownFunction470da0(0x12);
        UnknownGameUiControl* list = dropDown->field_0x1fc;
        int i = 0;
        if (g_UnknownGlobal56e26c->field_0x2d4_bit1) {
            if (g_UnknownGlobal56e26c->field_0x2d0) {
                for (; i < g_UnknownGlobal56e26c->field_0x0c->field_0x08; i++) {
                    UnknownDisplayMode* mode = &g_UnknownGlobal56e26c->field_0x0c->field_0x10[i];
                    if (mode->field_0x18 == 1) {
                        sprintf(text, "%d X %d", mode->width, mode->height);
                        list->UnknownFunction476d80(text, i, 0);
                    }
                }
            } else {
                for (; i < g_UnknownGlobal56e26c->field_0x0c->field_0x08; i++) {
                    UnknownDisplayMode* mode = &g_UnknownGlobal56e26c->field_0x0c->field_0x10[i];
                    int usable = g_UnknownGlobal56e26c->field_0x0c->field_0x10[i].field_0x14;
                    if (usable == 1) {
                        sprintf(text, "%d X %d", mode->width, mode->height);
                        list->UnknownFunction476d80(text, i, 0);
                    }
                }
            }
        }
        list->UnknownFunction476b30(g_UnknownGlobal6887d8.field_0x00);
        UnknownFunction46ecc0(0);
        break;
    }
    case 3:
        if (_stricmp("DisplayResolutionDropDown", event->field_0x04) != 0) {
            Sound* sound = UnknownFunction46e9a0("Ratchet03");
            sound->UnknownFunction4bcbe0(field_0x30->field_0x34c, 0);
            if (sound)
                sound->UnknownFunction4bc6b0(0, 0, 0);
        }
        if (_stricmp("TerrainQualitySliderBar", event->field_0x04) == 0) {
            int value = event->field_0x14->UnknownFunction475500();
            g_UnknownGlobal6887d8.field_0x18 = value;
            g_UnknownGlobal56e26c->mode.field_0x195c[1] = value;
            g_UnknownGlobal56e26c->mode.field_0x195c[3] = value;
            g_UnknownGlobal56e26c->mode.field_0x195c[2] = value;
            g_UnknownGlobal56e26c->mode.field_0x195c[4] = value;
        }
        break;
    case 16:
    case 17:
        if (_stricmp("DisplayResolutionDropDown", event->field_0x04) != 0) {
            Sound* sound = UnknownFunction46e9a0("Ratchet03");
            if (sound)
                sound->UnknownFunction4bc940(0);
        }
        break;
    }
}

// 0x004b2670
void OptGraphicsDlg::UnknownVirtualSlot31(int save)
{
    if (save)
        g_UnknownGlobal6887d8.field_0x00 = UnknownDropDownList(UnknownFunction46ebf0("DisplayResolutionDropDown", 6))->UnknownFunction4768d0(-1);
    else
        UnknownDropDownList(UnknownFunction46ebf0("DisplayResolutionDropDown", 6))->UnknownFunction476b30(g_UnknownGlobal6887d8.field_0x00);
}

// 0x004b26c0
void OptAdvancedGraphicsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    int* settings = g_UnknownGlobal56e26c->mode.field_0x195c;
    char key[1024];
    switch (event->field_0x08) {
    case 6: {
        int showPicker = UnknownFunction46ebf0("ChkShowPick3D", 2)->UnknownFunction4755c0();
        g_UnknownGlobal56e26c->UnknownVirtualSlot27("UseLastVideoCard", showPicker == 0);
        int method;
        if (UnknownFunction46ebf0("RadTerrSingle", 4)->UnknownFunction4755c0())
            method = 2;
        else
            method = UnknownFunction46ebf0("RadTerrDual", 4)->UnknownFunction4755c0() != 0;
        sprintf(key, "DriverInfo\\%s\\TerrainDetailTextureMethod", g_UnknownGlobal56e26c->field_0x0c->field_0x4bc);
        g_UnknownGlobal56e26c->UnknownVirtualSlot25(key, method);
        g_UnknownGlobal6887d8.field_0x18 = (settings[4] + settings[3] + settings[2] + settings[1]) / 4;
        break;
    }
    case 5: {
        UnknownFunction46ebf0("ChkForceVideoMem", 0)->UnknownVirtualSlot54(&settings[0]);
        UnknownFunction46ebf0("SldEcosystem", 0)->UnknownVirtualSlot54(&settings[1]);
        UnknownFunction46ebf0("SldObjects", 0)->UnknownVirtualSlot54(&settings[2]);
        UnknownFunction46ebf0("SldTerrain", 0)->UnknownVirtualSlot54(&settings[3]);
        UnknownFunction46ebf0("SldVisibility", 0)->UnknownVirtualSlot54(&settings[4]);
        if (g_UnknownGlobal56e26c->field_0x2d0)
            UnknownFunction46ebf0("Txt3DCard", 12)->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x1463);
        else
            UnknownFunction46ebf0("Txt3DCard", 12)->UnknownFunction470b20(g_UnknownGlobal56e26c->field_0x0c->field_0x5c0.description);
        int useLast = g_UnknownGlobal56e26c->UnknownVirtualSlot22("UseLastVideoCard", 1);
        UnknownFunction46ebf0("ChkShowPick3D", 2)->UnknownFunction478cf0(useLast == 0);
        sprintf(key, "DriverInfo\\%s\\TerrainDetailTextureMethod", g_UnknownGlobal56e26c->field_0x0c->field_0x4bc);
        int method = g_UnknownGlobal56e26c->UnknownVirtualSlot20(key, -1);
        sprintf(key, "DriverInfo\\%s\\CanRenderDualTextureInSinglePass", g_UnknownGlobal56e26c->field_0x0c->field_0x4bc);
        int dual = g_UnknownGlobal56e26c->UnknownVirtualSlot20(key, 0);
        if (method == -1 || (method == 2 && !dual)) {
            if (dual == 1)
                method = 2;
            else
                method = 1;
        }
        if (method == 0)
            UnknownFunction46ebf0("RadTerrNone", 4)->UnknownFunction479310(method);
        else if (method == 1)
            UnknownFunction46ebf0("RadTerrDual", 4)->UnknownFunction479310(0);
        else if (method == 2)
            UnknownFunction46ebf0("RadTerrSingle", 4)->UnknownFunction479310(0);
        if (!dual)
            UnknownFunction46ebf0("RadTerrSingle", 4)->UnknownVirtualSlot49(dual);
        UnknownFunction46ebf0("ChkFilterInput", 2)->UnknownFunction478cf0(
            g_UnknownGlobal56e26c->UnknownVirtualSlot22("JoystickFilter", 0));
        UnknownFunction46ebf0("ChkShowPickController", 2)->UnknownFunction478cf0(
            g_UnknownGlobal56e26c->UnknownVirtualSlot22("UseLastController", 0) == 0);
        UnknownFunction46ecc0(0);
        break;
    }
    case 1:
        if (_stricmp("Back", event->field_0x04) == 0) {
            UnknownFunction46ff30(0x52);
        } else if (_stricmp("ChkFilterInput", event->field_0x04) == 0) {
            int filter = event->field_0x14->UnknownFunction4755c0();
            g_UnknownGlobal56e26c->UnknownVirtualSlot27("JoystickFilter", filter);
            if (g_UnknownGlobal56e26c->field_0x14->activeJoystick)
                ((PCJoystickDevice*)g_UnknownGlobal56e26c->field_0x14->activeJoystick)->UnknownMethod4c3ae0(filter);
        } else if (_stricmp("ChkShowPickController", event->field_0x04) == 0) {
            int useLast = event->field_0x14->UnknownFunction4755c0() == 0;
            g_UnknownGlobal56e26c->UnknownVirtualSlot27("UseLastController", useLast);
            g_UnknownGlobal56e26c->UnknownVirtualSlot27("PresetSelected", 0);
        } else if (_stricmp("Help", event->field_0x04) == 0) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523d30("MCM2HELP", 0);
        }
        break;
    }
}

// 0x004b2b20
void OptSoundDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->field_0x08) {
    case 5:
        UnknownFunction46ebf0("InGameSoundEffectsCheck", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.field_0x08);
        UnknownFunction46ebf0("SoundHardwareCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.field_0x10);
        UnknownFunction46ebf0("UserInterfaceSoundsCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.field_0x04);
        UnknownFunction46ebf0("ChkEAX", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.field_0x14);
        UnknownFunction46ebf0("ChkWaypointChime", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.field_0x18);
        UnknownFunction46ebf0("UserInterfaceVolumeSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.field_0x1c);
        UnknownFunction46ebf0("InGameVolumeSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.field_0x20);
        UnknownFunction46ebf0("Rad16Bit", 4)->UnknownVirtualSlot49(0);
        UnknownFunction46ebf0("Rad8Bit", 4)->UnknownFunction479310(0);
        UnknownFunction46ecc0(0);
        break;
    case 3: {
        Sound* sound = UnknownFunction46e9a0("Ratchet03");
        sound->UnknownFunction4bcbe0(field_0x30->field_0x34c, 0);
        if (sound)
            sound->UnknownFunction4bc6b0(0, 0, 0);
        UnknownFunction4b2d40();
        break;
    }
    case 16:
    case 17: {
        Sound* sound = UnknownFunction46e9a0("Ratchet03");
        if (sound)
            sound->UnknownFunction4bc940(0);
    }
        // fall through
    case 1:
        UnknownFunction4b2d40();
        break;
    }
}

// 0x004b2ce0
void OptSoundDlg::UnknownVirtualSlot31(int save)
{
    if (save) {
        g_UnknownGlobal688798.field_0x28 = UnknownFunction46ebf0("Rad16Bit", 4)->UnknownFunction4793f0() - 1;
    } else if (g_UnknownGlobal688798.field_0x28) {
        UnknownFunction46ebf0("Rad16Bit", 4)->UnknownFunction479310(0);
    } else {
        UnknownFunction46ebf0("Rad8Bit", 4)->UnknownFunction479310(0);
    }
}

// 0x004b2d40
void OptSoundDlg::UnknownFunction4b2d40()
{
    UnknownFunction46ecc0(1);
    if (g_UnknownGlobal56e26c->field_0x04) {
        ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)->field_0x45c_bit0 = g_UnknownGlobal688798.field_0x10;
        ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)->field_0x45c_bit3 = g_UnknownGlobal688798.field_0x14;
        ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)->UnknownFunction4be910(22050, 1, g_UnknownGlobal688798.field_0x28 ? 16 : 8);
    }
    field_0x30->field_0x34c = (g_UnknownGlobal688798.field_0x04 ? (g_UnknownGlobal688798.field_0x1c - 100) * 25 : -10000) - 200;
}


// 0x004b2e00
void OptControlsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    char text[128];
    char path[260];
    int i;
    switch (event->field_0x08) {
    case 5: {
        field_0x7f70 = 0;
        field_0x7f74 = 0;
        field_0x7f78 = 0;
        UnknownFunction46ebf0("ForceFeedCheck", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x20);
        UnknownFunction46ebf0("SteeringSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x00);
        UnknownFunction46ebf0("CrossupSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x04);
        UnknownFunction46ebf0("PitchSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x08);
        UnknownFunction46ebf0("PressThrottleSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x0c);
        UnknownFunction46ebf0("PressBrakesSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x10);
        UnknownFunction46ebf0("ReleaseThrottleSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x14);
        UnknownFunction46ebf0("ReleaseBrakesSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x18);
        UnknownFunction46ebf0("FeedbackSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x1c);
        UnknownFunction46ebf0("ChkGasGyro", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x24);
        UnknownFunction46ebf0("ChkBrakeGyro", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x28);
        UnknownGameUiControl* actions = UnknownFunction46ebf0("ActionListBox", 0);
        for (i = 0; i < 14; i++) {
            g_UnknownGlobal56e26c->UnknownFunction521970(0x192 + i, text, 128);
            actions->UnknownFunction476d80(text, i, 0);
        }
        UnknownGameUiControl* devices = UnknownDropDownList(UnknownFunction46ebf0("InputDeviceDDL", 6));
        devices->UnknownFunction4775f0();
        for (i = 0; i < 8; i++) {
            g_UnknownGlobal56e26c->UnknownFunction521970(0x1cc + i, text, 128);
            devices->UnknownFunction476d80(text, i, 0);
        }
        UnknownFunction4b3410();
        UnknownGameUiControl* keys = UnknownFunction46ebf0("MapKeyListBox", 0);
        for (i = 0; i < 14; i++) {
            g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449350(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, i, text);
            keys->UnknownFunction476d80(text, i, 0);
        }
        UnknownFunction4b54c0();
        UnknownGameUiControl* analog = UnknownFunction46ebf0("ChkAnalogAxes", 0);
        analog->UnknownVirtualSlot54(&g_UnknownGlobal688868.field_0x2c);
        if (g_UnknownGlobal56e26c->field_0x14->activeJoystick)
            g_UnknownGlobal56e26c->field_0x14->activeJoystick->UnknownFunction4897e0(4);
        analog->UnknownFunction470660(0, 1);
        field_0x7f78 = 0;
        if (g_UnknownGlobal56e26c->field_0x14->activeJoystick)
            field_0x7f78 = g_UnknownGlobal56e26c->field_0x14->activeJoystick->deviceKind == 3;
        UnknownFunction46eb30(0x12f, field_0x7f78);
        UnknownFunction46ecc0(0);
        field_0x7f74 = 0;
        UnknownFunction46ecc0(0);
        break;
    }
    case 11:
    case 12:
        event->field_0x18 = 0;
        break;
    case 3:
        if (_stricmp("InputDeviceDDL", event->field_0x04) != 0 && _stricmp("ListboxScroll", event->field_0x04) != 0) {
            Sound* sound = UnknownFunction46e9a0("Ratchet03");
            sound->UnknownFunction4bcbe0(field_0x30->field_0x34c, 0);
            if (sound)
                sound->UnknownFunction4bc6b0(0, 0, 0);
        }
        break;
    case 16:
    case 17:
        if (_stricmp("InputDeviceDDL", event->field_0x04) != 0 && _stricmp("ListboxScroll", event->field_0x04) != 0) {
            Sound* sound = UnknownFunction46e9a0("Ratchet03");
            if (sound)
                sound->UnknownFunction4bc940(0);
        }
        break;
    case 2:
        if (field_0x7f70 || field_0x7f74)
            break;
        if (_stricmp("InputDeviceDDL", event->field_0x04) == 0) {
            g_UnknownGlobal56e26c->field_0x33fc->field_0x00 =
                UnknownDropDownList(UnknownFunction46ebf0("InputDeviceDDL", 6))->UnknownFunction4768d0(-1);
            UnknownFunction4b3410();
        } else if (_stricmp("MapKeyListBox", event->field_0x04) == 0 || _stricmp("ActionListBox", event->field_0x04) == 0) {
            int row = event->field_0x14->UnknownFunction476950();
            UnknownGameUiControl* keys = UnknownFunction46ebf0("MapKeyListBox", 3);
            g_UnknownGlobal56e26c->UnknownFunction521970(0x1c2, text, 128);
            keys->UnknownFunction476ff0(row, text);
            UnknownFunction46ea80(0x65, 0);
            UnknownFunction46ea80(0xca, 0);
            UnknownFunction46ea80(0x12f, 0);
            if (g_UnknownGlobal56e26c->field_0x14->activeJoystick) {
                for (i = 0; i < 6; i++)
                    field_0x7f58[i] = g_UnknownGlobal56e26c->field_0x14->activeJoystick->UnknownFunction489e20(i);
            }
            field_0x7f74 = 1;
            UnknownFunction46fce0(0, 0xfa, 0);
        }
        break;
    case 7:
        field_0x7f74 = 0;
        if (!field_0x7f70)
            field_0x7f70 = 1;
        UnknownFunction4b5760();
        break;
    case 6:
        sprintf(path, "%s\\%s\\%s", "ui\\profile", g_UnknownGlobal56e26c->mode.field_0x00, "control.ctl");
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448e90(path, -1);
        break;
    }
}

// 0x004b3410
void OptControlsDlg::UnknownFunction4b3410()
{
    char text[128];
    UnknownGameUiControl* keys = UnknownFunction46ebf0("MapKeyListBox", 3);
    keys->UnknownFunction4775f0();
    for (int i = 0; i < 14; i++) {
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449350(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, i, text);
        keys->UnknownFunction476d80(text, i, 0);
    }
}

// 0x004b3470
void OptControlsDlg::UnknownVirtualSlot31(int save)
{
    if (save) {
        if (g_UnknownGlobal56e26c->field_0x33fc)
            g_UnknownGlobal56e26c->field_0x33fc->field_0x00 =
                UnknownDropDownList(UnknownFunction46ebf0("InputDeviceDDL", 6))->UnknownFunction4768d0(-1);
    } else if (g_UnknownGlobal56e26c->field_0x33fc) {
        UnknownDropDownList(UnknownFunction46ebf0("InputDeviceDDL", 6))->UnknownFunction476b30(g_UnknownGlobal56e26c->field_0x33fc->field_0x00);
        UnknownFunction4b3410();
    }
}

// 0x004b54c0
void OptControlsDlg::UnknownFunction4b54c0()
{
    UnknownGameUiControl* list;
    int i;
    if (g_UnknownGlobal56e26c->field_0x33fc->field_0x00 == 1) {
        list = UnknownFunction46ebf0("MapKeyListBox", 3);
        for (i = 0; i < 4; i++)
            list->UnknownFunction476ba0(0x5555ff, i);
        list = UnknownFunction46ebf0("ActionListBox", 3);
        for (i = 0; i < 4; i++)
            list->UnknownFunction476ba0(0x5555ff, i);
    } else {
        list = UnknownFunction46ebf0("MapKeyListBox", 3);
        for (i = 0; i < 4; i++)
            list->UnknownFunction476ba0(0xff000000, i);
        list = UnknownFunction46ebf0("ActionListBox", 3);
        for (i = 0; i < 4; i++)
            list->UnknownFunction476ba0(0xff000000, i);
    }
}

// 0x004b5570
int UnknownFunction4b5570(int row, int kind, int code, char* text)
{
    strcpy(text, "");
    if (row < 4 && kind != 0)
        return 0;
    return g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449380(kind, code, text);
}

// 0x004b55d0
int OptControlsDlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    if (!field_0x7f70)
        return GameObject::UnknownVirtualSlot23(event, entry);
    return 1;
}

// 0x004b5a20
void OptControlsDlg::UnknownFunction4b5a20()
{
    field_0x7f70 = 0;
    UnknownFunction46fe40(0);
    UnknownFunction46ea80(0x65, 1);
    UnknownFunction46ea80(0xca, 1);
    UnknownFunction46ea80(0x12f, 1);
}

// 0x004b40a0
void OptMessagesDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    if (event->field_0x08 == 5) {
        UnknownFunction46ebf0("Message1", 11)->UnknownFunction473820(g_UnknownGlobal688c10[1], 128);
        UnknownFunction46ebf0("Message2", 11)->UnknownFunction473820(g_UnknownGlobal688c10[2], 128);
        UnknownFunction46ebf0("Message3", 11)->UnknownFunction473820(g_UnknownGlobal688c10[3], 128);
        UnknownFunction46ebf0("Message4", 11)->UnknownFunction473820(g_UnknownGlobal688c10[4], 128);
        UnknownFunction46ebf0("Message5", 11)->UnknownFunction473820(g_UnknownGlobal688c10[5], 128);
        UnknownFunction46ebf0("Message6", 11)->UnknownFunction473820(g_UnknownGlobal688c10[6], 128);
        UnknownFunction46ebf0("Message7", 11)->UnknownFunction473820(g_UnknownGlobal688c10[7], 128);
        UnknownFunction46ebf0("Message8", 11)->UnknownFunction473820(g_UnknownGlobal688c10[8], 128);
        UnknownFunction46ebf0("Message9", 11)->UnknownFunction473820(g_UnknownGlobal688c10[9], 128);
        UnknownFunction46ebf0("Message0", 11)->UnknownFunction473820(g_UnknownGlobal688c10[0], 128);
        UnknownFunction46ecc0(0);
        GameObjectIterator controls(field_0x7f3c, 1, "UIControl");
        GameObject* control;
        while ((control = controls.Next()) != 0)
            ((UnknownGameUiControl*)control)->UnknownVirtualSlot49(0);
    }
}

// TrackGame's settings blocks (see OptionProcs.h).
static inline UnknownOptGameSettings* UnknownGameSettingsOf(TrackGame* game)
{
    return (UnknownOptGameSettings*)((char*)game + 0xc24);
}

static inline UnknownOptSoundSettings* UnknownSoundSettingsOf(TrackGame* game)
{
    return (UnknownOptSoundSettings*)((char*)game + 0xf98);
}

static inline UnknownOptGraphicsSettings* UnknownGraphicsSettingsOf(TrackGame* game)
{
    return (UnknownOptGraphicsSettings*)&game->mode.field_0xa4c;
}

static inline UnknownOptControlSettings* UnknownControlSettingsOf(TrackGame* game)
{
    return (UnknownOptControlSettings*)((char*)game + 0xfe0);
}

static inline UnknownOptGarageSettings* UnknownGarageSettingsOf(TrackGame* game)
{
    return (UnknownOptGarageSettings*)game->mode.field_0xfd8;
}

static inline char (*UnknownMessagesOf(TrackGame* game))[128]
{
    return (char (*)[128])((char*)game + 0x19d4);
}

// 0x004b4280
void OptionsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->field_0x08) {
    case 5: {
        TrackGame* game = g_UnknownGlobal56e26c;
        g_UnknownGlobal688808 = *UnknownGarageSettingsOf(game);
        g_UnknownGlobal688898 = *UnknownGameSettingsOf(game);
        g_UnknownGlobal688798 = *UnknownSoundSettingsOf(game);
        g_UnknownGlobal6887d8 = *UnknownGraphicsSettingsOf(game);
        g_UnknownGlobal688868 = *UnknownControlSettingsOf(game);
        memcpy(g_UnknownGlobal688c10, UnknownMessagesOf(game), sizeof(g_UnknownGlobal688c10));
        field_0x7f58 = 0;
        field_0x7f5c = 0;
        field_0x7f60 = 0;
        field_0x7f64 = 0;
        field_0x7f68 = 0;
        field_0x7f6c = 0;
        UnknownGameUiControl* tab = UnknownFunction46ebf0("GameSettings", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x13dd);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x13dd);
        tab->UnknownFunction470da0(0x22);
        tab = UnknownFunction46ebf0("Graphics", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x13de);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x13de);
        tab->UnknownFunction470da0(0x22);
        tab = UnknownFunction46ebf0("Sound", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x13df);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x13df);
        tab->UnknownFunction470da0(0x22);
        tab = UnknownFunction46ebf0("Controls", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x13e0);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x13e0);
        tab->UnknownFunction470da0(0x22);
        tab = UnknownFunction46ebf0("Messages", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x13e1);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x13e1);
        tab->UnknownFunction470da0(0x22);
        tab = UnknownFunction46ebf0("Garage", 4);
        tab->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x13e2);
        tab->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x13e2);
        tab->UnknownFunction470da0(0x22);
        if (event->field_0x18 == 1) {
            UnknownFunction4b4ab0(3);
            UnknownFunction46ebf0("Controls", 4)->UnknownFunction479310(0);
        } else if (event->field_0x18 == 2) {
            UnknownFunction4b4ab0(5);
            UnknownFunction46ebf0("Garage", 4)->UnknownFunction479310(0);
        } else {
            UnknownFunction4b4ab0(0);
            UnknownFunction46ebf0("GameSettings", 4)->UnknownFunction479310(0);
        }
        if (g_UnknownGlobal56e26c->UnknownFunction521cd0())
            UnknownFunction46ebf0("Restore", 0)->UnknownFunction470660(0, 1);
        break;
    }
    case 1:
        if (_stricmp("Back", event->field_0x04) == 0) {
            if (field_0x7f58) {
                field_0x7f58->Release();
                field_0x7f58 = 0;
            }
            if (field_0x7f5c) {
                field_0x7f5c->Release();
                field_0x7f5c = 0;
            }
            if (field_0x7f60) {
                field_0x7f60->Release();
                field_0x7f60 = 0;
            }
            if (field_0x7f64) {
                field_0x7f64->Release();
                field_0x7f64 = 0;
            }
            if (field_0x7f68) {
                field_0x7f68->Release();
                field_0x7f68 = 0;
            }
            if (field_0x7f6c) {
                field_0x7f6c->Release();
                field_0x7f6c = 0;
            }
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (_stricmp("GameSettings", event->field_0x04) == 0) {
            UnknownFunction4b4ab0(0);
        } else if (_stricmp("Graphics", event->field_0x04) == 0) {
            UnknownFunction4b4ab0(1);
        } else if (_stricmp("Sound", event->field_0x04) == 0) {
            UnknownFunction4b4ab0(2);
        } else if (_stricmp("Controls", event->field_0x04) == 0) {
            UnknownFunction4b4ab0(3);
        } else if (_stricmp("Messages", event->field_0x04) == 0) {
            UnknownFunction4b4ab0(4);
        } else if (_stricmp("Garage", event->field_0x04) == 0) {
            UnknownFunction4b4ab0(5);
        } else if (_stricmp("Restore", event->field_0x04) == 0) {
            GlobalSettingsDlg* dialog = new(__FILE__, 979) GlobalSettingsDlg;
            event->field_0x10->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 0xc, 0,
                                                     (UnknownGuiDialog*)this, 0, 0, 1);
        } else if (_stricmp("Help", event->field_0x04) == 0) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523d30("MCM2HELP", 0);
        } else if (_stricmp("Advanced", event->field_0x04) == 0) {
            if (field_0x7f5c)
                field_0x7f5c->UnknownFunction46ecc0(1);
            OptAdvancedGraphicsDlg* dialog = new(__FILE__, 987) OptAdvancedGraphicsDlg;
            event->field_0x10->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 0xc, 0,
                                                     (UnknownGuiDialog*)this, 0, 0, 1);
        }
        break;
    case 9:
        if (event->field_0x00 == 0x51) {
            TrackGame* game = g_UnknownGlobal56e26c;
            g_UnknownGlobal688808 = *UnknownGarageSettingsOf(game);
            g_UnknownGlobal688898 = *UnknownGameSettingsOf(game);
            g_UnknownGlobal688798 = *UnknownSoundSettingsOf(game);
            g_UnknownGlobal6887d8 = *UnknownGraphicsSettingsOf(game);
            g_UnknownGlobal688868 = *UnknownControlSettingsOf(game);
            memcpy(g_UnknownGlobal688c10, UnknownMessagesOf(game), sizeof(g_UnknownGlobal688c10));
            if (field_0x7f58)
                field_0x7f58->UnknownFunction46ecc0(0);
            if (field_0x7f5c)
                field_0x7f5c->UnknownFunction46ecc0(0);
            if (field_0x7f60) {
                field_0x7f60->UnknownFunction46ecc0(0);
                field_0x7f60->UnknownFunction4b2d40();
            }
            if (field_0x7f64)
                field_0x7f64->UnknownFunction46ecc0(0);
            if (field_0x7f68)
                field_0x7f68->UnknownFunction46ecc0(0);
            if (field_0x7f6c) {
                field_0x7f6c->UnknownFunction46ecc0(0);
                field_0x7f6c->UnknownFunction4b38f0();
                field_0x7f6c->UnknownFunction4b3bc0();
            }
            KrustyUI* ui = g_UnknownGlobal56e26c->ui;
            int count = ui->field_0x54;
            for (int i = 0; i < count; i++) {
                if (((UnknownOptKrustyUIRecord*)ui->field_0x50)[i].field_0x8c == 0xfa) {
                    g_UnknownGlobal56e26c->mode.field_0x1974.field_0xc4 = i; // TrackGame+0x1fb0
                    break;
                }
            }
            UnknownFunction46ff70(0x51, 9);
        } else if (event->field_0x00 == 0x52) {
            if (field_0x7f5c)
                field_0x7f5c->UnknownFunction46ecc0(0);
        }
        break;
    case 6:
        *UnknownGarageSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688808;
        *UnknownGameSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688898;
        *UnknownSoundSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688798;
        *UnknownGraphicsSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal6887d8;
        *UnknownControlSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688868;
        memcpy(UnknownMessagesOf(g_UnknownGlobal56e26c), g_UnknownGlobal688c10, sizeof(g_UnknownGlobal688c10));
        field_0x30->field_0xec = g_UnknownGlobal688898.field_0x1c;
        if (field_0x2c)
            field_0x2c->field_0xc8 = g_UnknownGlobal688898.field_0x1c;
        break;
    }
}

// 0x004b4ab0
void OptionsDlg::UnknownFunction4b4ab0(int page)
{
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
    if (field_0x7f60 && page != 2) {
        field_0x7f60->Release();
        field_0x7f60 = 0;
    }
    if (field_0x7f64 && page != 3) {
        field_0x7f64->Release();
        field_0x7f64 = 0;
    }
    if (field_0x7f68 && page != 4) {
        field_0x7f68->Release();
        field_0x7f68 = 0;
    }
    if (field_0x7f6c && page != 5) {
        field_0x7f6c->Release();
        field_0x7f6c = 0;
    }
    switch (page) {
    case 0:
        if (!field_0x7f58) {
            field_0x7f58 = new(__FILE__, 1066) OptGameSettingsDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f58, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 1:
        if (!field_0x7f5c) {
            field_0x7f5c = new(__FILE__, 1072) OptGraphicsDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f5c, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 2:
        if (!field_0x7f60) {
            field_0x7f60 = new(__FILE__, 1078) OptSoundDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f60, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 3:
        if (!field_0x7f64) {
            field_0x7f64 = new(__FILE__, 1084) OptControlsDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f64, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 4:
        if (!field_0x7f68) {
            field_0x7f68 = new(__FILE__, 1090) OptMessagesDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f68, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    case 5:
        if (!field_0x7f6c) {
            field_0x7f6c = new(__FILE__, 1096) OptGarageDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f6c, 0, 1, (int)&area, (int)this, 0, 0, 1);
        }
        break;
    }
}

extern "C" __declspec(dllimport) int __stdcall CopyFileA(const char* from, const char* to, int failIfExists);

// 0x004b4e70
void GlobalSettingsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->field_0x08) {
    case 9:
        if (event->field_0x00 == 0x60) {
            char name[128];
            char profile[260];
            char preset[260];
            int choice = UnknownFunction46ebf0("ButFactory", 4)->UnknownFunction4793f0();
            strcpy(name, g_UnknownGlobal56e26c->mode.field_0x00);
            if (choice == 1) {
                g_UnknownGlobal56e26c->mode.UnknownFunction522440();
                sprintf(preset, "%s\\%s", "ui", "preset.ctl");
                sprintf(profile, "%s\\%s\\%s", "ui\\profile", name, "control.ctl");
                CopyFileA(preset, profile, 0);
                g_UnknownGlobal56e26c->mode.UnknownFunction523000();
                g_UnknownGlobal56e26c->mode.UnknownFunction523580();
                g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448e90(profile, -1);
            } else if (choice == 2) {
                UnknownGameUiControl* profiles = UnknownFunction46ebf0("LstProfiles", 3);
                UnknownGameUiListRow* entry = &profiles->UnknownInlineListBox()->field_0x214[profiles->UnknownFunction476950()];
                strcpy(g_UnknownGlobal56e26c->mode.field_0x00, entry->field_0x14);
                g_UnknownGlobal56e26c->mode.UnknownFunction5231f0();
            }
            strcpy(g_UnknownGlobal56e26c->mode.field_0x00, name);
            UnknownFunction46ff30(0x51);
        } else if (event->field_0x00 == 0x61) {
            UnknownFunction46ff30(0);
        }
        break;
    case 5:
        UnknownFunction4b51a0();
        UnknownFunction46ebf0("LstProfiles", 3)->UnknownFunction470660(0, 1);
        break;
    case 1:
        if (_stricmp("ButFactory", event->field_0x04) == 0) {
            UnknownFunction46ebf0("LstProfiles", 3)->UnknownFunction470660(0, 1);
        } else if (_stricmp("ButCopyUser", event->field_0x04) == 0) {
            UnknownFunction46ebf0("LstProfiles", 3)->UnknownFunction470660(1, 1);
        } else if (_stricmp("ButCancel", event->field_0x04) == 0) {
            UnknownFunction46ff30(0);
        } else if (_stricmp("ButLoad", event->field_0x04) == 0) {
            ConfirmRestoreDlg* dialog = new(__FILE__, 1123) ConfirmRestoreDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 0xc, 0,
                                              (UnknownGuiDialog*)this, 0, 0, 1);
        }
        break;
    }
}

// 0x004b51a0
void GlobalSettingsDlg::UnknownFunction4b51a0()
{
    char name[260];
    int count = 0;
    UnknownGameUiControl* profiles = UnknownFunction46ebf0("LstProfiles", 0);
    profiles->UnknownFunction4775f0();
    g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a1d0("ui\\profile");
    g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a220("*", 0);
    g_UnknownGlobal56e26c->profileDirectory->UnknownVirtualSlot1();
    g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a550(name);
    if (name[0] && strcmp(g_UnknownGlobal56e26c->mode.field_0x00, name) != 0) {
        profiles->UnknownFunction476d80(name, 0, 0);
        count = 1;
    }
    while (g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a4c0(name)) {
        if (name[0] && strcmp(g_UnknownGlobal56e26c->mode.field_0x00, name) != 0) {
            profiles->UnknownFunction476d80(name, 0, 0);
            count++;
        }
    }
    profiles->UnknownFunction477900(1);
    profiles->UnknownFunction476c70(0xfeb97a, -1);
    profiles->UnknownFunction476b80(0xffffff);
    profiles->UnknownFunction476cd0(0xfeb97a);
    if (count) {
        profiles->UnknownFunction476ad0(g_UnknownGlobal56e26c->mode.field_0x00);
    } else {
        UnknownGameUiControl* copy = UnknownFunction46ebf0("ButCopyUser", 4);
        copy->UnknownVirtualSlot49(0);
        copy->UnknownFunction470d40(0x757575);
    }
}

// 0x004b5380
void ConfirmRestoreDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->field_0x08) {
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("ButLeft", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e9);
        control = UnknownFunction46ebf0("ButRight", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13dc);
        control = UnknownFunction46ebf0("ButMiddle", 1);
        control->UnknownFunction470660(0, 1);
        control = UnknownFunction46ebf0("TitleText", 12);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x14c9);
        UnknownGameUiControl* prompt = UnknownFunction46ebf0("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x14ca);
        break;
    }
    case 1:
        if (_stricmp("ButLeft", event->field_0x04) == 0) {
            event->field_0x0c->UnknownFunction46ff30(0x60);
            event->field_0x20 = 1;
        } else if (_stricmp("ButRight", event->field_0x04) == 0) {
            event->field_0x0c->UnknownFunction46ff30(0x61);
            event->field_0x20 = 1;
        }
        break;
    }
}

// 0x004b3500
void OptGarageDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->field_0x08) {
    case 5: {
        UnknownFunction46ebf0("TxtUnused", 0)->field_0x1e8 = 1;
        UnknownFunction4b38f0();
        UnknownFunction4b3bc0();
        UnknownFunction46ecc0(0);
        GameObjectIterator controls(field_0x7f3c, 1, "UIControl");
        GameObject* control;
        while ((control = controls.Next()) != 0)
            ((UnknownGameUiControl*)control)->UnknownVirtualSlot49(0);
        break;
    }
    case 1:
        if (_stricmp("CurveLeft", event->field_0x04) == 0) {
            UnknownGameUiControl* curves = UnknownDropDownList(UnknownFunction46ebf0("DDLCurves", 6));
            int rows = curves->field_0x1ec;
            curves->UnknownFunction476a60((curves->UnknownFunction476950() + rows - 1) % rows);
            curves->UnknownVirtualSlot66(0);
        } else if (_stricmp("CurveRight", event->field_0x04) == 0) {
            UnknownGameUiControl* curves = UnknownDropDownList(UnknownFunction46ebf0("DDLCurves", 6));
            int rows = curves->field_0x1ec;
            curves->UnknownFunction476a60((curves->UnknownFunction476950() + 1) % rows);
            curves->UnknownVirtualSlot66(0);
        }
        break;
    case 2:
        if (_stricmp("DDLCurves", event->field_0x04) == 0) {
            int curve = event->field_0x14->UnknownFunction4768d0(-1);
            g_UnknownGlobal688898.field_0x360[UnknownBikeClassOf(g_UnknownGlobal688808.field_0x00)] = curve;
            TrackGame* game = g_UnknownGlobal56e26c;
            for (int band = 0; band < 11; band++) {
                if (curve < 3)
                    g_UnknownGlobal688808.field_0x24[band] =
                        game->ui->field_0x68[UnknownBikeClassOf(g_UnknownGlobal688808.field_0x00)][curve][band];
                else
                    g_UnknownGlobal688808.field_0x24[band] =
                        game->mode.field_0x10f0[UnknownBikeClassOf(g_UnknownGlobal688808.field_0x00)][curve - 3][band];
            }
            UnknownFunction4b3bc0();
        }
        break;
    case 3:
        if (_stricmp("DDLCurves", event->field_0x04) != 0) {
            Sound* sound = UnknownFunction46e9a0("Ratchet03");
            sound->UnknownFunction4bcbe0(field_0x30->field_0x34c, 0);
            if (sound)
                sound->UnknownFunction4bc6b0(0, 0, 0);
            if (strlen(event->field_0x04) > 5 && _strnicmp(event->field_0x04, "SldEQ", 5) == 0)
                UnknownFunction4b3aa0(event->field_0x14);
        }
        break;
    case 16:
    case 17:
        if (_stricmp("DDLCurves", event->field_0x04) != 0) {
            Sound* sound = UnknownFunction46e9a0("Ratchet03");
            if (sound)
                sound->UnknownFunction4bc940(0);
            if (strlen(event->field_0x04) > 5 && _strnicmp(event->field_0x04, "SldEQ", 5) == 0)
                UnknownFunction4b3aa0(event->field_0x14);
        }
        break;
    }
}

// 0x004b38f0
void OptGarageDlg::UnknownFunction4b38f0()
{
    char format[32];
    char text[128];
    char engine[128];
    char standard[128];
    char custom[128];
    g_UnknownGlobal56e26c->UnknownFunction521970(0x146a, format, 127);
    sprintf(engine, format, g_UnknownGlobal56cb6c[UnknownBikeClassOf(g_UnknownGlobal688808.field_0x00)]);
    g_UnknownGlobal56e26c->UnknownFunction521970(0x146b, standard, 127);
    g_UnknownGlobal56e26c->UnknownFunction521970(0x146c, custom, 127);
    UnknownGameUiControl* curves = UnknownDropDownList(UnknownFunction46ebf0("DDLCurves", 6));
    curves->UnknownFunction4775f0();
    for (int i = 0; i < 6; i++) {
        if (i < 3)
            sprintf(text, "%dcc (%s) %s #%d", g_UnknownGlobal688808.field_0x00, engine, standard, i + 1);
        else
            sprintf(text, "%dcc (%s) %s #%d", g_UnknownGlobal688808.field_0x00, engine, custom, i - 2);
        curves->UnknownFunction476d80(text, i, 0);
    }
    curves->UnknownFunction476b30(g_UnknownGlobal688898.field_0x360[UnknownBikeClassOf(g_UnknownGlobal688808.field_0x00)]);
}

// 0x004b3aa0
void OptGarageDlg::UnknownFunction4b3aa0(UnknownGameUiControl* slider)
{
    int bikeClass = UnknownBikeClassOf(g_UnknownGlobal688808.field_0x00);
    int band = slider->field_0x7c;
    int value = slider->UnknownFunction475300(g_UnknownGlobal56e26c->ui->field_0x450[bikeClass] - g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass]);
    value += g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass];
    int level = __min(g_UnknownGlobal688808.field_0x24[band] + (g_UnknownGlobal56e26c->ui->field_0x400[bikeClass] - field_0x7f58),
                      __min(value, g_UnknownGlobal56e26c->ui->field_0x324[bikeClass][band]));
    g_UnknownGlobal688808.field_0x24[band] = level;
    int custom = UnknownDropDownList(UnknownFunction46ebf0("DDLCurves", 6))->UnknownFunction4768d0(-1) - 3;
    g_UnknownGlobal56e26c->mode.field_0x10f0[bikeClass][custom][band] = level;
    UnknownFunction4b3bc0();
}

// 0x004b3bc0
void OptGarageDlg::UnknownFunction4b3bc0()
{
    char name[128];
    char text[128];
    int bikeClass = UnknownBikeClassOf(g_UnknownGlobal688808.field_0x00);
    field_0x7f58 = 0;
    int curve = UnknownDropDownList(UnknownFunction46ebf0("DDLCurves", 6))->UnknownFunction4768d0(-1);
    if (curve < 3) {
        UnknownFunction46ebf0("TxtUnused", 0)->UnknownFunction470660(0, 1);
        UnknownFunction46ebf0("HPUnused", 0)->UnknownFunction470660(0, 1);
    } else {
        UnknownFunction46ebf0("TxtUnused", 0)->UnknownFunction470660(1, 1);
        UnknownFunction46ebf0("HPUnused", 0)->UnknownFunction470660(1, 1);
    }
    int* level;
    int i;
    for (i = 1, level = g_UnknownGlobal688808.field_0x24; i - 1 < 11; i++, level++) {
        sprintf(name, "Txt%d", i);
        sprintf(text, "%2.1fk", ((i - 1) * g_UnknownGlobal688808.field_0x58 + g_UnknownGlobal688808.field_0x50) * 0.001f);
        UnknownFunction46ebf0(name, 0)->UnknownFunction470b20(text);
        sprintf(name, "SldEQ%d", i);
        UnknownGameUiControl* slider = UnknownFunction46ebf0(name, 8);
        slider->UnknownInlineScrollBar()->field_0x21c = 1;
        slider->UnknownFunction4754d0(g_UnknownGlobal56e26c->ui->field_0x450[bikeClass] - g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass] + 1);
        slider->UnknownFunction4753c0(*level - g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass],
                                      g_UnknownGlobal56e26c->ui->field_0x450[bikeClass] - g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass]);
        if (curve < 3)
            slider->UnknownVirtualSlot49(0);
        else
            slider->UnknownVirtualSlot49(1);
        field_0x7f58 += *level;
        sprintf(name, "HP%d", i);
        sprintf(text, "%d", *level);
        UnknownFunction46ebf0(name, 0)->UnknownFunction470b20(text);
    }
    sprintf(text, "%d", g_UnknownGlobal56e26c->ui->field_0x400[bikeClass] - field_0x7f58);
    UnknownFunction46ebf0("HPUnused", 0)->UnknownFunction470b20(text);
}

// 0x004b3e70
void OptGarageDlg::UnknownVirtualSlot31(int save)
{
    if (save) {
        g_UnknownGlobal688808.field_0x0c[0] = (unsigned int)UnknownFunction46ebf0("SldFrontComp", 8)->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.field_0x0c[1] = (unsigned int)UnknownFunction46ebf0("SldFrontDecomp", 8)->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.field_0x0c[2] = (unsigned int)UnknownFunction46ebf0("SldRearComp", 8)->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.field_0x0c[3] = (unsigned int)UnknownFunction46ebf0("SldRearDecomp", 8)->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.field_0x0c[4] = (unsigned int)UnknownFunction46ebf0("SldFrontDamp", 8)->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.field_0x0c[5] = (unsigned int)UnknownFunction46ebf0("SldRearDamp", 8)->UnknownFunction475500() * 0.01f;
    } else {
        UnknownFunction46ebf0("SldFrontComp", 8)->UnknownFunction4753c0((int)(g_UnknownGlobal688808.field_0x0c[0] * 100.0f), 100);
        UnknownFunction46ebf0("SldFrontDecomp", 8)->UnknownFunction4753c0((int)(g_UnknownGlobal688808.field_0x0c[1] * 100.0f), 100);
        UnknownFunction46ebf0("SldRearComp", 8)->UnknownFunction4753c0((int)(g_UnknownGlobal688808.field_0x0c[2] * 100.0f), 100);
        UnknownFunction46ebf0("SldRearDecomp", 8)->UnknownFunction4753c0((int)(g_UnknownGlobal688808.field_0x0c[3] * 100.0f), 100);
        UnknownFunction46ebf0("SldFrontDamp", 8)->UnknownFunction4753c0((int)(g_UnknownGlobal688808.field_0x0c[4] * 100.0f), 100);
        UnknownFunction46ebf0("SldRearDamp", 8)->UnknownFunction4753c0((int)(g_UnknownGlobal688808.field_0x0c[5] * 100.0f), 100);
    }
}
