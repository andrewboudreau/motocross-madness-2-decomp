// Near-miss PCJoystickDevice candidates, kept out of src/reconstructed until
// they match. See docs/INPUT_DEVICES.md.
//
// PCJoystickDevice::UnknownMethod4c3a10 (0x004c3a10, 206 bytes): everything
// up to the final acquire matches (179 bytes). Retail tests that call's
// result with separate `return 0` and `return 1` paths; VC6 here folds them
// into `setge`. Else, `>= 0`, result-variable, flag-variable and inline-helper
// forms do not change it.
#include <string.h>

#include "../../src/reconstructed/PCJoystickDevice.h"

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
