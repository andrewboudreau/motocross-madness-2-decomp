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
#include <stdlib.h>

#include "../reconstructed/BikeRaceCore.h"
#include "../reconstructed/GUIManagerCore.h"
#include "../reconstructed/KrustyUI.h"
#include "../reconstructed/DlgProcs.h"
#include "../reconstructed/EventManager.h"
#include "../reconstructed/MenuId.h"
#include "../reconstructed/SelectGamePicProcs.h"
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
    static int lastMenu = -1;
    if (menu != lastMenu) {
        lastMenu = menu;
        FILE* log = fopen("autorace.log", "a");
        if (log) {
            fprintf(log, "menu %d\n", menu);
            fclose(log);
        }
    }
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
    // MCM2_AUTORACE_EXIT=<seconds>: leave the race for the main menu after that many frames/60,
    // to reproduce the return-to-menu path (#36).
    static int raceTicks = 0;
    static bool left = false;
    if (started && !left && menu != kMenuMain && menu != kMenuLoading) {
        const char* exitEnv = getenv("MCM2_AUTORACE_EXIT");
        if (exitEnv && ++raceTicks >= atoi(exitEnv) * 60) {
            left = true;
            game->eventManager->LeaveRaceForMenu(kMenuMain);
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
        game->mode.SelectSeries(game->mode.raceSettings.eventType);    // sets currentSeriesDirectory, as the menus do
        UIDialog* mainDialog = game->ui->gui->FindInputDialog();
        LeaveMenusForRace();    // what the menus' start button calls: shuts the UI down, then opens loading
        if (mainDialog)
            mainDialog->MarkPendingRelease();
    }
}
