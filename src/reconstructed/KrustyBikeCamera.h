#pragma once

#include "GameObject.h"

#include "BikeCamera.h"
#include "RaceView.h"

// The followed racer (+0x3b4) is the KrustyBike (krusty2/vehicle/KrustyBikeCore.h).
class KrustyBike;

// RTTI: KrustyBikeCamera : BikeCamera. Its code (0x00497cb0..0x004987ec) follows
// KrustyBike.cpp's but ends with its own Math3D vector set, so it is a separate
// unit; the file name is ours.
// It keeps the camera state and presets in the 0x0056e26c object so they
// survive between cameras.
class KrustyBikeCamera : public BikeCamera {
public:
    explicit KrustyBikeCamera(int flags); // 0x00497cb0
    virtual ~KrustyBikeCamera();          // 0x00497d80 (deleting wrapper 0x00497d60)

    // 0x00497d90: BikeCamera's setup (0x00416e80), then the "FlybyCam.vue"
    // frames; this or 0.
    KrustyBikeCamera* SetupFlyby(void* context, float fieldOfViewEaseRate, float distanceEaseRate, float initialLookAhead,
                                            float initialFollowHeight, float initialYawOffset, int capacity, int count,
                                            const int* list);

    virtual int Tick(float frameTime);                 // 0x00497e20 (near miss)

    virtual int OnControlPress(ControlEvent* event, InputButtonState* entry);
    virtual Vector3 GetStateFiveFollowPoint(float dt);
    virtual void UpdateTurnYaw(bool flag);
    virtual Vector3 SelectFollowPoint(int a, bool flag, float dt);
    virtual void ClampPositionAboveGround(Vector3* point);
    virtual unsigned char IsLookBackHeld();
    virtual unsigned char IsTargetViewHeld();
    virtual void ShowCameraStateName();
    virtual void SaveCameraPreset();
    virtual void RestoreCameraPreset();
    virtual void SaveCameraState();
    virtual void RestoreCameraState();

    // Inline (three times in BikeRace::AimCamera 0x0041eb20, once in the
    // quarry loader 0x004de590): follows `racer`, showing the previous racer's
    // model again and hiding the new one's (KrustyBikeCameraFollow.h).
    void StartFollowing(KrustyBike* racer);

    KrustyBike* krustyBike;          // +0x3b4 the followed racer
    BikeRace* raceView;     // +0x3b8
};
