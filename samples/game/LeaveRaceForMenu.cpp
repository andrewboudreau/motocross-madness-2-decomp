// Complete race-to-menu candidate; see docs/LEAVE_RACE_FOR_MENU.md.
// This extraction does not establish an original translation-unit boundary.
#include "../../src/reconstructed/GUIManagerCore.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/UIDialog.h"
#include "../../src/reconstructed/PCCamera.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/UnknownResourceManager.h"

extern "C" void* __stdcall ImmAssociateContext(void* window, void* context);

// 0x0045e710: leaves the race for `menu`: restores the UI, the window's
// input context and 640x480x16, and shows the transition dialog.
void EventManager::LeaveRaceForMenu(int menu) {
    if (!g_TrackGame->ui)
        return;
    UIDialog* item = g_TrackGame->ui->gui->FindInputDialog();
    if (item)
        item->EndDialog(0);
    if (g_TrackGame->ui->savedInputContext)
        ImmAssociateContext(g_TrackGame->windowHandle, g_TrackGame->ui->savedInputContext);
    if (g_TrackGame->ui->allowIme)
        g_TrackGame->ui->gui->EnableWindowClipper(1);
    g_TrackGame->uiInteractionBlocked = 0;
    g_TrackGame->podiumFinished = 0;
    g_TrackGame->uiBlockedSeconds = 0;
    if (!g_TrackGame->mode.field_0xa20)
        g_TrackGame->StopCDAudio();
    UnknownDisplayMode* current = &g_TrackGame->display->displayModes[g_TrackGame->mode.displayModeIndex];
    if (current->width != 640 || current->height != 480 || current->bitDepth != 16)
        g_TrackGame->SwitchDisplayMode(
            g_TrackGame->display->FindDisplayMode(640, 480, 16, 0, 0));
    g_TrackGame->ui->gui->CreateBackground();
    UnknownKrustyUIGuiLayer* layer = g_TrackGame->ui->gui->GetUser(0);
    ((UnknownKrustyUIToolTipView*)layer->userToolTip)->enabled = g_TrackGame->mode.toolTips;
    if (!g_TrackGame->playingReplay && !g_TrackGame->ui->playingDemo &&
        g_TrackGame->mode.raceSettings.raceMode != 0 && g_TrackGame->mode.raceSettings.raceMode != 4)
        g_TrackGame->ui->RecordRaceResults();
    g_TrackGame->ui->gui->ShowCursors(1);
    TransDlg* dialog = new(__FILE__, 1064) TransDlg;
    g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
    dialog->SetNextMenu(menu);
    ((PCSoundInterface*)g_TrackGame->soundInterface)->SetPrimaryVolume(0);
}
