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
//   reproduce the layout.
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
// MultiPlayerDlg::UnknownVirtualSlot24 (0x004f3a70, 2729 bytes, 2714
//   match): only the scheduling of the arrival-delay conversion differs
//   (retail stores the sent time before the fild, VC6 here after loading
//   TrackGame). Unchanged by unsigned/int locals for either operand (an
//   int local for the sent time drops the qword temp and is worse), float
//   locals in either order (worse), unsigned locals in either order, a
//   double delay (worse), an inline elapsed-time helper, `-sent + now`,
//   `delay -=`, the comparison direction, a pointer to the minimum or to
//   the racer slot taken after the delay (worse), no local, and the /G, /O,
//   /Zp flag sweep. Not sensitive to the declaration count (0..63
//   prepended typedefs).

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
    for (int i = 0; i < g_TrackGame->ui->field_0x54; i++) {
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[i];
        if (!entry->field_0x88) {
            int kind = ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[entry->model].modelKind;
            if (kind != 2 && kind != 3 && (kind != 10 || !(g_TrackGame->mode.field_0x1970 & 1)))
                continue;
        }
        int flags = g_TrackGame->mode.field_0x1bd4;
        if (!flags) {
            sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[entry->model].displayName,
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
            sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[entry->model].displayName,
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
    for (int j = 0; j < g_TrackGame->ui->field_0x5c; j++) {
        UnknownKrustyUIModel* entry = &((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[j];
        if (entry->modelKind == 2 || entry->modelKind == 3 || entry->field_0xc0) {
            list->AddRow(entry->displayName, j, 0);
            if (!strcmp(((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[j].modelName,
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
    if (pointer && field_0x7f78 && field_0x7f7c) {
        float dx = (float)(pointer->pointerPosition.x - guiUser->field_0x34);
        viewEye.y += (pointer->pointerPosition.y - guiUser->field_0x38) * 0.1f;
        viewEye.y = __max(1.0f, __min(viewEye.y, 10.0f));
        offset = UnknownVectorDifference(viewEye, g_TrackGame->ui->field_0x474);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, dx * 0.015707964f);
        float distance = viewDistance;
        turned *= distance;
        viewEye = UnknownVectorSum(turned, g_TrackGame->ui->field_0x474);
        g_TrackGame->ui->field_0x468->UnknownFunction42e9b0(&viewEye, 0, 0, 0, 0);
        g_TrackGame->ui->field_0x468->UnknownVirtualSlot29(viewTarget);
        field_0x7f78 = 1;
    } else if (!field_0x7f78) {
        offset = UnknownVectorDifference(viewEye, g_TrackGame->ui->field_0x474);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, frameTime * 0.39269909f);
        turned *= viewDistance;
        viewEye = UnknownVectorSum(turned, g_TrackGame->ui->field_0x474);
        g_TrackGame->ui->field_0x468->UnknownFunction42e9b0(&viewEye, 0, 0, 0, 0);
        g_TrackGame->ui->field_0x468->UnknownVirtualSlot29(viewTarget);
    }
    g_TrackGame->ui->field_0x46c->ModelVirtualSlot7(frameTime, 0, 0);
    g_TrackGame->ui->field_0x470->ModelVirtualSlot7(frameTime, 0, 0);
    if (field_0x7f80 && rand() % 200 == 1) {
        UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        bikes->GetRowData(-1);
        switch (rand() % 3) {
        case 0:
            g_TrackGame->ui->field_0x46c->UnknownFunction4a8b10("LookLeftR");
            break;
        case 1:
            g_TrackGame->ui->field_0x46c->UnknownFunction4a8b10("LookRightR");
            break;
        case 2:
            g_TrackGame->ui->field_0x46c->UnknownFunction4a8b10("StretchR");
            break;
        }
        field_0x7f80 = 0;
    }
    if (!field_0x7f80 && g_TrackGame->ui->field_0x46c->motionFinished) {
        UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        UnknownKrustyUIModel* model =
            &((UnknownKrustyUIModel*)ui->field_0x48)[((UnknownKrustyUIBike*)ui->field_0x50)[bikes->GetRowData(-1)].model];
        model->field_0xc0->UnknownFunction4a8b10("WaitB");
        g_TrackGame->ui->field_0x46c->UnknownFunction4a8b10("WaitR");
        field_0x7f80 = 1;
    }
    if (dialogBackground)
        dialogBackground->UnknownFunction404240(previewRegion, (CameraRect*)&previewArea);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

#include "../../src/reconstructed/DlgProcs.h" // MainDlg, built inline by slot 24 (0x004f43c1)

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
        field_0x7f68.UnknownFunction4f2fe0(g_UnknownGlobal689df8 + 1, g_TrackGame->mode.field_0x27f8.field_0x28);
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
            for (int k = 0; k < field_0x7f68.field_0x00[j].grantedRacers; k++) {
                g_TrackGame->mode.field_0x1be4[racer].field_0xd4 = field_0x7f68.field_0x00[j].playerId;
                g_TrackGame->mode.field_0x1be4[racer].field_0xd8 = racer;
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[racer].field_0xdc, names[racer], 0x10);
                g_TrackGame->mode.field_0x1be4[racer].field_0xc0 = 100 - (int)(RandomUnit() * -899.0f);
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[racer].field_0x00,
                          ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)
                              [((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[bikes[racer]].model]
                                  .modelName,
                          0x40);
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[racer].field_0x40,
                          ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[bikes[racer]].field_0x48, 0x40);
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[racer].field_0x80,
                          ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[riders[racer]].modelName, 0x40);
                g_TrackGame->mode.field_0x1be4[racer].field_0xec =
                    ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[bikes[racer]].engineSize;
                g_TrackGame->mode.field_0x1be4[racer].field_0xf0 =
                    ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[bikes[racer]].field_0x90;
                racer++;
            }
        }
        g_TrackGame->mode.UnknownFunction522cd0();
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2 && g_TrackGame->eventManager->field_0x48 > 0) {
            // By the championship points.
            for (i = 0; i < racer; i++) {
                UnknownGridEntry entry;
                entry.field_0x00 = g_TrackGame->mode.field_0x1be4[i].field_0xd4;
                entry.field_0x04 = g_TrackGame->eventManager->field_0x50[i].field_0x28;
                entry.field_0x05 = g_TrackGame->mode.field_0x1be4[i].field_0xd8;
                ((UnknownGridEntry*)g_TrackGame->mode.field_0x27f8.field_0x14c)[i] = entry;
            }
            qsort(g_TrackGame->mode.field_0x27f8.field_0x14c, racer, sizeof(UnknownGridEntry),
                  UnknownFunction4f2080);
        } else {
            // At random.
            UnknownGridDraw draws[8];
            for (i = 0; i < racer; i++) {
                UnknownGridDraw draw;
                draw.field_0x00 = g_TrackGame->mode.field_0x1be4[i].field_0xd4;
                draw.field_0x04 = RandomUnit() * 10.0f;
                draw.field_0x08 = g_TrackGame->mode.field_0x1be4[i].field_0xd8;
                draws[i] = draw;
            }
            qsort(draws, racer, sizeof(UnknownGridDraw), UnknownFunction4f20a0);
            for (i = 0; i < racer; i++) {
                UnknownGridEntry entry;
                entry.field_0x00 = draws[i].field_0x00;
                entry.field_0x05 = draws[i].field_0x08;
                ((UnknownGridEntry*)g_TrackGame->mode.field_0x27f8.field_0x14c)[i] = entry;
            }
        }
        if (names)
            DebugFree(names, __FILE__, 0x23e);
    }
    UnknownLobbyStartMessage message;
    message.field_0x04 = start;
    message.field_0x08 = g_TrackGame->mode.field_0x27f8;
    message.field_0x1f4 = g_TrackGame->mode.field_0x94;
    memcpy(message.field_0x08.field_0x18c, field_0x7f68.field_0x00, sizeof(field_0x7f68.field_0x00));
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

// 0x004f3a70: the lobby's network messages.
int MultiPlayerDlg::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    int i;
    if (from && type == 2) {
        UnknownPlayerStateMessage* state = (UnknownPlayerStateMessage*)data;
        for (i = 0; i < 7; i++) {
            if (g_UnknownGlobal689d08[i].id != from)
                continue;
            char name[0x80];
            sprintf(name, "ButRdyPlayer%d", i + 2);
            UIControl* button = FindControl(name, 1);
            if (button) {
                if (state->field_0x01) {
                    button->SetFontColor(0xffffff);
                    g_UnknownGlobal689dbc[i] = 1;
                    field_0x7f68.SetSlot(from, state->field_0xc3);
                    if (!g_UnknownGlobal689dd8[i]) {
                        char text[0x80];
                        char line[0x80];
                        g_TrackGame->LoadResourceString(0x14d5, text, 0x80);
                        sprintf(line, text, g_UnknownGlobal689d08[i].name, state->field_0xc8);
                        AddChatLine(-1, line);
                        g_UnknownGlobal689dd8[i] = 1;
                    }
                } else {
                    button->SetFontColor(0x808080);
                    g_UnknownGlobal689dbc[i] = 0;
                    g_UnknownGlobal689dd8[i] = 0;
                    field_0x7f68.FreeSlot(from);
                }
            }
            g_TrackGame->mode.field_0x1be4[i + 1].field_0xd4 = g_UnknownGlobal689d08[i].id;
            g_TrackGame->mode.field_0x1be4[i + 1].field_0xd8 = 0;
            COPY_TEXT(g_TrackGame->mode.field_0x1be4[i + 1].field_0xdc, g_UnknownGlobal689d08[i].name, 0x10);
            g_TrackGame->mode.field_0x1be4[i + 1].UnknownFunction521fb0(state->field_0x42, state->field_0x02,
                                                                                 state->field_0x82);
            g_TrackGame->mode.field_0x1be4[i + 1].field_0xec = state->field_0xc8;
            g_TrackGame->mode.field_0x1be4[i + 1].field_0xf0 = state->field_0xcc;
            g_TrackGame->mode.field_0x1be4[i + 1].field_0xf4 = state->field_0xcd;
            g_TrackGame->mode.field_0x1be4[i + 1].field_0xc0 = state->field_0xd0;
            g_TrackGame->mode.field_0x1be4[i + 1].field_0xc4 = state->field_0xc2;
            float delay = (float)(unsigned int)flags - (float)(unsigned int)state->field_0xc4;
            if (delay < g_TrackGame->mode.field_0x1be4[i + 1].field_0xd0)
                g_TrackGame->mode.field_0x1be4[i + 1].field_0xd0 = delay;
            field_0x7f68.SetSlot(from, state->field_0xc3);
            if (g_TrackGame->network->isHost && field_0x7f58) {
                UIListBox* list = static_cast<UIListBox*>(field_0x7f58->FindControl("OpponentsListBox", 0));
                if (list && list->rowCount - 1 != g_TrackGame->mode.field_0x1bd8) {
                    int selection = list->GetSelectedRow();
                    list->RemoveAllRows();
                    for (int j = 0; j <= g_TrackGame->mode.field_0x1bd8; j++) {
                        char number[0x80];
                        sprintf(number, "%d", j);
                        list->AddRow(number, j, 0);
                    }
                    if (g_TrackGame->mode.field_0x1bd8 < selection)
                        list->SelectRow(g_TrackGame->mode.field_0x1bd8);
                    else
                        list->SelectRow(selection);
                }
            }
        }
    } else if (from && type == 0x83) {
        UnknownLobbyStartMessage* start = (UnknownLobbyStartMessage*)data;
        g_TrackGame->mode.field_0x27f8 = start->field_0x08;
        g_TrackGame->mode.field_0x94 = start->field_0x1f4;
        int total = g_TrackGame->mode.field_0x27f8.field_0x35 + g_TrackGame->mode.field_0x27f8.field_0x28;
        if (start->field_0x04) {
            for (i = g_TrackGame->mode.field_0x27f8.field_0x35; i < total; i++) {
                g_TrackGame->mode.field_0x1be4[i].field_0xc0 = start->field_0x1f8[i];
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[i].field_0xdc, start->field_0x218[i], 0x10);
                g_TrackGame->mode.field_0x1be4[i].field_0xd8 = i;
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[i].field_0x00, start->field_0x298[i], 0x40);
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[i].field_0x40, start->field_0x498[i], 0x40);
                COPY_TEXT(g_TrackGame->mode.field_0x1be4[i].field_0x80, start->field_0x698[i], 0x40);
                g_TrackGame->mode.field_0x1be4[i].field_0xec = start->field_0x898[i];
                g_TrackGame->mode.field_0x1be4[i].field_0xf0 = start->field_0x8b8[i];
            }
            memcpy(field_0x7f68.field_0x00, start->field_0x08.field_0x18c, sizeof(field_0x7f68.field_0x00));
            FillRacerSlots();
        } else {
            FollowTrackChange();
        }
    } else if (from && type == 0xf) {
        UnknownLobbySettingsMessage* settings = (UnknownLobbySettingsMessage*)data;
        g_TrackGame->mode.field_0x27f8.field_0x140 = settings->minutes;
        g_TrackGame->mode.field_0x27f8.field_0x04 = settings->eventType;
        g_TrackGame->mode.field_0x27f8.field_0x00 = settings->raceMode;
        g_TrackGame->mode.field_0x27f8.field_0x08 = settings->field_0x2d78;
        g_TrackGame->mode.field_0x27f8.field_0x20 = settings->laps;
        g_TrackGame->mode.field_0x27f8.field_0x28 = settings->opponents;
        g_TrackGame->mode.field_0x27f8.field_0x35 = settings->players;
        g_TrackGame->mode.field_0x27f8.field_0x0c = settings->races;
        g_TrackGame->mode.field_0x27f8.field_0x144 = settings->tagBall;
        g_TrackGame->mode.field_0x27f8.field_0x148 = settings->stuntMode;
        g_TrackGame->mode.field_0x27f8.field_0x10 = settings->treeCollision;
        g_TrackGame->mode.field_0x27f8.field_0x14 = settings->riderCollision;
        g_TrackGame->mode.field_0x27f8.field_0x18 = settings->fastFinishes;
        g_TrackGame->mode.field_0x27f8.field_0x1c = settings->bikeClass;
        g_TrackGame->mode.field_0x27f8.field_0x34 = settings->trackNumber;
        g_TrackGame->mode.field_0x27f8.field_0x138 = settings->field_0x0c;
        g_TrackGame->mode.field_0x27f8.field_0x13c = settings->field_0x10;
        g_TrackGame->mode.field_0x94 = settings->detail;
        g_TrackGame->mode.UnknownFunction5240e0(g_TrackGame->mode.field_0x27f8.field_0x08);
        g_TrackGame->mode.field_0x27f8.field_0x36[0] = 0;
        strncat(g_TrackGame->mode.field_0x27f8.field_0x36, settings->trackName, 0x3f);
        memcpy(field_0x7f68.field_0x00, settings->slots, sizeof(field_0x7f68.field_0x00));
        FollowTrackChange();
    } else if (from && type == 0x85) {
        AddChatLine(from, (const char*)data + 4);
        return 1;
    } else if (from && type == 0x92) {
        AddChatLine(-1, (const char*)data + 4);
        return 1;
    } else if (type == 5) {
        field_0x7f68.FreeSlot(((UnknownPlayerLeftMessage*)data)->field_0x08);
        if (field_0x7f58) {
            UIListBox* list = static_cast<UIListBox*>(field_0x7f58->FindControl("OpponentsListBox", 0));
            if (g_TrackGame->network->isHost && list &&
                list->rowCount - 1 != g_TrackGame->mode.field_0x1bd8) {
                int selection = list->GetSelectedRow();
                list->RemoveAllRows();
                for (int j = 0; j <= g_TrackGame->mode.field_0x1bd8; j++) {
                    char number[0x80];
                    sprintf(number, "%d", j);
                    list->AddRow(number, j, 0);
                }
                if (g_TrackGame->mode.field_0x1bd8 < selection)
                    list->SelectRow(g_TrackGame->mode.field_0x1bd8);
                else
                    list->SelectRow(selection);
            }
        }
    } else if (from && type == 0x8e) {
        UnknownKickMessage* kick = (UnknownKickMessage*)data;
        int player = g_TrackGame->network->localPlayer;
        if (kick->field_0x04 == player) {
            if (g_TrackGame->network->lobbyConnected) {
                EndDialog(0);
                g_TrackGame->ui->OpenExitDialog();
                return 1;
            }
            EndDialog(0);
            g_TrackGame->ui->field_0x2c->ShowDialog(new(__FILE__, 0x537) MainDlg, 0x64, 2, 0, 0, 0x63,
                                                                         0, 1);
            return 1;
        }
        field_0x7f68.FreeSlot(kick->field_0x04);
        if (field_0x7f58) {
            UIListBox* list = static_cast<UIListBox*>(field_0x7f58->FindControl("OpponentsListBox", 0));
            if (g_TrackGame->network->isHost && list &&
                list->rowCount - 1 != g_TrackGame->mode.field_0x1bd8) {
                int selection = list->GetSelectedRow();
                list->RemoveAllRows();
                for (int j = 0; j <= g_TrackGame->mode.field_0x1bd8; j++) {
                    char number[0x80];
                    sprintf(number, "%d", j);
                    list->AddRow(number, j, 0);
                }
                if (g_TrackGame->mode.field_0x1bd8 < selection)
                    list->SelectRow(g_TrackGame->mode.field_0x1bd8);
                else
                    list->SelectRow(selection);
            }
        }
    }
    return GameObject::UnknownVirtualSlot24(type, data, from, to, flags);
}
