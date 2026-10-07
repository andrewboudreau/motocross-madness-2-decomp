#include "InGameProcs.h"

#include "DialogEventKind.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "FollowCamera.h"
#include "Net.h"
#include "TrackGame.h"

// The 0x14-byte message 0x89 that a network player sends when it leaves
// (EventManager slot 24 marks the sender done).
struct UnknownPlayerLeftMessage {
    int field_0x00;
    char field_0x04[16];                      // player name
};

// 0x004886e0
void ExitDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ButMiddle", event->controlName)) {
            guiManager->ShowCursors(0);
            if (!g_TrackGame->uiInteractionBlocked) {
                UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
                if (view && !view->field_0x38->field_0x7a4) {
                    view->UnknownFunction420650(1, 0, 0);
                    g_TrackGame->SetMenuOpen(0, 0x191, 1);
                }
            }
        } else if (!_stricmp("ButLeft", event->controlName)) {
            UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
            if (view && g_TrackGame->mode.field_0x27f8.field_0x00 == 4 && view->field_0x1e8 > 0) {
                ChoiceDlg* dialog = new(__FILE__, 66) ChoiceDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0,
                                                  (UIDialog*)this, 0, 0, 1);
                dialog->SetTextsOrResources(0, 0x1440, 0, 0, 0, 0x143e, 0, 0, 0, 0x143d);
            } else {
                if (g_TrackGame->network) {
                    char name[128];
                    UnknownPlayerLeftMessage message;
                    NetworkInterface* network = g_TrackGame->network;
                    int player = network->localPlayer;
                    network->GetPlayerName(player, name);
                    if (strcmp(name, "") != 0) {
                        int length = strlen(name);
                        int count = length > 15 ? 15 : length;
                        strncpy(message.field_0x04, name, count);
                        message.field_0x04[count] = 0;
                        g_TrackGame->network->Send(
                            0x89, &message, sizeof(message), player, 0);
                    }
                }
                guiManager->ShowCursors(0);
                UnknownFunction4526b0(0x191, event);
            }
        } else if (!_stricmp("ButRight", event->controlName)) {
            guiManager->ShowCursors(0);
            UnknownFunction452930(0x191, event);
            g_TrackGame->UnknownFunction468880();
        }
        break;
    case kDialogInit: {
        UIControl* control = FindControl("TitleText", 12);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13d9);
        control = FindControl("ButMiddle", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13da);
        UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
        if (g_TrackGame->network || view->field_0x38->field_0x7a4 ||
            g_TrackGame->mode.field_0x27f8.field_0x00 == 4)
            control->Show(0, 1);
        control = FindControl("ButLeft", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13db);
        control = FindControl("ButRight", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13dc);
        guiManager->ShowCursors(1);
        break;
    }
    case 9:
        if (event->code == 0x65) {
            g_TrackGame->mode.field_0x26f0 = 1;
            guiManager->ShowCursors(0);
            UnknownFunction4526b0(0x191, event);
        } else if (event->code == 0x67) {
            g_TrackGame->mode.field_0x26f0 = 0;
            guiManager->ShowCursors(0);
            UnknownFunction4526b0(0x191, event);
        }
        break;
    }
}

// 0x00488ad0
void ContinueDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ButMiddle", event->controlName)) {
            guiManager->ShowCursors(0);
            UnknownFunction453090(0x190, event);
            g_TrackGame->UnknownFunction468880();
        }
        break;
    case kDialogInit: {
        UIControl* control = FindControl("TitleText", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x8ff);
        control = FindControl("ButMiddle", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x916);
        control->keyBind = 0x1c;
        control = FindControl("ButLeft", 1);
        control->Show(0, 1);
        control->keyBind = 0;
        FindControl("ButRight", 0)->Show(0, 1);
        break;
    }
    }
}

// 0x00488bd0
void FormatTime(char* text, float seconds) {
    if (seconds == 0.0f || seconds == FLT_MAX) {
        strcpy(text, "00:00:00.00");
        return;
    }
    int hours = (int)(seconds / 3600.0f);
    int minutes = (int)(seconds / 60.0f);
    sprintf(text, "%02d:%02d:%05.2f", hours, minutes, seconds - hours * 3600 - minutes * 60);
}

