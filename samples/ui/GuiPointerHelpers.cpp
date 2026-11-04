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

// 0x00488120 SetPointerDevice (48/52 bytes, 16 vs retail's 18 instructions): retail materialises
// the ternary (`xor edx, edx` arm, then `test edx, edx`); VC6 threads the null arm straight to the
// assign path in every spelling tried: owner-first and this-first nesting, `owner != this` early
// return, `owner != 0 && owner != this`, if/else instead of `?:`, `owner = 0; if (device) ...`, and a
// static inline OwnerOf(device) helper (48-49 bytes, all 16 instructions).
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
