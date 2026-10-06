// Near misses for dlgprocs.cpp (canonical file:
// src/reconstructed/DlgProcs.cpp, included below for its types and its
// matched functions). Check: compile this file and compare each function with
// DlgProcsNearMisses.bindings.json.
//
// SPBikeRiderDlg::UnknownVirtualSlot23 (0x0044fa00, 174 bytes): the same
//   calls and stores; retail keeps the event in ebx and copies the preview
//   rectangle's address into edi before reading it, VC6 here reads the
//   rectangle through `this`. A local pointer, an inline accessor, an inline
//   inset helper and an aggregate initializer compile the same.
// SPBikeRiderDlg::UnknownFunction44f750 (0x0044f750, 509 bytes): the same
//   flow and calls as MPBikeRiderDlg 0x004f8220 (also a near miss); retail
//   swaps the registers holding KrustyUI and the bike entry.
// UnknownFunction452930 (0x00452930, 1875 bytes): the same cases, calls and
//   line numbers; retail keeps the event in ebx for the option cases (whose
//   identical tails it merges into case 0xcb's) and does not keep zero in a
//   register, VC6 here merges the tails into case 0xd0 and uses ebx for zero.
//   Every order of the first five cases was tried.

#include "../../src/reconstructed/DlgProcs.cpp"

extern "C" const GUID DPSPGUID_TCPIP;         // 0x005567e0
extern "C" const GUID DPSPGUID_SERIAL;        // 0x005567f0

// TrackGame's settings blocks, as OptionProcs.cpp names them.
static inline UnknownOptGameSettings* UnknownGameSettingsOf(TrackGame* game) {
    return (UnknownOptGameSettings*)((char*)game + 0xc24);
}

static inline UnknownOptSoundSettings* UnknownSoundSettingsOf(TrackGame* game) {
    return (UnknownOptSoundSettings*)((char*)game + 0xf98);
}

static inline UnknownOptGraphicsSettings* UnknownGraphicsSettingsOf(TrackGame* game) {
    return (UnknownOptGraphicsSettings*)&game->mode.field_0xa4c;
}

// 0x0044fa00
int SPBikeRiderDlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind == 1 && event->control == 0) {
        CameraRect* preview = &field_0x7f8c;
        RECT area;
        area.left = preview->left + 50;
        area.top = preview->top + 30;
        area.right = preview->right - 50;
        area.bottom = preview->bottom - 30;
        if (PtInRect(&area, field_0x34->field_0x2c->field_0xa4)) {
            field_0x7f78 = 1;
            field_0x7f7c = 1;
            UnknownFunction46ebf0("ChkAutoRotate", 2)->UnknownFunction478cf0(0);
        }
    }
    return UIDialog::UnknownVirtualSlot23(event, entry);
}

// 0x0044f750
void SPBikeRiderDlg::UnknownFunction44f750() {
    int bike = 0;
    int rider = 0;
    char text[0x80];
    SPBikeRiderDlg* self = this;
    UnknownGameUiControl* list = UnknownFunction46ebf0("DDLBikes", 6)->field_0x1fc;
    list->UnknownFunction4775f0();
    for (int i = 0; i < g_UnknownGlobal56e26c->ui->field_0x54; i++) {
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[i];
        if (!entry->field_0x88) {
            int kind = ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[entry->field_0x00].field_0xc4;
            if (kind != 2 && kind != 3 && (kind != 10 || !(g_UnknownGlobal56e26c->mode.field_0x1970 & 1)))
                continue;
        }
        sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[entry->field_0x00].field_0x00,
                entry->field_0x04);
        list->UnknownFunction476d80(text, i, 0);
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
        UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x58)[j];
        if (model->field_0xc4 == 2 || model->field_0xc4 == 3 || model->field_0xc0) {
            list->UnknownFunction476d80(model->field_0x00, j, 0);
            if (!strcmp(((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x58)[j].field_0x40,
                        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80))
                rider = j;
        }
    }
    list->UnknownFunction476b30(rider);
}

