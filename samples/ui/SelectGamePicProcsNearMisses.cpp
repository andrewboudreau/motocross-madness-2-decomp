// Near misses for SelectGamePicProcs.cpp (canonical file:
// src/reconstructed/SelectGamePicProcs.cpp, included below for its types and
// its matched functions). Check: compile this file and compare each function
// with SelectGamePicProcsNearMisses.bindings.json.
//
// MultiPlayerDlg::UnknownFunction4f3720 (0x004f3720, 361 bytes): the same
//   calls and strings; retail lays the system-message branch and the player
//   colour loop out after the "~1" branch and merges the "%s: %s" sprintf
//   into the first branch's call, VC6 here keeps source order. Inverting
//   the tests, swapping the if/else arms and moving the IME test do not
//   reproduce the layout.
// MPBikeRiderDlg::UnknownFunction4f8220 (0x004f8220, 833 bytes): the same
//   flow and calls; retail keeps the bike index in edi and KrustyUI in ebp
//   and lays the locals out differently (VC6 here swaps the registers).
//   Dropping the `this` copy, hoisting the index and caching KrustyUI do
//   not move it.
// MPBikeRiderDlg::UnknownVirtualSlot29 (0x004f78a0, 2419 bytes, 2415
//   match): the only difference is the x87 order of the bike view's
//   distance: retail squares x first (fld st(2)/fmul st(3)), VC6 here z
//   first. Operand order, a dot-product helper, a by-value vector, a named
//   temporary and a separate root helper all compile the same.
// UnknownFunction4f17a0 (0x004f17a0, 2258 bytes, 2244 match): the same
//   flow and calls; retail swaps the frame slots of the selected and the
//   fallback entry buffers (VC6 orders the frame by reference counts).
// MPBikeRiderDlg::UnknownVirtualSlot10 (0x004f8820, 1268 bytes, 1214
//   match): as SPBikeRiderDlg slot 10 (DlgProcsNearMisses.cpp), only the
//   scheduling of the by-value Vector3 copies for the camera call differs.
// MultiPlayerDlg::UnknownFunction4f2340 (0x004f2340, 2120 bytes): the
//   same flow, calls, frame size and message layout; register allocation
//   differs: retail keeps the racer total in memory (+0x1c) and the bike,
//   rider and name arrays in edi/ebx/ebp, VC6 here enregisters the total.
// MultiPlayerDlg::UnknownVirtualSlot24 (0x004f3a70, 2729 bytes, 2714
//   match): only the scheduling of the arrival-delay conversion differs
//   (retail stores the sent time before the fild, VC6 here after loading
//   TrackGame).

#include "../../src/reconstructed/SelectGamePicProcs.cpp"

// 0x004f3720
void MultiPlayerDlg::UnknownFunction4f3720(int player, const char* text) {
    char line[0x60];
    char name[0x80];
    char system[0x80];
    UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("ListChat", 3));
    if (!list)
        return;
    if (player != -1) {
        if (!g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac720(player, name))
            return;
        sprintf(line, "%s: %s", name, text);
    } else {
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14ba, system, 0x80);
        if (!g_UnknownGlobal56e26c->field_0x538)
            sprintf(line, "~5%s: %s", system, text);
        else
            sprintf(line, "%s: %s", system, text);
    }
    if (!g_UnknownGlobal56e26c->field_0x538) {
        if (g_UnknownGlobal56e26c->field_0x08->field_0x0c == player) {
            sprintf(line, "~1%s: ~0%s", name, text);
        } else {
            for (int i = 0; i < 7; i++) {
                if (g_UnknownGlobal689d08[i].field_0x04 == player)
                    sprintf(line, "~%1d%s: ~0%s", i + 2, name, text);
            }
        }
    }
    int rows = list->field_0x1ec;
    if ((short)rows >= 100)
        list->UnknownFunction477490(0);
    list->UnknownFunction476d80(line, 0, 0);
    list->UnknownFunction476860((short)rows, 1);
}

