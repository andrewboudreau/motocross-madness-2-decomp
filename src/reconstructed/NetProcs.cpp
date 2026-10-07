#include "NetProcs.h"

#include "DialogEventKind.h"

#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "Net.h"
#include "TrackGame.h"

extern "C" const GUID DPSPGUID_TCPIP;         // 0x005567e0

UnknownSerialSettings g_SerialSettings;
int g_SessionListCount;

// 0x0056eadc: the baud rates SerialPopupDlg offers ("ButBaud").
int g_SerialBaudRates[10] = {4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200, 128000, 256000};

// 0x0056eb94: the characters the session name edit box accepts.
static const char kSessionNameCharacters[] =
    "\xc7\xfc\xe9\xe2\xe4\xe0\xe5\xe7\xea\xeb\xe8\xef\xee\xec\xc4\xc5"
    "\xc9\xe6\xc6\xf4\xf6\xf2\xfb\xf9\xff\xd6\xdc\xe1\xed\xf3\xfa\xf1"
    "\xd1\xaa\xba\xbf\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d"
    "\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d"
    "\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xab\xac\xad\xae"
    "\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe"
    "\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce"
    "\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde"
    "\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xf0\xf5\xf7\xf8\xf9"
    "\xfd\xfe"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-()$#";

// 0x004ae460
void JoinSelectedSession(UnknownDialogEvent* event) {
    int selection = static_cast<UIListBox*>(event->dialog->FindControl("LstSessions", 0))->GetSelectedRow();
    g_TrackGame->mode.field_0xfd4 = selection;
    if (selection != -1 && event->dialog &&
        static_cast<HostJoinDlg*>(event->dialog)->openingMenu != 0x85d &&
        g_TrackGame->mode.field_0xa98.field_0x00[g_TrackGame->mode.field_0xfd4].instance &&
        g_TrackGame->network->JoinSession(
            (const GUID*)g_TrackGame->mode.field_0xa98.field_0x00[g_TrackGame->mode.field_0xfd4].instance,
            0x80)) {
        g_TrackGame->ui->OpenMenu(0x866);
        event->dialog->EndDialog(0x63);
        event->handled = 1;
    }
}

