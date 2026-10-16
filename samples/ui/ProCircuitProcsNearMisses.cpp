// Near misses for ProCircuitProcs.cpp (canonical file:
// src/reconstructed/ProCircuitProcs.cpp, included below for its types and
// its matched functions). Check: compile this file and compare each function
// with ProCircuitProcsNearMisses.bindings.json.
//
// PCLastRaceDlg::PayOutRace (0x004d8c40, 883 bytes): the same
//   flow, calls and x87 code; in the per-racer update retail loads +0x0c
//   before +0x08 for `field_0x30 += stunt - medical - repairs - fee +
//   winnings` and then adds +0x0c into +0x24 in memory, VC6 here loads +0x08
//   first. Reassociating or splitting the expression compiles the same
//   (all 120 orders of the five terms give one object); reordering the
//   points/paid/+0x30 statements, a winnings local, an AddStats inline
//   helper (pointer or reference), a by-value net helper or a stats pointer
//   are worse. The stats field types do not matter: unsigned int or long
//   for +0x04, +0x08, +0x0c or +0x14 alone, or for every field of
//   UnknownProCircuitStats (ProCircuit.h), all give 427. Retail keeps stunt
//   (ecx) and repairs (edx) and reloads medical for the +0x24 update; VC6
//   here keeps medical and repairs and reloads stunt. A `const int&` alias
//   of medical, repairs, stunt or winnings used in the money expression,
//   in its +0x18 accumulation or both, and a stats reference for the money
//   expression, give the same or more instruction diffs (12-25).
// PCBailoutDlg::HandleEvent (0x004d8fc0, 882 bytes, 836 match) and
// PCBunnyDlg::HandleEvent (0x004d9860, 1122 bytes, 1068 match): the
//   init case reads the fee and the cash into locals and stores the fee
//   member last; that gives retail's ecx/edx choices from the first
//   FindControl call on (750 and 225 with the fee member stored first, as
//   retail does: its fee goes to ecx with `mov edx, ecx; mov ecx, esi;
//   sub edx, eax`, and every fee-first form restores the old registers;
//   all fee/cash store orders, chained and embedded assignments were
//   swept). What remains: retail loads the
//   message kind into ecx after moving `this` to esi and before loading
//   TrackGame+0x3444 (the reverse here), and picks ecx/edx the other way for
//   the TrackGame/module temporaries. Local order, scopes and a `kind` copy do
//   not move it, nor do per-case circuit locals, a `TrackGame* game` local,
//   a const/register circuit, an inline circuit getter or a circuit macro
//   (exact PCNewEventDlg slot 29 has the same source shape and retail's
//   order). g_TrackGame->proCircuit read in every case, a leading `kind`
//   local and `self = this` are worse or the same, and no typedef count
//   (0..63) ahead of the TU makes 0x004d8c40, 0x004d8fc0, 0x004d9860 or
//   0x004d9cd0 exact, so it is not a symbol-arena tie-break.
// PCBonusTrackDlg::HandleEvent (0x004d9cd0, 752 bytes): the same
//   prologue difference, and retail copies the circuit pointer from ebp to
//   edi for the init case (VC6 here keeps one register; `current = circuit`
//   and using `circuit` directly compile the same). The init stores placed
//   before `current = g_TrackGame->proCircuit` match everything after them
//   (19 instruction diffs) but reload `current` (766 bytes); references,
//   casts, inline getters, helper parameters and declaration orders for
//   `current`/`circuit` all give 48.
// PCCentralBikeRiderDlg::Tick (0x004d72c0, 1291 bytes, 1233
//   match): as SPBikeRiderDlg slot 10 (DlgProcsNearMisses.cpp), only the
//   scheduling of the by-value Vector3 copies for the camera call differs.

#include "../../src/reconstructed/ProCircuitProcs.cpp"

