// DustParticleEmitter.cpp -- ownership uncertain, see ParticleEmitters.h.
#include "ParticleEmitters.h"
#include "math/FastMath.h"

extern ParticleGameContext* g_gameContext;   // 0x0056e26c

DustParticleEmitter::DustParticleEmitter(int flags) : GameObject(flags)
{
    manager = 0;
    field_0x70 = 0;
    firstUpdate = 1;
    emitRate = 1000.0f;
    lifetimeScale = 6.0f;
    speed = 1.0f;
    maxAge = 10.0f;
    field_0x5c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    field_0x68 = 0;
    field_0x6c = 0;
    count = 10;
    intensity = 0;
    color = 0xffffff;
}

DustParticleEmitter::~DustParticleEmitter() {}

int DustParticleEmitter::UnknownVirtualSlot27(void* a, ParticleManager* m)
{
    GameObject::GameObjectVirtualSlot8((int)a);
    if (m) {
        manager = m;
        return (int)this;
    }
    BaseObjectVirtualSlot2();
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
// emission rate follow the speed (distance / dt, scaled by intensity when it is nonzero).
int DustParticleEmitter::GameObjectVirtualSlot10(float dt)
{
    if (g_gameContext->frozen == 0) {
        if (field_0x60 == 0) {
            field_0x68 = 0;
            field_0x6c = 0;
            previousPosition = position;
            field_0x64 = 0;
            return 1;
        }
        if (field_0x64 == 0) {
            field_0x64 = 1;
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
        emitRate = speed * 2000.0f;
        if (emitRate < 500.0f) emitRate = 500.0f;
        else if (emitRate > 1000.0f) emitRate = 1000.0f;
        float spacing = (speed + field_0x5c) * 0.3f;
        dt = len / spacing;  // dt is dead here; retail keeps the particle count in its stack slot
        field_0x6c = len - spacing * dt;
        float grow = (speed - field_0x5c) / dt;
        float inv = 1.0f / dt;
        float age = emitRate * 0.25f;
        ParticleVec3 step = d * inv;
        int i;
        for (i = 0; (float)i < dt; i++) {
            if (manager == 0) return 1;
            ParticleVec3 p = previousPosition + step * (float)i;
            float size = (float)i * grow + speed;
            if (!manager->AddParticle(age, &p, field_0x70, size, emitRate, size + size, count, color)) return 1;
            field_0x70++;
            if (field_0x70 >= 13) field_0x70 = 0;
        }
        field_0x5c = speed;
    }
    return 1;
}
