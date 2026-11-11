// Link bring-up definition (best-guess admission, see docs/GAME_BUILD.md):
// KbPlayer::Fn_004A9E80 = TrackGameViewOwner::SetFollowedRacer (0x004a9e80, 0x110
// bytes, ret 0xc), FollowRacer.cpp's code.  KrustyBike.cpp reaches it through its
// KbPlayer view (src/krusty2/vehicle/KrustyBikeTypes.h); the near-miss body for the
// TrackGameViewOwner view is in samples/race/FollowRacerNearMisses.cpp.
//
// Decoded retail behavior (tier 1 offsets, tier 3 names):
//   previous: totalTagRun(+0x764) += tagRun(+0x75c); bestTagRun(+0x760) =
//             max(bestTagRun, tagRun); tagRun = 0; previous->slot 50(1, 5.0f, 0);
//             if ball(+0xdc) ball->+0x10a = 0
//   this+0xac = 5.0f
//   tell:     raceView(+0x34)->localRacer(+0x38)->0x004925a0(racer, false)
//   racer:    raceView->raceSound(+0x44)->0x004e5860(); racer->slot 50(1, 5.0f, 0)
//   0x004a9f90(racer, previous)
//   racer == localRacer && previous && previous != racer:
//             racer->score(+0x768) += 2500.0f (0x00550bb8);
//             raceView->scoreBoard(+0xbc)->0x0048d1e0(2500.0f, 1.0f) when set
//   followedRacer(+0xa8) = racer
#include "vehicle/KrustyBike.h"
#include "RaceSound.h"

struct UnknownEventRacer;

// The owner's FollowRacer.cpp helper is defined on the TrackGameViewOwner view
// (src/reconstructed/FollowRacer.cpp); only that member is declared here.
struct TrackGameViewOwner {
    void AnnounceFollowedRacer(UnknownEventRacer* racer, UnknownEventRacer* previous);  // 0x004a9f90
};

// Raw view of the race view at owner +0x34 (RaceView.h UnknownKrustyBikeView).
struct Group3FollowRaceView {
    char pad_0x00[0x38];
    KrustyBike* localRacer;      // +0x38
    char pad_0x3c[0x44 - 0x3c];
    RaceSound* raceSound;        // +0x44
    char pad_0x48[0xbc - 0x48];
    KbScoreBoard* scoreBoard;    // +0xbc
};

// Retail reads the race view from +0x34 afresh at every use (0x4a9f0d,
// 0x4a9f1f, 0x4a9f46, 0x4a9f68) rather than caching it across the calls.
// With that and the best-run locals the body is 272/272 bytes, 78/78
// instructions (linked masked score 0.735 -> 0.974); what remains is the
// fstp-after-first-push scheduling difference recorded in the sample.
#define RACE_VIEW() (*(Group3FollowRaceView**)((char*)this + 0x34))

void KbPlayer::Fn_004A9E80(Vehicle* a, Vehicle* b, int c)
{
    KrustyBike* racer = (KrustyBike*)a;
    KrustyBike* previous = (KrustyBike*)b;

    if (previous) {
        previous->totalTagRun += previous->tagRun;
        // Compared through locals, as the TrackGameViewOwner near miss in
        // samples/race/FollowRacerNearMisses.cpp (retail fld best; fld run;
        // fcompp at 0x4a9e9f).
        float best = previous->bestTagRun;
        float run = previous->tagRun;
        previous->bestTagRun = best > run ? previous->bestTagRun : previous->tagRun;
        previous->tagRun = 0.0f;
        previous->SetSpawnProtection(1, 5.0f, 0);
        if (ball)
            ball->asleep = 0;
    }
    *(float*)((char*)this + 0xac) = 5.0f;
    if (c)
        RACE_VIEW()->localRacer->Fn_004925A0(racer, false);
    if (racer) {
        RACE_VIEW()->raceSound->PlayItSound();
        racer->SetSpawnProtection(1, 5.0f, 0);
    }
    ((TrackGameViewOwner*)this)->AnnounceFollowedRacer((UnknownEventRacer*)racer,
                                                       (UnknownEventRacer*)previous);
    if (racer == RACE_VIEW()->localRacer && previous && previous != racer) {
        racer->score += 2500.0f;
        if (RACE_VIEW()->scoreBoard)
            RACE_VIEW()->scoreBoard->ShowValue(2500.0f, 1.0f);
    }
    followedRacer = racer;
}

#undef RACE_VIEW
