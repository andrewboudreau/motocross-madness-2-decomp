// Near misses for dlgprocs.cpp (canonical file:
// src/reconstructed/DlgProcs.cpp, included below for its types and its
// matched functions). Check: compile this file and compare each function with
// DlgProcsNearMisses.bindings.json.
//
// MainDlg::HandleEvent (0x0044b200, 5232 bytes): the same cases,
//   calls and strings; retail shares one frame slot between the two
//   ConnectionInfoType[5] arrays (frame 0x6c4), VC6 here gives each array its
//   own slot (frame 0xc64), so every later frame offset differs. Retail also
//   pushes the constant 1 where VC6 here keeps it in edi. One array at
//   function scope is constructed at entry (worse) and one array in a
//   braced kDialogCommand case is worse. In probes VC6 SP3 shares disjoint
//   slots for POD arrays, constructor-only arrays and scalar objects with
//   destructors, but never for two arrays whose element has a destructor
//   (sibling blocks, else-if chains, switch cases, sequential blocks, try,
//   same name, inline virtual InfoType destructor), so the source shape
//   that lets retail share them is still unknown. Retail's unwind map
//   (FuncInfo 0x00560770, 16 states) places both arrays at [ebp-0x5ac]
//   (states 1 and 0xc) and every `new` temporary at [ebp-0x6d0]. Also
//   0xc64 here: each array block moved into a static inline helper (both
//   inline, slots still separate), a wrapper struct holding the array,
//   scalar destructor objects in place of the arrays, the blocks as switch
//   cases, sequential blocks, nested depths, a loop body, a try block, the
//   helper returning early or allocating first, and the flags /Od /Ox /O1
//   /Og /Oy- /Ob0 /Ob2 /Gy /Oa /Ow /GF /Gf /Zi /Gs. Only /GX- shares the
//   slot (it drops the unwind states).
//   ButModem returns from inside its loop once the modem connection is set
//   up: retail destroys that block's array on two paths (three ??_M calls in
//   all, the found path jumping straight to the epilogue), which `break`
//   merged into one (tools/reference_diff.py, session 14). With `return`
//   both OpenExitDialog calls and every g_kbGame load stay apart as in
//   retail; aligned instructions 1293 -> 1342 (regmasked hunks 44 -> 37),
//   strict unchanged because the frame still differs.
// SPBikeRiderDlg::Tick (0x0044fae0, 1235 bytes, 1181
//   match): the same flow and x87 code; only the scheduling of the by-value
//   Vector3 copies passed to the camera's slot 29 differs (retail takes
//   `edi = esp` after the camera and vtable loads and loads z before storing
//   y). A pointer, reference, `this->`, explicit copy, cast or camera local
//   for the argument leaves it; a user-defined Vector3 copy constructor
//   copies before the camera is evaluated (worse). Also unchanged: inline
//   helpers taking the camera and the vector (const reference, by value,
//   pointer), a `Vector3(viewTarget)` temporary, parentheses, and the
//   declaration count (0..63 prepended typedefs); a `Vector3(x, y, z)`
//   temporary is much worse. In probes VC6 interleaves the copy like retail
//   only when the destination pointer lands in a scratch register; with it
//   in a callee-saved register (edi, `this` live afterwards) it takes the
//   pointer first and copies x, y, z in order, whatever the struct type.
//   VC6 gives retail's order when the callee is one load (EventManager slot
//   10 0x0045f200, `podiumCamera->UnknownVirtualSlot29(field_0x3d8)`, is
//   exact) or when the camera comes from a separate tested block, a
//   volatile local or a register source pointer, all of which add code;
//   with the two-load chain g_TrackGame->ui->garageCamera it takes edi = esp
//   first. Also 1181: compiler flags (-G3/-G5/-Ox), float[3]/union
//   arguments, `(camera = ...)->` and a function-scope camera, the target
//   declared at any scope; `eye[1]` gives retail's `add edi, 0xc` but moves
//   earlier registers (183). The same copy differs in 0x004f8820
//   (SelectGamePicProcs) and 0x004d72c0 (ProCircuitProcs). Also 1181 or
//   worse (11 forms): a comma expression, `1 ? camera : 0`, `&*camera`,
//   `camera + 0`, inline helpers taking the KrustyUI and the vector (by
//   value or const reference), an inline camera getter with or without a
//   redundant test, a `*(Vector3*)&` argument and a qualified
//   `Camera::LookAt` call.
//   Mechanism (Fable probe, 2026-10-16): the difference is VC6's Pentium
//   scheduler, whose region is the whole basic block (calls are not
//   barriers). A label (or `goto l; l:`) between the SetFrame call and the
//   slot-29 call in both branches makes all three dialogs' copies exact
//   (0x0044fae0 1235/1235). No label-free construct split the block ({},
//   if (1), do/while (0), switch, empty inline call, (void)x, dead copies),
//   so the original had some code-free block boundary there; a bare label
//   is a matching-only form and is not applied. Session 14: an inline
//   helper wrapping both calls with `goto look; look:` between them is also
//   exact (1235/1235), the same label moved into a helper, so not applied.
//   Without the label the helper (const reference or pointer target, void or
//   int result, one helper per call) gives 1181, by value 74; so do
//   branches that fold after inlining or propagation (an `int look = 1`
//   parameter or local, `static const` flag, `if (look) {}`) and
//   `for (;;) { ...; break; }`, `while (1)` and `if (LookAt(..)) {}`
//   around the slot-29 call; tests on `this`, `&viewTarget` or a target
//   pointer add code (109-523).
//   Session 14 copy forms, all 1181 unless noted: a `const Vector3&` local
//   passed by value, that reference copied into a named local first, a
//   named copy assigned after its declaration, and a `pointer ? viewTarget
//   : viewTarget` argument in the drag branch only (pointer is known there,
//   so the test is threaded away). The same ternary in both branches, or on
//   previewDragging or frameTime, keeps a real test (75-295). Repeated
//   `if (pointer)` or `if (dx == dx)` tests ahead of the slot-29 call are
//   also threaded away and leave the block unsplit (1181); an
//   `if (!previewDragged)` test on the member is reloaded (801).
//   The reference, named-copy and threaded-ternary forms also leave
//   0x004f8820 (1214) and 0x004d72c0 (1233) unchanged.
// SPRaceInfoDlg::HandleEvent (0x004507a0, 1652 bytes) and
// MPRaceInfoDlg::HandleEvent (0x004513a0, 2004 bytes): the same
//   calls, strings and list columns; inside the per-racer loops retail
//   reloads TrackGame/EventManager into other registers (VC6 here keeps them
//   live across the loop), so the loop bodies differ from the first loop on.
//   In 0x004513a0's first loop (595/2004) retail reloads the EventManager
//   for the +0x04 sprintf argument while VC6 here reuses it. Unchanged
//   (2026-10-11): `continue` on the +0x24 test, an `EventManager* const&`
//   alias, an entry reference or pointer, a TrackGame local, an `(int)`
//   cast and `(field_0x50 + i)->`.
//   Probe (2026-10-12): reading the first loop's position through
//   `(EventManager* volatile&)g_TrackGame->eventManager` gives 1659/1668
//   for 0x004507a0 (every later loop then matches); only the load's place
//   differs (retail schedules it after `lea ecx`, which a volatile load
//   does not allow), so retail's load is an ordinary one that VC6 did not
//   carry across the branch. Not adopted. Casts through int, char* + 0x570,
//   `const&`, `void**` and `results + i` stay at 589.

