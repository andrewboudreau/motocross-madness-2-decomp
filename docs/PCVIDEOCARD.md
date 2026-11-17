# PCVideoCard.cpp and VideoCard.cpp

`src/reconstructed/VideoCard.h/.cpp` and `PCVideoCard.h/.cpp`. Evidence:
the `__FILE__` literals of both files (PCVideoCard.cpp lines 280, 307,
381 and 2053–2110; VideoCard.cpp line 63, xref `0x0052d22a`), RTTI
`PCVideoCard : VideoCard` (vtables `0x00556df4` and `0x00558d74`,
constructor `0x004c9760`, 0xb78 bytes) and the DirectDraw 7 API the code
calls (IIDs confirmed by their GUID bytes). PCVideoCard.cpp spans
`0x004c94a0..0x004cb667`, VideoCard.cpp `0x0052d0d0..0x0052d2bc`. Names
other than the RTTI classes are provisional.

The display object (Game+0x0c) is a `PCVideoCard`: its constructor
`0x004c9760` writes vtable `0x00556df4` (`0x004c9771`), and both
DirectDraw enumeration callbacks construct it. The class is declared in
`PCVideoCard.h`; its callers (GUIManager, PCGame, Game, EventManager,
PickDevice, PCRenderTarget, RenderTarget) bind the real mangled names, and
the source-built game's `PCVideoCard` and `VideoCard` vtables match every
retail slot (`tools/audit_game_vtables.py`). It enumerates
DirectDraw devices and display modes (`DirectDrawEnumerateExA`,
`EnumDisplayModes`, a sorted 36-byte mode table), creates the device,
sets cooperative level and display mode (full screen or windowed with a
clipper), builds the flip chain and render surfaces with a gamma ramp,
presents frames, and reads the back buffer back for partial texture
blits. `0x004ca790` and `0x004ca900` return int.

## Readable reconstruction names

Display creation and mode changes now have names such as `InitializeDisplay`,
`SetFullscreenCooperativeLevel`, `CreateFlipChain`, `CreateWindowedSurfaces`
and `SetFullscreenDisplayMode`. The partial-upload probes use
`CopyRenderSurfaceToTexture`, `ProbePartialTextureUploads`,
`TestPartialTextureUpload` and `MeanTextureChannelDifference`. The base
helpers are `CompareDisplayModes`, `FindDisplayMode` and `RecordFrameTime`.
These names describe decoded calls, arguments and data flow (tier 3);
retail entry addresses remain in comments and bindings.

The DirectDraw/Direct3D objects, primary/back/render surfaces, driver identity,
mode table, presentation flags and timing fields have role names, with
their decoded offsets retained beside the declarations. DirectX interface
methods and flag values follow the SDK layouts (strong inference from IIDs,
slot order and call shapes). Remaining uncertain fields and virtual contracts
keep their offset/slot names. Callers and near-miss samples use the same names;
binding addresses and calibration extents are unchanged.

The current authentic VC6 gate retains every prior strict calibration case.
Function-byte checks remain distinct from complete display-driver integration.

## Matching status

