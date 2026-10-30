// Group6_BuildStatusList.cpp -- best-guess BuildStatusList (0x004e5d00), body taken
// unchanged from samples/race/RaceStatusNearMisses.cpp (statements, calls and loops line
// up with retail; register allocation differs; not byte-matched). BikeRace::Create
// (Group6_BikeRaceCreate.cpp) calls it; the canonical RaceStatus.cpp does not define it.

#include "RaceStatus.h"

#include <stdlib.h>

#include "BikeRace.h"
#include "DebugAlloc.h"
#include "FollowCamera.h"
#include "TrackGame.h"

// best guess: sample body (samples/race/RaceStatusNearMisses.cpp), unreviewed.
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
