#include <stddef.h>
#include "TextureMap.h"
#include "SoultreeMaterial.h"

// The local six-word material options are passed to the loader unchanged.
typedef char SameSize[(sizeof(SoultreeTextureOptions)==sizeof(UnknownTextureFormatChoice))?1:-1];
typedef char MaterialSize[(sizeof(SoultreeMaterial)==0xd0)?1:-1];
#define SAME_OFFSET(a,b) typedef char Offset_##a[(offsetof(SoultreeTextureOptions,a)==offsetof(UnknownTextureFormatChoice,b))?1:-1]
SAME_OFFSET(textureManager,manager);
SAME_OFFSET(opaqueTextures,opaqueGroup);
SAME_OFFSET(alphaTextures,alphaGroup);
SAME_OFFSET(format,opaqueFormat);
SAME_OFFSET(field_0x10,alphaFormat);
SAME_OFFSET(field_0x14,halvings);
#undef SAME_OFFSET