// 0x004f78a0 (with its inline vector helpers)
// Inline vector helpers (the bike view): a difference and a length that
// skips the square root for unit vectors.
static inline Vector3 UnknownInlineDifference(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline float UnknownInlineDot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float UnknownInlineLength(const Vector3& v) {
    float lengthSquared = UnknownInlineDot(v, v);
    if (lengthSquared == 1.0f)
        return 1.0f;
    return FastSqrt(lengthSquared);
}

void MPBikeRiderDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char plate[12];
    char typed[12];
    char text[0x80];
    char format[0x80];
    switch (event->field_0x08) {
    case 5: {
        field_0x7f78 = 0;
        g_UnknownGlobal56e26c->mode.field_0x9c = 0;
        field_0x7f88 = 1;
        field_0x7f84 = 1;
        UnknownFunction4f8220();
        UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLEngineSize", 6))->field_0x1fc;
        list->UnknownFunction4775f0();
        list->UnknownFunction476d80("125cc 2-stroke", 0, 0);
        list->UnknownFunction476d80("250cc 2-stroke", UnknownBikeClassOf(g_UnknownGlobal56cb6c[1]), 0);
        list->UnknownFunction476d80("400cc 4-stroke", UnknownBikeClassOf(g_UnknownGlobal56cb6c[2]), 0);
        list->UnknownFunction476d80("500cc 2-stroke", UnknownBikeClassOf(g_UnknownGlobal56cb6c[3]), 0);
        list->UnknownFunction476d80("600cc 4-stroke", UnknownBikeClassOf(g_UnknownGlobal56cb6c[4]), 0);
        list->UnknownFunction476b30(
            UnknownBikeClassOf(((UnknownOptGarageSettings*)g_UnknownGlobal56e26c->mode.field_0xfd8)->field_0x00));
        g_UnknownGlobal56e26c->UnknownFunction521970(0x146a, format, 0x80);
        list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("LargestOpponentDropDown", 6))->field_0x1fc;
        list->UnknownFunction4775f0();
        sprintf(text, format, g_UnknownGlobal56cb6c[0]);
        list->UnknownFunction476d80(text, 0, 0);
        sprintf(text, format, g_UnknownGlobal56cb6c[1]);
        list->UnknownFunction476d80(text, 1, 0);
        sprintf(text, format, g_UnknownGlobal56cb6c[2]);
        list->UnknownFunction476d80(text, 2, 0);
        sprintf(text, format, g_UnknownGlobal56cb6c[3]);
        list->UnknownFunction476d80(text, 3, 0);
        sprintf(text, format, g_UnknownGlobal56cb6c[4]);
        list->UnknownFunction476d80(text, 4, 0);
        list->UnknownFunction476a60(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x1c);
        UIEditBox* edit = static_cast<UIEditBox*>(UnknownFunction46ebf0("EditPlateNumber", 0xb));
        edit->UnknownFunction473f30("0123456789");
        g_UnknownGlobal56e26c->ui->UnknownFunction4999f0(this);
        Vector3* eye = &field_0x7f58;
        field_0x7f8c.left = 0x1b;
        field_0x7f8c.right = 0x138;
        field_0x7f8c.top = 0x69;
        field_0x7f8c.bottom = 0xf6;
        *eye = kVec3Zero;
        float fov = 50.0f;
        Vector3* target = &field_0x7f64;
        field_0x7f58.z = 15.0f;
        field_0x7f58.y = 1.0f;
        *target = g_UnknownGlobal56e26c->ui->field_0x474;
        field_0x7f64.y += 3.0f;
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownFunction42e9b0(eye, 0, 0, 0, (int)&fov);
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownVirtualSlot29(*target);
        g_UnknownGlobal56e26c->ui->field_0x468->UnknownFunction42f190(
            field_0x7f8c.left, field_0x7f8c.top, field_0x7f8c.right - field_0x7f8c.left,
            field_0x7f8c.bottom - field_0x7f8c.top);
        field_0x7f70 = UnknownInlineLength(UnknownInlineDifference(*eye, g_UnknownGlobal56e26c->ui->field_0x474));
        UnknownFunction4500d0();
        UnknownFunction4f8d20();
        UnknownFunction4f8570(g_UnknownGlobal56e26c->mode.field_0x1bcc);
        srand(ReadClock());
        if (field_0x110)
            field_0x7f74 = field_0x110->UnknownFunction4040f0(1);
        if ((g_UnknownGlobal56e26c->mode.field_0x1bd4 & 4) || g_UnknownGlobal689df4) {
            UnknownFunction46ebf0("ButWrench", 0)->UnknownVirtualSlot49(0);
            UnknownFunction46ebf0("DDLBikes", 6)->UnknownVirtualSlot49(0);
            UnknownFunction46ebf0("BikeLeft", 0)->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("BikeRight", 0)->UnknownFunction470660(0, 1);
            UnknownFunction46ebf0("DDLEngineSize", 6)->UnknownVirtualSlot49(0);
        }
        UnknownFunction4f8700();
        break;
    }
    case 2:
        if (!_stricmp("DDLBikes", event->field_0x04)) {
            UnknownFunction4f8d20();
            UnknownFunction46ecc0(1);
        } else if (!_stricmp("DDLRiders", event->field_0x04)) {
            UnknownFunction4500d0();
            UnknownFunction46ecc0(1);
        } else if (!_stricmp("DDLEngineSize", event->field_0x04)) {
            UnknownFunction4f8d20();
        } else if (!_stricmp("LargestOpponentDropDown", event->field_0x04)) {
            if (g_UnknownGlobal56e26c->field_0x08->isHost)
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x1c = static_cast<UIListBox*>(event->field_0x14)->UnknownFunction4768d0(-1);
        }
        break;
    case 1:
        if (!_stricmp("BikeLeft", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("BikeRight", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RiderLeft", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLRiders", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RiderRight", event->field_0x04)) {
            UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLRiders", 6))->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("ButWrench", event->field_0x04)) {
            OptionsDlg* dialog = new(__FILE__, 0xb19) OptionsDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, (UnknownGuiDialog*)field_0x2c, 2, 0, 1);
        }
        break;
    case 10:
        if (!_stricmp("EditPlateNumber", event->field_0x04)) {
            static_cast<UIEditBox*>(event->field_0x14)->UnknownFunction473ef0(plate, 9);
            int number = atoi(plate);
            if (number < 100)
                number += 100;
            if (number >= 101) {
                if (number > 999)
                    number = 999;
            } else {
                number = 101;
            }
            g_UnknownGlobal56e26c->mode.field_0x1bcc = number;
            _itoa(number, plate, 10);
            static_cast<UIEditBox*>(event->field_0x14)->UnknownFunction473da0(plate);
            UnknownFunction4f8570(number);
        }
        break;
    case 19:
        if (!_stricmp("EditPlateNumber", event->field_0x04)) {
            static_cast<UIEditBox*>(event->field_0x14)->UnknownFunction473ef0(typed, 9);
            int number = atoi(typed);
            if (number >= 100 && number <= 999) {
                UnknownFunction4f8570(number);
                g_UnknownGlobal56e26c->mode.field_0x1bcc = number;
            }
        }
        break;
    case 6:
        g_UnknownGlobal56e26c->ui->UnknownFunction499a20();
        if (field_0x110)
            field_0x110->UnknownFunction404200(field_0x7f74);
        ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(0);
        break;
    }
}

