#pragma once

#include "GameObject.h"

// RTTI: UIDialog : GameObject (complete object locator 0x0055c280). Its
// constructor 0x00469db0 takes a flag and the dialog resource and writes
// vtable 0x00552a9c. Only what EventManager uses is declared.
class UnknownGameUiControl;
class Sound;

class UIDialog : public GameObject {
public:
    UIDialog(int flags, const char* resource);    // 0x00469db0
    // 0x0046ebf0: finds a control by name (GameUi.h declares the same
    // function on UnknownGameUiPage).
    UnknownGameUiControl* UnknownFunction46ebf0(const char* name, int flags);

    // NetProcs.cpp and InGameProcs.cpp (provisional names).
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0046ef00
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0046a780 (SelectGamePicProcs.cpp)
    void UnknownFunction46ecc0(int value);    // 0x0046ecc0
    Sound* UnknownFunction46e9a0(const char* name); // 0x0046e9a0: a dialog sound (OptionProcs.cpp)
    void UnknownFunction46fce0(int a, int time, int b); // 0x0046fce0
    void UnknownFunction46fe40(int value);    // 0x0046fe40
    void UnknownFunction46ea80(int id, int value); // 0x0046ea80 (OptionProcs.cpp)
    void UnknownFunction46eb30(int id, int value); // 0x0046eb30 (OptionProcs.cpp)
    void UnknownFunction46ff30(int result);   // 0x0046ff30: closes the dialog with `result`
    void UnknownFunction46ff70(int a, int b); // 0x0046ff70 (OptionProcs.cpp)
    void UnknownFunction470000(UnknownGameUiControl* control, int a, int b); // 0x00470000
};

// RTTI: TransDlg : UIDialog (vtable 0x00552624; 0x7f64 bytes, the size
// EventManager 0x0045e710 allocates). Its constructor is inline.
class TransDlg : public UIDialog {
public:
    TransDlg() : UIDialog(1, "Trans.dtm") {}
    void UnknownFunction455c40(int menu);         // 0x00455c40: stores +0x7f60

    unsigned char field_0x2c[0x7f60 - 0x2c];
    int field_0x7f60;                             // the menu to open next
};

// RTTI: Intro1Dlg : UIDialog (vtable 0x005548fc; 0x7f5c bytes, the size
// KrustyUI 0x004988a0 allocates). Its constructor is inline.
class Intro1Dlg : public UIDialog {
public:
    Intro1Dlg() : UIDialog(1, "intro1.dtm") {}

    unsigned char field_0x2c[0x7f5c - 0x2c];
};

// RTTI: Exit1Dlg : UIDialog (vtable 0x0055503c; 0x7f68 bytes, the size
// KrustyUI 0x0049a4a0 allocates). Its constructor is inline.
class Exit1Dlg : public UIDialog {
public:
    Exit1Dlg() : UIDialog(1, "Exit1.dtm") {}

    unsigned char field_0x2c[0x7f68 - 0x2c];
};
