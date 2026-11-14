#pragma once

#include "FollowCamera.h"

class SoultreeObject;

// An entry of the vehicle's +0x53c list (VehicleCamera slot 35).
struct VehicleCameraWheelView {
    unsigned char field_0x000[0x150];
    float penetration;                        // +0x150  VehicleWheel penetration (CollisionPoint+0x98, reset to -999); slot 35 lifts the camera while it is above -0.5
    unsigned char field_0x154[0x2a8 - 0x154];
    int aux;  // +0x2A8  element of Vehicle+0x53c wheelList = VehicleWheel, whose +0x2a8 is aux (counted by auxWheelCount)
};

struct VehicleCameraVehicleView {
    unsigned char field_0x000[0x48];
    float savedRoll;                          // +0x48 VehicleCamera slot 51; krusty2 SoultreePhysicsBaseObject
    float savedPitch;                         // +0x4c VehicleCamera slot 35 pitch angle
    unsigned char field_0x050[0x64 - 0x50];
    Vector3 velocity;                         // +0x64 VehicleCamera slot 33
    Vector3 acceleration;                     // +0x070 Vehicle acceleration (SoultreePhysicsBaseObject)
    Vector3 prevVelocity;                     // +0x07c Vehicle prevVelocity (SoultreePhysicsBaseObject)
    Vector3 bodyForward;                      // +0x88 VehicleCamera slot 41 heading fallback
    Vector3 bodyUp;                           // +0x094 Vehicle bodyUp (SoultreePhysicsBaseObject)
    Vector3 savedForward;                     // +0x0a0 Vehicle savedForward (SoultreePhysicsBaseObject)
    Vector3 savedUp;                          // +0x0ac Vehicle savedUp (SoultreePhysicsBaseObject)
    float prevSpeed;                          // +0x0b8 Vehicle prevSpeed (SoultreePhysicsBaseObject)
    float linearSpeed;                        // +0xbc |velocity|; VehicleCamera slot 41 (at least 0.01)
    unsigned char field_0x0c0[0x3bc - 0xc0];
    SoultreeObject* modelNode;            // +0x3bc VehicleCamera slot 50
    unsigned char field_0x3c0[0x434 - 0x3c0];
    float forwardSpeed;  // +0x434  Vehicle +0x434 = dot(velocity, bodyForward) (BikeAI); KrustyBike compares it with 60 mph in ft/s
    float referenceSpeed;                     // +0x438  Vehicle +0x438 referenceSpeed (180); the bike cameras divide by it
    float turnRate;                           // +0x43c VehicleCamera slot 42 scale
    int movingForward;                        // +0x440
    int crashState;                           // +0x444 VehicleCamera slots 72, 74, 75
    unsigned char field_0x448[0x45c - 0x448];
    float crashYaw;  // +0x45C  Vehicle::CheckCrash stores savedYaw here on a crash; VehicleCamera slot 39 returns it
    int crashReason;                          // +0x460 BikeCamera slot 75 tests 6
    int crashedUpsideDown;  // +0x464  Vehicle::CheckCrash stores savedUp.y < 0 on a crash; VehicleCamera slot 39 turns the camera by pi
    unsigned char field_0x468[0x488 - 0x468];
    Vector3 takeoffPosition;                  // +0x488 0x0052bc50 in vehicle mode
    Vector3 takeoffVelocity;                  // +0x494 0x0052bac0 in vehicle mode
    unsigned char field_0x4a0[0x53c - 0x4a0];
    VehicleCameraWheelView** riders;              // +0x53c riders (VehicleCamera slot 35)
    int wheelCapacity;                        // +0x540
    int riderCount;                           // +0x544 rider count
    unsigned char field_0x548[0x56c - 0x548];
    int auxWheelCount;                        // +0x56c nonzero: look for a rider with +0x2a8 set

    // Inlined by VehicleCamera slot 35: the first rider with +0x2a8 set
    // when +0x56c asks for one, else the first rider.
    VehicleCameraWheelView* PickCameraWheel() {
        if (auxWheelCount) {
            for (int i = 0; i < riderCount; i++) {
                if (riders[i]->aux)
                    return riders[i];
            }
        }
        return riders[0];
    }
};

// Optional targets at VehicleCamera+0x384 / +0x388 (slot 33).
struct VehicleCameraCharacterTargetView {
    unsigned char field_0x000[0x1a0];
    SoultreeObject* modelNode;       // +0x1a0 0x0052bb60; the characters' model node (krusty2 Character)
    unsigned char field_0x1a4[0x224 - 0x1a4];
    Vector3 nodeVelocity;                // +0x224 krusty2 CollisionCharacter::nodeVelocity
};

struct VehicleCameraCarProceduralView {
    unsigned char field_0x00[0x34];
    SoultreeObject* carModel;  // +0x34  camera +0x388 is a scene procedural entry (CarProcedural, BikeRace TargetObject); CarProcedural +0x34 carModel
    unsigned char field_0x38[0x40 - 0x38];
    Vector3 pathVelocity;  // +0x40  CarProcedural +0x40 pathVelocity; slot 33 returns it as the tracked velocity (like vehicle velocity)
};

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

protected:
    VehicleCameraCharacterTargetView* targetCharacter;  // +0x384  BikeRace TargetCharacter stores a key-framed scene character; slots 33/41/TargetPoint read its node
    VehicleCameraCarProceduralView* targetCar;  // +0x388  BikeRace TargetObject stores a scene procedural object (CarProcedural)
    SoultreeObject* targetCaster;  // +0x38C  BikeRace view +0x38c shadowCaster (TargetCaster); TargetPoint takes its translation
    bool vehicleMode;                    // +0x390, vehicle mode
    unsigned char targetPointCached;     // +0x391 cachedTargetPoint is valid (cleared on a new target)
    unsigned char targetHeadingCached;   // +0x392 cachedTargetHeading is valid
    VehicleCameraVehicleView* vehicle;       // +0x394
    Vector3 cachedTargetHeading;         // +0x398 slot 33's value
    Vector3 cachedTargetPoint;           // +0x3a4 TargetPoint's value
};
