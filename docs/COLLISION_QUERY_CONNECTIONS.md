# Collision query connections

CollisionObject now calls eleven existing source providers through their canonical
declarations. The declarations live in
`src/krusty2/bvh/BoundingBoxTreeQueryFunctions.h`; it imports no query implementation,
inline arithmetic, vector constants or alternative class definition. The query
provider header shares those declarations. Collision payloads retain their
existing provisional types and layouts, with explicit casts at the call boundary.

| Provider | Retail VA | Complete bytes | Production call sites |
| --- | --- | ---: | ---: |
| TreeTreeQuery | `0x00428950` | 708 | 5 |
| TreeBoxOverlap | `0x00428c20` | 395 | 4 |
| SphereTreeQuery | `0x00429570` | 789 | 2 |
| SphereTreeQueryWithVertices | `0x00429540` | 48 | 2 |
| CapsuleTreeQueryWithVertices | `0x00429890` | 48 | 2 |
| SoultreeObject::GetPositionIn | `0x004fc9a0` | 134 | 3 |
| SweptBoxOverlap | `0x00424730` | 891 | 6 |
| BoxSphereOverlap | `0x00425750` | 419 | 2 |
| GrowBoxByTransformedBox | `0x004290d0` | 805 | 2 |
| Vec3TransformNormal | `0x0042a450` | 91 | 5 |
| Vec3TransformPoint | `0x0042a510` | 100 | 3 |

All eleven providers already exist in `BoundingBoxTreeQuery.cpp` or
`soultree.cpp` and match their complete retail extents with resolved relocations.
The 36 emitted calls belong to 17 CollisionObject methods. The extra capsule
query in `samples/physics/collision/CollisionObjectNearMisses.cpp` also uses the
canonical declaration at its two call sites; its complete 532-byte body remains
strict. That aggregate uses the canonical CollisionObject bindings.

## Boundary evidence

The query's interior node and collision bounds view both place the center at
`+4`, half extents at `+0x10`, and children at `+0x1c/+0x20`, with size `0x24`.
Point and segment leaves are shorter (`0x10` and `0x1c`); the query discriminates
them using the marker at zero. This is a boundary/layout observation, not proof
that every provisional payload represents one original C++ class.
`TreeBoxOverlap` reads only center/half extents from its two node arguments.
Consequently, the existing stack bounds whose marker/children are uninitialized
remain valid for that specific call.

`TreeTreeQuery` receives two tree pointers, two 64-byte input matrices, a mode,
a vertex-array pointer and an optional **input motion matrix**. Its last argument
was previously named `out`; the provider copies it and selects motion traversal.
The collision callers retain the exact addresses: hull `+8`, relative frame
`+0x148`, or null. Vertex pointers retain their original addresses and mutability.
Capsule calls still pass two consecutive 12-byte endpoints. No arrays become
pointer storage, and no temporary geometry is substituted.

Vectors passed by value remain three stack DWORDs. Reference casts expose those
same bits to the existing by-value parameters; they do not change those
parameters into references or perform floating-point conversion. This also
preserves negated temporaries and the in-place normal/point transforms.
`GrowBoxByTransformedBox` reads the existing center and half extents before
updating their union with the transformed box. The connection retains this
input/output behavior, including alias relationships.

The three node-position calls retain the primary scene pointer, null frame and
12-byte destination. Retail primary D3DIMSoultreeObject RTTI places SoultreeObject
at zero and its separate GameObject base at `+0xc`; no GameObject adjustment is
introduced. Direct caller bytes and the complete `ret 8` provider establish the
specific call boundary. Whole provisional-class identity is not claimed.

## Preservation and native checks

Both the six pointer-only connections and the subsequent five by-value
connections were compiled privately against `799f6e9d` before production changes.
Across CollisionObject, the query provider, CollisionCharacter and the two BVH
sample aggregates, all **315 nondebug sections and 2,116 relocation destinations**
remain identical under the eleven named external substitutions. The additional
collision sample aggregate preserves all **136 sections and 732 relocations**.
Comparisons use defined destination section/offset identity for local symbols;
there are no instruction, extent, filename or other data exceptions.

The isolated authentic source build compiled **214/214 TUs**, reducing unresolved
names **404 → 393**: exactly those eleven imports disappeared and none appeared.
All **214 pre-existing production objects, 10,460 nondebug sections and 45,294
relocations** are preserved under the same substitutions.
This connects supplied implementations; it does not complete the collision
subsystem or the game link.

`tests/test_collision_query_connections.py` checks **193 strict cases**, **31
complete partial fingerprints**, all eleven providers and the 36 retail direct
call boundaries. The sample aggregate's included production bodies are checked
as separate emitted functions. Partial fingerprints come from the published
baseline; no partial result is promoted to exactness. Registered target windows
can contain jump-table data after code and are not asserted to be new CFG proofs.

The native fixture checks both actual header layouts and executes four unchanged
production helper bodies extracted into a temporary test TU. Both the original
production object and the extracted object must match each complete retail body
before execution. **10,535 cases** cover overlapping inputs/outputs, by-value
snapshots, negated temporaries, in-place transforms, signed zero, subnormal and
nonfinite input bits, and existing bounds that participate in a union. Independent
finite geometry checks distinguish points from directions, compare unions against
eight transformed corners, and verify the sphere test's exclusion of tangency.
No helper implementation is replaced by a mock.

The native checks cover these helpers and call boundaries, not recursive tree
traversal, complete collision behavior or gameplay. Other dependencies, including
`TreeTreeQueryNodes`, `BoxCapsuleOverlap` and provisional shared-global views,
remain unresolved. Query TU attribution remains the documented inference in
`BoundingBoxTreeQuery.cpp`.

```powershell
$env:PYTHONPATH = '.;tests'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_collision_query_connections
```
