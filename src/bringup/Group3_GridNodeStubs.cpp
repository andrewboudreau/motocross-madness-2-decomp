// Link bring-up definition (best-guess admission, see docs/GAME_BUILD.md):
// DrawableGridNode's shared empty helper. RTTI QuadTreeObject's two slots, which
// this file used to define for a stand-in base, are the canonical
// broadphase/QuadTreeObject.h ones (slot 0 in Quadtree.cpp, slot 1 inline).
//
// Evidence (decoded retail): 0x00464e90 is the shared `ret` body.
#include "Griddraw.h"

// 0x00464e90: shared empty body.
void DrawableGridNode::OnMaskCleared()
{
}
