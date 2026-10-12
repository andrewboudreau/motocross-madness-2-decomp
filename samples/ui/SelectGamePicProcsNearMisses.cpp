// Near misses for SelectGamePicProcs.cpp (canonical file:
// src/reconstructed/SelectGamePicProcs.cpp, included below for its types and
// its matched functions). Check: compile this file and compare each function
// with SelectGamePicProcsNearMisses.bindings.json.
//
// MultiPlayerDlg::AddChatLine (0x004f3720, 361 bytes): the same
//   calls and strings; retail lays the system-message branch and the player
//   colour loop out after the "~1" branch and merges the "%s: %s" sprintf
//   into the first branch's call, VC6 here keeps source order. Inverting
//   the tests, swapping the if/else arms and moving the IME test do not
//   reproduce the layout. With the system branch first (`player == -1`,
//   `< 0` or `-1 == player`, 62) VC6 merges the system sprintf calls into
//   the player branch's as retail does, but keeps the system block inline
//   instead of moving it after the "~1" branch. The IME sprintfs as a
//   ternary format (104, 48 diffs), `if (imeLibrary)` first (101), a
//   TrackGame local and an else-if GetPlayerName test do not move it.
// MPBikeRiderDlg::FillBikeRiderLists (0x004f8220, 833 bytes): the same
//   flow and calls; retail keeps the bike index in edi and KrustyUI in ebp
//   and lays the locals out differently (VC6 here swaps the registers).
//   Dropping the `this` copy, hoisting the index and caching KrustyUI do
//   not move it.
// MPBikeRiderDlg::UnknownVirtualSlot10 (0x004f8820, 1268 bytes, 1214
//   match): as SPBikeRiderDlg slot 10 (DlgProcsNearMisses.cpp), only the
//   scheduling of the by-value Vector3 copies for the camera call differs.
// MultiPlayerDlg::SendStartMessage (0x004f2340, 2120 bytes): the
//   same flow, calls, frame size and message layout; register allocation
//   differs: retail keeps the racer total in memory (+0x1c) and the bike,
//   rider and name arrays in edi/ebx/ebp, VC6 here enregisters the total.

#include "../../src/reconstructed/SelectGamePicProcs.cpp"

// 0x004f3720
void MultiPlayerDlg::AddChatLine(int player, const char* text) {
    char line[0x60];
    char name[0x80];
    char system[0x80];
    UIListBox* list = static_cast<UIListBox*>(FindControl("ListChat", 3));
    if (!list)
        return;
    if (player != -1) {
        if (!g_TrackGame->network->GetPlayerName(player, name))
            return;
        sprintf(line, "%s: %s", name, text);
    } else {
        g_TrackGame->LoadResourceString(0x14ba, system, 0x80);
        if (!g_TrackGame->imeLibrary)
            sprintf(line, "~5%s: %s", system, text);
        else
            sprintf(line, "%s: %s", system, text);
    }
    if (!g_TrackGame->imeLibrary) {
        if (g_TrackGame->network->localPlayer == player) {
            sprintf(line, "~1%s: ~0%s", name, text);
        } else {
            for (int i = 0; i < 7; i++) {
                if (g_UnknownGlobal689d08[i].id == player)
                    sprintf(line, "~%1d%s: ~0%s", i + 2, name, text);
            }
        }
    }
    int rows = list->rowCount;
    if ((short)rows >= 100)
        list->RemoveRow(0);
    list->AddRow(line, 0, 0);
    list->ScrollToRow((short)rows, 1);
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
        if (!entry->field_0x88) {
            int kind = ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)[entry->model].modelKind;
            if (kind != 2 && kind != 3 && (kind != 10 || !(g_TrackGame->mode.field_0x1970 & 1)))
                continue;
        }
        int flags = g_TrackGame->mode.field_0x1bd4;
        if (!flags) {
            sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)[entry->model].displayName,
                    entry->displayName);
            list->AddRow(text, i, flags);
        } else {
            if (entry->field_0x88)
                continue;
            int allowed = 0;
            int byModel = flags & 4;
            if (byModel) {
                int model = g_TrackGame->networkGameObject->field_0x298;
                if ((model == entry->model || (!model && entry->model == 7)) &&
                    UnknownBikeClassOf(entry->engineSize) <= 2) {
                    allowed = 1;
                    if ((flags & 8) && i != g_TrackGame->mode.field_0x1974.field_0xc4)
                        allowed = 0;
                }
            }
            if (!(flags & 8) && !byModel && UnknownBikeClassOf(entry->engineSize) <= 2)
                allowed = 1;
            if (!allowed)
                continue;
            sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)[entry->model].displayName,
                    entry->displayName);
            list->AddRow(text, i, 0);
        }
        if (g_TrackGame->mode.field_0x1974.field_0xc4 == i)
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
        if (entry->modelKind == 2 || entry->modelKind == 3 || entry->field_0xc0) {
            list->AddRow(entry->displayName, j, 0);
            if (!strcmp(((UnknownKrustyUIModel*)g_TrackGame->ui->riders)[j].modelName,
                        g_TrackGame->mode.field_0x1974.field_0x80))
                rider = j;
        }
    }
    list->SelectRowByData(rider);
}

