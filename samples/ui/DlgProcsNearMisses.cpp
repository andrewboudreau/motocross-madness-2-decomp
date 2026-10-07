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
// MainDlg::UnknownVirtualSlot29 (0x0044b200, 5232 bytes): the same cases,
//   calls and strings; retail shares one frame slot between the two
//   ConnectionInfoType[5] arrays (frame 0x6c4), VC6 here gives each array its
//   own slot (frame 0xc64), so every later frame offset differs. Retail also
//   pushes the constant 1 where VC6 here keeps it in edi.
// SPBikeRiderDlg::UnknownVirtualSlot10 (0x0044fae0, 1235 bytes, 1181
//   match): the same flow and x87 code; only the scheduling of the by-value
//   Vector3 copies passed to the camera's slot 29 differs (retail takes
//   `edi = esp` after the camera and vtable loads and loads z before storing
//   y). A pointer, reference, `this->`, explicit copy, cast or camera local
//   for the argument leaves it; a user-defined Vector3 copy constructor
//   copies before the camera is evaluated (worse).
// SPRaceInfoDlg::UnknownVirtualSlot29 (0x004507a0, 1652 bytes) and
// MPRaceInfoDlg::UnknownVirtualSlot29 (0x004513a0, 2004 bytes): the same
//   calls, strings and list columns; inside the per-racer loops retail
//   reloads TrackGame/EventManager into other registers (VC6 here keeps them
//   live across the loop), so the loop bodies differ from the first loop on.

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
            static_cast<UIMultiState*>(UnknownFunction46ebf0("ChkAutoRotate", 2))->UnknownFunction478cf0(0);
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
    UIListBox* list = static_cast<UIDropDownList*>(UnknownFunction46ebf0("DDLBikes", 6))->field_0x1fc;
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
    list = static_cast<UIDropDownList*>(self->UnknownFunction46ebf0("DDLRiders", 6))->field_0x1fc;
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
        UIListBox* modes = static_cast<UIDropDownList*>(event->field_0x0c->UnknownFunction46ebf0("ModeDDL", 0))->field_0x1fc;
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
            DebugFree(address, __FILE__, 3843);
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x85c);
            return;
        }
        event->field_0x0c->UnknownFunction46ff30(0xd);
        event->field_0x20 = 1;
        DebugFree(address, __FILE__, 3850);
        break;
    }
    case 0xd9:
        address = 0;
        static_cast<UIEditBox*>(event->field_0x0c->UnknownFunction46ebf0("EditBox", 0))->UnknownFunction473ef0(text, 128);
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
            DebugFree(address, __FILE__, 3874);
            break;
        }
        event->field_0x0c->UnknownFunction46ff30(0xd);
        event->field_0x20 = 1;
        DebugFree(address, __FILE__, 3879);
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
        static_cast<UIEditBox*>(event->field_0x0c->UnknownFunction46ebf0("EditBox", 0))
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

