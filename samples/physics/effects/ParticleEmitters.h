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

#include "effects/ParticleManager.h"

// Vec3 whose default constructor zeroes it; the emitters' ctors store the zeros for position and
// previousPosition through a shared register, which a member with this constructor explains.
struct EmitterVec3 : public ParticleVec3 {
    EmitterVec3() { x = 0.0f; y = 0.0f; z = 0.0f; }
    EmitterVec3& operator=(const ParticleVec3& o) { x = o.x; y = o.y; z = o.z; return *this; }
};

// Dust: vtable 0x00555ab8.  Ctor 0x004b8a00, deleting dtor 0x004b8a80, dtor 0x004b8aa0.
class DustParticleEmitter : public GameObject {
public:
    explicit DustParticleEmitter(int flags);
    virtual ~DustParticleEmitter();
    virtual int GameObjectVirtualSlot10(float dt);
    virtual int UnknownVirtualSlot27(void* a, ParticleManager* manager);
    void SetPosition(ParticleVec3 p, float intensity);   // 0x004b8d90
    void SetColor(unsigned char r, unsigned char g, int b);          // 0x004b8dd0

    int firstUpdate;          // +0x2c ctor 1; slot 10 clears it after copying position to previousPosition
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor); target of AddParticle
    float emitRate;           // +0x34 ctor 1000.0; slot 10 clamps it to 500..1000 (particles per second)
    float lifetimeScale;      // +0x38 ctor 6.0
    float speed;              // +0x3c ctor 1.0; slot 10 clamps to 0.1..2.0 from distance moved / dt
    float maxAge;             // +0x40 ctor 10.0
    EmitterVec3 position;    // +0x44 SetPosition's xyz
    EmitterVec3 previousPosition; // +0x50 SetPosition (and slot 10) copy position here first
    float field_0x5c;         // +0x5c
    int field_0x60;           // +0x60
    int field_0x64;           // +0x64
    float field_0x68;         // +0x68
    float field_0x6c;         // +0x6c
    int field_0x70;           // +0x70
    int count;                // +0x74 ctor 10
    float intensity;          // +0x78 SetPosition's fourth argument; scales speed in slot 10
    int color;                // +0x7c ctor 0xffffff; SetColor packs r<<16|g<<8|b
};


// DirtChunkParticleEmitter: vtable 0x00555b34.  Ctor 0x004b8df0, deleting dtor 0x004b8f10, dtor 0x004b8f30,
// slot 10 0x004b8f40 (970 bytes) (not reconstructed).  Size 0x608 (the ctor stores +0x604).
class DirtChunkParticleEmitter : public GameObject {
public:
    explicit DirtChunkParticleEmitter(int flags);
    virtual ~DirtChunkParticleEmitter();
    virtual int GameObjectVirtualSlot10(float dt);
    virtual int UnknownVirtualSlot27(void* a, ParticleManager* manager);

    int firstUpdate;          // +0x2c ctor 1
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor)
    float field_0x34;         // +0x34 ctor 375.0
    float field_0x38;         // +0x38 ctor 6.0
    float field_0x3c;         // +0x3c ctor 1.0
    float field_0x40;         // +0x40 ctor 200.0
    EmitterVec3 position;    // +0x44 ctor 0
    EmitterVec3 previousPosition; // +0x50 ctor 0
    float field_0x5c;         // +0x5c ctor 0
    int field_0x60;           // +0x60 ctor 0
    float field_0x64;         // +0x64 ctor 0; slot 10 keeps the fractional part of dt*rate here
    int field_0x68;           // +0x68 ctor 0
    int field_0x6c;           // +0x6c ctor 0x1d
    int field_0x70;           // +0x70 ctor 0x14
    ParticleVec3 launchVelocity; // +0x74 ctor (0, 2, 0); slot 10 adds it to each new particle velocity
    float randomTable[256];   // +0x80 ctor fills rand()/32768*0.25
    int field_0x480;          // +0x480 ctor 0
    ParticleVec3 randomVectors[32]; // +0x484 ctor fills rand()/32768+0.5 per component
    int field_0x604;          // +0x604 ctor 0
};

// DirtSprayParticleEmitter: vtable 0x00555ba8.  Ctor 0x004b9310, deleting dtor 0x004b9430, dtor 0x004b9450,
// slot 10 0x004b9460 (970 bytes) (not reconstructed).  Size 0x608 (the ctor stores +0x604).
class DirtSprayParticleEmitter : public GameObject {
public:
    explicit DirtSprayParticleEmitter(int flags);
    virtual ~DirtSprayParticleEmitter();
    virtual int GameObjectVirtualSlot10(float dt);
    virtual int UnknownVirtualSlot27(void* a, ParticleManager* manager);

