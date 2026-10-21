// Complete nonexact GUI pointer candidates; original standalone TU ownership is unproven.
// See docs/GUI_POINTER_HELPERS.md for private behavioral evidence.
#include "../../src/reconstructed/GUIManager.h"

// 0x00486b10
void ToolTip::ShowControlToolTip(UnknownGuiControl* control) {
    int position[2];
    GUIInputDevice* device = control->ownerDialog->guiUser->pointerDevice;

    position[0] = device ? device->pointerPosition.x
                         : control->area.left + (control->area.right - control->area.left) / 2;
    position[1] = device ? device->pointerPosition.y + 16 : control->area.bottom;
    ShowText(control->toolTipText, position, 1.0f);
}

// 0x00488120
int GUIUser::SetPointerDevice(GUIInputDevice* device) {
    GUIUser* owner = device ? device->ownerUser : 0;

    if (owner == this)
        return 1;
    if (owner)
        return 0;
    pointerDevice = device;
    if (device)
        device->ownerUser = this;
    return 1;
}
