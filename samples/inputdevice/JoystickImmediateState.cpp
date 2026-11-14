// Complete nonexact immediate joystick reader; original TU ownership is unknown.
// Retail extent is 454 bytes, including the complete final ret 4.
// See docs/JOYSTICK_IMMEDIATE_STATE.md.
#include <string.h>
#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x004c3790: reads the immediate state. Non-gamepads feed the six axes to
// their bindings (axis 1 flipped by the JoyDirectionFlipped bit, axis 2
// always); button changes are queued as events, and the POV values kept.
// 453 of 454 (454 bytes): the new state is assigned and copied in one
// statement (`int pressed = buttonStates[i].state = ...`) and the release
// arm passes that copy, which keeps eax busy while the arguments are
// loaded and gives retail's ecx/edx, eax/ecx rotation through the loop and
// the POV copy. Left: retail pushes the zero register (`push ebp`) for that
// argument where this pushes eax. A plain `pressed` local stored by its own
// statement is replaced by 0 after the `== 0` test, so eax is free again
// (373, whether the argument is `pressed` or 0); the chained copy is not.
// Unchanged or worse (2026-11-13): a result local, a subtype local, the
// previous/pressed order and either inlined, an unsigned char pressed
// (309), joystick index and ControlInterface locals (function, loop or
// arm scope), explicit POV stores (460 bytes), a shared loop index,
// chained zero stores (369), swapped branches (356), `!pressed` and a
// button pointer; and with the chained copy: the release test or the
// changed test on `buttonStates[i].state`, `!pressed`, `pressed < 1`,
// `pressed != 1`, a `switch`, `if (previous)`, `continue` on no change, a
// `changed` local (`!=` or `^`), the argument as `pressed != 0`,
// `pressed ? 1 : 0`, `!!pressed`, `pressed & 1` (457), `pressed == 1`
// (461) or `buttonStates[i].state`, the press arm passing `pressed` (433,
// one merged call), zero stores from an `int time = 0` local, chained, or
// storing `pressed` (452).
int PCJoystickDevice::ReadImmediateState(int value) {
    DIJoyState state;
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
        int previous = buttonStates[i].state;
        int pressed = buttonStates[i].state = state.buttons[i] >> 7;
        if (pressed != previous) {
            if (pressed == 0) {
                buttonStates[i].previousReleaseTime = 0;
                buttonStates[i].releaseTime = 0;
                g_TrackGame->controlInterface->QueueEvent(i, 2, pressed, joystickIndex);
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
