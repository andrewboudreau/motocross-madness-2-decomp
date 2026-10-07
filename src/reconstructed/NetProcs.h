#pragma once

#include "DialogProc.h"

// NetProcs.cpp (__FILE__ 0x0056eb2c): the network setup dialogs' message
// procedures (their vtable slot 29) and two helpers that drop or check the
// network object. Names are provisional; the dialog classes are RTTI names.

// The serial settings SerialPopupDlg's selectors write (0x006886b8, in
// NetProcs.cpp's .bss between Net.cpp's 0x006886b4 and NetThread.cpp's
// 0x006886d0). Each field is the selected state index of the control the
// procedure binds to it; dlgprocs.cpp 0x00452af9 passes the record to
// 0x004ae410, which converts it to DirectPlay serial values.
struct UnknownSerialSettings {
    int port;                                 // "ButPort"
    int baudRate;                             // "ButBaud"
    int stopBits;                             // "ButStop"
    int parity;                               // "ButParity"
    int flowControl;                          // "ButFlow"
};
extern UnknownSerialSettings g_UnknownGlobal6886b8;

// 0x006886cc: session count filled by NetworkInterface 0x004adff0.
extern int g_UnknownGlobal6886cc;

// Net.cpp's error report (0x004ad5a0).
void UnknownFunction4ad5a0(long result, const char* file, int line);

// cdecl 0x004ae460: joins the session selected in "LstSessions" (the
// HostJoinDlg procedure calls it for a kind 13 message).
void UnknownFunction4ae460(UnknownDialogEvent* event);

// cdecl 0x004aefa0: 1 when the network object exists or Game slot 16
// creates it; otherwise drops it (0x004aef40) and returns 0.
int UnknownFunction4aefa0();

// RTTI: HostJoinDlg : UIDialog (vtable 0x00554b98). +0xc4 is 0x85d when the
// dialog is the host variant.
class HostJoinDlg : public UIDialog {
public:
    HostJoinDlg() : UIDialog(1, "HostJoin.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004ae500

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* field_0x30;
    unsigned char field_0x34[0xc4 - 0x34];
    int field_0xc4;
    unsigned char field_0xc8[0x7f58 - 0xc8];
};

// RTTI: SerialPopupDlg : UIDialog (vtable 0x00554eb0).
class SerialPopupDlg : public UIDialog {
public:
    SerialPopupDlg() : UIDialog(1, "SerPopup.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004aea60

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: TCPAddressDlg : UIDialog (vtable 0x00554e2c).
class TCPAddressDlg : public UIDialog {
public:
    TCPAddressDlg() : UIDialog(1, "messbox1.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004aee10

    unsigned char field_0x2c[0x34 - 0x2c];
    GUIUser* field_0x34;
    unsigned char field_0x38[0x7f58 - 0x38];
};

// RTTI: WaitOrCallDlg : UIDialog (vtable 0x00554da8).
class WaitOrCallDlg : public UIDialog {
public:
    WaitOrCallDlg() : UIDialog(1, "messbox2.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004af100

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* field_0x30;
    unsigned char field_0x34[0x7f58 - 0x34];
};

// RTTI: ConnectErrorDlg : UIDialog (vtable 0x00554c1c).
class ConnectErrorDlg : public UIDialog {
public:
    ConnectErrorDlg() : UIDialog(1, "messbox2.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004af470

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: PlayerRemovedDlg : UIDialog (vtable 0x00551b9c).
class PlayerRemovedDlg : public UIDialog {
public:
    PlayerRemovedDlg() : UIDialog(1, "messbox2.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004af590

    unsigned char field_0x2c[0x7f58 - 0x2c];
};
