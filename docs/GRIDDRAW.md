# Griddraw.cpp and Gridbase.cpp: the terrain height-field nodes

`src/reconstructed/Griddraw.cpp` with `Griddraw.h`, and
`src/reconstructed/Gridbase.cpp` with `Gridbase.h`. Griddraw.cpp holds every
function of the unit in retail order; four are near misses with `// Not exact`
status comments.

Terrain's surface loader now writes the existing `g_gridSize` and `g_gridLevels`
definitions from Griddraw.cpp. The decoded stores target `0x0067a684` and
`0x0056c200`: the final surface width and its bit length. Griddraw initializes
the latter to 9 and computes the former as `1 << (g_gridLevels - 1)`.
The former terrain-only names described the same mutable integer storage;
no additional globals or initializers are introduced.

## Ownership

- Confirmed: `D:\aardvark\VC\krusty2\Gridbase.cpp` (`0x0056c150`) is used
  once, at `0x0047dc36`, by the line-136 allocation in `0x0047dc20`.
  `D:\aardvark\VC\krusty2\Griddraw.cpp` (`0x0056c288`) is used 19 times
  between `0x0047e773` and `0x004825b0`.
- Strong inference, Gridbase.cpp is `0x0047db60..0x0047dd9b`. `0x0047db60`
  owns the `.bss` just before the constructor's buffer pointer `0x006754a8`
  (`0x006652a4..0x006754a4`). `0x0047d780` is called only from Grid1.cpp
  code (`0x0047cd4e`), so its owner is open.
- Strong inference, Griddraw.cpp is `0x0047dda0..0x0048389f`. The TU's
  `.CRT$XCU` entries are `0x00566224..0x00566238`: the four per-TU vector
  initializers (code at `0x004807c0`), then `0x0047dda0` and `0x0047ddd0`.
  Its `.bss` (`0x006754b0..0x0067b424`) holds both groups of globals. The
  code from `0x0047de90` to `0x004835c0` reads the copy of the shared row
  tables at `0x0056c174`/`0x0056c1b8`. `0x00483910` and later read the
  next copy (`0x0056c2ac`), and GUIManager.cpp's initializers start at
  `0x00484dd0`. Read that way, each Grid*.cpp TU's `.data` ends
  with its `__FILE__` string, so `0x0056c174..0x0056c2ab`
  belongs to Griddraw.cpp, including the levels word `0x0056c200` (9), the
  normal scale `0x0056c204` (1/12) and the 4 x 4 order tables
  `0x0056c208`/`0x0056c248`.
- Tier 1 RTTI: `GridNode` (COL `0x0055cb08`, vtable `0x00553ec0`, 2 slots)
  and `DrawableGridNode : GridNode` (COL `0x0055cad8`, vtable `0x00553e98`,
  9 slots). The vptr writes are at `0x0047e562` (constructor) and
  `0x0047ec7d` (destructor). Grid1.cpp's
  `DrawableGridNodeSharedTextures` derives from it. The GridNode destructor
  `0x004838a0` and slot 1 `0x00483d40` are in the next TU. Terrain's
  `heightField` is the root node, typed `GridNode*` (`0x0047edb0`,
  `DrawableGridNode::ReleaseVertexBuffers(1)`, runs before its delete).

## What the code does (names tier 3)

- Gridbase: `GridBaseBlock` (0xd50 bytes, made by Terrain's loader with
  `new(0xd50)`) holds 17 x 17 eight-byte samples and two more arrays. Each
  array is stored as (packed, unpacked) sizes and LZW-decoded
  (`0x004a03d0`) when packed is smaller. `0x0047db60` builds once the
  curve `(i + 1)^(1 + i*i/255^2) - 1` (`_CIpow`) and its inverse byte table.
- The vertex cache `g_gridVertexCache` (`0x00677910`, 0x2d74 bytes) turns
  (x, z) grid vertices of the current node into 32-byte vertices and
  indices. Each 8-byte slot holds a frame stamp and an index.
