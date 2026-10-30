// Link bring-up definitions (best-guess admissions, see docs/GAME_BUILD.md):
// RTTI QuadTreeObject's two slots and DrawableGridNode's shared empty helper.
//
// Evidence (decoded retail): QuadTreeObject vtable 0x005511c4 (COL 0x0055b038) holds
//   slot 0 0x004dc610: `or ax, 0xffff; ret`      (no stack arguments, returns 16-bit -1)
//   slot 1 0x00434ce0: `mov eax, 0xf; ret 4`     (one stack argument, returns 15)
// 0x00464e90 is the shared `ret` body.
//
// best guess / ABI caveat: the canonical declarations in D3DIMSoulTree.h are
// `void QuadTreeVirtualSlot0()` and `int QuadTreeVirtualSlot1()`, which must be kept
// for the mangled names.  Retail slot 1 pops one stack argument and slot 0 returns a
// short; no reconstructed caller invokes either slot by name, so the bodies below only
// fill the vtable entries.  A caller that pushes the retail argument for slot 1 would
// be stack-imbalanced against this definition until the declaration is reconciled.
#include "D3DIMSoulTree.h"
#include "Griddraw.h"

// Slot 0: 0x004dc610 (returns (short)-1 in ax; declared void here).
void UnknownSoultreeQuadTreeObject::QuadTreeVirtualSlot0()
{
}

// Slot 1: 0x00434ce0 returns 15 (argument ignored in retail).
int UnknownSoultreeQuadTreeObject::QuadTreeVirtualSlot1()
{
    return 15;
}

// 0x00464e90: shared empty body.
void DrawableGridNode::UnknownFunction464e90()
{
}
