#include "InGameProcs.h"

#include "DialogEventKind.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "FollowCamera.h"
#include "Net.h"
#include "TrackGame.h"
#include "MenuId.h"

// The 0x14-byte message 0x89 that a network player sends when it leaves
// (EventManager slot 24 marks the sender done).
struct UnknownPlayerLeftMessage {
    int field_0x00;
    char field_0x04[16];                      // player name
};

// 0x004886e0
void ExitDlg::HandleEvent(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ButMiddle", event->controlName)) {
            guiManager->ShowCursors(0);
            if (!g_TrackGame->uiInteractionBlocked) {
                UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
                if (view && !view->localRacer->hasFinished) {
                    view->RestartRace(1, 0, 0);
                    g_TrackGame->SetMenuOpen(0, kMenuExit, 1);
                }
            }
        } else if (!_stricmp("ButLeft", event->controlName)) {
            UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
            if (view && g_TrackGame->mode.raceSettings.raceMode == 4 && view->ghostLap > 0) {
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
                HandleMenuCancel(0x191, event);
            }
        } else if (!_stricmp("ButRight", event->controlName)) {
            guiManager->ShowCursors(0);
            HandleMenuOk(0x191, event);
            g_TrackGame->ResetClock();
        }
        break;
    case kDialogInit: {
        UIControl* control = FindControl("TitleText", 12);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13d9);
        control = FindControl("ButMiddle", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13da);
        UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
        if (g_TrackGame->network || view->localRacer->hasFinished ||
            g_TrackGame->mode.raceSettings.raceMode == 4)
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
            g_TrackGame->mode.saveGhost = 1;
            guiManager->ShowCursors(0);
            HandleMenuCancel(0x191, event);
        } else if (event->code == 0x67) {
            g_TrackGame->mode.saveGhost = 0;
            guiManager->ShowCursors(0);
            HandleMenuCancel(0x191, event);
        }
        break;
    }
}

