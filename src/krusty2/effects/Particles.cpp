// Particles.cpp -- reconstruction of D:\aardvark\VC\krusty2\Particles.cpp (0x0056fa00).
//
// Ownership: the file's own __FILE__ references (0x004ba4f1, 0x004ba5c8) are the two debug
// `new`s in ParticleManager's slot 27; the other ParticleManager methods are contiguous with it.
#include "effects/ParticleManager.h"

extern "C" void qsort(void* base, unsigned int num, unsigned int width,
                      int (*compare)(const void*, const void*));   // 0x00534426

extern ParticleGameContext* g_gameContext;   // 0x0056e26c

extern "C" void* memset(void* dest, int c, unsigned int count);
#pragma intrinsic(memset)
// 0x00460c00 (math/FastMath.h).
float FastInvSqrt(float x);

static inline ParticleVec3 ParticleNormalize(const ParticleVec3& v)
{
    return v * FastInvSqrt(v.LengthSquared());
}

// 0x00689158: sprite-cell corner texture coordinates (61 cells; this file's .bss).
extern ParticleUVRect g_particleCellUVs[61];

ParticleManager::ParticleManager(int flags) : GameObject(flags)
{
    activeCount = 0;
    visibleCount = 0;
    gravity.x = 0.0f;
    gravity.y = -64.0f;
    gravity.z = 0.0f;
    texture = 0;
    for (int i = 0; i < 1000; i++)
        particles[i] = 0;
    vertexBuffer = 0;
    indexBufferObject = 0;
    indices = 0;
    field_0x1f90 = 1;
}

ParticleManager::~ParticleManager()
{
    if (texture)
        texture->BaseObjectVirtualSlot2();
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
            if (p->flags & Particle::Grow)
                p->size += dt * p->growth;
            p = *slot;
            if (p->flags & Particle::IntegratePosition) {
                p->position += p->velocity * dt;
            }
            p = *slot;
            if (p->flags & Particle::ApplyGravity) {
                p->velocity += gravity * dt;
            }
        }
    }
    return 1;
}

