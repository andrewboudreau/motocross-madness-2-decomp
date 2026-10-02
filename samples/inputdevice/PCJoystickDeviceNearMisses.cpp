// Near-miss joystick device candidates, kept out of src/reconstructed until
// they match. See docs/INPUT_DEVICES.md.
//
// PCJoystickDevice::UnknownMethod4c3a10 (0x004c3a10, 206 bytes): everything
// up to the final acquire matches (179 bytes). Retail tests that call's
// result with separate `return 0` and `return 1` paths; VC6 here folds them
// into `setge`. Else, `>= 0`, result-variable, flag-variable and inline-helper
// forms do not change it.
// JoystickDevice::UnknownVirtualSlot2 (0x00489980, 145 bytes): the modifier
// test matches only as an inline helper (written in place, VC6 moves the
// no-keyboard branch). Retail then sends the failed button test to the same
// `return 0` block; here VC6 emits a second copy, shifting the remaining
// bytes. `&&`, nested and inline-button forms do not change it.
#include <string.h>

#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/UnknownObject56e26c.h"

// 0x004c3a10: switches buffered input on (16 entries) or off. Property 1 is
// consistent with DIPROP_BUFFERSIZE; the device is unacquired around it.
int PCJoystickDevice::UnknownMethod4c3a10(int buffered) {
    if (!field_0x25c)
        return 0;
    field_0x25c->UnknownMethod8();
    UnknownInputProperty property;
    memset(&property, 0, sizeof(property));
    property.size = sizeof(property);
    property.headerSize = 0x10;
    property.object = 0;
    property.how = 0;
    property.data = buffered ? 16 : 0;
    long result = field_0x25c->UnknownMethod6(1, &property);
    if (result < 0) {
        UnknownReportError(result, __FILE__, 590);
        return 0;
    }
    field_0x5d4_bit0 = buffered;
    if (field_0x25c->UnknownMethod7() < 0)
        return 0;
    return 1;
}

// Modifier test: through the keyboard when there is one, else only "none"
// (0, 0x80000000 or 0x3f) holds.
static inline int ModifierHeld(UnknownInterface56e26c* keyboard, int modifier) {
    if (keyboard)
        return keyboard->UnknownFunction48a240(modifier);
    if (modifier == 0 || modifier == (int)0x80000000 || modifier == 0x3f)
        return 1;
    return 0;
}

// 0x00489980
int JoystickDevice::UnknownVirtualSlot2(int button, int modifier, UnknownInputEntry* entry) {
    if (!ModifierHeld(g_UnknownGlobal56e26c->field_0x14->field_0x34, modifier))
        return 0;
    if (field_0x264[button].state != 1)
        return 0;
    if (entry) {
        entry->state = 1;
        entry->field_0x04 = field_0x264[button].field_0x04;
        entry->field_0x08 = field_0x264[button].field_0x08;
        entry->field_0x0c = field_0x264[button].field_0x0c;
        entry->field_0x10 = field_0x264[button].field_0x10;
    }
    return 1;
}
