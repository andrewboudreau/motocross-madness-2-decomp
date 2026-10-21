# Collision ownership and scene traversal

CollisionObject now calls five existing source implementations through their
canonical declarations. No provider body changes and no forwarding wrappers
are introduced.

| Provider | Retail entry | Complete bytes | Direct caller sites |
| --- | --- | ---: | ---: |
| FreeBoxTree(QueryTreeNode*) | `0x42a160` | 52 | 3 |
| GameObject::AppendClassName(GameObject*) | `0x469ce0` | 201 | 1 |
| SoultreeObject::CountNodes | `0x4fda30` | 46 | 2 |
| SoultreeObject::CollectDescendants | `0x4fda60` | 76 | 2 |
| D3DIMSoultreeObject::NodeMovesVertices | `0x445060` | 91 | 2 |

All complete providers and their five affected caller bodies match strictly
with relocations resolved: hull cleanup (`0x431da0`, 71 bytes), mesh cleanup
(`0x431e50`, 32), CollisionObject construction (`0x431e70`, 346), model shape
construction (`0x4324b0`, 615) and model shape reading (`0x43a050`, 387).

## Boundaries and retained behavior

Tree destruction receives the unchanged allocated root address. Its marker at
offset zero determines whether to recurse through the children at `+0x1c/+0x20`
before deleting the node. Leaf storage can be shorter than the 36-byte interior
node; the negative marker prevents those child accesses. The caller retains
its null checks. The declaration in BoundingBoxTreeQueryFunctions.h avoids
importing the query TU's unrelated inline helpers and vector constants. Keeping
this interface separate from the builder header also preserves VC6's existing
instruction scheduling in the tree-builder sample.

The removed GraphicsTest declaration hid the existing inherited GameObject
method. CollisionObject's receiver and explicit owner argument both still
point at the GameObject subobject at `+0xc`. Retail RTTI places GraphicsTest
and GameObject there, and the complete constructor preserves both adjustments.
The supplied method reads RTTI and updates GameObject's class-name buffer at
`+0x28`; treating the complete CollisionObject address as that receiver would
be wrong.

Scene traversal retains primary Soultree addresses and the same mutable output
array. CountNodes includes the receiver, its sibling chain and descendants;
CollectDescendants appends children before recursively visiting their children.
The caller continues to seed element zero and initialize the count to one.
ReadModelShape still climbs the parent field at `+0x13c` before counting.
No change is made to these traversal or array-allocation semantics.

The former Accepts call names now resolve to NodeMovesVertices. Its decoded
body indexes the selected model LOD, walks surface vertex groups and tests
whether a group has the supplied scene node and a positive vertex count.
The caller's existing integer argument is passed unchanged as that LOD index;
the semantic names remain inferred. Model receivers retain their primary
address, independently of the model's GameObject secondary base.

## Verification

The private same-path before/after comparison covers CollisionObject,
BoundingBoxTreeQuery, soultree, GameObject and D3DIMSoulTree: all **463 nondebug
sections and 2,239 relocations** preserve bytes, attributes and destinations
under only these five verified external-name substitutions. No instruction
bytes, relocation addends or function extents are excluded.

An independent fresh compilation against published `7dd638b8` also preserves
the complete tree-builder sample: all 42 nondebug sections and 294 relocation
destinations, with no name or filename normalization. GatherModelTriangles
retains its full 1,627 emitted bytes and 26 relocations; its 1,622 matching
positions remain explicitly nonexact. The separate query interface is included
in this final proof (`work/game-build/tree25-review/proof.json`).

The inherited-member cleanup changes one nonadmitted CreateRacers sample's
instruction scheduling. Within its complete 13,428-byte body, offsets 538–544
exchange `MOV EBX,1` and `XOR EDI,EDI`. These independent register writes have
the same final registers and flags, no memory effects, and no interior branch,
symbol or relocation destination. All other body bytes and all 1,929 relocations
across the sample's 206 nondebug sections are preserved. Matching positions
improve from 13,390 to 13,396; this remains explicitly nonexact.

`tests/test_collision_scene_connections.py` verifies all ten complete bodies,
ten direct call sites, retail CollisionObject RTTI, the compiler's `+0xc`
GameObject conversion and separately compiled actual scene/query layout views.
The fixture includes the real local CollisionSourceNode declaration.

The isolated real build `build-3fd68y1r` compiles 212/212 translation units and
removes exactly five unresolved names, reducing 420 to 415 without additions.
This establishes source connections; integrated shape loading, collision and
racing still require a linked executable and runtime tests.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python tests/test_collision_scene_connections.py
python tools/build_game.py -j6
```
