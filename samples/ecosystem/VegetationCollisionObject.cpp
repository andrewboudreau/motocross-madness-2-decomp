#include "../../src/reconstructed/EcoSystem.h"

// Complete 386-byte retail method; unchanged readable candidate remains nonexact.
#define ECO_SPHERE ((UnknownEcoSphereShape*)object->shapeData)
#define ECO_CAPSULE ((UnknownEcoCapsuleShape*)object->shapeData)
#define ECO_HULL ((UnknownEcoHullShape*)object->shapeData)

CollisionObject* Vegetation::GetCollisionObject(int index) {
    int table = definitionIndex;
    CollisionObject* object = g_EcoSystem->definitionTable[table]->collisionObjects[index];
    float x = quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
    float y = quantizedPosition.y * g_EcoSystem->unitsPerCoordinate;
    float z = quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
    UnknownEcoDefinition* definition = g_EcoSystem->definitionTable[definitionIndex];
    float radius = definition->RadiusForParameter(radiusParam) / definition->meanRadius;
    float height = definition->HeightForParameter(heightParam)
                   / g_EcoSystem->definitionTable[definitionIndex]->meanHeight;
    if (object->shapeType == 4) {
        ECO_SPHERE->radiusScale = radius;
        ECO_SPHERE->centreScale = height;
        ECO_SPHERE->transform(3, 0) = x;
        ECO_SPHERE->transform(3, 1) = y;
        ECO_SPHERE->transform(3, 2) = z;
    } else if (object->shapeType == 3) {
        ECO_CAPSULE->radiusScale = radius;
        ECO_CAPSULE->endpointScale = height;
        ECO_CAPSULE->transform(3, 0) = x;
        ECO_CAPSULE->transform(3, 1) = y;
        ECO_CAPSULE->transform(3, 2) = z;
    } else if (object->shapeType == 0) {
        Matrix4 m = ECO_HULL->transform;
        m(3, 1) = y;
        m(3, 0) = x;
        m(3, 2) = z;
        ECO_HULL->transform(3, 1) = y;
        ECO_HULL->transform(3, 2) = z;
        object->SetTransform(&m);
    }
    return object;
}

#undef ECO_SPHERE
#undef ECO_CAPSULE
#undef ECO_HULL
