#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OptionProcs.h"

#include "DialogEventKind.h"
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
static inline UIListBox* UnknownDropDownList(UnknownGameUiControl* dropDown)
{
    return static_cast<UIDropDownList*>(dropDown)->listPart;
}


// 0x004b2000
void OptGameSettingsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->kind) {
    case kDialogInit: {
        FindControl("RiderPositionCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.showRiderPositions);
        FindControl("RaceStatsCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.showRaceStats);
        FindControl("GuagesCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.showGauges);
        FindControl("ChkUIAnims", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.uiAnimations);
        FindControl("OverviewCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.showOverview);
        FindControl("RiderNamesCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.showRiderNames);
        FindControl("ChkPodiums", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.showPodiums);
        FindControl("ChkIntroMovie", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.introMovie);
        FindControl("ChkPlayerIndicator", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.playerIndicator);
        FindControl("ChkToolTips", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.toolTips);
        FindControl("ChkWreckResetOnTrack", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688898.wreckResetOnTrack);
        UIMultiState* chat = static_cast<UIMultiState*>(FindControl("MultChatMode", 2));
        chat->SetStateCount(3);
        chat->SetStateTextFromResource(0, g_UnknownGlobal56e26c->field_0x420, 0x140b);
        chat->SetStateTextFromResource(1, g_UnknownGlobal56e26c->field_0x420, 0x140a);
        chat->SetStateTextFromResource(2, g_UnknownGlobal56e26c->field_0x420, 0x140c);
        UnknownGameUiControl* pid = FindControl("TxtPID", 12);
        unsigned long size;
        char value[128];
        char label[128];
        char text[256];
        size = sizeof(value);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13b7, label, 128);
        g_UnknownGlobal56e26c->UnknownVirtualSlot23("PID", "", value, &size);
        sprintf(text, "%s: %s", label, value);
        pid->SetText(text);
        UpdateBoundValues(0);
        break;
    }
    case kDialogCommand:
        if (_stricmp("ChkToolTips", event->controlName) == 0) {
            if (static_cast<UIMultiState*>(event->control)->UnknownFunction4755c0())
                guiManager->UnknownFunction486540(0)->userToolTip->enabled = 1;
            else
                guiManager->UnknownFunction486540(0)->userToolTip->enabled = 0;
        }
        break;
    }
}

// 0x004b22d0
void OptGameSettingsDlg::UnknownVirtualSlot31(int save)
{
    if (save) {
        switch (static_cast<UIMultiState*>(FindControl("MultChatMode", 2))->UnknownFunction4755c0()) {
        case 1:
            g_UnknownGlobal688898.chatMode = 1;
            break;
        case 2:
            g_UnknownGlobal688898.chatMode = 2;
            break;
        default:
            g_UnknownGlobal688898.chatMode = 0;
            break;
        }
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1) {
            if (static_cast<UIMultiState*>(FindControl("ChkToolTips", 2))->UnknownFunction4755c0())
                guiManager->UnknownFunction486540(0)->userToolTip->enabled = 1;
            else
                guiManager->UnknownFunction486540(0)->userToolTip->enabled = 0;
        }
    } else {
        UIMultiState* chat = static_cast<UIMultiState*>(FindControl("MultChatMode", 2));
        switch (g_UnknownGlobal688898.chatMode) {
        case 1:
            chat->SetCurrentState(1);
            break;
        case 2:
            chat->SetCurrentState(2);
            break;
        default:
            chat->SetCurrentState(0);
            break;
        }
    }
}

// 0x004b23b0
void OptGraphicsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        FindControl("ShadowsCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal6887d8.shadows);
        FindControl("ParticlesCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal6887d8.particles);
        FindControl("SkyCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal6887d8.sky);
        FindControl("TerrainQualitySliderBar", 0)->UnknownVirtualSlot54(&g_UnknownGlobal6887d8.terrainQuality);
        UIDropDownList* dropDown = static_cast<UIDropDownList*>(FindControl("DisplayResolutionDropDown", 0));
        dropDown->SetFontColor(0xffffff);
        dropDown->SetTextAlign(0x12);
        UIListBox* list = dropDown->listPart;
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
        list->SelectRowByData(g_UnknownGlobal6887d8.resolutionRow);
        UpdateBoundValues(0);
        break;
    }
    case 3:
        if (_stricmp("DisplayResolutionDropDown", event->controlName) != 0) {
            Sound* sound = FindSectionObject("Ratchet03");
            sound->UnknownFunction4bcbe0(guiManager->field_0x34c, 0);
            if (sound)
                sound->UnknownFunction4bc6b0(0, 0, 0);
        }
        if (_stricmp("TerrainQualitySliderBar", event->controlName) == 0) {
            int value = static_cast<UIScrollBar*>(event->control)->UnknownFunction475500();
            g_UnknownGlobal6887d8.terrainQuality = value;
            g_UnknownGlobal56e26c->mode.field_0x195c[1] = value;
            g_UnknownGlobal56e26c->mode.field_0x195c[3] = value;
            g_UnknownGlobal56e26c->mode.field_0x195c[2] = value;
            g_UnknownGlobal56e26c->mode.field_0x195c[4] = value;
        }
        break;
    case 16:
    case 17:
        if (_stricmp("DisplayResolutionDropDown", event->controlName) != 0) {
            Sound* sound = FindSectionObject("Ratchet03");
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
        g_UnknownGlobal6887d8.resolutionRow = UnknownDropDownList(FindControl("DisplayResolutionDropDown", 6))->GetRowData(-1);
    else
        UnknownDropDownList(FindControl("DisplayResolutionDropDown", 6))->SelectRowByData(g_UnknownGlobal6887d8.resolutionRow);
}

// 0x004b26c0
void OptAdvancedGraphicsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    int* settings = g_UnknownGlobal56e26c->mode.field_0x195c;
    char key[1024];
    switch (event->kind) {
    case kDialogClose: {
        int showPicker = static_cast<UIMultiState*>(FindControl("ChkShowPick3D", 2))->UnknownFunction4755c0();
        g_UnknownGlobal56e26c->UnknownVirtualSlot27("UseLastVideoCard", showPicker == 0);
        int method;
        if (static_cast<UIRadioButton*>(FindControl("RadTerrSingle", 4))->UnknownFunction4755c0())
            method = 2;
        else
            method = static_cast<UIRadioButton*>(FindControl("RadTerrDual", 4))->UnknownFunction4755c0() != 0;
        sprintf(key, "DriverInfo\\%s\\TerrainDetailTextureMethod", g_UnknownGlobal56e26c->field_0x0c->field_0x4bc);
        g_UnknownGlobal56e26c->UnknownVirtualSlot25(key, method);
        g_UnknownGlobal6887d8.terrainQuality = (settings[4] + settings[3] + settings[2] + settings[1]) / 4;
        break;
    }
    case kDialogInit: {
        FindControl("ChkForceVideoMem", 0)->UnknownVirtualSlot54(&settings[0]);
        FindControl("SldEcosystem", 0)->UnknownVirtualSlot54(&settings[1]);
        FindControl("SldObjects", 0)->UnknownVirtualSlot54(&settings[2]);
        FindControl("SldTerrain", 0)->UnknownVirtualSlot54(&settings[3]);
        FindControl("SldVisibility", 0)->UnknownVirtualSlot54(&settings[4]);
        if (g_UnknownGlobal56e26c->field_0x2d0)
            FindControl("Txt3DCard", 12)->SetTextFromResource(g_UnknownGlobal56e26c->field_0x420, 0x1463);
        else
            FindControl("Txt3DCard", 12)->SetText(g_UnknownGlobal56e26c->field_0x0c->field_0x5c0.description);
        int useLast = g_UnknownGlobal56e26c->UnknownVirtualSlot22("UseLastVideoCard", 1);
        static_cast<UIMultiState*>(FindControl("ChkShowPick3D", 2))->SetCurrentState(useLast == 0);
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
            static_cast<UIRadioButton*>(FindControl("RadTerrNone", 4))->SelectInGroup(method);
        else if (method == 1)
            static_cast<UIRadioButton*>(FindControl("RadTerrDual", 4))->SelectInGroup(0);
        else if (method == 2)
            static_cast<UIRadioButton*>(FindControl("RadTerrSingle", 4))->SelectInGroup(0);
        if (!dual)
            FindControl("RadTerrSingle", 4)->UnknownVirtualSlot49(dual);
        static_cast<UIMultiState*>(FindControl("ChkFilterInput", 2))->SetCurrentState(
            g_UnknownGlobal56e26c->UnknownVirtualSlot22("JoystickFilter", 0));
        static_cast<UIMultiState*>(FindControl("ChkShowPickController", 2))->SetCurrentState(
            g_UnknownGlobal56e26c->UnknownVirtualSlot22("UseLastController", 0) == 0);
        UpdateBoundValues(0);
        break;
    }
    case kDialogCommand:
        if (_stricmp("Back", event->controlName) == 0) {
            EndDialog(0x52);
        } else if (_stricmp("ChkFilterInput", event->controlName) == 0) {
            int filter = static_cast<UIMultiState*>(event->control)->UnknownFunction4755c0();
            g_UnknownGlobal56e26c->UnknownVirtualSlot27("JoystickFilter", filter);
            if (g_UnknownGlobal56e26c->field_0x14->activeJoystick)
                ((PCJoystickDevice*)g_UnknownGlobal56e26c->field_0x14->activeJoystick)->UnknownMethod4c3ae0(filter);
        } else if (_stricmp("ChkShowPickController", event->controlName) == 0) {
            int useLast = static_cast<UIMultiState*>(event->control)->UnknownFunction4755c0() == 0;
            g_UnknownGlobal56e26c->UnknownVirtualSlot27("UseLastController", useLast);
            g_UnknownGlobal56e26c->UnknownVirtualSlot27("PresetSelected", 0);
        } else if (_stricmp("Help", event->controlName) == 0) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523d30("MCM2HELP", 0);
        }
        break;
    }
}

// 0x004b2b20
void OptSoundDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->kind) {
    case kDialogInit:
        FindControl("InGameSoundEffectsCheck", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.inGameSoundEffects);
        FindControl("SoundHardwareCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.soundHardware);
        FindControl("UserInterfaceSoundsCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.uiSounds);
        FindControl("ChkEAX", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.eax);
        FindControl("ChkWaypointChime", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.waypointChime);
        FindControl("UserInterfaceVolumeSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.uiVolume);
        FindControl("InGameVolumeSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688798.inGameVolume);
        FindControl("Rad16Bit", 4)->UnknownVirtualSlot49(0);
        static_cast<UIRadioButton*>(FindControl("Rad8Bit", 4))->SelectInGroup(0);
        UpdateBoundValues(0);
        break;
    case 3: {
        Sound* sound = FindSectionObject("Ratchet03");
        sound->UnknownFunction4bcbe0(guiManager->field_0x34c, 0);
        if (sound)
            sound->UnknownFunction4bc6b0(0, 0, 0);
        ApplySoundSettings();
        break;
    }
    case 16:
    case 17: {
        Sound* sound = FindSectionObject("Ratchet03");
        if (sound)
            sound->UnknownFunction4bc940(0);
    }
        // fall through
    case kDialogCommand:
        ApplySoundSettings();
        break;
    }
}

// 0x004b2ce0
void OptSoundDlg::UnknownVirtualSlot31(int save)
{
    if (save) {
        g_UnknownGlobal688798.sound16Bit = static_cast<UIRadioButton*>(FindControl("Rad16Bit", 4))->GetGroupSelection() - 1;
    } else if (g_UnknownGlobal688798.sound16Bit) {
        static_cast<UIRadioButton*>(FindControl("Rad16Bit", 4))->SelectInGroup(0);
    } else {
        static_cast<UIRadioButton*>(FindControl("Rad8Bit", 4))->SelectInGroup(0);
    }
}

// 0x004b2d40
void OptSoundDlg::ApplySoundSettings()
{
    UpdateBoundValues(1);
    if (g_UnknownGlobal56e26c->field_0x04) {
        ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)->field_0x45c_bit0 = g_UnknownGlobal688798.soundHardware;
        ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)->field_0x45c_bit3 = g_UnknownGlobal688798.eax;
        ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)->UnknownFunction4be910(22050, 1, g_UnknownGlobal688798.sound16Bit ? 16 : 8);
    }
    guiManager->field_0x34c = (g_UnknownGlobal688798.uiSounds ? (g_UnknownGlobal688798.uiVolume - 100) * 25 : -10000) - 200;
}


// 0x004b2e00
void OptControlsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    char text[128];
    char path[260];
    int i;
    switch (event->kind) {
    case kDialogInit: {
        waitingForInput = 0;
        field_0x7f74 = 0;
        field_0x7f78 = 0;
        FindControl("ForceFeedCheck", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.forceFeedback);
        FindControl("SteeringSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.steering);
        FindControl("CrossupSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.crossup);
        FindControl("PitchSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.pitch);
        FindControl("PressThrottleSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.pressThrottle);
        FindControl("PressBrakesSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.pressBrakes);
        FindControl("ReleaseThrottleSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.releaseThrottle);
        FindControl("ReleaseBrakesSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.releaseBrakes);
        FindControl("FeedbackSlider", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.feedback);
        FindControl("ChkGasGyro", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.gasGyro);
        FindControl("ChkBrakeGyro", 0)->UnknownVirtualSlot54(&g_UnknownGlobal688868.brakeGyro);
        UIListBox* actions = static_cast<UIListBox*>(FindControl("ActionListBox", 0));
        for (i = 0; i < 14; i++) {
            g_UnknownGlobal56e26c->UnknownFunction521970(0x192 + i, text, 128);
            actions->UnknownFunction476d80(text, i, 0);
        }
        UIListBox* devices = UnknownDropDownList(FindControl("InputDeviceDDL", 6));
        devices->RemoveAllRows();
        for (i = 0; i < 8; i++) {
            g_UnknownGlobal56e26c->UnknownFunction521970(0x1cc + i, text, 128);
            devices->UnknownFunction476d80(text, i, 0);
        }
        ListMappedKeys();
        UIListBox* keys = static_cast<UIListBox*>(FindControl("MapKeyListBox", 0));
        for (i = 0; i < 14; i++) {
            g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449350(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, i, text);
            keys->UnknownFunction476d80(text, i, 0);
        }
        ColorListRows();
        UnknownGameUiControl* analog = FindControl("ChkAnalogAxes", 0);
        analog->UnknownVirtualSlot54(&g_UnknownGlobal688868.analogAxes);
        if (g_UnknownGlobal56e26c->field_0x14->activeJoystick)
            g_UnknownGlobal56e26c->field_0x14->activeJoystick->UnknownFunction4897e0(4);
        analog->Show(0, 1);
        field_0x7f78 = 0;
        if (g_UnknownGlobal56e26c->field_0x14->activeJoystick)
            field_0x7f78 = g_UnknownGlobal56e26c->field_0x14->activeJoystick->deviceKind == 3;
        ShowGroup(0x12f, field_0x7f78);
        UpdateBoundValues(0);
        field_0x7f74 = 0;
        UpdateBoundValues(0);
        break;
    }
    case 11:
    case 12:
        event->field_0x18 = 0;
        break;
    case 3:
        if (_stricmp("InputDeviceDDL", event->controlName) != 0 && _stricmp("ListboxScroll", event->controlName) != 0) {
            Sound* sound = FindSectionObject("Ratchet03");
            sound->UnknownFunction4bcbe0(guiManager->field_0x34c, 0);
            if (sound)
                sound->UnknownFunction4bc6b0(0, 0, 0);
        }
        break;
    case 16:
    case 17:
        if (_stricmp("InputDeviceDDL", event->controlName) != 0 && _stricmp("ListboxScroll", event->controlName) != 0) {
            Sound* sound = FindSectionObject("Ratchet03");
            if (sound)
                sound->UnknownFunction4bc940(0);
        }
        break;
    case kDialogListSelect:
        if (waitingForInput || field_0x7f74)
            break;
        if (_stricmp("InputDeviceDDL", event->controlName) == 0) {
            g_UnknownGlobal56e26c->field_0x33fc->field_0x00 =
                UnknownDropDownList(FindControl("InputDeviceDDL", 6))->GetRowData(-1);
            ListMappedKeys();
        } else if (_stricmp("MapKeyListBox", event->controlName) == 0 || _stricmp("ActionListBox", event->controlName) == 0) {
            int row = static_cast<UIListBox*>(event->control)->GetSelectedRow();
            UIListBox* keys = static_cast<UIListBox*>(FindControl("MapKeyListBox", 3));
            g_UnknownGlobal56e26c->UnknownFunction521970(0x1c2, text, 128);
            keys->SetRowText(row, text);
            EnableGroup(0x65, 0);
            EnableGroup(0xca, 0);
            EnableGroup(0x12f, 0);
            if (g_UnknownGlobal56e26c->field_0x14->activeJoystick) {
                for (i = 0; i < 6; i++)
                    axisValuesAtWait[i] = g_UnknownGlobal56e26c->field_0x14->activeJoystick->UnknownFunction489e20(i);
            }
            field_0x7f74 = 1;
            AddTimer(0, 0xfa, 0);
        }
        break;
    case kDialogTimer:
        field_0x7f74 = 0;
        if (!waitingForInput)
            waitingForInput = 1;
        MapMovedInput();
        break;
    case kDialogClose:
        sprintf(path, "%s\\%s\\%s", "ui\\profile", g_UnknownGlobal56e26c->mode.field_0x00, "control.ctl");
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448e90(path, -1);
        break;
    }
}

// 0x004b3410
void OptControlsDlg::ListMappedKeys()
{
    char text[128];
    UIListBox* keys = static_cast<UIListBox*>(FindControl("MapKeyListBox", 3));
    keys->RemoveAllRows();
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
                UnknownDropDownList(FindControl("InputDeviceDDL", 6))->GetRowData(-1);
    } else if (g_UnknownGlobal56e26c->field_0x33fc) {
        UnknownDropDownList(FindControl("InputDeviceDDL", 6))->SelectRowByData(g_UnknownGlobal56e26c->field_0x33fc->field_0x00);
        ListMappedKeys();
    }
}

// 0x004b54c0
void OptControlsDlg::ColorListRows()
{
    UIListBox* list;
    int i;
    if (g_UnknownGlobal56e26c->field_0x33fc->field_0x00 == 1) {
        list = static_cast<UIListBox*>(FindControl("MapKeyListBox", 3));
        for (i = 0; i < 4; i++)
            list->SetItemBoxColor(0x5555ff, i);
        list = static_cast<UIListBox*>(FindControl("ActionListBox", 3));
        for (i = 0; i < 4; i++)
            list->SetItemBoxColor(0x5555ff, i);
    } else {
        list = static_cast<UIListBox*>(FindControl("MapKeyListBox", 3));
        for (i = 0; i < 4; i++)
            list->SetItemBoxColor(0xff000000, i);
        list = static_cast<UIListBox*>(FindControl("ActionListBox", 3));
        for (i = 0; i < 4; i++)
            list->SetItemBoxColor(0xff000000, i);
    }
}

// 0x004b5570
int GetInputText(int row, int kind, int code, char* text)
{
    strcpy(text, "");
    if (row < 4 && kind != 0)
        return 0;
    return g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449380(kind, code, text);
}

// 0x004b55d0
int OptControlsDlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    if (!waitingForInput)
        return GameObject::UnknownVirtualSlot23(event, entry);
    return 1;
}

// 0x004b5a20
void OptControlsDlg::EndInputWait()
{
    waitingForInput = 0;
    RemoveTimers(0);
    EnableGroup(0x65, 1);
    EnableGroup(0xca, 1);
    EnableGroup(0x12f, 1);
}

// 0x004b40a0
void OptMessagesDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    if (event->kind == kDialogInit) {
        static_cast<UIEditBox*>(FindControl("Message1", 11))->BindTextBuffer(g_UnknownGlobal688c10[1], 128);
        static_cast<UIEditBox*>(FindControl("Message2", 11))->BindTextBuffer(g_UnknownGlobal688c10[2], 128);
        static_cast<UIEditBox*>(FindControl("Message3", 11))->BindTextBuffer(g_UnknownGlobal688c10[3], 128);
        static_cast<UIEditBox*>(FindControl("Message4", 11))->BindTextBuffer(g_UnknownGlobal688c10[4], 128);
        static_cast<UIEditBox*>(FindControl("Message5", 11))->BindTextBuffer(g_UnknownGlobal688c10[5], 128);
        static_cast<UIEditBox*>(FindControl("Message6", 11))->BindTextBuffer(g_UnknownGlobal688c10[6], 128);
        static_cast<UIEditBox*>(FindControl("Message7", 11))->BindTextBuffer(g_UnknownGlobal688c10[7], 128);
        static_cast<UIEditBox*>(FindControl("Message8", 11))->BindTextBuffer(g_UnknownGlobal688c10[8], 128);
        static_cast<UIEditBox*>(FindControl("Message9", 11))->BindTextBuffer(g_UnknownGlobal688c10[9], 128);
        static_cast<UIEditBox*>(FindControl("Message0", 11))->BindTextBuffer(g_UnknownGlobal688c10[0], 128);
        UpdateBoundValues(0);
        GameObjectIterator controls(controlContainer, 1, "UIControl");
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
    switch (event->kind) {
    case kDialogInit: {
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
        UIMultiState* tab = static_cast<UIMultiState*>(FindControl("GameSettings", 4));
        tab->SetStateTextFromResource(0, g_UnknownGlobal56e26c->field_0x420, 0x13dd);
        tab->SetStateTextFromResource(1, g_UnknownGlobal56e26c->field_0x420, 0x13dd);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIMultiState*>(FindControl("Graphics", 4));
        tab->SetStateTextFromResource(0, g_UnknownGlobal56e26c->field_0x420, 0x13de);
        tab->SetStateTextFromResource(1, g_UnknownGlobal56e26c->field_0x420, 0x13de);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIMultiState*>(FindControl("Sound", 4));
        tab->SetStateTextFromResource(0, g_UnknownGlobal56e26c->field_0x420, 0x13df);
        tab->SetStateTextFromResource(1, g_UnknownGlobal56e26c->field_0x420, 0x13df);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIMultiState*>(FindControl("Controls", 4));
        tab->SetStateTextFromResource(0, g_UnknownGlobal56e26c->field_0x420, 0x13e0);
        tab->SetStateTextFromResource(1, g_UnknownGlobal56e26c->field_0x420, 0x13e0);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIMultiState*>(FindControl("Messages", 4));
        tab->SetStateTextFromResource(0, g_UnknownGlobal56e26c->field_0x420, 0x13e1);
        tab->SetStateTextFromResource(1, g_UnknownGlobal56e26c->field_0x420, 0x13e1);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIMultiState*>(FindControl("Garage", 4));
        tab->SetStateTextFromResource(0, g_UnknownGlobal56e26c->field_0x420, 0x13e2);
        tab->SetStateTextFromResource(1, g_UnknownGlobal56e26c->field_0x420, 0x13e2);
        tab->SetTextAlign(0x22);
        if (event->field_0x18 == 1) {
            OpenPage(3);
            static_cast<UIRadioButton*>(FindControl("Controls", 4))->SelectInGroup(0);
        } else if (event->field_0x18 == 2) {
            OpenPage(5);
            static_cast<UIRadioButton*>(FindControl("Garage", 4))->SelectInGroup(0);
        } else {
            OpenPage(0);
            static_cast<UIRadioButton*>(FindControl("GameSettings", 4))->SelectInGroup(0);
        }
        if (g_UnknownGlobal56e26c->UnknownFunction521cd0())
            FindControl("Restore", 0)->Show(0, 1);
        break;
    }
    case kDialogCommand:
        if (_stricmp("Back", event->controlName) == 0) {
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
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (_stricmp("GameSettings", event->controlName) == 0) {
            OpenPage(0);
        } else if (_stricmp("Graphics", event->controlName) == 0) {
            OpenPage(1);
        } else if (_stricmp("Sound", event->controlName) == 0) {
            OpenPage(2);
        } else if (_stricmp("Controls", event->controlName) == 0) {
            OpenPage(3);
        } else if (_stricmp("Messages", event->controlName) == 0) {
            OpenPage(4);
        } else if (_stricmp("Garage", event->controlName) == 0) {
            OpenPage(5);
        } else if (_stricmp("Restore", event->controlName) == 0) {
            GlobalSettingsDlg* dialog = new(__FILE__, 979) GlobalSettingsDlg;
            event->gui->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 0xc, 0,
                                                     (UnknownGuiDialog*)this, 0, 0, 1);
        } else if (_stricmp("Help", event->controlName) == 0) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523d30("MCM2HELP", 0);
        } else if (_stricmp("Advanced", event->controlName) == 0) {
            if (field_0x7f5c)
                field_0x7f5c->UpdateBoundValues(1);
            OptAdvancedGraphicsDlg* dialog = new(__FILE__, 987) OptAdvancedGraphicsDlg;
            event->gui->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 0xc, 0,
                                                     (UnknownGuiDialog*)this, 0, 0, 1);
        }
        break;
    case 9:
        if (event->code == 0x51) {
            TrackGame* game = g_UnknownGlobal56e26c;
            g_UnknownGlobal688808 = *UnknownGarageSettingsOf(game);
            g_UnknownGlobal688898 = *UnknownGameSettingsOf(game);
            g_UnknownGlobal688798 = *UnknownSoundSettingsOf(game);
            g_UnknownGlobal6887d8 = *UnknownGraphicsSettingsOf(game);
            g_UnknownGlobal688868 = *UnknownControlSettingsOf(game);
            memcpy(g_UnknownGlobal688c10, UnknownMessagesOf(game), sizeof(g_UnknownGlobal688c10));
            if (field_0x7f58)
                field_0x7f58->UpdateBoundValues(0);
            if (field_0x7f5c)
                field_0x7f5c->UpdateBoundValues(0);
            if (field_0x7f60) {
                field_0x7f60->UpdateBoundValues(0);
                field_0x7f60->ApplySoundSettings();
            }
            if (field_0x7f64)
                field_0x7f64->UpdateBoundValues(0);
            if (field_0x7f68)
                field_0x7f68->UpdateBoundValues(0);
            if (field_0x7f6c) {
                field_0x7f6c->UpdateBoundValues(0);
                field_0x7f6c->FillCurveList();
                field_0x7f6c->ShowCurve();
            }
            KrustyUI* ui = g_UnknownGlobal56e26c->ui;
            int count = ui->field_0x54;
            for (int i = 0; i < count; i++) {
                if (((UnknownOptKrustyUIRecord*)ui->field_0x50)[i].field_0x8c == 0xfa) {
                    g_UnknownGlobal56e26c->mode.field_0x1974.field_0xc4 = i; // TrackGame+0x1fb0
                    break;
                }
            }
            NotifyParent(0x51, 9);
        } else if (event->code == 0x52) {
            if (field_0x7f5c)
                field_0x7f5c->UpdateBoundValues(0);
        }
        break;
    case kDialogClose:
        *UnknownGarageSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688808;
        *UnknownGameSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688898;
        *UnknownSoundSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688798;
        *UnknownGraphicsSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal6887d8;
        *UnknownControlSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688868;
        memcpy(UnknownMessagesOf(g_UnknownGlobal56e26c), g_UnknownGlobal688c10, sizeof(g_UnknownGlobal688c10));
        guiManager->field_0xec = g_UnknownGlobal688898.uiAnimations;
        if (parentDialog)
            parentDialog->field_0xc8 = g_UnknownGlobal688898.uiAnimations;
        break;
    }
}

// 0x004b4ab0
void OptionsDlg::OpenPage(int page)
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
    switch (event->kind) {
    case 9:
        if (event->code == 0x60) {
            char name[128];
            char profile[260];
            char preset[260];
            int choice = static_cast<UIRadioButton*>(FindControl("ButFactory", 4))->GetGroupSelection();
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
                UIListBox* profiles = static_cast<UIListBox*>(FindControl("LstProfiles", 3));
                UnknownGameUiListRow* entry = &profiles->rowTable[profiles->GetSelectedRow()];
                strcpy(g_UnknownGlobal56e26c->mode.field_0x00, entry->text);
                g_UnknownGlobal56e26c->mode.UnknownFunction5231f0();
            }
            strcpy(g_UnknownGlobal56e26c->mode.field_0x00, name);
            EndDialog(0x51);
        } else if (event->code == 0x61) {
            EndDialog(0);
        }
        break;
    case kDialogInit:
        ListOtherProfiles();
        FindControl("LstProfiles", 3)->Show(0, 1);
        break;
    case kDialogCommand:
        if (_stricmp("ButFactory", event->controlName) == 0) {
            FindControl("LstProfiles", 3)->Show(0, 1);
        } else if (_stricmp("ButCopyUser", event->controlName) == 0) {
            FindControl("LstProfiles", 3)->Show(1, 1);
        } else if (_stricmp("ButCancel", event->controlName) == 0) {
            EndDialog(0);
        } else if (_stricmp("ButLoad", event->controlName) == 0) {
            ConfirmRestoreDlg* dialog = new(__FILE__, 1123) ConfirmRestoreDlg;
            guiManager->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 0xc, 0,
                                              (UnknownGuiDialog*)this, 0, 0, 1);
        }
        break;
    }
}

