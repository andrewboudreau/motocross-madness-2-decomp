// Complete camera-control candidate. Original TU ownership is unproven.
// See docs/FOLLOW_CAMERA_CONTROLS.md for the full extent and nonexact review.
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/UnknownResourceManager.h"
#include <stdio.h>
#include <string.h>
#include "FollowCameraCandidateOperators.h"
#include "FollowCameraControlSupport.h"

// 0x00463a30. Inactive: clears +0x274. With the override on and +0x26c set
// (slot 23), the keys and the joystick move the cached target (+0x2a8);
// otherwise, outside states 3, 6 and 7, state 5 moves the camera position
// (+0x170) and the other states turn the pitch (+0x22c, +-1.55), the yaw
// (+0x234, wrapped to +-pi) and the distance (+0x220, at least 7). Steps are
// 300 or 30 units per second; below distance 50 the turn rate grows by
// 3 - 0.04 * distance. Returns whether the camera moved.
bool FollowCamera::UnknownVirtualSlot45(bool active, float dt) {
    bool moved = false;
    if (active) {
    if (overrideActive && isMovingTarget) {
        float x;
        float y;
        float speed = dt * 300.0f;
        isSettled = 0;
        if (FOLLOWCAMERA_KEY(0x48))
            cachedTarget.y += speed;
        if (FOLLOWCAMERA_KEY(0x50) && targetDistance > 0.0f)
            cachedTarget.y -= speed;
        if (FOLLOWCAMERA_KEY(0x4b))
            cachedTarget -= Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
        if (FOLLOWCAMERA_KEY(0x4d))
            cachedTarget += Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
        if (FOLLOWCAMERA_KEY(0x4a) || FOLLOWCAMERA_KEY(0x0c))
            cachedTarget += speed * viewDirection;
        if ((FOLLOWCAMERA_KEY(0x4e) || FOLLOWCAMERA_KEY(0x0d)) && targetDistance > 0.0)
            cachedTarget -= speed * viewDirection;
        if (FOLLOWCAMERA_CONTROLS()->activeJoystick) {
            if (FollowCameraJoystick(FOLLOWCAMERA_CONTROLS())
                    ->UnknownVirtualSlot4(0, &x, &y)) {
                if (targetDistance > 0.0f)
                    cachedTarget.y += y * speed;
                Vector3 side(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0));
                float xs = x * speed;
                cachedTarget += side * xs;
                if (x != 0.0f || y != 0.0f)
                    return moved;
            }
            if (FOLLOWCAMERA_CONTROLS()->IsDeviceButtonDown(2, 2, 0xc, 0))
                cachedTarget += speed * viewDirection;
            if (FOLLOWCAMERA_CONTROLS()->IsDeviceButtonDown(3, 2, 0xc, 0) && targetDistance > 0.0)
                cachedTarget -= speed * viewDirection;
        }
    } else if (cameraState != 3 && cameraState != 6 && cameraState != 7) {
    float x;
    float y;
    float turn = dt * 30.0f;
    float speed = dt * 300.0f;
    if (cameraState == 5) {
        if (FOLLOWCAMERA_KEY(0x48)) {
            eyePosition.y += speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x50) && targetDistance > 6.5f) {
            eyePosition.y -= speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x4b)) {
            eyePosition -= Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x4d)) {
            eyePosition += Vector3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0)) * speed;
            moved = true;
        }
        if (FOLLOWCAMERA_KEY(0x4a) || FOLLOWCAMERA_KEY(0x0c)) {
            eyePosition -= speed * viewDirection;
            moved = true;
        }
        if ((FOLLOWCAMERA_KEY(0x4e) || FOLLOWCAMERA_KEY(0x0d)) && targetDistance > 6.5f) {
            eyePosition += speed * viewDirection;
            moved = true;
        }
        if (FOLLOWCAMERA_CONTROLS()->activeJoystick) {
            if (FollowCameraJoystick(FOLLOWCAMERA_CONTROLS())
                    ->UnknownVirtualSlot4(0, &x, &y)) {
                if (targetDistance > 6.5f)
                    eyePosition.y += y * speed;
                Vector3 side(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0));
                float xs = x * speed;
                eyePosition += side * xs;
                if (x != 0.0 || y != 0.0f)
                    moved = true;
                if (x != 0.0f || y != 0.0f)
                    return moved;
            }
            if (FOLLOWCAMERA_CONTROLS()->IsDeviceButtonDown(2, 2, 0xc, 0)) {
                eyePosition -= speed * viewDirection;
                moved = true;
            }
            if (FOLLOWCAMERA_CONTROLS()->IsDeviceButtonDown(3, 2, 0xc, 0) && targetDistance > 6.5f) {
                eyePosition += speed * viewDirection;
                moved = true;
            }
        }
        return moved;
    }
    if (FOLLOWCAMERA_KEY(0x48)) {
        if (followDistance < 50.0)
            FollowCameraTilt(followPitch, (3.0f - followDistance * 0.04f) * turn * 0.01745329f);
        else
            FollowCameraTilt(followPitch, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x50)) {
        if (followDistance < 50.0)
            FollowCameraTilt(followPitch, (3.0f - followDistance * 0.04f) * turn * -0.01745329f);
        else
            FollowCameraTiltBack(followPitch, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4b)) {
        if (followDistance < 50.0)
            FollowCameraTurn(followYaw, (3.0f - followDistance * 0.04f) * turn * 0.01745329f);
        else
            FollowCameraTurn(followYaw, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4d)) {
        if (followDistance < 50.0)
            FollowCameraTurn(followYaw, (3.0f - followDistance * 0.04f) * turn * -0.01745329f);
        else
            FollowCameraTurnBack(followYaw, turn * 0.01745329f);
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4a) || FOLLOWCAMERA_KEY(0x0c)) {
        followDistance += turn;
        if (followDistance < 7.0f)
            followDistance = 7.0f;
        moved = true;
    }
    if (FOLLOWCAMERA_KEY(0x4e) || FOLLOWCAMERA_KEY(0x0d)) {
        followDistance -= turn;
        if (followDistance < 7.0f)
            followDistance = 7.0f;
        moved = true;
    }
    if (FOLLOWCAMERA_CONTROLS()->activeJoystick) {
        if (FollowCameraJoystick(FOLLOWCAMERA_CONTROLS())
                ->UnknownVirtualSlot4(0, &x, &y)) {
            if (followDistance < 50.0)
                FollowCameraTilt(followPitch, (3.0f - followDistance * 0.04f) * y * turn * 0.01745329f);
            else
                FollowCameraTilt(followPitch, y * turn * 0.01745329f);
            if (followDistance < 50.0)
                FollowCameraTurn(followYaw, (3.0f - followDistance * 0.04f) * x * turn * -0.01745329f);
            else
                FollowCameraTurn(followYaw, x * turn * -0.01745329f);
            if (x != 0.0 || y != 0.0f)
                moved = true;
            if (x != 0.0f || y != 0.0f)
                return moved;
        }
        if (FOLLOWCAMERA_CONTROLS()->IsDeviceButtonDown(2, 2, 0xc, 0)) {
            followDistance += turn;
            if (followDistance < 7.0f)
                followDistance = 7.0f;
            moved = true;
        }
        if (FOLLOWCAMERA_CONTROLS()->IsDeviceButtonDown(3, 2, 0xc, 0)) {
            followDistance -= turn;
            if (followDistance < 7.0f)
                followDistance = 7.0f;
            moved = true;
        }
    }
    return moved;
    } else {
        isSettled = 1;
    }
    } else {
        isSettled = 0;
    }
    return moved;
}
