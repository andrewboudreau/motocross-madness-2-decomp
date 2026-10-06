#include "NetProcs.h"

#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "Net.h"
#include "TrackGame.h"

extern "C" const GUID DPSPGUID_TCPIP;         // 0x005567e0

UnknownSerialSettings g_UnknownGlobal6886b8;
int g_UnknownGlobal6886cc;

// 0x0056eadc: the baud rates SerialPopupDlg offers ("ButBaud").
int g_UnknownGlobal56eadc[10] = {4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200, 128000, 256000};

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
void UnknownFunction4ae460(UnknownDialogEvent* event) {
    int selection = event->field_0x0c->UnknownFunction46ebf0("LstSessions", 0)->UnknownFunction476950();
    g_UnknownGlobal56e26c->mode.field_0xfd4 = selection;
    if (selection != -1 && event->field_0x0c &&
        static_cast<HostJoinDlg*>(event->field_0x0c)->field_0xc4 != 0x85d &&
        g_UnknownGlobal56e26c->mode.field_0xa98.field_0x00[g_UnknownGlobal56e26c->mode.field_0xfd4].field_0x108 &&
        g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac480(
            (const GUID*)g_UnknownGlobal56e26c->mode.field_0xa98.field_0x00[g_UnknownGlobal56e26c->mode.field_0xfd4].field_0x108,
            0x80)) {
        g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x866);
        event->field_0x0c->UnknownFunction46ff30(0x63);
        event->field_0x20 = 1;
    }
}

// 0x004ae500
void HostJoinDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("ButJoin", 0);
        if (field_0xc4 != 0x85d) {
            control->UnknownVirtualSlot49(0);
            control->UnknownFunction470d40(0x808080);
        }
        control = UnknownFunction46ebf0("LstSessions", 0);
        control->UnknownFunction476c70(0xfeb97a, -1);
        control->UnknownFunction476b80(0xffffff);
        control->UnknownFunction476cd0(0xfeb97a);
        g_UnknownGlobal56e26c->mode.field_0xfd4 = 0;
        control->UnknownVirtualSlot54(&g_UnknownGlobal56e26c->mode.field_0xfd4);
        UnknownFunction46ecc0(0);
        control->UnknownFunction476a60(0);
        UnknownFunction470000(control, 0, 0);
        if (field_0xc4 != 0x85d)
            UnknownFunction46fce0(0, 1000, 0);
        break;
    }
    case 13:
        if (!_stricmp("LstSessions", event->field_0x04))
            UnknownFunction4ae460(event);
        break;
    case 1:
        if (!_stricmp("ButHost", event->field_0x04)) {
            SessionDlg* dialog = new(__FILE__, 110) SessionDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 300, 4, 0,
                                              (UnknownGuiDialog*)this, 0, 0, 1);
        } else if (!_stricmp("ButJoin", event->field_0x04)) {
            UnknownFunction46ecc0(1);
            if (field_0xc4 == 0x85d)
                g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0xd9);
            else
                UnknownFunction4ae460(event);
        } else if (!_stricmp("ButCancel", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
            UnknownFunction4aef40();
            UnknownFunction4aefa0();
        }
        break;
    case 7: {
        UnknownGameUiControl* list = UnknownFunction46ebf0("LstSessions", 0);
        int selection = list->UnknownFunction4768d0(-1);
        list->UnknownFunction4775f0();
        g_UnknownGlobal6886cc = 0;
        if (g_UnknownGlobal56e26c->field_0x08->UnknownFunction4adff0(
                (SessionInfoType*)g_UnknownGlobal56e26c->mode.field_0xa98.field_0x00, &g_UnknownGlobal6886cc, 0x91) < 0) {
            UnknownFunction4aef40();
            event->field_0x0c->UnknownFunction46ff30(0xd);
            event->field_0x20 = 1;
            break;
        }
        for (int i = 0; i < g_UnknownGlobal6886cc; i++) {
            char* name = g_UnknownGlobal56e26c->mode.field_0xa98.field_0x00[i].field_0x04;
            list->UnknownFunction476d80(name, (int)name, 0);
        }
        list->UnknownFunction476b30(selection);
        if (!g_UnknownGlobal6886cc) {
            UnknownGameUiControl* join = UnknownFunction46ebf0("ButJoin", 0);
            join->UnknownVirtualSlot49(0);
            join->UnknownFunction470d40(0x808080);
        } else {
            UnknownGameUiControl* join = UnknownFunction46ebf0("ButJoin", 0);
            join->UnknownVirtualSlot49(1);
            join->UnknownFunction470d40(0xffffff);
        }
        break;
    }
    case 9:
        if (event->field_0x00 == 0x14) {
            if (field_0xc4 == 0x85d) {
                void* address;
                unsigned long size;
                if (!g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abf10(DPSPGUID_TCPIP, "", "", 0,
                                                                             &address, &size))
                    break;
                if (!g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abdf0(address, size, 4))
                    break;
                operator delete(address, __FILE__, 179);
            }
            if (g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac3c0(
                    g_UnknownGlobal56e26c->mode.field_0x10, 0x80)) {
                g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x866);
                event->field_0x0c->UnknownFunction46ff30(0x63);
                event->field_0x20 = 1;
            }
        } else if (event->field_0x00 == 0xf) {
            field_0x30->UnknownFunction4865e0(field_0x30->field_0x280, 1);
            g_UnknownGlobal6886cc = 0;
            long result = g_UnknownGlobal56e26c->field_0x08->UnknownFunction4adff0(
                (SessionInfoType*)g_UnknownGlobal56e26c->mode.field_0xa98.field_0x00, &g_UnknownGlobal6886cc, 0x81);
            if (g_UnknownGlobal6886cc > 0 && result >= 0 &&
                g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac480(
                    (const GUID*)g_UnknownGlobal56e26c->mode.field_0xa98.field_0x00[0].field_0x108, 0)) {
                field_0x30->UnknownFunction4865e0(field_0x30->field_0x200, 1);
                g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x866);
                event->field_0x0c->UnknownFunction46ff30(0x63);
                event->field_0x20 = 1;
            } else {
                if (result < 0)
                    UnknownFunction4ad5a0(result, __FILE__, 207);
                UnknownFunction4aef40();
                UnknownFunction4aefa0();
                field_0x30->UnknownFunction4865e0(field_0x30->field_0x200, 1);
                g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x1f9);
            }
        } else if (event->field_0x00 == 0xd) {
            UnknownFunction46ff30(0xd);
        } else if (event->field_0x00 == 0x63) {
            event->field_0x0c->UnknownFunction46ff30(0x63);
            event->field_0x20 = 1;
        }
        break;
    case 6:
        UnknownFunction46fe40(0);
        break;
    }
}

