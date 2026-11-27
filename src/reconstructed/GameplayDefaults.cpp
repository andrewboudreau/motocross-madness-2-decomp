// Independently identified gameplay defaults through actual call boundaries.
// Existing view names and original TU ownership remain provisional.
#include "MovingPart.h"
#include "../krusty2/vehicle/VehicleSpeedState.h"
#include "RaceSound.h"

MovingPart::~MovingPart() {
}

VehicleSpeedState::~VehicleSpeedState() {
}

void RaceSound::NotifyContactImpulse(void*, float, int) {
}
