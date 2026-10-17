// Near-miss Krusty3DObjects.cpp candidates, kept out of src/reconstructed
// until they match. See docs/KRUSTY3DOBJECTS.md. Check one with
//   tools/compile.py --compiler vc6 samples/race/Krusty3DObjectsNearMisses.cpp -o X.obj
// and compare the symbol against retail with
// samples/race/Krusty3DObjectsNearMisses.bindings.json.
//
// BonusObjectManager::Tick (0x0048ccc0, 1308 bytes): steps
// and places the bonus frame. 1299 of 1308 bytes match. The size factor's
// denominator multiplies `fld [camera+0x198]; fmul [this+0x1e4]` as in
// retail once the camera's value is read through a by-value accessor
// (CameraZoom below; written plainly VC6 loads this+0x1e4 first, 1295/1308,
// whichever operand order). Left: retail loads camera before storing the
// length to field_0x1ec, VC6 here after. A separate length local, a
// chained assignment, the length stored in its own statement, a
// reference parameter for the camera, a camera local or by-value camera
// struct, BonusZoom/denominator/size helpers, ten parenthesisations of the
// size expression, `field_0x1f0 = 0` moved, camera re-read, `this->`
// spellings and Length helper forms (result local, ?:, != tests) leave
// that order (at best 1299; parentheses around field_0x1e4 or a plain
// camera->field_0x198 give 1295). Storing the length after the size
// statement gives retail's `mov edx` first but keeps the length on the x87
// stack (`fld st(0)`, store at the end; 544). Size helpers taking the length
// by value, or the camera, `this` and the chained store in any argument
// order, give 600 or the same 1299.
// Cycle 3 sweep (156 forms): a `UnknownBonusCamera& view = *camera`
// alias (1164), a `UnknownBonusCamera* const&` alias of camera (1299),
// a `const float&` alias of the camera's +0x198 (1152), a `float&` alias of
// field_0x1ec for the chained store, and all 24 orders of the alias, the
// placedFieldOfView store, the delta and `field_0x1f0 = 0`: at best the same 1299.
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
// instructions 299 -> 604 of 748). Left: retail copies the camera's third
// matrix column with integer moves into a stack vector, where VC6 keeps two
// of its components on the x87 stack (member-wise copies in either order,
// a by-value column helper: same); retail keeps 0 in edi (pushing ebp)
// and gives the two explicit early returns their own epilogues, where this
// candidate caches no zero and jumps to one shared return (writing every
// `goto hide` as an explicit return makes VC6 cache 0 in ebx but still
// shares the epilogue). The half field of view goes through a macro: as an
// inline function, VC6 folded `* 0.5f * 0.01745329f` into one 0.0087266f
// multiply, where retail keeps both (fmul 0x005507f4, fmul 0x00550480); the
// macro restores retail's two constants (candidate 2784 -> 2808 bytes of
// 2796, same-mnemonic alignment 653 -> 657 of 752 instructions).

#include <math.h>

#include "../../src/reconstructed/Krusty3DObjects.h"

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


