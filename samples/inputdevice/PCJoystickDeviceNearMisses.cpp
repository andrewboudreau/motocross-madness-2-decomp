// Near-miss joystick device candidates, kept out of src/reconstructed until
// they match. See docs/INPUT_DEVICES.md.
//
// PCJoystickDevice::UnknownMethod4c3790 (0x004c3790, 452 bytes): 373 bytes
// match. The axes match only with `axis = field = (float)value` chains, and
// the POV copy only as a loop (memcpy and four statements place `mov eax, 1`
// differently). Retail then rotates registers differently in both
// button-event calls (field_0x260 in ecx and the global in edx, not eax and
// ecx) and in the POV copy. Pointer, unsigned, store-in-branch and
// device-local forms do not fix it.
#include <string.h>

#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/UnknownObject56e26c.h"

// 0x004c3790: reads the immediate state. Non-gamepads feed the six axes to
// their bindings (axis 1 flipped by the JoyDirectionFlipped bit, axis 2
// always); button changes are queued as events, and the POV values kept.
int PCJoystickDevice::UnknownMethod4c3790(int value) {
    UnknownJoystickState state;
    if (!field_0x25c)
        return 0;
    if (field_0x25c->UnknownMethod9(sizeof(state), &state) < 0)
        return 0;
    if (DEVICE_SUBTYPE(field_0x18.deviceType) != 4) {
        float axis;
        axis = (float)state.axes[0];
        field_0x4e4[0] = axis;
        UnknownFunction489c00(0, axis, field_0x58c[0]);
        axis = field_0x4e4[1] = (float)state.axes[1];
        if (field_0x574_bit0)
            axis = field_0x58c[1] - axis;
        UnknownFunction489c00(1, axis, field_0x58c[1]);
        axis = (float)state.axes[2];
        field_0x4e4[2] = axis;
        UnknownFunction489c00(2, field_0x58c[2] - axis, field_0x58c[2]);
        axis = (float)state.axes[3];
        field_0x4e4[3] = axis;
        UnknownFunction489c00(3, axis, field_0x58c[3]);
        axis = (float)state.axes[4];
        field_0x4e4[4] = axis;
        UnknownFunction489c00(4, axis, field_0x58c[4]);
        axis = (float)state.axes[5];
        field_0x4e4[5] = axis;
        UnknownFunction489c00(5, axis, field_0x58c[5]);
    }
    for (int i = 0; i < 32; i++) {
        int pressed = state.buttons[i] >> 7;
        int previous = field_0x264[i].state;
        field_0x264[i].state = pressed;
        if (pressed != previous) {
            if (pressed == 0) {
                field_0x264[i].field_0x0c = 0;
                field_0x264[i].field_0x04 = 0;
                g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(i, 2, 0, field_0x260);
            } else {
                field_0x264[i].field_0x10 = 0;
                field_0x264[i].field_0x08 = 0;
                g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(i, 2, 1, field_0x260);
            }
        }
    }
    for (int j = 0; j < 4; j++)
        field_0x5d8[j] = state.pov[j];
    return 1;
}
