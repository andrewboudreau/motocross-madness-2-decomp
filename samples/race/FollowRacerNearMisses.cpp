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
// forms tried move the compare or the stores instead, as do const-reference
// and by-value inline max helpers, the ternary on the locals or on
// const-reference aliases of the fields, and the fields read directly in the
// ternary; a `keep` local for the selected value, block-scoping the two
// locals and the /G6 and /ML profiles leave the fstp where it is.
// Session 10 also left it in place with an inline StartRole wrapper, a
// slots pointer local, a by-pointer BankRun helper, a reference alias of
// the racer, the zero store folded into an argument or as an int store,
// and bool/char/short/pointer/float types for the slot 50 arguments.

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
void TrackGameViewOwner::PickFollowedRacer()
{
    if (!g_TrackGame->network || !g_TrackGame->network->isHost)
        return;
    int racing = 0;
    int iterator = 0;
    int tries = 0;
    UnknownEventRacer* last = 0;
    UnknownEventRacer* racer = raceView->NextBike(&iterator);
    while (racer) {
        if (racer->outOfRace == 0) {
            last = racer;
            racing++;
        }
        racer = raceView->NextBike(&iterator);
    }
    if (racing == 0) {
        followedRacer = raceView->localRacer;
    } else if (racing == 1) {
        followedRacer = last;
    } else {
        int count = raceView->racerCount;
        int index = (int)(UNKNOWN_RANDOM_UNIT() * count);
        while (raceView->racerSlots[index]->outOfRace == 1 && tries < count) {
            index++;
            if (index >= count)
                index = 0;
            tries++;
        }
        followedRacer = raceView->racerSlots[index];
    }
    UnknownBikeRaceVcrFollow follow;
    follow.field_0x04 = 0;
    follow.followedRacerId = followedRacer->networkPlayerId;
    g_TrackGame->network->Send(0x87, &follow, sizeof(follow), g_TrackGame->network->localPlayer, 0);
    if (raceView->replayVcr && g_TrackGame->mode.saveReplay && !g_TrackGame->playingReplay)
        ((VCRInterface*)raceView->replayVcr)->QueueRecord(0x87, followedRacer->networkPlayerId, &follow, 1);
}

// 0x004a9e80: switches the followed racer from `previous` to `racer`. The
// previous racer banks its run (tagRun into totalTagRun, the best into bestTagRun),
// both racers get their tier-3 role for five seconds and every other racer
// hears of it; the view's own racer taking over scores 2500.
void TrackGameViewOwner::SetFollowedRacer(UnknownEventRacer* racer, UnknownEventRacer* previous, int tell)
{
    if (previous) {
        previous->totalTagRun += previous->tagRun;
        // Compared through locals, as EventManager.cpp keeps the best run.
        float best = previous->bestTagRun;
        float run = previous->tagRun;
        previous->bestTagRun = best > run ? previous->bestTagRun : previous->tagRun;
        previous->tagRun = 0.0f;
        FOLLOWED_RACER_SLOTS(previous)->UnknownVirtualSlot50(1, 5.0f, 0);
        if (ball)
            ball->field_0x10a = 0;
    }
    *(float*)&field_0xac = 5.0f;
    if (tell)
        raceView->localRacer->UnknownFunction4925a0(racer, false);
    if (racer) {
        ((RaceSound*)raceView->raceSound)->PlayItSound();
        FOLLOWED_RACER_SLOTS(racer)->UnknownVirtualSlot50(1, 5.0f, 0);
    }
    AnnounceFollowedRacer(racer, previous);
    if (racer == raceView->localRacer && previous && previous != racer) {
        racer->score += 2500.0f;
        if (raceView->scoreBoard)
            raceView->scoreBoard->ShowValue(2500.0f, 1.0f);
    }
    followedRacer = racer;
}