// 0x004aea60
void SerialPopupDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[128];
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("ButCancel", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        } else if (!_stricmp("ButOk", event->field_0x04)) {
            UnknownFunction46ecc0(1);
            UnknownFunction452930(0xd8, event);
        }
        break;
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("ButPort", 2);
        control->UnknownFunction478860(4);
        control->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x147b);
        control->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x147c);
        control->UnknownFunction478a50(2, g_UnknownGlobal56e26c->field_0x420, 0x147d);
        control->UnknownFunction478a50(3, g_UnknownGlobal56e26c->field_0x420, 0x147e);
        g_UnknownGlobal6886b8.port = 0;
        control->UnknownVirtualSlot54(&g_UnknownGlobal6886b8.port);

        control = UnknownFunction46ebf0("ButBaud", 2);
        control->UnknownFunction478860(10);
        for (int i = 0; i < 10; i++) {
            sprintf(text, "%d", g_UnknownGlobal56eadc[i]);
            control->UnknownFunction478ad0(i, text);
        }
        g_UnknownGlobal6886b8.baudRate = 6;
        control->UnknownVirtualSlot54(&g_UnknownGlobal6886b8.baudRate);

        control = UnknownFunction46ebf0("ButStop", 2);
        control->UnknownFunction478860(3);
        control->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x1478);
        control->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x1479);
        control->UnknownFunction478a50(2, g_UnknownGlobal56e26c->field_0x420, 0x147a);
        g_UnknownGlobal6886b8.stopBits = 0;
        control->UnknownVirtualSlot54(&g_UnknownGlobal6886b8.stopBits);

        control = UnknownFunction46ebf0("ButParity", 2);
        control->UnknownFunction478860(4);
        control->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x146f);
        control->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x1470);
        control->UnknownFunction478a50(2, g_UnknownGlobal56e26c->field_0x420, 0x1471);
        control->UnknownFunction478a50(3, g_UnknownGlobal56e26c->field_0x420, 0x1472);
        g_UnknownGlobal6886b8.parity = 0;
        control->UnknownVirtualSlot54(&g_UnknownGlobal6886b8.parity);

        control = UnknownFunction46ebf0("ButFlow", 2);
        control->UnknownFunction478860(5);
        control->UnknownFunction478a50(0, g_UnknownGlobal56e26c->field_0x420, 0x1474);
        control->UnknownFunction478a50(1, g_UnknownGlobal56e26c->field_0x420, 0x1473);
        control->UnknownFunction478a50(2, g_UnknownGlobal56e26c->field_0x420, 0x1476);
        control->UnknownFunction478a50(3, g_UnknownGlobal56e26c->field_0x420, 0x1477);
        control->UnknownFunction478a50(4, g_UnknownGlobal56e26c->field_0x420, 0x1475);
        g_UnknownGlobal6886b8.flowControl = 4;
        control->UnknownVirtualSlot54(&g_UnknownGlobal6886b8.flowControl);

        UnknownFunction46ecc0(0);
        break;
    }
    case 9:
        if (event->field_0x00 == 0x63) {
            event->field_0x0c->UnknownFunction46ff30(0x63);
            event->field_0x20 = 1;
        } else if (event->field_0x00 == 0xd) {
            UnknownFunction4aefa0();
        }
        break;
    }
}

