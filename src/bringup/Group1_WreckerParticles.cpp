// Group1_WreckerParticles.cpp -- bring-up body for Wrecker.h's UnknownWreckerParticles
// view of ParticleManager.
//
// Retail 0x004baa50 (thiscall, ret 0x20) is ParticleManager::AddParticle
// (src/krusty2/effects/Particles.cpp); this is the same body against Wrecker.h's view
// (identical offsets: count +0x2c, 1000 particle pointers at +0x40).

#include "Wrecker.h"

// 0x004baa50: fills the next free particle; 0 when all 1000 are live.
int UnknownWreckerParticles::AddParticle(float age, const Vector3* position, int frame,
                                         float size, float lifetime, float growth,
                                         unsigned int flags, int color)
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