// 0x004b51a0
void GlobalSettingsDlg::ListOtherProfiles()
{
    char name[260];
    int count = 0;
    UIListBox* profiles = static_cast<UIListBox*>(FindControl("LstProfiles", 0));
    profiles->RemoveAllRows();
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
    profiles->SetRowTextColor(0xfeb97a, -1);
    profiles->SetSelectColor(0xffffff);
    profiles->SetSelectBoxColor(0xfeb97a);
    if (count) {
        profiles->SelectRowByText(g_UnknownGlobal56e26c->mode.field_0x00);
    } else {
        UnknownGameUiControl* copy = FindControl("ButCopyUser", 4);
        copy->UnknownVirtualSlot49(0);
        copy->SetFontColor(0x757575);
    }
}

// 0x004b5380
void ConfirmRestoreDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->kind) {
    case kDialogInit: {
        UnknownGameUiControl* control = FindControl("ButLeft", 0);
        control->SetTextFromResource(g_UnknownGlobal56e26c->field_0x420, 0x13e9);
        control = FindControl("ButRight", 0);
        control->SetTextFromResource(g_UnknownGlobal56e26c->field_0x420, 0x13dc);
        control = FindControl("ButMiddle", 1);
        control->Show(0, 1);
        control = FindControl("TitleText", 12);
        control->SetTextFromResource(g_UnknownGlobal56e26c->field_0x420, 0x14c9);
        UnknownGameUiControl* prompt = FindControl("TxtPrompt", 12);
        prompt->field_0x1e8 = 1;
        prompt->SetTextFromResource(g_UnknownGlobal56e26c->field_0x420, 0x14ca);
        break;
    }
    case kDialogCommand:
        if (_stricmp("ButLeft", event->controlName) == 0) {
            event->dialog->EndDialog(0x60);
            event->handled = 1;
        } else if (_stricmp("ButRight", event->controlName) == 0) {
            event->dialog->EndDialog(0x61);
            event->handled = 1;
        }
        break;
    }
}

