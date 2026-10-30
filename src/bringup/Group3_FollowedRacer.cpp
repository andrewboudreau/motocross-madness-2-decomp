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

void KbPlayer::Fn_004A9E80(Vehicle* a, Vehicle* b, int c)
{
    KrustyBike* racer = (KrustyBike*)a;
    KrustyBike* previous = (KrustyBike*)b;
    Group3FollowRaceView* view = *(Group3FollowRaceView**)((char*)this + 0x34);

    if (previous) {
        previous->totalTagRun += previous->tagRun;
        previous->bestTagRun = previous->tagRun < previous->bestTagRun
                                   ? previous->bestTagRun : previous->tagRun;
        previous->tagRun = 0.0f;
        previous->UnknownVirtualSlot50(1, 5.0f, 0);
        if (ball)
            ball->asleep = 0;
    }
    *(float*)((char*)this + 0xac) = 5.0f;
    if (c)
        view->localRacer->Fn_004925A0(racer, false);
    if (racer) {
        view->raceSound->PlayItSound();
        racer->UnknownVirtualSlot50(1, 5.0f, 0);
    }
    ((TrackGameViewOwner*)this)->AnnounceFollowedRacer((UnknownEventRacer*)racer,
                                                       (UnknownEventRacer*)previous);
    view = *(Group3FollowRaceView**)((char*)this + 0x34);
    if (racer == view->localRacer && previous && previous != racer) {
        racer->score += 2500.0f;
        if (view->scoreBoard)
            view->scoreBoard->ShowValue(2500.0f, 1.0f);
    }
    followedRacer = racer;
}