// 0x00488ad0
void ContinueDlg::HandleEvent(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ButMiddle", event->controlName)) {
            guiManager->ShowCursors(0);
            HandleMenuMiddle(0x190, event);
            g_TrackGame->ResetClock();
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
void VCRDlg::HandleEvent(UnknownDialogEvent* event) {
    UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
    switch (event->kind) {
    case kDialogInit:
        field_0x7f58 = 0;
        static_cast<UIScrollBar*>(FindControl("SliderTime", 8))->SetScrollRange(1000);
        FindControl("ButSave", 0)->Show(0, 1);
        if (!g_TrackGame->GetRegistryFlag("AllowVCRControls", 0)) {
            FindControl("TogFF", 0)->Show(0, 1);
            FindControl("TogFFF", 0)->Show(0, 1);
            FindControl("TogReverse", 0)->Show(0, 1);
            FindControl("ButBeginning", 0)->Show(0, 1);
            FindControl("ButEnd", 0)->Show(0, 1);
            FindControl("ButFrame", 0)->Show(0, 1);
            FindControl("TogFF", 0)->SetEnabled(0);
            FindControl("TogFFF", 0)->SetEnabled(0);
            FindControl("TogReverse", 0)->SetEnabled(0);
            FindControl("ButBeginning", 0)->SetEnabled(0);
            FindControl("ButEnd", 0)->SetEnabled(0);
            FindControl("SliderTime", 8)->SetEnabled(0);
            FindControl("ButFrame", 0)->SetEnabled(0);
        }
        break;
    case kDialogCommand:
        if (!_stricmp("TogPlay", event->controlName)) {
            if (view->lastRecordKind != -2) {
                view->ToggleReplayPause(1);
                view->ToggleReplayPause(0);
                view->replayMode = 4;
                g_TrackGame->replaySpeed = 1.0f;
            }
        } else if (!_stricmp("TogPause", event->controlName)) {
            if (view->lastRecordKind != -2) {
                view->ToggleReplayPause(0);
                view->replayMode = 4;
                g_TrackGame->replaySpeed = 1.0f;
            }
        } else if (!_stricmp("TogFF", event->controlName)) {
            if (view->lastRecordKind != -2) {
                if (static_cast<UIMultiState*>(event->control)->GetSelectedState() == 1) {
                    if (view->replayPaused)
                        view->ToggleReplayPause(0);
                    view->replayMode = 8;
                    g_TrackGame->replaySpeed = 2.0f;
                } else {
                    view->ToggleReplayPause(1);
                    view->replayMode = 4;
                    g_TrackGame->replaySpeed = 1.0f;
                }
            }
        } else if (!_stricmp("TogFFF", event->controlName)) {
            if (view->lastRecordKind != -2) {
                if (static_cast<UIMultiState*>(event->control)->GetSelectedState() == 1) {
                    if (view->replayPaused)
                        view->ToggleReplayPause(0);
                    view->replayMode = 9;
                    g_TrackGame->replaySpeed = 8.0f;
                } else {
                    view->ToggleReplayPause(1);
                    view->replayMode = 4;
                    g_TrackGame->replaySpeed = 1.0f;
                }
            }
        } else if (!_stricmp("TogReverse", event->controlName)) {
            if (static_cast<UIMultiState*>(event->control)->GetSelectedState() == 1) {
                if (view->replayMode != 14) {
                    if (view->replayPaused)
                        view->ToggleReplayPause(0);
                    view->replayMode = 11;
                    g_TrackGame->replaySpeed = 0.0f;
                }
            } else {
                view->ToggleReplayPause(1);
                view->replayMode = 4;
                g_TrackGame->replaySpeed = 1.0f;
            }
        } else if (!_stricmp("ButFrame", event->controlName)) {
            if (view->lastRecordKind != -2) {
                if (view->replayPaused)
                    view->ToggleReplayPause(0);
                view->replayStepFrame = 1;
                g_TrackGame->stepOneFrame = 1;
            }
        } else if (!_stricmp("ButBeginning", event->controlName)) {
            if (view->replayPaused)
                view->ToggleReplayPause(0);
            view->replayMode = 12;
        } else if (!_stricmp("ButEnd", event->controlName)) {
            if (view->lastRecordKind != -2) {
                if (view->replayPaused)
                    view->ToggleReplayPause(0);
                view->replayMode = 9;
                g_TrackGame->replaySpeed = 30.0f;
            }
        } else if (!_stricmp("ButCamera", event->controlName)) {
            TrackGameViewOwner* owner = g_TrackGame->eventManager->FindRaceMode();
            if (owner->raceCamera->cameraState == 5) {
                owner->raceCamera->cameraState = owner->raceCamera->savedCameraState;
                owner->raceCamera->targetFieldOfView = owner->raceCamera->savedFieldOfView;
                owner->raceCamera->UnknownVirtualSlot58();
            } else {
                owner->raceCamera->UnknownVirtualSlot72();
            }
        } else if (!_stricmp("ButPrevRider", event->controlName) ||
                   !_stricmp("ButNextRider", event->controlName)) {
            UnknownKrustyBikeView* racers =
                g_TrackGame->eventManager->FindRaceMode()->raceView;
            if (!_stricmp("ButPrevRider", event->controlName))
                racers->NextCameraTarget(1, 1, 0);
            else
                racers->NextCameraTarget(0, 1, 0);
        } else if (!_stricmp("ButExit", event->controlName)) {
            event->dialog->EndDialog(0);
            event->handled = 1;
            memcpy(&g_TrackGame->mode.raceSettings, &g_TrackGame->mode.savedSettings,
                   sizeof(g_TrackGame->mode.savedSettings));
            memcpy(g_TrackGame->mode.garageOptions, g_TrackGame->mode.optionsBackup,
                   sizeof(g_TrackGame->mode.optionsBackup));
            memcpy(&g_TrackGame->mode.racerChoice, &g_TrackGame->mode.racerChoiceBackup,
                   sizeof(g_TrackGame->mode.racerChoiceBackup));
            g_TrackGame->eventManager->CloseRace(2);
            g_TrackGame->eventManager->LeaveRaceForMenu(kMenuMain);
            guiManager->ShowCursors(1);
            field_0x7f58 = 1;
            break;
        }
        ShowReplayMode();
        break;
    case 16:
        if (!_stricmp("SliderTime", event->controlName)) {
            float fraction = static_cast<UIScrollBar*>(event->control)->GetScrollValue() / 1000.0f;
            UnknownKrustyBikeView* replay = g_TrackGame->eventManager->FindRaceView();
            replay->replaySeekTarget = fraction * replay->replayVcr->replayLength;
        }
        break;
    case kDialogClose:
        if (!g_TrackGame->shuttingDown && !field_0x7f58)
            guiManager->ShowCursors(0);
        break;
    }
}

// 0x004893c0
int VCRDlg::Tick(float frameTime) {
    char text[32];
    guiManager->ShowCursors(1);
    UIScrollBar* slider = static_cast<UIScrollBar*>(FindControl("SliderTime", 8));
    UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
    UIControl* control = FindControl("TxtTime", 12);
    FormatTime(text, view->replayTime);
    control->SetText(text);
    control = FindControl("TxtTimeEnd", 12);
    FormatTime(text, view->replayVcr->replayLength);
    control->SetText(text);
    if (view->replaySeekTarget == -1.0f)
        slider->SetThumbPosition((int)(view->replayTime * 1000.0f),
                                      (int)(view->replayVcr->replayLength * 1000.0f));
    ShowReplayMode();
    return UIDialog::Tick(frameTime);
}

// 0x004894c0
void VCRDlg::ShowReplayMode() {
    UnknownKrustyBikeView* view = g_TrackGame->eventManager->FindRaceView();
    UIMultiState* play = static_cast<UIMultiState*>(FindControl("TogPlay", 2));
    UIMultiState* pause = static_cast<UIMultiState*>(FindControl("TogPause", 2));
    UIMultiState* reverse = static_cast<UIMultiState*>(FindControl("TogReverse", 2));
    UIMultiState* fastForward = static_cast<UIMultiState*>(FindControl("TogFF", 2));
    UIMultiState* fasterForward = static_cast<UIMultiState*>(FindControl("TogFFF", 2));
    if (view->replayPaused) {
        play->SetCurrentState(0);
        pause->SetCurrentState(1);
        reverse->SetCurrentState(0);
        fastForward->SetCurrentState(0);
        fasterForward->SetCurrentState(0);
        return;
    }
    switch (view->replayMode) {
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
