#include <stddef.h>
#include "../../src/reconstructed/CarProcedural.h"

// Independently observed retail sizes and offsets, including the secondary
// GraphicsTest/GameObject base. No game implementation is replaced here.
#define CHECK_LAYOUT(name, expression) typedef char name[(expression) ? 1 : -1]
CHECK_LAYOUT(graphics_extent, sizeof(GraphicsTest) == 0x34);
CHECK_LAYOUT(collision_extent, sizeof(CollisionObject) == 0xb8);
CHECK_LAYOUT(collision_center, offsetof(CollisionObject, boundCenter) == 0x40);
CHECK_LAYOUT(collision_radius, offsetof(CollisionObject, boundRadius) == 0x4c);
CHECK_LAYOUT(collision_shape, offsetof(CollisionObject, shape) == 0x54);
CHECK_LAYOUT(collision_contact, offsetof(CollisionObject, contactRecord) == 0x5c);
CHECK_LAYOUT(collision_ignore, offsetof(CollisionObject, ignoreVegetation) == 0x68);
CHECK_LAYOUT(collision_callback, offsetof(CollisionObject, onHitCallback) == 0x88);
CHECK_LAYOUT(wrecker_model_extent, sizeof(UnknownWreckerCollisionModel) == 0x10c);
CHECK_LAYOUT(wrecker_model_manual, offsetof(UnknownWreckerCollisionModel, manualStep) == 0xe8);
CHECK_LAYOUT(wrecker_model_probes, offsetof(UnknownWreckerCollisionModel, probes) == 0x104);
CHECK_LAYOUT(wrecker_field, offsetof(Wrecker, character) == 0x34);
CHECK_LAYOUT(car_field, offsetof(CarProcedural, carModel) == 0x34);