// 0x004f8220
void MPBikeRiderDlg::UnknownFunction4f8220() {
    int bike = 0;
    int rider = 0;
    char text[0x80];
    MPBikeRiderDlg* self = this;
    UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
    list->UnknownFunction4775f0();
    for (int i = 0; i < g_UnknownGlobal56e26c->ui->field_0x54; i++) {
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[i];
        if (!entry->field_0x88) {
            int kind = ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[entry->field_0x00].field_0xc4;
            if (kind != 2 && kind != 3 && (kind != 10 || !(g_UnknownGlobal56e26c->mode.field_0x1970 & 1)))
                continue;
        }
        int flags = g_UnknownGlobal56e26c->mode.field_0x1bd4;
        if (!flags) {
            sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[entry->field_0x00].field_0x00,
                    entry->field_0x04);
            list->UnknownFunction476d80(text, i, flags);
        } else {
            if (entry->field_0x88)
                continue;
            int allowed = 0;
            int byModel = flags & 4;
            if (byModel) {
                int model = g_UnknownGlobal56e26c->networkGameObject->field_0x298;
                if ((model == entry->field_0x00 || (!model && entry->field_0x00 == 7)) &&
                    UnknownBikeClassOf(entry->field_0x8c) <= 2) {
                    allowed = 1;
                    if ((flags & 8) && i != g_UnknownGlobal56e26c->mode.field_0x1974.field_0xc4)
                        allowed = 0;
                }
            }
            if (!(flags & 8) && !byModel && UnknownBikeClassOf(entry->field_0x8c) <= 2)
                allowed = 1;
            if (!allowed)
                continue;
            sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[entry->field_0x00].field_0x00,
                    entry->field_0x04);
            list->UnknownFunction476d80(text, i, 0);
        }
        if (g_UnknownGlobal56e26c->mode.field_0x1974.field_0xc4 == i)
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
        if (entry->field_0xc4 == 2 || entry->field_0xc4 == 3 || entry->field_0xc0) {
            list->UnknownFunction476d80(entry->field_0x00, j, 0);
            if (!strcmp(((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x58)[j].field_0x40,
                        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80))
                rider = j;
        }
    }
    list->UnknownFunction476b30(rider);
}

