// Near-miss PCTextureMap candidates, kept out of src/reconstructed until
// they match. See docs/PCTEXTUREMAP.md.
//
// CopyRectTo is included from TextureRectCopy.cpp. The complete nonexact
// body preserves retail's low-address read and destination-before-source
// WORD access order; see docs/TEXTURE_RECT_COPY.md for the native proof.
//
// PCTextureMap::Clone (0x004c71c0, 600 bytes): the copy.
// Everything lines up except that VC6 here keeps the constant 1 in ebp and
// the copy in ebx (retail: ebx and ebp), which also moves the 8-bit key
// expansion's scratch registers. The shape that gets this close: the
// cached flag normalised to 0/1 (`(field_0x68 & 1) != 0`, stored in the
// slot the `new` temporary reuses), the manager in a local before each
// `new`, the copy assigned directly (a separate ManagedTexture local
// assigns ebp before the registration call; retail after), the two
// caps-derived flags as separate `if`s, and the ManagedTexture `new`
// first (its EH state is 0). `int cached = field_0x68 & 1` keeps the
// store but not the constant register; `if (field_0x68 & 1)` keeps the
// register but tests the byte in place.
// A bool/char `cached` gets retail's ebx/ebp split (the byte `and al, bl`
// needs a byte register) but takes its own byte slot and grows the frame;
// unsigned/long/ternary/!! forms, separate copy locals, a single return,
// 1..33 dead locals and moving Clone within the TU leave it unchanged.
//
// PCTextureMap::CreateSurface (0x004c69c0, 2033 bytes): the setup.
// Everything but the non-mip format fallback chain lines up, including
// the mip chain (same source shape). VC6 cross-jumps the identical
// CreateSystemSurface call tails of that chain into the first case
// (4444); retail merges them into the last (565) in mirror order, which
// also changes which blocks interleave their array stores with the pushes.
// if/else, nested negated ifs, switch, goto-to-label, return/goto mixes,
// aggregate initializers and an inline helper all keep VC6's order.

#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/TrackGame.h"

#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/D3DConstants.h"

#include "TextureRectCopy.cpp"

// 0x004c71c0: a copy of the texture (a ManagedTexture registered with this
// one's cache, or a PCTextureMap of the game's manager), filled from +0x70
// through its slot 4 and given the same colour key; 0 on failure.
#include "TextureClone.cpp"

// Appends a render-state pair for slot 19 to apply.
#include "TextureSurfaceCreate.cpp"
