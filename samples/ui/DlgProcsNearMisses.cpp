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
// SPBikeRiderDlg::FillBikeRiderLists (0x0044f750, 509 bytes): the same
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
        CameraRect* preview = &previewArea;
        RECT area;
        area.left = preview->left + 50;
        area.top = preview->top + 30;
        area.right = preview->right - 50;
        area.bottom = preview->bottom - 30;
        if (PtInRect(&area, guiUser->pointerDevice->pointerPosition)) {
            previewDragged = 1;
            previewDragging = 1;
            static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->SetCurrentState(0);
        }
    }
    return UIDialog::UnknownVirtualSlot23(event, entry);
}

// 0x0044f750
void SPBikeRiderDlg::FillBikeRiderLists() {
    int bike = 0;
    int rider = 0;
    char text[0x80];
    SPBikeRiderDlg* self = this;
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    list->RemoveAllRows();
    for (int i = 0; i < g_TrackGame->ui->field_0x54; i++) {
        UnknownKrustyUIBike* entry = &((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[i];
        if (!entry->field_0x88) {
            int kind = ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[entry->model].modelKind;
            if (kind != 2 && kind != 3 && (kind != 10 || !(g_TrackGame->mode.field_0x1970 & 1)))
                continue;
        }
        sprintf(text, "%s %s", ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[entry->model].displayName,
                entry->displayName);
        list->AddRow(text, i, 0);
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
        UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[j];
        if (model->modelKind == 2 || model->modelKind == 3 || model->field_0xc0) {
            list->AddRow(model->displayName, j, 0);
            if (!strcmp(((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[j].modelName,
                        g_TrackGame->mode.field_0x1974.field_0x80))
                rider = j;
        }
    }
    list->SelectRowByData(rider);
}

// 0x00452930
void UnknownFunction452930(int menu, UnknownDialogEvent* event) {
    void* address;
    unsigned long size;
    char text[128];
    switch (menu) {
    case 0xc9:
        event->dialog->UpdateBoundValues(1);
        *UnknownGameSettingsOf(g_TrackGame) = g_UnknownGlobal688898;
        g_TrackGame->ui->field_0x2c->field_0x0ec = UnknownGameSettingsOf(g_TrackGame)->uiAnimations;
        g_TrackGame->ui->OpenMenu(0x66);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xca:
        event->dialog->UpdateBoundValues(1);
        *UnknownSoundSettingsOf(g_TrackGame) = g_UnknownGlobal688798;
        g_TrackGame->UnknownFunction521a30();
        g_TrackGame->ui->OpenMenu(0x66);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xcb: {
        event->dialog->UpdateBoundValues(1);
        UIListBox* modes = static_cast<UIDropDownList*>(event->dialog->FindControl("ModeDDL", 0))->listPart;
        g_UnknownGlobal6887d8.resolutionRow = modes->GetRowData(-1);
        *UnknownGraphicsSettingsOf(g_TrackGame) = g_UnknownGlobal6887d8;
        g_TrackGame->ui->OpenMenu(0x66);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    }
    case 0xcd:
        if (strcmp(g_TrackGame->mode.field_0x27f8.field_0x36, "")) {
            g_TrackGame->ui->OpenMenu(0xd0);
            event->dialog->EndDialog(0);
            event->handled = 1;
        }
        break;
    case 0xd0:
        g_TrackGame->ui->OpenMenu(0x68);
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xd8: {
        UnknownComPortAddress serial;
        serial.port = 0;
        serial.baudRate = 0;
        serial.stopBits = 0;
        serial.parity = 0;
        serial.flowControl = 0;
        UnknownFunction4ae410(&g_SerialSettings, &serial);
        if (!g_TrackGame->network->CreateAddress(DPSPGUID_SERIAL, "", "", &serial, &address,
                                                                     &size)) {
            event->dialog->EndDialog(0xd);
            event->handled = 1;
            break;
        }
        if (g_TrackGame->network->InitializeConnection(address, size, 8)) {
            DebugFree(address, __FILE__, 3843);
            g_TrackGame->ui->OpenMenu(0x85c);
            return;
        }
        event->dialog->EndDialog(0xd);
        event->handled = 1;
        DebugFree(address, __FILE__, 3850);
        break;
    }
    case 0xd9:
        address = 0;
        static_cast<UIEditBox*>(event->dialog->FindControl("EditBox", 0))->GetEditText(text, 128);
        if (!strcmp(text, "")) {
            event->dialog->EndDialog(0xe);
            event->handled = 1;
            break;
        }
        if (!g_TrackGame->network->CreateAddress(DPSPGUID_TCPIP, text, "", 0, &address, &size)) {
            event->dialog->EndDialog(0xd);
            event->handled = 1;
            break;
        }
        if (g_TrackGame->network->InitializeConnection(address, size, 4)) {
            event->dialog->EndDialog(0xf);
            event->handled = 1;
            DebugFree(address, __FILE__, 3874);
            break;
        }
        event->dialog->EndDialog(0xd);
        event->handled = 1;
        DebugFree(address, __FILE__, 3879);
        break;
    case 0x104:
    case 0x105:
        g_TrackGame->ui->field_0x30 = g_TrackGame->ui->field_0x3c;
        switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
        case 0:
        case 2:
            event->dialog->EndDialog(0);
            event->handled = 1;
            break;
        case 3:
            g_TrackGame->ui->OpenMenu(0xcd);
            break;
        }
        break;
    case 0x12c:
        static_cast<UIEditBox*>(event->dialog->FindControl("EditBox", 0))
            ->GetEditText(g_TrackGame->mode.field_0x10, 128);
        if (!strcmp(g_TrackGame->mode.field_0x10, "")) {
            char computer[16];
            size = sizeof(computer);
            GetComputerNameA(computer, &size);
            COPY_TEXT(g_TrackGame->mode.field_0x10, computer, 128);
            if (!strcmp(g_TrackGame->mode.field_0x10, "")) {
                g_TrackGame->LoadResourceString(0xbd8, text, 128);
                COPY_TEXT(g_TrackGame->mode.field_0x10, text, 128);
            }
        }
        event->dialog->EndDialog(0x14);
        event->handled = 1;
        break;
    case 0x12f:
        event->dialog->EndDialog(0x32);
        event->handled = 1;
        break;
    case 0x191:
        g_TrackGame->SetMenuOpen(0, 0x191, 1);
        break;
    case 0x1f9:
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0x1fa:
        event->dialog->EndDialog(0);
        event->handled = 1;
        break;
    case 0xbb9:
    case 0xbba:
    case 0xbbb:
        CreateProfile(event);
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
    switch (event->kind) {
    case kDialogInit:
        menuSingleAnimation = FindControl("MenuSingleAnimation", 5);
        menuMultiAnimation = FindControl("MenuMultiAnimation", 5);
        menuUserAnimation = FindControl("MenuUserAnimation", 5);
        menuSingleAnimation->Show(0, 0);
        menuMultiAnimation->Show(0, 0);
        menuUserAnimation->Show(0, 0);
        field_0x7f6c = 0;
        field_0x7f70 = 0;
        field_0x7f74 = 0;
        field_0x7f7c = 0.0f;
        g_TrackGame->ui->field_0x4b0 = 0;
        g_TrackGame->mode.field_0x10ec = 0;
        FillProfileList();
        ShowCurrentProfile();
        if (g_TrackGame->profileDirectory->count >= 1 && g_TrackGame->ui->field_0x48c) {
            if (event->field_0x18 == 0x63) {
                PlayerRemovedDlg* dialog = new(__FILE__, 168) PlayerRemovedDlg;
                g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0x1fa, 0xc, 0, (int)this, 0, 0, 1);
            }
        } else {
            g_TrackGame->ui->OpenMenu(0xbba);
        }
        if (g_TrackGame->field_0x3444) {
            delete g_TrackGame->field_0x3444;
            g_TrackGame->field_0x3444 = 0;
        }
        FindControl("MenuReplay", 0)->UnknownVirtualSlot49(0);
        FindControl("MenuHighScore", 0)->UnknownVirtualSlot49(0);
        FindControl("MenuCredits", 0)->UnknownVirtualSlot49(0);
        FindControl("ButProCircuit", 0)->UnknownVirtualSlot49(0);
        movieControl = 0;
        break;
    case kDialogCommand:
        if (!_stricmp("MenuSingle", event->controlName)) {
            field_0x7f6c = field_0x7f6c == 0;
            menuSingleAnimation->Show(field_0x7f6c, 1);
            field_0x7f78 = 1;
            if (field_0x7f6c) {
                field_0x7f7c = 0.0f;
                if (movieControl) {
                    movieControl->field_0x1ec->UnknownVirtualSlot16(1);
                    movieControl->Show(0, 1);
                }
            } else if (movieControl) {
                movieControl->field_0x1ec->UnknownVirtualSlot16(1);
                movieControl->Show(0, 1);
            }
            if (field_0x7f70) {
                menuMultiAnimation->Show(0, 1);
                field_0x7f70 = 0;
                EndNetworkGame();
                g_TrackGame->ui->field_0x4b0 = 0;
            }
            if (field_0x7f74) {
                menuUserAnimation->Show(0, 1);
                field_0x7f74 = 0;
            }
        } else if (!_stricmp("MenuMulti", event->controlName)) {
            field_0x7f70 = field_0x7f70 == 0;
            menuMultiAnimation->Show(field_0x7f70, 1);
            if (field_0x7f6c) {
                menuSingleAnimation->Show(0, 1);
                field_0x7f6c = 0;
            }
            if (field_0x7f74) {
                menuUserAnimation->Show(0, 1);
                field_0x7f74 = 0;
            }
            if (field_0x7f70) {
                guiManager->UnknownFunction4865e0(guiManager->waitCursorImage, 1);
                FindControl("ButIPX", 1)->UnknownVirtualSlot49(0);
                FindControl("ButTCP", 1)->UnknownVirtualSlot49(0);
                FindControl("ButZone", 1)->UnknownVirtualSlot49(1);
                FindControl("ButCable", 1)->UnknownVirtualSlot49(0);
                FindControl("ButModem", 1)->UnknownVirtualSlot49(0);
                g_TrackGame->mode.field_0x2bd0.field_0x28 = 0;
                if (EnsureNetworkInterface()) {
                    ConnectionInfoType connections[5];
                    g_TrackGame->network->EnumConnections(connections, &count);
                    for (int i = 0; i < count; i++) {
                        if (!memcmp(connections[i].provider, &DPSPGUID_IPX, sizeof(GUID)))
                            FindControl("ButIPX", 1)->UnknownVirtualSlot49(1);
                        else if (!memcmp(connections[i].provider, &DPSPGUID_TCPIP, sizeof(GUID)))
                            FindControl("ButTCP", 1)->UnknownVirtualSlot49(1);
                        else if (!memcmp(connections[i].provider, &DPSPGUID_SERIAL, sizeof(GUID)))
                            FindControl("ButCable", 1)->UnknownVirtualSlot49(1);
                        else if (!memcmp(connections[i].provider, &DPSPGUID_MODEM, sizeof(GUID)))
                            FindControl("ButModem", 1)->UnknownVirtualSlot49(1);
                    }
                }
                guiManager->UnknownFunction4865e0(guiManager->cursorImage, 0);
            } else {
                EndNetworkGame();
                g_TrackGame->ui->field_0x4b0 = 0;
            }
        } else if (!_stricmp("MenuUserProfile", event->controlName) || !_stricmp("ButCancelProfile", event->controlName)) {
            field_0x7f74 = field_0x7f74 == 0;
            menuUserAnimation->Show(field_0x7f74, 1);
            if (field_0x7f70) {
                menuMultiAnimation->Show(0, 1);
                field_0x7f70 = 0;
                EndNetworkGame();
                g_TrackGame->ui->field_0x4b0 = 0;
            }
            if (field_0x7f74)
                UnknownFunction470000(FindControl("LstProfiles", 3), 0, 0);
            if (field_0x7f6c) {
                menuSingleAnimation->Show(0, 1);
                field_0x7f6c = 0;
            }
        } else if (!_stricmp("MenuOptions", event->controlName)) {
            OptionsDlg* dialog = new(__FILE__, 345) OptionsDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, (UIDialog*)this, 0, 0, 1);
        } else if (!_stricmp("ButStuntQuarry", event->controlName)) {
            if (movieControl)
                movieControl->field_0x1ec->Stop();
            g_TrackGame->mode.field_0x29e4.field_0x04 = 0;
            g_TrackGame->mode.field_0x29e4.field_0x08 = 0;
            g_TrackGame->mode.field_0x6a0 = (int)"Teraform\\Quarries";
            SinglePlayerDlg* dialog = new(__FILE__, 352) SinglePlayerDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButBaja", event->controlName)) {
            if (movieControl)
                movieControl->field_0x1ec->Stop();
            g_TrackGame->mode.field_0x29e4.field_0x04 = 1;
            g_TrackGame->mode.field_0x29e4.field_0x08 = 1;
            g_TrackGame->mode.field_0x6a0 = (int)"Teraform\\Baja";
            SinglePlayerDlg* dialog = new(__FILE__, 360) SinglePlayerDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButSupercross", event->controlName)) {
            if (movieControl)
                movieControl->field_0x1ec->Stop();
            g_TrackGame->mode.field_0x29e4.field_0x04 = 3;
            g_TrackGame->mode.field_0x29e4.field_0x08 = 3;
            g_TrackGame->mode.field_0x6a0 = (int)"Teraform\\SX";
            SinglePlayerDlg* dialog = new(__FILE__, 368) SinglePlayerDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButEnduro", event->controlName)) {
            if (movieControl)
                movieControl->field_0x1ec->Stop();
            g_TrackGame->mode.field_0x29e4.field_0x04 = 5;
            g_TrackGame->mode.field_0x29e4.field_0x08 = 5;
            g_TrackGame->mode.field_0x6a0 = (int)"Teraform\\Enduro";
            SinglePlayerDlg* dialog = new(__FILE__, 376) SinglePlayerDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButNationals", event->controlName)) {
            if (movieControl)
                movieControl->field_0x1ec->Stop();
            g_TrackGame->mode.field_0x29e4.field_0x04 = 2;
            g_TrackGame->mode.field_0x29e4.field_0x08 = 2;
            g_TrackGame->mode.field_0x6a0 = (int)"Teraform\\National";
            SinglePlayerDlg* dialog = new(__FILE__, 384) SinglePlayerDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButProCircuit", event->controlName)) {
            if (movieControl)
                movieControl->field_0x1ec->Stop();
            PCStartupDlg* dialog = new(__FILE__, 389) PCStartupDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButLoadProfile", event->controlName)) {
            g_TrackGame->mode.UnknownFunction523580();
            if (UseSelectedProfile(event))
                g_TrackGame->UnknownFunction521a30();
            UnknownFunction470000(FindControl("LstProfiles", 3), 0, 0);
            ShowCurrentProfile();
            field_0x7f74 = 0;
            menuUserAnimation->Show(0, 1);
        } else if (!_stricmp("ButNewProfile", event->controlName)) {
            g_TrackGame->mode.UnknownFunction523580();
            g_TrackGame->ui->OpenMenu(0xbb9);
            UnknownFunction470000(FindControl("LstProfiles", 3), 0, 0);
            FillProfileList();
            ShowCurrentProfile();
            field_0x7f74 = 0;
            menuUserAnimation->Show(0, 1);
        } else if (!_stricmp("ButRemoveProfile", event->controlName)) {
            UIListBox* list = static_cast<UIListBox*>(FindControl("LstProfiles", 0));
            short row = list->GetSelectedRow();
            if (!_stricmp(list->GetRowText(-1), g_TrackGame->mode.field_0x00)) {
                NoDelCurProfileDlg* dialog = new(__FILE__, 423) NoDelCurProfileDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
            } else if (row != -1) {
                g_UnknownGlobal59adbc = row;
                RemoveProfileDlg* dialog = new(__FILE__, 426) RemoveProfileDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0x12f, 4, 0, (UIDialog*)this,
                                                  (int)list->GetRowText(-1), 0, 1);
            }
            UnknownFunction470000(list, 0, 0);
            FillProfileList();
        } else if (!_stricmp("ButIPX", event->controlName)) {
            g_TrackGame->ui->field_0x4b0 = 1;
            if (g_TrackGame->network->CreateAddress(DPSPGUID_IPX, "", "", 0, &address, &size) &&
                g_TrackGame->network->InitializeConnection(address, size, 1)) {
                DebugFree(address, __FILE__, 443);
                g_TrackGame->ui->OpenMenu(0x85c);
            }
        } else if (!_stricmp("ButTCP", event->controlName)) {
            g_TrackGame->ui->field_0x4b0 = 1;
            ChooseTCPMethodDlg* dialog = new(__FILE__, 453) ChooseTCPMethodDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
        } else if (!_stricmp("ButZone", event->controlName)) {
            g_TrackGame->openLocalizedWebPageOnExit = 1;
            event->dialog->EndDialog(0);
            event->handled = 1;
            g_TrackGame->ui->OpenExitDialog();
        } else if (!_stricmp("ButCable", event->controlName)) {
            g_TrackGame->ui->field_0x4b0 = 1;
            g_TrackGame->ui->OpenMenu(0xd8);
        } else if (!_stricmp("ButModem", event->controlName)) {
            g_TrackGame->ui->field_0x4b0 = 1;
            ConnectionInfoType connections[5];
            g_TrackGame->network->EnumConnections(connections, &count);
            for (int i = 0; i < count; i++) {
                if (!memcmp(connections[i].provider, &DPSPGUID_MODEM, sizeof(GUID))) {
                    g_TrackGame->network->directPlay->InitializeConnection(connections[i].connection, 0);
                    g_TrackGame->ui->OpenMenu(0xfc);
                    break;
                }
            }
        } else if (!_stricmp("MenuReplay", event->controlName)) {
            GhostReplayDlg* dialog = new(__FILE__, 487) GhostReplayDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        } else if (!_stricmp("MenuHighScore", event->controlName)) {
            TrackRecordDlg* dialog = new(__FILE__, 492) TrackRecordDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0x911, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        } else if (!_stricmp("MenuCredits", event->controlName)) {
            CreditsVidDlg* dialog = new(__FILE__, 501) CreditsVidDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        } else if (!_stricmp("MenuHelp", event->controlName)) {
            g_TrackGame->mode.OpenHelp("MCM2HELP", 0);
        } else if (!_stricmp("MenuExit", event->controlName)) {
            EndDialog(0);
            g_TrackGame->ui->OpenExitDialog();
        }
        break;
    case 9:
        if (event->code == 0x1e || event->code == 0x32) {
            UIListBox* list = static_cast<UIListBox*>(FindControl("LstProfiles", 0));
            if (event->code == 0x1e) {
                list->AddRow(g_TrackGame->mode.field_0x00, 0, 0);
                list->Sort(1);
                list->SelectRowByText(g_TrackGame->mode.field_0x00);
            } else if (event->code == 0x32) {
                COPY_TEXT(name, list->GetRowText(g_UnknownGlobal59adbc), 16);
                sprintf(path, "%s\\%s", "ui\\profile", name);
                if (g_TrackGame->profileDirectory->UnknownFunction44a960(path)) {
                    g_TrackGame->profileDirectory->UnknownVirtualSlot1();
                    list->RemoveRow(g_UnknownGlobal59adbc);
                }
            }
            FillProfileList();
            ShowCurrentProfile();
            if (field_0x7f74)
                UnknownFunction470000(list, 0, 0);
        }
        if (event->code == 0x1f)
            ShowCurrentProfile();
        if (event->code == 0x63) {
            event->dialog->EndDialog(0);
            event->handled = 1;
        }
        if (event->code == 0x3d)
            g_TrackGame->ui->OpenMenu(0x85d);
        if (event->code == 0x3c) {
            if (g_TrackGame->network->CreateAddress(DPSPGUID_TCPIP, "", "", 0, &address, &size) &&
                g_TrackGame->network->InitializeConnection(address, size, 2)) {
                DebugFree(address, __FILE__, 596);
                g_TrackGame->ui->OpenMenu(0x85c);
            }
        }
        break;
    case kDialogClose:
        field_0x7f74 = 0;
        field_0x7f70 = 0;
        field_0x7f6c = 0;
        menuUserAnimation = 0;
        menuMultiAnimation = 0;
        menuSingleAnimation = 0;
        break;
    }
}

// 0x0044fae0: turns the bike view (dragged with the pointer, else slowly by
// itself), animates the previews and now and then plays a rider idle motion.
int SPBikeRiderDlg::UnknownVirtualSlot10(float frameTime) {
    if (dialogBackground)
        dialogBackground->UnknownFunction404da0();
    Vector3 offset;
    Vector3 turned;
    GUIInputDevice* pointer = guiUser->pointerDevice;
    if (pointer && previewDragged && previewDragging) {
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
        previewDragged = 1;
        static_cast<UIMultiState*>(FindControl("ChkAutoRotate", 2))->SetCurrentState(0);
    } else if (!previewDragged) {
        offset = UnknownVectorDifference(viewEye, g_TrackGame->ui->field_0x474);
        D3DRMVectorRotate(&turned, &offset, (Vector3*)&kVec3YAxis, frameTime * 0.39269909f);
        turned *= viewDistance;
        viewEye = UnknownVectorSum(turned, g_TrackGame->ui->field_0x474);
        g_TrackGame->ui->field_0x468->UnknownFunction42e9b0(&viewEye, 0, 0, 0, 0);
        g_TrackGame->ui->field_0x468->UnknownVirtualSlot29(viewTarget);
    }
    g_TrackGame->ui->field_0x46c->ModelVirtualSlot7(frameTime, 0, 0);
    g_TrackGame->ui->field_0x470->ModelVirtualSlot7(frameTime, 0, 0);
    if (idleMotionPlaying && rand() % 200 == 1) {
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
        idleMotionPlaying = 0;
    }
    if (!idleMotionPlaying && g_TrackGame->ui->field_0x46c->motionFinished) {
        UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
        bikes->GetRowData(-1);
        g_TrackGame->ui->field_0x46c->UnknownFunction4a8b10("WaitR");
        idleMotionPlaying = 1;
    }
    if (dialogBackground)
        dialogBackground->UnknownFunction404240(previewRegion, &previewArea);
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
    switch (event->kind) {
    case kDialogInit: {
        if (g_TrackGame->ui->field_0x40)
            FindControl("ButViewReplay", 1)->Show(1, 1);
        else
            FindControl("ButViewReplay", 1)->Show(0, 1);
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
            UIControl* text = FindControl("TitleText", 0xc);
            g_TrackGame->LoadResourceString(0x93b, position, 0x80);
            sprintf(title, "%s (%d/%d)", position, g_TrackGame->eventManager->field_0x48,
                    g_TrackGame->mode.field_0x27f8.field_0x0c + 1);
            text->SetText(title);
            if (g_TrackGame->eventManager->field_0x48 > g_TrackGame->mode.field_0x27f8.field_0x0c) {
                text->SetTextFromResource(g_TrackGame->resourceInstance, 0x93a);
                g_TrackGame->mode.field_0x10ec = 0;
                g_TrackGame->ui->field_0x3c = 0x10e;
            }
        }
        UIControl* first = FindControl("LstStats1", 0);
        UnknownFunction470000(first, 0, 0);
        ClearStatsLists(event);
        int i;
        UIListBox* list;
        switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
        case 1:
        case 2:
        case 3:
        case 5: {
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2)
                SetStatsButtonResources(event, 0x934, 0x935, 0, 0x938);
            else
                SetStatsButtonResources(event, 0x934, 0x935, 0, 0);
            g_TrackGame->LoadResourceString(0x936, label, 0x80);
            g_TrackGame->LoadResourceString(0x13b8, suffix, 0x80);
            sprintf(title, "%s (%s)", label, suffix);
            UIControl* button = FindControl("ButStats3", 0);
            button->SetText(title);
            for (i = 0; i < g_TrackGame->eventManager->field_0x4c; i++) {
                sprintf(position, "%d", g_TrackGame->eventManager->field_0x50[i].field_0x04);
                UnknownFunction518690(time, g_TrackGame->eventManager->field_0x50[i].field_0x08);
                if (g_TrackGame->eventManager->field_0x50[i].field_0x20)
                    UnknownFunction518690(best, g_TrackGame->eventManager->field_0x50[i].field_0x14);
                else
                    g_TrackGame->LoadResourceString(0xff0, best, 0x80);
                sprintf(score, "%s (%s)", time, best);
                if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
                    sprintf(points, "%d", g_TrackGame->eventManager->field_0x50[i].field_0x28);
                    AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, position,
                                 score, points);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, position,
                                 score, 0);
                }
            }
            UIListBox* stats = static_cast<UIListBox*>(FindControl("LstStats4", 0));
            static_cast<UIButton*>(FindControl("ButStats4", 1))->SetSortList(stats);
            stats->SetSortCompare(CompareRowNumbersDescending);
            list = static_cast<UIListBox*>(FindControl("LstStats2", 0));
            static_cast<UIButton*>(FindControl("ButStats2", 1))->SetSortList(list);
            list->SetSortCompare(CompareRowNumbersAscending);
            list->Sort(1);
            break;
        }
        case 0: {
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2)
                SetStatsButtonResources(event, 0x934, 0, 0x937, 0x938);
            else
                SetStatsButtonResources(event, 0x934, 0, 0x937, 0);
            for (i = 0; i < g_TrackGame->eventManager->field_0x4c; i++) {
                sprintf(position, "%d", (int)g_TrackGame->eventManager->field_0x50[i].field_0x2c);
                if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
                    sprintf(score, "%d", g_TrackGame->eventManager->field_0x50[i].field_0x28);
                    AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, 0, position, score);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, 0, position, 0);
                }
            }
            UIListBox* stats = static_cast<UIListBox*>(FindControl("LstStats4", 0));
            static_cast<UIButton*>(FindControl("ButStats4", 1))->SetSortList(stats);
            stats->SetSortCompare(CompareRowNumbersDescending);
            list = static_cast<UIListBox*>(FindControl("LstStats3", 0));
            list->SetSortCompare(CompareRowNumbersDescending);
            list->Sort(1);
            break;
        }
        }
        break;
    }
    case kDialogCommand:
        if (!_stricmp("ButViewReplay", event->controlName)) {
            UnknownReplayTapeHeader header;
            FILE* file = fopen("VCRtape.dat", "rb");
            if (file) {
                fread(&header, sizeof(header), 1, file);
                fclose(file);
                g_TrackGame->mode.field_0x27f8 = header.field_0x34;
                g_TrackGame->mode.UnknownFunction5240e0(g_TrackGame->mode.field_0x27f8.field_0x04);
                g_TrackGame->field_0x3428 = 1;
                g_TrackGame->field_0x342c = 1;
                UNKNOWN_GARAGE_SETTINGS->engineSize = header.field_0x33c;
                UNKNOWN_GARAGE_SETTINGS->field_0x04 = header.field_0x340;
                COPY_TEXT(g_TrackGame->mode.field_0x26f4, "VCRtape.dat", 0x104);
                UnknownFunction4536e0();
                parentDialog->UnknownVirtualSlot26();
            } else {
                event->control->Show(0, 1);
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
    if (event->kind != kDialogInit)
        return;
    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
        UIControl* text = FindControl("TitleText", 0xc);
        g_TrackGame->LoadResourceString(0x93b, position, 0x80);
        sprintf(title, "%s (%d/%d)", position, g_TrackGame->eventManager->field_0x48,
                g_TrackGame->mode.field_0x27f8.field_0x0c + 1);
        text->SetText(title);
        if (g_TrackGame->eventManager->field_0x48 > g_TrackGame->mode.field_0x27f8.field_0x0c) {
            text->SetTextFromResource(g_TrackGame->resourceInstance, 0x93a);
            g_TrackGame->mode.field_0x10ec = 0;
            g_TrackGame->ui->field_0x3c = 0x10e;
        }
    }
    UIControl* first = FindControl("LstStats1", 0);
    UnknownFunction470000(first, 0, 0);
    ClearStatsLists(event);
    int i;
    UIListBox* list;
    switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
    case 1:
    case 2:
    case 3:
    case 5: {
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2)
            SetStatsButtonResources(event, 0x934, 0x935, 0, 0x938);
        else
            SetStatsButtonResources(event, 0x934, 0x935, 0, 0);
        g_TrackGame->LoadResourceString(0x936, label, 0x80);
        g_TrackGame->LoadResourceString(0x13b8, suffix, 0x80);
        sprintf(title, "%s (%s)", label, suffix);
        UIControl* button = FindControl("ButStats3", 0);
        button->SetText(title);
        for (i = 0; i < g_TrackGame->eventManager->field_0x4c; i++) {
            if (!g_TrackGame->eventManager->field_0x50[i].field_0x24) {
                sprintf(position, "%d", g_TrackGame->eventManager->field_0x50[i].field_0x04);
                UnknownFunction518690(time, g_TrackGame->eventManager->field_0x50[i].field_0x08);
                if (g_TrackGame->eventManager->field_0x50[i].field_0x20)
                    UnknownFunction518690(best, g_TrackGame->eventManager->field_0x50[i].field_0x14);
                else
                    g_TrackGame->LoadResourceString(0xff0, best, 0x80);
                sprintf(score, "%s (%s)", time, best);
                if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
                    sprintf(points, "%d", g_TrackGame->eventManager->field_0x50[i].field_0x28);
                    AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, position, score,
                                 points);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, position, score, 0);
                }
            }
        }
        UIListBox* stats = static_cast<UIListBox*>(FindControl("LstStats4", 0));
        static_cast<UIButton*>(FindControl("ButStats4", 1))->SetSortList(stats);
        stats->SetSortCompare(CompareRowNumbersDescending);
        list = static_cast<UIListBox*>(FindControl("LstStats2", 0));
        list->SetSortCompare(CompareRowNumbersAscending);
        list->Sort(1);
        break;
    }
    case 0: {
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2)
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0x938);
        else
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0);
        for (i = 0; i < g_TrackGame->eventManager->field_0x4c; i++) {
            if (!g_TrackGame->eventManager->field_0x50[i].field_0x24) {
                sprintf(position, "%d", (int)g_TrackGame->eventManager->field_0x50[i].field_0x2c);
                if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
                    sprintf(score, "%d", g_TrackGame->eventManager->field_0x50[i].field_0x28);
                    AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, 0, position, score);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, 0, position, 0);
                }
            }
        }
        UIListBox* stats = static_cast<UIListBox*>(FindControl("LstStats4", 0));
        static_cast<UIButton*>(FindControl("ButStats4", 1))->SetSortList(stats);
        stats->SetSortCompare(CompareRowNumbersDescending);
        list = static_cast<UIListBox*>(FindControl("LstStats3", 0));
        list->SetSortCompare(CompareRowNumbersDescending);
        list->Sort(1);
        break;
    }
    case 4:
        if (g_TrackGame->mode.field_0x27f8.field_0x148) {
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2)
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0x938);
        else
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0);
            for (i = 0; i < g_TrackGame->eventManager->field_0x4c; i++) {
                if (!g_TrackGame->eventManager->field_0x50[i].field_0x24) {
                    sprintf(position, "%d", (int)g_TrackGame->eventManager->field_0x50[i].field_0x2c);
                    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
                        sprintf(score, "%d", g_TrackGame->eventManager->field_0x50[i].field_0x28);
                        AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, 0, position, score);
                    } else {
                        AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, 0, position, 0);
                    }
                }
            }
            UIListBox* stats = static_cast<UIListBox*>(FindControl("LstStats4", 0));
            static_cast<UIButton*>(FindControl("ButStats4", 1))->SetSortList(stats);
            stats->SetSortCompare(CompareRowNumbersDescending);
            list = static_cast<UIListBox*>(FindControl("LstStats3", 0));
            list->SetSortCompare(CompareRowNumbersDescending);
            list->Sort(1);
        } else {
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
                SetStatsButtonResources(event, 0x934, 0x939, 0x938, 0);
            } else {
                g_TrackGame->LoadResourceString(0x934, position, 0x80);
                g_TrackGame->LoadResourceString(0x935, score, 0x80);
                g_TrackGame->LoadResourceString(0x939, points, 0x80);
                g_TrackGame->LoadResourceString(0x142b, title, 0x80);
                SetStatsButtonTexts(event, position, score, points, title);
            }
            for (i = 0; i < g_TrackGame->eventManager->field_0x4c; i++) {
                if (!g_TrackGame->eventManager->field_0x50[i].field_0x24) {
                    sprintf(position, "%d", g_TrackGame->eventManager->field_0x50[i].field_0x04);
                    UnknownFunction518690(score, g_TrackGame->eventManager->field_0x50[i].field_0x1c);
                    UnknownFunction518690(points, g_TrackGame->eventManager->field_0x50[i].field_0x18);
                    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
                        sprintf(points, "%d", g_TrackGame->eventManager->field_0x50[i].field_0x28);
                        AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, position, score,
                                     points);
                    } else {
                        AddStatsRows(event, g_TrackGame->eventManager->field_0x50[i].field_0x40, position, score,
                                     points);
                    }
                }
            }
            list = static_cast<UIListBox*>(FindControl("LstStats2", 0));
            list->SetSortCompare(CompareRowNumbersAscending);
            list->Sort(1);
        }
        break;
    }
}
