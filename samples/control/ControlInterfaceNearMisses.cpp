// Historical aggregate compilation context for the nonexact device query.
// It includes canonical definitions and must not be linked into the game.
// The unchanged query emits 104 bytes/76 matching positions here, but
// 124 bytes/43 positions as its standalone candidate. Retail is 104 bytes.
// See docs/DEVICE_BUTTON_QUERY.md for identity, preservation and native proof.

#include "../../src/reconstructed/ControlInterface.cpp"
#include "../../src/reconstructed/PCControl.h"
#include "../../src/reconstructed/JoystickDevice.h"
#include "../../src/reconstructed/KeyboardDevice.h"
#include "../../src/reconstructed/MouseDevice.h"
#include "../../src/reconstructed/TrackGame.h"

#include "DeviceButtonQuery.cpp"
