// Complete 0x00462ee0 constructor. The original 564-byte near-miss remains
// in FollowCameraNearMisses.cpp; this separate candidate corrects observable
// partial-write ordering. Original translation-unit ownership is unproven.
#include "../../src/reconstructed/FollowCamera.h"

static Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

FollowCamera::FollowCamera(int flags) : PCCamera(flags) {
    isPlaced = 0;
    targetPoint = kVec3Zero;
    cachedTarget = kVec3Zero;
    worldYaw = 0;
    targetDistance = 0;
    // These explicit accesses preserve the observed writes before a guarded
    // destination faults. They do not establish original volatile member types.
    volatile FollowCamera* ordered = this;
    ordered->followHeading.x = kVec3ZAxis.x;
    ordered->followHeading.y = kVec3ZAxis.y;
    float headingZ = kVec3ZAxis.z;
    ordered->points = 0;
    ordered->isCrashView = 0;
    ordered->followHeading.z = headingZ;
    lift = 0;
    lookAheadDistance = 27.0f;
    targetFieldOfView = fieldOfView;
    state4FieldOfView = fieldOfView;
    fieldOfViewEase = 0;
    subject = 0;
    targetEaseX = 0;
    targetEaseY = 0;
    targetEaseZ = 0;
    positionEaseX = 0;
    positionEaseY = 0;
    positionEaseZ = 0;
    distanceEase = 0;
    fovKeysEnabled = 0;
    overrideActive = 0;
    isMovingTarget = 0;
    ordered->isSettled = 0;
    ordered->pointCapacity = 0;
    pointCount = 0;
    followHeight = 7.0f;
    subjectCellSize = 3.0f;
    cameraState = 0;
    UnknownVirtualSlot71(0);
    resumeEyePosition = eyePosition;
    savedCameraState = cameraState;
    resumeState = cameraState;
    savedFieldOfView = fieldOfView;
    initialDistance = followDistance;
    initialPitch = followPitch;
    initialYaw = followYaw;
    resumeFieldOfView = fieldOfView;
    settleChecked = 0;
    wasWide = 0;
    stateCount = 0;
    stateIndex = 0;
    blendClock = 0;
    turnYaw = 0;
    isBlending = 0;
    recordsPerSecond = 15.0f;
    frameTime = 0;
    distanceFovMinimum = 10.0f;
    distanceForMinimumFov = 280.0f;
    distanceFovMaximum = 70.0f;
    distanceForMaximumFov = 20.0f;
}
