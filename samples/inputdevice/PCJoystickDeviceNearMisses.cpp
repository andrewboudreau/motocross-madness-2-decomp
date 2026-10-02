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
//
// PCJoystickDevice::UnknownMethod4c3100 (0x004c3100, 1652 bytes with jump
// tables): the buffered reader. Its control flow and switches line up (a
// register-insensitive comparison gives 0.81), but VC6 here keeps 0 in a
// register for the whole function (ebp), while retail uses immediate zeros
// and enregisters the values in ebx/ebp inside the loop. Abs must be an
// inline ternary (retail uses jns/neg, not the cdq abs intrinsic).
#include <string.h>

#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/TrackGame.h"

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

static inline int AxisAbs(int value) {
    return value < 0 ? -value : value;
}

inline int PCJoystickDevice::FilterAxisSpike(int axis, unsigned long value) {
    if (field_0x5d5) {
        int current = (int)field_0x4e4[axis];
        if (AxisAbs(current - (int)value) > 10000 && (int)field_0x5a4[axis] == current)
            return current;
    }
    return value;
}

// 0x004c3100: reads up to 16 buffered events (re-acquiring a lost device).
// Buttons are queued on the ControlInterface; gamepads store raw axes,
// other joysticks filter spikes, apply the X/Y dead zone and feed the
// bindings; POV values are kept.
int PCJoystickDevice::UnknownMethod4c3100(int) {
    UnknownDeviceObjectData events[16];
    long count;
    int i;
    unsigned long value;
    float axis;
    if (!field_0x25c)
        return 0;
    count = 16;
    long result = field_0x25c->UnknownMethod10(sizeof(UnknownDeviceObjectData), events, &count, 0);
    if (result == (long)0x8007001e || result == (long)0x8007000c) {
        if (field_0x25c->UnknownMethod7() < 0) {
            UnknownReportError(result, __FILE__, 317);
            return 0;
        }
        result = field_0x25c->UnknownMethod10(sizeof(UnknownDeviceObjectData), events, &count, 0);
    }
    if (result < 0)
        return 0;
    for (i = 0; i < count; i++) {
        int button = UnknownFunction4c2ef0(events[i].offset);
        if (button != -1) {
            if (events[i].data & 0x80) {
                field_0x264[button].state = 1;
                field_0x264[button].field_0x10 = field_0x264[button].field_0x08;
                field_0x264[button].field_0x08 = events[i].timeStamp;
                g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(button, 2, 1, field_0x260);
            } else {
                field_0x264[button].state = 0;
                field_0x264[button].field_0x0c = field_0x264[button].field_0x04;
                field_0x264[button].field_0x04 = events[i].timeStamp;
                g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(button, 2, 0, field_0x260);
            }
            continue;
        }
        if (DEVICE_SUBTYPE(field_0x18.deviceType) == 4) {
            switch (events[i].offset) {
            case 0:
                field_0x4e4[0] = (float)events[i].data;
                break;
            case 4:
                field_0x4e4[1] = (float)events[i].data;
                break;
            case 8:
                field_0x4e4[2] = (float)events[i].data;
                break;
            case 12:
                field_0x4e4[3] = (float)events[i].data;
                break;
            case 16:
                field_0x4e4[4] = (float)events[i].data;
                break;
            case 20:
                field_0x4e4[5] = (float)events[i].data;
                break;
            }
        } else {
            switch (events[i].offset) {
            case 0:
                value = UnknownFunction4c3090(0, FilterAxisSpike(0, events[i].data));
                axis = field_0x4e4[0] = (float)value;
                field_0x5a4[0] = (float)events[i].data;
                UnknownFunction489c00(0, axis, field_0x58c[0]);
                break;
            case 4:
                value = UnknownFunction4c3090(1, FilterAxisSpike(1, events[i].data));
                axis = field_0x4e4[1] = (float)value;
                field_0x5a4[1] = (float)events[i].data;
                if (field_0x574_bit0)
                    axis = field_0x58c[1] - axis;
                UnknownFunction489c00(1, axis, field_0x58c[1]);
                break;
            case 8:
                value = FilterAxisSpike(2, events[i].data);
                axis = field_0x4e4[2] = (float)value;
                field_0x5a4[2] = (float)events[i].data;
                UnknownFunction489c00(2, field_0x58c[2] - axis, field_0x58c[2]);
                break;
            case 12:
                value = FilterAxisSpike(3, events[i].data);
                axis = field_0x4e4[3] = (float)value;
                field_0x5a4[3] = (float)events[i].data;
                UnknownFunction489c00(3, axis, field_0x58c[3]);
                break;
            case 16:
                value = FilterAxisSpike(4, events[i].data);
                axis = field_0x4e4[4] = (float)value;
                field_0x5a4[4] = (float)events[i].data;
                UnknownFunction489c00(4, axis, field_0x58c[4]);
                break;
            case 20:
                value = FilterAxisSpike(5, events[i].data);
                axis = field_0x4e4[5] = (float)value;
                field_0x5a4[5] = (float)events[i].data;
                UnknownFunction489c00(5, axis, field_0x58c[5]);
                break;
            }
        }
        switch (events[i].offset) {
        case 0x20:
            field_0x5d8[0] = events[i].data;
            break;
        case 0x24:
            field_0x5d8[1] = events[i].data;
            break;
        case 0x28:
            field_0x5d8[2] = events[i].data;
            break;
        case 0x2c:
            field_0x5d8[3] = events[i].data;
            break;
        }
    }
    return 1;
}