// 0x004d8c40
void PCLastRaceDlg::PayOutRace()
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    int fee = circuit->entryFees[circuit->series];
    int purse = (int)(fee * circuit->purseMultiplier);
    float* shares = (float*)DebugMalloc(circuit->racerCount * 4, __FILE__, 1420);
    float total = 0.0f;
    int i;
    for (i = 0; i < circuit->racerCount; i++)
        total += circuit->placePct[i];
    for (i = 0; i < circuit->racerCount; i++)
        shares[i] = circuit->placePct[i] / total;
    circuit->feesPaid += fee;

    int paid = 0;
    for (int j = 0; j < g_TrackGame->eventManager->resultCount; j++) {
        UnknownProCircuitResult* result = &((UnknownProCircuitResult*)g_TrackGame->eventManager->results)[j];
        UnknownProCircuitRacer* racer = &circuit->racers[result->racer];
        racer->raceStats.medicalCost = (int)__min(purse * circuit->medicalCapPct, result->medical * 250.0f);
        racer->raceStats.repairCost = (int)__min(purse * circuit->repairCapPct, result->repairs * 500.0f);
        racer->raceStats.stuntWinnings = (int)__min(purse * circuit->stuntCapPct, result->stunts * 400.0f);
        racer->raceStats.purseWinnings = (int)ceil(purse * shares[result->position - 1]);
        racer->raceStats.points = result->points;
        paid += racer->raceStats.purseWinnings;
        racer->money += racer->raceStats.stuntWinnings - racer->raceStats.medicalCost - racer->raceStats.repairCost - fee +
                             racer->raceStats.purseWinnings;
        racer->totalStats.medicalCost += racer->raceStats.medicalCost;
        racer->totalStats.repairCost += racer->raceStats.repairCost;
        racer->totalStats.stuntWinnings += racer->raceStats.stuntWinnings;
        racer->totalStats.purseWinnings += racer->raceStats.purseWinnings;
        racer->totalStats.points += racer->raceStats.points;
    }

    UnknownProCircuitRank order[11];
    for (i = 0; i < circuit->racerCount; i++) {
        order[i].points = circuit->racers[i].raceStats.points;
        order[i].racer = i;
    }
    qsort(order, circuit->racerCount, sizeof(UnknownProCircuitRank), CompareRankPoints);
    for (i = 0; i < circuit->racerCount; i++)
        circuit->racers[order[i].racer].raceStats.rank = i + 1;
    int rest = purse - paid;
    circuit->racers[order[0].racer].raceStats.purseWinnings += rest;
    circuit->racers[order[0].racer].totalStats.purseWinnings += rest;
    circuit->racers[order[0].racer].money += rest;
    for (i = 0; i < circuit->racerCount; i++) {
        order[i].points = circuit->racers[i].totalStats.points;
        order[i].racer = i;
    }
    qsort(order, circuit->racerCount, sizeof(UnknownProCircuitRank), CompareRankPoints);
    for (i = 0; i < circuit->racerCount; i++)
        circuit->racers[order[i].racer].totalStats.rank = i + 1;
}

// 0x004d8fc0
void PCBailoutDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    UIControl* label;
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        int fee = circuit->entryFees[circuit->series];
        int cash = circuit->racers[0].money;
        playerCash = cash;
        cashShortfall = fee - cash;
        entryFee = fee;
        UnknownSetText(FindControl("TitleText", 12), 0x144a);
        UIControl* description = FindControl("TxtDescription", 12);
        description->wordWrap = 1;
        UnknownSetText(description, 0x1449);
        unsigned int color = circuit->racers[0].money < 0 ? 0xff : 0xff00;
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
            circuit->racers[0].money += cashShortfall;
            circuit->racers[0].debts += cashShortfall;
            g_TrackGame->proCircuit->Save(g_TrackGame->careerFile);
            PCCentralDlg* dialog = new(__FILE__, 1551) PCCentralDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        } else if (_stricmp("ButDecline", event->controlName) == 0) {
            circuit->careerFlags |= 4;
            PCFailedDlg* dialog = new(__FILE__, 1556) PCFailedDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        }
        break;
    }
}

