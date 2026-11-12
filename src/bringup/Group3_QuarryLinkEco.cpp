// Bring-up glue (see Group3_QuarryLoadLinks.h): the EcoSystem call made by
// BaseQuarryEvent::LoadEventObjects (0x004de590).
#include "../reconstructed/EcoSystem.h"
#include "Group3_QuarryLoadLinks.h"

// 0x004594d0: the environment argument is the scene environment record whose path
// the canonical declaration reads as char*.
void* G3Q_EcoSystemCreate(void* eco, void* target, void* textureManager, void* terrain,
                          void* lights, void* environment, void* stream, int format,
                          int collisions, int level)
{
    return ((EcoSystem*)eco)->Create(target, (TextureMapManager*)textureManager,
                                     (UnknownEcoTerrain*)terrain, (LightManager*)lights,
                                     (char*)environment, (FileStream*)stream, format,
                                     collisions, level);
}