#include "../../src/reconstructed/DlgProcs.cpp"

// 0x0044b200
void MainDlg::HandleEvent(UnknownDialogEvent* event) {
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
        singleMenuOpen = 0;
        multiMenuOpen = 0;
        userMenuOpen = 0;
        movieRestartTime = 0.0f;
        g_TrackGame->ui->userNameDialogOpen = 0;
        g_TrackGame->mode.championshipInProgress = 0;
        FillProfileList();
        ShowCurrentProfile();
        if (g_TrackGame->profileDirectory->count >= 1 && g_TrackGame->ui->profileLoaded) {
            if (event->param == 0x63) {
                PlayerRemovedDlg* dialog = new(__FILE__, 168) PlayerRemovedDlg;
                g_TrackGame->ui->gui->ShowDialog(dialog, 0x1fa, 0xc, 0, (UIDialog*)this, 0, 0, 1);
            }
        } else {
            g_TrackGame->ui->OpenMenu(0xbba);
        }
        if (g_TrackGame->proCircuit) {
            delete g_TrackGame->proCircuit;
            g_TrackGame->proCircuit = 0;
        }
        FindControl("MenuReplay", 0)->SetEnabled(0);
        FindControl("MenuHighScore", 0)->SetEnabled(0);
        FindControl("MenuCredits", 0)->SetEnabled(0);
        FindControl("ButProCircuit", 0)->SetEnabled(0);
        movieControl = 0;
        break;
    case kDialogCommand:
        if (!_stricmp("MenuSingle", event->controlName)) {
            singleMenuOpen = singleMenuOpen == 0;
            menuSingleAnimation->Show(singleMenuOpen, 1);
            movieRestartPending = 1;
            if (singleMenuOpen) {
                movieRestartTime = 0.0f;
                if (movieControl) {
                    movieControl->moviePlayer->SetPaused(1);
                    movieControl->Show(0, 1);
                }
            } else if (movieControl) {
                movieControl->moviePlayer->SetPaused(1);
                movieControl->Show(0, 1);
            }
            if (multiMenuOpen) {
                menuMultiAnimation->Show(0, 1);
                multiMenuOpen = 0;
                EndNetworkGame();
                g_TrackGame->ui->userNameDialogOpen = 0;
            }
            if (userMenuOpen) {
                menuUserAnimation->Show(0, 1);
                userMenuOpen = 0;
            }
        } else if (!_stricmp("MenuMulti", event->controlName)) {
            multiMenuOpen = multiMenuOpen == 0;
            menuMultiAnimation->Show(multiMenuOpen, 1);
            if (singleMenuOpen) {
                menuSingleAnimation->Show(0, 1);
                singleMenuOpen = 0;
            }
            if (userMenuOpen) {
                menuUserAnimation->Show(0, 1);
                userMenuOpen = 0;
            }
            if (multiMenuOpen) {
                guiManager->RecreateCursors(guiManager->waitCursorImage, 1);
                FindControl("ButIPX", 1)->SetEnabled(0);
                FindControl("ButTCP", 1)->SetEnabled(0);
                FindControl("ButZone", 1)->SetEnabled(1);
                FindControl("ButCable", 1)->SetEnabled(0);
                FindControl("ButModem", 1)->SetEnabled(0);
                g_TrackGame->mode.multiplayerSettings.multiplayerAiRacerCount = 0;
                if (EnsureNetworkInterface()) {
                    ConnectionInfoType connections[5];
                    g_TrackGame->network->EnumConnections(connections, &count);
                    for (int i = 0; i < count; i++) {
                        if (!memcmp(connections[i].provider, &DPSPGUID_IPX, sizeof(GUID)))
                            FindControl("ButIPX", 1)->SetEnabled(1);
                        else if (!memcmp(connections[i].provider, &DPSPGUID_TCPIP, sizeof(GUID)))
                            FindControl("ButTCP", 1)->SetEnabled(1);
                        else if (!memcmp(connections[i].provider, &DPSPGUID_SERIAL, sizeof(GUID)))
                            FindControl("ButCable", 1)->SetEnabled(1);
                        else if (!memcmp(connections[i].provider, &DPSPGUID_MODEM, sizeof(GUID)))
                            FindControl("ButModem", 1)->SetEnabled(1);
                    }
                }
                guiManager->RecreateCursors(guiManager->cursorImage, 0);
            } else {
                EndNetworkGame();
                g_TrackGame->ui->userNameDialogOpen = 0;
            }
        } else if (!_stricmp("MenuUserProfile", event->controlName) || !_stricmp("ButCancelProfile", event->controlName)) {
            userMenuOpen = userMenuOpen == 0;
            menuUserAnimation->Show(userMenuOpen, 1);
            if (multiMenuOpen) {
                menuMultiAnimation->Show(0, 1);
                multiMenuOpen = 0;
                EndNetworkGame();
                g_TrackGame->ui->userNameDialogOpen = 0;
            }
            if (userMenuOpen)
                RequestHover(FindControl("LstProfiles", 3), 0, 0);
            if (singleMenuOpen) {
                menuSingleAnimation->Show(0, 1);
                singleMenuOpen = 0;
            }
        } else if (!_stricmp("MenuOptions", event->controlName)) {
            OptionsDlg* dialog = new(__FILE__, 345) OptionsDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, (UIDialog*)this, 0, 0, 1);
        } else if (!_stricmp("ButStuntQuarry", event->controlName)) {
            if (movieControl)
                movieControl->moviePlayer->Stop();
            g_TrackGame->mode.savedSettings.eventType = 0;
            g_TrackGame->mode.savedSettings.series = 0;
            g_TrackGame->mode.currentSeriesDirectory = (int)"Teraform\\Quarries";
            SinglePlayerDlg* dialog = new(__FILE__, 352) SinglePlayerDlg;
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButBaja", event->controlName)) {
            if (movieControl)
                movieControl->moviePlayer->Stop();
            g_TrackGame->mode.savedSettings.eventType = 1;
            g_TrackGame->mode.savedSettings.series = 1;
            g_TrackGame->mode.currentSeriesDirectory = (int)"Teraform\\Baja";
            SinglePlayerDlg* dialog = new(__FILE__, 360) SinglePlayerDlg;
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButSupercross", event->controlName)) {
            if (movieControl)
                movieControl->moviePlayer->Stop();
            g_TrackGame->mode.savedSettings.eventType = 3;
            g_TrackGame->mode.savedSettings.series = 3;
            g_TrackGame->mode.currentSeriesDirectory = (int)"Teraform\\SX";
            SinglePlayerDlg* dialog = new(__FILE__, 368) SinglePlayerDlg;
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButEnduro", event->controlName)) {
            if (movieControl)
                movieControl->moviePlayer->Stop();
            g_TrackGame->mode.savedSettings.eventType = 5;
            g_TrackGame->mode.savedSettings.series = 5;
            g_TrackGame->mode.currentSeriesDirectory = (int)"Teraform\\Enduro";
            SinglePlayerDlg* dialog = new(__FILE__, 376) SinglePlayerDlg;
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButNationals", event->controlName)) {
            if (movieControl)
                movieControl->moviePlayer->Stop();
            g_TrackGame->mode.savedSettings.eventType = 2;
            g_TrackGame->mode.savedSettings.series = 2;
            g_TrackGame->mode.currentSeriesDirectory = (int)"Teraform\\National";
            SinglePlayerDlg* dialog = new(__FILE__, 384) SinglePlayerDlg;
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButProCircuit", event->controlName)) {
            if (movieControl)
                movieControl->moviePlayer->Stop();
            PCStartupDlg* dialog = new(__FILE__, 389) PCStartupDlg;
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButLoadProfile", event->controlName)) {
            g_TrackGame->mode.SaveProfile();
            if (UseSelectedProfile(event))
                g_TrackGame->MuteSound();
            RequestHover(FindControl("LstProfiles", 3), 0, 0);
            ShowCurrentProfile();
            userMenuOpen = 0;
            menuUserAnimation->Show(0, 1);
        } else if (!_stricmp("ButNewProfile", event->controlName)) {
            g_TrackGame->mode.SaveProfile();
            g_TrackGame->ui->OpenMenu(0xbb9);
            RequestHover(FindControl("LstProfiles", 3), 0, 0);
            FillProfileList();
            ShowCurrentProfile();
            userMenuOpen = 0;
            menuUserAnimation->Show(0, 1);
        } else if (!_stricmp("ButRemoveProfile", event->controlName)) {
            UIListBox* list = static_cast<UIListBox*>(FindControl("LstProfiles", 0));
            short row = list->GetSelectedRow();
            if (!_stricmp(list->GetRowText(-1), g_TrackGame->mode.profileName)) {
                NoDelCurProfileDlg* dialog = new(__FILE__, 423) NoDelCurProfileDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
            } else if (row != -1) {
                g_RemoveProfileRow = row;
                RemoveProfileDlg* dialog = new(__FILE__, 426) RemoveProfileDlg;
                guiManager->ShowDialog((UIDialog*)dialog, 0x12f, 4, 0, (UIDialog*)this,
                                                  (int)list->GetRowText(-1), 0, 1);
            }
            RequestHover(list, 0, 0);
            FillProfileList();
        } else if (!_stricmp("ButIPX", event->controlName)) {
            g_TrackGame->ui->userNameDialogOpen = 1;
            if (g_TrackGame->network->CreateAddress(DPSPGUID_IPX, "", "", 0, &address, &size) &&
                g_TrackGame->network->InitializeConnection(address, size, 1)) {
                DebugFree(address, __FILE__, 443);
                g_TrackGame->ui->OpenMenu(0x85c);
            }
        } else if (!_stricmp("ButTCP", event->controlName)) {
            g_TrackGame->ui->userNameDialogOpen = 1;
            ChooseTCPMethodDlg* dialog = new(__FILE__, 453) ChooseTCPMethodDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
        } else if (!_stricmp("ButZone", event->controlName)) {
            g_TrackGame->openLocalizedWebPageOnExit = 1;
            event->dialog->EndDialog(0);
            event->handled = 1;
            g_TrackGame->ui->OpenExitDialog();
        } else if (!_stricmp("ButCable", event->controlName)) {
            g_TrackGame->ui->userNameDialogOpen = 1;
            g_TrackGame->ui->OpenMenu(0xd8);
        } else if (!_stricmp("ButModem", event->controlName)) {
            g_TrackGame->ui->userNameDialogOpen = 1;
            ConnectionInfoType connections[5];
            g_TrackGame->network->EnumConnections(connections, &count);
            for (int i = 0; i < count; i++) {
                if (!memcmp(connections[i].provider, &DPSPGUID_MODEM, sizeof(GUID))) {
                    g_TrackGame->network->directPlay->InitializeConnection(connections[i].connection, 0);
                    g_TrackGame->ui->OpenMenu(0xfc);
                    return;
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
                list->AddRow(g_TrackGame->mode.profileName, 0, 0);
                list->Sort(1);
                list->SelectRowByText(g_TrackGame->mode.profileName);
            } else if (event->code == 0x32) {
                COPY_TEXT(name, list->GetRowText(g_RemoveProfileRow), 16);
                sprintf(path, "%s\\%s", "ui\\profile", name);
                if (g_TrackGame->profileDirectory->DeleteTree(path)) {
                    g_TrackGame->profileDirectory->Scan();
                    list->RemoveRow(g_RemoveProfileRow);
                }
            }
            FillProfileList();
            ShowCurrentProfile();
            if (userMenuOpen)
                RequestHover(list, 0, 0);
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
        userMenuOpen = 0;
        multiMenuOpen = 0;
        singleMenuOpen = 0;
        menuUserAnimation = 0;
        menuMultiAnimation = 0;
        menuSingleAnimation = 0;
        break;
    }
}

