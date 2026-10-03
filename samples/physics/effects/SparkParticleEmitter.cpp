// SparkParticleEmitter.cpp -- ownership uncertain, see ParticleEmitters.h.
#include "ParticleEmitters.h"
#include <stdlib.h>

extern ParticleGameContext* g_gameContext;   // 0x0056e26c

SparkParticleEmitter::SparkParticleEmitter(int flags) : GameObject(flags)
{
    int i;
    firstUpdate = 1;
    manager = 0;
    field_0x34 = 500.0f;
    field_0x3c = 1.0f;
    field_0x38 = 0;
    field_0x40 = 2400.0f;
    launchVelocity.x = 0;
    launchVelocity.y = 2.0f;
    launchVelocity.z = 0;
    field_0x68 = 0;
    field_0x6c = 0;
    field_0x5c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    field_0x70 = 0x2d;
    field_0x74 = 0x14;
    field_0x484 = 0;
    field_0x608 = 0;
    for (i = 0; i < 256; i++)
        randomTable[i] = (float)rand() / 32768.0f * 0.075f;
    for (i = 0; i < 8; i++) {
        randomVectors[i].x = (float)rand() / 32768.0f * 10.0f - 5.0f;
        randomVectors[i].y = (float)rand() / 32768.0f * 10.0f - 5.0f;
        randomVectors[i].z = (float)rand() / 32768.0f * 10.0f - 5.0f;
    }
}

SparkParticleEmitter::~SparkParticleEmitter() {}

int SparkParticleEmitter::UnknownVirtualSlot27(void* a, ParticleManager* m)
{
    GameObject::GameObjectVirtualSlot8((int)a);
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

// 0x004b99a0.  Like DirtChunk, but fires once while field_0x64 is positive and then switches itself off.
int SparkParticleEmitter::GameObjectVirtualSlot10(float dt)
{
    if (g_gameContext->frozen == 0) {
        if (firstUpdate) {
            firstUpdate = 0;
            previousPosition = position;
            return 1;
        }
        if (field_0x60 == 0) {
            field_0x64 = dt + field_0x64;
            field_0x68 = 0;
            field_0x6c = 0;
            return 1;
        }
        if (field_0x64 > 0.0)
            previousPosition = position;
        ParticleVec3 d(position.x - previousPosition.x, position.y - previousPosition.y, position.z - previousPosition.z);
        dt += field_0x68;
        float n = dt * field_0x40;
        field_0x68 = dt - n / field_0x40;
        float inv = 1.0f / n;
        ParticleVec3 step = d * inv;
        ParticleVec3 velStep = (dt * launchVelocity) * inv;
        ParticleVec3 gravStep = (dt * manager->gravity) * inv;
        int i;
        for (i = 0; (float)i < n; i++) {
            if (manager->activeCount == 1000) {
                field_0x5c = field_0x3c;
                field_0x60 = 0;
                field_0x64 = 0;
                return 1;
            }
            manager->particles[manager->activeCount]->age = 0;
            manager->particles[manager->activeCount]->position = (previousPosition + step * (float)i) + (n - (float)i) * velStep;
            manager->particles[manager->activeCount]->frame = field_0x70;
            manager->particles[manager->activeCount]->size = randomTable[field_0x484];
            manager->particles[manager->activeCount]->lifetime = field_0x34;
            manager->particles[manager->activeCount]->velocity = launchVelocity + (n - (float)i) * gravStep;
            manager->particles[manager->activeCount]->velocity.x = randomVectors[field_0x608].x;
            manager->particles[manager->activeCount]->velocity.y = randomVectors[field_0x608].y;
            manager->particles[manager->activeCount]->velocity.z = randomVectors[field_0x608].z;
            manager->particles[manager->activeCount]->flags = field_0x74;
            manager->particles[manager->activeCount]->growth = randomTable[field_0x484] + randomTable[field_0x484];
            manager->activeCount++;
            field_0x70++;
            if (field_0x70 >= 0x3d) field_0x70 = 0x2d;
            field_0x608++;
            if (field_0x608 > 0x20) field_0x608 = 0;
            field_0x484++;
            if (field_0x484 > 0x100) field_0x484 = 0;
        }
        field_0x5c = field_0x3c;
        field_0x60 = 0;
        field_0x64 = 0;
    }
    return 1;
}
