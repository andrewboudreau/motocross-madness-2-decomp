// Complete nonexact device-query dispatcher; original TU ownership is unproven.
// Standalone VC6: 124 bytes/43 positions; aggregate: 104 bytes/76 positions.
// Complete retail extent: 104 bytes including the switch table.
// See docs/DEVICE_BUTTON_QUERY.md.
#include "../../src/reconstructed/ControlInterface.h"
#include "../../src/reconstructed/InputDevice.h"
#include "../../src/reconstructed/JoystickDevice.h"
#include "../../src/reconstructed/KeyboardDevice.h"
#include "../../src/reconstructed/MouseDevice.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/PCControl.h"

// 0x004bf4f0: `control` on one device: kind 0 the keyboard, 1 the mouse,
// 2 and 3 joystick `device`.
int PCControlInterface::IsDeviceButtonDown(int control, int kind, int modifier, int device) {
    switch (kind) {
    case 0:
        if (keyboard)
            return keyboard->IsButtonDown(control, modifier, 0);
        break;
    case 1:
        if (mouse)
            return mouse->IsButtonDown(control, modifier, 0);
        break;
    case 2:
    case 3:
        if (joysticks[device])
            return joysticks[device]->IsButtonDown(control, modifier, 0);
        break;
    }
    return 0;
}
