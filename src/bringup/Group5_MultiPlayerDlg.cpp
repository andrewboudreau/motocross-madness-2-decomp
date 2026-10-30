// Group5 bring-up definitions (best-guess, link frontier). Not strict matches.
//
// MultiPlayerDlg::AddChatLine (0x004f3720) and SendStartMessage (0x004f2340),
// extracted from samples/ui/SelectGamePicProcsNearMisses.cpp (same calls,
// strings, flow and message layout as retail; register allocation and block
// layout differ). The file-local message/grid types and COPY_TEXT are copied
// from src/reconstructed/SelectGamePicProcs.cpp, which keeps them private.
#include "GUIManagerCore.h"
#include "SelectGamePicProcs.h"
#include "DialogEventKind.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BackgroundImage.h"
#include "Camera.h"
#include "DebugAlloc.h"
#include "DlgProcs.h"
#include "EventManager.h"
#include "../krusty2/math/FastMath.h"
#include "MatrixUtil.h"
#include "Net.h"
#include "TextureMap.h"
#include "Track.h"
#include "TrackGame.h"

// A truncating copy into a `size`-byte buffer (as in SelectGamePicProcs.cpp).
#define COPY_TEXT(dest, source, size)                              \
    {                                                              \
        int length = strlen(source);                               \
        int copied = length > (size) - 1 ? (size) - 1 : length;    \
        strncpy(dest, source, copied);                             \
        (dest)[copied] = 0;                                        \
    }

// Defined in SelectGamePicProcs.cpp.
extern PlayerInfoType g_LobbyRemotePlayers[7];  // 0x00689d08
extern int g_LobbyRemotePlayerCount;
int CompareGridEntries(const void* a, const void* b);  // 0x004f2080
int CompareGridDraws(const void* a, const void* b);    // 0x004f20a0

// A racer's random grid key (12 bytes).
struct UnknownGridDraw {
    int playerId;                             // +0x00: player id
    float key;                                // +0x04: random key
    char racerIndex;                          // +0x08: racer index
};

// The lobby start message (type 0x83, 0x8d8 bytes).
struct UnknownLobbyStartMessage {
    int header;                               // +0x00 SendRaw writes the type byte and sequence here
    short start;                              // +0x04: start
    UnknownTrackGameModeSettings raceSettings; // +0x08: TrackGame+0x2d70
    int difficulty;                           // +0x1f4: TrackGame+0x60c (mode.difficulty)
    int plateNumbers[8];                      // +0x1f8: racer slot +0xc0
    char playerNames[8][0x10];                // +0x218: racer slot +0xdc
    char bikeModelNames[8][0x40];             // +0x298: racer slot +0x00
    char bikeNames[8][0x40];                  // +0x498: racer slot +0x40
    char riderNames[8][0x40];                 // +0x698: racer slot +0x80
    int engineSizes[8];                       // +0x898: racer slot +0xec
    int fourStrokes[8];                       // +0x8b8: racer slot +0xf0
};

// A grid entry at race settings +0x14c with the racer index at +5.
struct UnknownGridEntry {
    UnknownGridEntry() {
        playerId = 0;
        points = 0;
    }

    int playerId;                             // +0x00: player id
    char points;                              // +0x04: championship points
    char racerIndex;                          // +0x05: racer index
};

// A random value in [0, 1) (as in KrustyUI.cpp).
static inline float RandomUnit() {
    return (float)(rand() * (1.0f / 32768));
}

