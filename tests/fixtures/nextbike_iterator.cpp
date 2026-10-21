#include "../../src/reconstructed/BikeRaceCore.h"
#include "../../src/reconstructed/TrackGame.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef char RaceSize[sizeof(BikeRace) == 0x400 ? 1 : -1];
typedef char RaceLocal[offsetof(BikeRace, localRacer) == 0x38 ? 1 : -1];
typedef char RaceSlots[offsetof(BikeRace, racerSlots) == 0x3c ? 1 : -1];
typedef char RaceAi[offsetof(BikeRace, aiRacers) == 0x40 ? 1 : -1];
typedef char GateSize[sizeof(UnknownBikeRaceNode) == 0x3c ? 1 : -1];
typedef char GateNext[offsetof(UnknownBikeRaceNode, nextGate) == 0x38 ? 1 : -1];
typedef char ViewLocal[offsetof(UnknownKrustyBikeView, localRacer) == 0x38 ? 1 : -1];
typedef char GamePlayers[offsetof(TrackGame, playerCount) == 0x18 ? 1 : -1];
typedef char GameBlocked[offsetof(TrackGame, uiInteractionBlocked) == 0x3430 ? 1 : -1];

TrackGame* g_TrackGame;

// No game constructor or racer vtable is needed: NextBike only returns these
// primary pointers. Exact full-body proofs bind this execution to production.
int main() {
    unsigned long raceStorage[sizeof(BikeRace) / 4];
    unsigned long gameStorage[(sizeof(TrackGame) + 3) / 4];
    unsigned long savedRace[sizeof(BikeRace) / 4];
    unsigned long savedGame[(sizeof(TrackGame) + 3) / 4];
    unsigned long tokens[32];
    UnknownBikeRaceRacer* slots[32];
    UnknownBikeRaceRacer* ai[32];
    BikeRace* race = reinterpret_cast<BikeRace*>(raceStorage);
    g_TrackGame = reinterpret_cast<TrackGame*>(gameStorage);
    unsigned cases = 0;
    for (int players = 0; players <= 4; ++players)
    for (int blocked = 0; blocked != 2; ++blocked)
    for (int mode = 0; mode != 2; ++mode)
    for (int count = 0; count != 5; ++count)
    for (int nullAi = 0; nullAi != 2; ++nullAi)
    for (int holes = 0; holes != 2; ++holes) {
        memset(raceStorage, 0xa5, sizeof(raceStorage));
        memset(gameStorage, 0x5a, sizeof(gameStorage));
        for (int i = 0; i != 32; ++i) {
            slots[i] = holes && i % 3 == 0 ? 0 : reinterpret_cast<UnknownBikeRaceRacer*>(&tokens[i]);
            ai[i] = holes && i % 3 == 1 ? 0 : reinterpret_cast<UnknownBikeRaceRacer*>(&tokens[31-i]);
        }
        race->localRacer = reinterpret_cast<UnknownBikeRaceRacer*>(&tokens[31]);
        race->racerSlots = slots;
        race->aiRacers = nullAi ? 0 : ai;
        g_TrackGame->playerCount = players;
        g_TrackGame->uiInteractionBlocked = blocked;
        g_TrackGame->mode.raceSettings.raceMode = mode ? 4 : 0;
        g_TrackGame->mode.raceSettings.aiRacerCount = count;
        g_TrackGame->mode.raceSettings.multiplayerAiRacerCount = count;
        memcpy(savedRace, raceStorage, sizeof(savedRace));
        memcpy(savedGame, gameStorage, sizeof(savedGame));
        for (int start = 0; start != 12; ++start) {
            int cursor = start;
            int expectedCursor = start;
            for (int step = 0; step != 3; ++step) {
                UnknownBikeRaceRacer* expected = 0;
                int limit = players > 1 ? players + count : (mode ? 2 : count + 1);
                if (!blocked) {
                    if (expectedCursor < limit) {
                        if (players > 1) expected = slots[expectedCursor];
                        else if (players == 1 && expectedCursor == 0) expected = race->localRacer;
                        else if (!nullAi && expectedCursor >= players) expected = ai[expectedCursor - players];
                    }
                    expectedCursor = expectedCursor + 1 > limit ? 0 : expectedCursor + 1;
                }
                UnknownBikeRaceRacer* result = race->NextBike(&cursor);
                if (result != expected || cursor != expectedCursor ||
                    memcmp(savedRace, raceStorage, sizeof(savedRace)) ||
                    memcmp(savedGame, gameStorage, sizeof(savedGame))) {
                    printf("failed players=%d blocked=%d mode=%d count=%d null=%d holes=%d start=%d step=%d\n",
                           players, blocked, mode, count, nullAi, holes, start, step);
                    return 1;
                }
                ++cases;
            }
        }
        // Retail's blocked path returns before accessing the iterator.
        if (blocked && race->NextBike(0) != 0) return 2;
    }
    printf("%u iterator cases passed\n", cases);
    return 0;
}
