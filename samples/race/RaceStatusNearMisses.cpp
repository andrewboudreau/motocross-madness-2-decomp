// Near-miss RaceStatus.cpp candidates, kept out of src/reconstructed until
// they match. See docs/RACESTATUS.md.
//
// UpdateGateRace (0x004e6a50, 1004 bytes): the gate-race update.
// Everything matches except one instruction: retail tests the laps left as
// `cmp eax, 1; jl` (`remaining >= 1`), but written that way VC6 lays the
// `else` (`remaining * lap time`) out of line, keeps the lap-time pointer in
// ebx and spills the laps pointer, shifting every stack slot (one more
// local). `remaining > 0` (here) keeps retail's layout and allocation but
// emits `test eax, eax; jle`; a ternary keeps both but stores the 0 through
// fld/fstp. RaceStatus.cpp 0x004e63e0 has the same block and matches with
// `>= 1` and if/else. With `>= 1` none of these restore the layout (638 of
// 1006 at best, against 671 for `> 0`): the loop body wrapped in
// `if (!racer->field_0x7a4)` as in 0x004e63e0, `goto next` for both
// `continue`s, `1 <= remaining`, `!(remaining < 1)`, an empty then-branch
// with the work in `else`, the inner if/else swapped, an int/long/short
// `remaining`, or `remaining` declared at the top in any order with the
// other locals.
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

#define UNKNOWN_RANDOM_UNIT() (rand() * (1.0f / 32768))

// The view's camera (+0x50), whose +0x3b4 racer is the one followed (as
// RaceSound.h's UnknownRaceSoundCamera; KrustyBikeCamera keeps it protected).
struct UnknownRaceStatusCamera {
    unsigned char field_0x000[0x3b4];
    UnknownEventRacer* field_0x3b4;
};

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// |v|, exact for unit vectors (as CarProcedural.cpp).
static inline float Length(Vector3 v) {
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
}

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

// 0x004e6a50
int UpdateGateRace(UnknownEventRacerPart** list, UnknownBikeRaceNode* firstGate, float frameTime, int lapLimit,
                          int unlimited) {
    UnknownEventRacerPart* previous = 0;
    UnknownEventRacerPart* status;
    UnknownEventRacer* racer;

    if (!list || !firstGate)
        return 0;
    PruneStatusList(list);
    for (status = *list; status; previous = status, status = status->next) {
        racer = status->racer;
        if (racer->hasFinished)
            continue;
        {
            int gate = racer->field_0x7b8;
            status->previousPosition.x = racer->field_0x5f0->field_0x200.x;
            status->previousPosition.y = racer->field_0x5f0->field_0x200.y;
            status->previousPosition.z = racer->field_0x5f0->field_0x200.z;
            if (!AdvanceGate(&status->previousPosition, &status->currentPosition, &status->nextGate, firstGate,
                                       &racer->gatesPassed, &racer->field_0x7b8, racer->gateTimes, racer->totalRunningTime,
                                       0, &racer->lapCount, &racer->lastLapTime, &racer->fastestLap,
                                       &racer->lapStartTime, &racer->hasFinished, lapLimit, unlimited,
                                       racer->field_0x735, racer->field_0x7c0))
                return 0;
            if (g_TrackGame->bajaEnduroEvent && gate != racer->field_0x7b8) {
                racer->field_0x7bc = (racer->field_0x7b8 + 1) % g_TrackGame->bajaEnduroEvent->field_0xac;
                if (racer == ((UnknownRaceStatusCamera*)racer->bikeView->field_0x50)->field_0x3b4)
                    g_TrackGame->bajaEnduroEvent->UnknownFunction404df0(racer->field_0x7b8, racer->field_0x7bc,
                                                                              racer);
            }
        }
        if (racer->hasFinished && racer == racer->bikeView->localRacer)
            g_ViewRacerFinished = 1;
        float before = Length(status->nextGate->center - status->currentPosition);
        float after = Length(status->nextGate->center - status->previousPosition);
        status->currentPosition.x = status->previousPosition.x;
        status->currentPosition.y = status->previousPosition.y;
        status->currentPosition.z = status->previousPosition.z;
        status->field_0x14 += before - after;
        status->field_0x08 = after;
        if (g_TrackGame->playerCount == 1 && g_ViewRacerFinished) {
            if (racer->hasFinished)
                continue;
            if (racer->field_0x734) {
                int remaining = lapLimit - racer->lapCount;
                if (remaining > 0) {
                    if (racer->lapCount < 1)
                        racer->totalRunningTime = 0;
                    else
                        racer->totalRunningTime = remaining * racer->lastLapTime;
                }
                if (previous && racer->totalRunningTime < previous->racer->totalRunningTime)
                    racer->totalRunningTime = UNKNOWN_RANDOM_UNIT() * 10 + previous->racer->totalRunningTime + 1.0f;
                if (lapLimit == 1)
                    racer->lastLapTime = racer->totalRunningTime;
                else if (racer->lapCount < 1)
                    racer->lastLapTime =
                        UNKNOWN_RANDOM_UNIT() * 10 + racer->bikeView->localRacer->lastLapTime + 1.0f;
                if (racer->lastLapTime < racer->fastestLap || racer->fastestLap <= 0.0f)
                    racer->fastestLap = racer->lastLapTime;
                racer->lapCount = lapLimit;
                racer->hasFinished = 1;
                continue;
            }
        }
        if (!racer->hasFinished)
            racer->totalRunningTime += frameTime;
    }
    if (!OrderByGates(list))
        return 0;
    return ComputeTimeBehind(*list, 1) != 0;
}
