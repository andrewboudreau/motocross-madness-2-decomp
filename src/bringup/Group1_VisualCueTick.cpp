// Group1_VisualCueTick.cpp -- bring-up body for VisualCue::Tick (slot 10, 0x0048b100,
// 2796 bytes), which VisualCue's vtable (Krusty3DObjects.cpp) references but src/
// does not define yet.  The body is the near miss in
// samples/race/Krusty3DObjectsNearMisses.cpp (every mode decoded; the 16 calls are
// retail's, in retail's order; see that file's header), taken unchanged together with
// the TU-local helpers it uses.

#include "TrackGameGlobal.h"
#include <math.h>

#include "Krusty3DObjects.h"

// By-value read of the camera's +0x198 value (see slot 10 above).
static inline float CameraZoom(const UnknownBonusCamera* camera) {
    return camera->focalLength;
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3& operator*=(Vector3& v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

// |v|, exact for unit vectors.
static inline float Length(Vector3 v) {
    float squared = v.z * v.z + (v.y * v.y + v.x * v.x);
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
}

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);

#define UNKNOWN_DEGREES_TO_RADIANS(degrees) ((degrees) * 0.01745329f)


struct UnknownCueOwner {
    unsigned char field_0x00[0x08];
    UnknownArcadeView* currentCamera;          // +0x08
};

// Places the cue at its screen position in front of the camera, rebuilding
// the screen offsets when the field of view changed. The view direction goes
// into the caller's `direction`, which then holds the direction to the target.
#define UNKNOWN_PLACE_CUE() \
    { \
    direction.x = arcadeView->cameraMatrix[0][2]; \
    direction.y = arcadeView->cameraMatrix[1][2]; \
    direction.z = arcadeView->cameraMatrix[2][2]; \
    Vector3 up = Vector3(arcadeView->cameraMatrix[0][1], arcadeView->cameraMatrix[1][1], arcadeView->cameraMatrix[2][1]); \
    Vector3 right = Vector3(arcadeView->cameraMatrix[0][0], arcadeView->cameraMatrix[1][0], arcadeView->cameraMatrix[2][0]); \
    if (arcadeView->fieldOfView != builtFieldOfView) { \
        builtFieldOfView = arcadeView->fieldOfView; \
        float half = (float)tan(UNKNOWN_DEGREES_TO_RADIANS(builtFieldOfView * 0.5f)) * viewDistance; \
        screenOffsetX = (UnknownScreenX() - 0.5f) * half * 2.0f; \
        screenOffsetY = (0.5f - UnknownScreenY()) * arcadeView->aspectRatio * half * 2.0f; \
        modelScale = targetPixels * viewDistance / (arcadeView->focalLength * modelSize); \
    } \
    Vector3 position = arcadeView->cameraPosition; \
    position += direction * viewDistance; \
    position += right * screenOffsetX; \
    position += up * screenOffsetY; \
    SetPosition(&position); \
    }

// 0x0048b100
int VisualCue::Tick(float frameTime) {
    float scale;
    Vector3 direction;

    if (!ArcadeObject::Tick(frameTime))
        return 1;
    if (!cycleCues) {
        if (((UnknownCueOwner*)renderContext)->currentCamera != arcadeView ||
            cueView->raceCamera->followedBike != cueView->ownRacer || !cueView->ownRacer ||
            cueView->ownRacer->finished) {
            isVisible = 0;
            return 1;
        }
    }
    switch (reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->cameraMode) {
    case 2:
    case 3: {
        if (!cueView->raceTrack)
            { isVisible = 0; return 1; }
        if (cueView->ownRacer->onTrack) {
            isVisible = 0;
            return 1;
        }
        isVisible = 1;
        UNKNOWN_PLACE_CUE();
        Vector3 target;
        if (cueView->raceTrack)
            cueView->raceTrack->PointAt(cueView->ownRacer->statusNode->progressPos, &target);
        else
            target = cueView->ownRacer->trackPoint;
        groundTerrain->QueryGround(&target, 0, 0, 0);
        direction = target - arcadeView->cameraPosition;
        SetAxes(&direction, &kVec3YAxis, 1, 1);
        scale = modelScale;
        arcadeModel->Scale(scale, scale, scale);
        return 1;
    }
    case 1:
    case 5: {
        UNKNOWN_PLACE_CUE();
        UnknownKrustyRacerRef* ref = reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->bajaEnduroEvent->raceView->raceCamera->followedRacer;
        if (!ref)
            return 1;
        direction =
            reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->bajaEnduroEvent->gates[ref->gateIndex].position - arcadeView->cameraPosition;
        SetAxes(&direction, &kVec3YAxis, 1, 1);
        scale = modelScale;
        break;
    }
    case 0: {
        if (!cycleCues || cueRacerCount <= 1)
            { isVisible = 0; return 1; }
        isVisible = 1;
        UNKNOWN_PLACE_CUE();
        Vector3 target = kVec3Zero;
        for (int i = 0; i < cueRacerCount; i++) {
            if (currentRacer == i) {
                target = cueRacers[i]->position;
                break;
            }
        }
        direction = target - arcadeView->cameraPosition;
        SetAxes(&direction, &kVec3YAxis, 1, 1);
        scale = modelScale;
        arcadeModel->Scale(scale, scale, scale);
        return 1;
    }
    case 4: {
        UNKNOWN_PLACE_CUE();
        if (reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->tagBall) {
            Vector3 target;
            reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->tagEvent->ball->sceneObject.GetPosition(&target);
            direction = target - arcadeView->cameraPosition;
            SetAxes(&direction, &kVec3YAxis, 1, 1);
            scale = modelScale;
        } else {
            UnknownVisualCueRacer* followed = 0;
            if (reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->tagEvent->raceCamera->statusFlags & 1)
                followed = reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->tagEvent->raceCamera->followedRacer;
            if (!reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->tagEvent->followedRacer)
                return 1;
            if (followed == reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->tagEvent->followedRacer)
                { isVisible = 0; return 1; }
            direction = reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->tagEvent->followedRacer->position - arcadeView->cameraPosition;
            SetAxes(&direction, &kVec3YAxis, 1, 1);
            scale = modelScale;
        }
        break;
    }
    default:
        return 1;
    }
    arcadeModel->Scale(scale, scale, scale);
    isVisible = 1;
    return 1;
}
