// Followed-racer and ghost helpers, 0x004a9aa0..0x004aa001 (provisional file
// name; no __FILE__ or RTTI). The run sits between Motnctrl.cpp (last xref
// 0x004a9a87) and MSZoneInterface.cpp (first xref 0x004aa36c). Two views of
// the race view owner's objects are implemented here: the ghost bike at
// TrackGameViewOwner+0xdc (BikeRace.cpp 0x00422dec applies replay deltas
// through 0x004a9aa0; 0x00420a91 hands it the recorder) and the owner itself
// (DlgProcs.cpp 0x0042062a/0x004527ef pick the followed racer; KrustyBike.cpp
// 0x0048db13/0x00497986 switch it). The picker 0x004a9d20 and the switch
// 0x004a9e80 are near misses in samples/race/FollowRacerNearMisses.cpp.

#include "BikeRace.h"
#include "TrackGame.h"

// 0x004a9aa0: applies a kind-0x10 replay delta to the ghost's state and
// copies the state into `part` (the same shape as KrustyBike.cpp 0x004933e0).
void UnknownBikeRaceGhost::UnknownFunction4a9aa0(const UnknownBikeRaceGhostDelta* delta,
                                                 UnknownBikeRaceGhostPart* part)
{
    Vector3 d0(delta->delta0[0] * 0.234375f, delta->delta0[1] * 0.234375f, delta->delta0[2] * 0.234375f);
    Vector3 d1(delta->delta1[0] * 0.234375f, delta->delta1[1] * 0.234375f, delta->delta1[2] * 0.234375f);
    Vector3 d2(delta->delta2[0] * 0.049087387f, delta->delta2[1] * 0.049087387f, delta->delta2[2] * 0.049087387f);
    field_0x5d4 += d0;
    field_0x5e0 += d1;
    field_0x5ec += d2;
    field_0x5f8 += delta->delta3[0] * 0.049087387f;
    field_0x5fc += delta->delta3[1] * 0.049087387f;
    field_0x600 += delta->delta3[2] * 0.049087387f;
    int step;
    if (delta->step & 1)
        step = (delta->step >> 1) * 8;
    else
        step = delta->step >> 1;
    part->field_0x00.field_0x02 = step;
    field_0x604 += part->field_0x00.field_0x02;
    part->field_0x00.field_0x34 = field_0x604;
    part->field_0x00.field_0x28 = field_0x5d4;
    part->field_0x00.field_0x04 = field_0x5e0;
    part->field_0x00.field_0x1c = field_0x5ec;
    part->field_0x00.field_0x10 = field_0x5f8;
    part->field_0x00.field_0x14 = field_0x5fc;
    part->field_0x00.field_0x18 = field_0x600;
}

// 0x004a9d10
void UnknownTrackGameViewOwnerDc::UnknownFunction4a9d10(KrustyVCR* vcr)
{
    field_0x708 = vcr;
}

// 0x004a9f90: every other racer hears about the new and the previous racer,
// and the new racer hears about each of them.
void TrackGameViewOwner::UnknownFunction4a9f90(UnknownEventRacer* racer, UnknownEventRacer* previous)
{
    int iterator = 0;
    for (UnknownEventRacer* other = field_0x34->UnknownFunction4204e0(&iterator); other;
         other = field_0x34->UnknownFunction4204e0(&iterator)) {
        if (other == racer)
            continue;
        if (racer)
            racer->UnknownFunction496f90(other);
        if (previous && previous != other)
            other->UnknownFunction496f90(previous);
        if (racer)
            other->UnknownFunction496f90(racer);
    }
}
