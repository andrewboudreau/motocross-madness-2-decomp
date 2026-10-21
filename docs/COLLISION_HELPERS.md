# Collision query and vector providers

`src/reconstructed/CollisionQueries.cpp` and
`src/reconstructed/CollisionVectorHelpers.cpp` supply four complete functions
previously kept in samples. All four remain strict exact under VC6 SP3
`/O2 /GR /GX /MT /Z7`, with complete compiler-reported extents and every
relocation resolved. Function bodies, calling conventions and caller
declarations were preserved.

| Function | Retail VA | Bytes | Verified direct call sites |
|---|---:|---:|---|
| `SphereContainsPointXZ` | `0x0043a1e0` | 134 | QueryCollisions `0x00439055` |
| `BoundingSpheresOverlap` | `0x0043a270` | 183 | QueryCollisions `0x00438f45` |
| `CollisionDivide` | `0x0043c890` | 71 | HullVsModel `0x00437581`, `0x004376b2`; UpdateRelativeMotion `0x0043aa1e` |
| `CollisionRotateVector` | `0x0043ca20` | 115 | ApplyContactImpulse `0x0043c524`, `0x0043c75b` |

The query functions read the center at `+0x40` and radius at `+0x4c` of the
caller's object. Their header declares a provisional bounds view, preserving
the existing caller signatures. The point query uses only x and z; the sphere
query uses all three components. Both reject per-axis separation first and
then compare squared distance against the squared sum of radii, including
equality as an overlap. Conditional negation, floating-point comparisons and
operand order remain as decoded; no numerical edge-case rewrite was made.
The vector helpers preserve destination-pointer returns, reciprocal
multiplication for division, and row-vector rotation without translation.

Three exact-name unresolved symbols are supplied to the current production
callers. The rotation helper's caller remains a partial constraint sample,
so that fourth exact body is not counted as a new production connection.
Both compiled provider objects have only the VC6 `__fltused` marker undefined;
they introduce no missing game-code dependency.

Original TU identity remains separate from these body and caller proofs. No
literal assigns the sphere queries to either neighboring CollisionObject.cpp
or CollisionPoint.cpp. The divide and rotation bodies flank the vector
initializer set adjacent to ConstraintMethodCollisionModel.cpp, and its
ApplyContactImpulse calls them; header-inline COMDAT emission with that unit
is a strong inference. The reconstruction filenames and semantic names are
ours. Each new unit also emits the existing Math3D header's four local vector
constants and initializer entries. Their storage is private to the object;
no original retail ownership or linked initializer ordering is assigned to
these additional copies. Four exact bodies do not mean whole-unit identity.

The existing physics target manifest routes the four checks to canonical
source. `tests/test_collision_helper_providers.py` checks the verified retail
hash and seven decoded direct-call instructions, recompiles both providers
and the two real canonical caller units, checks strict bytes, verifies exact
import names, and checks dependency and initializer scope.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_collision_helper_providers.py -v
```