// 0x004f17a0
int UnknownFunction4f17a0(DirectoryList* directories, const char* directory, const char* pattern, const char* kind,
                          int a, const char* picture, const char* listName, char* name, int* value,
                          UIDialog* dialog, int b, int append) {
    char first[260];
    char second[260];
    char fallback[260];
    char selected[260];
    char found[260];
    char path[260];
    char file[260];
    char entry[260];
    char text[64];
    UIListBox* list = 0;
    int index = 0;
    if (!dialog)
        dialog = (UIDialog*)g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485df0();
    UIListBox* pictures = static_cast<UIListBox*>(dialog->UnknownFunction46ebf0(picture, 0));
    pictures->UnknownFunction477e60(1);
    if (!append)
        pictures->UnknownFunction4775f0();
    if (strcmp(listName, "")) {
        list = static_cast<UIDropDownList*>(dialog->UnknownFunction46ebf0(listName, 0))->field_0x1fc;
        if (!append)
            list->UnknownFunction4775f0();
    }
    if (!append)
        g_UnknownGlobal56e26c->ui->UnknownFunction49bb80();
    COPY_TEXT(selected, name, 260);
    strcpy(name, "");
    if (value)
        *value = 0;
    sprintf(first, "%s\\%s", g_UnknownGlobal56e26c->mode.field_0x23d0, directory);
    sprintf(second, "%s\\%s", g_UnknownGlobal56e26c->mode.field_0x24d4, directory);
    ((CombinedDirectoryList*)directories)->UnknownFunction44abb0(first, second);
    directories->UnknownFunction44a220(pattern, 1);
    directories->UnknownVirtualSlot1();
    if (!directories->UnknownFunction4673b0())
        return 1;
    if (a)
        sprintf(path, "%s\\%s", "ui", "unart.tga");
    else
        sprintf(path, "%s\\%s", "ui", "unarts.tga");
    g_UnknownGlobal56e26c->UnknownVirtualSlot18(path, fallback);
    int more = directories->UnknownFunction44a550(entry);
    UnknownTextureStream* stream = new(__FILE__, 180) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (more) {
        do {
            int i;
            int x;
            int y;
            int series;
            for (i = 0; i < 6; i++) {
                if (!_stricmp(entry, g_UnknownGlobal56e26c->mode.field_0x139c[i])) {
                    if (!g_UnknownGlobal56e26c->mode.field_0x1384[i])
                        goto next;
                    break;
                }
            }
            g_UnknownGlobal56e26c->mode.UnknownFunction523a60((int)directory, entry, kind, found);
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9b80(found);
            series = g_UnknownGlobal56e26c->mode.UnknownFunction524100();
            if (_strnicmp(entry, "Quarry01.env", strlen(entry)) && _strnicmp(entry, "Nat04.env", strlen(entry)))
                series = -1;
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 != 1 &&
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 != 5) {
                g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(text, entry, 0, "scn", &x, &y);
                int row;
                if (g_UnknownGlobal56e26c->field_0x08 && g_UnknownGlobal56e26c->field_0x08->isHost)
                    row = g_UnknownGlobal56e26c->ui->UnknownFunction49ba70(index, 0, series, entry, x, y);
                else
                    row = g_UnknownGlobal56e26c->ui->UnknownFunction49ba70(index, 0, series, entry, 0, 0);
                if (g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 1 ||
                    g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 5) {
                    if (a)
                        sprintf(file, "%s01.tga", entry);
                    else
                        sprintf(file, "%s01s.tga", entry);
                } else {
                    if (a)
                        sprintf(file, "%s.tga", entry);
                    else
                        sprintf(file, "%ss.tga", entry);
                }
                sprintf(path, "%s\\%s", directory, file);
                if (g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)file))
                    pictures->UnknownFunction477110(file, row, 1, 0);
                else if (g_UnknownGlobal56e26c->UnknownVirtualSlot18(path, found))
                    pictures->UnknownFunction477110(found, row, 0, 0);
                else
                    pictures->UnknownFunction477110(fallback, row, 0, 0);
                if (!_stricmp("no name", text))
                    g_UnknownGlobal56e26c->UnknownFunction521970(0x143b, text, 128);
                list->UnknownFunction476d80(text, row, 0);
            } else {
                g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(text, entry, 1, "scn", 0, 0);
                int count = g_UnknownGlobal56e26c->sceneObject->field_0x390;
                for (int scene = 1; scene <= count; scene++) {
                    g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(text, entry, scene, "scn", &x, &y);
                    int row;
                    if (g_UnknownGlobal56e26c->field_0x08 && g_UnknownGlobal56e26c->field_0x08->isHost)
                        row = g_UnknownGlobal56e26c->ui->UnknownFunction49ba70(index, scene, series, entry, x, y);
                    else
                        row = g_UnknownGlobal56e26c->ui->UnknownFunction49ba70(index, scene, series, entry, 0, 0);
                    if (a)
                        sprintf(file, "%s%02d.tga", entry, scene);
                    else
                        sprintf(file, "%s%02ds.tga", entry, scene);
                    sprintf(path, "%s\\%s", directory, file);
                    if (g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)file))
                        pictures->UnknownFunction477110(file, row, 1, 0);
                    else if (g_UnknownGlobal56e26c->UnknownVirtualSlot18(path, found))
                        pictures->UnknownFunction477110(found, row, 0, 0);
                    else
                        pictures->UnknownFunction477110(fallback, row, 0, 0);
                    if (!_stricmp("no name", text))
                        g_UnknownGlobal56e26c->UnknownFunction521970(0x143b, text, 128);
                    list->UnknownFunction476d80(text, row, 0);
                }
            }
        next:
            index++;
        } while (directories->UnknownFunction44a4c0(entry));
    }
    list->UnknownFunction477900(1);
    if (!append) {
        list->UnknownFunction476a60(0);
        if (value)
            *value = 0;
    }
    if (strcmp(selected, "")) {
        g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9e30(selected, "scn", 0);
        if (g_UnknownGlobal56e26c->sceneObject->field_0x38c) {
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea390(text, g_UnknownGlobal56e26c->sceneObject->field_0x24c, b);
            list->UnknownFunction476ad0(text);
            int row = list->UnknownFunction476950();
            if (value)
                *value = row;
        }
    }
    int row = list->UnknownFunction4768d0(-1);
    directories->UnknownVirtualSlot1();
    directories->UnknownFunction44a2f0(g_UnknownGlobal56e26c->ui->field_0x60[row].field_0x00, name);
    delete stream;
    return 1;
}

