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
// `>= 1` and if/else.
//
// BuildStatusList (0x004e5d00, 519 bytes): builds a view's status
// list. The statements, calls and loops line up; only register allocation
// differs: retail keeps the list pointer in ebp and the index in ebx (also
// reusing it for the constant 1 in the calloc count and the race-mode test)
// and keeps `tail` in the view argument's stack slot, while VC6 here folds
// the first index to 1, keeps `tail` in ebx and reloads `list`. Every
// declaration order, `tail = list` for the first node and an unsigned index
// leave that allocation.

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
    return UnknownFunction460b50(squared);
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
    status->field_0x04 = view->field_0x38;
    status->field_0x34 = view->field_0x0c8;
    view->field_0x38->field_0x744 = status;
    status->field_0x00 = index;
    status->field_0x28 = view->field_0x38->field_0x00c;
    *list = status;
    tail = &status->field_0x50;
    if (g_TrackGame->field_0x18 == 1 && g_TrackGame->mode.field_0x27f8.field_0x00 != 4) {
        for (i = 0; i < g_TrackGame->mode.field_0x27f8.field_0x24; i++) {
            if (!HasStatusNode(*list, view->field_0x40[i])) {
                status = (UnknownEventRacerPart*)DebugCalloc(1, sizeof(UnknownEventRacerPart), __FILE__, 235);
                if (!status)
                    return 0;
                index++;
                status->field_0x04 = view->field_0x40[i];
                status->field_0x34 = view->field_0x0c8;
                view->field_0x40[i]->field_0x744 = status;
                status->field_0x00 = index;
                status->field_0x28 = view->field_0x40[i]->field_0x00c;
                *tail = status;
                tail = &status->field_0x50;
            }
        }
    }
    if (view->field_0x3c) {
        for (i = 0; i < view->field_0x158; i++) {
            if (!HasStatusNode(*list, view->field_0x3c[i]) && view->field_0x3c[i] != view->field_0x38) {
                status = (UnknownEventRacerPart*)DebugCalloc(1, sizeof(UnknownEventRacerPart), __FILE__, 257);
                if (!status)
                    return 0;
                index++;
                status->field_0x04 = view->field_0x3c[i];
                status->field_0x34 = view->field_0x0c8;
                view->field_0x3c[i]->field_0x744 = status;
                status->field_0x00 = index;
                status->field_0x28 = view->field_0x3c[i]->field_0x00c;
                *tail = status;
                tail = &status->field_0x50;
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
    for (status = *list; status; previous = status, status = status->field_0x50) {
        racer = status->field_0x04;
        if (racer->field_0x7a4)
            continue;
        {
            int gate = racer->field_0x7b8;
            status->field_0x1c.x = racer->field_0x5f0->field_0x200.x;
            status->field_0x1c.y = racer->field_0x5f0->field_0x200.y;
            status->field_0x1c.z = racer->field_0x5f0->field_0x200.z;
            if (!AdvanceGate(&status->field_0x1c, &status->field_0x28, &status->field_0x34, firstGate,
                                       &racer->field_0x790, &racer->field_0x7b8, racer->field_0x780, racer->field_0x754,
                                       0, &racer->field_0x7a0, &racer->field_0x74c, &racer->field_0x750,
                                       &racer->field_0x774, &racer->field_0x7a4, lapLimit, unlimited,
                                       racer->field_0x735, racer->field_0x7c0))
                return 0;
            if (g_TrackGame->field_0x560 && gate != racer->field_0x7b8) {
                racer->field_0x7bc = (racer->field_0x7b8 + 1) % g_TrackGame->field_0x560->field_0xac;
                if (racer == ((UnknownRaceStatusCamera*)racer->field_0x740->field_0x50)->field_0x3b4)
                    g_TrackGame->field_0x560->UnknownFunction404df0(racer->field_0x7b8, racer->field_0x7bc,
                                                                              racer);
            }
        }
        if (racer->field_0x7a4 && racer == racer->field_0x740->field_0x38)
            g_ViewRacerFinished = 1;
        float before = Length(status->field_0x34->field_0x00 - status->field_0x28);
        float after = Length(status->field_0x34->field_0x00 - status->field_0x1c);
        status->field_0x28.x = status->field_0x1c.x;
        status->field_0x28.y = status->field_0x1c.y;
        status->field_0x28.z = status->field_0x1c.z;
        status->field_0x14 += before - after;
        status->field_0x08 = after;
        if (g_TrackGame->field_0x18 == 1 && g_ViewRacerFinished) {
            if (racer->field_0x7a4)
                continue;
            if (racer->field_0x734) {
                int remaining = lapLimit - racer->field_0x7a0;
                if (remaining > 0) {
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
                    racer->field_0x74c =
                        UNKNOWN_RANDOM_UNIT() * 10 + racer->field_0x740->field_0x38->field_0x74c + 1.0f;
                if (racer->field_0x74c < racer->field_0x750 || racer->field_0x750 <= 0.0f)
                    racer->field_0x750 = racer->field_0x74c;
                racer->field_0x7a0 = lapLimit;
                racer->field_0x7a4 = 1;
                continue;
            }
        }
        if (!racer->field_0x7a4)
            racer->field_0x754 += frameTime;
    }
    if (!OrderByGates(list))
        return 0;
    return ComputeTimeBehind(*list, 1) != 0;
}
