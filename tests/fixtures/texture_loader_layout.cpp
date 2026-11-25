#include <stddef.h>
#include "TextureMap.h"
#include "SoultreeMaterial.h"

// SoultreeMaterial keeps the loader's TextureContext at +0x74.
typedef char ContextSize[(sizeof(TextureContext)==0x18)?1:-1];
typedef char MaterialSize[(sizeof(SoultreeMaterial)==0xd0)?1:-1];
typedef char MaterialContext[(offsetof(SoultreeMaterial,textureOptions)==0x74)?1:-1];
#define AT(f,o) typedef char Offset_##f[(offsetof(TextureContext,f)==o)?1:-1]
AT(manager,0x0);
AT(opaqueGroup,0x4);
AT(alphaGroup,0x8);
AT(opaqueFormat,0xc);
AT(alphaFormat,0x10);
AT(halvings,0x14);
#undef AT
