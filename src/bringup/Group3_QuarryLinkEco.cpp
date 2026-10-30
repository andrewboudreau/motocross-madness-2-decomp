// Bring-up glue (see Group3_QuarryLoadLinks.h): the EcoSystem and SkyCube calls made by
// BaseQuarryEvent::LoadEventObjects (0x004de590).
#include "../reconstructed/EcoSystem.h"
#include "../reconstructed/SkyCube.h"
#include "../reconstructed/DebugAlloc.h"
#include "Group3_QuarryLoadLinks.h"

// 0x00457250 (0x5c4 bytes).
void* G3Q_NewEcoSystem(const char* file, int line, int flags)
{
    return new(file, line) EcoSystem(flags);
}

// 0x004594d0: the environment argument is the scene environment record whose path
// the canonical declaration reads as char*.
void* G3Q_EcoSystemCreate(void* eco, void* target, void* textureManager, void* terrain,
                          void* lights, void* environment, void* stream, int format,
                          int collisions, int level)
{
    return ((EcoSystem*)eco)->Create(target, (TextureMapManager*)textureManager,
                                     (UnknownEcoTerrain*)terrain, (LightManager*)lights,
                                     (char*)environment, (UnknownTextureStream*)stream, format,
                                     collisions, level);
}

// 0x004fb1e0 (0x48 bytes).
void* G3Q_NewSkyCube(const char* file, int line, int flags)
{
    return new(file, line) SkyCube(flags);
}

// 0x004fb230.
void* G3Q_SkyCubeCreate(void* sky, void* target, void* stream, void* textures, float size)
{
    return ((SkyCube*)sky)->Create(target, (UnknownTextureStream*)stream,
                                   (UnknownCubeTextureContext*)textures, size);
}
