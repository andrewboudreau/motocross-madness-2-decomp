// Bring-up (best-guess-unreviewed): the particle emitters and procedural car
// the Scene readers (src/bringup/Group4_SceneRead.cpp) construct.
//
// Every constructor named here is a canonical src/reconstructed provider; this
// unit only gives the Scene readers' `new` sites a body.
#include "../reconstructed/ParticleEmitters.h"
#include "../reconstructed/CarProcedural.h"

// 0x004b8a00 DustParticleEmitter(flags), `new` site of the caller.
void* Group4Scene_NewDustEmitter(const char* file, int line, int flags) {
    return new(file, line) DustParticleEmitter(flags);
}

// 0x004b8df0 DirtChunkParticleEmitter(flags).
void* Group4Scene_NewChunkEmitter(const char* file, int line, int flags) {
    return new(file, line) DirtChunkParticleEmitter(flags);
}

// 0x004b9310 DirtSprayParticleEmitter(flags).
void* Group4Scene_NewSprayEmitter(const char* file, int line, int flags) {
    return new(file, line) DirtSprayParticleEmitter(flags);
}

// 0x0042f390 CarProcedural(flags).
void* Group4Scene_NewCarProcedural(const char* file, int line, int flags) {
    return new(file, line) CarProcedural(flags);
}
