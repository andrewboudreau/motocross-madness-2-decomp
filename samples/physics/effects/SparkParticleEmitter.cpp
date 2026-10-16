// SparkParticleEmitter.cpp -- ownership uncertain, see ParticleEmitters.h.
#include "ParticleEmitters.h"
#include <stdlib.h>

extern ParticleGameContext* g_gameContext;   // 0x0056e26c

SparkParticleEmitter::SparkParticleEmitter(int flags) : GameObject(flags)
{
    int i;
    firstUpdate = 1;
    particleLifetime = 500.0f;
    field_0x3c = 1.0f;
    field_0x38 = 0;
    emitRate = 2400.0f;
    position.x = 0;
    position.y = 0;
    position.z = 0;
    launchVelocity.x = 0;
    launchVelocity.y = 2.0f;
    launchVelocity.z = 0;
    previousPosition = position;
    emitRemainder = 0;
    field_0x6c = 0;
    field_0x5c = 0;
    burstPending = 0;
    idleTime = 0;
    nextFrame = 0x2d;
    manager = 0;
    particleFlags = 0x14;
    randomTableIndex = 0;
    randomVectorIndex = 0;
    for (i = 0; i < 256; i++)
        randomTable[i] = ((float)rand() / 32768.0f) * 0.075f;
    for (i = 0; i < 8; i++) {
        randomVectors[i].x = ((float)rand() / 32768.0f) * 10.0f - 5.0f;
        randomVectors[i].y = ((float)rand() / 32768.0f) * 10.0f - 5.0f;
        randomVectors[i].z = ((float)rand() / 32768.0f) * 10.0f - 5.0f;
    }
}

SparkParticleEmitter::~SparkParticleEmitter() {}

int SparkParticleEmitter::UnknownVirtualSlot27(void* a, ParticleManager* m)
{
    GameObject::SetRenderContext((int)a);
    if (m) {
        manager = m;
        return (int)this;
    }
    BaseObjectVirtualSlot2();
    return 0;
}

void SparkParticleEmitter::SetPosition(ParticleVec3 p)
{
    previousPosition = position;
    position = p;
}

// 0x004b99a0.  Like DirtChunk, but fires once while burstPending is set and then switches itself off.
int SparkParticleEmitter::Tick(float dt)
{
    if (g_gameContext->frozen == 0) {
        if (firstUpdate) {
            firstUpdate = 0;
            previousPosition = position;
            return 1;
        }
        if (burstPending == 0) {
            idleTime = dt + idleTime;
            emitRemainder = 0;
            field_0x6c = 0;
            return 1;
        }
        if (idleTime > 0.25)
            previousPosition = position;
        ParticleVec3 d(position.x - previousPosition.x, position.y - previousPosition.y, position.z - previousPosition.z);
        dt += emitRemainder;
        float n = dt * emitRate;
        emitRemainder = dt - n / emitRate;
        float inv = 1.0f / n;
        ParticleVec3 step = d * inv;
        ParticleVec3 velStep = (dt * launchVelocity) * inv;
        // The scalar-first form below gives velStep's first component retail's operand order.
        ParticleVec3 gravStep = inv * (dt * manager->gravity);
        int i;
        for (i = 0; (float)i < n; i++) {
            if (manager->activeCount == 1000) {
                field_0x5c = field_0x3c;
                burstPending = 0;
                idleTime = 0;
                return 1;
            }
            manager->particles[manager->activeCount]->age = 0;
            manager->particles[manager->activeCount]->position = (previousPosition + step * (float)i) + (n - (float)i) * velStep;
            manager->particles[manager->activeCount]->frame = nextFrame;
            manager->particles[manager->activeCount]->size = randomTable[randomTableIndex];
            manager->particles[manager->activeCount]->lifetime = particleLifetime;
            // Copying the step inside the loop makes its components newer leaves than the
            // (n - i) scale, so the x product loads the component first like retail
            // (`fld [esp+N]; fmul st(1)`); using gravStep directly gives `fld st(0); fmul`.
            ParticleVec3 g(gravStep.x, gravStep.y, gravStep.z);
            manager->particles[manager->activeCount]->velocity = launchVelocity + (n - (float)i) * g;
            manager->particles[manager->activeCount]->velocity.x = randomVectors[randomVectorIndex].x;
            manager->particles[manager->activeCount]->velocity.y = randomVectors[randomVectorIndex].y;
            manager->particles[manager->activeCount]->velocity.z = randomVectors[randomVectorIndex].z;
            manager->particles[manager->activeCount]->flags = particleFlags;
            manager->particles[manager->activeCount]->growth = randomTable[randomTableIndex] + randomTable[randomTableIndex];
            manager->activeCount++;
            nextFrame++;
            if (nextFrame >= 0x3d) nextFrame = 0x2d;
            randomVectorIndex++;
            if (randomVectorIndex > 0x20) randomVectorIndex = 0;
            randomTableIndex++;
            if (randomTableIndex > 0x100) randomTableIndex = 0;
        }
        field_0x5c = field_0x3c;
        burstPending = 0;
        idleTime = 0;
    }
    return 1;
}
