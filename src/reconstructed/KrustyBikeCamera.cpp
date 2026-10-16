#include "KrustyBikeCamera.h"

// The four Math3D vector constants (.CRT$XCU 159-162, initialisers
// 0x004986b0..0x004987eb, after the last method): 0x0067c3c8, 0x0067c3d8,
// 0x0067c3e8 and 0x0067c3b8. Slot 10 passes the y axis to FollowCamera
// slot 46.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// By-value read of a vector's x; slot 34's dot product needs it for retail's
// load order, as in BikeCamera.cpp.
static inline float VectorX(const Vector3& v) { return v.x; }

// The inline dot product slot 34 uses; the grouping gives retail's y, x, z order.
static inline float KrustyCameraDot(const Vector3& a, const Vector3& b) {
    return a.z * b.z + (VectorX(a) * b.x + a.y * b.y);
}

// 0x00497cb0: restores the saved state (slot 62) and applies it; virtual
// calls in a constructor bind statically.
KrustyBikeCamera::KrustyBikeCamera(int flags) : BikeCamera(flags) {
    UnknownVirtualSlot62();
    UnknownVirtualSlot71(cameraState);
    krustyBike = 0;
    raceView = 0;
    widthKeysEnabled = 0;
    resumeEyePosition = eyePosition;
    savedCameraState = cameraState;
    resumeState = cameraState;
    savedFieldOfView = fieldOfView;
}

// 0x00497d80: an explicit empty destructor.
KrustyBikeCamera::~KrustyBikeCamera() {}

// 0x00497d90
KrustyBikeCamera* KrustyBikeCamera::SetupFlyby(void* value, float rate294, float rate298,
                                                          float value228, float value2d0, float value2e8,
                                                          int capacity, int count, const int* list) {
    if (!SetupBikeCamera(value, rate294, rate298, value228, value2d0, value2e8, capacity, count, list))
        return 0;
    LoadCameraFile("FlybyCam.vue");
    return this;
}

// 0x00497df0: the FollowCamera search, unless the global +0x3430 blocks it.
int KrustyBikeCamera::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (g_TrackGame->uiInteractionBlocked)
        return 0;
    return BikeCamera::OnControlPress(event, entry);
}

// 0x00497fa0: keeps the point at least 3.5 above the subject's ground height
// (easing +0x22c when it lifts it), and in modes 3 and 4 no higher than 400.
void KrustyBikeCamera::UnknownVirtualSlot52(Vector3* point) {
    bool capped = g_TrackGame->mode.GetSelectedSeries() == 3 ||
                  g_TrackGame->mode.GetSelectedSeries() == 4;
    Vector3 ground = *point;
    subject->QueryGround(&ground, 0, 0, 0);
    float minimum = ground.y + 3.5f;
    if (minimum > point->y) {
        if (followPitch < 0.0f)
            followPitch = (minimum - point->y) / followDistance + followPitch;
        point->y = minimum;
    } else if (capped && point->y > 400.0f) {
        point->y = 400.0f;
    }
}

// 0x00498080
unsigned char KrustyBikeCamera::UnknownVirtualSlot55() {
    return g_TrackGame->controlInterface->IsControlDown(0x0B, 0x3F);
}

// 0x004980a0: input 0x0a (unless +0x38c) clears +0x391/+0x392 when absent;
// outside state 7 it also needs the bike's +0x108, its axis below -2 and no
// +0x735 in vehicle mode.
bool KrustyBikeCamera::UnknownVirtualSlot56() {
    bool pressed;
    if (!targetCaster && g_TrackGame->controlInterface->IsControlDown(0x0A, 0x3F)) {
        pressed = true;
    } else {
        pressed = false;
        targetPointCached = pressed;
        targetHeadingCached = pressed;
    }
    if (cameraState == 7)
        return pressed;
    if (pressed && (!vehicleMode || (krustyBike->field_0x108 &&
                                     krustyBike->field_0x5f4->field_0x150 < -2.0f &&
                                     !krustyBike->isRemote)))
        return true;
    return false;
}

