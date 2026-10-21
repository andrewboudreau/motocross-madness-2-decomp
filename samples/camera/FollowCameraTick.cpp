// Complete per-frame camera candidate. Original TU ownership is unproven.
// See docs/FOLLOW_CAMERA_TICK.md for the explicit nonexact review boundary.
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/UnknownResourceManager.h"
#include <stdio.h>
#include <string.h>

// The aggregate already owns these private axis objects. A standalone build
// owns its own initialized axes; neither context aliases retail storage.
#ifndef MCM2_FOLLOW_CAMERA_TICK_AXES_DEFINED
#define MCM2_FOLLOW_CAMERA_TICK_AXES_DEFINED
static Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
#endif
#include "FollowCameraCandidateOperators.h"

// 0x00465c20 (3672 bytes; near miss, see the header comment).
int FollowCamera::Tick(float dt) {
    Vector3 position;
    Vector3 target;
    Vector3 smoothed;
    bool current = ((RenderTarget*)renderContext)->currentCamera == this;
    followHeading = UnknownVirtualSlot41();
    if (!isPlaced) {
        isPlaced = 1;
        targetPoint = UnknownVirtualSlot57(0);
        position = followHeading * -followDistance;
        position += targetPoint;
        subject->QueryGround(&position, 0, 0, 0);
        Vector3 direction = position - targetPoint;
        D3DRMVectorRotate(&position, &direction, &kVec3XAxis, followPitch);
        D3DRMVectorRotate(&direction, &position, &kVec3YAxis, followYaw);
        direction *= followDistance;
        position = direction + targetPoint;
        SetFrame(&position, 0, 0, 0, 0);
        target = followHeading * lookAheadDistance;
        target += targetPoint;
        subject->QueryGround(&target, 0, 0, 0);
        LookAt(target);
        wasWide = FollowCameraAbs(followYaw) > 2.0f;
    }
    if (cameraState != 6 && isCrashView && !UnknownVirtualSlot74()) {
        isCrashView = 0;
        followDistance = savedDistance;
        followPitch = savedPitch;
        followYaw = savedYaw;
    }
    bool wide = FollowCameraAbs(followYaw) > 2.0f;
    if (cameraState != 6 && !wasWide && wide)
        isBlending = 1;
    if (overrideActive)
        targetPoint = cachedTarget;
    else
        targetPoint = UnknownVirtualSlot57(dt);
    if (cameraState == 5) {
        float dx = eyePosition.x - targetPoint.x;
        float dy = eyePosition.y - targetPoint.y;
        float dz = eyePosition.z - targetPoint.z;
        targetDistance = FastSqrt(dz * dz + dx * dx + dy);
    }
    bool moved = UnknownVirtualSlot45(current, dt);
    UnknownVirtualSlot42(wide);
    bool following = UnknownVirtualSlot75();
    target = UnknownVirtualSlot48(wide || isSettled, following, dt);
    position = UnknownVirtualSlot49(following, target, dt);
    if (!positionEaseX) {
        targetEaseZ = new(__FILE__, 0x4b0) UnknownFollowCameraValue(target.z, 0.3f);
        targetEaseX = new(__FILE__, 0x4b1) UnknownFollowCameraValue(target.x, 0.3f);
        targetEaseY = new(__FILE__, 0x4b2) UnknownFollowCameraValue(target.y, 0.3f);
        UnknownVirtualSlot43(target);
        positionEaseX = new(__FILE__, 0x4b4) UnknownFollowCameraValue(position.x, 0.3f);
        positionEaseY = new(__FILE__, 0x4b5) UnknownFollowCameraValue(position.y, 0.3f);
        positionEaseZ = new(__FILE__, 0x4b6) UnknownFollowCameraValue(position.z, 0.3f);
        UnknownVirtualSlot44(position);
    }
    if (cameraState == 5) {
        if (!overrideActive) {
            targetFieldOfView = (distanceForMinimumFov - targetDistance) / (distanceForMinimumFov - distanceForMaximumFov) *
                              (distanceFovMaximum - distanceFovMinimum) + distanceFovMinimum;
            if (targetFieldOfView < distanceFovMinimum)
                targetFieldOfView = distanceFovMinimum;
            if (targetFieldOfView > distanceFovMaximum)
                targetFieldOfView = distanceFovMaximum;
            if (moved)
                targetFieldOfView = (targetFieldOfView - fieldOfView) * 0.75f + fieldOfView;
        }
    } else if (cameraState == 7) {
        float dx = eyePosition.x - target.x;
        if (dx < 0.0f) dx = -dx;
        float dz = eyePosition.z - target.z;
        if (dz < 0.0f) dz = -dz;
        targetFieldOfView = ((200.0f - FastSqrt(dz * dz + dx * dx)) / 180.0f) * 60.0f + 10.0f;
        if (targetFieldOfView < 10.0f) targetFieldOfView = 10.0f;
        if (targetFieldOfView > 70.0f) targetFieldOfView = 70.0f;
        fieldOfViewEase->Set(targetFieldOfView, FLT_MAX);
    } else if (following) {
        if (cameraState != 3) {
            if (cameraState == 4)
                UnknownVirtualSlot59();
            isSettled = UnknownVirtualSlot36(position, wide, moved);
        }
    } else if (UnknownVirtualSlot74()) {
        isCrashView = 1;
    }
    cachedTarget.x = targetEaseX->Update(target.x, dt);
    cachedTarget.y = targetEaseY->Update(target.y, dt);
    cachedTarget.z = targetEaseZ->Update(target.z, dt);
    smoothed.x = positionEaseX->Update(position.x, dt);
    smoothed.y = positionEaseY->Update(position.y, dt);
    smoothed.z = positionEaseZ->Update(position.z, dt);
    if (isBlending) {
        UnknownVirtualSlot33();
        blendClock += dt;
        blendClock = FollowCameraMin(blendClock, 1.0f);
        float blend = blendClock;
        isSettled = 0;
        smoothed.x = (position.x - smoothed.x) * blend + smoothed.x;
        smoothed.y = (position.y - smoothed.y) * blend + smoothed.y;
        smoothed.z = (position.z - smoothed.z) * blend + smoothed.z;
        if (!moved && blend >= 1.0f) {
            UnknownVirtualSlot44(position);
            UnknownVirtualSlot43(cachedTarget);
            blendClock = 0.0f;
            isBlending = moved;
        }
    }
    if (savedCameraState == 6 || resumeState == 7) {
        isSettled = 1;
        if (resumeState == 7)
            resumeState = cameraState;
        else
            savedCameraState = cameraState;
        UnknownVirtualSlot44(position);
        UnknownVirtualSlot43(target);
        smoothed = position;
        cachedTarget = target;
    } else if (isSettled) {
        smoothed = position;
        if (!overrideActive)
            cachedTarget = target;
    }
    UnknownVirtualSlot52(&smoothed);
    if (current) {
        float roll;
        if (cameraState == 3 && following) {
            roll = UnknownVirtualSlot51();
            UnknownVirtualSlot53();
        } else {
            roll = 0.0f;
            UnknownVirtualSlot54();
        }
        SetFrame(&smoothed, 0, 0, &roll, 0);
        LookAt(cachedTarget);
        if (UnknownVirtualSlot56()) {
            if (cameraState != 7 && !isCrashView) {
                resumeState = cameraState;
                resumeFieldOfView = targetFieldOfView;
                initialPitch = followPitch;
                initialYaw = followYaw;
                initialDistance = followDistance;
                cameraState = 7;
                resumeEyePosition = eyePosition;
                UnknownVirtualSlot68();
                fieldOfViewEase->Set(targetFieldOfView, FLT_MAX);
            }
        } else if (cameraState == 7) {
            cameraState = resumeState;
            followPitch = initialPitch;
            followYaw = initialYaw;
            followDistance = initialDistance;
            targetFieldOfView = resumeFieldOfView;
            eyePosition = resumeEyePosition;
            resumeState = 7;
            fieldOfViewEase->Set(targetFieldOfView, FLT_MAX);
        }
        if (cameraState != 5 && cameraState != 7) {
            if (UnknownVirtualSlot55()) {
                if (cameraState != 6 && !isCrashView) {
                    savedCameraState = cameraState;
                    initialPitch = followPitch;
                    initialYaw = followYaw;
                    initialDistance = followDistance;
                    cameraState = 6;
                    savedFieldOfView = targetFieldOfView;
                    UnknownVirtualSlot67();
                    fieldOfViewEase->Set(targetFieldOfView, FLT_MAX);
                }
            } else if (cameraState == 6) {
                cameraState = savedCameraState;
                targetFieldOfView = savedFieldOfView;
                savedCameraState = 6;
                followPitch = initialPitch;
                followYaw = initialYaw;
                followDistance = initialDistance;
                fieldOfViewEase->Set(targetFieldOfView = savedFieldOfView, FLT_MAX);

            }
        }
        if (g_TrackGame->paused || (cameraState != 5 && cameraState != 7)) {
            if (g_TrackGame->controlInterface->IsDeviceButtonDown(0xc7, 0, 0x80000000, 0)) {
                targetFieldOfView -= 3.0f;
                if (targetFieldOfView < minFieldOfView)
                    targetFieldOfView = minFieldOfView;
            }
            if (g_TrackGame->controlInterface->IsDeviceButtonDown(0xcf, 0, 0x80000000, 0)) {
                targetFieldOfView += 3.0f;
                if (targetFieldOfView > maxFieldOfView)
                    targetFieldOfView = maxFieldOfView;
            }
        }
        SetFieldOfView(fieldOfViewEase->Update(targetFieldOfView, dt));
        Camera::Tick(dt);
        wasWide = wide;
        return 1;
    }
    UnknownVirtualSlot54();
    wasWide = wide;
    return 1;
}
