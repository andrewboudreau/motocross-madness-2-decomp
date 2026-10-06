#include "InGameProcs.h"

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
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("ButMiddle", event->field_0x04)) {
            field_0x30->UnknownFunction486630(0);
            if (!g_UnknownGlobal56e26c->uiInteractionBlocked) {
                UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
                if (view && !view->field_0x38->field_0x7a4) {
                    view->UnknownFunction420650(1, 0, 0);
                    g_UnknownGlobal56e26c->UnknownFunction521860(0, 0x191, 1);
                }
            }
        } else if (!_stricmp("ButLeft", event->field_0x04)) {
            UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
            if (view && g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 4 && view->field_0x1e8 > 0) {
                ChoiceDlg* dialog = new(__FILE__, 66) ChoiceDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0,
                                                  (UnknownGuiDialog*)this, 0, 0, 1);
                dialog->UnknownFunction455700(0, 0x1440, 0, 0, 0, 0x143e, 0, 0, 0, 0x143d);
            } else {
                if (g_UnknownGlobal56e26c->field_0x08) {
                    char name[128];
                    UnknownPlayerLeftMessage message;
                    NetworkInterface* network = g_UnknownGlobal56e26c->field_0x08;
                    int player = network->field_0x0c;
                    network->UnknownFunction4ac720(player, name);
                    if (strcmp(name, "") != 0) {
                        int length = strlen(name);
                        int count = length > 15 ? 15 : length;
                        strncpy(message.field_0x04, name, count);
                        message.field_0x04[count] = 0;
                        g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac830(
                            0x89, &message, sizeof(message), player, 0);
                    }
                }
                field_0x30->UnknownFunction486630(0);
                UnknownFunction4526b0(0x191, event);
            }
        } else if (!_stricmp("ButRight", event->field_0x04)) {
            field_0x30->UnknownFunction486630(0);
            UnknownFunction452930(0x191, event);
            g_UnknownGlobal56e26c->UnknownFunction468880();
        }
        break;
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("TitleText", 12);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13d9);
        control = UnknownFunction46ebf0("ButMiddle", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13da);
        UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
        if (g_UnknownGlobal56e26c->field_0x08 || view->field_0x38->field_0x7a4 ||
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 4)
            control->UnknownFunction470660(0, 1);
        control = UnknownFunction46ebf0("ButLeft", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13db);
        control = UnknownFunction46ebf0("ButRight", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13dc);
        field_0x30->UnknownFunction486630(1);
        break;
    }
    case 9:
        if (event->field_0x00 == 0x65) {
            g_UnknownGlobal56e26c->mode.field_0x26f0 = 1;
            field_0x30->UnknownFunction486630(0);
            UnknownFunction4526b0(0x191, event);
        } else if (event->field_0x00 == 0x67) {
            g_UnknownGlobal56e26c->mode.field_0x26f0 = 0;
            field_0x30->UnknownFunction486630(0);
            UnknownFunction4526b0(0x191, event);
        }
        break;
    }
}

// 0x00488ad0
void ContinueDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("ButMiddle", event->field_0x04)) {
            field_0x30->UnknownFunction486630(0);
            UnknownFunction453090(0x190, event);
            g_UnknownGlobal56e26c->UnknownFunction468880();
        }
        break;
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("TitleText", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x8ff);
        control = UnknownFunction46ebf0("ButMiddle", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x916);
        control->field_0x1d8 = 0x1c;
        control = UnknownFunction46ebf0("ButLeft", 1);
        control->UnknownFunction470660(0, 1);
        control->field_0x1d8 = 0;
        UnknownFunction46ebf0("ButRight", 0)->UnknownFunction470660(0, 1);
        break;
    }
    }
}

