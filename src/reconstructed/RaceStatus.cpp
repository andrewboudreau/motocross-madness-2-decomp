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
        if (!UnknownFunction47b670(from, to, gate, direction))
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
    if ((!skipTest && UnknownFunction47b670(from, to, *current, direction)) || (skipTest && forced)) {
        *current = (*current)->field_0x38;
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
    g_UnknownGlobal689c30[0] = (*current)->field_0x00 - (*current)->field_0x18;
    g_UnknownGlobal689c30[0].y += 3.0f;
    g_UnknownGlobal689c30[1] = (*current)->field_0x18 * 2.0f + g_UnknownGlobal689c30[0];
    g_UnknownGlobal689c48[0] = (*current)->field_0x00;
    g_UnknownGlobal689c48[0].y += 3.0f;
    g_UnknownGlobal689c48[1] = (*current)->field_0x0c * 10.0f + g_UnknownGlobal689c48[0];
    return 1;
}

// 0x004e5c70
int HasStatusNode(UnknownEventRacerPart* list, UnknownEventRacer* racer) {
    for (; list; list = list->field_0x50) {
        if (list->field_0x04 == racer)
            return 1;
    }
    return 0;
}

// 0x004e5ca0
int PruneStatusList(UnknownEventRacerPart** list) {
    while (list && *list) {
        if (!(*list)->field_0x04->field_0x25_bit0) {
            (*list)->field_0x04->field_0x744 = 0;
            UnknownEventRacerPart* status = *list;
            *list = status->field_0x50;
            DebugFree(status, __FILE__, 193);
        } else {
            list = &(*list)->field_0x50;
        }
    }
    return 1;
}

