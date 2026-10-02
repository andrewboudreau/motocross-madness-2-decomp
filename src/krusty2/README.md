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
| `collision/` | `CollisionObject.h`, `CollisionTypes.h` |
| `broadphase/` | `Quadtree.cpp`/`.h`, `Terrain.cpp`/`.h` |
| `bvh/` | `BoundingBoxTreeBuild.cpp`/`.h`: collision box-tree build (21 of 23 exact) |
| `effects/` | `Particles.cpp`, `NormalDistribution.cpp`, `Nulls.cpp` (17 of 17 exact) |
| `motion/` | `D3DIMSoultreeMotnctrl.cpp`, `Spheres.cpp`, `SteeringControl.cpp` (49 of 53 exact) |
| `shadow/` | `ProjectedShadow.cpp`/`.h` (11 of 18 exact) |
| `visibility/` | `VisibilityQuadTree.cpp`/`.h` (16 of 17 exact) |

The table's exact counts are relocation-masked diagnostics, not strict byte proof.
The combined SelectiveGravityModel/Shock candidate remains in
`samples/physics/suspension/`: Shock TU ownership has only proximity evidence.

Each wave 4 folder has a `README.md` with its `__FILE__` string, bracket and ownership evidence.
Code that sits in a bracket without `__FILE__` or class evidence lives in the matching
`samples/physics/<folder>/` instead.

Include shared headers by their path under this folder, e.g. `#include "core/GameObject.h"`.
`tools/run_physics_samples.py` puts `src/krusty2` on the include path.

## Evidence: Quadtree.cpp

- Name: `D:\aardvark\VC\krusty2\Quadtree.cpp`, string at 0x00572040.
- Code bracket: after `ProjectedShadow.cpp` (last xref 0x4dacbc) and before
  `Quantize.cpp` (first xref 0x4dde47). Quadtree.cpp's own xrefs span 0x4dc729..0x4ddce6.
- All 35 QuadTree/QuadTreeNode targets (0x4dc4d0..0x4ddd90) are inside the bracket;
  32 match exactly, and 3 are documented partials in `broadphase/targets.json`.
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
- Terrain derives from `GameObject` and `GroundFogableObject`, as the RTTI says (mdisp 0,
  and 0x2c for GroundFogableObject, which has no vfptr). Both bases are kept.
- `QueryGround` 0x507c10 contains inline fistp instructions not reproduced by the
  tested C++ casts. The original source mechanism is unproven.
- The helper types (`TerrainVec3`, `TerrainMatrix`, `TerrainShutdownObject`,
  `TerrainComObject`, `TerrainOwned` and others) are provisional stand-ins with tier 3 names.
  `TerrainVec3` stays separate from the shared Vec3 because including `Math3D.h` would add
  static initializers that Terrain.cpp does not have.

## Gate

```bash
python tools/run_physics_samples.py                             # samples/physics + src/krusty2
python tools/run_physics_samples.py --strict --root src/krusty2/broadphase
```

`tools/analyze.py` writes filename-only placeholders under ignored
`generated/krusty2-skeletons/`, never into this source tree.

The default physics run is a masked diagnostic regression check. `--strict` is
the acceptance check; it fails until every required target has independently
reviewed relocation bindings (a `bindings` path relative to its targets.json).
