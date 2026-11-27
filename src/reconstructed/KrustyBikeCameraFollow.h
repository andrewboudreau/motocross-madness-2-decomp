#pragma once

// KrustyBikeCamera::StartFollowing, inline three times in BikeRace::AimCamera
// (0x0041eb20) and once in the quarry loader (0x004de590): follows `racer`,
// showing the previous racer's model again and hiding the new one's. Both
// units include this after their other headers.

#include "KrustyBikeCamera.h"
#include "BikeRaceCore.h"
#include "D3DIMSoulTree.h"
#include "../krusty2/vehicle/KrustyBikeCore.h"

inline void KrustyBikeCamera::StartFollowing(KrustyBike* racer) {
    if (bike != 0 && !((BikeRace*)racer->race)->forceHighLod) {
        ((D3DIMSoultreeObject*)bike->modelNode)->SetLowestLod(1);
        ((D3DIMSoultreeObject*)bike->riderCharacter->modelNode)->SetLowestLod(1);
        if (((D3DIMSoultreeObject*)bike->modelNode)->currentLod == 0) {
            ((D3DIMSoultreeObject*)bike->modelNode)->SetLod(1);
        }
        if (((D3DIMSoultreeObject*)bike->riderCharacter->modelNode)->currentLod == 0) {
            ((D3DIMSoultreeObject*)bike->riderCharacter->modelNode)->SetLod(1);
        }
    }
    vehicle = racer;
    targetPointCached = 0;
    targetHeadingCached = 0;
    vehicleMode = 1;
    targetCaster = 0;
    targetCharacter = 0;
    targetCar = 0;
    bike = racer;
    isSettled = 0;
    krustyBike = racer;
    if (!((BikeRace*)racer->race)->forceHighLod) {
        ((D3DIMSoultreeObject*)racer->modelNode)->SetLowestLod(0);
        ((D3DIMSoultreeObject*)bike->riderCharacter->modelNode)->SetLowestLod(0);
    }
    isSettled = 0;
}
