#include "../../src/reconstructed/BikeRaceCore.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/TrackGame.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef char RaceSize[sizeof(BikeRace) == 0x400 ? 1 : -1];
typedef char RaceViewSize[sizeof(UnknownKrustyBikeView) == 0x3fc ? 1 : -1];
typedef char CanonicalLocal[offsetof(BikeRace, localRacer) == 0x38 ? 1 : -1];
typedef char ProvisionalLocal[offsetof(UnknownKrustyBikeView, localRacer) == 0x38 ? 1 : -1];
typedef char CanonicalPause[offsetof(BikeRace, replayPaused) == 0x3f8 ? 1 : -1];
typedef char ProvisionalPause[offsetof(UnknownKrustyBikeView, replayPaused) == 0x3f8 ? 1 : -1];
typedef char EventRace[offsetof(TrackGameViewOwner, raceView) == 0x34 ? 1 : -1];

TrackGame* g_TrackGame;

// Only the two separately compiled, full-byte-verified methods execute.
// Their accessed fields are initialized; no constructors, virtual calls or
// other operations on the partial event/race declarations are invoked.
int main() {
    unsigned long gameStorage[(sizeof(TrackGame) + 3) / 4];
    unsigned long raceStorage[sizeof(BikeRace) / 4];
    unsigned long savedRace[sizeof(BikeRace) / 4];
    unsigned long savedGame[(sizeof(TrackGame) + 3) / 4];
    unsigned long managerStorage[(sizeof(EventManager) + 3) / 4];
    unsigned long events[5][0x38 / 4];
    unsigned long identities[5];
    const unsigned offsets[] = {
        offsetof(TrackGame, supercrossEvent), offsetof(TrackGame, quarryEvent),
        offsetof(TrackGame, bajaEnduroEvent), offsetof(TrackGame, nationalEvent),
        offsetof(TrackGame, tagEvent)
    };
    const int pauses[] = {0, 1, -1, 0x7fffffff, (int)0x80000000};
    g_TrackGame = reinterpret_cast<TrackGame*>(gameStorage);
    BikeRace* race = reinterpret_cast<BikeRace*>(raceStorage);
    EventManager* manager = reinterpret_cast<EventManager*>(managerStorage);
    unsigned checks = 0;
    for (unsigned mask = 0; mask < 32; ++mask) {
        memset(gameStorage, 0xa5, sizeof(gameStorage));
        memset(events, 0x6b, sizeof(events));
        UnknownKrustyBikeView* first = 0;
        for (int n = 0; n < 5; ++n) {
            *(void**)((char*)gameStorage + offsets[n]) = mask & (1u << n) ? events[n] : 0;
            events[n][0x34 / 4] = (unsigned long)&identities[n];
            if (!first && (mask & (1u << n))) first = (UnknownKrustyBikeView*)&identities[n];
        }
        memcpy(savedGame, gameStorage, sizeof(gameStorage));
        if (manager->FindRaceView() != first || memcmp(savedGame, gameStorage, sizeof(gameStorage))) return 1;
        ++checks;
    }
    // A present first event wins even when its race pointer is null. There
    // is no search for a later nonnull race after choosing the event.
    *(void**)((char*)gameStorage + offsets[0]) = events[0];
    events[0][0x34 / 4] = 0;
    if (manager->FindRaceView()) return 2;
    ++checks;

    for (unsigned seed = 0; seed < 32; ++seed)
    for (int paused = 0; paused < 2; ++paused)
    for (int globalPause = 0; globalPause < 3; ++globalPause)
    for (int value = 0; value < 5; ++value) {
        memset(gameStorage, seed * 7, sizeof(gameStorage));
        memset(raceStorage, seed * 11, sizeof(raceStorage));
        race->replayPaused = paused != 0;
        g_TrackGame->paused = globalPause == 2 ? -7 : globalPause;
        memcpy(savedRace, raceStorage, sizeof(raceStorage));
        memcpy(savedGame, gameStorage, sizeof(gameStorage));
        if (!pauses[value] || !paused) {
            bool next = pauses[value] ? true : !paused;
            ((BikeRace*)savedRace)->replayPaused = next;
            ((TrackGame*)savedGame)->paused = next;
        }
        // The call crosses precisely the provisional primary-pointer boundary
        // used by the production callers, without a receiver adjustment.
        UnknownKrustyBikeView* view = reinterpret_cast<UnknownKrustyBikeView*>(race);
        reinterpret_cast<BikeRace*>(view)->ToggleReplayPause(pauses[value]);
        if (memcmp(savedRace, raceStorage, sizeof(raceStorage)) ||
            memcmp(savedGame, gameStorage, sizeof(gameStorage))) return 3;
        ++checks;
    }
    printf("Race control: %u exact-provider route and pause cases passed\n", checks);
    return 0;
}