// 0x004aee10
void TCPAddressDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("CancelButton", event->field_0x04))
            UnknownFunction4526b0(0xd9, event);
        else if (!_stricmp("OkButton", event->field_0x04))
            UnknownFunction452930(0xd9, event);
        break;
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("TitleText", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x898);
        control = UnknownFunction46ebf0("OkButton", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e9);
        control = UnknownFunction46ebf0("CancelButton", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13dc);
        control = UnknownFunction46ebf0("TxtPrompt", 12);
        control->field_0x1e8 = 1;
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x899);
        control = UnknownFunction46ebf0("EditBox", 0);
        field_0x34->UnknownFunction487790((UnknownGuiControl*)control, 0, 0);
        break;
    }
    }
}

// 0x004aef40
void UnknownFunction4aef40() {
    if (g_UnknownGlobal56e26c->field_0x08) {
        delete g_UnknownGlobal56e26c->field_0x08;
        g_UnknownGlobal56e26c->field_0x08 = 0;
    }
    g_UnknownGlobal56e26c->field_0x18 = 1;
    g_UnknownGlobal56e26c->field_0x3424 = 0;
    g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35 = 0;
    g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = 0;
}

// 0x004aefa0
int UnknownFunction4aefa0() {
    if (!g_UnknownGlobal56e26c->field_0x08 && !g_UnknownGlobal56e26c->UnknownVirtualSlot16(1)) {
        UnknownFunction4aef40();
        return 0;
    }
    return 1;
}

// 0x004aefd0
void SessionDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("CancelButton", event->field_0x04))
            UnknownFunction4526b0(300, event);
        else if (!_stricmp("OKButton", event->field_0x04))
            UnknownFunction452930(300, event);
        break;
    case 5: {
        UnknownFunction46ebf0("TitleText", 12)->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x911);
        UnknownGameUiControl* control = UnknownFunction46ebf0("OKButton", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e9);
        control = UnknownFunction46ebf0("CancelButton", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13dc);
        control = UnknownFunction46ebf0("EditBox", 0);
        control->UnknownFunction473c70(11);
        control->UnknownFunction473da0(g_UnknownGlobal56e26c->mode.field_0x10);
        control->UnknownFunction473f30(kSessionNameCharacters);
        field_0x34->UnknownFunction487790((UnknownGuiControl*)control, 0, 0);
        break;
    }
    }
}