// 0x004b3500
void OptGarageDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    switch (event->kind) {
    case kDialogInit: {
        FindControl("TxtUnused", 0)->field_0x1e8 = 1;
        FillCurveList();
        ShowCurve();
        UpdateBoundValues(0);
        GameObjectIterator controls(controlContainer, 1, "UIControl");
        GameObject* control;
        while ((control = controls.Next()) != 0)
            ((UnknownGameUiControl*)control)->UnknownVirtualSlot49(0);
        break;
    }
    case kDialogCommand:
        if (_stricmp("CurveLeft", event->controlName) == 0) {
            UIListBox* curves = UnknownDropDownList(FindControl("DDLCurves", 6));
            int rows = curves->rowCount;
            curves->SelectRow((curves->GetSelectedRow() + rows - 1) % rows);
            curves->UnknownVirtualSlot66(0);
        } else if (_stricmp("CurveRight", event->controlName) == 0) {
            UIListBox* curves = UnknownDropDownList(FindControl("DDLCurves", 6));
            int rows = curves->rowCount;
            curves->SelectRow((curves->GetSelectedRow() + 1) % rows);
            curves->UnknownVirtualSlot66(0);
        }
        break;
    case kDialogListSelect:
        if (_stricmp("DDLCurves", event->controlName) == 0) {
            int curve = static_cast<UIListBox*>(event->control)->GetRowData(-1);
            g_UnknownGlobal688898.curveRows[UnknownBikeClassOf(g_UnknownGlobal688808.engineSize)] = curve;
            TrackGame* game = g_UnknownGlobal56e26c;
            for (int band = 0; band < 11; band++) {
                if (curve < 3)
                    g_UnknownGlobal688808.eqBands[band] =
                        game->ui->field_0x68[UnknownBikeClassOf(g_UnknownGlobal688808.engineSize)][curve][band];
                else
                    g_UnknownGlobal688808.eqBands[band] =
                        game->mode.field_0x10f0[UnknownBikeClassOf(g_UnknownGlobal688808.engineSize)][curve - 3][band];
            }
            ShowCurve();
        }
        break;
    case 3:
        if (_stricmp("DDLCurves", event->controlName) != 0) {
            Sound* sound = FindSectionObject("Ratchet03");
            sound->UnknownFunction4bcbe0(guiManager->field_0x34c, 0);
            if (sound)
                sound->UnknownFunction4bc6b0(0, 0, 0);
            if (strlen(event->controlName) > 5 && _strnicmp(event->controlName, "SldEQ", 5) == 0)
                EqSliderMoved(event->control);
        }
        break;
    case 16:
    case 17:
        if (_stricmp("DDLCurves", event->controlName) != 0) {
            Sound* sound = FindSectionObject("Ratchet03");
            if (sound)
                sound->UnknownFunction4bc940(0);
            if (strlen(event->controlName) > 5 && _strnicmp(event->controlName, "SldEQ", 5) == 0)
                EqSliderMoved(event->control);
        }
        break;
    }
}

