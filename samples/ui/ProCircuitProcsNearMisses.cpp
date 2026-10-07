// Near misses for ProCircuitProcs.cpp (canonical file:
// src/reconstructed/ProCircuitProcs.cpp, included below for its types and
// its matched functions). Check: compile this file and compare each function
// with ProCircuitProcsNearMisses.bindings.json.
//
// PCLastRaceDlg::UnknownFunction4d8c40 (0x004d8c40, 883 bytes): the same
//   flow, calls and x87 code; in the per-racer update retail loads +0x0c
//   before +0x08 for `field_0x30 += stunt - medical - repairs - fee +
//   winnings` and then adds +0x0c into +0x24 in memory, VC6 here loads +0x08
//   first. Reassociating or splitting the expression compiles the same.
// PCBailoutDlg::UnknownVirtualSlot29 (0x004d8fc0, 882 bytes, 750 match) and
// PCBunnyDlg::UnknownVirtualSlot29 (0x004d9860, 1120 bytes): retail loads the
//   message kind into ecx after moving `this` to esi and before loading
//   TrackGame+0x3444 (the reverse here), and picks ecx/edx the other way for
//   the TrackGame/module temporaries. Local order, scopes and a `kind` copy do
//   not move it.
// PCBonusTrackDlg::UnknownVirtualSlot29 (0x004d9cd0, 752 bytes): the same
//   prologue difference, and retail copies the circuit pointer from ebp to
//   edi for the init case (VC6 here keeps one register).
// PCNewEventDlg::UnknownVirtualSlot29 (0x004d9fd0, 456 bytes): byte-exact
//   once the /GX handler is bound (0x0054d257), but its prologue schedules
//   `mov edx, [esp+4]` between the fs:[0] load and `push -1`, a shape
//   mcm2tool/resolved_match.py's handler-push recognition does not accept, so
//   the handler label stays unresolved.
// PCStartupDlg::UnknownFunction4d59a0 (0x004d59a0, 756 bytes): exact in
//   some states of the shared headers; in others the only difference is the
//   order of the two reloads after the shared strcpy of the "Class" column
//   (retail reloads `this` before `row`). Unrelated declarations elsewhere in
//   the translation unit flip it.
// PCCentralBikeRiderDlg::UnknownFunction4d6fc0 (0x004d6fc0, 557 bytes, 555
//   match): retail reloads the bike row offset into ecx, VC6 here into ebp.
// PCCentralBikeRiderDlg::UnknownVirtualSlot10 (0x004d72c0, 1291 bytes, 1233
//   match): as SPBikeRiderDlg slot 10 (DlgProcsNearMisses.cpp), only the
//   scheduling of the by-value Vector3 copies for the camera call differs.

#include "../../src/reconstructed/ProCircuitProcs.cpp"

// 0x004d8c40
void PCLastRaceDlg::UnknownFunction4d8c40()
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
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
    for (int j = 0; j < g_UnknownGlobal56e26c->eventManager->field_0x4c; j++) {
        UnknownProCircuitResult* result = &((UnknownProCircuitResult*)g_UnknownGlobal56e26c->eventManager->field_0x50)[j];
        UnknownProCircuitRacer* racer = &circuit->field_0x465[result->field_0x00];
        racer->field_0x00.field_0x0c = (int)__min(purse * circuit->field_0x1249, result->field_0x38 * 250.0f);
        racer->field_0x00.field_0x08 = (int)__min(purse * circuit->field_0x1245, result->field_0x34 * 500.0f);
        racer->field_0x00.field_0x14 = (int)__min(purse * circuit->field_0x124d, result->field_0x3c * 400.0f);
        racer->field_0x00.field_0x04 = (int)ceil(purse * shares[result->field_0x04 - 1]);
        racer->field_0x00.field_0x00 = result->field_0x28;
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
        order[i].field_0x00 = circuit->field_0x465[i].field_0x00.field_0x00;
        order[i].field_0x04 = i;
    }
    qsort(order, circuit->field_0x460, sizeof(UnknownProCircuitRank), UnknownFunction4d8c20);
    for (i = 0; i < circuit->field_0x460; i++)
        circuit->field_0x465[order[i].field_0x04].field_0x00.field_0x10 = i + 1;
    int rest = purse - paid;
    circuit->field_0x465[order[0].field_0x04].field_0x00.field_0x04 += rest;
    circuit->field_0x465[order[0].field_0x04].field_0x18.field_0x04 += rest;
    circuit->field_0x465[order[0].field_0x04].field_0x30 += rest;
    for (i = 0; i < circuit->field_0x460; i++) {
        order[i].field_0x00 = circuit->field_0x465[i].field_0x18.field_0x00;
        order[i].field_0x04 = i;
    }
    qsort(order, circuit->field_0x460, sizeof(UnknownProCircuitRank), UnknownFunction4d8c20);
    for (i = 0; i < circuit->field_0x460; i++)
        circuit->field_0x465[order[i].field_0x04].field_0x18.field_0x10 = i + 1;
}

