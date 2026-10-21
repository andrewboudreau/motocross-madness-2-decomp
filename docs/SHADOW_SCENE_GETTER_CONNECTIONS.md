# Shadow scene getter connections

`D3DIMSoultreeShadow` now queries its caster through the existing canonical
scene-node getters. The production visibility method and the separate
nonadmitted sample each contain one call to each getter. The changes replace
two unresolved provisional method names without adding bodies or wrappers.

| Existing `SoultreeObject` provider | Retail VA | Full VC6 extent |
| --- | --- | --- |
| `GetMatrixIn(SoultreeObject*, Matrix4*)` | `0x004fca80` | 494 bytes |
| `GetSubtreeBounds(Vec3*, Vec3*)` | `0x004fe850` | 74 bytes |

Both complete providers and the production shadow slot 27 at
`0x004468f0` / 767 bytes match retail strictly with relocations resolved.
Their source ownership remains the existing `soultree.cpp`; the shadow methods
remain in the existing `D3DIMSoultreeShadow.cpp` reconstruction. Semantic method
names are inferred from their behavior, not recovered original declarations.

## Pointer and argument evidence

The garage loads a character's model pointer from `+0x1a0` at `0x00499504`,
passes that unchanged pointer to `D3DIMSoultreeShadow::Attach` at `0x00499511`,
and Attach stores it to the receiver's caster field `+0x2c` at `0x004468db`.
The model producer and RTTI evidence described in
[GARAGE_SETTER_CONNECTIONS.md](GARAGE_SETTER_CONNECTIONS.md) identify this as
the primary `D3DIMSoultreeObject` address. Its `SoultreeObject` base is at zero;
its distinct `GameObject` base is at `+0x0c`. These getter calls must retain the
primary address, with no secondary-base adjustment.

Production calls at `0x0044691f` and `0x00446937` pass a null reference frame,
a writable 64-byte matrix buffer, and two writable twelve-byte xyz buffers.
The matrix is an explicit output argument, not a hidden aggregate return.
Native checks compile the actual local matrix/caster declarations extracted
from the production caller and verify matrix size, translation/final-component
offsets, and the caster's `+0x14c` flag. Vector layout and the receiver's
`+0x2c` caster offset are also checked. The casts connect these specific
boundaries; they do not assert complete equivalence of every provisional view.

The source adds only narrow `Vec3`, `Matrix4` and scene-node declarations.
It retains the local matrix product and its operand order, private constants,
other camera/clipper declarations, and the existing caster flag access.

## Preservation and native checks

The private before/after comparison covers both complete objects:
`src/krusty2/shadow/D3DIMSoultreeShadow.cpp` and the independently written
`samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp`. All 54 nondebug
sections and 183 relocation records retain their raw bytes and destination
section/offsets under only the two documented external name changes.

The retained regression checks 83 full strict functions, including all
registered source shadow and scene-node methods. Both nonadmitted sample
fingerprints remain identical before/after relocation resolution:

| Sample | Retail extent | Candidate extent | Matching positions |
| --- | --- | --- | --- |
| Slot 29, `0x00446c30` | 771 | 771 | 730 |
| Slot 30, `0x00446f40` | 1,524 | 1,485 | 180 |

Neither sample is promoted or repaired. In particular, slot 30's conversion
behavior and slot 29's matrix scheduling remain separate open work.

**6,144 native cases pass.** The fixture executes unchanged extracted production
getters and their real scene/math dependencies: `UpdateWorldMatrix`,
`LocalToWorldDirection`, `LocalToWorldPoint`, `UpdateSubtreeBounds`,
`AccumulateBounds`, the out-of-line vector constructor and
`CollisionMatrixMultiply`. Each of these nine bodies is independently strict
in its complete production object and its extracted test object (18 checks).
No helper is scripted and no retail machine code is embedded in the fixture.

Tests cover the null-frame matrix path used by the shadow, valid and invalid
world caches, parent/child/grandchild traversal, empty and populated subtree
bounds, cached versus refreshed bounds, matrix output aliasing its source,
and shared center/extent output storage. Independent matrix multiplication
and corner-based bounding-box expectations check the results for bounded
integer translations and signed diagonal rotations. All node and output bytes,
including guards, are compared. The fixture preserves the provider's observed
behavior of leaving `subtreeDirty` set after refresh. It does not claim arbitrary
floating-point coverage or execution of GetMatrixIn's nonnull-frame path.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_shadow_getter_connections -v
```

Private evidence remains under `work/game-build/shadow-getters33`; it is not a
committed game asset or substitute implementation. Shadow initialization,
visibility clipper `TestBox`, and the unfinished shadow update methods remain
unresolved or nonadmitted. These two connections do not establish complete
shadow rendering, menus or racing.

The isolated real link in `work/game-build/build-_0ekrczz` compiles 226
translation units and removes exactly these two missing names: 341 to 339,
with no new unresolved symbols. The game still does not link.
