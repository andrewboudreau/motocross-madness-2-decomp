// Complete nonexact immediate joystick reader; original TU ownership is unknown.
// Retail extent is 454 bytes, including the complete final ret 4.
// See docs/JOYSTICK_IMMEDIATE_STATE.md.
#include <string.h>
#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x004c3790: reads the immediate state. Non-gamepads feed the six axes to
// their bindings (axis 1 flipped by the JoyDirectionFlipped bit, axis 2
// always); button changes are queued as events, and the POV values kept.
int PCJoystickDevice::ReadImmediateState(int value) {
    UnknownJoystickState state;
    if (!device)
        return 0;
    if (device->GetDeviceState(sizeof(state), &state) < 0)
        return 0;
    if (DEVICE_SUBTYPE(deviceInfo.deviceType) != 4) {
        float axis;
        axis = (float)state.axes[0];
        axisValues[0] = axis;
        FeedAxis(0, axis, axisMaximums[0]);
        axis = axisValues[1] = (float)state.axes[1];
        if (directionFlipped)
            axis = axisMaximums[1] - axis;
        FeedAxis(1, axis, axisMaximums[1]);
        axis = (float)state.axes[2];
        axisValues[2] = axis;
        FeedAxis(2, axisMaximums[2] - axis, axisMaximums[2]);
        axis = (float)state.axes[3];
        axisValues[3] = axis;
        FeedAxis(3, axis, axisMaximums[3]);
        axis = (float)state.axes[4];
        axisValues[4] = axis;
        FeedAxis(4, axis, axisMaximums[4]);
        axis = (float)state.axes[5];
        axisValues[5] = axis;
        FeedAxis(5, axis, axisMaximums[5]);
    }
    for (int i = 0; i < 32; i++) {
        int pressed = state.buttons[i] >> 7;
        int previous = buttonStates[i].state;
        buttonStates[i].state = pressed;
        if (pressed != previous) {
            if (pressed == 0) {
                buttonStates[i].previousReleaseTime = 0;
                buttonStates[i].releaseTime = 0;
                g_TrackGame->controlInterface->QueueEvent(i, 2, 0, joystickIndex);
            } else {
                buttonStates[i].previousPressTime = 0;
                buttonStates[i].pressTime = 0;
                g_TrackGame->controlInterface->QueueEvent(i, 2, 1, joystickIndex);
            }
        }
    }
    for (int j = 0; j < 4; j++)
        povValues[j] = state.pov[j];
    return 1;
}