// 0x004d9860
void PCBunnyDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        int debt = circuit->racers[0].debts;
        int fee = circuit->entryFees[circuit->series];
        int cash = circuit->racers[0].money;
        playerCash = cash;
        loanAmount = fee - cash + debt;
        entryFee = fee;
        UnknownSetText(FindControl("TitleText", 12), 0x1452);
        UIControl* description = FindControl("TxtDescription", 12);
        description->wordWrap = 1;
        UnknownSetText(description, 0x1451);
        unsigned int color = circuit->racers[0].money < 0 ? 0xff : 0xff00;
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
        UnknownSetText(FindControl("TxtItem3", 12), 0x1453);
        item = FindControl("Item3", 12);
        sprintf(text, "$%d", debt);
        item->SetText(text);
        item->SetFontColor(0xff);
        UnknownSetText(FindControl("TxtItem4", 12), 0x144f);
        item = FindControl("Item4", 12);
        sprintf(text, "$%d", loanAmount);
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
            circuit->racers[0].money += loanAmount - circuit->racers[0].debts;
            circuit->racers[0].debts = 0;
            g_TrackGame->proCircuit->Save(g_TrackGame->careerFile);
            PCCentralDlg* dialog = new(__FILE__, 1744) PCCentralDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        } else if (_stricmp("ButDecline", event->controlName) == 0) {
            circuit->careerFlags |= 4;
            PCFailedDlg* dialog = new(__FILE__, 1749) PCFailedDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        }
        break;
    }
}

// 0x004d9cd0
void PCBonusTrackDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        UnknownTrackGameObject3444* current = g_TrackGame->proCircuit;
        // TrackGame+0x18fc: an int per series (the bonus track was offered).
        ((int*)g_TrackGame->mode.bonusTrackUnlocked)[current->series] = 1;
        int title;
        int description;
        switch (current->series) {
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
        control->wordWrap = 1;
        UnknownSetText(control, description);
        UnknownSetText(FindControl("TxtItem1", 12), 0x145b);
        UIControl* item = FindControl("Item1", 12);
        sprintf(text, "$%d", 3000);
        item->SetText(text);
        item->SetFontColor(0xff00);
        FindControl("Back", 1)->Show(0, 1);
        current->racers[0].money += 3000;
        break;
    }
    case kDialogCommand:
        if (_stricmp("ButAccept", event->controlName) == 0) {
            g_TrackGame->eventManager->ResetEntries();
            g_TrackGame->mode.raceSettings.eventType = circuit->series;
            g_TrackGame->mode.raceSettings.raceMode = 1;
            g_TrackGame->mode.raceSettings.lapCount = circuit->schedules[circuit->series].laps;
            g_TrackGame->mode.raceSettings.aiRacerCount = circuit->racerCount - 1;
            g_TrackGame->mode.raceSettings.treeCollision = 1;
            g_TrackGame->mode.SelectSeries(circuit->series);
            strcpy(text, circuit->schedules[circuit->series].tracks[circuit->raceIndex - 1].trackKey);
            char* extension = strrchr(text, '.');
            if (extension)
                *extension = 0;
            strcpy(g_TrackGame->mode.raceSettings.trackName, text);
            if (circuit->series == 1 || circuit->series == 5)
                g_TrackGame->mode.raceSettings.trackNumber = 1;
            else
                g_TrackGame->mode.raceSettings.trackNumber = 0;
            g_TrackGame->mode.difficulty = circuit->difficulty;
            LeaveMenusForRace();
            MarkPendingRelease();
        } else if (_stricmp("ButDecline", event->controlName) == 0) {
            circuit->AdvanceAfterRace();
            OpenCareerDialog();
            EndDialog(0);
        }
        break;
    }
}

// 0x004d72c0: as SPBikeRiderDlg slot 10 (dlgprocs.cpp 0x0044fae0).
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
            g_TrackGame->ui->riderPreview->SetMotionByName("LookLeftR");
            break;
        case 1:
            g_TrackGame->ui->riderPreview->SetMotionByName("LookRightR");
            break;
        case 2:
            g_TrackGame->ui->riderPreview->SetMotionByName("StretchR");
            break;
        }
        idleMotionPlaying = 0;
    }
    if (!idleMotionPlaying && g_TrackGame->ui->riderPreview->motionFinished) {
        UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        KrustyUI* ui = g_TrackGame->ui;
        UnknownKrustyUIModel* model =
            &((UnknownKrustyUIModel*)ui->manufacturers)[((UnknownKrustyUIBike*)ui->bikes)[bikes->GetRowData(-1)].model];
        model->uiModel->SetMotionByName("WaitB");
        g_TrackGame->ui->riderPreview->SetMotionByName("WaitR");
        idleMotionPlaying = 1;
    }
    if (dialogBackground)
        dialogBackground->SetRegionRect(previewRegion, (CameraRect*)&previewArea);
    return UIDialog::Tick(frameTime);
}