// 0x004ae500
void HostJoinDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogInit: {
        UIListBox* control = static_cast<UIListBox*>(FindControl("ButJoin", 0));
        if (openingMenu != 0x85d) {
            control->UnknownVirtualSlot49(0);
            control->SetFontColor(0x808080);
        }
        control = static_cast<UIListBox*>(FindControl("LstSessions", 0));
        control->SetRowTextColor(0xfeb97a, -1);
        control->SetSelectColor(0xffffff);
        control->SetSelectBoxColor(0xfeb97a);
        g_TrackGame->mode.field_0xfd4 = 0;
        control->UnknownVirtualSlot54(&g_TrackGame->mode.field_0xfd4);
        UpdateBoundValues(0);
        control->SelectRow(0);
        UnknownFunction470000(control, 0, 0);
        if (openingMenu != 0x85d)
            AddTimer(0, 1000, 0);
        break;
    }
    case kDialogListDoubleClick:
        if (!_stricmp("LstSessions", event->controlName))
            JoinSelectedSession(event);
        break;
    case kDialogCommand:
        if (!_stricmp("ButHost", event->controlName)) {
            SessionDlg* dialog = new(__FILE__, 110) SessionDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 300, 4, 0,
                                              (UIDialog*)this, 0, 0, 1);
        } else if (!_stricmp("ButJoin", event->controlName)) {
            UpdateBoundValues(1);
            if (openingMenu == 0x85d)
                g_TrackGame->ui->OpenMenu(0xd9);
            else
                JoinSelectedSession(event);
        } else if (!_stricmp("ButCancel", event->controlName)) {
            event->dialog->EndDialog(0);
            event->handled = 1;
            EndNetworkGame();
            EnsureNetworkInterface();
        }
        break;
    case kDialogTimer: {
        UIListBox* list = static_cast<UIListBox*>(FindControl("LstSessions", 0));
        int selection = list->GetRowData(-1);
        list->RemoveAllRows();
        g_SessionListCount = 0;
        if (g_TrackGame->network->EnumSessions(
                (SessionInfoType*)g_TrackGame->mode.field_0xa98.field_0x00, &g_SessionListCount, 0x91) < 0) {
            EndNetworkGame();
            event->dialog->EndDialog(0xd);
            event->handled = 1;
            break;
        }
        for (int i = 0; i < g_SessionListCount; i++) {
            char* name = g_TrackGame->mode.field_0xa98.field_0x00[i].name;
            list->AddRow(name, (int)name, 0);
        }
        list->SelectRowByData(selection);
        if (!g_SessionListCount) {
            UIControl* join = FindControl("ButJoin", 0);
            join->UnknownVirtualSlot49(0);
            join->SetFontColor(0x808080);
        } else {
            UIControl* join = FindControl("ButJoin", 0);
            join->UnknownVirtualSlot49(1);
            join->SetFontColor(0xffffff);
        }
        break;
    }
    case 9:
        if (event->code == 0x14) {
            if (openingMenu == 0x85d) {
                void* address;
                unsigned long size;
                if (!g_TrackGame->network->CreateAddress(DPSPGUID_TCPIP, "", "", 0,
                                                                             &address, &size))
                    break;
                if (!g_TrackGame->network->InitializeConnection(address, size, 4))
                    break;
                DebugFree(address, __FILE__, 179);
            }
            if (g_TrackGame->network->CreateSession(
                    g_TrackGame->mode.field_0x10, 0x80)) {
                g_TrackGame->ui->OpenMenu(0x866);
                event->dialog->EndDialog(0x63);
                event->handled = 1;
            }
        } else if (event->code == 0xf) {
            guiManager->UnknownFunction4865e0(guiManager->waitCursorImage, 1);
            g_SessionListCount = 0;
            long result = g_TrackGame->network->EnumSessions(
                (SessionInfoType*)g_TrackGame->mode.field_0xa98.field_0x00, &g_SessionListCount, 0x81);
            if (g_SessionListCount > 0 && result >= 0 &&
                g_TrackGame->network->JoinSession(
                    (const GUID*)g_TrackGame->mode.field_0xa98.field_0x00[0].instance, 0)) {
                guiManager->UnknownFunction4865e0(guiManager->cursorImage, 1);
                g_TrackGame->ui->OpenMenu(0x866);
                event->dialog->EndDialog(0x63);
                event->handled = 1;
            } else {
                if (result < 0)
                    ReportDirectPlayError(result, __FILE__, 207);
                EndNetworkGame();
                EnsureNetworkInterface();
                guiManager->UnknownFunction4865e0(guiManager->cursorImage, 1);
                g_TrackGame->ui->OpenMenu(0x1f9);
            }
        } else if (event->code == 0xd) {
            EndDialog(0xd);
        } else if (event->code == 0x63) {
            event->dialog->EndDialog(0x63);
            event->handled = 1;
        }
        break;
    case kDialogClose:
        RemoveTimers(0);
        break;
    }
}

