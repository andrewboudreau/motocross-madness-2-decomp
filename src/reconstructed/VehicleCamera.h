#pragma once

#include "FollowCamera.h"
#include "../krusty2/vehicle/VehicleCore.h"

class SoultreeObject;

// Optional targets at VehicleCamera+0x384 / +0x388 (slot 33). The character
// target is a scene CollisionCharacter (krusty2/motion/CollisionCharacter.h):
// slot 33 tracks its nodeVelocity (+0x224), and 0x0052bb60 reads its modelNode
// (+0x1a0).
class CollisionCharacter;

// The camera's car target is a scene procedural car (CarProcedural.h); slot 33
// tracks its pathVelocity (+0x40), and the position and heading come from its
// carModel (+0x34).
class CarProcedural;

// RTTI: VehicleCamera : FollowCamera. Vehicle.cpp is a name-overlap candidate
// for its translation unit, not established. Implements FollowCamera's pure
// slots 33, 35, 39, 41, 42, 50, 51, 57 and 74; names are provisional.
class VehicleCamera : public FollowCamera {
public:
    explicit VehicleCamera(int flags); // 0x0052b920
    virtual ~VehicleCamera();          // 0x0052b9d0 (deleting wrapper 0x0052b9b0)

    // 0x0052b9e0: FollowCamera's setup (0x00463140); returns this, or 0
    // when it failed.
    VehicleCamera* SetupVehicleCamera(void* context, float fieldOfViewEaseRate, float distanceEaseRate, float initialLookAhead,
                                         float initialFollowHeight, float initialYawOffset, int capacity, int count,
                                         const int* list);

    // 0x0052bac0 / 0x0052bc50: the vehicle's +0x494 / +0x488 in vehicle
    // mode; otherwise slot 33 / 0x0052bb60, latched in +0x398 / +0x3a4 by
    // +0x392 / +0x391.
    Vector3 CachedTargetHeading();
    Vector3 CachedTargetPoint();
    // 0x0052bb60: the position of the followed part or target.
    Vector3 TargetPoint();

    virtual Vector3 GetTargetVelocity();
    virtual Vector3 ComputeLookAheadPoint(unsigned char a, float dt);

    virtual float GetCrashYaw();
    virtual Vector3 ComputeFollowHeading();
    virtual void UpdateTurnYaw(bool flag);
    virtual Vector3 GetOnboardEyePosition();
    virtual float GetRoll();
    virtual bool TrySettleOnPoint(const Vector3& point, bool enable, bool force);
    virtual Vector3 GetLeadTargetPoint(float dt);
    virtual void ApplyReverseViewPreset();
    virtual void LookAtTargetPoint();
    virtual void SelectNextCameraState();
    // Inline: retail's copy (0x00417490) is emitted next to BikeCamera code.
    virtual bool IsSubjectCrashed() {
        if (vehicleMode) {
            bool active = vehicle->crashState != 0;
            return active;
        }
        return false;
    }
    virtual bool ShouldFollowSubject();

    // Inlined by slot 75: no vehicle mode, or the vehicle's +0x444 is clear.
    int IsSubjectUncrashed() {
        if (!vehicleMode)
            return 1;
        return vehicle->crashState == 0;
    }

    // Inline (BikeRace::AimCamera 0x0041eb20): the camera targets one scene
    // object instead of the racer. Each stores its own target first (the
    // store order is what makes AimCamera exact).
    void TargetShadowCaster(SoultreeObject* caster) {
        targetCaster = caster;
        isSettled = 0;
        targetPointCached = 0;
        targetHeadingCached = 0;
        vehicleMode = 0;
        targetCharacter = 0;
        targetCar = 0;
    }
    void TargetSceneCharacter(CollisionCharacter* character) {
        targetCharacter = character;
        isSettled = 0;
        targetPointCached = 0;
        targetHeadingCached = 0;
        vehicleMode = 0;
        targetCar = 0;
        targetCaster = 0;
    }
    void TargetSceneObject(CarProcedural* object) {
        targetCar = object;
        isSettled = 0;
        targetPointCached = 0;
        targetHeadingCached = 0;
        vehicleMode = 0;
        targetCharacter = 0;
        targetCaster = 0;
    }

    CollisionCharacter* targetCharacter;  // +0x384  BikeRace TargetCharacter stores a key-framed scene character; slots 33/41/TargetPoint read its node
    CarProcedural* targetCar;  // +0x388  BikeRace TargetObject stores a scene procedural object (CarProcedural)
    SoultreeObject* targetCaster;  // +0x38C  BikeRace view +0x38c shadowCaster (TargetCaster); TargetPoint takes its translation
    unsigned char vehicleMode;           // +0x390 following the vehicle (BikeRace::NextCameraTarget stores its int argument as a byte)
    unsigned char targetPointCached;     // +0x391 cachedTargetPoint is valid (cleared on a new target)
    unsigned char targetHeadingCached;   // +0x392 cachedTargetHeading is valid
    Vehicle* vehicle;                    // +0x394 the followed vehicle (BikeRace's KrustyBike racers)
    Vector3 cachedTargetHeading;         // +0x398 slot 33's value
    Vector3 cachedTargetPoint;           // +0x3a4 TargetPoint's value
};
