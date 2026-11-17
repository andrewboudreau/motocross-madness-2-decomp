# Capturing the menu background

`GUIManager::CopyScreenToTexture` `0x00486170`, demanded by GameUi's background
grab path, is in [GUIManager.cpp](../src/reconstructed/GUIManager.cpp) between
`SetGuiPalette` `0x00486150` and `0x004864f0`, with a `// Not exact` status
comment. The class is identified by RTTI; the semantic method name remains
provisional. The behaviour test links a validation unit named `GUIManager.cpp`
holding only this definition, so the fixture checks retail's file name.

## Extent and preservation

The complete retail body is **896 bytes**, `0x00486170..0x004864f0`.
Its final `ret 8` is at `0x004864ed`; the next function starts immediately
at `0x004864f0`. All direct branch destinations are decoded instruction
boundaries in the body. Its single-state unwind map at `0x00561898` points
to the complete 26-byte allocation cleanup at `0x0054b870`; the 10-byte
handler at `0x0054b88a` uses FuncInfo `0x00561878`.

VC6 SP3 `/O2` emits **888 bytes**, with **284 matching positions** after
relocation resolution. No extent is shortened. The regression checks the
validation unit's complete emitted body and relocation roles against the
canonical unit's.

## Functional build review

The function releases a held background DC, hides the cursor and tooltip,
renders/presents a frame, advances its index, then shows those objects again.
It locks the render surface, falling back to the primary surface when absent.
On lock success it copies the selected rectangle row by row into a raw buffer,
unlocks the same surface, optionally dims pixels, creates a PCTextureMap,
passes the buffer to its virtual CreateSurface operation, frees the raw buffer
and returns the texture. A nonzero lock result returns null without allocating
or unlocking. CreateSurface's return value is ignored.

The 8-bit dim path zeros alternating pixels on alternating rows. The 16-bit
path uses the format-specific 555/565 mask; the 24- and 32-bit paths halve
three channels and retain the fourth byte for 32-bit pixels. Row copying
preserves the distinction between surface pitch and packed destination width,
including negative surface pitch in the tested valid-memory cases.

Allocation and exception behavior is retained exactly. If the texture
constructor throws, placement delete frees that allocation but the earlier
raw capture buffer remains live. An exception from CreateSurface retains both
allocations. Null texture allocation faults while loading its vptr before the
EH state is cleared, so unwind invokes placement delete with null. No cleanup
absent from the retail body is added.

`tests/test_gui_screen_texture_behavior.py` passes **8,237 native scenarios**
against the hash-pinned complete retail body and the separately compiled
candidate. The fixture is compiled with `/Od`; the candidate retains `/O2`.
Only thirteen audited direct call/jump sites in the private reference mapping
are redirected, including the allocation cleanup and CRT exception handler.
All five indirect dispatch sites retain the retail ABI and execute scripted
render, surface and texture boundaries.

The comparison checks return identity, all tracked object/storage bytes,
ordered arguments, allocation lifetime, exception/fault details, and state at
every callback boundary. Coverage includes:

- Pixel widths 0 through selected 15-pixel edges, heights through 6, byte widths
  0 through 5, both 16-bit masks, optional cropping, negative/positive pitches,
  primary/render surfaces, and dimming. An independent per-pixel oracle checks
  every format/crop/pitch grid case, including unchanged guard bytes and alpha.
- Zero, positive and negative lock/callback results, ignored returns, null
  raw/texture allocations, and independent successful/failure/unwind outcomes.
- Mutations at every boundary in four representative pixel paths, including
  replacement global game, target/display/child pointers, rectangle/format,
  manager, surface selection and pixel data. Distinct objects expose cached
  values versus later reloads.
- Exceptions at each boundary, actual null-pointer faults, and signed-dimension
  extremes observed through allocation arguments before a scripted exception.
  Additional finite nonpositive-height cases reach later operations.

All ten direct game-owned dependencies already have strict canonical source
providers in `build-kioe61ys`: class-name enable/disable (308 bytes each),
ReleaseDc (41), AdvanceFrame (40), PCTextureMap constructor (46), bytes-per-pixel
lookup (78), DebugMalloc (57), DebugFree (96), debug new (70) and its placement
delete (96). Complete object comparisons are recorded in
`work/game-build/gui-screen-provider-proof.json`. The global TrackGame pointer
and VC6 CRT exception support are existing providers. PCTextureMap::CreateSurface
has a separate [reviewed candidate](TEXTURE_SURFACE_CREATE.md#functional-build-review);
this capture fixture continues to script that virtual boundary.

The fixture's minimal class declarations are private ABI boundary views, not
replacement production types. Rendering, format lookup, COM surfaces, allocator
accounting and texture creation are scripted. It does not test actual DirectDraw,
real screen capture, modal-menu rendering, full texture ownership or arbitrary
overlapping buffers. The private allocator raises on requests above 8 KiB;
extreme/overflowing dimensions are tested only through that bounded dependency
or a scripted allocation exception, not by executing unbounded memory loops.
These are finite behavioral checks, not universal equivalence for invalid inputs.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_gui_screen_texture_behavior.py -v
```

## Forms tried

0x00486170 (896 bytes): same logic; retail keeps three more stack locals
  (frame 0xb0 against 0xa4) and allocates registers differently around
  the row copy and the 8-bit dim loop. Retail has two return tails
  (`mov eax, edi` for the new texture, `mov eax, ebx` with ebx cleared
  at the top for the Lock failure) and never uses that zero for the
  null tests or `push 0`; VC6 here caches 0 in ebx for both. A
  `return texture` in the success branch gives the two tails (304, 73
  instruction diffs) but folds the failure return to `xor eax, eax`;
  the texture's zero assigned at its declaration, after the format
  read or just before the Lock test does not change that; an early
  `return texture` or `return 0` on a Lock failure gives 294. The row
  copy written without the row pointer (`memcpy(bits + y * width *
  bytesPerPixel, ...)`) reproduces retail's copy loop instruction for
  instruction (38 instruction diffs against 70) but scores 249/901;
  the 8-bit dim loop as `for (x = line & 1; x < width; x += 2)
  bits[line * width + x] = 0;` gives 68.
  Session 14: `texture = 0` after the bytes-per-pixel read puts the
  `xor` after that call as retail does (strict 300, same hunks), but the
  zero still serves the null tests. Retail returns the new texture from
  edi, which it zeroes before the new-result test and shares with the EH
  state store; here ebx holds the texture and the null arm is
  `xor ebx, ebx` after the branch.