- `g_gridEdges` (`0x006754e0`, 289 x 32 bytes) records, for every vertex
  of the 17 x 17 grid, the two vertices it depends on at its subdivision
  level and their direction bits. `0x0047e340`, `0x0047e430` and
  `0x0047e4d0` build it on the first node construction.
- DrawableGridNode loads its header and draw data from the stream
  (`0x0047e600`), walks its quadrants and child nodes (`0x0047ef80`,
  `0x00480940`, `0x00481180`), re-tests vertices against the screen-space
  error (`0x00481cc0`, `0x00482a40`, `0x00482b40`, `0x00482f00`) and
  propagates pinned vertices to neighbours and parents (`0x00482c90`,
  `0x00482dd0`, `0x004835c0`). It also frustum-tests 4 x 4 blocks
  (`0x00481a20`), and frees its vertex buffers when the AgeManager evicts
  them (`0x0047ecc0`, `0x0047edb0`).

## Exact

Gridbase.cpp: `0x0047db60` and the `GridBaseBlock` constructor `0x0047dc20`.

Griddraw.cpp: the ten initializer stubs and bodies (`0x0047dda0..0x0047de10`
and `0x004807c0..0x004808fb`), `GridVertexCache::Reset` `0x0047de20`, the
edge builders `0x0047e340` and `0x0047e4d0`, and the DrawableGridNode
constructor, scalar deleting destructor and destructor (`0x0047e540`,
`0x0047e5e0`, `0x0047ec60`). Also exact: the eviction callback
`0x0047ecc0`, the buffer release `0x0047edb0` and the quadrant walks
`0x0047ef70`/`0x0047ef80`, `0x00480900`/`0x00480920`/`0x00480940` and
`0x00481170`/`0x00481180`. Then the triangle emitter `0x00480700`, the slot 2
stub `0x004806e0` and slot 3 `0x004806f0` (identical-code folded with
RenderTarget's slot 18), and `0x00481580`, `0x00481a20`, `0x00481cc0` and
`0x00481db0`. The rest are `0x004824c0`, `0x004826d0`, `0x00482a40`,
`0x00482ae0`, `0x00482c90`, `0x00482f00`, `0x00483040` and `0x00483100`.

Also exact: `0x004813e0` (415 bytes), a
consistency pass that compares the shared edge cells of neighbouring
children (height, curve index, vertex bit 7), counts mismatches nothing
reads, and recurses into `children[row]` once per column (sic) and then
into every child.

Also exact (8 more calibration cases): the buffer
rebuild `0x0047f840`, the index builders `0x0047fce0`, `0x0047fe70` and
`0x00480200`, the draw submissions `0x00480c90` and `0x00480fb0`, the
detail-band re-test `0x00482760` and the border hand-off `0x004835c0`.
Then the block distance `0x00481b30` and the direction-bit update
`0x00482dd0`.

## Source shapes retail needs

- `0x0047db60` casts the loop counter to `double` inline. A named
  `double` local makes VC6 align the frame (`and esp,-8`).
- The edge builder writes each link as x0, z0, x1, dir0, z1, dir1. Other
  orders, or `z0 = z1 = z`, change VC6's store schedule.
- `0x0047e4d0` keeps three counters and doubles them with `<<= 1`. With
  `*= 2` VC6 emits `add`.
- `0x0047ef80` halves `size` in place, and the 8-cell branch sets it back
  to 16. Retail jumps from that branch straight into the top-level
  rebuild, which the later `size == 16` test reaches only that way. A
  separate `half` local gives a different function (41 of 626 bytes).
- `0x00483040`/`0x00483100` walk the sixteen block centres with one
  running index (`i += 4`, then `i += 4 * 17 - 16`). Retail's loop
  counters are down-counters that the body never reads. In `0x00483100`
  the edge's x and z go into locals before the test call, as retail keeps
  them for the second call.
- `0x00482f00` declares `result` after the index and cell pointer. VC6 then
  keeps it in ebx and spills it into the dead `z` slot, as retail does, and
  every path returns it.
- `0x00480700` indexes the triangle list through a local `k = n * 3`. A
  `[n * 3 + j]` index or a 6-byte triangle struct make VC6 scale by 6.
- `0x00481cc0` computes the limit `distance * factor * distance` into a
  local before the counter increment, and tests `<= limit` to return 1.
- `0x00480ad0` assigns all four range values in each arm and computes the
  counts from the range records (`ranges[block].end - ranges[block -
  1].end`), as `0x00481300` does. Subtracting the start locals gives a
  different register plan (395 of 438).
- `0x00481300` steps a row counter and names the row offset, `int j =
  row * 16;`, so VC6 adds `i + j` first and the table entry last, as
  retail does. The `j += 16` loop of `0x0047ef80` and `0x00480ad0`, an
  inline `row * 16` or any order of the three index terms load the table
  entry first; a `row << 4` local is worse.
- `0x00481a20` re-reads the block range through the draw data for each
  field. A range-pointer local turns the float copies into integer moves.
- `0x004824c0` stores the 0x004815e0 result in a float local before passing
  it to slot 3. Its last parameter is the flag 0x004815e0 tests.
- `0x00480200` and `0x0047fce0` call each other per subdivision level.
  Retail's neighbour test is one inline helper per child
  (`Blocked`), the leaf case is the `else` of `if (children != 0)`, and
  `a = b = 0` stores b first. Both emitters index through `k = n * 3`.
- `0x0047fe70` writes the second diagonal flag as `(1 - bit) && ...`; retail
  computes `1 - (byte & 1)`. Its `else` branch tests `dx == 1`, not `dz`.
- `0x0047f840` starts each half with `n = 0; m = n;` so the fallback call
  pushes the returned count, as retail does.
- `0x00480fb0` tests `block >= 0` first for the slot 7/6 choice. In
  `0x00480c90` each colour branch has its own draw call (VC6 tail-merges
  them and places the second one last, as retail does), and the texture
  coordinate loop writes through `float* uv = &vertices[i].field_0x10`, which
  makes VC6 base its loop pointer on `packed` (+0xc) as retail does.
- `0x00482760` scans the curve-sorted vertex list of GridBaseBlock (+0x908
  start positions per curve value, +0xb08 vertex indices).
- `0x004835c0` computes the shift with a `shift += 4` loop over
  `level - 1`; VC6 reduces it to retail's `lea ecx,[eax*4]`.
- `0x00481b30` writes each axis delta back into the bound it was taken
  from inside the test (`(minX = ex - minX) < 0.0f`). The assignment makes
  VC6 compare a copy (`fld st(0) ; fcomp`), and overwriting minX keeps the
  single-use maxX stored: with a separate delta local VC6 folds maxX into
  `minX + size`. The three axis results are if/else-if/else assignments
  added after the tests, so VC6 copies the FastSqrt tail into each y case.
- `0x00482dd0` sets one `notify` flag in the set and the clear path and
  tests it after the if/else (`else if (flag && !changed) return;`).
- `0x004815e0`'s 65535 clamps are `w >= 65535.0f ? 0xffff : ...`; the `<`
  form inverts the branch.
- Unsigned shorts: the draw data's +0x122..+0x128 (stored as 0xffff),
  `GridNode::field_0x20` (compared with `ja`), and the sample height.

## Near misses

These four sit in [Griddraw.cpp](../src/reconstructed/Griddraw.cpp) in retail order, each with a
`// Not exact` status comment; the forms tried are in
[Near misses: forms tried](#near-misses-forms-tried).

| Function | Bytes | What differs |
|---|---:|---|
| `GridVertexCache::GetVertex` `0x0047de90` | 1069/1194 (1187 compiled) | x/z/index register assignment; the packed-word expression |
| `RebuildBlockBuffers` `0x0047f210` | 426/1576 (1586 compiled) | x/count-copy registers, frame slots, minY/maxY load order |
| `ComputeViewerDetail` `0x004815e0` | 64/1082 (1083 compiled) | the two axis deltas stay on the x87 stack across the branch in retail; VC6 stores them, which shifts the frame and keeps `push edi` in the prologue |
| `WalkVisibilityQuadrants` `0x00481de0` | 73/1750 (1712 compiled) | `extent`/`screenMatrix` in ebp/ebx where retail keeps `size` in ebx and `screenMatrix` in ebp; the subdivision block's placement |

All four agree with retail under `tools/emu_diff.py`
(`grid_vertex_cache_get_vertex`, `grid_rebuild_block_buffers`,
`grid_viewer_detail`, `grid_walk_visibility`).

## Not reconstructed

Gridbase's possible `0x0047d780`. Every Griddraw.cpp function start is now
exact or a documented near miss.

## Remaining uncertainty

All type, member and function names are provisional. `GridTerrain`,
`GridCamera`, `GridVisibilityClipper`, `GridAgeManager` and
`GridVertexSink` are boundary views of classes reconstructed elsewhere, or
not yet reconstructed, as are `GridRenderDevice` (Terrain+0x18, thiscall
virtuals), `GridManagedTexture` (`0x00510910`) and `GridGameSettings` (`0x0056e26c`);
only their called slots and touched fields are known. GridTerrain+0x2c is the
canonical `GroundFogableObject` base, whose `FogVertices` (`0x00484f10`) fogs unlit vertices. The fifth argument of
`0x00482dd0` is the originating cell (`0x00483200` compares it with cell
addresses), not a flag. The shared row tables are written as per-TU statics
because three identical copies exist. The header that supplied them, and
the vector constants (written as Lzw.cpp's stand-in), is not identified.

## Reproduce

```bash
python tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" \
  src/reconstructed/Griddraw.cpp -o work/griddraw.obj
python tools/match.py --exe "$MCM2_EXE" --target-va 0x0047ef80 \
  --target-size 643 --obj work/griddraw.obj \
  --symbol '?WalkVisibleQuadrants@DrawableGridNode@@QAEHHHHH@Z' \
  --bindings src/reconstructed/Griddraw.bindings.json --json
```

Not function starts (branch targets inside the functions above):
`0x0047f190`, `0x0047f470`, `0x00480020`, `0x00480074`, `0x00481330`,
`0x004831a0` and `0x00483510`.

`0x004815e0` computes, for the viewer at Terrain+0x60, the squared nearest
and farthest distances to the node's x/z range (+0x164..+0x170 of the draw
data) and the y delta to +0x18/+0x1c, returns `FastSqrt(dy² + near²)` and,
unless `coarse`, writes the detail words +0x122/+0x124 (the old values go
to +0x126/+0x128; +0x124 is 0 while Terrain+0x6c is clear and 0xffff when
either range distance is 0). `0x00481de0` tests the node box against the
clipper (`0x0052f570`, the camera from the renderer's +0x08, counting the
tests in `0x0068a084`), recurses into the four quadrants until single
child blocks, sets their masks (0x3fffff fully visible, 0x1fffff a leaf,
cleared through the empty `0x00464e90`) and marks the ancestors dirty
when the mask changed.

## GridNode.cpp (provisional name)

`src/reconstructed/GridNode.cpp`, `0x004838a0..0x00484dd0`, the code after
Griddraw.cpp and before GUIManager.cpp's initializers; its `.data` is the
third copy of the row tables (`0x0056c2ac`/`0x0056c2f0`) and the constant
1/127 at `0x00553ec8` right after the GridNode vtable. Exact: the GridNode
destructor `0x004838a0`, its deleting wrapper `0x004838f0` and the cell
sampler `0x00483910` `GetCellCorners` (1067 bytes, in GridNode.cpp: reading
`level` through a `const unsigned char&` makes VC6 reuse the leaf-test byte,
`and eax, 0xff ; shl eax, 2`, and `mask`/`one` must be computed before the
cell coordinates `cx`/`cz`), and the leaf lookup `0x00484d70` `FindLeaf`
(90 bytes, a tail call into the child through an index local with the
child slot read twice; VC6 eliminates the tail call into retail's loop).
Near miss (in GridNode.cpp, in retail order with a `// Not exact` status comment):

| Function | Bytes | What differs |
|---|---:|---|
| `0x00483d40` (GridNode slot 1, 4140 B) | 96/4189 | ebp frame and four bare `fistp dword ptr [eax]` (0x4844aa, 0x4844be, 0x484670, 0x484687) from an `__asm fld/fistp` rounding helper while `__ftol` is still called for the other casts, which the no-inline-asm rule rules out; `/Oy-` only fakes the frame and leaves the x87 crossing loops different |

What the two large functions do (names tier 3; `0x00483910` is
`GetCellCorners` in Griddraw.h): `0x00483910(x, z, heights,
normals, bytes)` descends to the leaf holding sample (x, z), clamping out of
range indices to the border child, and writes the four corner heights
`((h - base) * field_0x10 + base) * field_0x14`, normals (signed bytes / 127,
x and z also scaled by field_0x10) and sample bytes of that cell in the order
(x,z), (x+1,z), (x,z+1), (x+1,z+1); without a leaf it writes 0, (0,1,0), 0
and returns 0. Slot 1 `(p0, p1, out, outNode, outX, outZ)` intersects the
segment with the height field: it collects the crossings with the x and y
grid lines of the node (cell size `1 << shift`), merges them along the
segment, and per cell either recurses into the child whose height range
(+0x18..+0x1c) the segment reaches or, in a leaf, intersects the two cell
triangles (diagonal chosen by the parity of ix ^ iy) through
`0x004a1300`; a hit inside the segment's box is written in parent units
(`gridX << (level * 4)` added) and returns 1.

## Field names

`GridTerrain` takes `appliedDrawDistance` (+0x6c) and `canSinglePassDual`
(+0xcac, printed as "CanSinglePassDual" by Terrain slot 14) from
`src/krusty2/broadphase/Terrain.h`; the statistics counters (+0x90, +0x98,
+0xa4, +0xb0), the child-table stream offset (+0xc38), the close-up state
flag (+0xcb4) and `GridNode::childOffsets` (+0x30) are named from the code
that writes them (tier 3). The renames changed no calibration bytes.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x0047ca00` `EvictNodeTexture`
- `0x0047ca40` `EvictBlockTexture`
- `0x0047e340` `FindEdgeNeighbours`
- `0x0047e430` `BuildEdgeLevel`
- `0x0047ecc0` `EvictNodeBuffers`
- `0x0047edb0` `ReleaseVertexBuffers`
- `0x0047f840` `RebuildWholeNodeBuffers`
- `0x0047fce0` `EmitDiagonalTriangles`
- `0x0047fe70` `EmitLeafEdgeFan`
- `0x00480200` `EmitEdgeTriangles`
- `0x00480700` `AppendCornerTriangle`
- `0x00480c90` `SubmitUnlitRange`
- `0x00480fb0` `SubmitLitRange`
- `0x004813e0` `CountEdgeDifferences`
- `0x00481a20` `TestBlockFrustum`
- `0x00481b30` `BlockDistance`
- `0x00481cc0` `CanDropVertex`
- `0x00481db0` `TestVisibility`
- `0x004824c0` `UpdateLevelOfDetail`
- `0x00482760` `RetestChangedVertices`
- `0x00482a40` `TestVertex`
- `0x00482ae0` `TestNodeVertices`
- `0x00482b40` `TestRectangleLevel`
- `0x00482c90` `PropagateVertexChange`
- `0x00482dd0` `SetVertexDirection`
- `0x00482f00` `RetestUnpinnedVertex`
- `0x00483040` `ForceBlockCentres`
- `0x00483100` `ReleaseBlockCentres`
- `0x004835c0` `PropagateBorderChange`
- `0x0047db60` `BuildCurveTables` (Gridbase.cpp)
- `0x0047d470` `DrawableGridNodeSharedTextures::BuildNodeTexture` (Grid1.h)
- `0x0047d780` `DrawableGridNodeSharedTextures::BuildBlockTexture` (Grid1.h)
- `0x00484d70` `GridNode::FindLeaf` (GridNode.cpp)
- `0x0047e600` `ReadNode`, `0x0047ef80` `WalkVisibleQuadrants`, `0x00480940`
  `DrawVisibleQuadrants` and `0x004826d0` `PropagateFullMask` (near-miss sample)

## Near misses: forms tried

Carried over from the header of the former `samples/render` aggregate when
its bodies moved into Griddraw.cpp. In the visibility-walk notes `a7`, `a8`
and `a9` are the parameter names of that time.

k-scan (s22): compiled behind 0..63 prepended typedefs, 0x0047de90,
0x0047f210, 0x004815e0, 0x00481de0 have the same code at every
count, so their differences are not a declaration-count effect
(docs/VC6_OPERAND_ORDER.md section 2).

### GetVertex `0x0047de90`

GridVertexCache::GetVertex (0x0047de90, 1194 bytes; 1069 of 1187): control
flow, the x87 sequences and all stores match. VC6 keeps x in esi, z in ebp
and the slot index in edi; retail keeps x in ebp, z in edi and the index
in esi, and computes the packed (dx << 16) + dz word as
((gx * 0xffff + x) << 16) - gz + z. Index expression forms, a split
declaration and seven spellings of the packed word leave both unchanged. Also
unchanged or worse: `* 65536`/`* 0x10000` of the x difference, the shifts
distributed over x and the origin, unsigned short casts, and `|` instead of
`+` (the `|` form gives z retail's edi but not the x/index registers, 1084).
dx/dz locals shared by the u, ny and packed terms give 744; the operand
order of the index and origin sums, and ten more packed-word spellings
(`* 65536` distributed or not, `- gz + z`, `x + -gx`), leave 1069.

### RebuildBlockBuffers `0x0047f210`

DrawableGridNode 0x0047f210 (per-block buffer rebuild, 1576 bytes; 426 of
1586): the calls, reallocations, copies and the bookkeeping after the loops
match in shape. VC6 here gives x edi and the triangle-count copy esi
(retail: esi and edi), swaps the frame slots of z and indexTotal and of bx
and the two strength-reduced block offsets, loads minY before maxY for the
box centre, re-reads indexTotal for the index byte count and stores a dead
vertexStart * 32 (10 bytes longer). Tried: all 24 declaration orders of
x, z, indexTotal and bx (no effect), x and bx as one variable (VC6 merges
them; retail keeps both, so x steps inside the body), three
triangles/prevTriangles initialisations and the origin taken from x/z.

### ComputeViewerDetail `0x004815e0`

DrawableGridNode 0x004815e0 (detail update, 1082 bytes; candidate 1083):
computes the squared near and far distances of the viewer to the node's
x and z range, the y delta, the distance (FastSqrt), and the two
16-bit detail words from (dyÂ² / nearest + nearest) * +0x174 * +0x74,
nearest being the closest of the three distances. Control flow, calls and clamps match
(the 65535 clamps are ternaries; `0xffff` immediates stay immediates
once they are). Retail keeps the two axis deltas on the x87 stack across
the three-way branch (fcom on the first, the second loaded before the
fnstsw, spilled ad hoc in the arms); VC6 here stores both and reloads
them, which shifts the frame slots, keeps `push edi` in the prologue
instead of after the early return and tail-merges the early return.
Tried: locals, block scopes, const/register, repeated expressions, an
inline helper with reference, pointer or by-value arguments, and an
accumulating form (which enregisters only the first delta).
The two 65535 clamps are written `upperError >= 65535.0f ? 0xffff : ...`, which
gives retail's branch polarity (64 of 1083; the `<` form 60). Also tried for the
deltas: no named deltas (expressions throughout), only one of them named,
the delta assigned inside the test (`(ax = ...) < 0`, comma forms), copies
of the bounds overwritten by the deltas (the form that made 0x00481b30
exact), and the far term written before the near term; none keeps both on
the x87 stack, and small probes never do either. Also inert or worse: the
second delta assigned in the else-if test (1067 bytes, strict 49), the
grid/data locals swapped or dropped, the far offset without a local, and
the early return as `coarse != 0`, a goto past it or with `distance`
declared first (the return still merges with the final epilogue).

### WalkVisibilityQuadrants `0x00481de0`

DrawableGridNode 0x00481de0 (visibility walk, 1750 bytes; candidate
1712): tests the node's box (or the four quadrant boxes, recursively,
down to the child blocks) against the clipper and updates the child
mask; `a7` is the recursion level, `a8` the quadrant path and the
child blocks' masks are set to 0x3fffff (all visible), 0x1fffff (leaf)
or cleared with 0x00464e90. Structure and calls match; the frame is
the same size. VC6 here keeps `extent` in ebp and `screenMatrix` in ebx where
retail keeps `size` then `a9` in ebx and `screenMatrix` in ebp, and lays out the
subdivision block after the general tests rather than as the
fall-through of the `size <= 1` branch, so the body differs throughout.
Retail's order is: the size > 1 tests, the size <= 1 block falling into
the subdivision, then the visible/corners tests, the all-visible loops,
the size == 0 child case, a second `visible` test before the a7 switch
(`childMask |= 1`, `2 << a8`, `0x20 << a8`; cases 1 and 2 return through
the `a7 != 0` exit). Writing that switch after the if/else under its own
`if (visible)`, `if (visible && corners == 0) ... else if (visible)`, and
the size branches swapped (`size <= 1` first) do not move the subdivision
block ahead of the tests.
Session 25 (masked linked-style 0.675 -> 0.711, strict 143 -> 81/1750):
- logic fix: on the size == 16 path with data->b0 set retail loads ebx = 1
  (0x481e8a), stores it to count/childMask/changed and then pushes ebx as
  the `coarse` argument of every later UpdateLevelOfDetail and recursive
  call (0x481eec, 0x4821b5, 0x48225a, 0x482335) without reloading it, so
  the walk continues with coarse = 1 there; the source now says so.
- the corners test is `if (corners) { loops } else { subdivide }` (retail
  falls into the loops and jumps back to the subdivision, 0x482144), the
  depth switch sits under its own `if (visible)` after the if/else, and
  the parent walk is a do/while (no null test of `this`, 0x482448).
- ruled out here: center/extent through `const GridVec3&` vs indexed
  (0.710 vs 0.711), `size <= 1` first (0.563), visible/corners initialised
  at the declaration instead of the else block (0.631).
Linked-accuracy audit (masked 0.725 -> 0.719, 500/500 instructions):
- logic fix: the count test after the four recursive calls compared
  `depth + 1 == 8`; retail reloads esi from the halved `size` slot at
  0x4820fe (`mov esi,[esp+0x8c]` with three pushes outstanding) and tests
  `cmp esi,8` at 0x482370, so the condition is `size == 8 || size == 16`.
- retail shares one `count == 0 / childMask == 0` block (0x482370) between
  the all-visible path (reached only for size 16) and the subdivision path
  (size 8 or 16); the source writes it once after the corners if/else as
  `(corners == 0 && size == 8) || size == 16`. That is behaviourally the
  same: the child case reaches it with size 0. Two separate tests scored
  0.632; ordering `size == 16 ||` first 0.721 (larger body); a split or
  goto form 0.620-0.638.
Big-gap pass (73 of 1750 strict, candidate 1712, same frame 0x54): the
first divergence is at offset 4, register choice. Retail loads `size`
into ebx, screenMatrix into ebp and center into esi between the pushes
and compares `cmp ebx, 1`; VC6 here loads size (edx), extent (ebp),
center (esi) and screenMatrix (ebx) up front and keeps the constant 1
in ecx (`cmp edx, ecx`), then pushes TestBox's arguments from the
registers where retail reloads extent from the frame. Unchanged
(186 of 500 aligned): `count = 1` before `coarse = 1`, the 1s chained
through coarse (`count = coarse` ..., 69), `visible`/`corners` order
in the else block, oldMask declared last; `size >= 2` 101 strict by
position only (185 aligned), oldMask declared first 174 aligned.
Dropping `coarse = 1` (a probe, not retail's logic) does not free the
constant register either.

### IntersectSegment `0x00483d40`

Carried over from the header of the former `samples/render/GridNodeNearMisses.cpp` when its body moved into GridNode.cpp.

```text
0x00483d40 (GridNode slot 1, 4140 bytes): linked masked 0.920, strict 0.520
(1369/1387 insns; was 0.832/0.449 without the frame pragma, 0.602 before). Readable reconstruction of the decoded control flow
(grid-line crossings, merge, child recursion, leaf triangles).
Retail has an ebp frame (`sub esp, 0x43c`) and rounds the cell indices with four
bare `fistp dword ptr [eax]` (0x4844aa, 0x4844be, 0x484670, 0x484687; the
`fstp [ebp+8] ; fld [ebp+8] ; mov eax, [ebp+0xc] ; fistp [eax]` pattern of an
inlined `__asm { fld f ; mov eax, out ; fistp [eax] }` helper) while `__ftol` is
still called for the other casts (0x483dd2, 0x483f96, 0x484d40, 0x484d57), so
/QIfist is excluded and the match is out of reach under the no-inline-asm rule.
GridRoundToInt rounds them with src/bringup/X87FloatToInt.h (fistp's ties-to-even).
Retail has no range test on the rounded cell indices (only the child loop's
`== 16 -> 15` clamp, 0x4844c3/0x4844d3).
Differential emulation against retail (unicorn: random 1- to 3-level grid trees, lattice,
tie and axis-aligned segments, every x87 rounding and precision mode) finds no behaviour
difference in 6000 cases; see docs/GAME_BUILD.md (bodies checked by emulation).

Logic fixes from the retail branches (session 25):
- merge dedupe: after taking an x crossing, retail drops the next y crossing when
  x OR y is equal (0x4842a5/0x484305 jump on the x compare alone); it was `&&`.
  The tail arms keep `&& k != 0`.
- leaf distance tests are `A <= B` (`test ah, 1 ; je found`), not `!(B < A)`, and
  the height max is a `>` ternary (`GridMax`): both differ only for NaN inputs.
Shape levers that moved the masked score: the crossing loops split by the sign of
dx/dy with `step` set by an if/else, member-wise stores with `nx = 1` placed between
them, i/j/k at function scope, the y midpoint computed before the x rounding,
`goto found` with the label after the loop's `return 0`, `v0 = Vector3(...)`
temporaries, nested GridMin/GridMax (retail evaluates Min(h01, h11) first).
Ruled out: `twoCells` spelled cellSize + cellSize / * 2 / 2.0f (identical); a
GridCopy(dst, src) pointer helper (one strength-reduced pointer instead of the
per-member ones retail has); a struct-copy loop (becomes rep movsd); an inline
rounding helper (VC6 inlines the long X87FloatToInt body at some sites).
Frame: `#pragma optimize("y", off)` on this function alone gives the EBP frame retail's
inline-asm helper forces (docs/GAME_BUILD.md#frame-pointer-units; the unit's other
functions are unchanged). With it the leaf cell is an index into block->cells (retail
addresses `[cells + idx*8 + disp]` per corner; a cell pointer became `lea` + offsets)
and out->y is stored before out->z, as retail does (0x484fa4..0x485029).
Remaining: retail keeps p1 in its argument slot (`mov edx, [ebp+0xc] ; fld st(0) ;
fcomp [edx]`, the asm helper's register cost) and nx/ny in ebx/edi, and its four
fistp sites are inline where GridRoundToInt is a call.
k-scan (s22): compiled behind 0..63 prepended typedefs, 0x00483d40 has the
same code at every count, so its difference is not a declaration-count
effect (docs/VC6_OPERAND_ORDER.md section 2).
```

### Node loader `0x0047e600` (exact)

DrawableGridNode 0x0047e600 (node loader) is exact and lives in
src/reconstructed/Griddraw.cpp. Its last difference was the operand order
of the extent.x/extent.y products (`fld cell; fmul min` against retail's
`fld min; fmul cell`, 1605 of 1617). Copying the two minimums into float
locals just before the products makes them the newer leaves (1617; one
local alone gives 1613 for x, 1609 for y). Forms that did not move it:
either operand order, the sum order of the centre terms, a `cell` local,
inline multiply helpers, a GridVec3 constructor for the centre, 432
centre/extent spellings, references or pointer aliases to either operand
(216 forms), 0..33 dead locals and 0..63 prepended typedefs.
