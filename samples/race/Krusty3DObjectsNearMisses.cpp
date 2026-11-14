// Near-miss Krusty3DObjects.cpp candidates, kept out of src/reconstructed
// until they match. See docs/KRUSTY3DOBJECTS.md. Check one with
//   tools/compile.py --compiler vc6 samples/race/Krusty3DObjectsNearMisses.cpp -o X.obj
// and compare the symbol against retail with
// samples/race/Krusty3DObjectsNearMisses.bindings.json.
//
// BonusObjectManager::Tick (0x0048ccc0) is exact in
// src/reconstructed/Krusty3DObjects.cpp: naming the size numerator before
// the cameraDistance store lets VC6 load the camera ahead of the store.
//
// VisualCue::Tick (0x0048b100, 2796 bytes, jump table at
// 0x0048bbec): places the cue in front of the camera by camera mode
// (TrackGame +0x2d74) and turns it toward the followed racer, the next
// gate, the selected racer or the free camera's target. Retail expands the
// placement block and every Vector3 operator in it in all four modes (its
// 16 calls are the ones below, in the same order). An inline member
// function for the block makes VC6 call the Vector3 constructor and
// operator* (0x00404e60, 0x005015b0) out of line; the block written as a
// macro (UNKNOWN_PLACE_CUE) expands everything as retail does (aligned
// instructions 299 -> 604 of 748). Retail copies the camera's third
// matrix column with integer moves into a stack vector: one function-scope
// `direction` that the placement block fills and each mode then overwrites
// with the direction to its target gives that copy (254 -> 401 of 2820);
// a separate local per block keeps two components on the x87 stack
// (member-wise copies in either order, a by-value column helper: same).
// Every hide path is written as an explicit `isVisible = 0; return 1;`:
// VC6 then caches 0 in a register as retail does (172 -> 254), but picks
// ebx (the `finished` byte test becomes `cmp byte, bl`, retail loads cl
// and keeps 0 in edi, pushing ebp) and still merges the hide blocks into
// one return where retail gives the two early ones their own epilogues.
// Retail's layout: the first hide, case 2/3's onTrack hide and case 4's
// hide fall through into their own epilogues, and only the raceTrack and
// case 0 hides jump into case 4's; VC6 here moves one merged hide after the
// final epilogue. Still 401: `return isVisible = 0, 1;` for the early
// hides, `finished` tested as `*(bool*)&`, `!= 0` or `(unsigned char)`, and
// `scale` initialised (0, 1 or modelScale) before the switch.
// Retail computes `0.5f - screenY` as `fld 0.5; fsub [esi+0x4c]`; reading
// the field directly (not through UnknownScreenY) gives that order but
// changes the `position +=` x87 sequence (fxch/fadd/fstp), 401 -> 383; a
// screenY local: 401. The half field of view goes through a macro: as an
// inline function, VC6 folded `* 0.5f * 0.01745329f` into one 0.0087266f
// multiply, where retail keeps both (fmul 0x005507f4, fmul 0x00550480); the
// macro restores retail's two constants (candidate 2784 -> 2808 bytes of
// 2796, same-mnemonic alignment 653 -> 657 of 752 instructions).

#include "../../src/reconstructed/TrackGameGlobal.h"
#include <math.h>

#include "../../src/reconstructed/Krusty3DObjects.h"

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

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);

#define UNKNOWN_DEGREES_TO_RADIANS(degrees) ((degrees) * 0.01745329f)


// The owner a VisualCue compares its camera with (its +0x18).
struct UnknownCueOwner {
    unsigned char field_0x00[0x08];
    ArcadeCameraView* currentCamera;          // +0x08
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
    switch (reinterpret_cast<CueTrackGameView*>(g_TrackGame)->cameraMode) {
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
        CueGateKrustyBikeView* ref = reinterpret_cast<CueTrackGameView*>(g_TrackGame)->bajaEnduroEvent->raceView->raceCamera->followedRacer;
        if (!ref)
            return 1;
        direction =
            reinterpret_cast<CueTrackGameView*>(g_TrackGame)->bajaEnduroEvent->gates[ref->gateIndex].position - arcadeView->cameraPosition;
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
        if (reinterpret_cast<CueTrackGameView*>(g_TrackGame)->tagBall) {
            Vector3 target;
            reinterpret_cast<CueTrackGameView*>(g_TrackGame)->tagEvent->ball->sceneObject.GetPosition(&target);
            direction = target - arcadeView->cameraPosition;
            SetAxes(&direction, &kVec3YAxis, 1, 1);
            scale = modelScale;
        } else {
            VisualCueKrustyBikeView* followed = 0;
            if (reinterpret_cast<CueTrackGameView*>(g_TrackGame)->tagEvent->raceCamera->statusFlags & 1)
                followed = reinterpret_cast<CueTrackGameView*>(g_TrackGame)->tagEvent->raceCamera->followedRacer;
            if (!reinterpret_cast<CueTrackGameView*>(g_TrackGame)->tagEvent->followedRacer)
                return 1;
            if (followed == reinterpret_cast<CueTrackGameView*>(g_TrackGame)->tagEvent->followedRacer)
                { isVisible = 0; return 1; }
            direction = reinterpret_cast<CueTrackGameView*>(g_TrackGame)->tagEvent->followedRacer->position - arcadeView->cameraPosition;
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

