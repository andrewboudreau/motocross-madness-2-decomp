// Bring-up definition of SPBikeRiderDlg::Tick (retail 0x0044fae0, DlgProcs.cpp).
//
// Status: best-guess-unreviewed bring-up body, extracted from the near miss in
// samples/ui/DlgProcsNearMisses.cpp (1181 of 1235 bytes matching; only the
// scheduling of the by-value Vector3 copy passed to the camera's LookAt
// differs). Not a strict match; it exists so the source-built game links.
// The TU-local helpers of DlgProcs.cpp are replicated here as statics.
#include "BikeRaceCore.h"
#include "GUIManagerCore.h"
#include "DlgProcs.h"

#include "DialogEventKind.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BackgroundImage.h"
#include "Camera.h"
#include "D3DIMSoulTree.h"
#include "DebugAlloc.h"
#include "../krusty2/math/FastMath.h"
#include "InGameProcs.h"
#include "MatrixUtil.h"
#include "MenuId.h"
#include "Net.h"
#include "NetProcs.h"
#include "OptionProcs.h"
#include "ProCircuitProcs.h"
#include "RenderTarget.h"
#include "SelectGamePicProcs.h"
#include "TrackGame.h"
#include "EventManager.h"
#include "BaseQuarryEvent.h"
#include "TrackRecordDlg.h"
#include "../krusty2/motion/D3DIMSoultreeCharacter.h"

// DlgProcs.cpp's own copy of the Y axis (retail 0x0059add0).
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);

// d3drm.dll's D3DRMVectorRotate.
extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

static inline Vector3 UnknownVectorDifference(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 UnknownVectorSum(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

// 0x004a8b10, the preview model's SetMotionByName, is Character::SetMotionByName
// (Motnctrl.cpp); SelectGamePicProcs.h's UnknownKrustyUIModelObject view declares it
// under its own name, which nothing defines. Retail passes the model pointer
// unchanged as `this`, so call the defined one on the same pointer.
static inline void PlayModelMotion(UnknownKrustyUIModelObject* model, const char* motion) {
    reinterpret_cast<Character*>(model)->SetMotionByName(motion);
}

static inline Vector3& operator*=(Vector3& v, float scale) {
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
    return v;
}

// 0x0044fae0: turns the bike view (dragged with the pointer, else slowly by
// itself), animates the previews and now and then plays a rider idle motion.
int SPBikeRiderDlg::Tick(float frameTime) {
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
        // pi/200 radians per pointer pixel.
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, dx * 0.015707964f);
        float distance = viewDistance;
        turned *= distance;
        viewEye = UnknownVectorSum(turned, g_TrackGame->ui->garageFocus);
        g_TrackGame->ui->garageCamera->SetFrame(&viewEye, 0, 0, 0, 0);
        g_TrackGame->ui->garageCamera->LookAt(viewTarget);
        previewDragged = 1;
        static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->SetCurrentState(0);
    } else if (!previewDragged) {
        // Auto rotation: pi/8 radians per second.
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
        bikes->GetRowData(-1);
        PlayModelMotion(g_TrackGame->ui->riderPreview, "WaitR");
        idleMotionPlaying = 1;
    }
    if (dialogBackground)
        dialogBackground->SetRegionRect(previewRegion, &previewArea);
    return UIDialog::Tick(frameTime);
}
