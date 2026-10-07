// Near-miss candidates for the followed-racer helpers, kept out of
// src/reconstructed until they match. See docs/FOLLOWRACER.md. The canonical
// file is included first so the TU-local declarations are the same.
//
// 0x004a9d20 (340 bytes, 20%): the whole function is retail's with two
// register pairs swapped: retail keeps `this` in edi and the still-racing
// count in esi (VC6 here: esi/edi), and retail's zero register ebp is the
// retry counter of the random pick (VC6 here merges the zero with the last
// racer seen, so the loop compares with `test` instead of `cmp reg, ebp`).
// All 24 declaration orders, hoisting any of the four locals above the host
// check, for/while/assign-in-condition loops, a cached racer array, a single
// pick variable, renaming and extra declarations in the TU leave both swaps.
//
// 0x004a9e80 (272 bytes, 95%): one scheduling difference. After the best-run
// ternary retail pushes the three arguments of the virtual call before the
// `fstp` into +0x760; VC6 here stores after the first push. The if/else,
// max-helper, local-result, int-zero, double-literal and statement-order
// forms tried move the compare or the stores instead.

#include <stdlib.h>

#include "../../src/reconstructed/FollowRacer.cpp"
#include "../../src/reconstructed/RaceSound.h"

// rand() scaled to [0, 1), as ProCircuit.cpp.
#define UNKNOWN_RANDOM_UNIT() (rand() * (1.0f / 32768.0f))

// The racer's own vtable (UnknownEventRacer+0; RaceView.h models only slot
// 0): 0x004a9e80 calls slot 50 (+0xc8) with (1, 5.0f, 0) on the racer it
// stops and the one it starts following (tier 3 roles).
struct UnknownFollowedRacerSlots {
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1(); virtual void UnknownVirtualSlot2(); virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4(); virtual void UnknownVirtualSlot5(); virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7(); virtual void UnknownVirtualSlot8(); virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10(); virtual void UnknownVirtualSlot11(); virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13(); virtual void UnknownVirtualSlot14(); virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16(); virtual void UnknownVirtualSlot17(); virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19(); virtual void UnknownVirtualSlot20(); virtual void UnknownVirtualSlot21();
    virtual void UnknownVirtualSlot22(); virtual void UnknownVirtualSlot23(); virtual void UnknownVirtualSlot24();
    virtual void UnknownVirtualSlot25(); virtual void UnknownVirtualSlot26(); virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28(); virtual void UnknownVirtualSlot29(); virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31(); virtual void UnknownVirtualSlot32(); virtual void UnknownVirtualSlot33();
    virtual void UnknownVirtualSlot34(); virtual void UnknownVirtualSlot35(); virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot37(); virtual void UnknownVirtualSlot38(); virtual void UnknownVirtualSlot39();
    virtual void UnknownVirtualSlot40(); virtual void UnknownVirtualSlot41(); virtual void UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot43(); virtual void UnknownVirtualSlot44(); virtual void UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot46(); virtual void UnknownVirtualSlot47(); virtual void UnknownVirtualSlot48();
    virtual void UnknownVirtualSlot49();
    virtual void UnknownVirtualSlot50(int a, float seconds, int b);
};
#define FOLLOWED_RACER_SLOTS(racer) ((UnknownFollowedRacerSlots*)(racer))

// 0x004a9d20: the host picks the racer to follow: the only one still racing
// (+0x4a0 clear), or a random still-racing one, or its own racer when none
// is; then tells the players (message 0x87) and the replay recorder.
void TrackGameViewOwner::UnknownFunction4a9d20()
{
    if (!g_TrackGame->network || !g_TrackGame->network->isHost)
        return;
    int racing = 0;
    int iterator = 0;
    int tries = 0;
    UnknownEventRacer* last = 0;
    UnknownEventRacer* racer = field_0x34->UnknownFunction4204e0(&iterator);
    while (racer) {
        if (racer->field_0x4a0 == 0) {
            last = racer;
            racing++;
        }
        racer = field_0x34->UnknownFunction4204e0(&iterator);
    }
    if (racing == 0) {
        field_0xa8 = field_0x34->field_0x38;
    } else if (racing == 1) {
        field_0xa8 = last;
    } else {
        int count = field_0x34->field_0x158;
        int index = (int)(UNKNOWN_RANDOM_UNIT() * count);
        while (field_0x34->field_0x3c[index]->field_0x4a0 == 1 && tries < count) {
            index++;
            if (index >= count)
                index = 0;
            tries++;
        }
        field_0xa8 = field_0x34->field_0x3c[index];
    }
    UnknownBikeRaceVcrFollow follow;
    follow.field_0x04 = 0;
    follow.field_0x08 = field_0xa8->field_0x11bc;
    g_TrackGame->network->Send(0x87, &follow, sizeof(follow), g_TrackGame->network->localPlayer, 0);
    if (field_0x34->field_0x1a0 && g_TrackGame->mode.field_0x2dbc && !g_TrackGame->field_0x3428)
        ((VCRInterface*)field_0x34->field_0x1a0)->QueueRecord(0x87, field_0xa8->field_0x11bc, &follow, 1);
}

// 0x004a9e80: switches the followed racer from `previous` to `racer`. The
// previous racer banks its run (+0x75c into +0x764, the best into +0x760),
// both racers get their tier-3 role for five seconds and every other racer
// hears of it; the view's own racer taking over scores 2500.
void TrackGameViewOwner::UnknownFunction4a9e80(UnknownEventRacer* racer, UnknownEventRacer* previous, int tell)
{
    if (previous) {
        previous->field_0x764 += previous->field_0x75c;
        // Compared through locals, as EventManager.cpp keeps the best of +0x75c.
        float best = previous->field_0x760;
        float run = previous->field_0x75c;
        previous->field_0x760 = best > run ? previous->field_0x760 : previous->field_0x75c;
        previous->field_0x75c = 0.0f;
        FOLLOWED_RACER_SLOTS(previous)->UnknownVirtualSlot50(1, 5.0f, 0);
        if (field_0xdc)
            field_0xdc->field_0x10a = 0;
    }
    field_0xac_seconds = 5.0f;
    if (tell)
        field_0x34->field_0x38->UnknownFunction4925a0(racer, false);
    if (racer) {
        ((RaceSound*)field_0x34->field_0x44)->UnknownFunction4e5860();
        FOLLOWED_RACER_SLOTS(racer)->UnknownVirtualSlot50(1, 5.0f, 0);
    }
    UnknownFunction4a9f90(racer, previous);
    if (racer == field_0x34->field_0x38 && previous && previous != racer) {
        racer->field_0x768 += 2500.0f;
        if (field_0x34->field_0x0bc)
            field_0x34->field_0x0bc->UnknownFunction48d1e0(2500.0f, 1.0f);
    }
    field_0xa8 = racer;
}
