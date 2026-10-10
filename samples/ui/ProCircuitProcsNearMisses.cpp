// Near misses for ProCircuitProcs.cpp (canonical file:
// src/reconstructed/ProCircuitProcs.cpp, included below for its types and
// its matched functions). Check: compile this file and compare each function
// with ProCircuitProcsNearMisses.bindings.json.
//
// PCLastRaceDlg::PayOutRace (0x004d8c40, 883 bytes): the same
//   flow, calls and x87 code; in the per-racer update retail loads +0x0c
//   before +0x08 for `field_0x30 += stunt - medical - repairs - fee +
//   winnings` and then adds +0x0c into +0x24 in memory, VC6 here loads +0x08
//   first. Reassociating or splitting the expression compiles the same.
// PCBailoutDlg::UnknownVirtualSlot29 (0x004d8fc0, 882 bytes, 750 match) and
// PCBunnyDlg::UnknownVirtualSlot29 (0x004d9860, 1120 bytes): retail loads the
//   message kind into ecx after moving `this` to esi and before loading
//   TrackGame+0x3444 (the reverse here), and picks ecx/edx the other way for
//   the TrackGame/module temporaries. Local order, scopes and a `kind` copy do
//   not move it, nor do per-case circuit locals, a `TrackGame* game` local,
//   a const/register circuit, an inline circuit getter or a circuit macro
//   (exact PCNewEventDlg slot 29 has the same source shape and retail's
//   order).
// PCBonusTrackDlg::UnknownVirtualSlot29 (0x004d9cd0, 752 bytes): the same
//   prologue difference, and retail copies the circuit pointer from ebp to
//   edi for the init case (VC6 here keeps one register; `current = circuit`
//   and using `circuit` directly compile the same).
// PCCentralBikeRiderDlg::UnknownVirtualSlot10 (0x004d72c0, 1291 bytes, 1233
//   match): as SPBikeRiderDlg slot 10 (DlgProcsNearMisses.cpp), only the
//   scheduling of the by-value Vector3 copies for the camera call differs.

#include "../../src/reconstructed/ProCircuitProcs.cpp"

// 0x004d8c40
void PCLastRaceDlg::PayOutRace()
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    int fee = circuit->field_0x1229[circuit->field_0x40];
    int purse = (int)(fee * circuit->field_0x1241);
    float* shares = (float*)DebugMalloc(circuit->field_0x460 * 4, __FILE__, 1420);
    float total = 0.0f;
    int i;
    for (i = 0; i < circuit->field_0x460; i++)
        total += circuit->field_0x1251[i];
    for (i = 0; i < circuit->field_0x460; i++)
        shares[i] = circuit->field_0x1251[i] / total;
    circuit->field_0x454 += fee;

    int paid = 0;
    for (int j = 0; j < g_TrackGame->eventManager->field_0x4c; j++) {
        UnknownProCircuitResult* result = &((UnknownProCircuitResult*)g_TrackGame->eventManager->field_0x50)[j];
        UnknownProCircuitRacer* racer = &circuit->field_0x465[result->racer];
        racer->field_0x00.field_0x0c = (int)__min(purse * circuit->field_0x1249, result->medical * 250.0f);
        racer->field_0x00.field_0x08 = (int)__min(purse * circuit->field_0x1245, result->repairs * 500.0f);
        racer->field_0x00.field_0x14 = (int)__min(purse * circuit->field_0x124d, result->stunts * 400.0f);
        racer->field_0x00.field_0x04 = (int)ceil(purse * shares[result->position - 1]);
        racer->field_0x00.field_0x00 = result->points;
        paid += racer->field_0x00.field_0x04;
        racer->field_0x30 += racer->field_0x00.field_0x14 - racer->field_0x00.field_0x0c - racer->field_0x00.field_0x08 - fee +
                             racer->field_0x00.field_0x04;
        racer->field_0x18.field_0x0c += racer->field_0x00.field_0x0c;
        racer->field_0x18.field_0x08 += racer->field_0x00.field_0x08;
        racer->field_0x18.field_0x14 += racer->field_0x00.field_0x14;
        racer->field_0x18.field_0x04 += racer->field_0x00.field_0x04;
        racer->field_0x18.field_0x00 += racer->field_0x00.field_0x00;
    }

    UnknownProCircuitRank order[11];
    for (i = 0; i < circuit->field_0x460; i++) {
        order[i].points = circuit->field_0x465[i].field_0x00.field_0x00;
        order[i].racer = i;
    }
    qsort(order, circuit->field_0x460, sizeof(UnknownProCircuitRank), CompareRankPoints);
    for (i = 0; i < circuit->field_0x460; i++)
        circuit->field_0x465[order[i].racer].field_0x00.field_0x10 = i + 1;
    int rest = purse - paid;
    circuit->field_0x465[order[0].racer].field_0x00.field_0x04 += rest;
    circuit->field_0x465[order[0].racer].field_0x18.field_0x04 += rest;
    circuit->field_0x465[order[0].racer].field_0x30 += rest;
    for (i = 0; i < circuit->field_0x460; i++) {
        order[i].points = circuit->field_0x465[i].field_0x18.field_0x00;
        order[i].racer = i;
    }
    qsort(order, circuit->field_0x460, sizeof(UnknownProCircuitRank), CompareRankPoints);
    for (i = 0; i < circuit->field_0x460; i++)
        circuit->field_0x465[order[i].racer].field_0x18.field_0x10 = i + 1;
}