// 0x00488c90
void VCRDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
    switch (event->kind) {
    case kDialogInit:
        field_0x7f58 = 0;
        static_cast<UIScrollBar*>(FindControl("SliderTime", 8))->UnknownFunction4751c0(1000);
        FindControl("ButSave", 0)->Show(0, 1);
        if (!g_TrackGame->GetRegistryFlag("AllowVCRControls", 0)) {
            FindControl("TogFF", 0)->Show(0, 1);
            FindControl("TogFFF", 0)->Show(0, 1);
            FindControl("TogReverse", 0)->Show(0, 1);
            FindControl("ButBeginning", 0)->Show(0, 1);
            FindControl("ButEnd", 0)->Show(0, 1);
            FindControl("ButFrame", 0)->Show(0, 1);
            FindControl("TogFF", 0)->UnknownVirtualSlot49(0);
            FindControl("TogFFF", 0)->UnknownVirtualSlot49(0);
            FindControl("TogReverse", 0)->UnknownVirtualSlot49(0);
            FindControl("ButBeginning", 0)->UnknownVirtualSlot49(0);
            FindControl("ButEnd", 0)->UnknownVirtualSlot49(0);
            FindControl("SliderTime", 8)->UnknownVirtualSlot49(0);
            FindControl("ButFrame", 0)->UnknownVirtualSlot49(0);
        }
        break;
    case kDialogCommand:
        if (!_stricmp("TogPlay", event->controlName)) {
            if (view->field_0x1e4 != -2) {
                view->UnknownFunction41f550(1);
                view->UnknownFunction41f550(0);
                view->field_0x1dc = 4;
                g_TrackGame->field_0x2e0 = 1.0f;
            }
        } else if (!_stricmp("TogPause", event->controlName)) {
            if (view->field_0x1e4 != -2) {
                view->UnknownFunction41f550(0);
                view->field_0x1dc = 4;
                g_TrackGame->field_0x2e0 = 1.0f;
            }
        } else if (!_stricmp("TogFF", event->controlName)) {
            if (view->field_0x1e4 != -2) {
                if (static_cast<UIMultiState*>(event->control)->UnknownFunction4755c0() == 1) {
                    if (view->field_0x3f8)
                        view->UnknownFunction41f550(0);
                    view->field_0x1dc = 8;
                    g_TrackGame->field_0x2e0 = 2.0f;
                } else {
                    view->UnknownFunction41f550(1);
                    view->field_0x1dc = 4;
                    g_TrackGame->field_0x2e0 = 1.0f;
                }
            }
        } else if (!_stricmp("TogFFF", event->controlName)) {
            if (view->field_0x1e4 != -2) {
                if (static_cast<UIMultiState*>(event->control)->UnknownFunction4755c0() == 1) {
                    if (view->field_0x3f8)
                        view->UnknownFunction41f550(0);
                    view->field_0x1dc = 9;
                    g_TrackGame->field_0x2e0 = 8.0f;
                } else {
                    view->UnknownFunction41f550(1);
                    view->field_0x1dc = 4;
                    g_TrackGame->field_0x2e0 = 1.0f;
                }
            }
        } else if (!_stricmp("TogReverse", event->controlName)) {
            if (static_cast<UIMultiState*>(event->control)->UnknownFunction4755c0() == 1) {
                if (view->field_0x1dc != 14) {
                    if (view->field_0x3f8)
                        view->UnknownFunction41f550(0);
                    view->field_0x1dc = 11;
                    g_TrackGame->field_0x2e0 = 0.0f;
                }
            } else {
                view->UnknownFunction41f550(1);
                view->field_0x1dc = 4;
                g_TrackGame->field_0x2e0 = 1.0f;
            }
        } else if (!_stricmp("ButFrame", event->controlName)) {
            if (view->field_0x1e4 != -2) {
                if (view->field_0x3f8)
                    view->UnknownFunction41f550(0);
                view->field_0x3f9 = 1;
                g_TrackGame->field_0x1c8 = 1;
            }
        } else if (!_stricmp("ButBeginning", event->controlName)) {
            if (view->field_0x3f8)
                view->UnknownFunction41f550(0);
            view->field_0x1dc = 12;
        } else if (!_stricmp("ButEnd", event->controlName)) {
            if (view->field_0x1e4 != -2) {
                if (view->field_0x3f8)
                    view->UnknownFunction41f550(0);
                view->field_0x1dc = 9;
                g_TrackGame->field_0x2e0 = 30.0f;
            }
        } else if (!_stricmp("ButCamera", event->controlName)) {
            TrackGameViewOwner* owner = g_TrackGame->eventManager->FindRaceMode();
            if (owner->field_0x3c->cameraState == 5) {
                owner->field_0x3c->cameraState = owner->field_0x3c->savedCameraState;
                owner->field_0x3c->field_0x258 = owner->field_0x3c->savedParameter;
                owner->field_0x3c->UnknownVirtualSlot58();
            } else {
                owner->field_0x3c->UnknownVirtualSlot72();
            }
        } else if (!_stricmp("ButPrevRider", event->controlName) ||
                   !_stricmp("ButNextRider", event->controlName)) {
            UnknownKrustyBikeView* racers =
                g_TrackGame->eventManager->FindRaceMode()->field_0x34;
            if (!_stricmp("ButPrevRider", event->controlName))
                racers->UnknownFunction41f1d0(1, 1, 0);
            else
                racers->UnknownFunction41f1d0(0, 1, 0);
        } else if (!_stricmp("ButExit", event->controlName)) {
            event->dialog->EndDialog(0);
            event->handled = 1;
            memcpy(&g_TrackGame->mode.field_0x27f8, &g_TrackGame->mode.field_0x29e4,
                   sizeof(g_TrackGame->mode.field_0x29e4));
            memcpy(g_TrackGame->mode.field_0xfd8, g_TrackGame->mode.field_0x1034,
                   sizeof(g_TrackGame->mode.field_0x1034));
            memcpy(&g_TrackGame->mode.field_0x1974, &g_TrackGame->mode.field_0x1a3c,
                   sizeof(g_TrackGame->mode.field_0x1a3c));
            g_TrackGame->eventManager->UnknownFunction45cdc0(2);
            g_TrackGame->eventManager->UnknownFunction45e710(100);
            guiManager->ShowCursors(1);
            field_0x7f58 = 1;
            break;
        }
        ShowReplayMode();
        break;
    case 16:
        if (!_stricmp("SliderTime", event->controlName)) {
            float fraction = static_cast<UIScrollBar*>(event->control)->UnknownFunction475500() / 1000.0f;
            UnknownKrustyBikeView* replay = g_TrackGame->eventManager->FindRaceView();
            replay->field_0x1c4 = fraction * replay->field_0x1a0->field_0x10c;
        }
        break;
    case kDialogClose:
        if (!g_TrackGame->field_0x2d5_bit1 && !field_0x7f58)
            guiManager->ShowCursors(0);
        break;
    }
}