// 0x004b38f0
void OptGarageDlg::FillCurveList()
{
    char format[32];
    char text[128];
    char engine[128];
    char standard[128];
    char custom[128];
    g_UnknownGlobal56e26c->UnknownFunction521970(0x146a, format, 127);
    sprintf(engine, format, g_UnknownGlobal56cb6c[UnknownBikeClassOf(g_UnknownGlobal688808.engineSize)]);
    g_UnknownGlobal56e26c->UnknownFunction521970(0x146b, standard, 127);
    g_UnknownGlobal56e26c->UnknownFunction521970(0x146c, custom, 127);
    UIListBox* curves = UnknownDropDownList(FindControl("DDLCurves", 6));
    curves->RemoveAllRows();
    for (int i = 0; i < 6; i++) {
        if (i < 3)
            sprintf(text, "%dcc (%s) %s #%d", g_UnknownGlobal688808.engineSize, engine, standard, i + 1);
        else
            sprintf(text, "%dcc (%s) %s #%d", g_UnknownGlobal688808.engineSize, engine, custom, i - 2);
        curves->UnknownFunction476d80(text, i, 0);
    }
    curves->SelectRowByData(g_UnknownGlobal688898.curveRows[UnknownBikeClassOf(g_UnknownGlobal688808.engineSize)]);
}