// 0x004f8820: as SPBikeRiderDlg slot 10 (dlgprocs.cpp 0x0044fae0).
int MPBikeRiderDlg::UnknownVirtualSlot10(float frameTime) {
    if (dialogBackground)
        dialogBackground->UnknownFunction404da0();
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
        g_TrackGame->ui->garageCamera->UnknownFunction42e9b0(&viewEye, 0, 0, 0, 0);
        g_TrackGame->ui->garageCamera->UnknownVirtualSlot29(viewTarget);
        previewDragged = 1;
    } else if (!previewDragged) {
        offset = UnknownVectorDifference(viewEye, g_TrackGame->ui->garageFocus);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, frameTime * 0.39269909f);
        turned *= viewDistance;
        viewEye = UnknownVectorSum(turned, g_TrackGame->ui->garageFocus);
        g_TrackGame->ui->garageCamera->UnknownFunction42e9b0(&viewEye, 0, 0, 0, 0);
        g_TrackGame->ui->garageCamera->UnknownVirtualSlot29(viewTarget);
    }
    g_TrackGame->ui->riderPreview->ModelVirtualSlot7(frameTime, 0, 0);
    g_TrackGame->ui->garageCharacter->ModelVirtualSlot7(frameTime, 0, 0);
    if (idleMotionPlaying && rand() % 200 == 1) {
        UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        bikes->GetRowData(-1);
        switch (rand() % 3) {
        case 0:
            g_TrackGame->ui->riderPreview->UnknownFunction4a8b10("LookLeftR");
            break;
        case 1:
            g_TrackGame->ui->riderPreview->UnknownFunction4a8b10("LookRightR");
            break;
        case 2:
            g_TrackGame->ui->riderPreview->UnknownFunction4a8b10("StretchR");
            break;
        }
        idleMotionPlaying = 0;
    }
    if (!idleMotionPlaying && g_TrackGame->ui->riderPreview->motionFinished) {
        UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        UnknownKrustyUIModel* model =
            &((UnknownKrustyUIModel*)ui->manufacturers)[((UnknownKrustyUIBike*)ui->bikes)[bikes->GetRowData(-1)].model];
        model->field_0xc0->UnknownFunction4a8b10("WaitB");
        g_TrackGame->ui->riderPreview->UnknownFunction4a8b10("WaitR");
        idleMotionPlaying = 1;
    }
    if (dialogBackground)
        dialogBackground->UnknownFunction404240(previewRegion, (CameraRect*)&previewArea);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// A random value in [0, 1) (as in KrustyUI.cpp).
static inline float RandomUnit() {
    return (float)(rand() * (1.0f / 32768));
}

