// DirtSprayParticleEmitter.cpp -- ownership uncertain, see ParticleEmitters.h.
#include "../../../src/reconstructed/TrackGameGlobal.h"
#include "ParticleEmitters.h"
#include <stdlib.h>


DirtSprayParticleEmitter::DirtSprayParticleEmitter(int flags) : GameObject(flags)
{
    int i;
    firstUpdate = 1;
    particleLifetime = 750.0f;
    field_0x3c = 1.0f;
    field_0x38 = 6.0f;
    emitRate = 200.0f;
    position.x = 0;
    position.y = 0;
    position.z = 0;
    launchVelocity.x = 0;
    launchVelocity.y = 2.0f;
    launchVelocity.z = 0;
    previousPosition = position;
    emitRemainder = 0;
    field_0x68 = 0;
    field_0x5c = 0;
    active = 0;
    nextFrame = 0;
    manager = 0;
    particleFlags = 0x1e;
    randomTableIndex = 0;
    randomVectorIndex = 0;
    for (i = 0; i < 256; i++)
        randomTable[i] = ((float)rand() / 32768.0f + 1.0f) * 0.25f;
    for (i = 0; i < 32; i++) {
        ParticleVec3 v;
        v.x = v.y = v.z = ((float)rand() / 32768.0f) * 0.5f + 0.75f;
        randomVectors[i] = v;
    }
}

DirtSprayParticleEmitter::~DirtSprayParticleEmitter() {}

int DirtSprayParticleEmitter::UnknownVirtualSlot27(void* context, ParticleManager* m)
{
    GameObject::SetRenderContext((void*)context);
    if (m) {
        manager = m;
        return (int)this;
    }
    Release();
    return 0;
}

// 0x004b9460.  Writes the particles straight into the manager's table: dt (plus the carried
// remainder) times the rate is the number of particles, spread along the segment moved this tick.
int DirtSprayParticleEmitter::Tick(float dt)
{
    if (reinterpret_cast<ParticleGameContext*>(g_TrackGame)->frozen == 0) {
        if (firstUpdate) {
            firstUpdate = 0;
            previousPosition = position;
            return 1;
        }
        if (active == 0) {
            emitRemainder = 0;
            field_0x68 = 0;
            return 1;
        }
        ParticleVec3 travel(position.x - previousPosition.x, position.y - previousPosition.y,
                            position.z - previousPosition.z);
        dt += emitRemainder;
        float spawnCount = dt * emitRate;
        emitRemainder = dt - spawnCount / emitRate;
        float inv = 1.0f / spawnCount;
        ParticleVec3 step = travel * inv;
        ParticleVec3 velStep = (dt * launchVelocity) * inv;
        // The scalar-first form below gives velStep's first component retail's operand order.
        ParticleVec3 gravStep = inv * (dt * manager->gravity);
        int i;
        for (i = 0; (float)i < spawnCount; i++) {
            if (manager->activeCount == 1000) return 1;
            manager->particles[manager->activeCount]->age = 375.0f;
            manager->particles[manager->activeCount]->position =
                (previousPosition + step * (float)i) + (spawnCount - (float)i) * velStep;
            manager->particles[manager->activeCount]->frame = nextFrame;
            manager->particles[manager->activeCount]->size = randomTable[randomTableIndex];
            manager->particles[manager->activeCount]->lifetime = particleLifetime;
            // Copying the step inside the loop makes its components newer leaves than the
            // (n - i) scale, so the x product loads the component first like retail
            // (`fld [esp+N]; fmul st(1)`); using gravStep directly gives `fld st(0); fmul`.
            ParticleVec3 g(gravStep.x, gravStep.y, gravStep.z);
            manager->particles[manager->activeCount]->velocity = launchVelocity + (spawnCount - (float)i) * g;
            manager->particles[manager->activeCount]->velocity.x = randomVectors[randomVectorIndex].x * manager->particles[manager->activeCount]->velocity.x;
            manager->particles[manager->activeCount]->velocity.y = randomVectors[randomVectorIndex].y * manager->particles[manager->activeCount]->velocity.y;
            manager->particles[manager->activeCount]->velocity.z = randomVectors[randomVectorIndex].z * manager->particles[manager->activeCount]->velocity.z;
            manager->particles[manager->activeCount]->flags = particleFlags;
            manager->particles[manager->activeCount]->growth = randomTable[randomTableIndex] + randomTable[randomTableIndex];
            manager->activeCount++;
            nextFrame++;
            if (nextFrame >= 0xd) nextFrame = 0;
            randomVectorIndex++;
            if (randomVectorIndex > 0x20) randomVectorIndex = 0;
            randomTableIndex++;
            if (randomTableIndex > 0x100) randomTableIndex = 0;
        }
        field_0x5c = field_0x3c;
    }
    return 1;
}
