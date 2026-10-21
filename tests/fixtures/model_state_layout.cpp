#include <stddef.h>
#define CHECK(name, expression) typedef char name[(expression) ? 1 : -1]
#ifdef PHYSICS_VIEW
#include "../../src/krusty2/vehicle/Bike.h"
#include "../../src/krusty2/soultree/SoultreePhysicsObject.h"
CHECK(character_model, offsetof(BikeA5C4, c_0x1a0) == 0x1a0);
CHECK(bike_rider, offsetof(Bike, riderCharacter) == 0x5c4);
CHECK(vehicle_model, offsetof(Vehicle, modelNode) == 0x3bc);
extern "C" int PhysicsModelBase() {
    return (int)static_cast<D3DIMSoultreeObject*>(reinterpret_cast<SoultreePhysicsObject*>(0x1000)) - 0x1000;
}
#else
#include "../../src/reconstructed/BikeRace.h"
#include "../../src/reconstructed/SceneManager.h"
#include "../../src/reconstructed/D3DIMSoulTree.h"
CHECK(racer_model, offsetof(UnknownBikeRaceRacer, modelNode) == 0x3bc);
CHECK(racer_rider, offsetof(UnknownBikeRaceRacer, riderCharacter) == 0x5c4);
CHECK(rider_model, offsetof(UnknownBikeRaceRider, modelNode) == 0x1a0);
CHECK(scene_character_model, offsetof(UnknownSceneObject, modelNode) == 0x1a0);
CHECK(scene_car_model, offsetof(UnknownSceneAnimatedObject, carModel) == 0x34);
#endif
CHECK(model_size, sizeof(D3DIMSoultreeObject) == 0x2d8);
extern "C" int ModelGameBase() {
    return (int)static_cast<GameObject*>(reinterpret_cast<D3DIMSoultreeObject*>(0x1000)) - 0x1000;
}
