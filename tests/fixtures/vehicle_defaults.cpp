#include <stdio.h>
#include <string.h>
#include "vehicle/KrustyBike.h"

#define CHECK(name, condition) typedef char name[(condition) ? 1 : -1]
CHECK(vehicle_size, sizeof(Vehicle) == 0x5ec);
CHECK(bike_size, sizeof(Bike) == 0x764);
CHECK(krustybike_size, sizeof(KrustyBike) == 0x1638);

int main() {
    union { double align; unsigned char bytes[sizeof(KrustyBike) + 32]; } actual, expected;
    for (int seed = 0; seed < 256; ++seed) {
        for (unsigned i = 0; i < sizeof(actual.bytes); ++i)
            actual.bytes[i] = (unsigned char)(seed + 37 * i);
        memcpy(expected.bytes, actual.bytes, sizeof(actual.bytes));
        Vehicle* view = reinterpret_cast<Vehicle*>(actual.bytes + 16);
        // Test the actual defaults; constructed-object dispatch is checked
        // separately through the production compiler's complete vtables.
        view->Vehicle::UnknownVirtualSlot87();
        if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 1;
        view->Vehicle::UnknownVirtualSlot94();
        if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 2;
    }
    printf("Vehicle defaults: 512 complete-storage calls passed\n");
    return 0;
}