// 0x004d8fc0
void PCBailoutDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    UnknownGameUiControl* label;
    char text[128];
    switch (event->field_0x08) {
    case 5: {
        field_0x7f58 = circuit->field_0x1229[circuit->field_0x40];
        field_0x7f5c = circuit->field_0x465[0].field_0x30;
        field_0x7f60 = field_0x7f58 - field_0x7f5c;
        UnknownSetText(UnknownFunction46ebf0("TitleText", 12), 0x144a);
        UnknownGameUiControl* description = UnknownFunction46ebf0("TxtDescription", 12);
        description->field_0x1e8 = 1;
        UnknownSetText(description, 0x1449);
        unsigned int color = circuit->field_0x465[0].field_0x30 < 0 ? 0xff : 0xff00;
        UnknownSetText(UnknownFunction46ebf0("TxtItem1", 12), 0x144b);
        UnknownGameUiControl* item = UnknownFunction46ebf0("Item1", 12);
        sprintf(text, "$%d", field_0x7f5c);
        item->UnknownFunction470b20(text);
        item->UnknownFunction470d40(color);
        UnknownSetText(UnknownFunction46ebf0("TxtItem2", 12), 0x144d);
        item = UnknownFunction46ebf0("Item2", 12);
        sprintf(text, "$%d", field_0x7f58);
        item->UnknownFunction470b20(text);
        item->UnknownFunction470d40(0xff);
        UnknownSetText(UnknownFunction46ebf0("TxtItem4", 12), 0x144c);
        item = UnknownFunction46ebf0("Item4", 12);
        sprintf(text, "$%d", field_0x7f60);
        item->UnknownFunction470b20(text);
        item->UnknownFunction470d40(0xff00);
        UnknownFunction46ebf0("Back", 1)->UnknownFunction470660(0, 1);
        break;
    }
    case 1:
        if (_stricmp("ButAccept", event->field_0x04) == 0) {
            circuit->field_0x465[0].field_0x30 += field_0x7f60;
            circuit->field_0x465[0].field_0x34 += field_0x7f60;
            g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d4150((char*)g_UnknownGlobal56e26c->field_0x3448);
            PCCentralDlg* dialog = new(__FILE__, 1551) PCCentralDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            UnknownFunction46ff30(0);
        } else if (_stricmp("ButDecline", event->field_0x04) == 0) {
            circuit->field_0x464 |= 4;
            PCFailedDlg* dialog = new(__FILE__, 1556) PCFailedDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            UnknownFunction46ff30(0);
        }
        break;
    }
}

// 0x004d9860
void PCBunnyDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    char text[128];
    switch (event->field_0x08) {
    case 5: {
        int debt = circuit->field_0x465[0].field_0x34;
        field_0x7f58 = circuit->field_0x1229[circuit->field_0x40];
        field_0x7f5c = circuit->field_0x465[0].field_0x30;
        field_0x7f60 = field_0x7f58 - field_0x7f5c + debt;
        UnknownSetText(UnknownFunction46ebf0("TitleText", 12), 0x1452);
        UnknownGameUiControl* description = UnknownFunction46ebf0("TxtDescription", 12);
        description->field_0x1e8 = 1;
        UnknownSetText(description, 0x1451);
        unsigned int color = circuit->field_0x465[0].field_0x30 < 0 ? 0xff : 0xff00;
        UnknownSetText(UnknownFunction46ebf0("TxtItem1", 12), 0x144b);
        UnknownGameUiControl* item = UnknownFunction46ebf0("Item1", 12);
        sprintf(text, "$%d", field_0x7f5c);
        item->UnknownFunction470b20(text);
        item->UnknownFunction470d40(color);
        UnknownSetText(UnknownFunction46ebf0("TxtItem2", 12), 0x144d);
        item = UnknownFunction46ebf0("Item2", 12);
        sprintf(text, "$%d", field_0x7f58);
        item->UnknownFunction470b20(text);
        item->UnknownFunction470d40(0xff);
        UnknownSetText(UnknownFunction46ebf0("TxtItem3", 12), 0x1453);
        item = UnknownFunction46ebf0("Item3", 12);
        sprintf(text, "$%d", debt);
        item->UnknownFunction470b20(text);
        item->UnknownFunction470d40(0xff);
        UnknownSetText(UnknownFunction46ebf0("TxtItem4", 12), 0x144f);
        item = UnknownFunction46ebf0("Item4", 12);
        sprintf(text, "$%d", field_0x7f60);
        item->UnknownFunction470b20(text);
        item->UnknownFunction470d40(0xff00);
        UnknownSetText(UnknownFunction46ebf0("TxtItem5", 12), 0x1450);
        item = UnknownFunction46ebf0("Item5", 12);
        sprintf(text, "$%d", 1000);
        item->UnknownFunction470b20(text);
        item->UnknownFunction470d40(0xff00);
        UnknownFunction46ebf0("Back", 1)->UnknownFunction470660(0, 1);
        break;
    }
    case 1:
        if (_stricmp("ButAccept", event->field_0x04) == 0) {
            circuit->field_0x465[0].field_0x30 += field_0x7f60 - circuit->field_0x465[0].field_0x34;
            circuit->field_0x465[0].field_0x34 = 0;
            g_UnknownGlobal56e26c->field_0x3444->UnknownFunction4d4150((char*)g_UnknownGlobal56e26c->field_0x3448);
            PCCentralDlg* dialog = new(__FILE__, 1744) PCCentralDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            UnknownFunction46ff30(0);
        } else if (_stricmp("ButDecline", event->field_0x04) == 0) {
            circuit->field_0x464 |= 4;
            PCFailedDlg* dialog = new(__FILE__, 1749) PCFailedDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            UnknownFunction46ff30(0);
        }
        break;
    }
}

