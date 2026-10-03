// DirtSprayParticleEmitter.cpp -- ownership uncertain, see ParticleEmitters.h.
#include "ParticleEmitters.h"
#include <stdlib.h>

extern ParticleGameContext* g_gameContext;   // 0x0056e26c

DirtSprayParticleEmitter::DirtSprayParticleEmitter(int flags) : GameObject(flags)
{
    int i;
    firstUpdate = 1;
    manager = 0;
    field_0x34 = 750.0f;
    field_0x38 = 6.0f;
    field_0x3c = 1.0f;
    field_0x40 = 200.0f;
    launchVelocity.x = 0;
    launchVelocity.y = 2.0f;
    launchVelocity.z = 0;
    field_0x64 = 0;
    field_0x68 = 0;
    field_0x5c = 0;
    field_0x60 = 0;
    field_0x6c = 0;
    field_0x70 = 0x1e;
    field_0x480 = 0;
    field_0x604 = 0;
    for (i = 0; i < 256; i++)
        randomTable[i] = ((float)rand() / 32768.0f + 1.0f) * 0.25f;
    for (i = 0; i < 32; i++) {
        float v = (float)rand() / 32768.0f * 0.5f + 0.75f;
        randomVectors[i] = ParticleVec3(v, v, v);
    }
}

DirtSprayParticleEmitter::~DirtSprayParticleEmitter() {}

int DirtSprayParticleEmitter::UnknownVirtualSlot27(void* a, ParticleManager* m)
{
    GameObject::GameObjectVirtualSlot8((int)a);
    if (m) {
        manager = m;
        return (int)this;
    }
    BaseObjectVirtualSlot2();
    return 0;
}

// 0x004b9460.  Writes the particles straight into the manager's table: dt (plus the carried
// remainder) times the rate is the number of particles, spread along the segment moved this tick.
int DirtSprayParticleEmitter::GameObjectVirtualSlot10(float dt)
{
    if (g_gameContext->frozen == 0) {
        if (firstUpdate) {
            firstUpdate = 0;
            previousPosition = position;
            return 1;
        }
        if (field_0x60 == 0) {
            field_0x64 = 0;
            field_0x68 = 0;
            return 1;
        }
        ParticleVec3 d(position.x - previousPosition.x, position.y - previousPosition.y, position.z - previousPosition.z);
        dt += field_0x64;
        float n = dt * field_0x40;
        field_0x64 = dt - n / field_0x40;
        float inv = 1.0f / n;
        ParticleVec3 step = d * inv;
        ParticleVec3 velStep = (dt * launchVelocity) * inv;
        ParticleVec3 gravStep = (dt * manager->gravity) * inv;
        int i;
        for (i = 0; (float)i < n; i++) {
            if (manager->activeCount == 1000) return 1;
            manager->particles[manager->activeCount]->age = 375.0f;
            manager->particles[manager->activeCount]->position = (previousPosition + step * (float)i) + (n - (float)i) * velStep;
            manager->particles[manager->activeCount]->frame = field_0x6c;
            manager->particles[manager->activeCount]->size = randomTable[field_0x480];
            manager->particles[manager->activeCount]->lifetime = field_0x34;
            manager->particles[manager->activeCount]->velocity = launchVelocity + (n - (float)i) * gravStep;
            manager->particles[manager->activeCount]->velocity.x = randomVectors[field_0x604].x * manager->particles[manager->activeCount]->velocity.x;
            manager->particles[manager->activeCount]->velocity.y = randomVectors[field_0x604].y * manager->particles[manager->activeCount]->velocity.y;
            manager->particles[manager->activeCount]->velocity.z = randomVectors[field_0x604].z * manager->particles[manager->activeCount]->velocity.z;
            manager->particles[manager->activeCount]->flags = field_0x70;
            manager->particles[manager->activeCount]->growth = randomTable[field_0x480] + randomTable[field_0x480];
            manager->activeCount++;
            field_0x6c++;
            if (field_0x6c >= 0xd) field_0x6c = 0;
            field_0x604++;
            if (field_0x604 > 0x20) field_0x604 = 0;
            field_0x480++;
            if (field_0x480 > 0x100) field_0x480 = 0;
        }
        field_0x5c = field_0x3c;
    }
    return 1;
}