// 0x0044fae0: turns the bike view (dragged with the pointer, else slowly by
// itself), animates the previews and now and then plays a rider idle motion.
int SPBikeRiderDlg::Tick(float frameTime) {
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
        bikes->GetRowData(-1);
        g_TrackGame->ui->riderPreview->SetMotionByName("WaitR");
        idleMotionPlaying = 1;
    }
    if (dialogBackground)
        dialogBackground->SetRegionRect(previewRegion, &previewArea);
    return UIDialog::Tick(frameTime);
}

// The replay tape's header ("VCRtape.dat", 0xeb0 bytes): the race settings
// at +0x34 and the garage engine size and stroke at +0x33c.
struct UnknownReplayTapeHeader {
    unsigned char field_0x00[0x34];
    UnknownTrackGameModeSettings raceSettings;      // +0x34
    unsigned char field_0x220[0x33c - 0x220];
    int engineSize;
    int fourStroke;
    unsigned char field_0x344[0xeb0 - 0x344];
};

// 0x004507a0: fills the result lists (by event type) when the page opens;
// "ButViewReplay" plays the last race's tape.
void SPRaceInfoDlg::HandleEvent(UnknownDialogEvent* event) {
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
        if (g_TrackGame->ui->replayAvailable)
            FindControl("ButViewReplay", 1)->Show(1, 1);
        else
            FindControl("ButViewReplay", 1)->Show(0, 1);
        if (g_TrackGame->mode.raceSettings.raceMode == 2) {
            UIControl* text = FindControl("TitleText", 0xc);
            g_TrackGame->LoadResourceString(0x93b, position, 0x80);
            sprintf(title, "%s (%d/%d)", position, g_TrackGame->eventManager->racesRun,
                    g_TrackGame->mode.raceSettings.races + 1);
            text->SetText(title);
            if (g_TrackGame->eventManager->racesRun > g_TrackGame->mode.raceSettings.races) {
                text->SetTextFromResource(g_TrackGame->resourceInstance, 0x93a);
                g_TrackGame->mode.championshipInProgress = 0;
                g_TrackGame->ui->currentMenu = 0x10e;
            }
        }
        UIControl* first = FindControl("LstStats1", 0);
        RequestHover(first, 0, 0);
        ClearStatsLists(event);
        int i;
        UIListBox* list;
        switch (g_TrackGame->mode.raceSettings.eventType) {
        case 1:
        case 2:
        case 3:
        case 5: {
            if (g_TrackGame->mode.raceSettings.raceMode == 2)
                SetStatsButtonResources(event, 0x934, 0x935, 0, 0x938);
            else
                SetStatsButtonResources(event, 0x934, 0x935, 0, 0);
            g_TrackGame->LoadResourceString(0x936, label, 0x80);
            g_TrackGame->LoadResourceString(0x13b8, suffix, 0x80);
            sprintf(title, "%s (%s)", label, suffix);
            UIControl* button = FindControl("ButStats3", 0);
            button->SetText(title);
            for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
                sprintf(position, "%d", g_TrackGame->eventManager->results[i].position);
                FormatLapTime(time, g_TrackGame->eventManager->results[i].fastestLap);
                if (g_TrackGame->eventManager->results[i].finished)
                    FormatLapTime(best, g_TrackGame->eventManager->results[i].totalRunningTime);
                else
                    g_TrackGame->LoadResourceString(0xff0, best, 0x80);
                sprintf(score, "%s (%s)", time, best);
                if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                    sprintf(points, "%d", g_TrackGame->eventManager->results[i].points);
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position,
                                 score, points);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position,
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
            if (g_TrackGame->mode.raceSettings.raceMode == 2)
                SetStatsButtonResources(event, 0x934, 0, 0x937, 0x938);
            else
                SetStatsButtonResources(event, 0x934, 0, 0x937, 0);
            for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
                sprintf(position, "%d", (int)g_TrackGame->eventManager->results[i].score);
                if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                    sprintf(score, "%d", g_TrackGame->eventManager->results[i].points);
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, score);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, 0);
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
                g_TrackGame->mode.raceSettings = header.raceSettings;
                g_TrackGame->mode.SelectSeries(g_TrackGame->mode.raceSettings.eventType);
                g_TrackGame->playingReplay = 1;
                g_TrackGame->useReplayTape = 1;
                UNKNOWN_GARAGE_SETTINGS->engineSize = header.engineSize;
                UNKNOWN_GARAGE_SETTINGS->fourStroke = header.fourStroke;
                COPY_TEXT(g_TrackGame->mode.replayFile, "VCRtape.dat", 0x104);
                LeaveMenusForRace();
                parentDialog->MarkPendingRelease();
            } else {
                event->control->Show(0, 1);
            }
        }
        break;
    }
}