// 0x004aea60
void SerialPopupDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[128];
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ButCancel", event->controlName)) {
            event->dialog->EndDialog(0);
            event->handled = 1;
        } else if (!_stricmp("ButOk", event->controlName)) {
            UpdateBoundValues(1);
            UnknownFunction452930(0xd8, event);
        }
        break;
    case kDialogInit: {
        UIMultiState* control = static_cast<UIMultiState*>(FindControl("ButPort", 2));
        control->SetStateCount(4);
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x147b);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x147c);
        control->SetStateTextFromResource(2, g_TrackGame->resourceInstance, 0x147d);
        control->SetStateTextFromResource(3, g_TrackGame->resourceInstance, 0x147e);
        g_SerialSettings.port = 0;
        control->UnknownVirtualSlot54(&g_SerialSettings.port);

        control = static_cast<UIMultiState*>(FindControl("ButBaud", 2));
        control->SetStateCount(10);
        for (int i = 0; i < 10; i++) {
            sprintf(text, "%d", g_SerialBaudRates[i]);
            control->SetStateText(i, text);
        }
        g_SerialSettings.baudRate = 6;
        control->UnknownVirtualSlot54(&g_SerialSettings.baudRate);

        control = static_cast<UIMultiState*>(FindControl("ButStop", 2));
        control->SetStateCount(3);
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1478);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1479);
        control->SetStateTextFromResource(2, g_TrackGame->resourceInstance, 0x147a);
        g_SerialSettings.stopBits = 0;
        control->UnknownVirtualSlot54(&g_SerialSettings.stopBits);

        control = static_cast<UIMultiState*>(FindControl("ButParity", 2));
        control->SetStateCount(4);
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x146f);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1470);
        control->SetStateTextFromResource(2, g_TrackGame->resourceInstance, 0x1471);
        control->SetStateTextFromResource(3, g_TrackGame->resourceInstance, 0x1472);
        g_SerialSettings.parity = 0;
        control->UnknownVirtualSlot54(&g_SerialSettings.parity);

        control = static_cast<UIMultiState*>(FindControl("ButFlow", 2));
        control->SetStateCount(5);
        control->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1474);
        control->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1473);
        control->SetStateTextFromResource(2, g_TrackGame->resourceInstance, 0x1476);
        control->SetStateTextFromResource(3, g_TrackGame->resourceInstance, 0x1477);
        control->SetStateTextFromResource(4, g_TrackGame->resourceInstance, 0x1475);
        g_SerialSettings.flowControl = 4;
        control->UnknownVirtualSlot54(&g_SerialSettings.flowControl);

        UpdateBoundValues(0);
        break;
    }
    case 9:
        if (event->code == 0x63) {
            event->dialog->EndDialog(0x63);
            event->handled = 1;
        } else if (event->code == 0xd) {
            EnsureNetworkInterface();
        }
        break;
    }
}

// 0x004aee10
void TCPAddressDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("CancelButton", event->controlName))
            UnknownFunction4526b0(0xd9, event);
        else if (!_stricmp("OkButton", event->controlName))
            UnknownFunction452930(0xd9, event);
        break;
    case kDialogInit: {
        UIControl* control = FindControl("TitleText", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x898);
        control = FindControl("OkButton", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e9);
        control = FindControl("CancelButton", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13dc);
        control = FindControl("TxtPrompt", 12);
        control->field_0x1e8 = 1;
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x899);
        control = FindControl("EditBox", 0);
        guiUser->UnknownFunction487790((UnknownGuiControl*)control, 0, 0);
        break;
    }
    }
}

// 0x004aef40
void EndNetworkGame() {
    if (g_TrackGame->network) {
        delete g_TrackGame->network;
        g_TrackGame->network = 0;
    }
    g_TrackGame->field_0x18 = 1;
    g_TrackGame->field_0x3424 = 0;
    g_TrackGame->mode.field_0x27f8.field_0x35 = 0;
    g_TrackGame->mode.field_0x27f8.field_0x28 = 0;
}

// 0x004aefa0
int EnsureNetworkInterface() {
    if (!g_TrackGame->network && !g_TrackGame->CreateNetworkInterface(1)) {
        EndNetworkGame();
        return 0;
    }
    return 1;
}

// 0x004aefd0
void SessionDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("CancelButton", event->controlName))
            UnknownFunction4526b0(300, event);
        else if (!_stricmp("OKButton", event->controlName))
            UnknownFunction452930(300, event);
        break;
    case kDialogInit: {
        FindControl("TitleText", 12)->SetTextFromResource(g_TrackGame->resourceInstance, 0x911);
        UIEditBox* control = static_cast<UIEditBox*>(FindControl("OKButton", 0));
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e9);
        control = static_cast<UIEditBox*>(FindControl("CancelButton", 0));
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13dc);
        control = static_cast<UIEditBox*>(FindControl("EditBox", 0));
        control->SetCapacity(11);
        control->SetEditText(g_TrackGame->mode.field_0x10);
        control->SetAcceptedCharacters(kSessionNameCharacters);
        guiUser->UnknownFunction487790((UnknownGuiControl*)control, 0, 0);
        break;
    }
    }
}

