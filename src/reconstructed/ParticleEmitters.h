// ParticleEmitters.h -- the five particle emitter classes that sit in the Particles.cpp bracket.
//
// OWNERSHIP IS UNCERTAIN (see README of src/krusty2/effects): none of these functions references a
// __FILE__ string.  They are contiguous with Particles.cpp's own ParticleManager code, and four
// $E static initialisers at 0x004b9e00..0x004b9f3f mark a translation unit start inside the run, so the
// split between Particles.cpp and a neighbour is unproven.  Every class has a GameObject base chain
// (RTTI complete object locators 0x0055df20 Dust, and one per class) with object_offset 0.
// Slot 27 is ICF-folded: one body (0x004ba040) serves all five vtables, so it proves nothing about a
// shared base class; each class declares it separately.
#ifndef EFFECTS_PARTICLE_EMITTERS_H
#define EFFECTS_PARTICLE_EMITTERS_H

#include "../krusty2/effects/ParticleManager.h"

// position and previousPosition are plain ParticleVec3: every emitter ctor zeroes position and then
// struct-copies it into previousPosition, which is retail's shared-register `lea ecx, [previousPosition]; mov edx, eax`.

// Dust: vtable 0x00555ab8.  Ctor 0x004b8a00, deleting dtor 0x004b8a80, dtor 0x004b8aa0.
class DustParticleEmitter : public GameObject {
public:
    explicit DustParticleEmitter(int flags);
    virtual ~DustParticleEmitter();
    virtual int Tick(float dt);
    virtual int UnknownVirtualSlot27(void* context, ParticleManager* manager);
    void SetPosition(ParticleVec3 p, float intensity);   // 0x004b8d90
    void SetColor(unsigned char r, unsigned char g, int b);          // 0x004b8dd0

    int firstUpdate;          // +0x2c ctor 1; slot 10 clears it after copying position to previousPosition
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor); target of AddParticle
    float particleLifetime;   // +0x34 ctor 4000.0 (retail imm 0x457a0000); slot 10 sets it to speed*2000 clamped to 500..1000 and passes it as each particle's lifetime (a quarter of it as the starting age)
    float lifetimeScale;      // +0x38 ctor 6.0
    float speed;              // +0x3c ctor 1.0; slot 10 clamps to 0.1..2.0 from distance moved / dt
    float maxAge;             // +0x40 ctor 10.0
    ParticleVec3 position;    // +0x44 SetPosition's xyz
    ParticleVec3 previousPosition; // +0x50 SetPosition (and slot 10) copy position here first
    float previousSpeed;      // +0x5c ctor 0; slot 10 stores speed here after emitting and uses it for the spacing and size ramp
    int activeThisFrame;      // +0x60 ctor 0; slot 10 emits only while it is set (SoultreeAttachedObject view: SoulTree slot 18 sets it, slot 21 clears it)
    int primed;               // +0x64 ctor 0; slot 10 sets it and skips the first active tick; cleared while inactive
    float field_0x68;         // +0x68 ctor 0; slot 10 clears it while inactive (no reader decoded)
    float spacingRemainder;   // +0x6c ctor 0; slot 10 stores the distance left after the last particle (no reader decoded)
    int nextFrame;            // +0x70 ctor 0; each new particle's frame, cycling 0..12
    int particleFlags;        // +0x74 ctor 10; each new particle's flags (SoulTree slot 21 sets/clears 0x800 through the SoultreeAttachedObject view)
    float intensity;          // +0x78 SetPosition's fourth argument; scales speed in slot 10
    int color;                // +0x7c ctor 0xffffff; SetColor packs r<<16|g<<8|b
};


// DirtChunkParticleEmitter: vtable 0x00555b34.  Ctor 0x004b8df0, deleting dtor 0x004b8f10, dtor 0x004b8f30,
// slot 10 0x004b8f40 (970 bytes).  Size 0x608 (the ctor stores +0x604).
class DirtChunkParticleEmitter : public GameObject {
public:
    explicit DirtChunkParticleEmitter(int flags);
    virtual ~DirtChunkParticleEmitter();
    virtual int Tick(float dt);
    virtual int UnknownVirtualSlot27(void* context, ParticleManager* manager);