// 0x004af100
void WaitOrCallDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("ButRight", event->field_0x04)) {
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
            UnknownFunction4aef40();
            UnknownFunction4aefa0();
            return;
        } else if (!_stricmp("ButLeft", event->field_0x04)) {
            SessionDlg* dialog = new(__FILE__, 448) SessionDlg;
            field_0x30->UnknownFunction485a70((UnknownGuiDialog*)dialog, 300, 4, 0,
                                              (UnknownGuiDialog*)this, 0, 0, 1);
            return;
        } else if (!_stricmp("ButMiddle", event->field_0x04)) {
            SendMessageA((HWND)g_UnknownGlobal56e26c->field_0x31c, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            g_UnknownGlobal6886cc = 0;
            g_UnknownGlobal56e26c->field_0x08->UnknownFunction4adff0(
                (SessionInfoType*)g_UnknownGlobal56e26c->mode.field_0xa98.field_0x00, &g_UnknownGlobal6886cc, 1);
            SendMessageA((HWND)g_UnknownGlobal56e26c->field_0x31c, WM_SYSCOMMAND, SC_RESTORE, 0);
            if (g_UnknownGlobal6886cc >= 1 &&
                g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac480(
                    (const GUID*)g_UnknownGlobal56e26c->mode.field_0xa98.field_0x00[g_UnknownGlobal56e26c->mode.field_0xfd4].field_0x108,
                    0)) {
                g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x866);
                event->field_0x0c->UnknownFunction46ff30(0x63);
                event->field_0x20 = 1;
            } else {
                event->field_0x0c->UnknownFunction46ff30(0xd);
                event->field_0x20 = 1;
            }
        }
        break;
    case 5: {
        UnknownFunction46ebf0("TitleText", 12)->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x952);
        UnknownGameUiControl* control = UnknownFunction46ebf0("ButRight", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13dc);
        control = UnknownFunction46ebf0("ButLeft", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x950);
        control = UnknownFunction46ebf0("ButMiddle", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x951);
        return;
    }
    case 9:
        if (event->field_0x00 == 0x14) {
            SendMessageA((HWND)g_UnknownGlobal56e26c->field_0x31c, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            int opened = g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac3c0(
                g_UnknownGlobal56e26c->mode.field_0x10, 0);
            SendMessageA((HWND)g_UnknownGlobal56e26c->field_0x31c, WM_SYSCOMMAND, SC_RESTORE, 0);
            if (opened) {
                g_UnknownGlobal56e26c->ui->UnknownFunction499b20(0x866);
                event->field_0x0c->UnknownFunction46ff30(0x63);
                event->field_0x20 = 1;
                return;
            }
            event->field_0x0c->UnknownFunction46ff30(0xd);
            event->field_0x20 = 1;
            return;
        }
        break;
    }
}

// 0x004af470
void ConnectErrorDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("ButMiddle", event->field_0x04))
            UnknownFunction452930(0x1f9, event);
        break;
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("TitleText", 0);
        control->UnknownFunction470da0(10);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13c2);
        control = UnknownFunction46ebf0("ButLeft", 1);
        control->UnknownFunction470660(0, 1);
        control->field_0x1d8 = 0;
        control = UnknownFunction46ebf0("ButRight", 0);
        control->UnknownFunction470660(0, 1);
        control = UnknownFunction46ebf0("ButMiddle", 0);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e9);
        control->field_0x1d8 = 0x1c;
        control = UnknownFunction46ebf0("TxtPrompt", 12);
        control->field_0x1e8 = 1;
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13c9);
        break;
    }
    }
}

// 0x004af590
void PlayerRemovedDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("ButRight", event->field_0x04)) {
            UnknownFunction4aef40();
            event->field_0x0c->UnknownFunction46ff30(0);
            event->field_0x20 = 1;
        }
        break;
    case 5: {
        UnknownGameUiControl* control = UnknownFunction46ebf0("ButLeft", 1);
        control->UnknownFunction470660(0, 1);
        control->field_0x1d8 = 0;
        control = UnknownFunction46ebf0("ButMiddle", 1);
        control->UnknownFunction470660(0, 1);
        control = UnknownFunction46ebf0("ButRight", 1);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13e9);
        control->field_0x1d8 = 0x1c;
        control = UnknownFunction46ebf0("TitleText", 12);
        control->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x13d1);
        break;
    }
    }
}