// 0x004af100
void WaitOrCallDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ButRight", event->controlName)) {
            event->dialog->EndDialog(0);
            event->handled = 1;
            EndNetworkGame();
            EnsureNetworkInterface();
            return;
        } else if (!_stricmp("ButLeft", event->controlName)) {
            SessionDlg* dialog = new(__FILE__, 448) SessionDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 300, 4, 0,
                                              (UIDialog*)this, 0, 0, 1);
            return;
        } else if (!_stricmp("ButMiddle", event->controlName)) {
            SendMessageA((HWND)g_TrackGame->windowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            g_SessionListCount = 0;
            g_TrackGame->network->EnumSessions(
                (SessionInfoType*)g_TrackGame->mode.field_0xa98.field_0x00, &g_SessionListCount, 1);
            SendMessageA((HWND)g_TrackGame->windowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
            if (g_SessionListCount >= 1 &&
                g_TrackGame->network->JoinSession(
                    (const GUID*)g_TrackGame->mode.field_0xa98.field_0x00[g_TrackGame->mode.field_0xfd4].instance,
                    0)) {
                g_TrackGame->ui->OpenMenu(0x866);
                event->dialog->EndDialog(0x63);
                event->handled = 1;
            } else {
                event->dialog->EndDialog(0xd);
                event->handled = 1;
            }
        }
        break;
    case kDialogInit: {
        FindControl("TitleText", 12)->SetTextFromResource(g_TrackGame->resourceInstance, 0x952);
        UIControl* control = FindControl("ButRight", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13dc);
        control = FindControl("ButLeft", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x950);
        control = FindControl("ButMiddle", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x951);
        return;
    }
    case 9:
        if (event->code == 0x14) {
            SendMessageA((HWND)g_TrackGame->windowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            int opened = g_TrackGame->network->CreateSession(
                g_TrackGame->mode.field_0x10, 0);
            SendMessageA((HWND)g_TrackGame->windowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
            if (opened) {
                g_TrackGame->ui->OpenMenu(0x866);
                event->dialog->EndDialog(0x63);
                event->handled = 1;
                return;
            }
            event->dialog->EndDialog(0xd);
            event->handled = 1;
            return;
        }
        break;
    }
}

// 0x004af470
void ConnectErrorDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ButMiddle", event->controlName))
            UnknownFunction452930(0x1f9, event);
        break;
    case kDialogInit: {
        UIControl* control = FindControl("TitleText", 0);
        control->SetTextAlign(10);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13c2);
        control = FindControl("ButLeft", 1);
        control->Show(0, 1);
        control->keyBind = 0;
        control = FindControl("ButRight", 0);
        control->Show(0, 1);
        control = FindControl("ButMiddle", 0);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e9);
        control->keyBind = 0x1c;
        control = FindControl("TxtPrompt", 12);
        control->field_0x1e8 = 1;
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13c9);
        break;
    }
    }
}

// 0x004af590
void PlayerRemovedDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ButRight", event->controlName)) {
            EndNetworkGame();
            event->dialog->EndDialog(0);
            event->handled = 1;
        }
        break;
    case kDialogInit: {
        UIControl* control = FindControl("ButLeft", 1);
        control->Show(0, 1);
        control->keyBind = 0;
        control = FindControl("ButMiddle", 1);
        control->Show(0, 1);
        control = FindControl("ButRight", 1);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13e9);
        control->keyBind = 0x1c;
        control = FindControl("TitleText", 12);
        control->SetTextFromResource(g_TrackGame->resourceInstance, 0x13d1);
        break;
    }
    }
}
