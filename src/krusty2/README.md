# src/krusty2: promoted translation units

The retail game was built from one flat folder, `D:\aardvark\VC\krusty2\`, recovered
from the `__FILE__` strings in the binary (`analysis/source_paths.txt`). This tree
holds reconstructed code whose **retail file is known**. It is grouped into area
subfolders to keep things organised; the byte match doesn't depend on the folder.

- `__FILE__` strings are only reached through relocated addresses, and the matcher
  masks those, so the path text never enters the comparison.
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
| `core/` | `GameObject.h`, `GraphicsTest.h`, `DebugAlloc.h` (debug `new`/`delete`/realloc), `MemTag.h` |
| `math/` | `FastMath.h` (FastSqrt / FastInvSqrt) |
| `collision/` | `CollisionObject.h`, `CollisionTypes.h` |
| `broadphase/` | `Quadtree.cpp` and `Quadtree.h` |

Include shared headers by their path under this folder, e.g. `#include "core/GameObject.h"`.
`tools/run_physics_samples.py` puts `src/krusty2` on the include path.

## Evidence: Quadtree.cpp

- Name: `D:\aardvark\VC\krusty2\Quadtree.cpp`, string at 0x00572040.
- Code bracket: after `ProjectedShadow.cpp` (last xref 0x4dacbc) and before
  `Quantize.cpp` (first xref 0x4dde47). Quadtree.cpp's own xrefs span 0x4dc729..0x4ddce6.
- All 27 QuadTree/QuadTreeNode targets (0x4dc620..0x4ddd90) are inside the bracket;
  22 match exactly, and 5 are documented partials in `broadphase/targets.json`.
- The 5-byte Terrain stub at 0x4dc4c0 sits in the same range but is a Terrain virtual
  (likely a folded COMDAT), so it stays with Terrain in samples.

## Gate

```bash
python tools/run_physics_samples.py                             # samples/physics + src/krusty2
python tools/run_physics_samples.py --root src/krusty2/broadphase
```

`tools/analyze.py` writes a flat placeholder `src/krusty2/<RetailName>.cpp` for each
known name, and skips any name already present somewhere in this tree.
