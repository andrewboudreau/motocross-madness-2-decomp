// Independently identified primary UI vtable entries. Shared folded addresses
// do not imply shared method identity or establish original TU ownership.
#include "GameUi.h"

// UIControl primary slots 33, 59 and 60 each select RET 4 at 0x00464e80.
void UIControl::KillFocus(int value) {
}

void UIControl::SyncBoundValue(int value) {
}

void UIControl::HandleTimer(int value) {
}

// Primary slot 30: UIControl and UIDropDownList return one at 0x00467ae0;
// UIStaticText returns zero at 0x00478fe0.
int UIControl::IsInteractive() {
    return 1;
}

int UIDropDownList::IsInteractive() {
    return 1;
}

int UIStaticText::IsInteractive() {
    return 0;
}

// UIStaticText primary slot 29, inherited by UIDropDownList: 0x00479220.
void UIStaticText::SetState(int state) {
    needsRedraw = 1;
    currentState = state;
}
