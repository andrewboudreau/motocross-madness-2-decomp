// Bring-up definition (best-guess-unreviewed) for MainDlg::HandleEvent
// (retail 0x0044b200), the main menu's event handler. Extracted from the
// near-miss candidate in samples/ui/DlgProcsNearMisses.cpp (which compiles
// it inside a copy of DlgProcs.cpp); the remaining differences there are
// register/scheduling only. src/reconstructed/DlgProcs.cpp declares the
// method (DlgProcs.h) but does not define it yet.
#include "../reconstructed/BikeRaceCore.h"
#include "../reconstructed/GUIManagerCore.h"
#include "../reconstructed/DlgProcs.h"

#include "../reconstructed/DialogEventKind.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../reconstructed/BackgroundImage.h"
#include "../reconstructed/Camera.h"
#include "../reconstructed/D3DIMSoulTree.h"
#include "../reconstructed/DebugAlloc.h"
#include "../krusty2/math/FastMath.h"
#include "../reconstructed/InGameProcs.h"
#include "../reconstructed/MatrixUtil.h"
#include "../reconstructed/MenuId.h"
#include "../reconstructed/Net.h"
#include "../reconstructed/NetProcs.h"
#include "../reconstructed/OptionProcs.h"
#include "../reconstructed/ProCircuitProcs.h"
#include "../reconstructed/RenderTarget.h"
#include "../reconstructed/SelectGamePicProcs.h"
#include "../reconstructed/TrackGame.h"
#include "../reconstructed/EventManager.h"
#include "../reconstructed/BaseQuarryEvent.h"
#include "../reconstructed/TrackRecordDlg.h"

// A truncating copy into a `size`-byte buffer (DlgProcs.cpp's macro).
#define COPY_TEXT(dest, source, size)                              \
    {                                                              \
        int length = strlen(source);                               \
        int copied = length > (size) - 1 ? (size) - 1 : length;    \
        strncpy(dest, source, copied);                             \
        (dest)[copied] = 0;                                        \
    }

// 0x0059adbc, defined in src/reconstructed/DlgProcs.cpp.
extern int g_RemoveProfileRow;

// dplay.h's service provider GUIDs (declared as in DlgProcs.cpp / Net.cpp).
extern "C" const GUID DPSPGUID_IPX;                     // 0x005567d0
extern "C" const GUID DPSPGUID_TCPIP;                   // 0x005567e0
extern "C" const GUID DPSPGUID_SERIAL;                  // 0x005567f0
extern "C" const GUID DPSPGUID_MODEM;                   // 0x00556800

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
