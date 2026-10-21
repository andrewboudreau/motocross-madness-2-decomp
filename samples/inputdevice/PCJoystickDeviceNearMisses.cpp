// Near-miss joystick device candidates, kept out of src/reconstructed until
// they match. See docs/INPUT_DEVICES.md.
//
// PCJoystickDevice::ReadImmediateState (0x004c3790, 454 bytes): complete
// candidate is 453 bytes, with 373 matching positions. The former 452-byte
// extent truncated the final ret 4. The unchanged standalone body below is
// behaviorally reviewed; see docs/JOYSTICK_IMMEDIATE_STATE.md for the native
// comparison and its finite, scripted COM boundary.
//
// PCJoystickDevice::ReadBufferedState (0x004c3100): complete retail extent
// is 1665 bytes, including the three switch/remap tables. The former 1652
// omitted the last 13-byte remap. The unchanged candidate emits 1641 bytes
// with 128 matching positions; see docs/JOYSTICK_BUFFERED_STATE.md.
#include <string.h>

#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/TrackGame.h"

#include "JoystickImmediateState.cpp"

#include "JoystickBufferedState.cpp"
