#pragma once

#include "GameUi.h"
#include "GUIManager.h"
#include "UIDialog.h"

// Declarations shared by the dialog procedure files (InGameProcs.cpp,
// NetProcs.cpp). A dialog's message procedure is its vtable slot 29 (ret 4,
// one argument). Names are provisional.

// The message a dialog procedure receives. +0x08 is the kind: 1 a control
// command (+0x04 names the control), 5 the dialog's initialisation, 9 a
// notification whose code is +0x00. Other kinds occur in VCRDlg and
// HostJoinDlg.
struct UnknownDialogEvent {
    int field_0x00;                           // notification code
    const char* field_0x04;                   // control name
    int field_0x08;                           // kind
    UIDialog* field_0x0c;                     // the dialog the message is for
    GUIManager* field_0x10;                   // the GUI (OptionsDlg opens its sub-dialogs on it)
    UnknownGameUiControl* field_0x14;         // the control (VCRDlg)
    int field_0x18;                           // cleared by OptControlsDlg for kinds 11 and 12
    int field_0x1c;                           // a kind 11 message's key (dlgprocs.cpp RemoveProfileDlg)
    int field_0x20;                           // set to 1 once handled
};

// cdecl entry points in dlgprocs.cpp's code that the procedures pass a menu
// id and their message to (0x004526b0 near dlgprocs.cpp's line references at
// 0x00452b8e; 0x00453090 near 0x004531dc).
void UnknownFunction4526b0(int menu, UnknownDialogEvent* event);
void UnknownFunction452930(int menu, UnknownDialogEvent* event);
void UnknownFunction453090(int menu, UnknownDialogEvent* event);

// RTTI: ChoiceDlg : UIDialog (vtable 0x00551cb0; slot 29 is 0x004555b0 in
// dlgprocs.cpp). ExitDlg's procedure allocates 0x7f58 bytes for it and
// writes the vtable after UIDialog's constructor, so its constructor is
// inline.
class ChoiceDlg : public UIDialog {
public:
    ChoiceDlg() : UIDialog(1, "messbox2.dtm") {}
    // 0x00455700: sets up the title, the message and the three buttons, each
    // from a text or (when its id is nonzero) a string resource id.
    void UnknownFunction455700(const char* title, int titleId, const char* prompt, int promptId,
                               const char* left, int leftId, const char* middle, int middleId,
                               const char* right, int rightId);
    // 0x00455630: sets the title, the message and the buttons' texts (an empty
    // button text hides the button).
    void UnknownFunction455630(const char* title, const char* prompt, const char* left,
                               const char* middle, const char* right);
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004555b0

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: SessionDlg : UIDialog (vtable 0x00555408; type descriptor 0x0056eb10
// in NetProcs.cpp's data). HostJoinDlg and WaitOrCallDlg allocate 0x7f58
// bytes for it; its constructor is inline.
class SessionDlg : public UIDialog {
public:
    SessionDlg() : UIDialog(1, "messbox1.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004aefd0

    unsigned char field_0x2c[0x34 - 0x2c];
    GUIUser* field_0x34;
    unsigned char field_0x38[0x7f58 - 0x38];
};
