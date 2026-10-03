// SteamParticleEmitter.cpp -- ownership uncertain, see ParticleEmitters.h.
#include "ParticleEmitters.h"
#include <stdlib.h>
#include "math/FastMath.h"

extern ParticleGameContext* g_gameContext;   // 0x0056e26c

SteamParticleEmitter::SteamParticleEmitter(int flags) : GameObject(flags)
{
    int i;
    firstUpdate = 1;
    field_0x34 = 500.0f;
    field_0x3c = 0.1f;
    field_0x38 = 3.0f;
    field_0x60 = 0;
    field_0x64 = 14;
    manager = 0;
    field_0x6c = 0;
    field_0x40 = 0.25f;
    field_0x44 = 0;
    field_0x68 = 0x80a;
    float* t = randomTable;
    for (i = 0; i < 64; i++, t += 4) {
        float r = (float)rand() * (1.0f / 32768.0f);
        float v = (r + r) + 1.0f;
        t[0] = v;
        t[1] = v;
        t[2] = v;
        t[3] = v;
    }
    field_0x474 = 0;
    field_0x70 = 0;
    color = 0xffffff;
}

SteamParticleEmitter::~SteamParticleEmitter() {}

int SteamParticleEmitter::UnknownVirtualSlot27(void* a, ParticleManager* m)
{
    GameObject::GameObjectVirtualSlot8((int)a);
    if (m) {
        manager = m;
        return (int)this;
    }
    BaseObjectVirtualSlot2();
    return 0;
}

void SteamParticleEmitter::SetPosition(ParticleVec3 p)
{
    previousPosition = position;
    position = p;
}

void SteamParticleEmitter::Restart()
{
    field_0x6c = 1;
    field_0x44 = field_0x40;
    field_0x60 = 1;
}

void SteamParticleEmitter::Burst()
{
    field_0x44 = 0.0001f;
    field_0x6c = 1;
    field_0x70 = 1;
}

void SteamParticleEmitter::StopEmitting()
{
    field_0x60 = 0;
    previousPosition = position;
}

void SteamParticleEmitter::SetColor(unsigned char r, unsigned char g, int b)
{
    color = (r << 16) | (g << 8) | (b & 0xff);
}

// 0x004ba070.  Emits a trail of puffs between the previous and the current position each tick.
int SteamParticleEmitter::GameObjectVirtualSlot10(float dt)
{
    if (g_gameContext->frozen == 0 && field_0x60 != 0 && field_0x6c != 0) {
        if (field_0x44 <= 0.0f) {
            field_0x44 = 0;
            field_0x6c = 0;
            field_0x70 = 0;
            return 1;
        }
        if (firstUpdate) {
            firstUpdate = 0;
            previousPosition = position;
            return 1;
        }
        field_0x44 -= dt;
        ParticleVec3 d(position.x - previousPosition.x, position.y - previousPosition.y, position.z - previousPosition.z);
        float lenSq = d.LengthSquared();
        float len = (lenSq == 1.0f) ? 1.0f : FastSqrt(lenSq);
        float count = len / (field_0x3c * 0.3f);
        ParticleVec3 step = (1.0f / count) * d;
        if (field_0x70)
            count = 5.0f;
        int i;
        float grow = (dt * 3.0f - field_0x3c) / count;
        for (i = 0; (float)i < count; i++) {
            if (manager == 0)
                break;
            ParticleVec3 p = position - step * (float)i;
            if (!manager->AddParticle(0.0f, &p, field_0x64, (float)i * grow + field_0x3c, field_0x34,
                                      randomTable[field_0x474], field_0x68, color))
                break;
            field_0x474++;
            if (field_0x474 >= 256)
                field_0x474 = 0;
            field_0x64++;
            if (field_0x64 >= 0x12)
                field_0x64 = 0xe;
        }
    }
    return 1;
}
