// Near misses for src/reconstructed/PCVideoCard.cpp (TU PCVideoCard.cpp).
// Each compiles from readable source but differs from retail as noted.
//
// UnknownDisplay::CreateModeSurfaces (0x004ca520, 116 bytes): logic and
//   calls match; retail loads +0x74 and all four mode fields into registers
//   before testing the argument and computes the free memory as
//   neg/lea (memory - pixels * 2); local orders and pixel temporaries tried,
//   as were a `memory` local read first and `width * height` written twice.
//   Negated tests sharing one trailing `return 1` (with and without a
//   `pixels` local or `memory` local) let VC6 sink every load into the
//   full-screen block (6-21 of 110-125). Sweeps (1152 + 1920 combinations:
//   all orders of the four field locals and an optional `memory` local, a
//   `pixels` local, three return structures, both comparison directions and
//   ten spellings of the budget: `m - p * 2`, `2 * p`, `-p * 2 + m`,
//   `p + p`, `p << 1`, sizeof(short) with casts, ...) give at most 27/124:
//   VC6 emits the same shl/sub for every spelling and loads +0x74 late.
//   Also 26 at best: the fields read through a `mode` pointer (retail does
//   form one with `lea` for +8) with or without `memory`/`pixels` locals,
//   and the budget written `m + -p * 2`, `m + 2 * -p` or `m - 2 * p`.
//   Retail forms width * height once (edi) and reuses it for the budget
//   (`neg edi ; lea edi, [ebp + edi*2]`); VC6 here reassociates the total
//   as bitDepth * height * width and multiplies width * height again. A
//   `pixels` local in the block (with the budget written into it, into a
//   `free` local, as `memory -= pixels * 2` or inline) or at the top gives
//   the shared product but sinks every field load (6 of 116).
//   Mechanism (session 14 probe): VC6 forwards a local that has a single
//   use into that use, so the bitDepth and budget loads move into the
//   full-screen block, and the forwarded bitDepth load then reorders the
//   product (bitDepth * height * width), which breaks the shared
//   width * height. Giving bitDepth and a `memory` local a second use in the
//   windowed call (a wrong-code probe) loads all five values at the top in
//   retail's registers, with retail's `lea` mode pointer for +8. So in
//   retail every local was materialised. These leave the loads forwarded
//   (26): dead copies of bitDepth and memory, register or const locals, an
//   unused label before the test, an inline reader with out-parameters,
//   inline fit/size check helpers (three forms), and the budget and need
//   computed into locals at the top. /G6 scores 10 and /O1 18. An empty
//   inline function given all five values, before the test or in the
//   windowed branch, is removed before the decision (26); a `pixels` and
//   `memory` local pair at the top gives 6.
// UnknownDisplay::ProbeNonLocalTextureMemory (0x004ca5a0, 486 bytes): retail keeps
//   `target` in ebp, caches 480/16 in registers for the mode search and has
//   one more 4-byte local; the flow (EH-guarded new PCRenderTarget, 128x128
//   probe surface, caps bit 0x20000000) is as written. Retail's ebp holds
//   `target` outside the mode loop (reloaded from its argument slot after it)
//   and 480 inside it; VC6 here never pushes ebp. Tried in session 10 (66 of
//   490 base): `if (!target) {loop} else render = target` (77), `render !=
//   target` or plain `!target` before each delete (65-85), `*value = 0`
//   after the `render` declaration, `render = target` as the initialiser;
//   none puts `target` in a register. Early-failure forms (s14, 70 base):
//   the CreateSurface failure tested first, with its delete and a goto to
//   `failed` (67, retail's block order lost) or its own `return 0` (94).
// float MeanTextureChannelDifference (0x004cb330, 629 bytes): register allocation
//   (retail: ebp = first, esi/edi = pixel pointers) and the x87 sequence for
//   the three channel sums (retail keeps the green difference on the stack
//   and stores the blue one) differ; the expansion and loop shapes match.
// UnknownDisplay::ProbePartialTextureUploads (0x004cab00, 1129 bytes; ours
//   1113): the PartialTexBlt driver. Retail sets tile.top/bottom in the row
//   loop, threads the loop exit straight to the result store (the created
//   target is copied into `render` only on success, so `render` keeps its
//   tested-null entry value) and places the `!file` (-2) arm after the
//   epilogue (`if (file) ... else`). Left: retail keeps the mode index in
//   esi shared with the created target (reloaded on failure, two separate
//   -3 stores; VC6 here merges them), tests `render` again after the found
//   path, and places the Slot8-success path after the epilogue.
#include "../../src/reconstructed/PCVideoCard.cpp"

#include <string.h>

#include "ModeSurfaces.cpp"

#include "NonLocalTextureMemory.cpp"

#include "TextureChannelDifference.cpp"

#include "PartialTextureUploads.cpp"
