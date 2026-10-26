// UIEditBox primary slots 38 and 56. These folded bodies are independently
// identified by its RTTI-bound vtable; this grouping is not original TU proof.
#include "GameUi.h"
#include "DialogEventKind.h"
#include "DialogProc.h"

// 0x00474b10, 144 bytes: report an unmodified release inside the pressed edit
// control, then retain the base release behavior unless the event was handled.
int UIEditBox::ButtonUp(int button, int* position) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !button && currentState == 2) {
        UnknownDialogEvent event;
        event.handled = button;
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->HandleEvent(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::ButtonUp(button, position);
}

// 0x00479100, 107 bytes: empty, hidden or obscured controls age redraw frames.
void UIEditBox::UpdateNeedsRedraw() {
    int empty = 0;
    if (!controlText || !controlTextLength)
        empty = 1;
    UIControl* last = FindLastAnchor(0);
    if (!shown || empty || (last && (!last->shown || last->transitionState))) {
        if (--redrawFrames < 0) {
            redrawFrames = 0;
            needsRedraw = 1;
        }
    }
}
