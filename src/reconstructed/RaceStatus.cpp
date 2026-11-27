#include "RaceStatus.h"

#include <stdlib.h>

#include "BikeRace.h"
#include "DebugAlloc.h"
#include "EventManager.h"
#include "../krusty2/vehicle/KrustyBikeCore.h"
#include "../krusty2/tire/Tire.h"
#include "FollowCamera.h"
#include "TrackGame.h"

// The four vector constants of src/krusty2/math/Math3D.h: 0x00689c10,
// 0x00689c20, 0x00689c60 and 0x00689c00. Their initialisers
// (0x004e6e40..0x004e6f7b) follow this file's functions and come first in
// .CRT$XCU, ahead of the arrays' empty one (0x004e58a0).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// One declaration: retail has a single empty initializer for both arrays.
Vector3 g_GateDebugBoxLine[2], g_GateDebugDirectionLine[2];
int g_ViewRacerFinished;

// 0x00572988 and 0x0057298c (.data, before this file's __FILE__): how far
// ahead of and behind a racer's progress Track::PlaceNear accepts a
// position (0x004e63e0). The ahead distance also bounds the lap-line test
// and each progress step; BikeAI (0x00415091) scales it.
float g_TrackSearchAhead = 100.0f;
float g_TrackSearchBehind = 50.0f;

// Field-by-field TrackPos copy (retail loads all three before storing).
#define COPY_TRACK_POS(to, from)          \
    {                                     \
        (to).node = (from).node;          \
        (to).segment = (from).segment;    \
        (to).t = (from).t;                \
    }

// rand() scaled to [0, 1), as ProCircuit.cpp. Retail multiplies by 1/32768
// and then by the integer scale; a float scale lets VC6 fold the two.
#define UNKNOWN_RANDOM_UNIT() (rand() * (1.0f / 32768))