static const UnknownBonusKey g_UnknownBonusKeys[7] = {
    {0, 0.0f, {0.0275f, 0.0275f, 0.0275f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {6, 0.4f, {2.78f, 2.78f, 2.78f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {8, 0.5333f, {2.09f, 2.09f, 2.09f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {25, 1.6667f, {2.09f, 2.09f, 2.09f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {33, 2.2f, {1.99f, 1.99f, 1.99f}, {0.0f, 18.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 3.1415923f},
    {40, 2.6667f, {1.18f, 1.18f, 1.18f}, {0.0f, 29.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 4.607669f},
    {90, 6.0f, {0.05f, 0.05f, 0.05f}, {0.0f, 117.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 15.079643f},
};


// 0x0048ccc0
int BonusObjectManager::Tick(float frameTime) {
    int digit;
    int place;

    if (currentKey == -1 || g_UnknownKrustyGame56e26c->uiInteractionBlocked)
        return 1;
    if (raceView->localRacer->crashState && elapsed > 2.0f) {
        HideDigits();
        return 1;
    }
    elapsed += frameTime;
    if (elapsed < g_UnknownBonusKeys[6].time) {
        while (elapsed > g_UnknownBonusKeys[currentKey + 1].time)
            NextKey();
        float t = (elapsed - g_UnknownBonusKeys[currentKey].time) / keyDuration;
        Vector3 position;
        raceView->localRacer->modelNode->GetPosition(&position);
        position += t * keyPositionChange + keyPosition;
        baseFrame->TranslateIn(0, position);
        if (camera != g_UnknownKrustyGame56e26c->renderTarget->currentCamera || camera->fieldOfView != placedFieldOfView ||
            isResizePending) {
            camera = g_UnknownKrustyGame56e26c->renderTarget->currentCamera;
            placedFieldOfView = camera->fieldOfView;
            Vector3 delta = position - camera->eyePosition;
            isResizePending = 0;
            sizeFactor = (cameraDistance = Length(delta)) * targetScreenWidth / (CameraZoom(camera) * frameWidth);
        }
        position += raceView->localRacer->rearWheel->rollDirection * 5.0f;
        if (keyAngleChange == 0.0f && keyAngle == 0.0f) {
            eyeToFrame = position - camera->eyePosition;
            Vector3 forward = eyeToFrame;
            forward.y = 0.0f;
            baseFrame->SetAxesPtr(&forward, &kVec3YAxis, 1, 1);
        } else {
            Vector3 axis = t * keyAxisChange + keyAxis;
            baseFrame->SetRotation(axis.x, axis.y, axis.z, t * keyAngleChange + keyAngle);
        }
        Vector3 scale = t * keyScaleChange + keyScale;
        scale *= sizeFactor;
        baseFrame->Scale(scale.x, scale.y, scale.z);
        digitModels[0][0]->subtreeDirty = !digitModels[0][0]->field_0x14c;
        digitModels[0][1]->subtreeDirty = !digitModels[0][0]->field_0x14c;
        for (digit = 0; digit < 10; digit++)
            for (place = 2; place < 5; place++)
                digitModels[digit][place]->subtreeDirty = !digitModels[digit][place]->field_0x14c;
        for (digit = 0; digit < 10; digit++)
            for (place = 0; place < 2; place++)
                fractionDigitModels[digit][place]->subtreeDirty = !fractionDigitModels[digit][place]->field_0x14c;
        timesModel->subtreeDirty = !timesModel->field_0x14c;
        decimalPointModel->subtreeDirty = !decimalPointModel->field_0x14c;
    } else {
        HideDigits();
    }
    GameObject::Tick(frameTime);
    return 1;
}


// The owner a VisualCue compares its camera with (its +0x18).
struct UnknownCueOwner {
    unsigned char field_0x00[0x08];
    UnknownArcadeView* currentCamera;          // +0x08
};

// Places the cue at its screen position in front of the camera, rebuilding
// the screen offsets when the field of view changed.
#define UNKNOWN_PLACE_CUE() \
    { \
    Vector3 forward; \
    forward.x = arcadeView->cameraMatrix[0][2]; \
    forward.y = arcadeView->cameraMatrix[1][2]; \
    forward.z = arcadeView->cameraMatrix[2][2]; \
    Vector3 up = Vector3(arcadeView->cameraMatrix[0][1], arcadeView->cameraMatrix[1][1], arcadeView->cameraMatrix[2][1]); \
    Vector3 right = Vector3(arcadeView->cameraMatrix[0][0], arcadeView->cameraMatrix[1][0], arcadeView->cameraMatrix[2][0]); \
    if (arcadeView->fieldOfView != builtFieldOfView) { \
        builtFieldOfView = arcadeView->fieldOfView; \
        float half = (float)tan(UNKNOWN_DEGREES_TO_RADIANS(builtFieldOfView * 0.5f)) * field_0x44; \
        screenOffsetX = (UnknownScreenX() - 0.5f) * half * 2.0f; \
        screenOffsetY = (0.5f - UnknownScreenY()) * arcadeView->aspectRatio * half * 2.0f; \
        modelScale = field_0x38 * field_0x44 / (arcadeView->focalLength * field_0x3c); \
    } \
    Vector3 position = arcadeView->cameraPosition; \
    position += forward * field_0x44; \
    position += right * screenOffsetX; \
    position += up * screenOffsetY; \
    SetPosition(&position); \
    }

// 0x0048b100
int VisualCue::Tick(float frameTime) {
    float scale;

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
    switch (g_UnknownKrustyGame56e26c->cameraMode) {
    case 2:
    case 3: {
        if (!cueView->raceTrack)
            goto hide;
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
        Vector3 direction = target - arcadeView->cameraPosition;
        SetAxes(&direction, &kVec3YAxis, 1, 1);
        scale = modelScale;
        arcadeModel->Scale(scale, scale, scale);
        return 1;
    }
    case 1:
    case 5: {
        UNKNOWN_PLACE_CUE();
        UnknownKrustyRacerRef* ref = g_UnknownKrustyGame56e26c->bajaEnduroEvent->raceView->raceCamera->followedRacer;
        if (!ref)
            return 1;
        Vector3 direction =
            g_UnknownKrustyGame56e26c->bajaEnduroEvent->gates[ref->gateIndex].position - arcadeView->cameraPosition;
        SetAxes(&direction, &kVec3YAxis, 1, 1);
        scale = modelScale;
        break;
    }
    case 0: {
        if (!cycleCues || cueRacerCount <= 1)
            goto hide;
        isVisible = 1;
        UNKNOWN_PLACE_CUE();
        Vector3 target = kVec3Zero;
        for (int i = 0; i < cueRacerCount; i++) {
            if (currentRacer == i) {
                target = cueRacers[i]->position;
                break;
            }
        }
        Vector3 direction = target - arcadeView->cameraPosition;
        SetAxes(&direction, &kVec3YAxis, 1, 1);
        scale = modelScale;
        arcadeModel->Scale(scale, scale, scale);
        return 1;
    }
    case 4: {
        UNKNOWN_PLACE_CUE();
        if (g_UnknownKrustyGame56e26c->tagBall) {
            Vector3 target;
            g_UnknownKrustyGame56e26c->tagEvent->ball->sceneObject.GetPosition(&target);
            Vector3 direction = target - arcadeView->cameraPosition;
            SetAxes(&direction, &kVec3YAxis, 1, 1);
            scale = modelScale;
        } else {
            UnknownVisualCueRacer* followed = 0;
            if (g_UnknownKrustyGame56e26c->tagEvent->raceCamera->statusFlags & 1)
                followed = g_UnknownKrustyGame56e26c->tagEvent->raceCamera->followedRacer;
            if (!g_UnknownKrustyGame56e26c->tagEvent->followedRacer)
                return 1;
            if (followed == g_UnknownKrustyGame56e26c->tagEvent->followedRacer)
                goto hide;
            Vector3 direction = g_UnknownKrustyGame56e26c->tagEvent->followedRacer->position - arcadeView->cameraPosition;
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
hide:
    isVisible = 0;
    return 1;
}