Exact: 30 calibration cases (23 PCVideoCard, 7 VideoCard). The GDI-surface
flip `0x004cb5b0` returns int (0 when a flip, the GDI surface query or the
flip budget fails); its flip loop is a guarded do-while (`int i = 0; if
(primarySurface != gdi) do ... while (primarySurface != gdi)`), which keeps
retail's unrotated `je; jmp top` exit with the shared `return 0` after it.
Not exact, in retail order in PCVideoCard.cpp (forms tried below):
`0x004ca520`, `0x004ca5a0`, the PartialTexBlt driver `0x004cab00`
and `0x004cb330`. The VideoCard constructor `0x0052d180` is exact once the
bit field and +0x68 are cleared before the two 0x7fffffff times (that order
schedules retail's `xor esi, esi` ahead of the bit-2 mask).

## Functional build candidates

The four near misses now have independent standalone sources included by the
aggregate sample. Each is explicitly nonexact in the game-build configuration:
[mode-surface setup and channel comparison](DISPLAY_SURFACE_HELPERS.md),
[non-local texture memory](NONLOCAL_TEXTURE_MEMORY.md) and
[partial texture uploads](PARTIAL_TEXTURE_UPLOADS.md). Their native comparisons
cover complete bodies, dependency contracts and ownership. The aggregate's
normalized instruction bytes and relocation structure remain unchanged.

Complete control-flow decoding corrects two old retail extents: the memory
probe is 486 bytes, including the complete final `ret 8`; channel comparison
is 629 bytes, including its zero-height cold branch. Candidate extents remain
490 and 620 bytes respectively. These corrections do not promote either body
to a strict match or alter any existing strict calibration case.

## Forms tried

Near misses for src/reconstructed/PCVideoCard.cpp (TU PCVideoCard.cpp).
Each compiles from readable source but differs from retail as noted.

PCVideoCard::CreateModeSurfaces (0x004ca520, 116 bytes): logic and
  calls match; retail loads +0x74 and all four mode fields into registers
  before testing the argument and computes the free memory as
  neg/lea (memory - pixels * 2); local orders and pixel temporaries tried,
  as were a `memory` local read first and `width * height` written twice.
  Negated tests sharing one trailing `return 1` (with and without a
  `pixels` local or `memory` local) let VC6 sink every load into the
  full-screen block (6-21 of 110-125). Sweeps (1152 + 1920 combinations:
  all orders of the four field locals and an optional `memory` local, a
  `pixels` local, three return structures, both comparison directions and
  ten spellings of the budget: `m - p * 2`, `2 * p`, `-p * 2 + m`,
  `p + p`, `p << 1`, sizeof(short) with casts, ...) give at most 27/124:
  VC6 emits the same shl/sub for every spelling and loads +0x74 late.
  Also 26 at best: the fields read through a `mode` pointer (retail does
  form one with `lea` for +8) with or without `memory`/`pixels` locals,
  and the budget written `m + -p * 2`, `m + 2 * -p` or `m - 2 * p`.
  Retail forms width * height once (edi) and reuses it for the budget
  (`neg edi ; lea edi, [ebp + edi*2]`); VC6 here reassociates the total
  as bitDepth * height * width and multiplies width * height again. A
  `pixels` local in the block (with the budget written into it, into a
  `free` local, as `memory -= pixels * 2` or inline) or at the top gives
  the shared product but sinks every field load (6 of 116).
  Mechanism (session 14 probe): VC6 forwards a local that has a single
  use into that use, so the bitDepth and budget loads move into the
  full-screen block, and the forwarded bitDepth load then reorders the
  product (bitDepth * height * width), which breaks the shared
  width * height. Giving bitDepth and a `memory` local a second use in the
  windowed call (a wrong-code probe) loads all five values at the top in
  retail's registers, with retail's `lea` mode pointer for +8. So in
  retail every local was materialised. These leave the loads forwarded
  (26): dead copies of bitDepth and memory, register or const locals, an
  unused label before the test, an inline reader with out-parameters,
  inline fit/size check helpers (three forms), and the budget and need
  computed into locals at the top. /G6 scores 10 and /O1 18. An empty
  inline function given all five values, before the test or in the
  windowed branch, is removed before the decision (26); a `pixels` and
  `memory` local pair at the top gives 6.
PCVideoCard::ProbeNonLocalTextureMemory (0x004ca5a0, 486 bytes): retail keeps
  `target` in ebp, caches 480/16 in registers for the mode search and has
  one more 4-byte local; the flow (EH-guarded new PCRenderTarget, 128x128
  probe surface, caps bit 0x20000000) is as written. Retail's ebp holds
  `target` outside the mode loop (reloaded from its argument slot after it)
  and 480 inside it; VC6 here never pushes ebp. Tried in session 10 (66 of
  490 base): `if (!target) {loop} else render = target` (77), `render !=
  target` or plain `!target` before each delete (65-85), `*value = 0`
  after the `render` declaration, `render = target` as the initialiser;
  none puts `target` in a register. Early-failure forms (s14, 70 base):
  the CreateSurface failure tested first, with its delete and a goto to
  `failed` (67, retail's block order lost) or its own `return 0` (94).
float MeanTextureChannelDifference (0x004cb330, 629 bytes): register allocation
  (retail: ebp = first, esi/edi = pixel pointers) and the x87 sequence for
  the three channel sums (retail keeps the green difference on the stack
  and stores the blue one) differ; the expansion and loop shapes match.
PCVideoCard::ProbePartialTextureUploads (0x004cab00, 1129 bytes; ours
  1120, 423 positions): the PartialTexBlt driver. Retail matches it up to
  0x004cac5c. Left: the `this` and first-texture frame slots are swapped,
  the texture is cached in ebx instead of keeping ebx as the zero register,
  and retail places the CreateTextureSurface success arm after the `ret`.
k-scan (s22): compiled behind 0..63 prepended typedefs, 0x004ca520,
0x004ca5a0, 0x004cab00, 0x004cb330 have the same code at every count, so
their differences are not a declaration-count effect
(docs/VC6_OPERAND_ORDER.md section 2).
