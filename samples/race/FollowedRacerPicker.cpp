// Complete nonexact followed-racer picker; original TU ownership is unknown.
// See docs/FOLLOWED_RACER_PICKER.md for the bounded functional review.
#include <stdlib.h>
#include "../../src/reconstructed/BikeRace.h"
#include "../../src/reconstructed/TrackGame.h"

#ifndef UNKNOWN_RANDOM_UNIT
#define UNKNOWN_RANDOM_UNIT() (rand() * (1.0f / 32768.0f))
#endif

// 0x004a9d20: the host picks the racer to follow: the only one still racing
// (+0x4a0 clear), or a random still-racing one, or its own racer when none
// is; then tells the players (message 0x87) and the replay recorder.
void TrackGameViewOwner::PickFollowedRacer()
{
    if (!g_TrackGame->network || !g_TrackGame->network->isHost)
        return;
    int racing = 0;
    int iterator = 0;
    int tries = 0;
    UnknownEventRacer* last = 0;
    UnknownEventRacer* racer = reinterpret_cast<UnknownEventRacer*>(
            reinterpret_cast<BikeRace*>(raceView)->NextBike(&iterator));
    while (racer) {
        if (racer->outOfRace == 0) {
            last = racer;
            racing++;
        }
        racer = reinterpret_cast<UnknownEventRacer*>(
            reinterpret_cast<BikeRace*>(raceView)->NextBike(&iterator));
    }
    if (racing == 0) {
        followedRacer = raceView->localRacer;
    } else if (racing == 1) {
        followedRacer = last;
    } else {
        // Retail keeps this view across rand(), then reads its slot array.
        UnknownKrustyBikeView* selectionView = raceView;
        int count = selectionView->racerCount;
        int index = (int)(UNKNOWN_RANDOM_UNIT() * count);
        while (selectionView->racerSlots[index]->outOfRace == 1 && tries < count) {
            index++;
            if (index >= count)
                index = 0;
            tries++;
        }
        followedRacer = selectionView->racerSlots[index];
    }
    UnknownBikeRaceVcrFollow follow;
    follow.field_0x04 = 0;
    follow.followedRacerId = followedRacer->networkPlayerId;
    g_TrackGame->network->Send(0x87, &follow, sizeof(follow), g_TrackGame->network->localPlayer, 0);
    if (raceView->replayVcr && g_TrackGame->mode.saveReplay && !g_TrackGame->playingReplay)
        ((VCRInterface*)raceView->replayVcr)->QueueRecord(0x87, followedRacer->networkPlayerId, &follow, 1);
}