// 0x0044b200
void MainDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char name[16];
    char path[260];
    int count;
    void* address;
    unsigned long size;
    switch (event->field_0x08) {
    case 5:
        field_0x7f60 = UnknownFunction46ebf0("MenuSingleAnimation", 5);
        field_0x7f64 = UnknownFunction46ebf0("MenuMultiAnimation", 5);
        field_0x7f68 = UnknownFunction46ebf0("MenuUserAnimation", 5);
        field_0x7f60->UnknownFunction470660(0, 0);
        field_0x7f64->UnknownFunction470660(0, 0);
        field_0x7f68->UnknownFunction470660(0, 0);
        field_0x7f6c = 0;
        field_0x7f70 = 0;
        field_0x7f74 = 0;
        field_0x7f7c = 0.0f;
        g_UnknownGlobal56e26c->ui->field_0x4b0 = 0;
        g_UnknownGlobal56e26c->mode.field_0x10ec = 0;
        UnknownFunction44c7a0();
        UnknownFunction44c8c0();
        if (g_UnknownGlobal56e26c->profileDirectory->count >= 1 && g_UnknownGlobal56e26c->ui->field_0x48c) {
            if (event->field_0x18 == 0x63) {
                PlayerRemovedDlg* dialog = new(__FILE__, 168) PlayerRemovedDlg;
                g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0x1fa, 0xc, 0, (int)this, 0, 0, 1);
            }
        } else {
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0xbba);
        }
        if (g_UnknownGlobal56e26c->field_0x3444) {
            delete g_UnknownGlobal56e26c->field_0x3444;
            g_UnknownGlobal56e26c->field_0x3444 = 0;
        }
        UnknownFunction46ebf0("MenuReplay", 0)->UnknownVirtualSlot49(0);
        UnknownFunction46ebf0("MenuHighScore", 0)->UnknownVirtualSlot49(0);
        UnknownFunction46ebf0("MenuCredits", 0)->UnknownVirtualSlot49(0);
        UnknownFunction46ebf0("ButProCircuit", 0)->UnknownVirtualSlot49(0);
        field_0x7f58 = 0;
        break;
    case 1:
        if (!_stricmp("MenuSingle", event->field_0x04)) {
            field_0x7f6c = field_0x7f6c == 0;
            field_0x7f60->UnknownFunction470660(field_0x7f6c, 1);
            field_0x7f78 = 1;
            if (field_0x7f6c) {
                field_0x7f7c = 0.0f;
                if (field_0x7f58) {
                    field_0x7f58->field_0x1ec->UnknownVirtualSlot16(1);
                    field_0x7f58->UnknownFunction470660(0, 1);
                }
            } else if (field_0x7f58) {
                field_0x7f58->field_0x1ec->UnknownVirtualSlot16(1);
                field_0x7f58->UnknownFunction470660(0, 1);
            }
            if (field_0x7f70) {
                field_0x7f64->UnknownFunction470660(0, 1);
                field_0x7f70 = 0;
                UnknownFunction4aef40();
                g_UnknownGlobal56e26c->ui->field_0x4b0 = 0;
            }
            if (field_0x7f74) {
                field_0x7f68->UnknownFunction470660(0, 1);
                field_0x7f74 = 0;
            }
        } else if (!_stricmp("MenuMulti", event->field_0x04)) {
            field_0x7f70 = field_0x7f70 == 0;
            field_0x7f64->UnknownFunction470660(field_0x7f70, 1);
            if (field_0x7f6c) {
                field_0x7f60->UnknownFunction470660(0, 1);
                field_0x7f6c = 0;
            }
            if (field_0x7f74) {
                field_0x7f68->UnknownFunction470660(0, 1);
                field_0x7f74 = 0;
            }
            if (field_0x7f70) {
                field_0x30->UnknownFunction4865e0(field_0x30->field_0x280, 1);
                UnknownFunction46ebf0("ButIPX", 1)->UnknownVirtualSlot49(0);
                UnknownFunction46ebf0("ButTCP", 1)->UnknownVirtualSlot49(0);
                UnknownFunction46ebf0("ButZone", 1)->UnknownVirtualSlot49(1);
                UnknownFunction46ebf0("ButCable", 1)->UnknownVirtualSlot49(0);
                UnknownFunction46ebf0("ButModem", 1)->UnknownVirtualSlot49(0);
                g_UnknownGlobal56e26c->mode.field_0x2bd0.field_0x28 = 0;
                if (UnknownFunction4aefa0()) {
                    ConnectionInfoType connections[5];
                    g_UnknownGlobal56e26c->field_0x08->UnknownFunction4add50(connections, &count);
                    for (int i = 0; i < count; i++) {
                        if (!memcmp(connections[i].field_0x108, &DPSPGUID_IPX, sizeof(GUID)))
                            UnknownFunction46ebf0("ButIPX", 1)->UnknownVirtualSlot49(1);
                        else if (!memcmp(connections[i].field_0x108, &DPSPGUID_TCPIP, sizeof(GUID)))
                            UnknownFunction46ebf0("ButTCP", 1)->UnknownVirtualSlot49(1);
                        else if (!memcmp(connections[i].field_0x108, &DPSPGUID_SERIAL, sizeof(GUID)))
                            UnknownFunction46ebf0("ButCable", 1)->UnknownVirtualSlot49(1);
                        else if (!memcmp(connections[i].field_0x108, &DPSPGUID_MODEM, sizeof(GUID)))
                            UnknownFunction46ebf0("ButModem", 1)->UnknownVirtualSlot49(1);
                    }
                }
                field_0x30->UnknownFunction4865e0(field_0x30->field_0x200, 0);
            } else {
                UnknownFunction4aef40();
                g_UnknownGlobal56e26c->ui->field_0x4b0 = 0;
            }
        } else if (!_stricmp("MenuUserProfile", event->field_0x04) || !_stricmp("ButCancelProfile", event->field_0x04)) {
            field_0x7f74 = field_0x7f74 == 0;
            field_0x7f68->UnknownFunction470660(field_0x7f74, 1);
            if (field_0x7f70) {
                field_0x7f64->UnknownFunction470660(0, 1);
                field_0x7f70 = 0;
                UnknownFunction4aef40();
                g_UnknownGlobal56e26c->ui->field_0x4b0 = 0;
            }
            if (field_0x7f74)
                UnknownFunction470000(UnknownFunction46ebf0("LstProfiles", 3), 0, 0);
            if (field_0x7f6c) {
                field_0x7f60->UnknownFunction470660(0, 1);
                field_0x7f6c = 0;
            }
        } else if (!_stricmp("MenuOptions", event->field_0x04)) {
            OptionsDlg* dialog = new(__FILE__, 345) OptionsDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, (UnknownGuiDialog*)this, 0, 0, 1);
        } else if (!_stricmp("ButStuntQuarry", event->field_0x04)) {
            if (field_0x7f58)
                field_0x7f58->field_0x1ec->UnknownFunction4a2940();
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x04 = 0;
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x08 = 0;
            g_UnknownGlobal56e26c->mode.field_0x6a0 = (int)"Teraform\\Quarries";
            SinglePlayerDlg* dialog = new(__FILE__, 352) SinglePlayerDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButBaja", event->field_0x04)) {
            if (field_0x7f58)
                field_0x7f58->field_0x1ec->UnknownFunction4a2940();
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x04 = 1;
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x08 = 1;
            g_UnknownGlobal56e26c->mode.field_0x6a0 = (int)"Teraform\\Baja";
            SinglePlayerDlg* dialog = new(__FILE__, 360) SinglePlayerDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButSupercross", event->field_0x04)) {
            if (field_0x7f58)
                field_0x7f58->field_0x1ec->UnknownFunction4a2940();
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x04 = 3;
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x08 = 3;
            g_UnknownGlobal56e26c->mode.field_0x6a0 = (int)"Teraform\\SX";
            SinglePlayerDlg* dialog = new(__FILE__, 368) SinglePlayerDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButEnduro", event->field_0x04)) {
            if (field_0x7f58)
                field_0x7f58->field_0x1ec->UnknownFunction4a2940();
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x04 = 5;
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x08 = 5;
            g_UnknownGlobal56e26c->mode.field_0x6a0 = (int)"Teraform\\Enduro";
            SinglePlayerDlg* dialog = new(__FILE__, 376) SinglePlayerDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButNationals", event->field_0x04)) {
            if (field_0x7f58)
                field_0x7f58->field_0x1ec->UnknownFunction4a2940();
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x04 = 2;
            g_UnknownGlobal56e26c->mode.field_0x29e4.field_0x08 = 2;
            g_UnknownGlobal56e26c->mode.field_0x6a0 = (int)"Teraform\\National";
            SinglePlayerDlg* dialog = new(__FILE__, 384) SinglePlayerDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButProCircuit", event->field_0x04)) {
            if (field_0x7f58)
                field_0x7f58->field_0x1ec->UnknownFunction4a2940();
            PCStartupDlg* dialog = new(__FILE__, 389) PCStartupDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButLoadProfile", event->field_0x04)) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523580();
            if (UnknownFunction44b120(event))
                g_UnknownGlobal56e26c->UnknownFunction521a30();
            UnknownFunction470000(UnknownFunction46ebf0("LstProfiles", 3), 0, 0);
            UnknownFunction44c8c0();
            field_0x7f74 = 0;
            field_0x7f68->UnknownFunction470660(0, 1);
        } else if (!_stricmp("ButNewProfile", event->field_0x04)) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523580();
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0xbb9);
            UnknownFunction470000(UnknownFunction46ebf0("LstProfiles", 3), 0, 0);
            UnknownFunction44c7a0();
            UnknownFunction44c8c0();
            field_0x7f74 = 0;
            field_0x7f68->UnknownFunction470660(0, 1);
        } else if (!_stricmp("ButRemoveProfile", event->field_0x04)) {
            UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstProfiles", 0));
            short row = list->UnknownFunction476950();
            if (!_stricmp(list->UnknownFunction476d20(-1), g_UnknownGlobal56e26c->mode.field_0x00)) {
                NoDelCurProfileDlg* dialog = new(__FILE__, 423) NoDelCurProfileDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0, (UnknownGuiDialog*)this, 0, 0, 1);
            } else if (row != -1) {
                g_UnknownGlobal59adbc = row;
                RemoveProfileDlg* dialog = new(__FILE__, 426) RemoveProfileDlg;
                field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0x12f, 4, 0, (UnknownGuiDialog*)this,
                                                  (int)list->UnknownFunction476d20(-1), 0, 1);
            }
            UnknownFunction470000(list, 0, 0);
            UnknownFunction44c7a0();
        } else if (!_stricmp("ButIPX", event->field_0x04)) {
            g_UnknownGlobal56e26c->ui->field_0x4b0 = 1;
            if (g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abf10(DPSPGUID_IPX, "", "", 0, &address, &size) &&
                g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abdf0(address, size, 1)) {
                DebugFree(address, __FILE__, 443);
                g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x85c);
            }
        } else if (!_stricmp("ButTCP", event->field_0x04)) {
            g_UnknownGlobal56e26c->ui->field_0x4b0 = 1;
            ChooseTCPMethodDlg* dialog = new(__FILE__, 453) ChooseTCPMethodDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 4, 0, (UnknownGuiDialog*)this, 0, 0, 1);
        } else if (!_stricmp("ButZone", event->field_0x04)) {
            g_UnknownGlobal56e26c->openLocalizedWebPageOnExit = 1;
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
            g_UnknownGlobal56e26c->ui->UnknownFunction49a4a0();
        } else if (!_stricmp("ButCable", event->field_0x04)) {
            g_UnknownGlobal56e26c->ui->field_0x4b0 = 1;
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0xd8);
        } else if (!_stricmp("ButModem", event->field_0x04)) {
            g_UnknownGlobal56e26c->ui->field_0x4b0 = 1;
            ConnectionInfoType connections[5];
            g_UnknownGlobal56e26c->field_0x08->UnknownFunction4add50(connections, &count);
            for (int i = 0; i < count; i++) {
                if (!memcmp(connections[i].field_0x108, &DPSPGUID_MODEM, sizeof(GUID))) {
                    g_UnknownGlobal56e26c->field_0x08->field_0x04->InitializeConnection(connections[i].field_0x118, 0);
                    g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0xfc);
                    break;
                }
            }
        } else if (!_stricmp("MenuReplay", event->field_0x04)) {
            GhostReplayDlg* dialog = new(__FILE__, 487) GhostReplayDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            UnknownFunction46ff30(0);
        } else if (!_stricmp("MenuHighScore", event->field_0x04)) {
            TrackRecordDlg* dialog = new(__FILE__, 492) TrackRecordDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0x911, 2, 0, 0, 0, 0, 1);
            UnknownFunction46ff30(0);
        } else if (!_stricmp("MenuCredits", event->field_0x04)) {
            CreditsVidDlg* dialog = new(__FILE__, 501) CreditsVidDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            UnknownFunction46ff30(0);
        } else if (!_stricmp("MenuHelp", event->field_0x04)) {
            g_UnknownGlobal56e26c->mode.UnknownFunction523d30("MCM2HELP", 0);
        } else if (!_stricmp("MenuExit", event->field_0x04)) {
            UnknownFunction46ff30(0);
            g_UnknownGlobal56e26c->ui->UnknownFunction49a4a0();
        }
        break;
    case 9:
        if (event->field_0x00 == 0x1e || event->field_0x00 == 0x32) {
            UIListBox* list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstProfiles", 0));
            if (event->field_0x00 == 0x1e) {
                list->UnknownFunction476d80(g_UnknownGlobal56e26c->mode.field_0x00, 0, 0);
                list->UnknownFunction477900(1);
                list->UnknownFunction476ad0(g_UnknownGlobal56e26c->mode.field_0x00);
            } else if (event->field_0x00 == 0x32) {
                COPY_TEXT(name, list->UnknownFunction476d20(g_UnknownGlobal59adbc), 16);
                sprintf(path, "%s\\%s", "ui\\profile", name);
                if (g_UnknownGlobal56e26c->profileDirectory->UnknownFunction44a960(path)) {
                    g_UnknownGlobal56e26c->profileDirectory->UnknownVirtualSlot1();
                    list->UnknownFunction477490(g_UnknownGlobal59adbc);
                }
            }
            UnknownFunction44c7a0();
            UnknownFunction44c8c0();
            if (field_0x7f74)
                UnknownFunction470000(list, 0, 0);
        }
        if (event->field_0x00 == 0x1f)
            UnknownFunction44c8c0();
        if (event->field_0x00 == 0x63) {
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        }
        if (event->field_0x00 == 0x3d)
            g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x85d);
        if (event->field_0x00 == 0x3c) {
            if (g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abf10(DPSPGUID_TCPIP, "", "", 0, &address, &size) &&
                g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abdf0(address, size, 2)) {
                DebugFree(address, __FILE__, 596);
                g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x85c);
            }
        }
        break;
    case 6:
        field_0x7f74 = 0;
        field_0x7f70 = 0;
        field_0x7f6c = 0;
        field_0x7f68 = 0;
        field_0x7f64 = 0;
        field_0x7f60 = 0;
        break;
    }
}

