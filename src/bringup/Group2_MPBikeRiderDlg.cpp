// Group2_MPBikeRiderDlg.cpp -- link bring-up bodies for three MPBikeRiderDlg
// members that SelectGamePicProcs.cpp references but nothing defines yet.
// Best-guess admission (docs/GAME_BUILD.md): not byte-matched.
//  * FillBikeRiderLists (0x004f8220) and Tick (0x004f8820) are extracted from
//    samples/ui/SelectGamePicProcsNearMisses.cpp (same flow and calls as
//    retail; register allocation / copy scheduling differ).
//  * ApplyChosenRider (0x004500d0, 11 bytes) decoded from retail:
//    `mov dword ptr [ecx+0x7f88], 1; ret`.
// The headers are SelectGamePicProcs.cpp's, so the class layouts agree.
#include "GUIManagerCore.h"
#include "SelectGamePicProcs.h"

#include "DialogEventKind.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BackgroundImage.h"
#include "Camera.h"
#include "DebugAlloc.h"
#include "DlgProcs.h"
#include "EventManager.h"
#include "../krusty2/math/FastMath.h"
#include "MatrixUtil.h"
#include "Net.h"
#include "TextureMap.h"
#include "Track.h"
#include "TrackGame.h"
#include "../krusty2/motion/D3DIMSoultreeCharacter.h"

// This unit's copy of the Y axis constant (SelectGamePicProcs.cpp keeps its
// own file-static copies; Tick passes it to D3DRMVectorRotate).
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);

// d3drm.dll's D3DRMVectorRotate (declared as in SelectGamePicProcs.cpp).
extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

// Inline vector helpers of the bike views (as in SelectGamePicProcs.cpp).
static inline Vector3 UnknownVectorDifference(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 UnknownVectorSum(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

// 0x004a8b10, the model's SetMotionByName, is Character::SetMotionByName
// (Motnctrl.cpp); SelectGamePicProcs.h's UnknownKrustyUIModelObject view
// declares it under its own name, which nothing defines.  Call the defined
// one on the same object pointer (as BikeRace.cpp does for the flag girl).
static inline void PlayModelMotion(UnknownKrustyUIModelObject* model, const char* motion) {
    reinterpret_cast<Character*>(model)->SetMotionByName(motion);
}

static inline Vector3& operator*=(Vector3& v, float scale) {
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
    return v;
}

// 0x004500d0 lies in dlgprocs.cpp's code: marks the rider as changed
// (riderChanged, +0x7f88). best guess: name per the header; the consumer is unverified.
void MPBikeRiderDlg::ApplyChosenRider() {
    riderChanged = 1;
}

// 0x004f8220
void MPBikeRiderDlg::FillBikeRiderLists() {
    int bike = 0;
    int rider = 0;
    char text[0x80];
    MPBikeRiderDlg* self = this;
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    list->RemoveAllRows();
    for (int i = 0; i < g_TrackGame->ui->bikeCount; i++) {
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[i];
        if (!entry->engineSizeSelectable) {
            int kind = ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)[entry->model].modelKind;
            if (kind != 2 && kind != 3 && (kind != 10 || !(g_TrackGame->mode.bikeUnlockFlags & 1)))
                continue;
        }
        int flags = g_TrackGame->mode.lobbyPresetFlags;
        if (!flags) {
            sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)[entry->model].displayName,
                    entry->displayName);
            list->AddRow(text, i, flags);
        } else {
            if (entry->engineSizeSelectable)
                continue;
            int allowed = 0;
            int byModel = flags & 4;
            if (byModel) {
                int model = g_TrackGame->networkGameObject->bikeManufacturer;
                if ((model == entry->model || (!model && entry->model == 7)) &&
                    BikeClassOf(entry->engineSize) <= 2) {
                    allowed = 1;
                    if ((flags & 8) && i != g_TrackGame->mode.racerChoice.bikeIndex)
                        allowed = 0;
                }
            }
            if (!(flags & 8) && !byModel && BikeClassOf(entry->engineSize) <= 2)
                allowed = 1;
            if (!allowed)
                continue;
            sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)[entry->model].displayName,
                    entry->displayName);
            list->AddRow(text, i, 0);
        }
        if (g_TrackGame->mode.racerChoice.bikeIndex == i)
            bike = i;
    }
    if (bike)
        list->SelectRowByData(bike);
    else
        list->SelectRow(0);
    list->Sort(1);
    list = static_cast<UIDropDownList*>(self->FindControl("DDLRiders", 6))->listPart;
    list->RemoveAllRows();
    for (int j = 0; j < g_TrackGame->ui->riderCount; j++) {
        UnknownKrustyUIModel* entry = &((UnknownKrustyUIModel*)g_TrackGame->ui->riders)[j];
        if (entry->modelKind == 2 || entry->modelKind == 3 || entry->uiModel) {
            list->AddRow(entry->displayName, j, 0);
            if (!strcmp(((UnknownKrustyUIModel*)g_TrackGame->ui->riders)[j].modelName,
                        g_TrackGame->mode.racerChoice.riderName))
                rider = j;
        }
    }
    list->SelectRowByData(rider);
}

// 0x004f8820: as SPBikeRiderDlg slot 10 (dlgprocs.cpp 0x0044fae0).
int MPBikeRiderDlg::Tick(float frameTime) {
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

