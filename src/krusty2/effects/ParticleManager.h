// ParticleManager.h -- ParticleManager and its Particle record (D:\aardvark\VC\krusty2\Particles.cpp).
//
// Evidence (tier 1 unless noted):
//  * RTTI .?AVParticleManager@@ COL 0x0055e0b0, vtable 0x00555d10 (28 slots, object_offset 0),
//    single base GameObject (mdisp 0).  Overridden: slot 0 (deleting dtor 0x004ba400), slot 10
//    0x004baaf0, slot 14 0x004bac60, and the new slot 27 0x004ba4d0.
//  * Ctor 0x004ba390 (ret 4, argument forwarded to the GameObject ctor 0x00468ca0).
//  * Particles are allocated with new(0x48, __FILE__, 0x48a) (1000 times, slot 27), so a Particle
//    is 0x48 bytes.
#ifndef EFFECTS_PARTICLE_MANAGER_H
#define EFFECTS_PARTICLE_MANAGER_H

#include "core/GameObject.h"
#include "core/DebugAlloc.h"

// PROVISIONAL: DirectX-style interface; the dtor calls slot 2 with `this` pushed on the stack
// (`mov ecx,[eax]; push eax; call [ecx+8]`), i.e. an IUnknown::Release.
class ParticleComObject {
public:
    virtual long __stdcall QueryInterface(void* iid, void** out);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
};

// The global at 0x0056e26c (tier 3 name).  Only the two words the particle code tests are known.
struct ParticleGameContext {
    char field_0x00[0x1c4];
    int frozen;                    // +0x1c4 nonzero: every emitter and the manager skip their update (slot 10)
    char field_0x1c8[0x2d0 - 0x1c8];
    int hidden;                    // +0x2d0 nonzero: the manager skips drawing (slot 14)
};

struct ParticleVec3 {
    float x, y, z;
    ParticleVec3() {}
    ParticleVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    ParticleVec3 operator*(float s) const { return ParticleVec3(x * s, y * s, z * s); }
    float LengthSquared() const { return z * z + (y * y + x * x); }
    ParticleVec3 operator+(const ParticleVec3& o) const { return ParticleVec3(x + o.x, y + o.y, z + o.z); }
    ParticleVec3 operator-(const ParticleVec3& o) const { return ParticleVec3(x - o.x, y - o.y, z - o.z); }
    ParticleVec3& operator+=(const ParticleVec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
};

inline ParticleVec3 operator*(float s, const ParticleVec3& v) { return ParticleVec3(s * v.x, s * v.y, s * v.z); }

// First 12 bytes of a 0x20-byte vertex in the locked buffer (slot 14 stores x, y+0.5, z there).
struct ParticleVertex {
    float x, y, z;
};

// One live sprite.  Offsets come from AddParticle (0x004baa50, which stores the arguments),
// Update (slot 10, which ages and moves it) and Render (slot 14).  Names are tier 3.
struct Particle {
    // Provisional flag names from the three independent tests in slot 10.
    enum Flags {
        IntegratePosition = 4,
        Grow = 8,
        ApplyGravity = 0x10
    };

    ParticleVec3 position;   // +0x00 copied from AddParticle's vector argument; integrated by velocity (flag 4)
    int color;               // +0x0c AddParticle's last argument (emitters pass 0xffffff or a packed rgb)
    int frame;               // +0x10 AddParticle's third argument (an emitter's cycling sprite index)
    float age;               // +0x14 slot 10 adds dt*1000 each tick
    float lifetime;          // +0x18 slot 10 recycles the particle once age > lifetime
    float size;              // +0x1c grows by dt*growth every tick while flag 8 is set
    float growth;            // +0x20 see size
    char field_0x24[0x34 - 0x24];  // +0x24..+0x30 written by Render (projected corner values)
    unsigned int flags;      // +0x34 bit 8: size grows, bit 4: velocity moves it, bit 0x10: gravity applies
    ParticleVec3 velocity;   // +0x38 added to position * dt (flag 4); gravity is added to it (flag 0x10)
    ParticleVertex* vertex;  // +0x44 Render points it into the locked vertex buffer (0x20 bytes per particle); the depth sort reads vertex->z
};

// qsort comparator used by slot 14 (0x004bac20, cdecl): nearer vertices (larger z) first.
int CompareParticleDepth(const void* a, const void* b);

class ParticleManager : public GameObject {
public:
    explicit ParticleManager(int flags);
    virtual ~ParticleManager();
    virtual int GameObjectVirtualSlot10(float dt);
    virtual int GameObjectVirtualSlot14();
    virtual int UnknownVirtualSlot27(void* a, void* b, void* c, void* d);

    // 0x004baa50 (thiscall, ret 0x20).  Takes the next particle from the free tail of the table
    // and fills it in; returns 0 when all 1000 are in use.
    int AddParticle(float age, const ParticleVec3* position, int frame, float size,
                    float lifetime, float growth, unsigned int flags, int color);

    int activeCount;                    // +0x2c number of live particles at the head of particles[]; slot 10 swaps dead ones past it
    int visibleCount;                   // +0x30 slot 14 counts the particles it will draw this frame
    char field_0x34[0x40 - 0x34];
    Particle* particles[1000];          // +0x40 0x3e8 pointers, zeroed by the ctor, filled in slot 27, freed by the dtor
    Particle* visible[1000];            // +0xfe0 slot 14 collects the drawable particles here and qsorts them by depth
    BaseObject* field_0x1f80;           // +0x1f80 released through BaseObject slot 2 (`call [eax+8]` with this in ecx) by the dtor
    ParticleComObject* vertexBuffer;                 // +0x1f84 COM object released by the dtor (slot 27 asks the device for 0x1e2.. bytes)
    ParticleComObject* indexBufferObject;            // +0x1f88 COM object released by the dtor
    ParticleComObject* field_0x1f8c;                 // +0x1f8c COM object locked in slot 27 (0x7d00 dwords zeroed)
    int field_0x1f90;                   // +0x1f90 set to 1 by the ctor and again by slot 27
    unsigned short* indices;            // +0x1f94 delete'd by the dtor; slot 27 fills 0x2ee0 bytes of quad indices
    ParticleVec3 gravity;               // +0x1f98 ctor stores (0, -64, 0); slot 10 adds dt*gravity to flagged particle velocity
    char field_0x1fa4[0x2058 - 0x1fa4]; // +0x1fa4.. slot 27 fills texture/corner tables; not decoded yet
};

#endif