// 0x00498130: +0x308 from the fov/zoom ratio and the bike's +0x43c.
void KrustyBikeCamera::UnknownVirtualSlot42(bool flag) {
    float ratio = fieldOfView / maxFieldOfView;
    if (cameraState != 3) {
        if (vehicleMode && !flag &&
            (!krustyBike->field_0x7a4 || g_TrackGame->playerCount > 1))
            turnYaw = ratio * krustyBike->field_0x43c * 0.42f;
        else
            turnYaw = 0.0f;
    } else {
        turnYaw = ratio * krustyBike->field_0x43c * 0.55f;
    }
}

// 0x004981d0: saves the presets while in vehicle mode with the bike idle.
void KrustyBikeCamera::UnknownVirtualSlot59() {
    if (vehicleMode && !bike->crashState) {
        g_TrackGame->mode.savedCameraPitch = followPitch;
        g_TrackGame->mode.savedCameraYaw = followYaw;
        g_TrackGame->mode.savedCameraDistance = followDistance;
        g_TrackGame->mode.savedCameraFieldOfView = targetFieldOfView;
    }
}

// 0x00498230: restores the saved presets.
void KrustyBikeCamera::UnknownVirtualSlot60() {
    followPitch = g_TrackGame->mode.savedCameraPitch;
    followYaw = g_TrackGame->mode.savedCameraYaw;
    followDistance = g_TrackGame->mode.savedCameraDistance;
    targetFieldOfView = g_TrackGame->mode.savedCameraFieldOfView;
}

// 0x00498280: restores the saved state.
void KrustyBikeCamera::UnknownVirtualSlot62() {
    cameraState = g_TrackGame->mode.savedCameraState;
}

// 0x004982a0: saves the state.
void KrustyBikeCamera::UnknownVirtualSlot61() {
    g_TrackGame->mode.savedCameraState = cameraState;
}

// 0x004982c0: shows the state's name (string 0x13b9 + state) for 1.5 s,
// except in state 6.
void KrustyBikeCamera::UnknownVirtualSlot58() {
    TextQueueOverlay* target = g_TrackGame->eventManager->FindTextQueue();
    if (target && cameraState != 6) {
        char text[260];
        g_TrackGame->LoadResourceString(cameraState + 0x13B9, text, 0x80);
        UnknownMessage message(text, 1.5f);
        target->ReplaceHeadMessage(&message);
    }
}

// 0x00498340: BikeCamera slot 34 (0x00417050) with a raw-target case while
// the +0x3b8 view's +0x3f8 flag is set.
Vector3 KrustyBikeCamera::UnknownVirtualSlot34(float dt) {
    Vector3 result;
    if (isCrashView) {
        bike->riderCharacter->modelNode->GetPositionIn(0, &result);
        if (targetDistance < 500.0f) {
            Vector3 lead = bike->wrecker->rigidBody->velocity * dt;
            lead = lead * (4.0f - targetDistance * 0.006f);
            result += lead;
        } else {
            result += bike->wrecker->rigidBody->velocity * dt;
        }
    } else if (raceView->replayPaused) {
        result = targetPoint;
    } else {
        float dot = KrustyCameraDot(UnknownVirtualSlot33(), followHeading);
        float height = targetDistance * 0.009f + 1.0f;
        float ratio = fieldOfView / maxFieldOfView;
        float speed = vehicleMode ? vehicle->field_0x438 : 180.0f;
        float lag = (1.0f - targetDistance * 0.00052631577f) * ratio;
        float distance = lag * (height * height * dot / speed) * 35.0f;
        Vector3 offset = followHeading * distance;
        result = Vector3(offset.x + targetPoint.x, offset.y + targetPoint.y, offset.z + targetPoint.z);
    }
    result.y += 3.0f;
    return result;
}

// 0x004985b0: FollowCamera slot 48 plus a raw-target case while the +0x3b8
// view flags are set.
Vector3 KrustyBikeCamera::UnknownVirtualSlot48(int a, bool flag, float dt) {
    Vector3 result;
    if (cameraState == 5) {
        if (overrideActive)
            result = cachedTarget;
        else
            result = UnknownVirtualSlot34(dt);
    } else if (cameraState == 7) {
        result = targetPoint;
        result.y += 3.0f;
    } else if (raceView->replayPaused || raceView->replayStepFrame) {
        result = targetPoint;
    } else if (flag) {
        result = UnknownVirtualSlot35(a, dt);
    } else {
        result = UnknownVirtualSlot37();
    }
    return result;
}
