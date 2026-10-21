# Partial texture upload probe

`samples/render/PartialTextureUploads.cpp` contains the complete provisional
`void UnknownDisplay::ProbePartialTextureUploads(RenderTarget*)` body. The
declaration in `Display.h` and its caller in `PCGame::ProfileDisplays` use the
same thiscall symbol. Retail starts at `0x004cab00`; the extent is **1129 bytes**.
The only `ret 4` is at `0x004caeae`, followed by a reachable upload-success block
at `0x004caeb1` and a missing-file block at `0x004caf5c`. The latter ends with
the complete five-byte jump at `0x004caf64` back to cleanup. The next function
starts at `0x004caf70`, after seven padding bytes. The native test independently
checks complete decoding and every branch destination, including these tails.

The semantic method name remains provisional. Its diagnostic file argument
references retail `PCVideoCard.cpp`, but this standalone grouping does not claim
to reconstruct the original translation unit. The original aggregate sample
includes this file so the body has one maintained source. Header declaration
order is retained. The extracted standalone VC6 SP3 `/O2 /GR /GX /MT` object
contains one public function, **1145 bytes with 140 matching positions** against
the complete retail extent. In the original aggregate context it remains
1113 bytes with 144 matching positions. Extraction preserves the aggregate
instruction bytes after relocation operands are normalized; the diagnostic
source path and compiler-generated exception-label numbering change. Neither
form is a strict match, and no extent is shortened to accommodate either one.

## Functional build review

The probe loads `partblt.tga`, uses the supplied target or searches for a
640-by-480, 16-bit display mode and creates a temporary target. It creates three
texture objects, attempts a full upload, then tests sixteen 64-by-64 tiles. The
result is stored at display offset `0x5bc`. All seventeen `CreateSurface`
arguments, including signed file dimensions, formats, palette pointers, key and
alpha threshold, are retained. The initial full-upload result must equal 1;
subsequent tile results only need to be nonzero. It releases image, rendered
texture and expected texture in that order. A temporary target is deleted and
the global game's target/display fields cleared; a supplied target is retained.

Some behavior is deliberately unusual because it follows decoded retail:

- The file is loaded before finding a target. If no mode matches, result stays
  1 and the loaded file is not released. Failed target creation can instead
  leave result -3, with the same file ownership behavior.
- A missing file still permits temporary-target creation before reporting -2.
- Failure of the first texture's `CreateSurface` reports -2. The return values
  from the second and third `CreateSurface` calls are ignored. Failure of
  `CreateTextureSurface` reports -4; failed full upload reports -1; a zero tile
  result reports 0.
- Texture allocation returning null faults at the virtual-call boundary. No
  new null checks or recovery paths are added.
- Exceptions after construction do not acquire cleanup absent from retail.
  Existing files, textures, targets and transient global fields can remain live.

The function has four independent constructor-unwind states. Their complete
cleanup funclets are `0x0054cdd0`, `0x0054cde7`, `0x0054cdfe` and `0x0054ce15`,
each 23 bytes, followed by the ten-byte handler at `0x0054ce2c`. The fixture
checks candidate/retail exception-map shape and executes all four constructor
failure paths, including their matching placement deletes.

`tests/test_partial_texture_uploads.py` compiles the unchanged candidate as an
independent `/O2` object and a separate `/Od` fixture with authentic VC6 SP3.
The hash-pinned private retail code and unwind funclets are mapped only inside
the test executable. Only decoded direct dependency calls within those extents
are redirected. Production never maps or calls retail code.

The **26,600 finite scenarios** compare complete game/display/target storage,
texture allocation bytes and ownership, file state, mode records and ordered
call arguments. They record every upload rectangle and the transient global
game display/target pointers at every dependency boundary. The grid covers
supplied and created targets, zero through four modes, matching/nonmatching
modes and setup/initialization failures, missing files, signed dimensions,
pixel depths, each surface-creation return, full-upload return values and every
tile failure boundary. Additional cases exercise null allocations, allocation
and constructor exceptions, and exceptions during initialization, surface
creation, upload testing and release. Exactly **23 C++ exceptions and six access
violations** are observed and asserted. Fault comparisons include exception
code, read/write operation, accessed address and resulting ownership/state;
instruction addresses differ between executables and are not compared.

All allocation, image-loader, target, texture and upload-test dependencies are
scripted. The test does not establish real DirectDraw behavior, file decoding,
rendered pixel correctness or successful game startup. The scripted target
initializer accepts a null receiver to compare that caller boundary; this does
not claim the real initializer can safely do so. Diagnostic source-file paths
are checked before normalization. The finite cases are behavioral evidence,
not a proof for arbitrary dependency side effects or all possible inputs.

The standalone object's game-owned direct dependencies already have canonical
providers: `LoadFile`, `FreeFile`, `SetFullscreenDisplayMode`, the render-target
and texture constructors, `InitializeRenderTarget`, `TestPartialTextureUpload`,
debug allocation, the HAL GUID and `g_TrackGame`. Its virtual texture operations
use the existing canonical declarations. `TestPartialTextureUpload` itself
depends on the separate channel-difference candidate; validating this probe
does not validate that dependency. The complete probe is explicitly admitted
as nonexact in `config/game_build.json`; its real link removes its missing name
without introducing another. The final executable link remains incomplete.

Reproduce with private inputs in PowerShell:

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_partial_texture_uploads.py -v
```
