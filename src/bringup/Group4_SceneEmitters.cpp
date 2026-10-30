// Bring-up (best-guess-unreviewed): the particle emitters and procedural car
// the Scene readers (src/bringup/Group4_SceneRead.cpp) construct.
//
// DirtSprayParticleEmitter (ctor 0x004b9310, vtable 0x00555ba8) has no src
// provider yet; its reviewed-shape candidate samples/physics/effects/
// DirtSprayParticleEmitter.cpp is compiled here unchanged (it links against
// existing providers only). The other constructors already exist.
#include "../../samples/physics/effects/DirtSprayParticleEmitter.cpp"
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
