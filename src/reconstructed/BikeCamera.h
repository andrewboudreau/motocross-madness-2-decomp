#pragma once

#include "VehicleCamera.h"

// The followed bike (+0x3b0): BikeRace's KrustyBike racers, read as Bike
// (krusty2/vehicle/BikeCore.h).
class Bike;

// RTTI: BikeCamera : VehicleCamera. Its code sits near BikeAI.cpp references;
// the translation unit is not established.
class BikeCamera : public VehicleCamera {
public:
    explicit BikeCamera(int flags); // 0x00416e20
    virtual ~BikeCamera();          // 0x00416e70 (deleting wrapper 0x00416e50)

    // 0x00416e80: VehicleCamera's setup (0x0052b9e0); this or 0.
    BikeCamera* SetupBikeCamera(void* context, float fieldOfViewEaseRate, float distanceEaseRate, float initialLookAhead,
                                      float initialFollowHeight, float initialYawOffset, int capacity, int count,
                                      const int* list);

    virtual Vector3 GetStateFiveFollowPoint(float dt);
    virtual Vector3 GetTargetPoint();
    virtual void SteerToKeepTargetOnScreen(float dt);
    virtual Vector3 GetOnboardEyePosition();
    virtual float GetRoll();
    virtual void HideSubject();
    virtual void ShowSubject();
    virtual bool ShouldFollowSubject();

    Bike* bike; // +0x3b0 the followed bike
};