static inline Vector3& operator-=(Vector3& a, const Vector3& b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

// 0x004e58c0
int CountLap(Vector3* from, Vector3* to, unsigned short* laps, RaceGate* gate,
                          char* finished, int lapLimit, float* lapTime, float* lapTimes, float now,
                          float* lapStart, float* bestLap, int direction, int unlimited, int skipTest,
                          int forced) {
    if (!from || !to || !laps || !gate || !finished || !lapTime || !lapTimes || !bestLap)
        return 0;
    if (!skipTest) {
        if (!CrossesGate(from, to, gate, direction))
            return 0;
    } else if (!forced) {
        return 0;
    }
    ++*laps;
    if (!unlimited && *laps >= lapLimit)
        *finished = 1;
    *lapTime = now - *lapStart;
    if (*lapTime < 0.0f) {
        *lapTime = 0;
        return 0;
    }
    lapTimes[*laps % 100] = now;
    if (*laps == 2 || *lapTime < *bestLap)
        *bestLap = *lapTime;
    *lapStart = now;
    return 1;
}

// 0x004e59c0
int AdvanceGate(Vector3* from, Vector3* to, RaceGate** current, RaceGate* first,
                          int* count, int* gateIndex, float* times, float now, int direction,
                          unsigned short* laps, float* lapTime, float* bestLap, float* lapStart, char* finished,
                          int lapLimit, int unlimited, int skipTest, int forced) {
    if (!from || !to || !current || !*current || !count || !times || !gateIndex)
        return 0;
    if ((!skipTest && CrossesGate(from, to, *current, direction)) || (skipTest && forced)) {
        *current = (*current)->nextGate;
        ++*gateIndex;
        if (!*current) {
            *current = first;
            *gateIndex = 0;
            ++*laps;
            if (!unlimited && *laps >= lapLimit)
                *finished = 1;
            float time = now - *lapStart;
            *lapTime = time;
            if (time < 0.0f) {
                *lapTime = 0;
                return 0;
            }
            if (!skipTest && (time < *bestLap || *bestLap <= 0.0f))
                *bestLap = time;
            *lapStart = now;
        }
        times[++*count % 600] = now;
    }
    g_GateDebugBoxLine[0] = (*current)->center - (*current)->halfExtents;
    g_GateDebugBoxLine[0].y += 3.0f;
    g_GateDebugBoxLine[1] = (*current)->halfExtents * 2.0f + g_GateDebugBoxLine[0];
    g_GateDebugDirectionLine[0] = (*current)->center;
    g_GateDebugDirectionLine[0].y += 3.0f;
    g_GateDebugDirectionLine[1] = (*current)->direction * 10.0f + g_GateDebugDirectionLine[0];
    return 1;
}

// 0x004e5c70
int HasStatusNode(RaceStatusNode* list, KrustyBike* racer) {
    for (; list; list = list->next) {
        if (list->racer == racer)
            return 1;
    }
    return 0;
}

// 0x004e5ca0
int PruneStatusList(RaceStatusNode** list) {
    while (list && *list) {
        if (!(*list)->racer->isEnabled) {
            (*list)->racer->statusNode = 0;
            RaceStatusNode* status = *list;
            *list = status->next;
            DebugFree(status, __FILE__, 193);
        } else {
            list = &(*list)->next;
        }
    }
    return 1;
}

// 0x004e5d00: builds a view's status list.
// Not exact (168 of 519 bytes in place; docs/RACESTATUS.md). The statements,
// calls and loops line up; only register allocation differs: retail keeps the list pointer in ebp and the index in ebx (also
// reusing it for the constant 1 in the calloc count and the race-mode test)
// and keeps `tail` in the view argument's stack slot, while VC6 here folds
// the first index to 1, keeps `tail` in ebx and reloads `list`. Every
// declaration order, `tail = list` for the first node and an unsigned index
// leave that allocation, as do (72 combinations, all 168 of 519) `index = 1`
// after the first calloc or before it with `index` as its count, `tail`
// started from `&(*list)->next`, the race-mode test against `index`,
// `++index` in the node store, and the link written as `tail =
// &(*tail)->next` or `tail = &(*tail = status)->next`.
int BuildStatusList(RaceStatusNode** list, BikeRace* view) {
    RaceStatusNode* status;
    RaceStatusNode** tail;
    int index;
    int i;

    g_ViewRacerFinished = 0;
    if (!list)
        return 0;
    index = 1;
    status = (RaceStatusNode*)DebugCalloc(1, sizeof(RaceStatusNode), __FILE__, 214);
    if (!status)
        return 0;
    status->racer = view->localRacer;
    status->nextGate = view->gateList;
    view->localRacer->statusNode = status;
    status->creationOrder = index;
    status->currentPosition = (Vector3&)view->localRacer->position;
    *list = status;
    tail = &status->next;
    if (g_TrackGame->playerCount == 1 && g_TrackGame->mode.raceSettings.raceMode != 4) {
        for (i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++) {
            if (!HasStatusNode(*list, view->aiRacers[i])) {
                status = (RaceStatusNode*)DebugCalloc(1, sizeof(RaceStatusNode), __FILE__, 235);
                if (!status)
                    return 0;
                index++;
                status->racer = view->aiRacers[i];
                status->nextGate = view->gateList;
                view->aiRacers[i]->statusNode = status;
                status->creationOrder = index;
                status->currentPosition = (Vector3&)view->aiRacers[i]->position;
                *tail = status;
                tail = &status->next;
            }
        }
    }
    if (view->racerSlots) {
        for (i = 0; i < view->racerCount; i++) {
            if (!HasStatusNode(*list, view->racerSlots[i]) && view->racerSlots[i] != view->localRacer) {
                status = (RaceStatusNode*)DebugCalloc(1, sizeof(RaceStatusNode), __FILE__, 257);
                if (!status)
                    return 0;
                index++;
                status->racer = view->racerSlots[i];
                status->nextGate = view->gateList;
                view->racerSlots[i]->statusNode = status;
                status->creationOrder = index;
                status->currentPosition = (Vector3&)view->racerSlots[i]->position;
                *tail = status;
                tail = &status->next;
            }
        }
    }
    return 1;
}

// 0x004e5f10
int FreeStatusList(RaceStatusNode* list) {
    while (list) {
        RaceStatusNode* status = list;
        list = list->next;
        DebugFree(status, __FILE__, 285);
    }
    return 0;
}

// 0x004e5f40
int OrderByLapDistance(RaceStatusNode** list, Track* track, RaceGate* probe) {
    RaceStatusNode* status;
    RaceStatusNode** at;
    RaceStatusNode** next;
    int key;
    int i;

    if (!list || !track)
        return 0;
    for (status = *list; status; status = status->next) {
        status->gateDistance = track->DistanceAlong(status->placedPos, probe->trackPos);
        if (status->gateDistance < 0.0f)
            return 0;
        float distance = track->DistanceAlong(status->placedPos, status->progressPos);
        if (distance > 0.0f && (distance < status->progressStep || status->progressStep < 0.0f) &&
            distance > status->gateDistance)
            status->lapOffset = -1;
        else
            status->lapOffset = 0;
    }
    for (at = list; *at; ) {
        if (!(*at)->next)
            break;
        if (!(*at)->racer->hasFinished) {
            key = (*at)->racer->lapCount + (*at)->lapOffset;
            for (next = &(*at)->next; (status = *next) != 0; ) {
                int other = status->racer->lapCount + status->lapOffset;
                if (other > key || (other == key && status->gateDistance < (*at)->gateDistance)) {
                    *next = status->next;
                    status->next = *at;
                    *at = status;
                    key = status->racer->lapCount + status->lapOffset;
                }
                if (*next)
                    next = &(*next)->next;
            }
        }
        if (*at)
            at = &(*at)->next;
    }
    for (status = *list, i = 1; status; status = status->next, i++)
        status->racer->finishPosition = i;
    return 1;
}

// 0x004e6120
int OrderByGates(RaceStatusNode** list) {
    RaceStatusNode* status;
    RaceStatusNode** at;
    RaceStatusNode** next;
    int key;
    int i;

    if (!list)
        return 0;
    for (status = *list; status; status = status->next) {
        float dx = status->nextGate->center.x - status->previousPosition.x;
        float dz = status->nextGate->center.z - status->previousPosition.z;
        status->gateDistance = dx * dx + dz * dz;
        if (status->gateDistance < 0.0f)
            return 0;
    }
    for (at = list; *at; ) {
        if (!(*at)->next)
            break;
        if (!(*at)->racer->hasFinished) {
            key = (*at)->racer->gatesPassed;
            for (next = &(*at)->next; (status = *next) != 0; ) {
                int other = status->racer->gatesPassed;
                if (other > key || (other == key && status->gateDistance < (*at)->gateDistance)) {
                    *next = status->next;
                    status->next = *at;
                    *at = status;
                    key = status->racer->gatesPassed;
                }
                if (*next)
                    next = &(*next)->next;
            }
        }
        if (*at)
            at = &(*at)->next;
    }
    for (status = *list, i = 1; status; status = status->next, i++)
        status->racer->finishPosition = i;
    return 1;
}

// 0x004e6210
int ComputeTimeBehind(RaceStatusNode* list, int mode) {
    RaceStatusNode* status;

    for (status = list; status; status = status->next) {
        KrustyBike* racer = status->racer;
        if (racer->finishPosition > 1) {
            if (mode == 1)
                racer->timeBehindLeader = racer->gateTimes[racer->gatesPassed % 600] -
                                     list->racer->gateTimes[racer->gatesPassed % 600];
            else
                racer->timeBehindLeader = racer->lapStartTimes[racer->lapCount % 100] -
                                     list->racer->lapStartTimes[racer->lapCount % 100];
            if (status->racer->timeBehindLeader >= 0.0f)
                continue;
        }
        status->racer->timeBehindLeader = 0;
    }
    return 1;
}

// 0x004e62d0
int RankByScore(int keepRacing) {
    RacerScoreRank scores[11];
    int iterator;
    int count;
    int i;

    if (g_TrackGame->mode.raceSettings.eventType == 0) {
        count = 0;
        BikeRace* view = g_TrackGame->quarryEvent->raceView;
        KrustyBike* racer;
        iterator = 0;
        for (racer = reinterpret_cast<KrustyBike*>(view->NextBike(&iterator)); racer; racer = reinterpret_cast<KrustyBike*>(view->NextBike(&iterator))) {
            if (racer->isEnabled) {
                scores[count].value = racer->score;
                scores[count].racer = racer;
                count++;
            }
        }
        qsort(scores, count, sizeof(RacerScoreRank), CompareScoresDescending);
        for (i = 0; i < count; i++)
            scores[i].racer->finishPosition = i + 1;
        if (!keepRacing) {
            TrackGameViewOwner* owner = g_TrackGame->quarryEvent;
            if (owner->elapsedMinutes * 60.0f + owner->elapsedSeconds >=
                g_TrackGame->mode.raceSettings.timeLimitMinutes * 60.0f - 1.0f) {
                for (i = 0; i < count; i++)
                    scores[i].racer->hasFinished = 1;
            }
        }
    }
    return 1;
}

// 0x004e63e0
int UpdateLapRace(RaceStatusNode** list, Track* track, float frameTime, RaceGate* start,
                          RaceGate* finish, int lapLimit, int unlimited) {
    RaceStatusNode* previous = 0;
    RaceStatusNode* status;
    KrustyBike* racer;

    if (!list || !track)
        return 0;
    PruneStatusList(list);
    for (status = *list; status; previous = status, status = status->next) {
        racer = status->racer;
        if (!racer->hasFinished) {
            status->previousPosition = (Vector3&)racer->bodyForward * 3.5f + (Vector3&)racer->position;
            if (!status->placedPos.node) {
                status->progressPos.node = start->trackPos.node;
                status->progressPos.segment = start->trackPos.segment;
                status->progressPos.t = start->trackPos.t + 0.1f;
            }
            if (!racer->isRemote)
                racer->onTrack = track->PlaceNear(status->previousPosition, status->progressPos,
                                                                  g_TrackSearchAhead, g_TrackSearchBehind, 0,
                                                                  &status->placedPos);
            else
                track->ClosestPointOnTrack(status->previousPosition, &status->placedPos, 0);
            if (racer->onTrack) {
                racer->offTrackTime = 0;
                status->goalDistance = track->DistanceAlong(status->progressPos, finish->trackPos);
                status->gateDistance = track->DistanceAlong(status->placedPos, finish->trackPos);
                if (status->goalDistance < g_TrackSearchAhead && status->goalDistance < status->gateDistance)
                    COPY_TRACK_POS(status->placedPos, status->progressPos);
                if ((racer->isRemote && racer->forceGatePass) || g_TrackSearchAhead < 0.0f ||
                    (!racer->isRemote && status->goalDistance >= 0.0f && status->goalDistance < g_TrackSearchAhead)) {
                    if (CountLap(&status->previousPosition, &status->currentPosition, &racer->lapCount, finish,
                                              &racer->hasFinished, lapLimit, &racer->lastLapTime, racer->lapStartTimes,
                                              racer->totalRunningTime, &racer->lapStartTime, &racer->fastestLap, 1,
                                              unlimited, racer->isRemote, racer->forceGatePass)) {
                        status->progressPos.node = finish->trackPos.node;
                        status->progressPos.segment = finish->trackPos.segment;
                        status->progressPos.t = finish->trackPos.t + 1e-5f;
                    }
                }
                if (racer->hasFinished && racer == racer->race->localRacer)
                    g_ViewRacerFinished = 1;
                status->progressStep = track->DistanceAlong(status->progressPos, status->placedPos);
                if (status->progressStep > 0.0f && status->progressStep < g_TrackSearchAhead &&
                    status->progressStep < status->goalDistance) {
                    status->progressTotal += status->progressStep;
                    COPY_TRACK_POS(status->progressPos, status->placedPos);
                }
            } else {
                racer->offTrackTime += frameTime;
            }
            status->currentPosition.x = status->previousPosition.x;
            status->currentPosition.y = status->previousPosition.y;
            status->currentPosition.z = status->previousPosition.z;
            if (g_TrackGame->playerCount == 1 && g_ViewRacerFinished) {
                if (racer->hasFinished)
                    goto next;
                if (racer->aiKind) {
                    int remaining = lapLimit - racer->lapCount;
                    if (remaining >= 1) {
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
                        racer->lastLapTime = UNKNOWN_RANDOM_UNIT() * 10 +
                                             racer->race->localRacer->lastLapTime + 1.0f;
                    if (racer->lastLapTime < racer->fastestLap || racer->fastestLap <= 0.0f)
                        racer->fastestLap = racer->lastLapTime;
                    racer->lapCount = lapLimit;
                    racer->hasFinished = 1;
                    goto next;
                }
            }
            if (!racer->hasFinished)
                racer->totalRunningTime += frameTime;
        }
    next:
        if (status->progressPos.node && status->progressPos.segment && status->progressPos.segment->next) {
            if (!track->PointAt(status->progressPos, (Vec3*)&status->racer->respawnPosition))
                racer->respawnPosition = racer->position;
            if (!track->SegmentDirection(status->progressPos.segment,
                                              (Vec3*)&status->racer->respawnHeading))
                racer->respawnHeading = racer->bodyForward;
            (Vector3&)status->racer->respawnPosition -= (Vector3&)status->racer->respawnHeading * 3.5f;
        }
    }
    if (!OrderByLapDistance(list, track, finish))
        return 0;
    return ComputeTimeBehind(*list, 0) != 0;
}

// |v|, exact for unit vectors (as CarProcedural.cpp).
static inline float Length(Vector3 v) {
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
}

// 0x004e6a50: the gate-race update. Once the local racer has finished in
// a single-player race, the others get an estimated time. A finished racer
// leaves that block with `goto next` (retail's block layout and registers
// for the `remaining >= 1` test; a `continue` there lays the lap-time
// product out of line), the estimated one with `continue`.
int UpdateGateRace(RaceStatusNode** list, RaceGate* firstGate, float frameTime, int lapLimit,
                          int unlimited) {
    RaceStatusNode* previous = 0;
    RaceStatusNode* status;
    KrustyBike* racer;

    if (!list || !firstGate)
        return 0;
    PruneStatusList(list);
    for (status = *list; status; previous = status, status = status->next) {
        racer = status->racer;
        if (racer->hasFinished)
            continue;
        {
            int gate = racer->lapGate;
            status->previousPosition.x = racer->frontWheel->wheelCenter.x;
            status->previousPosition.y = racer->frontWheel->wheelCenter.y;
            status->previousPosition.z = racer->frontWheel->wheelCenter.z;
            if (!AdvanceGate(&status->previousPosition, &status->currentPosition, &status->nextGate, firstGate,
                                       &racer->gatesPassed, &racer->lapGate, racer->gateTimes, racer->totalRunningTime,
                                       0, &racer->lapCount, &racer->lastLapTime, &racer->fastestLap,
                                       &racer->lapStartTime, &racer->hasFinished, lapLimit, unlimited,
                                       racer->isRemote, racer->forceGatePass))
                return 0;
            if (g_TrackGame->bajaEnduroEvent && gate != racer->lapGate) {
                racer->nextLapGate = (racer->lapGate + 1) % g_TrackGame->bajaEnduroEvent->gateCount;
                if (racer == (KrustyBike*)((KrustyBikeCamera*)racer->race->raceCamera)->krustyBike)
                    g_TrackGame->bajaEnduroEvent->SelectLapGates(racer->lapGate, racer->nextLapGate,
                                                                              racer);
            }
        }
        if (racer->hasFinished && racer == racer->race->localRacer)
            g_ViewRacerFinished = 1;
        float before = Length(status->nextGate->center - status->currentPosition);
        float after = Length(status->nextGate->center - status->previousPosition);
        status->currentPosition.x = status->previousPosition.x;
        status->currentPosition.y = status->previousPosition.y;
        status->currentPosition.z = status->previousPosition.z;
        status->progressTotal += before - after;
        status->goalDistance = after;
        if (g_TrackGame->playerCount == 1 && g_ViewRacerFinished) {
            if (racer->hasFinished)
                goto next;
            if (racer->aiKind) {
                int remaining = lapLimit - racer->lapCount;
                if (remaining >= 1) {
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
                        UNKNOWN_RANDOM_UNIT() * 10 + racer->race->localRacer->lastLapTime + 1.0f;
                if (racer->lastLapTime < racer->fastestLap || racer->fastestLap <= 0.0f)
                    racer->fastestLap = racer->lastLapTime;
                racer->lapCount = lapLimit;
                racer->hasFinished = 1;
                continue;
            }
        }
        if (!racer->hasFinished)
            racer->totalRunningTime += frameTime;
next:;
    }
    if (!OrderByGates(list))
        return 0;
    return ComputeTimeBehind(*list, 1) != 0;
}
