// Followed-racer and tag ball helpers, 0x004a9aa0..0x004aa001 (provisional file
// name; no __FILE__ or RTTI). The run sits between Motnctrl.cpp (last xref
// 0x004a9a87) and MSZoneInterface.cpp (first xref 0x004aa36c). Two views of
// the race view owner's objects are implemented here: the tag ball at
// TrackGameViewOwner+0xdc (BikeRace.cpp 0x00422dec applies replay deltas
// through 0x004a9aa0; 0x00420a91 hands it the recorder) and the owner itself
// (DlgProcs.cpp 0x0042062a/0x004527ef pick the followed racer; KrustyBike.cpp
// 0x0048db13/0x00497986 switch it). The picker 0x004a9d20 is a near miss in
// samples/race/FollowRacerNearMisses.cpp; the switch 0x004a9e80 is here.

#include "BikeRace.h"
#include "TrackGame.h"
#include "../krusty2/vehicle/KrustyBikeCore.h"
#include "RaceSound.h"
#include "Krusty3DObjects.h"   // BonusObjectManager (the score board)

// Replay angle delta step. Retail holds 0.0490873456f (0x0055534c), which is
// 3.14159f / 64 exactly, not the correctly rounded pi / 64 that KrustyBike.cpp
// 0x004933e0 uses; the spelling is inferred from the bits.
#define GHOST_ANGLE_STEP (3.14159f / 64.0f)

// 0x004a9aa0: applies a kind-0x10 replay delta to the ball's state and
// copies the state into `part` (the same shape as KrustyBike.cpp 0x004933e0).
void TagBall::ApplyReplayDelta(const TagBallStateDelta* delta,
                               TagBallReceivedState* part)
{
    Vector3 d0(delta->velocity[0] * 0.234375f, delta->velocity[1] * 0.234375f, delta->velocity[2] * 0.234375f);
    Vector3 d1(delta->position[0] * 0.234375f, delta->position[1] * 0.234375f, delta->position[2] * 0.234375f);
    Vector3 d2(delta->angularVelocity[0] * GHOST_ANGLE_STEP, delta->angularVelocity[1] * GHOST_ANGLE_STEP, delta->angularVelocity[2] * GHOST_ANGLE_STEP);
    velocity += d0;
    position += d1;
    angularVelocity += d2;
    roll += delta->angles[0] * GHOST_ANGLE_STEP;
    pitch += delta->angles[1] * GHOST_ANGLE_STEP;
    yaw += delta->angles[2] * GHOST_ANGLE_STEP;
    int step;
    if (delta->step & 1)
        step = (delta->step >> 1) * 8;
    else
        step = delta->step >> 1;
    part->record.timeStep = step;
    runningTime += part->record.timeStep;
    part->record.runningTime = runningTime;
    part->record.velocity = velocity;
    part->record.position = position;
    part->record.angularVelocity = angularVelocity;
    part->record.roll = roll;
    part->record.pitch = pitch;
    part->record.yaw = yaw;
}

// 0x004a9d10
void TagBall::SetRecorder(KrustyVCR* vcr)
{
    recorder = vcr;
}

// 0x004a9e80: switches the followed racer from `previous` to `racer`. The
// previous racer banks its run (tagRun into totalTagRun, the best into
// bestTagRun), both racers get their spawn protection for five seconds and
// every other racer hears of it; the view's own racer taking over scores 2500.
// Status: near miss (259/272, 77/78 instructions). After the best-run ternary
// retail pushes the three arguments of the virtual call before the `fstp`
// into +0x760; VC6 here stores after the first push. The forms tried are
// listed in samples/race/FollowRacerNearMisses.cpp.
void TrackGameViewOwner::SetFollowedRacer(KrustyBike* racer, KrustyBike* previous, int tell)
{
    if (previous) {
        previous->totalTagRun += previous->tagRun;
        // Compared through locals, as EventManager.cpp keeps the best run
        // (retail fld best; fld run; fcompp at 0x4a9e9f).
        float best = previous->bestTagRun;
        float run = previous->tagRun;
        previous->bestTagRun = best > run ? previous->bestTagRun : previous->tagRun;
        previous->tagRun = 0.0f;
        previous->SetSpawnProtection(1, 5.0f, 0);
        if (ball)
            ball->asleep = 0;
    }
    // +0xac is a float in the bike race owner (TrackGame.h).
    *(float*)&gateCount = 5.0f;
    // Retail reads raceView from +0x34 afresh at every use (0x4a9f0d,
    // 0x4a9f1f, 0x4a9f46, 0x4a9f68).
    if (tell)
        raceView->localRacer->SendFollowedRacer(racer, false);
    if (racer) {
        raceView->raceSound->PlayItSound();
        racer->SetSpawnProtection(1, 5.0f, 0);
    }
    AnnounceFollowedRacer(racer, previous);
    if (racer == raceView->localRacer && previous && previous != racer) {
        racer->score += 2500.0f;
        if (raceView->scoreBoard)
            raceView->scoreBoard->ShowValue(2500.0f, 1.0f);
    }
    followedRacer = racer;
}

// 0x004a9f90: every other racer hears about the new and the previous racer,
// and the new racer hears about each of them.
void TrackGameViewOwner::AnnounceFollowedRacer(KrustyBike* racer, KrustyBike* previous)
{
    int iterator = 0;
    for (KrustyBike* other = reinterpret_cast<KrustyBike*>(raceView->NextBike(&iterator)); other;
         other = reinterpret_cast<KrustyBike*>(raceView->NextBike(&iterator))) {
        if (other == racer)
            continue;
        if (racer)
            reinterpret_cast<KrustyBike*>(racer)->IgnoreCollisionsWith(reinterpret_cast<KrustyBike*>(other));
        if (previous && previous != other)
            reinterpret_cast<KrustyBike*>(other)->IgnoreCollisionsWith(reinterpret_cast<KrustyBike*>(previous));
        if (racer)
            reinterpret_cast<KrustyBike*>(other)->IgnoreCollisionsWith(reinterpret_cast<KrustyBike*>(racer));
    }
}