    int firstUpdate;          // +0x2c ctor 1
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor)
    float particleLifetime;   // +0x34 ctor 375.0; slot 10 stores it as each new particle's lifetime
    float field_0x38;         // +0x38 ctor 6.0
    float field_0x3c;         // +0x3c ctor 1.0
    float emitRate;           // +0x40 ctor 200.0; slot 10 emits dt * it particles per tick
    ParticleVec3 position;    // +0x44 ctor 0
    ParticleVec3 previousPosition; // +0x50 ctor 0
    float field_0x5c;         // +0x5c ctor 0
    int activeThisFrame;      // +0x60 ctor 0; slot 10 emits only while it is set (Bike attaches it as SoulTree's slide emitter, whose slot 21 clears +0x60 every frame)
    float emitRemainder;      // +0x64 ctor 0; slot 10 carries the fractional particle count between ticks
    int field_0x68;           // +0x68 ctor 0
    int nextFrame;            // +0x6c ctor 0x1d; each new particle's frame, cycling 0x1d..0x2c
    int particleFlags;        // +0x70 ctor 0x14; stored in each new particle's flags
    ParticleVec3 launchVelocity; // +0x74 ctor (0, 2, 0); slot 10 adds it to each new particle velocity
    float randomTable[256];   // +0x80 ctor fills rand()/32768*0.25
    int randomTableIndex;     // +0x480 ctor 0; index into randomTable for the next particle's size; reset only once it exceeds 0x100, so it reaches 0x100, one past the table
    ParticleVec3 randomVectors[32]; // +0x484 ctor fills rand()/32768+0.5 per component
    int randomVectorIndex;    // +0x604 ctor 0; index into randomVectors for the next particle's velocity; reset only once it exceeds 0x20, so it reaches 0x20, one past the array
};

// DirtSprayParticleEmitter: vtable 0x00555ba8.  Ctor 0x004b9310, deleting dtor 0x004b9430, dtor 0x004b9450,
// slot 10 0x004b9460 (970 bytes).  Size 0x608 (the ctor stores +0x604).
class DirtSprayParticleEmitter : public GameObject {
public:
    explicit DirtSprayParticleEmitter(int flags);
    virtual ~DirtSprayParticleEmitter();
    virtual int Tick(float dt);
    virtual int UnknownVirtualSlot27(void* context, ParticleManager* manager);

    int firstUpdate;          // +0x2c ctor 1
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor)
    float particleLifetime;   // +0x34 ctor 750.0; slot 10 stores it as each new particle's lifetime
    float field_0x38;         // +0x38 ctor 6.0
    float field_0x3c;         // +0x3c ctor 1.0
    float emitRate;           // +0x40 ctor 200.0; slot 10 emits dt * it particles per tick
    ParticleVec3 position;    // +0x44 ctor 0
    ParticleVec3 previousPosition; // +0x50 ctor 0
    float field_0x5c;         // +0x5c ctor 0
    int active;               // +0x60 ctor 0; slot 10 emits only while it is set
    float emitRemainder;      // +0x64 ctor 0; slot 10 carries the fractional particle count between ticks
    int field_0x68;           // +0x68 ctor 0
    int nextFrame;            // +0x6c ctor 0; each new particle's frame, cycling 0..12
    int particleFlags;        // +0x70 ctor 0x1e; stored in each new particle's flags
    ParticleVec3 launchVelocity; // +0x74 ctor (0, 2, 0); slot 10 adds it to each new particle velocity
    float randomTable[256];   // +0x80 ctor fills (rand()/32768+1)*0.25
    int randomTableIndex;     // +0x480 ctor 0; index into randomTable for the next particle's size; reset only once it exceeds 0x100, so it reaches 0x100, one past the table
    ParticleVec3 randomVectors[32]; // +0x484 ctor fills x=y=z=rand()/32768*0.5+0.75
    int randomVectorIndex;    // +0x604 ctor 0; index into randomVectors for the next particle's velocity; reset only once it exceeds 0x20, so it reaches 0x20, one past the array
};

// SparkParticleEmitter: vtable 0x00555c1c.  Ctor 0x004b9830, deleting dtor 0x004b9970, dtor 0x004b9990,
// slot 10 0x004b99a0 (1055 bytes, reconstructed), position setter 0x004b9dc0.
class SparkParticleEmitter : public GameObject {
public:
    explicit SparkParticleEmitter(int flags);
    virtual ~SparkParticleEmitter();
    virtual int Tick(float dt);
    virtual int UnknownVirtualSlot27(void* context, ParticleManager* manager);
    void SetPosition(ParticleVec3 p);   // 0x004b9dc0