// 0x004f2340: sends the lobby start message; with `start` it first draws
// the AI racers and the starting grid.
void MultiPlayerDlg::SendStartMessage(short start) {
    int i;
    g_TrackGame->mode.field_0x27f8.field_0x35 = g_UnknownGlobal689df8 + 1;
    g_TrackGame->field_0x18 = g_TrackGame->mode.field_0x27f8.field_0x35;
    g_TrackGame->field_0x3424 = g_TrackGame->mode.field_0x27f8.field_0x28;
    g_TrackGame->mode.field_0x1be0 = g_TrackGame->field_0x3424 + g_TrackGame->field_0x18;
    int total = g_TrackGame->mode.field_0x1be0;
    g_TrackGame->mode.field_0x1bd8 =
        lobbySlots.GrantRacers(g_UnknownGlobal689df8 + 1, g_TrackGame->mode.field_0x27f8.field_0x28);
    if (g_TrackGame->mode.field_0x1bd8 < g_TrackGame->mode.field_0x27f8.field_0x28)
        g_TrackGame->mode.field_0x27f8.field_0x28 = g_TrackGame->mode.field_0x1bd8;
    if (start) {
        int* bikes = (int*)DebugCalloc(total, 4, __FILE__, 0x1f4);
        int* riders = (int*)DebugCalloc(total, 4, __FILE__, 0x1f5);
        const char** names = (const char**)DebugMalloc(total * 4, __FILE__, 0x1f6);
        g_TrackGame->ui->UnknownFunction49b0d0(bikes, total, riders);
        g_TrackGame->ui->UnknownFunction49b020(names, total);
        for (i = 0; i < g_TrackGame->field_0x18 - 1; i++) {
            g_TrackGame->mode.field_0x1be4[i + 1].field_0xd4 = g_UnknownGlobal689d08[i].id;
            g_TrackGame->mode.field_0x1be4[i + 1].field_0xd8 = 0;
        }
        g_TrackGame->mode.field_0x1be4[0].field_0xd4 = g_TrackGame->network->localPlayer;
        g_TrackGame->mode.field_0x1be4[0].field_0xd8 = 0;
        int racer = g_UnknownGlobal689df8 + 1;
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < lobbySlots.slots[j].grantedRacers; k++) {
                g_TrackGame->mode.field_0x1be4[racer].field_0xd4 = lobbySlots.slots[j].playerId;
                g_TrackGame->mode.field_0x1be4[racer].field_0xd8 = racer;
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[racer].field_0xdc, names[racer], 0x10);
                g_TrackGame->mode.field_0x1be4[racer].field_0xc0 = 100 - (int)(RandomUnit() * -899.0f);
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[racer].field_0x00,
                          ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)
                              [((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bikes[racer]].model]
                                  .modelName,
                          0x40);
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[racer].field_0x40,
                          ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bikes[racer]].field_0x48, 0x40);
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[racer].field_0x80,
                          ((UnknownKrustyUIModel*)g_TrackGame->ui->riders)[riders[racer]].modelName, 0x40);
                g_TrackGame->mode.field_0x1be4[racer].field_0xec =
                    ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bikes[racer]].engineSize;
                g_TrackGame->mode.field_0x1be4[racer].field_0xf0 =
                    ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bikes[racer]].field_0x90;
                racer++;
            }
        }
        g_TrackGame->mode.UnknownFunction522cd0();
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2 && g_TrackGame->eventManager->racesRun > 0) {
            // By the championship points.
            for (i = 0; i < racer; i++) {
                UnknownGridEntry entry;
                entry.playerId = g_TrackGame->mode.field_0x1be4[i].field_0xd4;
                entry.points = g_TrackGame->eventManager->results[i].points;
                entry.racerIndex = g_TrackGame->mode.field_0x1be4[i].field_0xd8;
                ((UnknownGridEntry*)g_TrackGame->mode.field_0x27f8.field_0x14c)[i] = entry;
            }
            qsort(g_TrackGame->mode.field_0x27f8.field_0x14c, racer, sizeof(UnknownGridEntry),
                  CompareGridEntries);
        } else {
            // At random.
            UnknownGridDraw draws[8];
            for (i = 0; i < racer; i++) {
                UnknownGridDraw draw;
                draw.playerId = g_TrackGame->mode.field_0x1be4[i].field_0xd4;
                draw.key = RandomUnit() * 10.0f;
                draw.racerIndex = g_TrackGame->mode.field_0x1be4[i].field_0xd8;
                draws[i] = draw;
            }
            qsort(draws, racer, sizeof(UnknownGridDraw), CompareGridDraws);
            for (i = 0; i < racer; i++) {
                UnknownGridEntry entry;
                entry.playerId = draws[i].playerId;
                entry.racerIndex = draws[i].racerIndex;
                ((UnknownGridEntry*)g_TrackGame->mode.field_0x27f8.field_0x14c)[i] = entry;
            }
        }
        if (names)
            DebugFree(names, __FILE__, 0x23e);
    }
    UnknownLobbyStartMessage message;
    message.start = start;
    message.field_0x08 = g_TrackGame->mode.field_0x27f8;
    message.field_0x1f4 = g_TrackGame->mode.field_0x94;
    memcpy(message.field_0x08.field_0x18c, lobbySlots.slots, sizeof(lobbySlots.slots));
    if (start) {
        char name[0x80];
        message.field_0x1f8[0] = g_TrackGame->mode.field_0x1bcc;
        g_TrackGame->network->GetPlayerName(g_TrackGame->network->localPlayer, name);
        strcpy(message.field_0x218[0], name);
        int humans = g_UnknownGlobal689df8 + 1;
        for (i = 1; i < humans; i++) {
            message.field_0x1f8[i] = g_TrackGame->mode.field_0x1be4[i].field_0xc0;
            strcpy(message.field_0x218[i], g_TrackGame->mode.field_0x1be4[i].field_0xdc);
        }
        for (i = humans; i < total; i++) {
            message.field_0x1f8[i] = g_TrackGame->mode.field_0x1be4[i].field_0xc0;
            strcpy(message.field_0x218[i], g_TrackGame->mode.field_0x1be4[i].field_0xdc);
            strcpy(message.field_0x298[i], g_TrackGame->mode.field_0x1be4[i].field_0x00);
            strcpy(message.field_0x498[i], g_TrackGame->mode.field_0x1be4[i].field_0x40);
            strcpy(message.field_0x698[i], g_TrackGame->mode.field_0x1be4[i].field_0x80);
            message.field_0x898[i] = g_TrackGame->mode.field_0x1be4[i].field_0xec;
            message.field_0x8b8[i] = g_TrackGame->mode.field_0x1be4[i].field_0xf0;
        }
    }
    g_TrackGame->network->Send(0x83, &message, sizeof(message), g_TrackGame->network->localPlayer, 0);
    if (start)
        FillRacerSlots();
}