// 0x004b3aa0
void OptGarageDlg::EqSliderMoved(UnknownGameUiControl* slider)
{
    int bikeClass = UnknownBikeClassOf(g_UnknownGlobal688808.engineSize);
    int band = slider->attachId;
    int value = static_cast<UIScrollBar*>(slider)->UnknownFunction475300(g_UnknownGlobal56e26c->ui->field_0x450[bikeClass] - g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass]);
    value += g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass];
    int level = __min(g_UnknownGlobal688808.eqBands[band] + (g_UnknownGlobal56e26c->ui->field_0x400[bikeClass] - curveSum),
                      __min(value, g_UnknownGlobal56e26c->ui->field_0x324[bikeClass][band]));
    g_UnknownGlobal688808.eqBands[band] = level;
    int custom = UnknownDropDownList(FindControl("DDLCurves", 6))->GetRowData(-1) - 3;
    g_UnknownGlobal56e26c->mode.field_0x10f0[bikeClass][custom][band] = level;
    ShowCurve();
}

// 0x004b3bc0
void OptGarageDlg::ShowCurve()
{
    char name[128];
    char text[128];
    int bikeClass = UnknownBikeClassOf(g_UnknownGlobal688808.engineSize);
    curveSum = 0;
    int curve = UnknownDropDownList(FindControl("DDLCurves", 6))->GetRowData(-1);
    if (curve < 3) {
        FindControl("TxtUnused", 0)->Show(0, 1);
        FindControl("HPUnused", 0)->Show(0, 1);
    } else {
        FindControl("TxtUnused", 0)->Show(1, 1);
        FindControl("HPUnused", 0)->Show(1, 1);
    }
    int* level;
    int i;
    for (i = 1, level = g_UnknownGlobal688808.eqBands; i - 1 < 11; i++, level++) {
        sprintf(name, "Txt%d", i);
        sprintf(text, "%2.1fk", ((i - 1) * g_UnknownGlobal688808.bandStep + g_UnknownGlobal688808.firstBand) * 0.001f);
        FindControl(name, 0)->SetText(text);
        sprintf(name, "SldEQ%d", i);
        UIScrollBar* slider = static_cast<UIScrollBar*>(FindControl(name, 8));
        slider->field_0x21c = 1;
        slider->UnknownFunction4754d0(g_UnknownGlobal56e26c->ui->field_0x450[bikeClass] - g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass] + 1);
        slider->UnknownFunction4753c0(*level - g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass],
                                      g_UnknownGlobal56e26c->ui->field_0x450[bikeClass] - g_UnknownGlobal56e26c->ui->field_0x43c[bikeClass]);
        if (curve < 3)
            slider->UnknownVirtualSlot49(0);
        else
            slider->UnknownVirtualSlot49(1);
        curveSum += *level;
        sprintf(name, "HP%d", i);
        sprintf(text, "%d", *level);
        FindControl(name, 0)->SetText(text);
    }
    sprintf(text, "%d", g_UnknownGlobal56e26c->ui->field_0x400[bikeClass] - curveSum);
    FindControl("HPUnused", 0)->SetText(text);
}

