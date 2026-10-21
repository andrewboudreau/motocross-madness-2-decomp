# Collision transform providers

`src/reconstructed/CollisionTransforms.cpp` supplies two complete readable
C++ bodies, grouped by their use in collision queries. This filename does
not claim original translation-unit identity.

| Provider | Retail extent | Direct retail callers |
|---|---|---|
| `MoveBox` | `0x00428db0`, 789 bytes | `0x00426b92`, `0x004335cb`, `0x00435b9c`, `0x00435d68` |
| `InvertRigidTransform` | `0x0042e2b0`, 144 bytes | `0x004399be`, `0x00439a47` |

Both match the full compiler-reported extent under authentic VC6 SP3
`/O2 /GR /GX /MT /Z7`. MoveBox has thirteen resolved literal relocations
(zero at `0x00550484`, half at `0x005507f4`); the inverse has none. Both
production caller units import the shared MoveBox symbol. CollisionObject's
former `CollisionInvertRigidCall` now imports `InvertRigidTransform(Matrix4*)`.
The calling convention, pointer layout and caller instructions are preserved.
These supply two previously missing linker names, with no new game dependency:
the provider object's only undefined symbol is the CRT `__fltused` marker.

MoveBox expands a center/half-extents box to contain both its original and
transformed bounds. All matrix reads and bound calculations precede the final
output writes. It first adds the computed midpoint to the center, then writes
the grown half-extents. Thus identical center/half pointers retain the final
half-extents store; that existing behavior is preserved, not generalized into
an arbitrary overlap guarantee.

The inverse transposes the upper-left 3-by-3 block in place, computes all three
negative rotated translation components before storing any, and leaves the
fourth column untouched. It assumes a rigid transform; it is not a general
matrix inverse. The builder retains its original TreeMatrix4 inline helper.
The promoted entry uses the already-shared Matrix4 layout through the identical
query-header inline; changing the pointer type introduces no body difference.

The inverse sits between ReadPointNode and Camera.cpp, without its own literal
source reference. Similarity to the builder inline is supporting context, not
proof of original TU ownership. MoveBox's name and the query-unit attribution
also remain inferred. Neither promotion relies on a shared tiny-address alias.

The Math3D header naturally emits four private vector constants and four
initializer entries in this grouping. Their storage remains local to the
object; no retail initializer addresses or original TU identity are claimed.
The test verifies local storage and closure as well as function bytes.

`tests/test_collision_transform_providers.py` verifies the pinned private image,
all six direct calls, both provider extents, the affected strict caller targets,
and actual COFF imports. On native Windows it also links the real providers
into a source-only executable and checks 229 finite geometry/alias cases:
affine box unions against an independent eight-corner oracle, identical output
pointers, matrix translation overlapping the output center, rigid inverse
composition/double inversion, and preservation of the fourth column on a
non-rigid diagnostic input. No retail code is loaded by that executable.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_collision_transform_providers.py -v
python tools/run_physics_samples.py --compiler vc6 --vc6-root work/vc6sp3-disc --profile vc6_o2_mt --source src/reconstructed/CollisionTransforms.cpp --source src/krusty2/collision/CollisionObject.cpp --source src/krusty2/bvh/BoundingBoxTreeQuery.cpp --strict
```

These proofs supply collision helpers; they do not establish a linked game or
working racing behavior. Remaining near-miss query bodies stay in samples.
