// Group0_BonusObjectManager.cpp -- bring-up body for BonusObjectManager slot
// 10, extracted from samples/race/Krusty3DObjectsNearMisses.cpp (that sample
// reports 1299 of 1308 retail bytes matching; the residue is an x87 store
// order). Only this function is taken from the sample.
#include "TrackGameGlobal.h"
#include <math.h>

#include "Krusty3DObjects.h"
#include "Group0_SoultreeBridge.h"

// By-value read of the camera's +0x198 value (the sample's byte-order aid).
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

static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);


// 0x0056c820: the bonus animation keys (times are frames / 15); a private
// copy of the static table in src/reconstructed/Krusty3DObjects.cpp.
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

    if (currentKey == -1 || reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->uiInteractionBlocked)
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
        // The header declares a Vector3 GetPosition/TranslateIn nobody defines;
        // call the defined SoultreeObject Vec3 forms (0x004fc970, 0x004fc690).
        Group0_SoultreeGetPosition(raceView->localRacer->modelNode, &position.x);
        position += t * keyPositionChange + keyPosition;
        Group0_SoultreeTranslateIn(baseFrame, 0, &position.x);
        if (camera != reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->renderTarget->currentCamera || camera->fieldOfView != placedFieldOfView ||
            isResizePending) {
            camera = reinterpret_cast<UnknownKrustyGame*>(g_TrackGame)->renderTarget->currentCamera;
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
            baseFrame->SetAxesPtr(reinterpret_cast<const Vec3*>(&forward), reinterpret_cast<const Vec3*>(&kVec3YAxis), 1, 1);
        } else {
            Vector3 axis = t * keyAxisChange + keyAxis;
            baseFrame->SetRotation(axis.x, axis.y, axis.z, t * keyAngleChange + keyAngle);
        }
        Vector3 scale = t * keyScaleChange + keyScale;
        scale *= sizeFactor;
        baseFrame->Scale(scale.x, scale.y, scale.z);
        digitModels[0][0]->subtreeDirty = !digitModels[0][0]->hidden;
        digitModels[0][1]->subtreeDirty = !digitModels[0][0]->hidden;
        for (digit = 0; digit < 10; digit++)
            for (place = 2; place < 5; place++)
                digitModels[digit][place]->subtreeDirty = !digitModels[digit][place]->hidden;
        for (digit = 0; digit < 10; digit++)
            for (place = 0; place < 2; place++)
                fractionDigitModels[digit][place]->subtreeDirty = !fractionDigitModels[digit][place]->hidden;
        timesModel->subtreeDirty = !timesModel->hidden;
        decimalPointModel->subtreeDirty = !decimalPointModel->hidden;
    } else {
        HideDigits();
    }
    GameObject::Tick(frameTime);
    return 1;
}

