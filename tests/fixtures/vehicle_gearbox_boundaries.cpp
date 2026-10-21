#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/krusty2/vehicle/Bike.h"
#include "../../src/reconstructed/GearRatios.h"

#define CHECK(name, condition) typedef char name[(condition) ? 1 : -1]
CHECK(engine_size, sizeof(UnknownEngineGearbox) == 0x1e4);
CHECK(view_size, sizeof(VehicleSpeedState) == 0x1e4);
#define FIELD(view, provider) CHECK(offset_##view, offsetof(VehicleSpeedState, view) == offsetof(UnknownEngineGearbox, provider))
FIELD(throttleFraction, throttleFraction); FIELD(currentRpm, currentRpm);
FIELD(driveCut, driveCut); FIELD(gearTimer, shiftTimer); FIELD(gear, currentGear);
FIELD(shiftRanges, shiftRanges); FIELD(shiftWindowWidths, shiftWindowWidths);
FIELD(randomStart, upShiftRpm); FIELD(lastUpdateRpm, lastUpdateRpm);
CHECK(aux_field8c, offsetof(VehicleWheelAux, field_0x8c) == offsetof(UnknownEngineGearbox, field_0x8c));
CHECK(aux_field90, offsetof(VehicleWheelAux, field_0x90) == offsetof(UnknownEngineGearbox, gearCount));
CHECK(vehicle_aux, offsetof(VehicleWheel, aux) == 0x2a8);
CHECK(bike_aux, offsetof(BikeWheel, aux) == 0x2a8);
CHECK(vehicle_magnitude, offsetof(VehicleWheel, field_0x280) == 0x280);
CHECK(bike_magnitude, offsetof(BikeWheel, driveForceMagnitude) == 0x280);
CHECK(vector_size, sizeof(Vec3) == sizeof(Vector3) && sizeof(Vec3) == 12);
CHECK(vector_x, offsetof(Vec3, x) == offsetof(Vector3, x));
CHECK(vector_y, offsetof(Vec3, y) == offsetof(Vector3, y));
CHECK(vector_z, offsetof(Vec3, z) == offsetof(Vector3, z));

int main() {
    static const unsigned bits[] = {0, 0x80000000, 0x3f800000, 0x7f800000, 0x7fc12345, 0xffffffff};
    static const unsigned char cuts[] = {0, 1, 2, 127, 128, 255};
    int count = 0;
    for (int b = 0; b < 6; ++b) {
        float gearArg;
        memcpy(&gearArg, &bits[b], 4);
        UnknownEngineGearbox engine("fixture", 0.02f, 0, 0, 0, 0,
            reinterpret_cast<const int&>(gearArg), 2.0f, 3.0f);
        if ((unsigned)engine.field_0x1d8 != bits[b]) return 1;
        VehicleSpeedState* view = reinterpret_cast<VehicleSpeedState*>(&engine);
        for (int c = 0; c < 6; ++c) {
            engine.throttleFraction = 0.0f;
            engine.shiftTimer = 0.0f;
            reinterpret_cast<UnknownEngineGearbox*>(view)->UpdateThrottle(0.25f, 1, cuts[c]);
            if (engine.driveCut != cuts[c] || engine.throttleFraction != 0.5f) return 2;
            ++count;
        }
        union { double align; unsigned char bytes[sizeof(VehicleWheel)]; } storage;
        VehicleWheel* wheel = reinterpret_cast<VehicleWheel*>(storage.bytes);
        memset(storage.bytes, 0x5a, sizeof(storage.bytes));
        wheel->aux = reinterpret_cast<VehicleWheelAux*>(&engine);
        wheel->rollDirection.x = 1.0f; wheel->rollDirection.y = -2.0f; wheel->rollDirection.z = 3.0f;
        engine.driveCut = 1;
        for (int reverse = 0; reverse < 2; ++reverse) {
            reinterpret_cast<UnknownEngineGearbox*>(wheel->aux)->DriveForce(1.0f,
                reinterpret_cast<const Vector3*>(&wheel->rollDirection), 0.5f, (char)reverse, 100.0f,
                reinterpret_cast<Vector3*>(&wheel->driveForce), reinterpret_cast<float*>(&wheel->field_0x280));
            if (wheel->field_0x280.x != (reverse ? 100.0f : 0.0f)) return 3;
            if (wheel->driveForce.x != (reverse ? -100.0f : 0.0f) ||
                wheel->driveForce.y != (reverse ? 200.0f : 0.0f) ||
                wheel->driveForce.z != (reverse ? -300.0f : 0.0f)) return 4;
            for (int i = 0x284; i < 0x28c; ++i) if (storage.bytes[i] != 0x5a) return 5;
            ++count;
        }
    }
    printf("Gearbox boundaries: %d native cases passed\n", count);
    return 0;
}
