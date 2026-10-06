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

#include "../../src/reconstructed/SelectGamePicProcs.cpp"

// 0x004f3720
void MultiPlayerDlg::UnknownFunction4f3720(int player, const char* text) {
    char line[0x60];
    char name[0x80];
    char system[0x80];
    UnknownGameUiControl* list = UnknownFunction46ebf0("ListChat", 3);
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
        UnknownGameUiControl* list = UnknownFunction46ebf0("DDLEngineSize", 6)->field_0x1fc;
        list->UnknownFunction4775f0();
        list->UnknownFunction476d80("125cc 2-stroke", 0, 0);
        list->UnknownFunction476d80("250cc 2-stroke", UnknownBikeClassOf(g_UnknownGlobal56cb6c[1]), 0);
        list->UnknownFunction476d80("400cc 4-stroke", UnknownBikeClassOf(g_UnknownGlobal56cb6c[2]), 0);
        list->UnknownFunction476d80("500cc 2-stroke", UnknownBikeClassOf(g_UnknownGlobal56cb6c[3]), 0);
        list->UnknownFunction476d80("600cc 4-stroke", UnknownBikeClassOf(g_UnknownGlobal56cb6c[4]), 0);
        list->UnknownFunction476b30(
            UnknownBikeClassOf(((UnknownOptGarageSettings*)g_UnknownGlobal56e26c->mode.field_0xfd8)->field_0x00));
        g_UnknownGlobal56e26c->UnknownFunction521970(0x146a, format, 0x80);
        list = UnknownFunction46ebf0("LargestOpponentDropDown", 6)->field_0x1fc;
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
        UnknownGameUiControl* edit = UnknownFunction46ebf0("EditPlateNumber", 0xb);
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
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x1c = event->field_0x14->UnknownFunction4768d0(-1);
        }
        break;
    case 1:
        if (!_stricmp("BikeLeft", event->field_0x04)) {
            UnknownGameUiControl* list = UnknownFunction46ebf0("DDLBikes", 6)->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("BikeRight", event->field_0x04)) {
            UnknownGameUiControl* list = UnknownFunction46ebf0("DDLBikes", 6)->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RiderLeft", event->field_0x04)) {
            UnknownGameUiControl* list = UnknownFunction46ebf0("DDLRiders", 6)->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RiderRight", event->field_0x04)) {
            UnknownGameUiControl* list = UnknownFunction46ebf0("DDLRiders", 6)->field_0x1fc;
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
            event->field_0x14->UnknownFunction473ef0(plate, 9);
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
            event->field_0x14->UnknownFunction473da0(plate);
            UnknownFunction4f8570(number);
        }
        break;
    case 19:
        if (!_stricmp("EditPlateNumber", event->field_0x04)) {
            event->field_0x14->UnknownFunction473ef0(typed, 9);
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
    UnknownGameUiControl* list = UnknownFunction46ebf0("DDLBikes", 6)->field_0x1fc;
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
    list = self->UnknownFunction46ebf0("DDLRiders", 6)->field_0x1fc;
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