// 0x004d8fc0
void PCBailoutDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    UIControl* label;
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        entryFee = circuit->field_0x1229[circuit->field_0x40];
        playerCash = circuit->field_0x465[0].field_0x30;
        cashShortfall = entryFee - playerCash;
        UnknownSetText(FindControl("TitleText", 12), 0x144a);
        UIControl* description = FindControl("TxtDescription", 12);
        description->field_0x1e8 = 1;
        UnknownSetText(description, 0x1449);
        unsigned int color = circuit->field_0x465[0].field_0x30 < 0 ? 0xff : 0xff00;
        UnknownSetText(FindControl("TxtItem1", 12), 0x144b);
        UIControl* item = FindControl("Item1", 12);
        sprintf(text, "$%d", playerCash);
        item->SetText(text);
        item->SetFontColor(color);
        UnknownSetText(FindControl("TxtItem2", 12), 0x144d);
        item = FindControl("Item2", 12);
        sprintf(text, "$%d", entryFee);
        item->SetText(text);
        item->SetFontColor(0xff);
        UnknownSetText(FindControl("TxtItem4", 12), 0x144c);
        item = FindControl("Item4", 12);
        sprintf(text, "$%d", cashShortfall);
        item->SetText(text);
        item->SetFontColor(0xff00);
        FindControl("Back", 1)->Show(0, 1);
        break;
    }
    case kDialogCommand:
        if (_stricmp("ButAccept", event->controlName) == 0) {
            circuit->field_0x465[0].field_0x30 += cashShortfall;
            circuit->field_0x465[0].field_0x34 += cashShortfall;
            g_TrackGame->field_0x3444->Save((char*)g_TrackGame->field_0x3448);
            PCCentralDlg* dialog = new(__FILE__, 1551) PCCentralDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        } else if (_stricmp("ButDecline", event->controlName) == 0) {
            circuit->field_0x464 |= 4;
            PCFailedDlg* dialog = new(__FILE__, 1556) PCFailedDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        }
        break;
    }
}

// 0x004d9860
void PCBunnyDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        int debt = circuit->field_0x465[0].field_0x34;
        field_0x7f58 = circuit->field_0x1229[circuit->field_0x40];
        field_0x7f5c = circuit->field_0x465[0].field_0x30;
        field_0x7f60 = field_0x7f58 - field_0x7f5c + debt;
        UnknownSetText(FindControl("TitleText", 12), 0x1452);
        UIControl* description = FindControl("TxtDescription", 12);
        description->field_0x1e8 = 1;
        UnknownSetText(description, 0x1451);
        unsigned int color = circuit->field_0x465[0].field_0x30 < 0 ? 0xff : 0xff00;
        UnknownSetText(FindControl("TxtItem1", 12), 0x144b);
        UIControl* item = FindControl("Item1", 12);
        sprintf(text, "$%d", field_0x7f5c);
        item->SetText(text);
        item->SetFontColor(color);
        UnknownSetText(FindControl("TxtItem2", 12), 0x144d);
        item = FindControl("Item2", 12);
        sprintf(text, "$%d", field_0x7f58);
        item->SetText(text);
        item->SetFontColor(0xff);
        UnknownSetText(FindControl("TxtItem3", 12), 0x1453);
        item = FindControl("Item3", 12);
        sprintf(text, "$%d", debt);
        item->SetText(text);
        item->SetFontColor(0xff);
        UnknownSetText(FindControl("TxtItem4", 12), 0x144f);
        item = FindControl("Item4", 12);
        sprintf(text, "$%d", field_0x7f60);
        item->SetText(text);
        item->SetFontColor(0xff00);
        UnknownSetText(FindControl("TxtItem5", 12), 0x1450);
        item = FindControl("Item5", 12);
        sprintf(text, "$%d", 1000);
        item->SetText(text);
        item->SetFontColor(0xff00);
        FindControl("Back", 1)->Show(0, 1);
        break;
    }
    case kDialogCommand:
        if (_stricmp("ButAccept", event->controlName) == 0) {
            circuit->field_0x465[0].field_0x30 += field_0x7f60 - circuit->field_0x465[0].field_0x34;
            circuit->field_0x465[0].field_0x34 = 0;
            g_TrackGame->field_0x3444->Save((char*)g_TrackGame->field_0x3448);
            PCCentralDlg* dialog = new(__FILE__, 1744) PCCentralDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        } else if (_stricmp("ButDecline", event->controlName) == 0) {
            circuit->field_0x464 |= 4;
            PCFailedDlg* dialog = new(__FILE__, 1749) PCFailedDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        }
        break;
    }
}

