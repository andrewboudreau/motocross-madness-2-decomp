# Out-of-line vector constructors

Seven existing vector call views now have ordinary C++ constructor definitions.
Each copies its three float arguments into `x`, `y` and `z`. Authentic VC6 SP3
emits the complete **25-byte** retail body at `0x00404e60` for each definition.
The views remain separate types: sharing this small body does not establish
their original C++ identity or original translation-unit ownership.

| Existing type | Definition location |
|---|---|
| `CollisionVec3` | `src/reconstructed/CollisionVectorConstructor.cpp` |
| `BikeVec3Call` | `src/krusty2/vehicle/Bike.cpp` |
| `CameraVec3Call` | `src/reconstructed/Camera.cpp` |
| `CollisionVec3Call` | `src/krusty2/collision/CollisionObject.cpp` |
| `SoultreeOolVec3` | `src/krusty2/soultree/soultree.cpp` |
| `Vec3Call` | `src/krusty2/bvh/BoundingBoxTreeQuery.cpp` |
| `VehVec3Call` | `src/krusty2/vehicle/Vehicle.cpp` |

Six definitions sit beside the callers' existing declarations. CollisionObject
and CollisionPoint share the actual `CollisionVec3` declaration in
`CollisionTypes.h`; its new provider filename and source grouping are ours.
No declaration, inheritance, header, call-site cast or compiler pragma changes
are required. Existing calls remain out of line under the calibrated profile.

## Retail and ABI evidence

The retail body comprises seven integer `mov` instructions and `ret 0xc`.
It receives the object in ECX, copies three stack argument words to offsets
zero, four and eight, and returns the original receiver in EAX. There is no
floating-point arithmetic or conversion. Seven NOPs separate its end at
`0x00404e79` from the next entry at `0x00404e80`.

Native layout checks confirm that all seven existing declarations occupy
12 bytes, with the same three field offsets. Actual production COFF function
auxiliary records independently report the complete 25-byte extent; every
provider has no body relocations. Caller relocation checks retain the expected
constructor imports/calls. The new shared object defines only its constructor
and has only the compiler's `__fltused` undefined marker, with no new game
dependency. These facts establish each narrow value-construction boundary,
without asserting equivalence of surrounding game classes.

## Validation

`tests/test_vector_constructors.py` pins the private retail image, checks the
complete decoded extent and padding, compiles the actual sources with VC6 SP3,
and verifies **356 strict comparisons**. It also preserves **27 complete
resolved partial-body fingerprints**, including candidate sizes and matching
positions; these partial functions are not claimed as retail matches.

The native fixture runs **72,240 cases** across all seven types. It checks the
entire backing storage and placement result for independent inputs, exact
overlap and shifted overlap. Inputs include deterministic arbitrary bit
patterns, signed zeros, infinities, normal/subnormal boundaries and NaN payloads.
Expected values are copied as words, with no floating-point arithmetic.

The shared CollisionVec3 production object executes directly. The six other
caller translation units still depend on unfinished game functions, so the
test extracts their unchanged declarations and constructor definitions into a
temporary execution unit. Both the actual production objects and the extracted
execution object must independently match every retail byte and full extent
before execution. This does not substitute extracted test code in the game.

The private baseline comparison in
`work/game-build/vector-constructors-round18/` preserves all 376 pre-existing
comparisons: 348 strict and 28 partial. The isolated real build
`build-3bhn75px` compiles **203/203 translation units** and changes unresolved
symbols from **500 to 493**, removing exactly these seven constructors with
none added. The game still does not link; this slice establishes these value
providers and caller preservation, not startup or racing readiness.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python tests/test_vector_constructors.py -v
```
