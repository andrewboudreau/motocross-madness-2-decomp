// Group9_AutoRace.cpp -- bring-up test aid, not part of retail behaviour.
//
// With MCM2_AUTORACE set in the environment, a watcher thread:
//   * saves the race settings (g_TrackGame->mode.raceSettings) to autorace.bin the first time
//     the loading dialog opens through the normal menus, and
//   * when autorace.bin exists and the main menu shows, restores those settings and opens the
//     loading dialog directly, so an event starts without navigating the menus.
// Without the variable it does nothing.  The thread only reads game state and issues the same
// OpenMenu call the menu's OK handler makes; it exists to speed up test iterations.

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

static DWORD WINAPI AutoRaceThread(LPVOID)
{
    bool saved = false;
    bool started = false;
    int mainMenuTicks = 0;
    for (;;) {
        Sleep(100);
        TrackGame* game = g_TrackGame;
        if (!game || !game->ui)
            continue;
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
            if (++mainMenuTicks < 8)    // let the main menu settle (~0.8 s)
                continue;
            FILE* f = fopen("autorace.bin", "rb");
            if (!f)
                continue;
            started = true;
            fread(&game->mode.raceSettings, sizeof(game->mode.raceSettings), 1, f);
            fclose(f);
            UIDialog* mainDialog = game->ui->gui->FindInputDialog();
            game->ui->OpenMenu(kMenuLoading);
            if (mainDialog)
                mainDialog->EndDialog(0);
        }
    }
    return 0;
}

struct AutoRaceStarter {
    AutoRaceStarter()
    {
        if (GetEnvironmentVariableA("MCM2_AUTORACE", 0, 0))
            CloseHandle(CreateThread(0, 0, AutoRaceThread, 0, 0, 0));
    }
};
static AutoRaceStarter s_autoRaceStarter;
