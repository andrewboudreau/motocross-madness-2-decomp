// Particles.cpp -- reconstruction of D:\aardvark\VC\krusty2\Particles.cpp (0x0056fa00).
//
// Ownership: the file's own __FILE__ references (0x004ba4f1, 0x004ba5c8) are the two debug
// `new`s in ParticleManager's slot 27; the other ParticleManager methods are contiguous with it.
#include "effects/ParticleManager.h"

extern "C" void qsort(void* base, unsigned int num, unsigned int width,
                      int (*compare)(const void*, const void*));   // 0x00534426

extern ParticleGameContext* g_gameContext;   // 0x0056e26c

ParticleManager::ParticleManager(int flags) : GameObject(flags)
{
    activeCount = 0;
    visibleCount = 0;
    gravity.x = 0.0f;
    gravity.y = -64.0f;
    gravity.z = 0.0f;
    field_0x1f80 = 0;
    for (int i = 0; i < 1000; i++)
        particles[i] = 0;
    vertexBuffer = 0;
    indexBufferObject = 0;
    indices = 0;
    field_0x1f90 = 1;
}

ParticleManager::~ParticleManager()
{
    if (field_0x1f80)
        field_0x1f80->BaseObjectVirtualSlot2();
    for (int i = 0; i < 1000; i++) {
        if (particles[i])
            delete particles[i];
    }
    if (indices)
        delete indices;
    if (vertexBuffer)
        vertexBuffer->Release();
    if (indexBufferObject)
        indexBufferObject->Release();
}

int ParticleManager::AddParticle(float age, const ParticleVec3* position, int frame, float size,
                                 float lifetime, float growth, unsigned int flags, int color)
{
    if (activeCount != 1000) {
        particles[activeCount]->age = age;
        particles[activeCount]->position = *position;
        particles[activeCount]->frame = frame;
        particles[activeCount]->size = size;
        particles[activeCount]->lifetime = lifetime;
        particles[activeCount]->growth = growth;
        particles[activeCount]->flags = flags;
        particles[activeCount]->color = color;
        activeCount++;
        return 1;
    }
    return 0;
}

int ParticleManager::GameObjectVirtualSlot10(float dt)
{
    if (g_gameContext->frozen)
        return 1;
    float ms = dt * 1000.0f;
    for (int i = 0; i < activeCount; i++) {
        Particle** slot = &particles[i];
        (*slot)->age += ms;
        Particle* p = *slot;
        if (p->age > p->lifetime) {
            *slot = particles[activeCount - 1];
            particles[activeCount - 1] = p;
            activeCount--;
        } else {
            if (p->flags & 8)
                p->size += dt * p->growth;
            p = *slot;
            if (p->flags & 4) {
                p->position += p->velocity * dt;

            }
            p = *slot;
            if (p->flags & 0x10) {
                p->velocity += gravity * dt;

            }
        }
    }
    return 1;
}



int CompareParticleDepth(const void* a, const void* b)
{
    ParticleVertex* va = (*(Particle**)a)->vertex;
    ParticleVertex* vb = (*(Particle**)b)->vertex;
    if (va->z > vb->z)
        return -1;
    if (va->z < vb->z)
        return 1;
    return 0;
}