// 0x004f3720: adds a chat line to "ListChat"; -1 is a system message.
// Retail falls into the colour pass for system messages too (the name buffer
// is then unset, but only used if a lobby record has id -1).
void MultiPlayerDlg::AddChatLine(int player, const char* text) {
    char line[0x60];
    char name[0x80];
    char system[0x80];
    UIListBox* list = static_cast<UIListBox*>(FindControl("ListChat", 3));
    if (!list)
        return;
    if (player != -1) {
        if (!g_TrackGame->network->GetPlayerName(player, name))
            return;
        sprintf(line, "%s: %s", name, text);
    } else {
        g_TrackGame->LoadResourceString(0x14ba, system, 0x80);
        if (!g_TrackGame->imeLibrary)
            sprintf(line, "~5%s: %s", system, text);
        else
            sprintf(line, "%s: %s", system, text);
    }
    if (!g_TrackGame->imeLibrary) {
        if (g_TrackGame->network->localPlayer == player) {
            sprintf(line, "~1%s: ~0%s", name, text);
        } else {
            for (int i = 0; i < 7; i++) {
                if (g_LobbyRemotePlayers[i].id == player)
                    sprintf(line, "~%1d%s: ~0%s", i + 2, name, text);
            }
        }
    }
    int rows = list->rowCount;
    if ((short)rows >= 100)
        list->RemoveRow(0);
    list->AddRow(line, 0, 0);
    list->ScrollToRow((short)rows, 1);
}

