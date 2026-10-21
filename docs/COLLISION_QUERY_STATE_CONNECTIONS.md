# Collision query state and scene matrices

Three provisional CollisionObject imports now connect to existing source
definitions. No provider algorithm or storage allocation changes.

| Previous boundary | Existing provider | Evidence |
| --- | --- | --- |
| `CollisionSceneNode::GetMatrixIn` | `SoultreeObject::GetMatrixIn` | `0x4fca80`, complete 494 bytes; four direct calls |
| `SegmentTreeQueryWithVertices` with `void*` node | Same function with `QueryTreeNode*` | `0x429e60`, complete 33 bytes; two direct calls |
| `g_CollisionBoxResult` as `CollisionBoxResult*` | Existing `QueryHit*` global | Writable pointer at `0x579058`; setter and query producer agree on record fields |

The scene calls retain the primary receiver, null frame and 64-byte destination.
The hull/model/mesh payload offsets and the model-element destination remain
unchanged. The canonical scene provider updates/copies its world matrix; these
calls introduce no GameObject secondary-base adjustment. The existing primary
RTTI and caller evidence described in [the query connections](COLLISION_QUERY_CONNECTIONS.md)
applies to this same SoultreeObject boundary.

The segment entry receives two contiguous 12-byte endpoints, the unchanged tree
pointer, the caller's inverse matrix and its mutable vertex array. Its complete
body stores that array for the leaf tests and calls the existing segment query.
The two calls remain inside the complete nonexact SegmentTouchesObject candidate;
their connection does not promote that candidate to exactness.

## Shared hit record

`BoundingBoxTreeQueryState.h` contains the existing `QueryHit` declaration moved
verbatim from the query header. It declares the existing shared pointer and
scratch count. Their definitions remain in `BoundingBoxTreeQuery.cpp`.

The pointer occupies four bytes of writable zero-initialized storage. Retail's
complete ten-byte setter writes the unchanged argument address to `0x579058`;
the source setter now casts the existing provisional record pointer to `QueryHit*`
before that same store. The global remains a pointer, not inline record storage.

Both record views are 20 bytes: fraction at `+0`, point pointer at `+4`, and three
normal components at `+8/+0xc/+0x10`. The query updates the fraction, endpoint
pointer and face normal after accepting a closer hit. Collision retains its
existing stack/contact records and casts only when storing the pointer or
accessing its provisional normal field. No object is copied, constructed or
replaced at this boundary. These field and access facts do not establish complete
original class identity.

The separate `g_CollisionScratchPoints` array connection remains deferred.
Changing its provisional element view altered a partial caller's inline decisions
and complete body. This slice preserves that array's declaration and accesses;
it supplies no alias or substitute array.

## Verification

Same-path VC6 compilations of published `eb470e53` and this change preserve all
**451 nondebug sections and 2,848 relocations** across CollisionObject, the query
provider, CollisionCharacter and all three collision/BVH sample aggregates.
Only the three external decorated names change. All raw bytes, relocation
addends and defined destination identities remain unchanged; no instruction,
filename or data normalization is needed.

The isolated real source build compiles **215/215 TUs** and reduces unresolved
names **386 → 383**, with exactly three removals and no additions. All **215 old
objects, 10,465 nondebug sections and 45,316 relocations** are identical under
those three substitutions. Private evidence is retained in
`work/game-build/collision27/{objects,renames,all-production-objects}.json`.

The existing query connection regression retains **193 strict cases and 31
complete partial fingerprints**, including the canonical bodies emitted again
inside the collision sample aggregate. The new state test checks the provider
storage, four-byte setter write, actual record layouts, canonical imports and
complete scene/query bodies.

Its native test executes the unchanged production setter and five query/helper
bodies in separate temporary objects. Both the production and extracted objects
must match their complete retail bodies before execution. Actual canonical
CrossProduct and VectorCalls objects supply the remaining math dependencies.
There are no scripted query workers or replacement helper implementations.

**56,448 native cases** compare against independent triangle/segment expectations:
front/back faces, translated coordinates, scaled normals, boundary points, misses,
pre-existing closest fractions, both child orders, repeated leaf pointers and
separate vertex arrays. They verify the complete guarded destination record,
untouched alternate record, shared pointer identity and unchanged geometry,
matrices and endpoints. Null setter calls and alternating caller-owned records
exercise pointer storage independently from query execution.

These tests establish the bounded source connections and hit-record behavior.
They do not establish complete collision handling or gameplay. Other query
workers, scratch-array imports and segment math dependencies still prevent the
complete game link.

```powershell
$env:PYTHONPATH = '.;tests'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_collision_query_state_connections test_collision_query_connections
```
