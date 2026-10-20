// Near-miss ControlInterface candidates, kept out of src/reconstructed until
// they match. See docs/INPUT_DEVICES.md.
//
// PCControlInterface::IsDeviceButtonDown (0x004bf4f0, 88 bytes + jump
// table; 76/104 compiled inside ControlInterface.cpp, included below): the
// keyboard and mouse cases call the same slot, so their tails merge. Inside
// the unit the plain switch below shares the tail as retail does; the one
// difference left is block placement: retail keeps the keyboard case whole
// and puts the mouse case's pointer load after its `ret`, jumping back into
// the shared test, where VC6 here places the mouse load straight after the
// keyboard load (which jumps over it). Inside the unit a shared `input`
// local with a `call:` label in case 0 and `input = mouse; goto call;` in
// case 1, or both cases jumping to a label after the switch, give the same
// 76; case 1 first 74, case 1 last 64, an `if (0) { case 0: ... }` block
// 74, a ternary pointer for both cases 32. Compiled on its own (with the
// headers only) VC6 kept the tails apart (43/124) unless the goto form was
// used. Earlier standalone sweeps without goto (600 combinations: case
// order, five return/break forms per case, a `default:`, a KeyboardDevice
// cast on the mouse; 108 result-variable forms) never merged the tails, and
// neither did a mouse cast to KeyboardDevice* in both cases or one inline
// query helper called from both. /Os, /O1, /Og, /Ob0, /Ob1, /Oy-, /GF and
// /Gy left 43.

#include "../../src/reconstructed/ControlInterface.cpp"
#include "../../src/reconstructed/PCControl.h"
#include "../../src/reconstructed/JoystickDevice.h"
#include "../../src/reconstructed/KeyboardDevice.h"
#include "../../src/reconstructed/MouseDevice.h"
#include "../../src/reconstructed/TrackGame.h"

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