// 0x004d9cd0
void PCBonusTrackDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    char text[128];
    switch (event->field_0x08) {
    case 5: {
        UnknownTrackGameObject3444* current = g_UnknownGlobal56e26c->field_0x3444;
        // TrackGame+0x18fc: an int per series (the bonus track was offered).
        ((int*)g_UnknownGlobal56e26c->mode.field_0x1384)[current->field_0x40] = 1;
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
        UnknownSetText(UnknownFunction46ebf0("TitleText", 12), title);
        UnknownGameUiControl* control = UnknownFunction46ebf0("TxtDescription", 12);
        control->field_0x1e8 = 1;
        UnknownSetText(control, description);
        UnknownSetText(UnknownFunction46ebf0("TxtItem1", 12), 0x145b);
        UnknownGameUiControl* item = UnknownFunction46ebf0("Item1", 12);
        sprintf(text, "$%d", 3000);
        item->UnknownFunction470b20(text);
        item->UnknownFunction470d40(0xff00);
        UnknownFunction46ebf0("Back", 1)->UnknownFunction470660(0, 1);
        current->field_0x465[0].field_0x30 += 3000;
        break;
    }
    case 1:
        if (_stricmp("ButAccept", event->field_0x04) == 0) {
            g_UnknownGlobal56e26c->eventManager->UnknownFunction45e520();
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 = circuit->field_0x40;
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 = 1;
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20 = circuit->field_0x1285[circuit->field_0x40].field_0x0c;
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x24 = circuit->field_0x460 - 1;
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x10 = 1;
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(circuit->field_0x40);
            strcpy(text, circuit->field_0x1285[circuit->field_0x40].field_0x00[circuit->field_0x44 - 1].field_0x00);
            char* extension = strrchr(text, '.');
            if (extension)
                *extension = 0;
            strcpy(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, text);
            if (circuit->field_0x40 == 1 || circuit->field_0x40 == 5)
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34 = 1;
            else
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34 = 0;
            g_UnknownGlobal56e26c->mode.field_0x94 = circuit->field_0x50;
            UnknownFunction4536e0();
            UnknownVirtualSlot26();
        } else if (_stricmp("ButDecline", event->field_0x04) == 0) {
            circuit->UnknownFunction4d41a0();
            UnknownFunction4d4ba0();
            UnknownFunction46ff30(0);
        }
        break;
    }
}

// 0x004d9fd0
void PCNewEventDlg::UnknownVirtualSlot29(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
    switch (event->field_0x08) {
    case 5: {
        int description;
        const char* image;
        switch (circuit->field_0x40) {
        case 5:
            description = 0x1465;
            image = "WinScreen";
            break;
        case 1:
            description = 0x1454;
            image = "BajaScreen";
            break;
        case 2:
            description = 0x1455;
            image = "NatScreen";
            break;
        case 3:
            description = 0x1456;
            image = "SuperScreen";
            break;
        }
        UnknownGameUiControl* control = UnknownFunction46ebf0("TxtDescription", 12);
        control->field_0x1e8 = 1;
        UnknownSetText(control, description);
        UnknownSetText(UnknownFunction46ebf0("TitleText", 12), 0x1467);
        control = UnknownFunction46ebf0("ButDecline", 1);
        control->UnknownFunction470660(0, 1);
        UnknownSetText(UnknownFunction46ebf0("ButAccept", 1), 0x916);
        control = UnknownFunction46ebf0("Pic", 5);
        control->UnknownFunction470730(0, UnknownFunction46e9a0(image));
        break;
    }
    case 1: {
        PCCentralDlg* dialog = new(__FILE__, 1896) PCCentralDlg;
        field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
        UnknownFunction46ff30(0);
        break;
    }
    }
}

