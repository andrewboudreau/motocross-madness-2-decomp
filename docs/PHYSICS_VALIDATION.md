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

## Preserved candidates

The tree builder, D3DIMSoultree motion control, steering, projected shadow,
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