// 0x004f2340: sends the lobby start message; with `start` it first draws
// the AI racers and the starting grid. Retail never frees `bikes` and
// `riders` (only `names`), kept as is.
void MultiPlayerDlg::SendStartMessage(short start) {
    int i;
    g_TrackGame->mode.raceSettings.networkPlayerCount = g_LobbyRemotePlayerCount + 1;
    g_TrackGame->playerCount = g_TrackGame->mode.raceSettings.networkPlayerCount;
    g_TrackGame->networkAiRacerCount = g_TrackGame->mode.raceSettings.multiplayerAiRacerCount;
    g_TrackGame->mode.racerCount = g_TrackGame->networkAiRacerCount + g_TrackGame->playerCount;
    int total = g_TrackGame->mode.racerCount;
    g_TrackGame->mode.maxOpponents =
        lobbySlots.GrantRacers(g_LobbyRemotePlayerCount + 1, g_TrackGame->mode.raceSettings.multiplayerAiRacerCount);
    if (g_TrackGame->mode.maxOpponents < g_TrackGame->mode.raceSettings.multiplayerAiRacerCount)
        g_TrackGame->mode.raceSettings.multiplayerAiRacerCount = g_TrackGame->mode.maxOpponents;
    if (start) {
        int* bikes = (int*)DebugCalloc(total, 4, "D:\\aardvark\\VC\\krusty2\\SelectGamePicProcs.cpp", 0x1f4);
        int* riders = (int*)DebugCalloc(total, 4, "D:\\aardvark\\VC\\krusty2\\SelectGamePicProcs.cpp", 0x1f5);
        const char** names =
            (const char**)DebugMalloc(total * 4, "D:\\aardvark\\VC\\krusty2\\SelectGamePicProcs.cpp", 0x1f6);
        g_TrackGame->ui->PickRandomRacers(bikes, total, riders);
        g_TrackGame->ui->MakeRandomNames(names, total);
        for (i = 0; i < g_TrackGame->playerCount - 1; i++) {
            g_TrackGame->mode.racers[i + 1].networkPlayerId = g_LobbyRemotePlayers[i].id;
            g_TrackGame->mode.racers[i + 1].racerIndex = 0;
        }
        g_TrackGame->mode.racers[0].networkPlayerId = g_TrackGame->network->localPlayer;
        g_TrackGame->mode.racers[0].racerIndex = 0;
        int racer = g_LobbyRemotePlayerCount + 1;
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < lobbySlots.slots[j].grantedRacers; k++) {
                g_TrackGame->mode.racers[racer].networkPlayerId = lobbySlots.slots[j].playerId;
                g_TrackGame->mode.racers[racer].racerIndex = racer;
                COPY_TEXT(g_TrackGame->mode.racers[racer].playerName, names[racer], 0x10);
                g_TrackGame->mode.racers[racer].plateNumber = 100 - (int)(RandomUnit() * -899.0f);
                COPY_TEXT(g_TrackGame->mode.racers[racer].bikeModelName,
                          ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)
                              [((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bikes[racer]].model]
                                  .modelName,
                          0x40);
                COPY_TEXT(g_TrackGame->mode.racers[racer].bikeName,
                          ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bikes[racer]].texture, 0x40);
                COPY_TEXT(g_TrackGame->mode.racers[racer].riderName,
                          ((UnknownKrustyUIModel*)g_TrackGame->ui->riders)[riders[racer]].modelName, 0x40);
                g_TrackGame->mode.racers[racer].engineSize =
                    ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bikes[racer]].engineSize;
                g_TrackGame->mode.racers[racer].fourStroke =
                    ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bikes[racer]].fourStroke;
                racer++;
            }
        }
        g_TrackGame->mode.ResetSettingsEntries();
        if (g_TrackGame->mode.raceSettings.raceMode == 2 && g_TrackGame->eventManager->racesRun > 0) {
            // By the championship points.
            for (i = 0; i < racer; i++) {
                UnknownGridEntry entry;
                entry.playerId = g_TrackGame->mode.racers[i].networkPlayerId;
                entry.points = g_TrackGame->eventManager->results[i].points;
                entry.racerIndex = g_TrackGame->mode.racers[i].racerIndex;
                ((UnknownGridEntry*)g_TrackGame->mode.raceSettings.gridOrder)[i] = entry;
            }
            qsort(g_TrackGame->mode.raceSettings.gridOrder, racer, sizeof(UnknownGridEntry),
                  CompareGridEntries);
        } else {
            // At random.
            UnknownGridDraw draws[8];
            for (i = 0; i < racer; i++) {
                UnknownGridDraw draw;
                draw.playerId = g_TrackGame->mode.racers[i].networkPlayerId;
                draw.key = RandomUnit() * 10.0f;
                draw.racerIndex = g_TrackGame->mode.racers[i].racerIndex;
                draws[i] = draw;
            }
            qsort(draws, racer, sizeof(UnknownGridDraw), CompareGridDraws);
            for (i = 0; i < racer; i++) {
                UnknownGridEntry entry;
                entry.playerId = draws[i].playerId;
                entry.racerIndex = draws[i].racerIndex;
                ((UnknownGridEntry*)g_TrackGame->mode.raceSettings.gridOrder)[i] = entry;
            }
        }
        if (names)
            DebugFree(names, "D:\\aardvark\\VC\\krusty2\\SelectGamePicProcs.cpp", 0x23e);
    }
    UnknownLobbyStartMessage message;
    message.start = start;
    message.raceSettings = g_TrackGame->mode.raceSettings;
    message.difficulty = g_TrackGame->mode.difficulty;
    memcpy(message.raceSettings.field_0x18c, lobbySlots.slots, sizeof(lobbySlots.slots));
    if (start) {
        char name[0x80];
        message.plateNumbers[0] = g_TrackGame->mode.plateNumber;
        g_TrackGame->network->GetPlayerName(g_TrackGame->network->localPlayer, name);
        strcpy(message.playerNames[0], name);
        int humans = g_LobbyRemotePlayerCount + 1;
        for (i = 1; i < humans; i++) {
            message.plateNumbers[i] = g_TrackGame->mode.racers[i].plateNumber;
            strcpy(message.playerNames[i], g_TrackGame->mode.racers[i].playerName);
        }
        for (i = humans; i < total; i++) {
            message.plateNumbers[i] = g_TrackGame->mode.racers[i].plateNumber;
            strcpy(message.playerNames[i], g_TrackGame->mode.racers[i].playerName);
            strcpy(message.bikeModelNames[i], g_TrackGame->mode.racers[i].bikeModelName);
            strcpy(message.bikeNames[i], g_TrackGame->mode.racers[i].bikeName);
            strcpy(message.riderNames[i], g_TrackGame->mode.racers[i].riderName);
            message.engineSizes[i] = g_TrackGame->mode.racers[i].engineSize;
            message.fourStrokes[i] = g_TrackGame->mode.racers[i].fourStroke;
        }
    }
    g_TrackGame->network->Send(0x83, &message, sizeof(message), g_TrackGame->network->localPlayer, 0);
    if (start)
        FillRacerSlots();
}