// 0x0044fae0: turns the bike view (dragged with the pointer, else slowly by
// itself), animates the previews and now and then plays a rider idle motion.
int SPBikeRiderDlg::UnknownVirtualSlot10(float frameTime) {
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
        bikes->UnknownFunction4768d0(-1);
        g_UnknownGlobal56e26c->ui->field_0x46c->UnknownFunction4a8b10("WaitR");
        field_0x7f80 = 1;
    }
    if (field_0x110)
        field_0x110->UnknownFunction404240(field_0x7f74, &field_0x7f8c);
    return UIDialog::UnknownVirtualSlot10(frameTime);
}

// The replay tape's header ("VCRtape.dat", 0xeb0 bytes): the race settings
// at +0x34 and the garage engine size and stroke at +0x33c.
struct UnknownReplayTapeHeader {
    unsigned char field_0x00[0x34];
    UnknownTrackGameModeSettings field_0x34;
    unsigned char field_0x220[0x33c - 0x220];
    int field_0x33c;
    int field_0x340;
    unsigned char field_0x344[0xeb0 - 0x344];
};

// 0x004507a0: fills the result lists (by event type) when the page opens;
// "ButViewReplay" plays the last race's tape.
void SPRaceInfoDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char position[128];
    char score[128];
    char points[128];
    char title[128];
    char best[128];
    char suffix[128];
    char label[128];
    char time[128];
    switch (event->field_0x08) {
    case 5: {
        if (g_UnknownGlobal56e26c->ui->field_0x40)
            UnknownFunction46ebf0("ButViewReplay", 1)->UnknownFunction470660(1, 1);
        else
            UnknownFunction46ebf0("ButViewReplay", 1)->UnknownFunction470660(0, 1);
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
            UnknownGameUiControl* text = UnknownFunction46ebf0("TitleText", 0xc);
            g_UnknownGlobal56e26c->UnknownFunction521970(0x93b, position, 0x80);
            sprintf(title, "%s (%d/%d)", position, g_UnknownGlobal56e26c->eventManager->field_0x48,
                    g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c + 1);
            text->UnknownFunction470b20(title);
            if (g_UnknownGlobal56e26c->eventManager->field_0x48 > g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c) {
                text->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x93a);
                g_UnknownGlobal56e26c->mode.field_0x10ec = 0;
                g_UnknownGlobal56e26c->ui->field_0x3c = 0x10e;
            }
        }
        UnknownGameUiControl* first = UnknownFunction46ebf0("LstStats1", 0);
        UnknownFunction470000(first, 0, 0);
        UnknownFunction44ca80(event);
        int i;
        UIListBox* list;
        switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
        case 1:
        case 2:
        case 3:
        case 5: {
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2)
                UnknownFunction44c930(event, 0x934, 0x935, 0, 0x938);
            else
                UnknownFunction44c930(event, 0x934, 0x935, 0, 0);
            g_UnknownGlobal56e26c->UnknownFunction521970(0x936, label, 0x80);
            g_UnknownGlobal56e26c->UnknownFunction521970(0x13b8, suffix, 0x80);
            sprintf(title, "%s (%s)", label, suffix);
            UnknownGameUiControl* button = UnknownFunction46ebf0("ButStats3", 0);
            button->UnknownFunction470b20(title);
            for (i = 0; i < g_UnknownGlobal56e26c->eventManager->field_0x4c; i++) {
                sprintf(position, "%d", g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x04);
                UnknownFunction518690(time, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x08);
                if (g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x20)
                    UnknownFunction518690(best, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x14);
                else
                    g_UnknownGlobal56e26c->UnknownFunction521970(0xff0, best, 0x80);
                sprintf(score, "%s (%s)", time, best);
                if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
                    sprintf(points, "%d", g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x28);
                    UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, position,
                                          score, points);
                } else {
                    UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, position,
                                          score, 0);
                }
            }
            UIListBox* stats = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats4", 0));
            static_cast<UIButton*>(UnknownFunction46ebf0("ButStats4", 1))->UnknownFunction473390(stats);
            stats->UnknownFunction4777f0(UnknownFunction44b0e0);
            list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats2", 0));
            static_cast<UIButton*>(UnknownFunction46ebf0("ButStats2", 1))->UnknownFunction473390(list);
            list->UnknownFunction4777f0(UnknownFunction44b0a0);
            list->UnknownFunction477900(1);
            break;
        }
        case 0: {
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2)
                UnknownFunction44c930(event, 0x934, 0, 0x937, 0x938);
            else
                UnknownFunction44c930(event, 0x934, 0, 0x937, 0);
            for (i = 0; i < g_UnknownGlobal56e26c->eventManager->field_0x4c; i++) {
                sprintf(position, "%d", (int)g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x2c);
                if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
                    sprintf(score, "%d", g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x28);
                    UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, 0, position, score);
                } else {
                    UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, 0, position, 0);
                }
            }
            UIListBox* stats = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats4", 0));
            static_cast<UIButton*>(UnknownFunction46ebf0("ButStats4", 1))->UnknownFunction473390(stats);
            stats->UnknownFunction4777f0(UnknownFunction44b0e0);
            list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats3", 0));
            list->UnknownFunction4777f0(UnknownFunction44b0e0);
            list->UnknownFunction477900(1);
            break;
        }
        }
        break;
    }
    case 1:
        if (!_stricmp("ButViewReplay", event->field_0x04)) {
            UnknownReplayTapeHeader header;
            FILE* file = fopen("VCRtape.dat", "rb");
            if (file) {
                fread(&header, sizeof(header), 1, file);
                fclose(file);
                g_UnknownGlobal56e26c->mode.field_0x27f8 = header.field_0x34;
                g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04);
                g_UnknownGlobal56e26c->field_0x3428 = 1;
                g_UnknownGlobal56e26c->field_0x342c = 1;
                UNKNOWN_GARAGE_SETTINGS->field_0x00 = header.field_0x33c;
                UNKNOWN_GARAGE_SETTINGS->field_0x04 = header.field_0x340;
                COPY_TEXT(g_UnknownGlobal56e26c->mode.field_0x26f4, "VCRtape.dat", 0x104);
                UnknownFunction4536e0();
                field_0x2c->UnknownVirtualSlot26();
            } else {
                event->field_0x14->UnknownFunction470660(0, 1);
            }
        }
        break;
    }
}