// 0x004e5f10
int FreeStatusList(UnknownEventRacerPart* list) {
    while (list) {
        UnknownEventRacerPart* status = list;
        list = list->field_0x50;
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
    for (status = *list; status; status = status->field_0x50) {
        status->field_0x0c = track->UnknownFunction517da0(status->field_0x38, probe->field_0x2c);
        if (status->field_0x0c < 0.0f)
            return 0;
        float distance = track->UnknownFunction517da0(status->field_0x38, status->field_0x44);
        if (distance > 0.0f && (distance < status->field_0x10 || status->field_0x10 < 0.0f) &&
            distance > status->field_0x0c)
            status->field_0x18 = -1;
        else
            status->field_0x18 = 0;
    }
    for (at = list; *at; ) {
        if (!(*at)->field_0x50)
            break;
        if (!(*at)->field_0x04->field_0x7a4) {
            key = (*at)->field_0x04->field_0x7a0 + (*at)->field_0x18;
            for (next = &(*at)->field_0x50; (status = *next) != 0; ) {
                int other = status->field_0x04->field_0x7a0 + status->field_0x18;
                if (other > key || (other == key && status->field_0x0c < (*at)->field_0x0c)) {
                    *next = status->field_0x50;
                    status->field_0x50 = *at;
                    *at = status;
                    key = status->field_0x04->field_0x7a0 + status->field_0x18;
                }
                if (*next)
                    next = &(*next)->field_0x50;
            }
        }
        if (*at)
            at = &(*at)->field_0x50;
    }
    for (status = *list, i = 1; status; status = status->field_0x50, i++)
        status->field_0x04->field_0x784 = i;
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
    for (status = *list; status; status = status->field_0x50) {
        float dx = status->field_0x34->field_0x00.x - status->field_0x1c.x;
        float dz = status->field_0x34->field_0x00.z - status->field_0x1c.z;
        status->field_0x0c = dx * dx + dz * dz;
        if (status->field_0x0c < 0.0f)
            return 0;
    }
    for (at = list; *at; ) {
        if (!(*at)->field_0x50)
            break;
        if (!(*at)->field_0x04->field_0x7a4) {
            key = (*at)->field_0x04->field_0x790;
            for (next = &(*at)->field_0x50; (status = *next) != 0; ) {
                int other = status->field_0x04->field_0x790;
                if (other > key || (other == key && status->field_0x0c < (*at)->field_0x0c)) {
                    *next = status->field_0x50;
                    status->field_0x50 = *at;
                    *at = status;
                    key = status->field_0x04->field_0x790;
                }
                if (*next)
                    next = &(*next)->field_0x50;
            }
        }
        if (*at)
            at = &(*at)->field_0x50;
    }
    for (status = *list, i = 1; status; status = status->field_0x50, i++)
        status->field_0x04->field_0x784 = i;
    return 1;
}

// 0x004e6210
int ComputeTimeBehind(UnknownEventRacerPart* list, int mode) {
    UnknownEventRacerPart* status;

    for (status = list; status; status = status->field_0x50) {
        UnknownEventRacer* racer = status->field_0x04;
        if (racer->field_0x784 > 1) {
            if (mode == 1)
                racer->field_0x770 = racer->field_0x780[racer->field_0x790 % 600] -
                                     list->field_0x04->field_0x780[racer->field_0x790 % 600];
            else
                racer->field_0x770 = racer->field_0x77c[racer->field_0x7a0 % 100] -
                                     list->field_0x04->field_0x77c[racer->field_0x7a0 % 100];
            if (status->field_0x04->field_0x770 >= 0.0f)
                continue;
        }
        status->field_0x04->field_0x770 = 0;
    }
    return 1;
}

// 0x004e62d0
int RankByScore(int keepRacing) {
    UnknownEventScore scores[11];
    int iterator;
    int count;
    int i;

    if (g_TrackGame->mode.field_0x27f8.field_0x04 == 0) {
        count = 0;
        UnknownKrustyBikeView* view = g_TrackGame->field_0x55c->field_0x34;
        UnknownEventRacer* racer;
        iterator = 0;
        for (racer = view->UnknownFunction4204e0(&iterator); racer; racer = view->UnknownFunction4204e0(&iterator)) {
            if (racer->field_0x25_bit0) {
                scores[count].value = racer->field_0x768;
                scores[count].racer = racer;
                count++;
            }
        }
        qsort(scores, count, sizeof(UnknownEventScore), UnknownFunction5199f0);
        for (i = 0; i < count; i++)
            scores[i].racer->field_0x784 = i + 1;
        if (!keepRacing) {
            TrackGameViewOwner* owner = g_TrackGame->field_0x55c;
            if (owner->field_0x70 * 60.0f + owner->field_0x74 >=
                g_TrackGame->mode.field_0x27f8.field_0x140 * 60.0f - 1.0f) {
                for (i = 0; i < count; i++)
                    scores[i].racer->field_0x7a4 = 1;
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
    for (status = *list; status; previous = status, status = status->field_0x50) {
        racer = status->field_0x04;
        if (!racer->field_0x7a4) {
            status->field_0x1c = racer->field_0x088 * 3.5f + racer->field_0x00c;
            if (!status->field_0x38.node) {
                status->field_0x44.node = start->field_0x2c.node;
                status->field_0x44.segment = start->field_0x2c.segment;
                status->field_0x44.t = start->field_0x2c.t + 0.1f;
            }
            if (!racer->field_0x735)
                racer->field_0x78c = track->UnknownFunction517340(status->field_0x1c, status->field_0x44,
                                                                  g_UnknownGlobal572988, g_UnknownGlobal57298c, 0,
                                                                  &status->field_0x38);
            else
                track->UnknownFunction516980(status->field_0x1c, &status->field_0x38, 0);
            if (racer->field_0x78c) {
                racer->field_0x76c = 0;
                status->field_0x08 = track->UnknownFunction517da0(status->field_0x44, finish->field_0x2c);
                status->field_0x0c = track->UnknownFunction517da0(status->field_0x38, finish->field_0x2c);
                if (status->field_0x08 < g_UnknownGlobal572988 && status->field_0x08 < status->field_0x0c)
                    COPY_TRACK_POS(status->field_0x38, status->field_0x44);
                if ((racer->field_0x735 && racer->field_0x7c0) || g_UnknownGlobal572988 < 0.0f ||
                    (!racer->field_0x735 && status->field_0x08 >= 0.0f && status->field_0x08 < g_UnknownGlobal572988)) {
                    if (CountLap(&status->field_0x1c, &status->field_0x28, &racer->field_0x7a0, finish,
                                              &racer->field_0x7a4, lapLimit, &racer->field_0x74c, racer->field_0x77c,
                                              racer->field_0x754, &racer->field_0x774, &racer->field_0x750, 1,
                                              unlimited, racer->field_0x735, racer->field_0x7c0)) {
                        status->field_0x44.node = finish->field_0x2c.node;
                        status->field_0x44.segment = finish->field_0x2c.segment;
                        status->field_0x44.t = finish->field_0x2c.t + 1e-5f;
                    }
                }
                if (racer->field_0x7a4 && racer == racer->field_0x740->field_0x38)
                    g_ViewRacerFinished = 1;
                status->field_0x10 = track->UnknownFunction517da0(status->field_0x44, status->field_0x38);
                if (status->field_0x10 > 0.0f && status->field_0x10 < g_UnknownGlobal572988 &&
                    status->field_0x10 < status->field_0x08) {
                    status->field_0x14 += status->field_0x10;
                    COPY_TRACK_POS(status->field_0x44, status->field_0x38);
                }
            } else {
                racer->field_0x76c += frameTime;
            }
            status->field_0x28.x = status->field_0x1c.x;
            status->field_0x28.y = status->field_0x1c.y;
            status->field_0x28.z = status->field_0x1c.z;
            if (g_TrackGame->field_0x18 == 1 && g_ViewRacerFinished) {
                if (racer->field_0x7a4)
                    goto next;
                if (racer->field_0x734) {
                    int remaining = lapLimit - racer->field_0x7a0;
                    if (remaining >= 1) {
                        if (racer->field_0x7a0 < 1)
                            racer->field_0x754 = 0;
                        else
                            racer->field_0x754 = remaining * racer->field_0x74c;
                    }
                    if (previous && racer->field_0x754 < previous->field_0x04->field_0x754)
                        racer->field_0x754 = UNKNOWN_RANDOM_UNIT() * 10 + previous->field_0x04->field_0x754 + 1.0f;
                    if (lapLimit == 1)
                        racer->field_0x74c = racer->field_0x754;
                    else if (racer->field_0x7a0 < 1)
                        racer->field_0x74c = UNKNOWN_RANDOM_UNIT() * 10 +
                                             racer->field_0x740->field_0x38->field_0x74c + 1.0f;
                    if (racer->field_0x74c < racer->field_0x750 || racer->field_0x750 <= 0.0f)
                        racer->field_0x750 = racer->field_0x74c;
                    racer->field_0x7a0 = lapLimit;
                    racer->field_0x7a4 = 1;
                    goto next;
                }
            }
            if (!racer->field_0x7a4)
                racer->field_0x754 += frameTime;
        }
    next:
        if (status->field_0x44.node && status->field_0x44.segment && status->field_0x44.segment->field_0x2c) {
            if (!track->UnknownFunction518080(status->field_0x44, (TrackVec3*)&status->field_0x04->field_0x10c))
                racer->field_0x10c = racer->field_0x00c;
            if (!track->UnknownFunction518130(status->field_0x44.segment,
                                              (TrackVec3*)&status->field_0x04->field_0x118))
                racer->field_0x118 = racer->field_0x088;
            status->field_0x04->field_0x10c -= status->field_0x04->field_0x118 * 3.5f;
        }
    }
    if (!OrderByLapDistance(list, track, finish))
        return 0;
    return ComputeTimeBehind(*list, 0) != 0;
}