    int firstUpdate;          // +0x2c ctor 1
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor)
    float particleLifetime;   // +0x34  ctor 500.0; slot 10 stores it as each new particle's lifetime
    float field_0x38;         // +0x38 ctor 0
    float field_0x3c;         // +0x3c ctor 1.0
    float emitRate;           // +0x40  ctor 2400.0; slot 10 emits dt * it particles per tick
    ParticleVec3 position;    // +0x44 ctor 0; SetPosition's arguments
    ParticleVec3 previousPosition; // +0x50 SetPosition copies position here first
    float field_0x5c;         // +0x5c ctor 0
    int burstPending;  // +0x60  ctor 0; slot 10 idles while it is clear and clears it after emitting one burst; Vehicle/Bike impact handlers set it after SetPosition
    float idleTime;  // +0x64  ctor 0; slot 10 adds dt while no burst is pending and clears it after a burst; above 0.25 the burst restarts from the current position
    float emitRemainder;      // +0x68  ctor 0; fractional particle remainder carried between ticks
    int field_0x6c;           // +0x6c ctor 0
    int nextFrame;            // +0x70  ctor 0x2d; each new particle's frame, cycling 0x2d..0x3c
    int particleFlags;        // +0x74  ctor 0x14; stored in each new particle's flags
    ParticleVec3 launchVelocity; // +0x78 ctor (0, 2, 0); slot 10 adds it to each new particle velocity
    float randomTable[256];   // +0x84 ctor fills rand()/32768*0.075
    int randomTableIndex;     // +0x484  ctor 0; index into randomTable for the next particle's size and growth
    ParticleVec3 randomVectors[8]; // +0x488 ctor fills each component rand()/32768*10-5
    char field_0x4e8[0x608 - 0x4e8]; // +0x4e8 not touched by the ctor
    int randomVectorIndex;    // +0x608  ctor 0; index into randomVectors for the next particle's velocity
};

// SteamParticleEmitter: vtable 0x00555c9c.  Ctor 0x004b9f40, deleting dtor 0x004ba010, dtor 0x004ba030,
// slot 10 0x004ba070 (590 bytes, reconstructed), setters 0x004ba2c0..0x004ba360.
class SteamParticleEmitter : public GameObject {
public:
    explicit SteamParticleEmitter(int flags);
    virtual ~SteamParticleEmitter();
    virtual int Tick(float dt);
    virtual int UnknownVirtualSlot27(void* context, ParticleManager* manager);
    void SetPosition(ParticleVec3 p);   // 0x004ba2c0
    void Restart();                                   // 0x004ba300
    void Burst();                                     // 0x004ba320
    void StopEmitting();                              // 0x004ba340
    void SetColor(unsigned char r, unsigned char g, int b);   // 0x004ba360

    int firstUpdate;          // +0x2c ctor 1
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor)
    float particleLifetime;   // +0x34 ctor 500.0; slot 10 passes it as each puff's lifetime
    float field_0x38;         // +0x38 ctor 3.0
    float startSize;          // +0x3c ctor 0.1; the newest puff's size and the spacing unit (count = distance / (0.3 * it))
    float pulseDuration;      // +0x40 ctor 0.25; Restart copies it to pulseTimeLeft
    float pulseTimeLeft;      // +0x44 ctor 0; Restart copies pulseDuration here, Burst stores 0.0001; slot 10 counts it down by dt and ends the pulse at 0
    ParticleVec3 position;    // +0x48 ctor 0; SetPosition's arguments
    ParticleVec3 previousPosition; // +0x54 SetPosition copies position here first
    int emitting;  // +0x60  ctor 0; Restart (0x004ba300) sets 1, StopEmitting (0x004ba340) clears it
    int nextFrame;            // +0x64 ctor 14; each new puff's frame, cycling 14..17
    int particleFlags;        // +0x68 ctor 0x80a; each new puff's flags
    int pulseActive;          // +0x6c ctor 0; Restart and Burst set 1; slot 10 emits only while it and emitting are set, and clears it when pulseTimeLeft runs out
    int bursting;             // +0x70 ctor 0; Burst sets 1; slot 10 then emits five puffs per tick until the pulse ends
    float randomTable[256];   // +0x74 ctor fills each group of four with one value 2*rand()/32768+1; slot 10 reads randomTable[randomTableIndex]
    int randomTableIndex;     // +0x474 ctor 0; index into randomTable for the next puff's growth
    int color;                // +0x478 ctor 0xffffff; SetColor packs r<<16|g<<8|b
};

#endif