// 0x004b3e70
void OptGarageDlg::UnknownVirtualSlot31(int save)
{
    if (save) {
        g_UnknownGlobal688808.suspension[0] = (unsigned int)static_cast<UIScrollBar*>(FindControl("SldFrontComp", 8))->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.suspension[1] = (unsigned int)static_cast<UIScrollBar*>(FindControl("SldFrontDecomp", 8))->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.suspension[2] = (unsigned int)static_cast<UIScrollBar*>(FindControl("SldRearComp", 8))->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.suspension[3] = (unsigned int)static_cast<UIScrollBar*>(FindControl("SldRearDecomp", 8))->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.suspension[4] = (unsigned int)static_cast<UIScrollBar*>(FindControl("SldFrontDamp", 8))->UnknownFunction475500() * 0.01f;
        g_UnknownGlobal688808.suspension[5] = (unsigned int)static_cast<UIScrollBar*>(FindControl("SldRearDamp", 8))->UnknownFunction475500() * 0.01f;
    } else {
        static_cast<UIScrollBar*>(FindControl("SldFrontComp", 8))->UnknownFunction4753c0((int)(g_UnknownGlobal688808.suspension[0] * 100.0f), 100);
        static_cast<UIScrollBar*>(FindControl("SldFrontDecomp", 8))->UnknownFunction4753c0((int)(g_UnknownGlobal688808.suspension[1] * 100.0f), 100);
        static_cast<UIScrollBar*>(FindControl("SldRearComp", 8))->UnknownFunction4753c0((int)(g_UnknownGlobal688808.suspension[2] * 100.0f), 100);
        static_cast<UIScrollBar*>(FindControl("SldRearDecomp", 8))->UnknownFunction4753c0((int)(g_UnknownGlobal688808.suspension[3] * 100.0f), 100);
        static_cast<UIScrollBar*>(FindControl("SldFrontDamp", 8))->UnknownFunction4753c0((int)(g_UnknownGlobal688808.suspension[4] * 100.0f), 100);
        static_cast<UIScrollBar*>(FindControl("SldRearDamp", 8))->UnknownFunction4753c0((int)(g_UnknownGlobal688808.suspension[5] * 100.0f), 100);
    }
}