ParticleManager* ParticleManager::UnknownVirtualSlot27(int parentArg, TextureMapManager* textures,
                                                       const char* textureName, int field)
{
    GameObject::GameObjectVirtualSlot8(parentArg);
    for (int i = 0; i < 1000; i++)
        particles[i] = new(__FILE__, 0x48a) Particle;

    ParticleVertexBufferDesc desc;
    desc.size = sizeof(desc);
    desc.caps = 0x800;
    desc.fvf = 0x1c4;
    desc.vertexCount = 1500;
    ((ParticleRenderContext*)field_0x18)->device->direct3D->CreateVertexBuffer(&desc, &vertexBuffer, 0);
    desc.fvf = 0x1e2;
    ((ParticleRenderContext*)field_0x18)->device->direct3D->CreateVertexBuffer(&desc, &indexBufferObject, 0);
    desc.fvf = 0x1c4;
    desc.vertexCount = 4000;
    ((ParticleRenderContext*)field_0x18)->device->direct3D->CreateVertexBuffer(&desc, &field_0x1f8c, 0);

    void* data;
    field_0x1f8c->Lock(1, &data, 0);
    memset(data, 0, 4000 * 0x20);
    field_0x1f8c->Unlock();

    indices = new(__FILE__, 0x4a8) unsigned short[6000];
    if (!vertexBuffer || !indices || !indexBufferObject) {
        BaseObjectVirtualSlot2();
        return 0;
    }
    for (i = 0; i < 1000; i++) {
        indices[i * 6] = i * 4;
        indices[i * 6 + 1] = i * 4 + 1;
        indices[i * 6 + 2] = i * 4 + 2;
        indices[i * 6 + 3] = i * 4;
        indices[i * 6 + 4] = i * 4 + 2;
        indices[i * 6 + 5] = i * 4 + 3;
    }
    field_0x1f90 = 1;

    for (i = 0; i < 14; i++) {
        int row = i / 4;
        int col = i - row * 4;
        float v = row * 0.25f;
        float u = col * 0.25f;
        g_particleCellUVs[i].u0 = u;
        g_particleCellUVs[i].v0 = v;
        g_particleCellUVs[i].u1 = u + 0.25f;
        g_particleCellUVs[i].v1 = v;
        g_particleCellUVs[i].u2 = u + 0.25f;
        v += 0.25f;
        g_particleCellUVs[i].v2 = v;
        g_particleCellUVs[i].u3 = u;
        g_particleCellUVs[i].v3 = v;
    }
    for (i = 14; i < 18; i++) {
        int row = (i - 14) / 2;
        int col = i - (row * 2 + 14);
        float v = row * 0.125f + 0.75f;
        float u = col * 0.125f + 0.25f;
        g_particleCellUVs[i].u0 = u;
        g_particleCellUVs[i].v0 = v;
        g_particleCellUVs[i].u1 = u + 0.125f;
        g_particleCellUVs[i].v1 = v;
        g_particleCellUVs[i].u2 = u + 0.125f;
        v += 0.125f;
        g_particleCellUVs[i].v2 = v;
        g_particleCellUVs[i].u3 = u;
        g_particleCellUVs[i].v3 = v;
    }
    for (i = 29; i < 45; i++) {
        int row = (i - 29) / 4;
        int col = i - row * 4 - 29;
        float v = row * 0.0625f + 0.75f;
        float u = col * 0.0625f + 0.5f;
        g_particleCellUVs[i].u0 = u;
        g_particleCellUVs[i].v0 = v;
        g_particleCellUVs[i].u1 = u + 0.0625f;
        g_particleCellUVs[i].v1 = v;
        g_particleCellUVs[i].u2 = u + 0.0625f;
        v += 0.0625f;
        g_particleCellUVs[i].v2 = v;
        g_particleCellUVs[i].u3 = u;
        g_particleCellUVs[i].v3 = v;
    }
    for (i = 45; i < 61; i++) {
        int row = (i - 45) / 4;
        int col = i - row * 4 - 45;
        float v = row * 0.0625f + 0.75f;
        float u = col * 0.0625f + 0.75f;
        g_particleCellUVs[i].u0 = u;
        g_particleCellUVs[i].v0 = v;
        g_particleCellUVs[i].u1 = u + 0.0625f;
        g_particleCellUVs[i].v1 = v;
        g_particleCellUVs[i].u2 = u + 0.0625f;
        v += 0.0625f;
        g_particleCellUVs[i].v2 = v;
        g_particleCellUVs[i].u3 = u;
        g_particleCellUVs[i].v3 = v;
    }

    texture = UnknownFunction50a590(textures, textureName, 0x115c, 0, 0, 5, 6, 0, 0x80,
                                    0xff00ff, 1, 1);
    texture->UnknownVirtualSlot8(1, 0, 0);

    corners[0].x = 1.0f;
    corners[0].y = 1.0f;
    corners[0].z = 0.0f;
    corners[1].x = -1.0f;
    corners[1].y = 1.0f;
    corners[1].z = 0.0f;
    corners[2].x = 1.0f;
    corners[2].y = -1.0f;
    corners[2].z = 0.0f;
    corners[3].x = -1.0f;
    corners[3].y = -1.0f;
    corners[3].z = 0.0f;
    cornerNormals[0] = ParticleNormalize(ParticleVec3(1.0f, 1.0f, 0.25f));
    cornerNormals[1] = ParticleNormalize(ParticleVec3(-1.0f, 1.0f, 0.25f));
    cornerNormals[2] = ParticleNormalize(ParticleVec3(1.0f, -1.0f, 0.25f));
    cornerNormals[3] = ParticleNormalize(ParticleVec3(-1.0f, -1.0f, 0.25f));
    field_0x2054 = field;
    return this;
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