// 0x004f8820: as SPBikeRiderDlg slot 10 (dlgprocs.cpp 0x0044fae0).
int MPBikeRiderDlg::UnknownVirtualSlot10(float frameTime) {
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

#include "../../src/reconstructed/DlgProcs.h" // MainDlg, built inline by slot 24 (0x004f43c1)

// The player left message (type 5).
struct UnknownPlayerLeftMessage {
    int field_0x00;
    int field_0x04;
    int field_0x08;                           // player id
};

// The lobby start message (type 0x83, 0x8d8 bytes).
struct UnknownLobbyStartMessage {
    int field_0x00;
    short field_0x04;                         // start
    UnknownTrackGameModeSettings field_0x08;  // TrackGame+0x2d70
    int field_0x1f4;                          // TrackGame+0x60c
    int field_0x1f8[8];                       // racer slot +0xc0
    char field_0x218[8][0x10];                // racer slot +0xdc (name)
    char field_0x298[8][0x40];                // racer slot +0x00
    char field_0x498[8][0x40];                // racer slot +0x40
    char field_0x698[8][0x40];                // racer slot +0x80
    int field_0x898[8];                       // racer slot +0xec
    int field_0x8b8[8];                       // racer slot +0xf0
};

// A grid entry at race settings +0x14c (TrackGame.h's
// UnknownTrackGameModeEntry) with the racer index at +5.
struct UnknownGridEntry {
    UnknownGridEntry() {
        field_0x00 = 0;
        field_0x04 = 0;
    }

    int field_0x00;                           // player id
    char field_0x04;                          // championship points
    char field_0x05;                          // racer index
};

// A random value in [0, 1) (as in KrustyUI.cpp).
static inline float RandomUnit() {
    return (float)(rand() * (1.0f / 32768));
}

// 0x004f2340: sends the lobby start message; with `start` it first draws
// the AI racers and the starting grid.
void MultiPlayerDlg::UnknownFunction4f2340(short start) {
    int i;
    g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35 = g_UnknownGlobal689df8 + 1;
    g_UnknownGlobal56e26c->field_0x18 = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35;
    g_UnknownGlobal56e26c->field_0x3424 = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28;
    g_UnknownGlobal56e26c->mode.field_0x1be0 = g_UnknownGlobal56e26c->field_0x3424 + g_UnknownGlobal56e26c->field_0x18;
    int total = g_UnknownGlobal56e26c->mode.field_0x1be0;
    g_UnknownGlobal56e26c->mode.field_0x1bd8 =
        field_0x7f68.UnknownFunction4f2fe0(g_UnknownGlobal689df8 + 1, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28);
    if (g_UnknownGlobal56e26c->mode.field_0x1bd8 < g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28)
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = g_UnknownGlobal56e26c->mode.field_0x1bd8;
    if (start) {
        int* bikes = (int*)DebugCalloc(total, 4, __FILE__, 0x1f4);
        int* riders = (int*)DebugCalloc(total, 4, __FILE__, 0x1f5);
        const char** names = (const char**)DebugMalloc(total * 4, __FILE__, 0x1f6);
        g_UnknownGlobal56e26c->ui->UnknownFunction49b0d0(bikes, total, riders);
        g_UnknownGlobal56e26c->ui->UnknownFunction49b020(names, total);
        for (i = 0; i < g_UnknownGlobal56e26c->field_0x18 - 1; i++) {
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xd4 = g_UnknownGlobal689d08[i].field_0x04;
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xd8 = 0;
        }
        g_UnknownGlobal56e26c->mode.field_0x1be4[0].field_0xd4 = g_UnknownGlobal56e26c->field_0x08->field_0x0c;
        g_UnknownGlobal56e26c->mode.field_0x1be4[0].field_0xd8 = 0;
        int racer = g_UnknownGlobal689df8 + 1;
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < field_0x7f68.field_0x00[j].field_0x08; k++) {
                g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0xd4 = field_0x7f68.field_0x00[j].field_0x00;
                g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0xd8 = racer;
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0xdc, names[racer], 0x10);
                g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0xc0 = 100 - (int)(RandomUnit() * -899.0f);
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0x00,
                          ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)
                              [((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bikes[racer]].field_0x00]
                                  .field_0x40,
                          0x40);
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0x40,
                          ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bikes[racer]].field_0x48, 0x40);
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0x80,
                          ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x58)[riders[racer]].field_0x40, 0x40);
                g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0xec =
                    ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bikes[racer]].field_0x8c;
                g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0xf0 =
                    ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bikes[racer]].field_0x90;
                racer++;
            }
        }
        g_UnknownGlobal56e26c->mode.UnknownFunction522cd0();
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2 && g_UnknownGlobal56e26c->eventManager->field_0x48 > 0) {
            // By the championship points.
            for (i = 0; i < racer; i++) {
                UnknownGridEntry entry;
                entry.field_0x00 = g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd4;
                entry.field_0x04 = g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x28;
                entry.field_0x05 = g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd8;
                ((UnknownGridEntry*)g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x14c)[i] = entry;
            }
            qsort(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x14c, racer, sizeof(UnknownGridEntry),
                  UnknownFunction4f2080);
        } else {
            // At random.
            UnknownGridDraw draws[8];
            for (i = 0; i < racer; i++) {
                UnknownGridDraw draw;
                draw.field_0x00 = g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd4;
                draw.field_0x04 = RandomUnit() * 10.0f;
                draw.field_0x08 = g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd8;
                draws[i] = draw;
            }
            qsort(draws, racer, sizeof(UnknownGridDraw), UnknownFunction4f20a0);
            for (i = 0; i < racer; i++) {
                UnknownGridEntry entry;
                entry.field_0x00 = draws[i].field_0x00;
                entry.field_0x05 = draws[i].field_0x08;
                ((UnknownGridEntry*)g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x14c)[i] = entry;
            }
        }
        if (names)
            operator delete(names, __FILE__, 0x23e);
    }
    UnknownLobbyStartMessage message;
    message.field_0x04 = start;
    message.field_0x08 = g_UnknownGlobal56e26c->mode.field_0x27f8;
    message.field_0x1f4 = g_UnknownGlobal56e26c->mode.field_0x94;
    memcpy(message.field_0x08.field_0x18c, field_0x7f68.field_0x00, sizeof(field_0x7f68.field_0x00));
    if (start) {
        char name[0x80];
        message.field_0x1f8[0] = g_UnknownGlobal56e26c->mode.field_0x1bcc;
        g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac720(g_UnknownGlobal56e26c->field_0x08->field_0x0c, name);
        strcpy(message.field_0x218[0], name);
        int humans = g_UnknownGlobal689df8 + 1;
        for (i = 1; i < humans; i++) {
            message.field_0x1f8[i] = g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xc0;
            strcpy(message.field_0x218[i], g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xdc);
        }
        for (i = humans; i < total; i++) {
            message.field_0x1f8[i] = g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xc0;
            strcpy(message.field_0x218[i], g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xdc);
            strcpy(message.field_0x298[i], g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0x00);
            strcpy(message.field_0x498[i], g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0x40);
            strcpy(message.field_0x698[i], g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0x80);
            message.field_0x898[i] = g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xec;
            message.field_0x8b8[i] = g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xf0;
        }
    }
    g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac830(0x83, &message, sizeof(message), g_UnknownGlobal56e26c->field_0x08->field_0x0c, 0);
    if (start)
        UnknownFunction4f20d0();
}

