// Near-miss EventManager candidates, kept out of src/reconstructed until
// they match. See docs/EVENTMANAGER.md.
//
// UnknownFunction45cb20 (0x0045cb20, 67 bytes): cdecl progress callback that
// 0x0045cb70 and 0x0045cdc0 pass by address. 63 of 67 bytes match: retail
// loads *step into edx and the bar value into ecx; VC6 here swaps them.
// Compound, spelled-out, commuted, local-variable, void* and struct
// parameter forms, and compiling the function alone, do not change it.
#include "../../src/reconstructed/TrackGame.h"

// A control found by name (its +0x1f0 is a progress bar's value). Declaring
// these two classes in KrustyUI.h changes VC6's register choice in
// TrackGame slot 1, so they stay here.
class UnknownGameUiControl {
public:
    unsigned char field_0x000[0x1f0];
    int field_0x1f0;
};

// Page object at KrustyUI+0x490; 0x0046ebf0 sits among gameui.cpp's literals.
class UnknownGameUiPage {
public:
    UnknownGameUiControl* UnknownFunction46ebf0(const char* name, int flags); // 0x0046ebf0
};


// 0x0045cb20: adds *step to KrustyUI's "ProgressBar"; without a step it
// calls KrustyUI 0x0049b530.
void UnknownFunction45cb20(int* step) {
    KrustyUI* ui = g_UnknownGlobal56e26c->ui;
    if (step) {
        if (ui->field_0x490) {
            UnknownGameUiControl* bar = ui->field_0x490->UnknownFunction46ebf0("ProgressBar", 0);
            bar->field_0x1f0 += *step;
        }
    } else {
        ui->UnknownFunction49b530();
    }
}
