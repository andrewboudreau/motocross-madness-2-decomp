// DustParticleEmitter.cpp -- ownership uncertain, see ParticleEmitters.h.
#include "TrackGameGlobal.h"
#include "ParticleEmitters.h"
#include "math/FastMath.h"


DustParticleEmitter::DustParticleEmitter(int flags) : GameObject(flags)
{
    firstUpdate = 1;
    particleLifetime = 4000.0f;
    speed = 1.0f;
    lifetimeScale = 6.0f;
    maxAge = 10.0f;
    position.x = 0;
    position.y = 0;
    position.z = 0;
    previousPosition = position;
    field_0x68 = 0;
    spacingRemainder = 0;
    previousSpeed = 0;
    activeThisFrame = 0;
    primed = 0;
    nextFrame = 0;
    manager = 0;
    particleFlags = 10;
    intensity = 0;
    color = 0xffffff;
}

DustParticleEmitter::~DustParticleEmitter() {}

int DustParticleEmitter::UnknownVirtualSlot27(void* context, ParticleManager* m)
{
    GameObject::SetRenderContext((void*)context);
    if (m) {
        manager = m;
        return (int)this;
    }
    Release();
    return 0;
}

void DustParticleEmitter::SetPosition(ParticleVec3 p, float i)
{
    previousPosition = position;
    position = p;
    intensity = i;
}

void DustParticleEmitter::SetColor(unsigned char r, unsigned char g, int b)
{
    color = (r << 16) | (g << 8) | (b & 0xff);
}

// 0x004b8ab0.  Spawns dust along the segment the emitter moved this tick; the spacing and the
// particles' lifetime follow the speed (distance / dt, scaled by intensity when it is nonzero).
int DustParticleEmitter::Tick(float dt)
{
    if (reinterpret_cast<ParticleGameContext*>(g_TrackGame)->frozen == 0) {
        if (activeThisFrame == 0) {
            field_0x68 = 0;
            spacingRemainder = 0;
            previousPosition = position;
            primed = 0;
            return 1;
        }
        if (primed == 0) {
            primed = 1;
            return 1;
        }
        if (firstUpdate) {
            firstUpdate = 0;
            previousPosition = position;
            return 1;
        }
        ParticleVec3 d(position.x - previousPosition.x, position.y - previousPosition.y, position.z - previousPosition.z);
        float lenSq = d.LengthSquared();
        float len = (lenSq == 1.0f) ? 1.0f : FastSqrt(lenSq);
        float rate = len / dt;
        if (intensity == 0.0f)
            speed = rate * 0.013333f;
        else
            speed = rate * (intensity * 0.013333f);
        if (speed < 0.1f) speed = 0.1f;
        if (speed > 2.0f) speed = 2.0f;
        particleLifetime = speed * 2000.0f;
        if (particleLifetime < 500.0f) particleLifetime = 500.0f;
        else if (particleLifetime > 1000.0f) particleLifetime = 1000.0f;
        float spacing = (speed + previousSpeed) * 0.3f;
        dt = len / spacing;  // dt is dead here; retail keeps the particle count in its stack slot
        spacingRemainder = len - spacing * dt;
        float grow = (speed - previousSpeed) / dt;
        float inv = 1.0f / dt;
        float age = particleLifetime * 0.25f;
        ParticleVec3 step = d * inv;
        int i;
        for (i = 0; (float)i < dt; i++) {
            if (manager == 0) return 1;
            ParticleVec3 p = previousPosition + step * (float)i;
            float size = (float)i * grow + speed;
            if (!manager->AddParticle(age, &p, nextFrame, size, particleLifetime, size + size, particleFlags, color)) return 1;
            nextFrame++;
            if (nextFrame >= 13) nextFrame = 0;
        }
        previousSpeed = speed;
    }
    return 1;
}
