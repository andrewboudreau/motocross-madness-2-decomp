// Link bring-up definitions (best-guess admissions, see docs/GAME_BUILD.md):
// RTTI QuadTreeObject's two slots and DrawableGridNode's shared empty helper.
//
// Evidence (decoded retail): QuadTreeObject vtable 0x005511c4 (COL 0x0055b038) holds
//   slot 0 0x004dc610: `or ax, 0xffff; ret`      (no stack arguments, returns 16-bit -1)
//   slot 1 0x00434ce0: `mov eax, 0xf; ret 4`     (one stack argument, returns 15)
// 0x00464e90 is the shared `ret` body.
//
// best guess: the D3DIMSoulTree.h declarations follow those bodies (a short return
// for slot 0, one ignored stack argument for slot 1), as the canonical QuadTreeObject
// in krusty2/collision/CollisionObject.h does.  Before they did, the build's slot 1
// returned with a plain `ret`, unbalancing any caller that pushes the quadtree.
#include "D3DIMSoulTree.h"
#include "Griddraw.h"

// Slot 0: 0x004dc610 returns (short)-1 in ax.
short UnknownSoultreeQuadTreeObject::QuadTreeVirtualSlot0()
{
    return -1;
}

// Slot 1: 0x00434ce0 returns 15; the argument is ignored.
unsigned int UnknownSoultreeQuadTreeObject::QuadTreeVirtualSlot1(void*)
{
    return 15;
}

// 0x00464e90: shared empty body.
void DrawableGridNode::OnMaskCleared()
{
}
