// Bring-up definition (best-guess-unreviewed) for 0x0048d1e0, the bonus
// counter's value display. KrustyBike.cpp reaches the object at
// BikeRace+0xbc as KbScoreBoard (KrustyBikeTypes.h); it is the race's
// BonusObjectManager (Krusty3DObjects.h declares the same address as
// BonusObjectManager::ShowValue). The body is written from the decoded
// retail function against BonusObjectManager's layout; not a byte match
// (retail rounds with a `fld; fistp` helper at 0x0048d221 and 0x0048d33a, see
// docs/KRUSTY3DOBJECTS.md).
// Audited against the retail disassembly on 2026-11-11 for the linked game: no
// behavioural difference (places, the -1 sentinels, the hundred rounding, the
// digit-model indexing, the 1.05 threshold, the multiplier*10 conversion, the
// fraction digits, NextKey, the eye distance through FastSqrt and the scaling).
// The differential run (tools/emu_cases/scoreboard_show_value.py) then found a
// last-bit difference in the squared eye distance under 53-bit precision (18 of
// 2000 runs), fixed by building the vector in a local; now 0 of 6000 runs differ
// under 53- and 24-bit precision.
// Rounding: the game build compiles this unit with /QIfist (config/game_build.json
// bringup_flags), so FistpFloatToInt's cast is a FISTP under the current rounding
// mode, as retail's helper; these two are the unit's only float-to-int conversions.
// Range (inference from the callers): `points` is one trick sequence's score, which
// the caller (KrustyBike 0x00495ff0) converts to int itself after multiplying it by
// the multiplier (>= 1), and the multiplier is a trick count plus 0.5, so both
// results stay inside int, where the 64-bit store equals retail's 32-bit one.
// The rest of the gap is retail's EBP frame from the asm helper.
#include "../reconstructed/Krusty3DObjects.h"
#include "../../samples/common/FistpFloatToInt.h"

float FastSqrt(float x); // 0x00460b50

// KrustyBikeTypes.h's view, declared here with its one method (the member
// function's mangled name does not depend on the class-key).
struct KbScoreBoard {
    void ShowValue(float points, float multiplier);
};

// Shows a model under the base frame (0x004fdb50, mark the subtree dirty,
// 0x004fd910 AddChild).
// Retail's inline-asm fistp helper gives ShowValue an EBP frame; the pragma keeps the
// frame pointer for it and its helper, which retail expands inside it
// (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
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
    int remaining;
    FistpFloatToInt(&remaining, points);
    remaining /= 100;
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
        int tenths;
        FistpFloatToInt(&tenths, multiplier * 10.0f);
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
    // Retail stores the three differences, copies them into eyeToFrame and
    // squares the stored floats; assigning the fields directly let VC6 keep
    // the unrounded z in ST0 (a last-bit difference under 53-bit precision,
    // tools/emu_cases/scoreboard_show_value.py).
    Vector3 delta;
    delta.x = position.x - self->camera->eyePosition.x;
    delta.y = position.y - self->camera->eyePosition.y;
    delta.z = position.z - self->camera->eyePosition.z;
    self->eyeToFrame = delta;
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
#pragma optimize("", on)