// 0x004513a0: fills the result lists (by event type) when the page opens.
void MPRaceInfoDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char position[128];
    char score[128];
    char points[128];
    char title[128];
    char best[128];
    char suffix[128];
    char label[128];
    char time[128];
    if (event->field_0x08 != 5)
        return;
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
        UnknownGameUiControl* text = UnknownFunction46ebf0("TitleText", 0xc);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x93b, position, 0x80);
        sprintf(title, "%s (%d/%d)", position, g_UnknownGlobal56e26c->eventManager->field_0x48,
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c + 1);
        text->UnknownFunction470b20(title);
        if (g_UnknownGlobal56e26c->eventManager->field_0x48 > g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c) {
            text->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x93a);
            g_UnknownGlobal56e26c->mode.field_0x10ec = 0;
            g_UnknownGlobal56e26c->ui->field_0x3c = 0x10e;
        }
    }
    UnknownGameUiControl* first = UnknownFunction46ebf0("LstStats1", 0);
    UnknownFunction470000(first, 0, 0);
    UnknownFunction44ca80(event);
    int i;
    UIListBox* list;
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 1:
    case 2:
    case 3:
    case 5: {
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2)
            UnknownFunction44c930(event, 0x934, 0x935, 0, 0x938);
        else
            UnknownFunction44c930(event, 0x934, 0x935, 0, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x936, label, 0x80);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13b8, suffix, 0x80);
        sprintf(title, "%s (%s)", label, suffix);
        UnknownGameUiControl* button = UnknownFunction46ebf0("ButStats3", 0);
        button->UnknownFunction470b20(title);
        for (i = 0; i < g_UnknownGlobal56e26c->eventManager->field_0x4c; i++) {
            if (!g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x24) {
                sprintf(position, "%d", g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x04);
                UnknownFunction518690(time, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x08);
                if (g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x20)
                    UnknownFunction518690(best, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x14);
                else
                    g_UnknownGlobal56e26c->UnknownFunction521970(0xff0, best, 0x80);
                sprintf(score, "%s (%s)", time, best);
                if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
                    sprintf(points, "%d", g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x28);
                    UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, position, score,
                                          points);
                } else {
                    UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, position, score, 0);
                }
            }
        }
        UIListBox* stats = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats4", 0));
        static_cast<UIButton*>(UnknownFunction46ebf0("ButStats4", 1))->UnknownFunction473390(stats);
        stats->UnknownFunction4777f0(UnknownFunction44b0e0);
        list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats2", 0));
        list->UnknownFunction4777f0(UnknownFunction44b0a0);
        list->UnknownFunction477900(1);
        break;
    }
    case 0: {
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2)
            UnknownFunction44c930(event, 0x934, 0, 0x937, 0x938);
        else
            UnknownFunction44c930(event, 0x934, 0, 0x937, 0);
        for (i = 0; i < g_UnknownGlobal56e26c->eventManager->field_0x4c; i++) {
            if (!g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x24) {
                sprintf(position, "%d", (int)g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x2c);
                if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
                    sprintf(score, "%d", g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x28);
                    UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, 0, position, score);
                } else {
                    UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, 0, position, 0);
                }
            }
        }
        UIListBox* stats = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats4", 0));
        static_cast<UIButton*>(UnknownFunction46ebf0("ButStats4", 1))->UnknownFunction473390(stats);
        stats->UnknownFunction4777f0(UnknownFunction44b0e0);
        list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats3", 0));
        list->UnknownFunction4777f0(UnknownFunction44b0e0);
        list->UnknownFunction477900(1);
        break;
    }
    case 4:
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x148) {
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2)
            UnknownFunction44c930(event, 0x934, 0, 0x937, 0x938);
        else
            UnknownFunction44c930(event, 0x934, 0, 0x937, 0);
            for (i = 0; i < g_UnknownGlobal56e26c->eventManager->field_0x4c; i++) {
                if (!g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x24) {
                    sprintf(position, "%d", (int)g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x2c);
                    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
                        sprintf(score, "%d", g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x28);
                        UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, 0, position, score);
                    } else {
                        UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, 0, position, 0);
                    }
                }
            }
            UIListBox* stats = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats4", 0));
            static_cast<UIButton*>(UnknownFunction46ebf0("ButStats4", 1))->UnknownFunction473390(stats);
            stats->UnknownFunction4777f0(UnknownFunction44b0e0);
            list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats3", 0));
            list->UnknownFunction4777f0(UnknownFunction44b0e0);
            list->UnknownFunction477900(1);
        } else {
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
                UnknownFunction44c930(event, 0x934, 0x939, 0x938, 0);
            } else {
                g_UnknownGlobal56e26c->UnknownFunction521970(0x934, position, 0x80);
                g_UnknownGlobal56e26c->UnknownFunction521970(0x935, score, 0x80);
                g_UnknownGlobal56e26c->UnknownFunction521970(0x939, points, 0x80);
                g_UnknownGlobal56e26c->UnknownFunction521970(0x142b, title, 0x80);
                UnknownFunction44c9f0(event, position, score, points, title);
            }
            for (i = 0; i < g_UnknownGlobal56e26c->eventManager->field_0x4c; i++) {
                if (!g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x24) {
                    sprintf(position, "%d", g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x04);
                    UnknownFunction518690(score, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x1c);
                    UnknownFunction518690(points, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x18);
                    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
                        sprintf(points, "%d", g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x28);
                        UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, position, score,
                                              points);
                    } else {
                        UnknownFunction44cae0(event, g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x40, position, score,
                                              points);
                    }
                }
            }
            list = static_cast<UIListBox*>(UnknownFunction46ebf0("LstStats2", 0));
            list->UnknownFunction4777f0(UnknownFunction44b0a0);
            list->UnknownFunction477900(1);
        }
        break;
    }
}
