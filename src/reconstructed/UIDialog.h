#pragma once

#include "GameObject.h"

// RTTI: UIDialog : GameObject (complete object locator 0x0055c280). Its
// constructor 0x00469db0 takes a flag and the dialog resource and writes
// vtable 0x00552a9c. Only what EventManager uses is declared.
class UIDialog : public GameObject {
public:
    UIDialog(int flags, const char* resource);    // 0x00469db0
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
