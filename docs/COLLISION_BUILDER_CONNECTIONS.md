# Collision shape construction connections

CollisionObject's hull, model and mesh setup now calls five existing source
providers: IdentityMatrix (`0x4a1410`, 71 bytes), BuildModelTriangleTree
(`0x42c8c0`, 928 bytes), BuildTriangleMeshTree (`0x42cc60`, 831 bytes),
BuildModelPointTree (`0x42d390`, 1,608 bytes) and BuildModelBoxTree
(`0x42dc90`, 501 bytes). All five complete provider bodies match retail with
relocations resolved. No implementation body changed.

`BoundingBoxTreeFunctions.h` shares those declarations between the provider
and physics caller using forward-declared pointer types. The full tree header
contains a different TU's vector constants; importing it into CollisionObject
would conflate those constants with the physics TU's existing initializer set.
This interface header does not introduce another class definition or wrapper.

## Boundary evidence

The six affected caller bodies are complete strict matches:

| Caller | Retail entry | Bytes | Connected calls |
| --- | --- | ---: | --- |
| SetModelShape | `0x4324b0` | 615 | identity, model triangles, model points |
| SetHullShape | `0x432720` | 214 | identity, model triangles, model points |
| SetTriangleMeshHullShape | `0x4328b0` | 231 | identity, mesh triangles |
| SetMeshShape | `0x432ab0` | 122 | model boxes |
| ReadShape | `0x439e10` | 178 | identity |
| ReadModelShape | `0x43a050` | 387 | identity |

Each root still uses the original `0x24` allocation. BoxTreeNode and the
collision bounds view have their center at `+4`, half-extents at `+0x10` and
children at `+0x1c/+0x20`. Hull local and relative matrices remain at `+0x88`
and `+0x148`; triangle roots, point roots and vertex arrays remain at
`+0x188/+0x18c/+0x190`. Vectors are 12 bytes and matrices 64 bytes with the
translation row at byte 48.

Model and frame pointers retain their primary scene addresses. Retail RTTI for
D3DIMSoultreeObject places SoultreeObject at zero and GameObject at `+0xc`;
no secondary-base adjustment belongs on these calls. The builder's model fields
at `+0x1a4/+0x27c/+0x28c` agree with the corresponding canonical model fields.
These selected accesses do not establish whole-class equivalence for the
remaining provisional views.

Model triangle construction returns its allocated vertices through the same
pointer-to-pointer argument. Mesh triangle construction receives the same
writable copied vertex array and integer indices. Point-tree construction keeps
the optional frame pointer. Box-tree construction intentionally transforms
`2 * count` consecutive corner vectors in place, using a copied inverse of the
const input matrix; its mutable vector pointer preserves this contract.

The five IdentityMatrix calls now express the canonical by-value Matrix4
return. VC6 supplies the same hidden output pointer and copies the same sixteen
DWORDs from the returned address. Complete caller identity covers the allocation,
argument order, calls and exception metadata around these operations.

## Verification

The point-tree builder also uses the supplied TransformVertexGroups
(`0x440d40`, 486 bytes) and GetMatrixIn (`0x4fca80`, 494 bytes) methods.
Both complete providers and the 1,608-byte caller match strictly. Its three
direct calls preserve the primary scene pointer and the 64-byte matrix output.
The isolated real link removes these two obsolete names, reducing 423 to 421
unresolved symbols without additions. All 40 production tree sections and
268 relocations remain identical under these two verified name substitutions.

The same declarations update the nonadmitted GatherModelTriangles sample.
Its complete 1,627-byte body improves from 1,618 to 1,622 matching positions:
two independent stack-local MOV reloads exchange order at offsets 1257–1264,
now matching retail. All other bytes and relocation records are unchanged;
neither reload changes flags or memory, and no branch or symbol targets the
interior of that window. This remains a partial match outside the game build.

`tests/test_collision_builder_connections.py` checks the thirteen complete
caller/provider bodies, fourteen direct call sites, primary/secondary model RTTI
and both independently compiled layout views, plus the complete partial sample.
The private before/after audit
preserves all 199 nondebug sections and 1,044 relocations across CollisionObject,
BoundingBoxTreeBuild and MatrixUtil under the five verified external renames.
No bytes or relocation addends are normalized, and no extent is shortened.

The isolated real build compiles 211/211 translation units and reduces missing
symbols from 434 to 429, removing exactly these five names with no additions.
This supplies the real collision constructors' dependencies; scene loading and
racing still require a linked source-built executable and runtime testing.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python tests/test_collision_builder_connections.py
python tools/build_game.py -j6
```