// 0x004893c0
int VCRDlg::UnknownVirtualSlot10(float frameTime) {
    char text[32];
    guiManager->ShowCursors(1);
    UIScrollBar* slider = static_cast<UIScrollBar*>(FindControl("SliderTime", 8));
    UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
    UIControl* control = FindControl("TxtTime", 12);
    FormatTime(text, view->field_0x1b8);
    control->SetText(text);
    control = FindControl("TxtTimeEnd", 12);
    FormatTime(text, view->field_0x1a0->field_0x10c);
    control->SetText(text);
    if (view->field_0x1c4 == -1.0f)
        slider->UnknownFunction4753c0((int)(view->field_0x1b8 * 1000.0f),
                                      (int)(view->field_0x1a0->field_0x10c * 1000.0f));
    ShowReplayMode();
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x004894c0
void VCRDlg::ShowReplayMode() {
    UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
    UIMultiState* play = static_cast<UIMultiState*>(FindControl("TogPlay", 2));
    UIMultiState* pause = static_cast<UIMultiState*>(FindControl("TogPause", 2));
    UIMultiState* reverse = static_cast<UIMultiState*>(FindControl("TogReverse", 2));
    UIMultiState* fastForward = static_cast<UIMultiState*>(FindControl("TogFF", 2));
    UIMultiState* fasterForward = static_cast<UIMultiState*>(FindControl("TogFFF", 2));
    if (view->field_0x3f8) {
        play->SetCurrentState(0);
        pause->SetCurrentState(1);
        reverse->SetCurrentState(0);
        fastForward->SetCurrentState(0);
        fasterForward->SetCurrentState(0);
        return;
    }
    switch (view->field_0x1dc) {
    case 4:
        play->SetCurrentState(1);
        pause->SetCurrentState(0);
        reverse->SetCurrentState(0);
        fastForward->SetCurrentState(0);
        fasterForward->SetCurrentState(0);
        break;
    case 8:
        play->SetCurrentState(0);
        pause->SetCurrentState(0);
        reverse->SetCurrentState(0);
        fastForward->SetCurrentState(1);
        fasterForward->SetCurrentState(0);
        break;
    case 9:
        play->SetCurrentState(0);
        pause->SetCurrentState(0);
        reverse->SetCurrentState(0);
        fastForward->SetCurrentState(0);
        fasterForward->SetCurrentState(1);
        break;
    case 11:
        play->SetCurrentState(0);
        pause->SetCurrentState(0);
        reverse->SetCurrentState(1);
        fastForward->SetCurrentState(0);
        fasterForward->SetCurrentState(0);
        break;
    case 14:
        play->SetCurrentState(0);
        pause->SetCurrentState(0);
        reverse->SetCurrentState(0);
        fastForward->SetCurrentState(0);
        fasterForward->SetCurrentState(0);
        break;
    }
}
