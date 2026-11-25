// Independently identified gameplay defaults through actual call boundaries.
// Existing view names and original TU ownership remain provisional.
#include "MovingPart.h"
#include "../krusty2/vehicle/VehicleSpeedState.h"
#include "../krusty2/vehicle/BikeRaceHandlerView.h"

MovingPart::~MovingPart() {
}

VehicleSpeedState::~VehicleSpeedState() {
}

void KbRaceHandler::NotifyContactImpulse(void*, float, int) {
}