    int firstUpdate;          // +0x2c ctor 1
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor)
    float field_0x34;         // +0x34 ctor 750.0
    float field_0x38;         // +0x38 ctor 6.0
    float field_0x3c;         // +0x3c ctor 1.0
    float field_0x40;         // +0x40 ctor 200.0
    EmitterVec3 position;    // +0x44 ctor 0
    EmitterVec3 previousPosition; // +0x50 ctor 0
    float field_0x5c;         // +0x5c ctor 0
    int field_0x60;           // +0x60 ctor 0
    float field_0x64;         // +0x64 ctor 0; slot 10 keeps the fractional part of dt*rate here
    int field_0x68;           // +0x68 ctor 0
    int field_0x6c;           // +0x6c ctor 0
    int field_0x70;           // +0x70 ctor 0x1e
    ParticleVec3 launchVelocity; // +0x74 ctor (0, 2, 0); slot 10 adds it to each new particle velocity
    float randomTable[256];   // +0x80 ctor fills (rand()/32768+1)*0.25
    int field_0x480;          // +0x480 ctor 0
    ParticleVec3 randomVectors[32]; // +0x484 ctor fills x=y=z=rand()/32768*0.5+0.75
    int field_0x604;          // +0x604 ctor 0
};

// SparkParticleEmitter: vtable 0x00555c1c.  Ctor 0x004b9830, deleting dtor 0x004b9970, dtor 0x004b9990,
// slot 10 0x004b99a0 (1055 bytes, not reconstructed), position setter 0x004b9dc0.
class SparkParticleEmitter : public GameObject {
public:
    explicit SparkParticleEmitter(int flags);
    virtual ~SparkParticleEmitter();
    virtual int GameObjectVirtualSlot10(float dt);
    virtual int UnknownVirtualSlot27(void* a, ParticleManager* manager);
    void SetPosition(ParticleVec3 p);   // 0x004b9dc0

    int firstUpdate;          // +0x2c ctor 1
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor)
    float field_0x34;         // +0x34 ctor 500.0
    float field_0x38;         // +0x38 ctor 0
    float field_0x3c;         // +0x3c ctor 1.0
    float field_0x40;         // +0x40 ctor 2400.0
    EmitterVec3 position;    // +0x44 ctor 0; SetPosition's arguments
    EmitterVec3 previousPosition; // +0x50 SetPosition copies position here first
    float field_0x5c;         // +0x5c ctor 0
    int field_0x60;           // +0x60 ctor 0
    float field_0x64;         // +0x64 ctor 0; a timer: slot 10 adds dt while idle and emits while it is positive
    float field_0x68;         // +0x68 ctor 0; fractional particle remainder carried between ticks
    int field_0x6c;           // +0x6c ctor 0
    int field_0x70;           // +0x70 ctor 0x2d
    int field_0x74;           // +0x74 ctor 0x14
    ParticleVec3 launchVelocity; // +0x78 ctor (0, 2, 0); slot 10 adds it to each new particle velocity
    float randomTable[256];   // +0x84 ctor fills rand()/32768*0.075
    int field_0x484;          // +0x484 ctor 0
    ParticleVec3 randomVectors[8]; // +0x488 ctor fills each component rand()/32768*10-5
    char field_0x4e8[0x608 - 0x4e8]; // +0x4e8 not touched by the ctor
    int field_0x608;          // +0x608 ctor 0
};

// SteamParticleEmitter: vtable 0x00555c9c.  Ctor 0x004b9f40, deleting dtor 0x004ba010, dtor 0x004ba030,
// slot 10 0x004ba070 (590 bytes, not reconstructed), setters 0x004ba2c0..0x004ba360.
class SteamParticleEmitter : public GameObject {
public:
    explicit SteamParticleEmitter(int flags);
    virtual ~SteamParticleEmitter();
    virtual int GameObjectVirtualSlot10(float dt);
    virtual int UnknownVirtualSlot27(void* a, ParticleManager* manager);
    void SetPosition(ParticleVec3 p);   // 0x004ba2c0
    void Restart();                                   // 0x004ba300
    void Burst();                                     // 0x004ba320
    void StopEmitting();                              // 0x004ba340
    void SetColor(unsigned char r, unsigned char g, int b);   // 0x004ba360

    int firstUpdate;          // +0x2c ctor 1
    ParticleManager* manager; // +0x30 stored by slot 27 (null in the ctor)
    float field_0x34;         // +0x34 ctor 500.0
    float field_0x38;         // +0x38 ctor 3.0
    float field_0x3c;         // +0x3c ctor 0.1
    float field_0x40;         // +0x40 ctor 0.25; Restart copies it to +0x44
    float field_0x44;         // +0x44 ctor 0; Restart copies +0x40 here, Burst stores 0.0001
    EmitterVec3 position;    // +0x48 ctor 0; SetPosition's arguments
    EmitterVec3 previousPosition; // +0x54 SetPosition copies position here first
    int field_0x60;           // +0x60 ctor 0; Restart sets 1, StopEmitting clears it
    int field_0x64;           // +0x64 ctor 14
    int field_0x68;           // +0x68 ctor 0x80a
    int field_0x6c;           // +0x6c ctor 0; Restart and Burst set 1
    int field_0x70;           // +0x70 ctor 0; Burst sets 1
    float randomTable[256];   // +0x74 ctor fills each group of four with one value 2*rand()/32768+1; slot 10 reads randomTable[field_0x474]
    int field_0x474;          // +0x474 ctor 0
    int color;                // +0x478 ctor 0xffffff; SetColor packs r<<16|g<<8|b
};

#endif
