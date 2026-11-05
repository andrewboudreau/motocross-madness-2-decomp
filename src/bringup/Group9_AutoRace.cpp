// Group9_AutoRace.cpp -- bring-up test aid, not part of retail behaviour.
//
// With MCM2_AUTORACE set in the environment, the runtime shim (runtime/d3drm) calls
// AutoRaceTick() on the game thread once per presented frame:
//   * it saves the race settings (g_TrackGame->mode.raceSettings) to autorace.bin the first
//     time the loading dialog opens through the normal menus, and
//   * when autorace.bin exists and the main menu shows, it restores those settings and opens
//     the loading dialog directly, so an event starts without navigating the menus.
// The shim finds the function by its decorated name in mcm2.map.  Without the variable
// nothing calls it.  A thread cannot do this: dialogs opened from a second thread left the
// UI with no live dialog.

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "../reconstructed/BikeRaceCore.h"
#include "../reconstructed/GUIManagerCore.h"
#include "../reconstructed/KrustyUI.h"
#include "../reconstructed/DlgProcs.h"
#include "../reconstructed/MenuId.h"
#include "../reconstructed/TrackGame.h"
#include "../reconstructed/TrackGameGlobal.h"

// Nothing in the game calls it; the runtime shim finds it in mcm2.map.  A global that points
// at it keeps it past the linker's unreferenced-function removal.
void AutoRaceTick();
void (*g_AutoRaceTickHook)() = AutoRaceTick;
void AutoRaceTick()
{
    static bool saved = false;
    static bool started = false;
    static int mainMenuTicks = 0;
    TrackGame* game = g_TrackGame;
    if (!game || !game->ui)
        return;
    int menu = game->ui->currentMenu;
    if ((menu == kMenuLoading || g_LoadingEventLoaded) && !saved) {
        saved = true;
        FILE* f = fopen("autorace.bin", "rb");
        if (f) {
            fclose(f);
        } else if ((f = fopen("autorace.bin", "wb")) != 0) {
            fwrite(&game->mode.raceSettings, sizeof(game->mode.raceSettings), 1, f);
            fclose(f);
        }
    }
    if (menu == kMenuMain && !started) {
        if (++mainMenuTicks < 120)    // let the main menu settle (about two seconds)
            return;
        FILE* f = fopen("autorace.bin", "rb");
        if (!f)
            return;
        started = true;
        fread(&game->mode.raceSettings, sizeof(game->mode.raceSettings), 1, f);
        fclose(f);
        UIDialog* mainDialog = game->ui->gui->FindInputDialog();
        game->ui->OpenMenu(kMenuLoading);
        if (mainDialog)
            mainDialog->EndDialog(0);
    }
}