// 0x00488bd0
void UnknownFunction488bd0(char* text, float seconds) {
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
    UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
    switch (event->field_0x08) {
    case 5:
        field_0x7f58 = 0;
        UnknownFunction46ebf0("SliderTime", 8)->UnknownFunction4751c0(1000);
        UnknownFunction46ebf0("ButSave", 0)->UnknownFunction470660(0, 1);
        if (!g_UnknownGlobal56e26c->UnknownVirtualSlot22("AllowVCRControls", 0)) {
            UnknownFunction46ebf0("TogFF", 0)->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("TogFFF", 0)->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("TogReverse", 0)->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("ButBeginning", 0)->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("ButEnd", 0)->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("ButFrame", 0)->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("TogFF", 0)->UnknownVirtualSlot49(0);
            UnknownFunction46ebf0("TogFFF", 0)->UnknownVirtualSlot49(0);
            UnknownFunction46ebf0("TogReverse", 0)->UnknownVirtualSlot49(0);
            UnknownFunction46ebf0("ButBeginning", 0)->UnknownVirtualSlot49(0);
            UnknownFunction46ebf0("ButEnd", 0)->UnknownVirtualSlot49(0);
            UnknownFunction46ebf0("SliderTime", 8)->UnknownVirtualSlot49(0);
            UnknownFunction46ebf0("ButFrame", 0)->UnknownVirtualSlot49(0);
        }
        break;
    case 1:
        if (!_stricmp("TogPlay", event->field_0x04)) {
            if (view->field_0x1e4 != -2) {
                view->UnknownFunction41f550(1);
                view->UnknownFunction41f550(0);
                view->field_0x1dc = 4;
                g_UnknownGlobal56e26c->field_0x2e0 = 1.0f;
            }
        } else if (!_stricmp("TogPause", event->field_0x04)) {
            if (view->field_0x1e4 != -2) {
                view->UnknownFunction41f550(0);
                view->field_0x1dc = 4;
                g_UnknownGlobal56e26c->field_0x2e0 = 1.0f;
            }
        } else if (!_stricmp("TogFF", event->field_0x04)) {
            if (view->field_0x1e4 != -2) {
                if (event->field_0x14->UnknownFunction4755c0() == 1) {
                    if (view->field_0x3f8)
                        view->UnknownFunction41f550(0);
                    view->field_0x1dc = 8;
                    g_UnknownGlobal56e26c->field_0x2e0 = 2.0f;
                } else {
                    view->UnknownFunction41f550(1);
                    view->field_0x1dc = 4;
                    g_UnknownGlobal56e26c->field_0x2e0 = 1.0f;
                }
            }
        } else if (!_stricmp("TogFFF", event->field_0x04)) {
            if (view->field_0x1e4 != -2) {
                if (event->field_0x14->UnknownFunction4755c0() == 1) {
                    if (view->field_0x3f8)
                        view->UnknownFunction41f550(0);
                    view->field_0x1dc = 9;
                    g_UnknownGlobal56e26c->field_0x2e0 = 8.0f;
                } else {
                    view->UnknownFunction41f550(1);
                    view->field_0x1dc = 4;
                    g_UnknownGlobal56e26c->field_0x2e0 = 1.0f;
                }
            }
        } else if (!_stricmp("TogReverse", event->field_0x04)) {
            if (event->field_0x14->UnknownFunction4755c0() == 1) {
                if (view->field_0x1dc != 14) {
                    if (view->field_0x3f8)
                        view->UnknownFunction41f550(0);
                    view->field_0x1dc = 11;
                    g_UnknownGlobal56e26c->field_0x2e0 = 0.0f;
                }
            } else {
                view->UnknownFunction41f550(1);
                view->field_0x1dc = 4;
                g_UnknownGlobal56e26c->field_0x2e0 = 1.0f;
            }
        } else if (!_stricmp("ButFrame", event->field_0x04)) {
            if (view->field_0x1e4 != -2) {
                if (view->field_0x3f8)
                    view->UnknownFunction41f550(0);
                view->field_0x3f9 = 1;
                g_UnknownGlobal56e26c->field_0x1c8 = 1;
            }
        } else if (!_stricmp("ButBeginning", event->field_0x04)) {
            if (view->field_0x3f8)
                view->UnknownFunction41f550(0);
            view->field_0x1dc = 12;
        } else if (!_stricmp("ButEnd", event->field_0x04)) {
            if (view->field_0x1e4 != -2) {
                if (view->field_0x3f8)
                    view->UnknownFunction41f550(0);
                view->field_0x1dc = 9;
                g_UnknownGlobal56e26c->field_0x2e0 = 30.0f;
            }
        } else if (!_stricmp("ButCamera", event->field_0x04)) {
            TrackGameViewOwner* owner = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0();
            if (owner->field_0x3c->cameraState == 5) {
                owner->field_0x3c->cameraState = owner->field_0x3c->savedCameraState;
                owner->field_0x3c->field_0x258 = owner->field_0x3c->savedParameter;
                owner->field_0x3c->UnknownVirtualSlot58();
            } else {
                owner->field_0x3c->UnknownVirtualSlot72();
            }
        } else if (!_stricmp("ButPrevRider", event->field_0x04) ||
                   !_stricmp("ButNextRider", event->field_0x04)) {
            UnknownKrustyBikeView* racers =
                g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0()->field_0x34;
            if (!_stricmp("ButPrevRider", event->field_0x04))
                racers->UnknownFunction41f1d0(1, 1, 0);
            else
                racers->UnknownFunction41f1d0(0, 1, 0);
        } else if (!_stricmp("ButExit", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
            memcpy(&g_UnknownGlobal56e26c->mode.field_0x27f8, &g_UnknownGlobal56e26c->mode.field_0x29e4,
                   sizeof(g_UnknownGlobal56e26c->mode.field_0x29e4));
            memcpy(g_UnknownGlobal56e26c->mode.field_0xfd8, g_UnknownGlobal56e26c->mode.field_0x1034,
                   sizeof(g_UnknownGlobal56e26c->mode.field_0x1034));
            memcpy(&g_UnknownGlobal56e26c->mode.field_0x1974, &g_UnknownGlobal56e26c->mode.field_0x1a3c,
                   sizeof(g_UnknownGlobal56e26c->mode.field_0x1a3c));
            g_UnknownGlobal56e26c->eventManager->UnknownFunction45cdc0(2);
            g_UnknownGlobal56e26c->eventManager->UnknownFunction45e710(100);
            field_0x30->UnknownFunction486630(1);
            field_0x7f58 = 1;
            break;
        }
        UnknownFunction4894c0();
        break;
    case 16:
        if (!_stricmp("SliderTime", event->field_0x04)) {
            float fraction = event->field_0x14->UnknownFunction475500() / 1000.0f;
            UnknownKrustyBikeView* replay = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
            replay->field_0x1c4 = fraction * replay->field_0x1a0->field_0x10c;
        }
        break;
    case 6:
        if (!g_UnknownGlobal56e26c->field_0x2d5_bit1 && !field_0x7f58)
            field_0x30->UnknownFunction486630(0);
        break;
    }
}

// 0x004893c0
int VCRDlg::UnknownVirtualSlot10(float frameTime) {
    char text[32];
    field_0x30->UnknownFunction486630(1);
    UnknownGameUiControl* slider = UnknownFunction46ebf0("SliderTime", 8);
    UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
    UnknownGameUiControl* control = UnknownFunction46ebf0("TxtTime", 12);
    UnknownFunction488bd0(text, view->field_0x1b8);
    control->UnknownFunction470b20(text);
    control = UnknownFunction46ebf0("TxtTimeEnd", 12);
    UnknownFunction488bd0(text, view->field_0x1a0->field_0x10c);
    control->UnknownFunction470b20(text);
    if (view->field_0x1c4 == -1.0f)
        slider->UnknownFunction4753c0((int)(view->field_0x1b8 * 1000.0f),
                                      (int)(view->field_0x1a0->field_0x10c * 1000.0f));
    UnknownFunction4894c0();
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// 0x004894c0
void VCRDlg::UnknownFunction4894c0() {
    UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
    UnknownGameUiControl* play = UnknownFunction46ebf0("TogPlay", 2);
    UnknownGameUiControl* pause = UnknownFunction46ebf0("TogPause", 2);
    UnknownGameUiControl* reverse = UnknownFunction46ebf0("TogReverse", 2);
    UnknownGameUiControl* fastForward = UnknownFunction46ebf0("TogFF", 2);
    UnknownGameUiControl* fasterForward = UnknownFunction46ebf0("TogFFF", 2);
    if (view->field_0x3f8) {
        play->UnknownFunction478cf0(0);
        pause->UnknownFunction478cf0(1);
        reverse->UnknownFunction478cf0(0);
        fastForward->UnknownFunction478cf0(0);
        fasterForward->UnknownFunction478cf0(0);
        return;
    }
    switch (view->field_0x1dc) {
    case 4:
        play->UnknownFunction478cf0(1);
        pause->UnknownFunction478cf0(0);
        reverse->UnknownFunction478cf0(0);
        fastForward->UnknownFunction478cf0(0);
        fasterForward->UnknownFunction478cf0(0);
        break;
    case 8:
        play->UnknownFunction478cf0(0);
        pause->UnknownFunction478cf0(0);
        reverse->UnknownFunction478cf0(0);
        fastForward->UnknownFunction478cf0(1);
        fasterForward->UnknownFunction478cf0(0);
        break;
    case 9:
        play->UnknownFunction478cf0(0);
        pause->UnknownFunction478cf0(0);
        reverse->UnknownFunction478cf0(0);
        fastForward->UnknownFunction478cf0(0);
        fasterForward->UnknownFunction478cf0(1);
        break;
    case 11:
        play->UnknownFunction478cf0(0);
        pause->UnknownFunction478cf0(0);
        reverse->UnknownFunction478cf0(1);
        fastForward->UnknownFunction478cf0(0);
        fasterForward->UnknownFunction478cf0(0);
        break;
    case 14:
        play->UnknownFunction478cf0(0);
        pause->UnknownFunction478cf0(0);
        reverse->UnknownFunction478cf0(0);
        fastForward->UnknownFunction478cf0(0);
        fasterForward->UnknownFunction478cf0(0);
        break;
    }
}
