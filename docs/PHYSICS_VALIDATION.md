# Physics reconstruction and verification

The physics sources include verified implementations and unfinished candidates.
The same readable C++ is retained while those two claims stay separate.

## Reviewed wave-4 slice

Authentic VC6 SP3, profile `vc6_o2_ml`, reproduces all bytes for 48 cases
(47 distinct retail address/extent pairs) in:

- `src/krusty2/visibility/VisibilityQuadTree.cpp`: 16 cases.
- `src/krusty2/effects/NormalDistribution.cpp`: 6 cases.
- `src/krusty2/effects/Nulls.cpp`: 6 cases.
- `src/krusty2/effects/Particles.cpp`: 6 cases.
- `src/krusty2/motion/Spheres.cpp`: 14 cases.

Every relocation is resolved by the adjacent binding files; none is masked.
NullManager and SphereManager both reproduce the shared 19-byte slot-8 body,
so their two cases count as one retail function. This does not establish their
original source spelling or exclusive ownership of a shared address.

The review corrected the persistent collision-tree pointer versus the active
traversal pointer, NormalDistribution's malloc-style debug allocation call,
and the two missing slot-8 overrides. Particle update flags now have provisional
names describing the decoded operations. Readability changes preserve codegen.
See the effects, motion and visibility READMEs for binding evidence.

Reproduce only the reviewed slice after private-input setup:

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/visibility/VisibilityQuadTree.cpp \
  --source src/krusty2/effects/NormalDistribution.cpp \
  --source src/krusty2/effects/Nulls.cpp \
  --source src/krusty2/effects/Particles.cpp \
  --source src/krusty2/motion/Spheres.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

## Reviewed wave-5 slice

A further 41 cases pass strict VC6 SP3 comparison under the same profile:

- `src/krusty2/shadow/D3DIMSoultreeShadow.cpp`: 16 cases, including the
  generated destructor core selected by its deleting wrapper.
- `src/reconstructed/TerrainSupport.cpp`: 25 cases covering global camera/timer
  initialization, owned-texture acquisition, and the height-range getter.

These two sources have adjacent reviewed bindings. TerrainSupport is our slice
filename, not a recovered retail TU name; the source attribution is Terrain.cpp.
Its timers reuse the canonical UnknownPeakHold type, and the texture constructor
arguments use TextureMapManager pointers. The remaining Terrain implementation
is unchanged in scope and is not included in this strict claim.

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/shadow/D3DIMSoultreeShadow.cpp \
  --source src/reconstructed/TerrainSupport.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

Across the two reviewed slices there are 89 cases, or 88 unique retail
address/extent pairs. Full-corpus totals also include older targets and are
not additive to the calibration progress snapshot.

## Wave-6 promoted slice

Four retail TUs moved from `samples/physics/` into `src/krusty2/`. The ownership and
binding evidence is in `src/krusty2/README.md`. 169 cases pass strict VC6 SP3
comparison:

- `src/krusty2/collision/CollisionObject.cpp`: 44 of 61 targets.
- `src/krusty2/vehicle/Vehicle.cpp`: 57 of 77 targets.
- `src/krusty2/vehicle/Bike.cpp`: 31 of 45 targets.
- `src/krusty2/soultree/SoulTreePhysics.cpp`: 37 of 43 targets.

Three targets are `expect: "masked"` because one called constructor or helper has no
independent identity yet. The rest are documented `partial` code-generation
mismatches. `tools/propose_bindings.py` drafted the bindings. It refuses to bind a
symbol away from its own target address, and that refusal found a case-order error
that the masked check had accepted (TestHullAgainst/TestModelAgainst).

```bash
python tools/run_physics_samples.py --strict   --root src/krusty2/collision --root src/krusty2/vehicle --root src/krusty2/soultree   --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

## Preserved candidates

The tree builder, D3DIMSoultree motion control, Motnctrl loaders/playback, steering, projected shadow,
and the visibility traversal remain under `samples/physics/` until their
remaining byte/relocation evidence is complete. Shared layout headers remain
under `src/krusty2/` for both verified source and samples; a header alone does
not claim a reconstructed implementation. Retail-file attribution is retained
where supported, without promoting that evidence into exact-code status.

`expect: "masked"` requests a diagnostic regression check only. It does not
count as strict validation. `expect: "partial"` retains known code-generation
mismatches. The full `--strict` audit continues to fail on required targets
with unresolved relocations, including pre-existing physics candidates:

```bash
python tools/run_physics_samples.py --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
python tools/run_physics_samples.py --strict --vc6-root "$VC6_ROOT" \
  --exe "$MCM2_EXE" --json-out work/physics-strict.json
```

Missing bindings, compilation errors, and completed byte mismatches are distinct
statuses in the report. Never infer incorrect C++ merely from an unresolved
symbol, or claim a strict match from a masked result. No target length trimming
or relocation-byte fitting is used to make a candidate pass.

`make progress` regenerates the public source inventory and the separately
reviewed calibration snapshot. It does not compile or count this physics suite.
A complete linked game remains unverified.
