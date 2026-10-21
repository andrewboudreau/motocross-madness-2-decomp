// Complete nonexact buffered joystick reader; original TU ownership is unknown.
// Retail extent is 1665 bytes, including all three switch/remap tables.
// See docs/JOYSTICK_BUFFERED_STATE.md.
#include <string.h>
#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/TrackGame.h"

static inline int AxisAbs(int value) {
    return value < 0 ? -value : value;
}

inline int PCJoystickDevice::FilterAxisSpike(int axis, unsigned long value) {
    if (field_0x5d5) {
        int current = (int)axisValues[axis];
        if (AxisAbs(current - (int)value) > 10000 && (int)previousRawAxes[axis] == current)
            return current;
    }
    return value;
}

// 0x004c3100: reads up to 16 buffered events (re-acquiring a lost device).
// Buttons are queued on the ControlInterface; gamepads store raw axes,
// other joysticks filter spikes, apply the X/Y dead zone and feed the
// bindings; POV values are kept.
int PCJoystickDevice::ReadBufferedState(int) {
    UnknownDeviceObjectData events[16];
    long count;
    int i;
    unsigned long value;
    float axis;
    if (!device)
        return 0;
    count = 16;
    long result = device->GetDeviceData(sizeof(UnknownDeviceObjectData), events, &count, 0);
    if (result == (long)0x8007001e || result == (long)0x8007000c) {
        if (device->Acquire() < 0) {
            UnknownReportError(result, __FILE__, 317);
            return 0;
        }
        result = device->GetDeviceData(sizeof(UnknownDeviceObjectData), events, &count, 0);
    }
    if (result < 0)
        return 0;
    for (i = 0; i < count; i++) {
        int button = ButtonIndexFromOffset(events[i].offset);
        if (button != -1) {
            if (events[i].data & 0x80) {
                buttonStates[button].state = 1;
                buttonStates[button].previousPressTime = buttonStates[button].pressTime;
                buttonStates[button].pressTime = events[i].timeStamp;
                g_TrackGame->controlInterface->QueueEvent(button, 2, 1, joystickIndex);
            } else {
                buttonStates[button].state = 0;
                buttonStates[button].previousReleaseTime = buttonStates[button].releaseTime;
                buttonStates[button].releaseTime = events[i].timeStamp;
                g_TrackGame->controlInterface->QueueEvent(button, 2, 0, joystickIndex);
            }
            continue;
        }
        if (DEVICE_SUBTYPE(deviceInfo.deviceType) == 4) {
            switch (events[i].offset) {
            case 0:
                axisValues[0] = (float)events[i].data;
                break;
            case 4:
                axisValues[1] = (float)events[i].data;
                break;
            case 8:
                axisValues[2] = (float)events[i].data;
                break;
            case 12:
                axisValues[3] = (float)events[i].data;
                break;
            case 16:
                axisValues[4] = (float)events[i].data;
                break;
            case 20:
                axisValues[5] = (float)events[i].data;
                break;
            }
        } else {
            switch (events[i].offset) {
            case 0:
                value = ApplyAxisDeadZone(0, FilterAxisSpike(0, events[i].data));
                axis = axisValues[0] = (float)value;
                previousRawAxes[0] = (float)events[i].data;
                FeedAxis(0, axis, axisMaximums[0]);
                break;
            case 4:
                value = ApplyAxisDeadZone(1, FilterAxisSpike(1, events[i].data));
                axis = axisValues[1] = (float)value;
                previousRawAxes[1] = (float)events[i].data;
                if (directionFlipped)
                    axis = axisMaximums[1] - axis;
                FeedAxis(1, axis, axisMaximums[1]);
                break;
            case 8:
                value = FilterAxisSpike(2, events[i].data);
                axis = axisValues[2] = (float)value;
                previousRawAxes[2] = (float)events[i].data;
                FeedAxis(2, axisMaximums[2] - axis, axisMaximums[2]);
                break;
            case 12:
                value = FilterAxisSpike(3, events[i].data);
                axis = axisValues[3] = (float)value;
                previousRawAxes[3] = (float)events[i].data;
                FeedAxis(3, axis, axisMaximums[3]);
                break;
            case 16:
                value = FilterAxisSpike(4, events[i].data);
                axis = axisValues[4] = (float)value;
                previousRawAxes[4] = (float)events[i].data;
                FeedAxis(4, axis, axisMaximums[4]);
                break;
            case 20:
                value = FilterAxisSpike(5, events[i].data);
                axis = axisValues[5] = (float)value;
                previousRawAxes[5] = (float)events[i].data;
                FeedAxis(5, axis, axisMaximums[5]);
                break;
            }
        }
        switch (events[i].offset) {
        case 0x20:
            povValues[0] = events[i].data;
            break;
        case 0x24:
            povValues[1] = events[i].data;
            break;
        case 0x28:
            povValues[2] = events[i].data;
            break;
        case 0x2c:
            povValues[3] = events[i].data;
            break;
        }
    }
    return 1;
}
