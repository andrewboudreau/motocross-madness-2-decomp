# Shared visibility clipper pointer

Six provisional pointer declarations now use the existing
`VisibilityClipper* g_visibilityClipper` declaration. They all refer to the
same retail pointer cell at `0x00575a98`. This removes six duplicate unresolved
names; it does **not** supply that cell, its pointed-to object, or another
clipper method.

The retired pointer types are `D3DIMShadowClipper`, `UnknownCubeClipper`,
`UnknownEcoVisibilityClipper`, `UnknownFollowCameraProjector`,
`GridVisibilityClipper`, and `UnknownProjector`. The last view used the variable
name `g_UnknownGlobal575a98`; the other five used `g_visibilityClipper`.
All now import `?g_visibilityClipper@@3PAVVisibilityClipper@@A`.
The provisional classes remain separate. Shared pointer storage does not prove
their complete C++ type identity.

## Pointer and receiver evidence

The retail writable `.data` cell contains `0x0068acd4`. An all-byte scan of the
pinned retail image finds exactly 46 encoded references to `0x00575a98`, all
decoded as `mov ecx, [0x00575a98]`. The retained test checks every instruction
and its subsequent direct call. The consumers include CubeDraw, FollowCamera,
TrackOverlay, EcoSystem, Griddraw, the shadow source and VisibilityQuadTree.
This establishes the direct pointer loads and receiver bits; it does not
exclude arbitrary indirect writes.

Calls already using the canonical clipper methods keep their existing
boundaries. Calls to the remaining provisional `SphereInFrustum` and `TestBox`
declarations retain explicit casts at the call site. There are no new method
joins, wrappers, pointer adjustments or copied object storage. The complete
object comparison checks those casts generate the same instructions and call
destinations.

`VisibilityClipperGlobal.h` contains only the forward declaration and extern
pointer. Headers which only share that pointer include this narrow declaration;
code requiring the class methods continues to include `VisibilityClipper.h`.
The class declaration and layouts are unchanged. This distinction matters to
VC6: exposing the full class through all these headers exchanged two relocation
targets in Terrain's 3,173-byte rendering method. The narrow dependency restores
both original targets and every byte. That exchange is not an accepted
normalization or partial-match exception.

## Storage remains unresolved

`0x0068acd4` lies in the zero-filled tail of the writable retail section. The
neighboring vector initializers finish at `0x0068acd4` and begin at
`0x0068acd8`. This leaves a four-byte interval, but does not establish the
object's size, padding or construction identity.

The nearby CRT entry at `0x00566570` points to a five-byte jump at `0x0052f080`
and a one-byte return at `0x0052f090`. It supplies no receiver address, store or
registration that independently identifies the object at `0x0068acd4`.
The image has no COFF symbols or relevant relocation/debug directory to supply
the missing size and ownership evidence. An empty constructor or fieldless
method declarations therefore do not justify a one-byte singleton, four-byte
substitute, or null pointer definition. The canonical pointer remains an
undefined external in every affected object.

## Retained verification

`tests/test_clipper_global_connections.py` compiles the actual sources with
authentic VC6 SP3 and checks:

- 345 complete strict calibration/physics comparisons.
- Fifteen complete canonical and sample objects: 1,035 nondebug sections and
  4,712 relocations, including Terrain's 85 sections and 517 relocations.
- All section flags, complete payloads, relocation types/addends and defined
  target section/offset identities. External identities change only under the
  six enumerated pointer-name substitutions.
- Every applicable compiled pointer load, all 46 retail loads and direct call
  destinations, the initialized retail pointer, and the still-undefined
  canonical import.

The object pins were derived from the published pre-change objects and checked
against independently compiled final objects. Only recognized complete source
filenames are made portable between checkout paths. The previous visibility
regression's seven pins were likewise checked against their old values before
applying these same six names; their bytes, extents and relocation destinations
did not change. The existing TrackOverlay sample's genuine include repair was
already part of the published baseline, rather than a new prerequisite here.

Terrain's older named-relocation fingerprint also needed a narrow correction:
the Load EH label changes from `$L6170` to `$L6172`, still targeting section 29
`.text$x+182` with zero addend. The published game object, final game object and
fresh test-profile object preserve all 85 sections and 517 relocation
destinations, including the complete handler. Only that named digest and its
explanation change; QueryGround's fingerprint and the fingerprint format remain
unchanged.

The previous method-boundary regression also passes: 214 strict comparisons,
seven complete objects and 66,845 native cases (including 432 access faults).
Those finite method tests execute the actual clipper helpers, including
`CullPolygon`; their prepared-storage and masked-FP limits remain documented in
[VISIBILITY_CONNECTIONS.md](VISIBILITY_CONNECTIONS.md). This pointer declaration
slice adds no new native simulation, global constructor or integrated renderer
claim.

The isolated real source build compiles all 251 translation units and reduces
the unresolved-name count from **256 to 250**, removing exactly the six obsolete
pointer names with no additions. The subsequent rectangle index storage slice
is separate. Linking and running the game, including supplying the real clipper
storage and remaining methods, are still unfinished.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_clipper_global_connections test_visibility_connections -v
```