// 0x004513a0: fills the result lists (by event type) when the page opens.
void MPRaceInfoDlg::HandleEvent(UnknownDialogEvent* event) {
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
    if (g_TrackGame->mode.raceSettings.raceMode == 2) {
        UIControl* text = FindControl("TitleText", 0xc);
        g_TrackGame->LoadResourceString(0x93b, position, 0x80);
        sprintf(title, "%s (%d/%d)", position, g_TrackGame->eventManager->racesRun,
                g_TrackGame->mode.raceSettings.races + 1);
        text->SetText(title);
        if (g_TrackGame->eventManager->racesRun > g_TrackGame->mode.raceSettings.races) {
            text->SetTextFromResource(g_TrackGame->resourceInstance, 0x93a);
            g_TrackGame->mode.championshipInProgress = 0;
            g_TrackGame->ui->currentMenu = 0x10e;
        }
    }
    UIControl* first = FindControl("LstStats1", 0);
    RequestHover(first, 0, 0);
    ClearStatsLists(event);
    int i;
    UIListBox* list;
    switch (g_TrackGame->mode.raceSettings.eventType) {
    case 1:
    case 2:
    case 3:
    case 5: {
        if (g_TrackGame->mode.raceSettings.raceMode == 2)
            SetStatsButtonResources(event, 0x934, 0x935, 0, 0x938);
        else
            SetStatsButtonResources(event, 0x934, 0x935, 0, 0);
        g_TrackGame->LoadResourceString(0x936, label, 0x80);
        g_TrackGame->LoadResourceString(0x13b8, suffix, 0x80);
        sprintf(title, "%s (%s)", label, suffix);
        UIControl* button = FindControl("ButStats3", 0);
        button->SetText(title);
        for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
            if (!g_TrackGame->eventManager->results[i].outOfRace) {
                sprintf(position, "%d", g_TrackGame->eventManager->results[i].position);
                FormatLapTime(time, g_TrackGame->eventManager->results[i].fastestLap);
                if (g_TrackGame->eventManager->results[i].finished)
                    FormatLapTime(best, g_TrackGame->eventManager->results[i].totalRunningTime);
                else
                    g_TrackGame->LoadResourceString(0xff0, best, 0x80);
                sprintf(score, "%s (%s)", time, best);
                if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                    sprintf(points, "%d", g_TrackGame->eventManager->results[i].points);
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position, score,
                                 points);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position, score, 0);
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
        if (g_TrackGame->mode.raceSettings.raceMode == 2)
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0x938);
        else
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0);
        for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
            if (!g_TrackGame->eventManager->results[i].outOfRace) {
                sprintf(position, "%d", (int)g_TrackGame->eventManager->results[i].score);
                if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                    sprintf(score, "%d", g_TrackGame->eventManager->results[i].points);
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, score);
                } else {
                    AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, 0);
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
        if (g_TrackGame->mode.raceSettings.stuntMode) {
            if (g_TrackGame->mode.raceSettings.raceMode == 2)
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0x938);
        else
            SetStatsButtonResources(event, 0x934, 0, 0x937, 0);
            for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
                if (!g_TrackGame->eventManager->results[i].outOfRace) {
                    sprintf(position, "%d", (int)g_TrackGame->eventManager->results[i].score);
                    if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                        sprintf(score, "%d", g_TrackGame->eventManager->results[i].points);
                        AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, score);
                    } else {
                        AddStatsRows(event, g_TrackGame->eventManager->results[i].name, 0, position, 0);
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
            if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                SetStatsButtonResources(event, 0x934, 0x939, 0x938, 0);
            } else {
                g_TrackGame->LoadResourceString(0x934, position, 0x80);
                g_TrackGame->LoadResourceString(0x935, score, 0x80);
                g_TrackGame->LoadResourceString(0x939, points, 0x80);
                g_TrackGame->LoadResourceString(0x142b, title, 0x80);
                SetStatsButtonTexts(event, position, score, points, title);
            }
            for (i = 0; i < g_TrackGame->eventManager->resultCount; i++) {
                if (!g_TrackGame->eventManager->results[i].outOfRace) {
                    sprintf(position, "%d", g_TrackGame->eventManager->results[i].position);
                    FormatLapTime(score, g_TrackGame->eventManager->results[i].totalTagRun);
                    FormatLapTime(points, g_TrackGame->eventManager->results[i].bestTagRun);
                    if (g_TrackGame->mode.raceSettings.raceMode == 2) {
                        sprintf(points, "%d", g_TrackGame->eventManager->results[i].points);
                        AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position, score,
                                     points);
                    } else {
                        AddStatsRows(event, g_TrackGame->eventManager->results[i].name, position, score,
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
