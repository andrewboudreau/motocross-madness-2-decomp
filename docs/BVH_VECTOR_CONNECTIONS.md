# BVH vector provider connections

`BoundingBoxTreeQuery.cpp` connects twelve calls in the complete strict
`CapsuleTreeQuery` body (0x004298c0, 1,436 bytes) to four existing source
providers. The original translation-unit identity and semantic names remain
provisional; this change establishes the decoded call boundaries.

| Previous call view | Canonical provider | Retail body | Sites |
| --- | --- | --- | --- |
| `Vec3Add(const Vec3&, const Vec3&)` | `Vec3AddCall(Vec3*, const Vec3*, const Vec3*)` | 0x00421cb0 / 69 bytes | 2 |
| `Vec3Sub(const Vec3&, const Vec3&)` | `Vec3SubtractCall(Vec3*, const Vec3*, const Vec3*)` | 0x00421d00 / 69 bytes | 8 |
| `Vec3Scale(const Vec3&, float)` | `Vec3ScaleCall(Vec3*, const Vec3*, float)` | 0x005015b0 / 69 bytes | 1 |
| `Vec3Length(const Vec3&)` | `CollisionLength(const CollisionVec3*)` | 0x00435ec0 / 74 bytes | 1 |

The three 12-byte returned vectors use VC6's hidden first output pointer.
The existing explicit-output providers take that same pointer, the same ordered
source pointers or scalar, return the output pointer in EAX and leave stack
cleanup to the caller. Named result storage is scoped to the original call
expressions. The caller retains the original copies, temporary addresses,
argument order and lifetimes in its emitted code. There are no forwarding
wrappers or linker aliases.

The length call passes the same xyz address to the existing provider. Both
views have three floats at offsets 0, 4 and 8 and occupy 12 bytes; the native
layout probe checks those offsets. This is a bounded vector-storage contract,
not evidence that every provisional vector class has the same original C++
identity. `CollisionPoint.h` supplies the actual declaration. The ordinary
header include and source expressions preserve complete emitted objects.

## Complete object and caller evidence

Against published baseline `291e16b5` and its actual
`build-3jal2tv5` object, the newly compiled canonical query object retains all
**53 nondebug sections and 491 relocations** under precisely the four names
above. The separate `BoundingBoxTreeQueryNearMisses.cpp` aggregate retains all
**38 nondebug sections and 465 relocations**, with no name substitutions.
It includes the unchanged query header, rather than the canonical cpp, and
still uses its own provisional calls. Its existing bindings remain intact.

The comparison retains every section payload, writable/code flags, extent,
relocation type, location, raw addend and destination. Defined destinations
are identified by their public section identity or section ordinal and offset,
so disposable compiler-local label spelling is not mistaken for a change in
address. No instruction or data bytes are ignored. The retained fingerprints
in `tests/fixtures/bvh_vector_objects.json` were derived from the pre-change
objects and independently matched the canonical-path candidate objects.
They preserve all aggregate partial functions, not merely the strict caller.

The focused regression verifies **54 complete strict bodies**, including
CapsuleTreeQuery and the existing vector, length and FastMath providers. It also
checks all twelve new caller relocations against decoded direct retail calls.
The isolated real game build `build-xy53_xk7` compiled all 235 translation units
and removed exactly four unresolved names, **304 to 300**, with none added.
This records source-link progress, not a completed game executable.

## Native point-leaf integration

`tests/test_bvh_vector_connections.py` extracts the unchanged complete
CapsuleTreeQuery definition and its Vec3Call constructor into a temporary
translation unit. Both extracted bodies and the complete production bodies
are checked against retail. CollisionLength is likewise extracted unchanged
and checked independently against the production object and retail. No source
function is replaced by a simplified implementation.

The executable links those bodies to the actual production VectorCalls,
VectorScaleLeft and FastMath objects. It runs the real strict square-root-table
initializer. VectorScaleLeft is a separately reconstructed strict provider;
its scalar-first parameter order is not interchangeable with Vec3ScaleCall's
vector-first order. See [its independent evidence](VECTOR_SCALE_LEFT.md).

The private hash-pinned retail image is used only as a reference. Decoded
vector and length call sites in the reference CapsuleTreeQuery are redirected
to the same proven source dependencies. Each patched site must have both the
expected CALL opcode and its original target. The reference caller's complete
control flow remains retail code; the candidate code and objects are unchanged.
The reference's 25-byte vector constructor runs directly from the mapped retail
image and is independently matched by the source constructor.

The **36,866** native cases comprise:

- 36,864 point-leaf cases: 64 finite input seeds, eight segment/point/transform
  arrangements, six radii, four rounding modes and three x87 precision modes.
  They include coincident segment endpoints, points at both endpoints,
  translation, rotation, nonuniform scale and a fixed one-unit distance.
- Two independent expected results at that fixed distance: radius 1 is a miss,
  while radius 1.25 is a hit, preserving the strict less-than comparison.

Each comparison checks the result, masked floating-point status and control,
and every byte of the segment, point leaf, matrix and surrounding guards.
Both paths must leave all input storage unchanged. Degenerate segments may
produce masked floating-point exceptions; unmasked exceptions, inaccessible
input storage and resumed exception contexts are outside this finite test.

Only point leaves are executed. Triangle/interior-only dependencies terminate
immediately if reached; they do not provide simulated successful geometry.
The complete triangle and recursion paths remain protected by full strict
caller matching and complete object preservation, but this fixture does not
claim native triangle queries, recursive tree traversal, complete BVH helper
closure or integrated racing. Existing
`tests/test_vector_calls.py` independently covers 31,873 actual-provider alias,
return-pointer and arithmetic scenarios and was rerun successfully.

Reproduce with private VC6 and retail inputs:

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_bvh_vector_connections.py -v
python -m unittest discover -s tests -p test_vector_calls.py -v
```

## Deferred connections

The two SegmentAdd/SegmentSubtract views in CollisionObject are unchanged.
Private explicit-output trials retained the 1,493-byte SegmentTouchesObject
extent but moved temporary stack slots and reduced its existing partial
matching positions from 1,309 to 1,281. No candidate or fingerprint change from
that experiment is admitted here. The separate aggregate's other provisional
vector calls also remain unresolved until their own source/ABI preservation is
established.
