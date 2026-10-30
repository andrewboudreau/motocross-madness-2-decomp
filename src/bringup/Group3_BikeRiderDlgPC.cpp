// Bring-up definition of PCCentralBikeRiderDlg::Tick (retail 0x004d72c0,
// ProCircuitProcs.cpp).
//
// Status: best-guess-unreviewed bring-up body, extracted from the near miss in
// samples/ui/ProCircuitProcsNearMisses.cpp (1237 of 1291 bytes matching; only
// the scheduling of the by-value Vector3 copy for the camera's LookAt differs).
// Not a strict match; it exists so the source-built game links. The TU-local
// helpers of ProCircuitProcs.cpp are replicated here as statics.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ProCircuitProcs.h"

#include "DialogEventKind.h"
#include "SelectGamePicProcs.h"
#include "KrustyUI.h"
#include "MenuId.h"
#include "TrackGame.h"
#include "EventManager.h"
#include "MatrixUtil.h"
#include "DebugAlloc.h"
#include "../krusty2/math/FastMath.h"
#include "OptionProcs.h"
#include "BackgroundImage.h"
#include "Camera.h"
#include "../krusty2/motion/D3DIMSoultreeCharacter.h"

// d3drm.dll's D3DRMVectorRotate.
extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

static inline Vector3 UnknownVectorDifference(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 UnknownVectorSum(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3& operator*=(Vector3& v, float scale) {
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
    return v;
}

// ProCircuitProcs.cpp's own copy of the Y axis (retail 0x00689b28).
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);

// 0x004a8b10, the model's SetMotionByName, is Character::SetMotionByName
// (Motnctrl.cpp); SelectGamePicProcs.h's UnknownKrustyUIModelObject view declares it
// under its own name, which nothing defines. Retail passes the model pointer
// unchanged as `this`, so call the defined one on the same pointer.
static inline void PlayModelMotion(UnknownKrustyUIModelObject* model, const char* motion) {
    reinterpret_cast<Character*>(model)->SetMotionByName(motion);
}

// 0x004d72c0: as SPBikeRiderDlg::Tick (0x0044fae0), and the chosen bike's
// model also returns to "WaitB" when the rider idles again.
int PCCentralBikeRiderDlg::Tick(float frameTime)
{
    if (dialogBackground)
        dialogBackground->MarkAllDirty();
    Vector3 offset;
    Vector3 turned;
    GUIInputDevice* pointer = guiUser->pointerDevice;
    if (pointer && previewDragged && previewDragging) {
        float dx = (float)(pointer->pointerPosition.x - guiUser->pointerX);
        viewEye.y += (pointer->pointerPosition.y - guiUser->pointerY) * 0.1f;
        viewEye.y = __max(1.0f, __min(viewEye.y, 10.0f));
        offset = UnknownVectorDifference(viewEye, g_TrackGame->ui->garageFocus);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, dx * 0.015707964f);
        float distance = viewDistance;
        turned *= distance;
        viewEye = UnknownVectorSum(turned, g_TrackGame->ui->garageFocus);
        g_TrackGame->ui->garageCamera->SetFrame(&viewEye, 0, 0, 0, 0);
        g_TrackGame->ui->garageCamera->LookAt(viewTarget);
        previewDragged = 1;
        static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->SetCurrentState(0);
    } else if (!previewDragged) {
        offset = UnknownVectorDifference(viewEye, g_TrackGame->ui->garageFocus);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, frameTime * 0.39269909f);
        turned *= viewDistance;
        viewEye = UnknownVectorSum(turned, g_TrackGame->ui->garageFocus);
        g_TrackGame->ui->garageCamera->SetFrame(&viewEye, 0, 0, 0, 0);
        g_TrackGame->ui->garageCamera->LookAt(viewTarget);
    }
    g_TrackGame->ui->riderPreview->ModelVirtualSlot7(frameTime, 0, 0);
    g_TrackGame->ui->garageCharacter->ModelVirtualSlot7(frameTime, 0, 0);
    if (idleMotionPlaying && rand() % 200 == 1) {
        UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        bikes->GetRowData(-1);
        switch (rand() % 3) {
        case 0:
            PlayModelMotion(g_TrackGame->ui->riderPreview, "LookLeftR");
            break;
        case 1:
            PlayModelMotion(g_TrackGame->ui->riderPreview, "LookRightR");
            break;
        case 2:
            PlayModelMotion(g_TrackGame->ui->riderPreview, "StretchR");
            break;
        }
        idleMotionPlaying = 0;
    }
    if (!idleMotionPlaying && g_TrackGame->ui->riderPreview->motionFinished) {
        UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        UnknownKrustyUIModel* model =
            &((UnknownKrustyUIModel*)ui->manufacturers)[((UnknownKrustyUIBike*)ui->bikes)[bikes->GetRowData(-1)].model];
        PlayModelMotion(model->uiModel, "WaitB");
        PlayModelMotion(g_TrackGame->ui->riderPreview, "WaitR");
        idleMotionPlaying = 1;
    }
    if (dialogBackground)
        dialogBackground->SetRegionRect(previewRegion, (CameraRect*)&previewArea);
    return UIDialog::Tick(frameTime);
}
