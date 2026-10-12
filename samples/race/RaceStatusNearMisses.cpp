// Near-miss RaceStatus.cpp candidates, kept out of src/reconstructed until
// they match. See docs/RACESTATUS.md.
//
// BuildStatusList (0x004e5d00, 519 bytes): builds a view's status
// list. The statements, calls and loops line up; only register allocation
// differs: retail keeps the list pointer in ebp and the index in ebx (also
// reusing it for the constant 1 in the calloc count and the race-mode test)
// and keeps `tail` in the view argument's stack slot, while VC6 here folds
// the first index to 1, keeps `tail` in ebx and reloads `list`. Every
// declaration order, `tail = list` for the first node and an unsigned index
// leave that allocation, as do (72 combinations, all 168 of 519) `index = 1`
// after the first calloc or before it with `index` as its count, `tail`
// started from `&(*list)->field_0x50`, the race-mode test against `index`,
// `++index` in the node store, and the link written as `tail =
// &(*tail)->field_0x50` or `tail = &(*tail = status)->field_0x50`.

#include "../../src/reconstructed/RaceStatus.h"

#include <stdlib.h>

#include "../../src/reconstructed/BikeRace.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x004e5d00
int BuildStatusList(UnknownEventRacerPart** list, UnknownKrustyBikeView* view) {
    UnknownEventRacerPart* status;
    UnknownEventRacerPart** tail;
    int index;
    int i;

    g_ViewRacerFinished = 0;
    if (!list)
        return 0;
    index = 1;
    status = (UnknownEventRacerPart*)DebugCalloc(1, sizeof(UnknownEventRacerPart), __FILE__, 214);
    if (!status)
        return 0;
    status->racer = view->localRacer;
    status->nextGate = view->gateList;
    view->localRacer->statusNode = status;
    status->creationOrder = index;
    status->currentPosition = view->localRacer->position;
    *list = status;
    tail = &status->next;
    if (g_TrackGame->playerCount == 1 && g_TrackGame->mode.raceSettings.raceMode != 4) {
        for (i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++) {
            if (!HasStatusNode(*list, view->aiRacers[i])) {
                status = (UnknownEventRacerPart*)DebugCalloc(1, sizeof(UnknownEventRacerPart), __FILE__, 235);
                if (!status)
                    return 0;
                index++;
                status->racer = view->aiRacers[i];
                status->nextGate = view->gateList;
                view->aiRacers[i]->statusNode = status;
                status->creationOrder = index;
                status->currentPosition = view->aiRacers[i]->position;
                *tail = status;
                tail = &status->next;
            }
        }
    }
    if (view->racerSlots) {
        for (i = 0; i < view->racerCount; i++) {
            if (!HasStatusNode(*list, view->racerSlots[i]) && view->racerSlots[i] != view->localRacer) {
                status = (UnknownEventRacerPart*)DebugCalloc(1, sizeof(UnknownEventRacerPart), __FILE__, 257);
                if (!status)
                    return 0;
                index++;
                status->racer = view->racerSlots[i];
                status->nextGate = view->gateList;
                view->racerSlots[i]->statusNode = status;
                status->creationOrder = index;
                status->currentPosition = view->racerSlots[i]->position;
                *tail = status;
                tail = &status->next;
            }
        }
    }
    return 1;
}