// 0x004f3a70: the lobby's network messages.
int MultiPlayerDlg::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    int i;
    if (from && type == 2) {
        UnknownPlayerStateMessage* state = (UnknownPlayerStateMessage*)data;
        for (i = 0; i < 7; i++) {
            if (g_UnknownGlobal689d08[i].field_0x04 != from)
                continue;
            char name[0x80];
            sprintf(name, "ButRdyPlayer%d", i + 2);
            UnknownGameUiControl* button = UnknownFunction46ebf0(name, 1);
            if (button) {
                if (state->field_0x01) {
                    button->UnknownFunction470d40(0xffffff);
                    g_UnknownGlobal689dbc[i] = 1;
                    field_0x7f68.UnknownFunction4f2f20(from, state->field_0xc3);
                    if (!g_UnknownGlobal689dd8[i]) {
                        char text[0x80];
                        char line[0x80];
                        g_UnknownGlobal56e26c->UnknownFunction521970(0x14d5, text, 0x80);
                        sprintf(line, text, g_UnknownGlobal689d08[i].field_0x08, state->field_0xc8);
                        UnknownFunction4f3720(-1, line);
                        g_UnknownGlobal689dd8[i] = 1;
                    }
                } else {
                    button->UnknownFunction470d40(0x808080);
                    g_UnknownGlobal689dbc[i] = 0;
                    g_UnknownGlobal689dd8[i] = 0;
                    field_0x7f68.UnknownFunction4f2f80(from);
                }
            }
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xd4 = g_UnknownGlobal689d08[i].field_0x04;
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xd8 = 0;
            COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xdc, g_UnknownGlobal689d08[i].field_0x08, 0x10);
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].UnknownFunction521fb0(state->field_0x42, state->field_0x02,
                                                                                 state->field_0x82);
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xec = state->field_0xc8;
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xf0 = state->field_0xcc;
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xf4 = state->field_0xcd;
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xc0 = state->field_0xd0;
            g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xc4 = state->field_0xc2;
            float delay = (float)(unsigned int)flags - (float)(unsigned int)state->field_0xc4;
            if (delay < g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xd0)
                g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xd0 = delay;
            field_0x7f68.UnknownFunction4f2f20(from, state->field_0xc3);
            if (g_UnknownGlobal56e26c->field_0x08->isHost && field_0x7f58) {
                UIListBox* list = static_cast<UIListBox*>(field_0x7f58->UnknownFunction46ebf0("OpponentsListBox", 0));
                if (list && list->field_0x1ec - 1 != g_UnknownGlobal56e26c->mode.field_0x1bd8) {
                    int selection = list->UnknownFunction476950();
                    list->UnknownFunction4775f0();
                    for (int j = 0; j <= g_UnknownGlobal56e26c->mode.field_0x1bd8; j++) {
                        char number[0x80];
                        sprintf(number, "%d", j);
                        list->UnknownFunction476d80(number, j, 0);
                    }
                    if (g_UnknownGlobal56e26c->mode.field_0x1bd8 < selection)
                        list->UnknownFunction476a60(g_UnknownGlobal56e26c->mode.field_0x1bd8);
                    else
                        list->UnknownFunction476a60(selection);
                }
            }
        }
    } else if (from && type == 0x83) {
        UnknownLobbyStartMessage* start = (UnknownLobbyStartMessage*)data;
        g_UnknownGlobal56e26c->mode.field_0x27f8 = start->field_0x08;
        g_UnknownGlobal56e26c->mode.field_0x94 = start->field_0x1f4;
        int total = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35 + g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28;
        if (start->field_0x04) {
            for (i = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35; i < total; i++) {
                g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xc0 = start->field_0x1f8[i];
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xdc, start->field_0x218[i], 0x10);
                g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd8 = i;
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0x00, start->field_0x298[i], 0x40);
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0x40, start->field_0x498[i], 0x40);
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0x80, start->field_0x698[i], 0x40);
                g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xec = start->field_0x898[i];
                g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xf0 = start->field_0x8b8[i];
            }
            memcpy(field_0x7f68.field_0x00, start->field_0x08.field_0x18c, sizeof(field_0x7f68.field_0x00));
            UnknownFunction4f20d0();
        } else {
            UnknownFunction4f5ca0();
        }
    } else if (from && type == 0xf) {
        UnknownLobbySettingsMessage* settings = (UnknownLobbySettingsMessage*)data;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140 = settings->minutes;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 = settings->eventType;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 = settings->raceMode;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x08 = settings->field_0x2d78;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20 = settings->laps;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = settings->opponents;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35 = settings->players;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c = settings->races;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x144 = settings->tagBall;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x148 = settings->stuntMode;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x10 = settings->treeCollision;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x14 = settings->riderCollision;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x18 = settings->fastFinishes;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x1c = settings->bikeClass;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34 = settings->field_0x09;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x138 = settings->field_0x0c;
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x13c = settings->field_0x10;
        g_UnknownGlobal56e26c->mode.field_0x94 = settings->detail;
        g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x08);
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36[0] = 0;
        strncat(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, settings->field_0x14, 0x3f);
        memcpy(field_0x7f68.field_0x00, settings->field_0x54, sizeof(field_0x7f68.field_0x00));
        UnknownFunction4f5ca0();
    } else if (from && type == 0x85) {
        UnknownFunction4f3720(from, (const char*)data + 4);
        return 1;
    } else if (from && type == 0x92) {
        UnknownFunction4f3720(-1, (const char*)data + 4);
        return 1;
    } else if (type == 5) {
        field_0x7f68.UnknownFunction4f2f80(((UnknownPlayerLeftMessage*)data)->field_0x08);
        if (field_0x7f58) {
            UIListBox* list = static_cast<UIListBox*>(field_0x7f58->UnknownFunction46ebf0("OpponentsListBox", 0));
            if (g_UnknownGlobal56e26c->field_0x08->isHost && list &&
                list->field_0x1ec - 1 != g_UnknownGlobal56e26c->mode.field_0x1bd8) {
                int selection = list->UnknownFunction476950();
                list->UnknownFunction4775f0();
                for (int j = 0; j <= g_UnknownGlobal56e26c->mode.field_0x1bd8; j++) {
                    char number[0x80];
                    sprintf(number, "%d", j);
                    list->UnknownFunction476d80(number, j, 0);
                }
                if (g_UnknownGlobal56e26c->mode.field_0x1bd8 < selection)
                    list->UnknownFunction476a60(g_UnknownGlobal56e26c->mode.field_0x1bd8);
                else
                    list->UnknownFunction476a60(selection);
            }
        }
    } else if (from && type == 0x8e) {
        UnknownKickMessage* kick = (UnknownKickMessage*)data;
        int player = g_UnknownGlobal56e26c->field_0x08->field_0x0c;
        if (kick->field_0x04 == player) {
            if (g_UnknownGlobal56e26c->field_0x08->field_0x14) {
                UnknownFunction46ff30(0);
                g_UnknownGlobal56e26c->ui->UnknownFunction49a4a0();
                return 1;
            }
            UnknownFunction46ff30(0);
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(new(__FILE__, 0x537) MainDlg, 0x64, 2, 0, 0, 0x63,
                                                                         0, 1);
            return 1;
        }
        field_0x7f68.UnknownFunction4f2f80(kick->field_0x04);
        if (field_0x7f58) {
            UIListBox* list = static_cast<UIListBox*>(field_0x7f58->UnknownFunction46ebf0("OpponentsListBox", 0));
            if (g_UnknownGlobal56e26c->field_0x08->isHost && list &&
                list->field_0x1ec - 1 != g_UnknownGlobal56e26c->mode.field_0x1bd8) {
                int selection = list->UnknownFunction476950();
                list->UnknownFunction4775f0();
                for (int j = 0; j <= g_UnknownGlobal56e26c->mode.field_0x1bd8; j++) {
                    char number[0x80];
                    sprintf(number, "%d", j);
                    list->UnknownFunction476d80(number, j, 0);
                }
                if (g_UnknownGlobal56e26c->mode.field_0x1bd8 < selection)
                    list->UnknownFunction476a60(g_UnknownGlobal56e26c->mode.field_0x1bd8);
                else
                    list->UnknownFunction476a60(selection);
            }
        }
    }
    return GameObject::UnknownVirtualSlot24(type, data, from, to, flags);
}
