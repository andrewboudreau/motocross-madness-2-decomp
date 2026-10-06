# src/krusty2: promoted translation units

The retail game was built from one flat folder, `D:\aardvark\VC\krusty2\`, recovered
from the `__FILE__` strings in the binary (`analysis/source_paths.txt`). This tree
holds reconstructed code with reviewed or explicitly provisional retail-file attribution. It is grouped into area
subfolders to keep things organised; the byte match doesn't depend on the folder.

- `__FILE__` strings are only reached through relocated addresses, and the matcher
  masks those in diagnostic mode. Strict mode requires a reviewed file-literal
  binding and compares the relocated address.
- Line numbers do enter the code (`push 0x7a`), so sources pass them as literals,
  e.g. `new(__FILE__, 0x7a)`. Moving a file never changes them.

Experimental reconstructions, layout probes and code with unknown or weakly
attributed files stay under `samples/physics/`.

## Promotion rule

A `.cpp` is promoted under its retail name when:

1. The name is in `analysis/source_paths.txt` (tier 1).
2. Every promoted function lies inside that file's code bracket (tier 2). The linker
   placed the `.cpp` objects in alphabetical order, so a file's code sits between the
   last `__FILE__` xref of its alphabetical predecessor and the first xref of its
   successor. EH funclets at `0x54xxxx` are excluded.
3. The function either references the file's own `__FILE__` string, or is a method
   of a class whose other methods do and sits contiguously with them.

Headers have no retail names beyond a few `.h` strings, so their names are ours
(tier 3). Shared headers live here so promoted code never includes from `samples/`.

## Layout

| Folder | Contents |
|---|---|
| `core/` | `GameObject.h`, `GraphicsTest.h`, `DebugAlloc.h` (debug malloc/`new`/`delete`/realloc), `MemTag.h` |
| `math/` | `FastMath.h` (FastSqrt / FastInvSqrt) |
| `collision/` | `CollisionObject.cpp` (+ `CollisionObject.h`, `CollisionShapeTests.h`, `CollisionPoint.h`, `CollisionTypes.h`): 44 strict cases |
| `contact/` | `ContactImpulse.h`, `ObjectPlacement.h` (shared contact layouts) |
| `soultree/` | `SoulTreePhysics.cpp` and the SoultreePhysicsBaseObject/Character headers: 37 strict cases; `SoultreeQuadTreeRenderer.cpp`: 8 strict cases |
| `gravity/` | `SelectiveGravityModel.cpp`: 15 strict cases |
| `vehicle/` | `Vehicle.cpp` (57 strict cases), `Bike.cpp` (31 strict cases) and `BikeAI.cpp` (23 strict cases), `Vehicle.h`, `Bike.h` |
| `broadphase/` | `Quadtree.cpp`/`.h`, `Terrain.cpp`/`.h` |
| `bvh/` | Shared box-tree layouts; the builder is matched in `src/reconstructed/BoundingBoxTreeBuild.cpp`, queries in `samples/physics/bvh/` |
| `effects/` | NormalDistribution, NullManager, ParticleManager: 18 strict cases |
| `motion/` | SphereManager: 14 strict cases; shared motion layouts |
| `shadow/` | D3DIMSoultreeShadow: 17 strict cases; other shadow candidates in samples |
| `visibility/` | VisibilityQuadTree: 16 strict cases; partial traversal in samples |

The earlier broad-phase counts are masked diagnostics. The new reviewed slices
and their exact reproduction commands are in [PHYSICS_VALIDATION.md](../../docs/PHYSICS_VALIDATION.md).
SelectiveGravityModel.cpp (0x4f9760..0x4f9a59) is promoted on its own. The Shock
family (0x4f9a60..0x4fb1d7) builds a second set of the Math3D.h constant vectors
(0x689e48..0x689e78), so it is a separate TU (strong inference). It stays in
`samples/physics/suspension/Shock.cpp` because its name is not attested. Motion control, steering and the box-tree
builder also remain samples pending complete relocation verification.

Shared headers use paths relative to this folder, e.g. `core/GameObject.h`.
The physics runner adds `src/krusty2` to the include path.

## Evidence: Quadtree.cpp

- Name: `D:\aardvark\VC\krusty2\Quadtree.cpp`, string at 0x00572040.
- Code bracket: after `ProjectedShadow.cpp` (last xref 0x4dacbc) and before
  `Quantize.cpp` (first xref 0x4dde47). Quadtree.cpp's own xrefs span 0x4dc729..0x4ddce6.
- All 27 QuadTree/QuadTreeNode targets (0x4dc620..0x4ddd90) are inside the bracket;
  24 match exactly, and 3 are documented partials in `broadphase/targets.json`.
  The vector `$E` set at 0x4dc4d0..0x4dc60b is ProjectedShadow.cpp's (only its
  constructor reads the zero vector; see `docs/INITIALIZERS.md`).
- The 5-byte stub at 0x4dc4c0 (`xor eax,eax; ret 8`) also sits in this stretch, but it is
  Terrain's slot 22 and is shared with the ProjectedShadow and StatsOverlay vtables.
  Identical code folding makes its address useless for attribution. It is reconstructed
  in Terrain.cpp.

## Evidence: Terrain.cpp

- Name: `D:\aardvark\VC\krusty2\Terrain.cpp`, string at 0x00574720, with debug deletes
  at lines 0x4b3 and 0x4d0.
- Code bracket: after `SteeringControl.cpp` (last xref 0x504bd2) and before `Texmap.cpp`
  (first xref 0x50a6bc). Terrain.cpp's own xrefs span 0x50567c..0x507b38.
- 13 of the 14 targets (0x5057d0..0x508850) are inside the bracket. The 14th is the shared
  0x4dc4c0 stub described above. 12 match exactly.
- The vector `$E` set 0x5089a0..0x508adc (8 targets, all exact) closes the unit: `.CRT$XCU`
  lists it just before the camera and timer initializers of `TerrainSupport.cpp`, and the
  y axis 0x68a058 is read by Terrain code (see `docs/INITIALIZERS.md`).
- Terrain derives from `GameObject` and `GroundFogableObject`, as the RTTI says (mdisp 0,
  and 0x2c for GroundFogableObject, which has no vfptr). Both bases are kept.
- `QueryGround` 0x507c10 contains inline fistp instructions not reproduced by the
  tested C++ casts. The original source mechanism is unproven.
- The helper types (`TerrainVec3`, `TerrainMatrix`, `TerrainShutdownObject`,
  `TerrainComObject`, `TerrainOwned` and others) are provisional stand-ins with tier 3 names.
  `TerrainVec3` stays separate from the shared Vec3 because including `Math3D.h` would add
  static initializers that Terrain.cpp does not have.

## Evidence: CollisionObject.cpp, Vehicle.cpp, Bike.cpp, SoulTreePhysics.cpp (wave 6)

| File | String | Own xrefs | Bracket (predecessor last xref .. successor first xref) |
|---|---|---|---|
| CollisionObject.cpp | 0x00568614 | 0x431dd8..0x43a124 (33) | CollisionCharacter.cpp 0x431a12 .. CollisionPoint.cpp 0x43a347 |
| Vehicle.cpp | 0x00575918 | 0x525e98..0x526261 (13) | VCRfile.cpp 0x5252ed .. vfwdeco.cpp 0x52d07e |
| Bike.cpp | 0x00566e4c | 0x407c3a..0x40943f (21) | BackgroundImage.cpp 0x404150 .. BikeAI.cpp 0x414847 |
| SoulTreePhysics.cpp | 0x00574320 | 0x500d6c..0x503f88 (10) | SoultreeMaterial.cpp 0x4ff835 .. SoultreeQuadTreeRenderer.cpp 0x504710 |

- CollisionObject.cpp merges seven wave-5 sample files whose functions all lie in
  0x431da0..0x439e10. The first, `CollisionHullShape_Free` 0x431da0, contains the first
  `__FILE__` xref (0x431dd8). The shape setters pass `__FILE__` with lines 0xd8..0x18a.
  The helpers outside the span (0x43b190, 0x43c890, 0x43ca20, 0x43ce90) stay in
  `samples/physics/collision/CollisionVectorHelpers.cpp`.
- Vehicle.cpp and Bike.cpp hold methods of their class that lie contiguously in the
  bracket. Bike's methods start at 0x405190, before the first xref inside the constructor
  0x407700. The Vec3 helpers 0x40ae00/0x40ae30 are not Bike methods, so they stay in
  `samples/physics/bike/BikeVec3Ops.cpp`.
- Vehicle.cpp also owns the four per-TU `Math3D.h` vector initializers
  (`_$E1`..`_$E11`, 0x5278e0..0x527a1f). Their constants (zero 0x0068a6e8, X, Y, Z) are this
  TU's `kVec3Zero`.. `kVec3ZAxis`, and both neighbouring method runs read the zero vector.
  SoulTreePhysics.cpp's zero vector 0x00689ee8 is its own `kVec3Zero` in the same way.
- Some Vehicle and SoultreePhysicsBaseObject overrides lie inside *other* TUs:
  - 0x40b410..0x40cb60 are in Bike.cpp;
  - 0x464e80 is in FollowCam;
  - 0x492220 is in KrustyBike;
  - 0x4aa150 and 0x4a6ba0 are in Motnctrl;
  - 0x507920 is in Terrain.

  The class tables point at these copies, so they are most likely inline members kept
  from the first TU that emitted each COMDAT. They stay in
  `samples/physics/vehicle/VehicleInlines.cpp` and
  `samples/physics/soultree_base/SoultreePhysicsInlines.cpp` until an inline-in-header
  reconstruction reproduces that placement.

Bindings (`*.bindings.json` next to each `.cpp`) were proposed by
`tools/propose_bindings.py` from masked-exact targets only and accepted only on
independent evidence:
- reviewed bindings elsewhere;
- this symbol's own matched target;
- RTTI tables and vptr writes;
- decoded `__real` and string bytes;
- `__FILE__` text;
- relocation-free data bytes;
- matched `$E` initializers;
- EH FuncInfo magic;
- imports;
- a retail call target named by address in the source.

A symbol that is itself a target elsewhere is a conflict, never fitted. That rule
exposed a real error: TestHullAgainst and TestModelAgainst had their capsule (3) and
sphere (4) cases in the opposite source order to retail's jump table. The masked match
hid it.

Strict failures that remain are missing evidence, not byte differences:
- the CollisionFileStream constructor 0x460d10, which has no RTTI;
- the BikeA604 constructor 0x52ff90, called from Bike slot 97;
- SoultreeRefreshContacts 0x43ad80, called from GameObjectVirtualSlot10. Its body is still a partial in samples.

These targets are `expect: "masked"`.

## Gate

```bash
python tools/run_physics_samples.py                             # samples/physics + src/krusty2
python tools/run_physics_samples.py --strict --root src/krusty2/broadphase
python tools/run_physics_samples.py --strict --root src/krusty2/collision   --root src/krusty2/vehicle --root src/krusty2/soultree
```

`tools/analyze.py` writes filename-only placeholders under ignored
`generated/krusty2-skeletons/`, never into this source tree.

The default physics run is a masked diagnostic regression check. `--strict` is
the acceptance check; it fails until every required target has independently
reviewed relocation bindings (a `bindings` path relative to its targets.json).
