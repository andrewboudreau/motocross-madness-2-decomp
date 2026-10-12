#include "RaceStatus.h"

#include <stdlib.h>

#include "BikeRace.h"
#include "DebugAlloc.h"
#include "EventManager.h"
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
Vector3 g_UnknownGlobal689c30[2], g_UnknownGlobal689c48[2];
int g_ViewRacerFinished;

// 0x00572988 and 0x0057298c (.data, before this file's __FILE__): the
// track-search distance and range of 0x004e63e0.
float g_UnknownGlobal572988 = 100.0f;
float g_UnknownGlobal57298c = 50.0f;

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
int CountLap(Vector3* from, Vector3* to, unsigned short* laps, UnknownBikeRaceNode* gate,
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
int AdvanceGate(Vector3* from, Vector3* to, UnknownBikeRaceNode** current, UnknownBikeRaceNode* first,
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
    g_UnknownGlobal689c30[0] = (*current)->center - (*current)->halfExtents;
    g_UnknownGlobal689c30[0].y += 3.0f;
    g_UnknownGlobal689c30[1] = (*current)->halfExtents * 2.0f + g_UnknownGlobal689c30[0];
    g_UnknownGlobal689c48[0] = (*current)->center;
    g_UnknownGlobal689c48[0].y += 3.0f;
    g_UnknownGlobal689c48[1] = (*current)->direction * 10.0f + g_UnknownGlobal689c48[0];
    return 1;
}

// 0x004e5c70
int HasStatusNode(UnknownEventRacerPart* list, UnknownEventRacer* racer) {
    for (; list; list = list->next) {
        if (list->racer == racer)
            return 1;
    }
    return 0;
}

// 0x004e5ca0
int PruneStatusList(UnknownEventRacerPart** list) {
    while (list && *list) {
        if (!(*list)->racer->isEnabled) {
            (*list)->racer->statusNode = 0;
            UnknownEventRacerPart* status = *list;
            *list = status->next;
            DebugFree(status, __FILE__, 193);
        } else {
            list = &(*list)->next;
        }
    }
    return 1;
}

// 0x004e5f10
int FreeStatusList(UnknownEventRacerPart* list) {
    while (list) {
        UnknownEventRacerPart* status = list;
        list = list->next;
        DebugFree(status, __FILE__, 285);
    }
    return 0;
}

// 0x004e5f40
int OrderByLapDistance(UnknownEventRacerPart** list, Track* track, UnknownBikeRaceNode* probe) {
    UnknownEventRacerPart* status;
    UnknownEventRacerPart** at;
    UnknownEventRacerPart** next;
    int key;
    int i;

    if (!list || !track)
        return 0;
    for (status = *list; status; status = status->next) {
        status->gateDistance = track->DistanceAlong(status->placedPos, probe->field_0x2c);
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
int OrderByGates(UnknownEventRacerPart** list) {
    UnknownEventRacerPart* status;
    UnknownEventRacerPart** at;
    UnknownEventRacerPart** next;
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
int ComputeTimeBehind(UnknownEventRacerPart* list, int mode) {
    UnknownEventRacerPart* status;

    for (status = list; status; status = status->next) {
        UnknownEventRacer* racer = status->racer;
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
    UnknownEventScore scores[11];
    int iterator;
    int count;
    int i;

    if (g_TrackGame->mode.raceSettings.eventType == 0) {
        count = 0;
        UnknownKrustyBikeView* view = g_TrackGame->quarryEvent->raceView;
        UnknownEventRacer* racer;
        iterator = 0;
        for (racer = view->NextBike(&iterator); racer; racer = view->NextBike(&iterator)) {
            if (racer->isEnabled) {
                scores[count].value = racer->score;
                scores[count].racer = racer;
                count++;
            }
        }
        qsort(scores, count, sizeof(UnknownEventScore), CompareScoresDescending);
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
int UpdateLapRace(UnknownEventRacerPart** list, Track* track, float frameTime, UnknownBikeRaceNode* start,
                          UnknownBikeRaceNode* finish, int lapLimit, int unlimited) {
    UnknownEventRacerPart* previous = 0;
    UnknownEventRacerPart* status;
    UnknownEventRacer* racer;

    if (!list || !track)
        return 0;
    PruneStatusList(list);
    for (status = *list; status; previous = status, status = status->next) {
        racer = status->racer;
        if (!racer->hasFinished) {
            status->previousPosition = racer->field_0x088 * 3.5f + racer->position;
            if (!status->placedPos.node) {
                status->progressPos.node = start->field_0x2c.node;
                status->progressPos.segment = start->field_0x2c.segment;
                status->progressPos.t = start->field_0x2c.t + 0.1f;
            }
            if (!racer->field_0x735)
                racer->onTrack = track->PlaceNear(status->previousPosition, status->progressPos,
                                                                  g_UnknownGlobal572988, g_UnknownGlobal57298c, 0,
                                                                  &status->placedPos);
            else
                track->ClosestPointOnTrack(status->previousPosition, &status->placedPos, 0);
            if (racer->onTrack) {
                racer->offTrackTime = 0;
                status->field_0x08 = track->DistanceAlong(status->progressPos, finish->field_0x2c);
                status->gateDistance = track->DistanceAlong(status->placedPos, finish->field_0x2c);
                if (status->field_0x08 < g_UnknownGlobal572988 && status->field_0x08 < status->gateDistance)
                    COPY_TRACK_POS(status->placedPos, status->progressPos);
                if ((racer->field_0x735 && racer->field_0x7c0) || g_UnknownGlobal572988 < 0.0f ||
                    (!racer->field_0x735 && status->field_0x08 >= 0.0f && status->field_0x08 < g_UnknownGlobal572988)) {
                    if (CountLap(&status->previousPosition, &status->currentPosition, &racer->lapCount, finish,
                                              &racer->hasFinished, lapLimit, &racer->lastLapTime, racer->lapStartTimes,
                                              racer->totalRunningTime, &racer->lapStartTime, &racer->fastestLap, 1,
                                              unlimited, racer->field_0x735, racer->field_0x7c0)) {
                        status->progressPos.node = finish->field_0x2c.node;
                        status->progressPos.segment = finish->field_0x2c.segment;
                        status->progressPos.t = finish->field_0x2c.t + 1e-5f;
                    }
                }
                if (racer->hasFinished && racer == racer->bikeView->localRacer)
                    g_ViewRacerFinished = 1;
                status->progressStep = track->DistanceAlong(status->progressPos, status->placedPos);
                if (status->progressStep > 0.0f && status->progressStep < g_UnknownGlobal572988 &&
                    status->progressStep < status->field_0x08) {
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
                if (racer->field_0x734) {
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
                                             racer->bikeView->localRacer->lastLapTime + 1.0f;
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
            if (!track->PointAt(status->progressPos, (TrackVec3*)&status->racer->trackPoint))
                racer->trackPoint = racer->position;
            if (!track->SegmentDirection(status->progressPos.segment,
                                              (TrackVec3*)&status->racer->trackDirection))
                racer->trackDirection = racer->field_0x088;
            status->racer->trackPoint -= status->racer->trackDirection * 3.5f;
        }
    }
    if (!OrderByLapDistance(list, track, finish))
        return 0;
    return ComputeTimeBehind(*list, 0) != 0;
}

// The view's camera (+0x50), whose +0x3b4 racer is the one followed (as
// RaceSound.h's UnknownRaceSoundCamera; KrustyBikeCamera keeps it protected).
struct UnknownRaceStatusCamera {
    unsigned char field_0x000[0x3b4];
    UnknownEventRacer* followedRacer;         // +0x3b4 (RaceSound.h)
};

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
            int gate = racer->lapGate;
            status->previousPosition.x = racer->frontWheel->field_0x200.x;
            status->previousPosition.y = racer->frontWheel->field_0x200.y;
            status->previousPosition.z = racer->frontWheel->field_0x200.z;
            if (!AdvanceGate(&status->previousPosition, &status->currentPosition, &status->nextGate, firstGate,
                                       &racer->gatesPassed, &racer->lapGate, racer->gateTimes, racer->totalRunningTime,
                                       0, &racer->lapCount, &racer->lastLapTime, &racer->fastestLap,
                                       &racer->lapStartTime, &racer->hasFinished, lapLimit, unlimited,
                                       racer->field_0x735, racer->field_0x7c0))
                return 0;
            if (g_TrackGame->bajaEnduroEvent && gate != racer->lapGate) {
                racer->nextLapGate = (racer->lapGate + 1) % g_TrackGame->bajaEnduroEvent->field_0xac;
                if (racer == ((UnknownRaceStatusCamera*)racer->bikeView->raceCamera)->followedRacer)
                    g_TrackGame->bajaEnduroEvent->UnknownFunction404df0(racer->lapGate, racer->nextLapGate,
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
        status->progressTotal += before - after;
        status->field_0x08 = after;
        if (g_TrackGame->playerCount == 1 && g_ViewRacerFinished) {
            if (racer->hasFinished)
                goto next;
            if (racer->field_0x734) {
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
next:;
    }
    if (!OrderByGates(list))
        return 0;
    return ComputeTimeBehind(*list, 1) != 0;
}
