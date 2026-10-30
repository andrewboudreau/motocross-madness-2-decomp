// Bring-up definition (best-guess-unreviewed) for 0x0048d1e0, the bonus
// counter's value display. KrustyBike.cpp reaches the object at
// BikeRace+0xbc as KbScoreBoard (KrustyBikeTypes.h); it is the race's
// BonusObjectManager (Krusty3DObjects.h declares the same address as
// BonusObjectManager::ShowValue). The body is written from the decoded
// retail function against BonusObjectManager's layout; not a byte match
// (retail rounds with a `fld; fistp` helper, see docs/KRUSTY3DOBJECTS.md).
#include <math.h>

#include "../reconstructed/Krusty3DObjects.h"

float FastSqrt(float x); // 0x00460b50

// KrustyBikeTypes.h's view, declared here with its one method (the member
// function's mangled name does not depend on the class-key).
struct KbScoreBoard {
    void ShowValue(float points, float multiplier);
};

// fistp under the default x87 control word: round to nearest, ties to even.
static int Group4Ovl_RoundNearest(double value)
{
    double low = floor(value);
    double diff = value - low;
    if (diff > 0.5)
        return (int)low + 1;
    if (diff < 0.5)
        return (int)low;
    return (fmod(low, 2.0) == 0.0) ? (int)low : (int)low + 1;
}

// Shows a model under the base frame (0x004fdb50, mark the subtree dirty,
// 0x004fd910 AddChild).
static void Group4Ovl_ShowModel(BonusObjectManager* self, D3DIMSoultreeObject* model)
{
    model->ClearFlag14c();
    model->subtreeDirty = 1;
    self->baseFrame->AddChild(model);
}

// 0x0048d1e0 (ret 8): hides the old digits, shows `points` rounded to the
// hundred (the two lowest places always 0, four places below 10000, else
// five), and when `multiplier` is at least 1.05 the "x" and decimal point
// with its two digits (tenths). Then starts the key animation and points the
// frame from the camera eye at the local racer's model node, flagging a
// re-size when that distance changed by more than 1.
void KbScoreBoard::ShowValue(float points, float multiplier)
{
    BonusObjectManager* self = reinterpret_cast<BonusObjectManager*>(this);
    int place;
    self->HideDigits();
    int places = points < 10000.0f ? 4 : 5;
    self->shownDigits[4] = -1;
    int remaining = Group4Ovl_RoundNearest(points) / 100;
    self->shownDigits[0] = 0;
    Group4Ovl_ShowModel(self, self->digitModels[0][0]);
    self->shownDigits[1] = 0;
    Group4Ovl_ShowModel(self, self->digitModels[0][1]);
    for (place = 2; place < places; place++) {
        int digit = remaining % 10;
        Group4Ovl_ShowModel(self, self->digitModels[digit][place]);
        self->shownDigits[place] = digit;
        remaining /= 10;
    }
    int resize = 0;
    if (multiplier >= 1.05f) {
        int tenths = Group4Ovl_RoundNearest(multiplier * 10.0f);
        for (place = 0; place < 2; place++) {
            int digit = tenths % 10;
            Group4Ovl_ShowModel(self, self->fractionDigitModels[digit][place]);
            self->shownFractionDigits[place] = digit;
            tenths /= 10;
        }
        Group4Ovl_ShowModel(self, self->timesModel);
        Group4Ovl_ShowModel(self, self->decimalPointModel);
    }
    self->NextKey();
    Vec3 position;
    reinterpret_cast<SoultreeObject*>(self->raceView->localRacer->modelNode)->GetPosition(&position);
    self->eyeToFrame.x = position.x - self->camera->eyePosition.x;
    self->eyeToFrame.y = position.y - self->camera->eyePosition.y;
    self->eyeToFrame.z = position.z - self->camera->eyePosition.z;
    float lengthSquared = self->eyeToFrame.x * self->eyeToFrame.x + self->eyeToFrame.y * self->eyeToFrame.y
                        + self->eyeToFrame.z * self->eyeToFrame.z;
    if (lengthSquared == 0.0f) {
        // 0x0067c308: the shared zero vector.
        self->eyeToFrame.x = 0.0f;
        self->eyeToFrame.y = 0.0f;
        self->eyeToFrame.z = 0.0f;
        self->cameraDistance = 0.0f;
        self->isResizePending = 0;
        return;
    }
    float length = FastSqrt(lengthSquared);
    float inverse = 1.0f / length;
    float change = length - self->cameraDistance;
    if (change < 0.0f)
        change = -change;
    if (change > 1.0f)
        resize = 1;
    self->cameraDistance = length;
    self->isResizePending = resize;
    self->eyeToFrame.x = inverse * self->eyeToFrame.x;
    self->eyeToFrame.y = inverse * self->eyeToFrame.y;
    self->eyeToFrame.z = inverse * self->eyeToFrame.z;
}
