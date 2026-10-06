#pragma once

#include "DialogProc.h"

// InGameProcs.cpp (__FILE__ 0x0056c5cc): the in-game menu dialogs' message
// procedures (their vtable slot 29) and the VCR dialog. Names are
// provisional; the classes are RTTI names.

// 0x00488bd0: formats `seconds` as "hh:mm:ss.ss" ("00:00:00.00" for 0 and
// FLT_MAX). VCRDlg and dlgprocs.cpp (0x00454919, 0x00455427) call it.
void UnknownFunction488bd0(char* text, float seconds);

// RTTI: ExitDlg : UIDialog (vtable 0x00554ca0).
class ExitDlg : public UIDialog {
public:
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004886e0

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* field_0x30;
};

// RTTI: ContinueDlg : UIDialog (vtable 0x00554d24).
class ContinueDlg : public UIDialog {
public:
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00488ad0

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* field_0x30;
};

// RTTI: VCRDlg : UIDialog (vtable 0x00550d94): the replay controls.
class VCRDlg : public UIDialog {
public:
    // Inline at bikerace.cpp's `new` (0x00420650, line 0x1073).
    VCRDlg(int flags, const char* resource) : UIDialog(flags, resource) {}
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x004893c0
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00488c90
    void UnknownFunction4894c0();             // 0x004894c0: shows the replay mode on the toggles

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* field_0x30;
    unsigned char field_0x34[0x7f58 - 0x34];
    int field_0x7f58;                         // set once the replay is left
};