// 0x004d6fc0
void PCCentralBikeRiderDlg::UnknownFunction4d6fc0()
{
    int bike = 0;
    int rider = 0;
    char text[128];
    PCCentralBikeRiderDlg* self = this;
    UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
    list->UnknownFunction4775f0();
    for (int i = 0; i < g_UnknownGlobal56e26c->ui->field_0x54; i++) {
        int rule = g_UnknownGlobal56e26c->field_0x3444->field_0x4c;
        if (rule != 3 && !((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[i].field_0x88) {
            int bikeClass = UnknownBikeClassOf(((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[i].field_0x8c);
            if (bikeClass != 0) {
                if (bikeClass <= 0 || bikeClass > 2)
                    continue;
                if (rule != 2)
                    continue;
            } else if (rule != 1) {
                continue;
            }
        }
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[i];
        UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[entry->field_0x00];
        if (model->field_0xc4 > g_UnknownGlobal56e26c->field_0x3444->field_0x48)
            continue;
        sprintf(text, "%s %s", model->field_0x00, entry->field_0x04);
        list->UnknownFunction476d80(text, i, 0);
        if (g_UnknownGlobal56e26c->field_0x3444->field_0x45c == i)
            bike = i;
    }
    if (bike)
        list->UnknownFunction476b30(bike);
    else
        list->UnknownFunction476a60(0);
    list->UnknownFunction477900(1);
    list = static_cast<UIDropDownList*>(self->UnknownFunction46ebf0("DDLRiders", 6))->field_0x1fc;
    list->UnknownFunction4775f0();
    for (int j = 0; j < g_UnknownGlobal56e26c->ui->field_0x5c; j++) {
        UnknownKrustyUIModel* entry = &((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x58)[j];
        if (entry->field_0xc4 <= g_UnknownGlobal56e26c->field_0x3444->field_0x48 || entry->field_0xc0) {
            list->UnknownFunction476d80(entry->field_0x00, j, 0);
            if (g_UnknownGlobal56e26c->field_0x3444->field_0x458 == j)
                rider = j;
        }
    }
    if (rider)
        list->UnknownFunction476b30(rider);
    else
        list->UnknownFunction476a60(0);
}

// 0x004d59a0
void PCStartupDlg::UnknownFunction4d59a0(UnknownTrackGameObject3444* circuit, int row)
{
    char name[64];
    char text[128];
    int finished = (circuit->field_0x464 >> 3) & 1;
    int failed = (circuit->field_0x464 >> 2) & 1;
    const char* prefix = finished || failed ? "LstDone" : "Lst";

    sprintf(name, "%s%s", prefix, "Name");
    static_cast<UIListBox*>(UnknownFunction46ebf0(name, 3))->UnknownFunction476d80(circuit->field_0x00, row, 0);

    sprintf(name, "%s%s", prefix, "Rank");
    sprintf(text, "%d / %d", circuit->field_0x465[0].field_0x18.field_0x10, circuit->field_0x460);
    static_cast<UIListBox*>(UnknownFunction46ebf0(name, 3))->UnknownFunction476d80(text, row, 0);

    sprintf(name, "%s%s", prefix, "Class");
    switch (circuit->field_0x4c) {
    case 1:
        strcpy(text, "125");
        break;
    case 2:
        strcpy(text, "250");
        break;
    case 3:
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14cf, text, 127);
        break;
    }
    static_cast<UIListBox*>(UnknownFunction46ebf0(name, 3))->UnknownFunction476d80(text, row, 0);

    sprintf(name, "%s%s", prefix, "Diff");
    switch (circuit->field_0x50) {
    case 1:
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14cb, text, 127);
        break;
    case 3:
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14cd, text, 127);
        break;
    default:
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14cc, text, 127);
        break;
    }
    static_cast<UIListBox*>(UnknownFunction46ebf0(name, 3))->UnknownFunction476d80(text, row, 0);

    sprintf(name, "%s%s", prefix, "Points");
    _itoa(circuit->field_0x465[0].field_0x18.field_0x00, text, 10);
    static_cast<UIListBox*>(UnknownFunction46ebf0(name, 3))->UnknownFunction476d80(text, row, 0);

    sprintf(name, "%s%s", prefix, "Cash");
    sprintf(text, "$%d", circuit->field_0x465[0].field_0x30);
    static_cast<UIListBox*>(UnknownFunction46ebf0(name, 3))->UnknownFunction476d80(text, row, 0);

    if (finished || failed) {
        sprintf(name, "%s%s", prefix, "Status");
        if (failed)
            g_UnknownGlobal56e26c->UnknownFunction521970(0x1442, text, 128);
        else if (finished)
            g_UnknownGlobal56e26c->UnknownFunction521970(0x1443, text, 128);
        static_cast<UIListBox*>(UnknownFunction46ebf0(name, 3))->UnknownFunction476d80(text, row, 0);
    }
}

// 0x004d72c0: as SPBikeRiderDlg slot 10 (dlgprocs.cpp 0x0044fae0).
int PCCentralBikeRiderDlg::UnknownVirtualSlot10(float frameTime)
{
    if (field_0x110)
        field_0x110->UnknownFunction404da0();
    Vector3 offset;
    Vector3 turned;
    GUIInputDevice* pointer = field_0x34->field_0x2c;
    if (pointer && field_0x7f78 && field_0x7f7c) {
        float dx = (float)(pointer->field_0xa4.x - field_0x34->field_0x34);
        field_0x7f58.y += (pointer->field_0xa4.y - field_0x34->field_0x38) * 0.1f;
        field_0x7f58.y = __max(1.0f, __min(field_0x7f58.y, 10.0f));
        offset = UnknownVectorDifference(field_0x7f58, g_UnknownGlobal56e26c->ui->field_0x474);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, dx * 0.015707964f);
        float distance = field_0x7f70;
        turned *= distance;
        field_0x7f58 = UnknownVectorSum(turned, g_UnknownGlobal56e26c->ui->field_0x474);
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownFunction42e9b0(&field_0x7f58, 0, 0, 0, 0);
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownVirtualSlot29(field_0x7f64);
        field_0x7f78 = 1;
        static_cast<UIMultiState*>(UnknownFunction46ebf0("ChkAutoRotate", 2))->UnknownFunction478cf0(0);
    } else if (!field_0x7f78) {
        offset = UnknownVectorDifference(field_0x7f58, g_UnknownGlobal56e26c->ui->field_0x474);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, frameTime * 0.39269909f);
        turned *= field_0x7f70;
        field_0x7f58 = UnknownVectorSum(turned, g_UnknownGlobal56e26c->ui->field_0x474);
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownFunction42e9b0(&field_0x7f58, 0, 0, 0, 0);
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownVirtualSlot29(field_0x7f64);
    }
    g_UnknownGlobal56e26c->ui->field_0x46c->ModelVirtualSlot7(frameTime, 0, 0);
    g_UnknownGlobal56e26c->ui->field_0x470->ModelVirtualSlot7(frameTime, 0, 0);
    if (field_0x7f80 && rand() % 200 == 1) {
        UIListBox* bikes = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
        bikes->UnknownFunction4768d0(-1);
        switch (rand() % 3) {
        case 0:
            g_UnknownGlobal56e26c->ui->field_0x46c->UnknownFunction4a8b10("LookLeftR");
            break;
        case 1:
            g_UnknownGlobal56e26c->ui->field_0x46c->UnknownFunction4a8b10("LookRightR");
            break;
        case 2:
            g_UnknownGlobal56e26c->ui->field_0x46c->UnknownFunction4a8b10("StretchR");
            break;
        }
        field_0x7f80 = 0;
    }
    if (!field_0x7f80 && g_UnknownGlobal56e26c->ui->field_0x46c->field_0x0c) {
        UIListBox* bikes = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
        KrustyUI* ui = g_UnknownGlobal56e26c->ui;
        UnknownKrustyUIModel* model =
            &((UnknownKrustyUIModel*)ui->field_0x48)[((UnknownKrustyUIBike*)ui->field_0x50)[bikes->UnknownFunction4768d0(-1)].field_0x00];
        model->field_0xc0->UnknownFunction4a8b10("WaitB");
        g_UnknownGlobal56e26c->ui->field_0x46c->UnknownFunction4a8b10("WaitR");
        field_0x7f80 = 1;
    }
    if (field_0x110)
        field_0x110->UnknownFunction404240(field_0x7f74, (CameraRect*)&field_0x7f8c);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}
