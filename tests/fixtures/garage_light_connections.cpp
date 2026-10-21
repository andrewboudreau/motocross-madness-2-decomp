#include "LightEmitter.h"
#include <stddef.h>

// The garage allocates these complete objects, then appends their GameObject
// bases and registers each emitter in the manager's pointer array.
typedef char EmitterSize[sizeof(LightEmitter) == 0xac ? 1 : -1];
typedef char ManagerSize[sizeof(LightManager) == 0x11c ? 1 : -1];
typedef char EmitterOwner[offsetof(LightEmitter, manager) == 0xa4 ? 1 : -1];
typedef char ManagerCount[offsetof(LightManager, lightCount) == 0x2c ? 1 : -1];
typedef char ManagerLights[offsetof(LightManager, lights) == 0x30 ? 1 : -1];

GameObject* GarageEmitterBase(LightEmitter* emitter) { return emitter; }
GameObject* GarageManagerBase(LightManager* manager) { return manager; }
