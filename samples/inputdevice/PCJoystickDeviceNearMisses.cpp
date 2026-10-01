// Near-miss PCJoystickDevice candidates, kept out of src/reconstructed until
// they match. See docs/INPUT_DEVICES.md.
//
// PCJoystickDevice::UnknownVirtualSlot19 (0x004c3af0, 46 bytes): retail
// zero-extends the byte at +0x3d (`xor eax, eax; mov al, [ecx+0x3d]`) and then
// repeats the mask with `and eax, 0xff`; VC6 here omits the mask, which shifts
// every later byte. An int, unsigned or unsigned char local, an explicit
// `& 0xff`, a char cast, an inline byte accessor, an 8-bit bitfield and
// switch forms all fail to reproduce it.
#include "../../src/reconstructed/PCJoystickDevice.h"

// 0x004c3af0: device states 4-7 are recorded in +0x10.
void PCJoystickDevice::UnknownVirtualSlot19() {
    int state = field_0x18[0x25];
    if (state == 4)
        field_0x10 = state;
    else if (state == 5)
        field_0x10 = state;
    else if (state == 6)
        field_0x10 = state;
    else if (state == 7)
        field_0x10 = state;
}
