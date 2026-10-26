// SteamParticleEmitter.cpp -- ownership uncertain, see ParticleEmitters.h.
#include "TrackGameGlobal.h"
#include "ParticleEmitters.h"
#include <stdlib.h>
#include "math/FastMath.h"


SteamParticleEmitter::SteamParticleEmitter(int flags) : GameObject(flags)
{
    int i;
    firstUpdate = 1;
    particleLifetime = 500.0f;
    startSize = 0.1f;
    field_0x38 = 3.0f;
    position.x = 0;
    position.y = 0;
    position.z = 0;
    previousPosition = position;
    emitting = 0;
    nextFrame = 14;
    manager = 0;
    pulseActive = 0;
    pulseDuration = 0.25f;
    pulseTimeLeft = 0;
    particleFlags = 0x80a;
    float* t = randomTable;
    for (i = 0; i < 64; i++, t += 4) {
        float r = (float)rand() * (1.0f / 32768.0f);
        float v = (r + r) + 1.0f;
        int k;
        for (k = 0; k < 4; k++)
            t[k] = v;
    }
    randomTableIndex = 0;
    bursting = 0;
    color = 0xffffff;
}

SteamParticleEmitter::~SteamParticleEmitter() {}

int SteamParticleEmitter::UnknownVirtualSlot27(void* a, ParticleManager* m)
{
    GameObject::SetRenderContext((void*)a);
    if (m) {
        manager = m;
        return (int)this;
    }
    Release();
    return 0;
}

void SteamParticleEmitter::SetPosition(ParticleVec3 p)
{
    previousPosition = position;
    position = p;
}

void SteamParticleEmitter::Restart()
{
    pulseActive = 1;
    pulseTimeLeft = pulseDuration;
    emitting = 1;
}

void SteamParticleEmitter::Burst()
{
    pulseTimeLeft = 0.0001f;
    pulseActive = 1;
    bursting = 1;
}

void SteamParticleEmitter::StopEmitting()
{
    emitting = 0;
    previousPosition = position;
}

void SteamParticleEmitter::SetColor(unsigned char r, unsigned char g, int b)
{
    color = (r << 16) | (g << 8) | (b & 0xff);
}

// 0x004ba070.  Emits a trail of puffs between the previous and the current position each tick.
int SteamParticleEmitter::Tick(float dt)
{
    if (reinterpret_cast<ParticleGameContext*>(g_TrackGame)->frozen == 0 && emitting != 0 && pulseActive != 0) {
        if (pulseTimeLeft <= 0.0f) {
            pulseTimeLeft = 0;
            pulseActive = 0;
            bursting = 0;
            return 1;
        }
        if (firstUpdate) {
            firstUpdate = 0;
            previousPosition = position;
            return 1;
        }
        pulseTimeLeft -= dt;
        ParticleVec3 d(position.x - previousPosition.x, position.y - previousPosition.y, position.z - previousPosition.z);
        float lenSq = d.LengthSquared();
        float len = (lenSq == 1.0f) ? 1.0f : FastSqrt(lenSq);
        float count = len / (startSize * 0.3f);
        ParticleVec3 step = (1.0f / count) * d;
        if (bursting)
            count = 5.0f;
        int i;
        float grow = (dt * 3.0f - startSize) / count;
        for (i = 0; (float)i < count; i++) {
            if (manager == 0)
                break;
            ParticleVec3 p = position - step * (float)i;
            if (!manager->AddParticle(0.0f, &p, nextFrame, (float)i * grow + startSize, particleLifetime,
                                      randomTable[randomTableIndex], particleFlags, color))
                break;
            randomTableIndex++;
            if (randomTableIndex >= 256)
                randomTableIndex = 0;
            nextFrame++;
            if (nextFrame >= 0x12)
                nextFrame = 0xe;
        }
    }
    return 1;
}