// 0x00452930
void UnknownFunction452930(int menu, UnknownDialogEvent* event) {
    void* address;
    unsigned long size;
    char text[128];
    switch (menu) {
    case 0xc9:
        event->field_0x0c->UnknownFunction46ecc0(1);
        *UnknownGameSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688898;
        g_UnknownGlobal56e26c->ui->field_0x2c->field_0x0ec = UnknownGameSettingsOf(g_UnknownGlobal56e26c)->field_0x1c;
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x66);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xca:
        event->field_0x0c->UnknownFunction46ecc0(1);
        *UnknownSoundSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal688798;
        g_UnknownGlobal56e26c->UnknownFunction521a30();
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x66);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xcb: {
        event->field_0x0c->UnknownFunction46ecc0(1);
        UnknownGameUiControl* modes = event->field_0x0c->UnknownFunction46ebf0("ModeDDL", 0)->field_0x1fc;
        g_UnknownGlobal6887d8.field_0x00 = modes->UnknownFunction4768d0(-1);
        *UnknownGraphicsSettingsOf(g_UnknownGlobal56e26c) = g_UnknownGlobal6887d8;
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x66);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    }
    case 0xcd:
        if (strcmp(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, "")) {
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0xd0);
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        }
        break;
    case 0xd0:
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x68);
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xd8: {
        UnknownComPortAddress serial;
        serial.field_0x00 = 0;
        serial.field_0x04 = 0;
        serial.field_0x08 = 0;
        serial.field_0x0c = 0;
        serial.field_0x10 = 0;
        UnknownFunction4ae410(&g_UnknownGlobal6886b8, &serial);
        if (!g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abf10(DPSPGUID_SERIAL, "", "", &serial, &address,
                                                                     &size)) {
            event->field_0x0c->UnknownFunction46ff30(0xd);
            event->field_0x20 = 1;
            break;
        }
        if (g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abdf0(address, size, 8)) {
            operator delete(address, __FILE__, 3843);
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x85c);
            return;
        }
        event->field_0x0c->UnknownFunction46ff30(0xd);
        event->field_0x20 = 1;
        operator delete(address, __FILE__, 3850);
        break;
    }
    case 0xd9:
        address = 0;
        event->field_0x0c->UnknownFunction46ebf0("EditBox", 0)->UnknownFunction473ef0(text, 128);
        if (!strcmp(text, "")) {
            event->field_0x0c->UnknownFunction46ff30(0xe);
            event->field_0x20 = 1;
            break;
        }
        if (!g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abf10(DPSPGUID_TCPIP, text, "", 0, &address, &size)) {
            event->field_0x0c->UnknownFunction46ff30(0xd);
            event->field_0x20 = 1;
            break;
        }
        if (g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abdf0(address, size, 4)) {
            event->field_0x0c->UnknownFunction46ff30(0xf);
            event->field_0x20 = 1;
            operator delete(address, __FILE__, 3874);
            break;
        }
        event->field_0x0c->UnknownFunction46ff30(0xd);
        event->field_0x20 = 1;
        operator delete(address, __FILE__, 3879);
        break;
    case 0x104:
    case 0x105:
        g_UnknownGlobal56e26c->ui->field_0x30 = g_UnknownGlobal56e26c->ui->field_0x3c;
        switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
        case 0:
        case 2:
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
            break;
        case 3:
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0xcd);
            break;
        }
        break;
    case 0x12c:
        event->field_0x0c->UnknownFunction46ebf0("EditBox", 0)
            ->UnknownFunction473ef0(g_UnknownGlobal56e26c->mode.field_0x10, 128);
        if (!strcmp(g_UnknownGlobal56e26c->mode.field_0x10, "")) {
            char computer[16];
            size = sizeof(computer);
            GetComputerNameA(computer, &size);
            COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x10, computer, 128);
            if (!strcmp(g_UnknownGlobal56e26c->mode.field_0x10, "")) {
                g_UnknownGlobal56e26c->UnknownFunction521970(0xbd8, text, 128);
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x10, text, 128);
            }
        }
        event->field_0x0c->UnknownFunction46ff30(0x14);
        event->field_0x20 = 1;
        break;
    case 0x12f:
        event->field_0x0c->UnknownFunction46ff30(0x32);
        event->field_0x20 = 1;
        break;
    case 0x191:
        g_UnknownGlobal56e26c->UnknownFunction521860(0, 0x191, 1);
        break;
    case 0x1f9:
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0x1fa:
        event->field_0x0c->UnknownFunction46ff30(0);
        event->field_0x20 = 1;
        break;
    case 0xbb9:
    case 0xbba:
    case 0xbbb:
        UnknownFunction451b80(event);
        break;
    }
}
