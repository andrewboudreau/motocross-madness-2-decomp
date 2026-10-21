// Physics caller view: this TU intentionally does not import the rendering view.
#include "vehicle/KrustyBike.h"
#include "soultree/SoultreePhysicsObject.h"
#include "../../src/reconstructed/GhostModCore.h"
#include "../../src/reconstructed/MorphBastardModifierCore.h"

typedef char GhostAllocation[(sizeof(GhostMod1) == 0x48) ? 1 : -1];
typedef char MorphAllocation[(sizeof(MorphBastardModifier) == 0x58) ? 1 : -1];
typedef char ModelExtent[(sizeof(D3DIMSoultreeObject) == 0x2d8) ? 1 : -1];

extern "C" GameObject* GhostPrimary(GhostMod1* value) { return value; }
extern "C" GameObject* MorphPrimary(MorphBastardModifier* value) { return value; }
extern "C" GameObject* ModelSecondary(D3DIMSoultreeObject* value) { return value; }