// 0x004d9cd0
void PCBonusTrackDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        UnknownTrackGameObject3444* current = g_TrackGame->field_0x3444;
        // TrackGame+0x18fc: an int per series (the bonus track was offered).
        ((int*)g_TrackGame->mode.field_0x1384)[current->field_0x40] = 1;
        int title;
        int description;
        switch (current->field_0x40) {
        case 2:
            title = 0x1457;
            description = 0x1459;
            break;
        case 3:
            title = 0x1458;
            description = 0x145a;
            break;
        }
        UnknownSetText(FindControl("TitleText", 12), title);
        UIControl* control = FindControl("TxtDescription", 12);
        control->field_0x1e8 = 1;
        UnknownSetText(control, description);
        UnknownSetText(FindControl("TxtItem1", 12), 0x145b);
        UIControl* item = FindControl("Item1", 12);
        sprintf(text, "$%d", 3000);
        item->SetText(text);
        item->SetFontColor(0xff00);
        FindControl("Back", 1)->Show(0, 1);
        current->field_0x465[0].field_0x30 += 3000;
        break;
    }
    case kDialogCommand:
        if (_stricmp("ButAccept", event->controlName) == 0) {
            g_TrackGame->eventManager->ResetEntries();
            g_TrackGame->mode.field_0x27f8.field_0x04 = circuit->field_0x40;
            g_TrackGame->mode.field_0x27f8.field_0x00 = 1;
            g_TrackGame->mode.field_0x27f8.field_0x20 = circuit->field_0x1285[circuit->field_0x40].field_0x0c;
            g_TrackGame->mode.field_0x27f8.field_0x24 = circuit->field_0x460 - 1;
            g_TrackGame->mode.field_0x27f8.field_0x10 = 1;
            g_TrackGame->mode.UnknownFunction5240e0(circuit->field_0x40);
            strcpy(text, circuit->field_0x1285[circuit->field_0x40].field_0x00[circuit->field_0x44 - 1].field_0x00);
            char* extension = strrchr(text, '.');
            if (extension)
                *extension = 0;
            strcpy(g_TrackGame->mode.field_0x27f8.field_0x36, text);
            if (circuit->field_0x40 == 1 || circuit->field_0x40 == 5)
                g_TrackGame->mode.field_0x27f8.field_0x34 = 1;
            else
                g_TrackGame->mode.field_0x27f8.field_0x34 = 0;
            g_TrackGame->mode.field_0x94 = circuit->field_0x50;
            UnknownFunction4536e0();
            UnknownVirtualSlot26();
        } else if (_stricmp("ButDecline", event->controlName) == 0) {
            circuit->AdvanceAfterRace();
            OpenCareerDialog();
            EndDialog(0);
        }
        break;
    }
}

// 0x004d72c0: as SPBikeRiderDlg slot 10 (dlgprocs.cpp 0x0044fae0).
int PCCentralBikeRiderDlg::UnknownVirtualSlot10(float frameTime)
{
    if (dialogBackground)
        dialogBackground->UnknownFunction404da0();
    Vector3 offset;
    Vector3 turned;
    GUIInputDevice* pointer = guiUser->pointerDevice;
    if (pointer && field_0x7f78 && field_0x7f7c) {
        float dx = (float)(pointer->pointerPosition.x - guiUser->field_0x34);
        field_0x7f58.y += (pointer->pointerPosition.y - guiUser->field_0x38) * 0.1f;
        field_0x7f58.y = __max(1.0f, __min(field_0x7f58.y, 10.0f));
        offset = UnknownVectorDifference(field_0x7f58, g_TrackGame->ui->field_0x474);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, dx * 0.015707964f);
        float distance = field_0x7f70;
        turned *= distance;
        field_0x7f58 = UnknownVectorSum(turned, g_TrackGame->ui->field_0x474);
        g_TrackGame->ui->field_0x468->UnknownFunction42e9b0(&field_0x7f58, 0, 0, 0, 0);
        g_TrackGame->ui->field_0x468->UnknownVirtualSlot29(field_0x7f64);
        field_0x7f78 = 1;
        static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->SetCurrentState(0);
    } else if (!field_0x7f78) {
        offset = UnknownVectorDifference(field_0x7f58, g_TrackGame->ui->field_0x474);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, frameTime * 0.39269909f);
        turned *= field_0x7f70;
        field_0x7f58 = UnknownVectorSum(turned, g_TrackGame->ui->field_0x474);
        g_TrackGame->ui->field_0x468->UnknownFunction42e9b0(&field_0x7f58, 0, 0, 0, 0);
        g_TrackGame->ui->field_0x468->UnknownVirtualSlot29(field_0x7f64);
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
        dialogBackground->UnknownFunction404240(field_0x7f74, (CameraRect*)&field_0x7f8c);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}
