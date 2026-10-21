# Creating texture surfaces

`samples/render/TextureSurfaceCreate.cpp` contains the complete readable
`PCTextureMap::CreateSurface` candidate extracted unchanged from
`PCTextureMapNearMisses.cpp`, including its source-level render-state append
helper. The aggregate now includes that file. The missing provider is required
by PCTextureMap, ManagedTexture and CacheTexture vtables and by the menu screen
capture path. The test independently verifies primary vtable `0x00555fec`,
object offset zero and slot 4's target `0x004c69c0`. The semantic method name
and original translation-unit ownership remain provisional.

## Complete extent and source preservation

The retail body is **2,033 bytes**, `0x004c69c0..0x004c71b1`. Its four
`ret 0x44` instructions are at `0x004c6d47`, `0x004c7093`, `0x004c70ed` and
`0x004c71ae`. Fifteen NOP bytes precede Clone at `0x004c71c0`. The old
2,031-byte extent stopped inside the last return instruction; it was not a
complete function. The test decodes every instruction and validates all direct
branch destinations before comparing the complete extent.

VC6 SP3 `/O2` emits **2,028 bytes**, with **518 matching positions** against
that complete retail body. The standalone and aggregate forms have identical
full emitted bodies and relocation roles. No source algorithm has changed,
and no function extent has been shortened. There is no local EH registration,
cleanup funclet, CRT initializer or additional public function in this object.

The independent baseline/current aggregate proof against `7dd638b8` preserves
all **three functions** and **six nondebug sections**, including complete bytes
and relocation destinations, types and addends. The extracted body has no
filename references. Only the known temporary baseline aggregate's diagnostic
filename is normalized to the current aggregate filename; the extraction adds
no filename storage. Private proof:
`work/game-build/check_texture_create_extraction.py` and
`work/game-build/texture-create-extraction-proof.json`.

## Functional build review

The functional build explicitly admits this standalone source as
`reviewed-nonexact`. Fresh build `build-8xuilhwf` compiles **214/214** source
units and reduces unresolved names from **405 to 404** against `build-_oyof5cf`:
only `PCTextureMap::CreateSurface` is removed, with no new missing names.
The paired data provider supplies the debug flag this body references.
The game still does not link or run. The isolated
comparison is recorded in `work/game-build/texture-create-link-delta.json`.

The function records dimensions, palettes and requested format, applies an
optional format choice, and prepares either a single surface or mip chain.
It can reuse shared 555/565 surfaces, attach a palette, convert input pixels,
compute average colour, generate mip levels and append alpha/render states.
The non-mip path tries the evidenced format fallbacks or calls DirectDraw
for other formats; the mip path has no general DirectDraw fallback. Invalid
formats can therefore reach a null surface rather than an invented fallback.

Existing surfaces skip creation and palette setup. Shared reuse takes one
reference for the texture and another when the shared entry is assigned.
Conversion and average-colour return values have their observed distinct uses;
conversion's return is ignored, average colour is stored. DumpLevel and
ApplyColorKey return values are ignored. Creation, palette, caps, lock,
unlock and mip-generation failure rules are retained. No local release or
unlock cleanup is added on failure or exception. For example, an exception
from conversion leaves the scripted surface locked.

`tests/test_texture_surface_create_behavior.py` passes **12,226 native
scenarios**, executing the complete hash-pinned retail body and the actual
separately compiled candidate. The fixture uses `/Od`, while the candidate
and three source-format helpers use `/O2`. It checks all **20 direct call
sites** and **12 indirect dispatch sites**. Only audited direct calls in the
private reference mapping are redirected; source-built production contains
neither retail code nor fixture dispatch.

Three complete format-helper bodies are generated at test time directly from
current canonical `Tgafile.cpp`, with only their test names changed. Their
full emitted extents must match retail strictly: bytes per pixel **78 bytes**,
pixel-format descriptor **266 bytes**, and alpha-format predicate **30 bytes**.
This is validation-only source extraction, not a second production definition
or an original-TU claim. Their actual results are used by both executions.

The comparison records return/exception/fault results, complete tracked object
and buffer bytes, shared-array contents, reference and lock counts, ordered
arguments and state at every scripted boundary. Independent assertions cover:

- Complete 124-byte descriptors, pixel-format contents, capability flags,
  ordered format fallbacks, mip counts, shared assignments and AddRef counts.
- Conversion source payload and all arguments, including signed destination
  pitch division, dimensions, source stride, formats, palette, alpha threshold
  and colour key. The resulting scripted payload and average-colour store are
  checked independently.
- Alpha fallback to colour key and exact appended render-state pairs, including
  address-mode values. Bounded initial pair counts also expose overlapping
  field writes in the fixture's extended backing storage.
- Positive/zero/negative helper results, output writes on failure, null or
  unwritten output pointers, surface aliases, and ignored return values.
- Replacement game/display/manager/surface pointers and shared entries at
  callback boundaries; exceptions at every boundary in representative paths;
  actual null-pointer and integer-division faults.

Mip scenarios use **positive minimum sizes** and dimensions chosen to finish
within the existing ten-entry shared arrays (tested widths/heights through
128). A nonpositive minimum can make the retail loop run indefinitely and is
outside this finite contract. COM surfaces, scratch growth, system-surface
creation, conversion, average colour, mip building, colour-key application
and dumping are scripted. The conversion payload checks call arguments and
state effects, **not pixel-conversion correctness**. This test does not create
DirectDraw surfaces, exercise drivers, prove arbitrary malformed-input safety,
or establish integrated texture rendering or menu capture.

All game helper functions already have canonical providers. The actual objects from
`build-_oyof5cf` are strict for the eight direct helpers: the three format
helpers above, GrowSecondScratchBuffer **88 bytes**, CreateSystemSurface
**212**, DumpLevel **101**, ConvertPixels **1,964**, and AverageColor **987**.
The two virtual game dependencies are also strict: BuildMipLevels **402** and
ApplyColorKey **457**. The TrackGame pointer and both shared-surface arrays
have existing storage providers. These are separate whole-body byte checks;
the larger helpers are not executed by this scripted fixture.
See `work/game-build/texture-create-provider-proof.json`.

`src/reconstructed/TextureSurfaceState.cpp` supplies the newly referenced
`g_DebugDumpTextureLevels` as an ordinary mutable `int`. Its four retail bytes
at `0x00689968` lie beyond raw data in the writable `.data` section's virtual
tail and are zeroed by the PE loader. The sole encoded address occurrence
across file-backed sections is operand `0x004c7097` of the full-DWORD load at
`0x004c7096`. No other direct address reference was located; this does not
exclude possible indirect writes. The definition preserves the nonzero dump
branch and does not replace it with constant false.

The new data-only object has exactly four bytes of writable BSS, no imports,
functions, CRT initializer or nondebug relocations. The separate file avoids
changing VC6's existing GraphicsState BSS symbol order; that source and all
its storage remain unchanged. Original TU ownership is unknown.
`tests/test_graphics_state.py` verifies the full value, extent, section flags,
loader-zero range and DWORD reference. Private object evidence is recorded in
`work/game-build/texture-dump-storage-proof.json`.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_texture_surface_create_behavior.py -v
```
