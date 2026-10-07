# Griddraw.cpp and Gridbase.cpp: the terrain height-field nodes

`src/reconstructed/Griddraw.cpp` with `Griddraw.h`, and
`src/reconstructed/Gridbase.cpp` with `Gridbase.h`. Near misses are in
`samples/render/GriddrawNearMisses.cpp`.

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
  `0x004838a0` and slot 1 `0x00483d40` are in the next TU. Terrain.h's
  `TerrainShutdownObject` is a boundary view of the same node
  (`0x0047edb0` is its `Shutdown(1)`).

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
- Unsigned shorts: the draw data's +0x122..+0x128 (stored as 0xffff),
  `GridNode::field_0x20` (compared with `ja`), and the sample height.

## Near misses

See the notes at the top of `samples/render/GriddrawNearMisses.cpp`:

| Function | Bytes | What differs |
|---|---:|---|
| `GridVertexCache::GetVertex` `0x0047de90` | 1069/1187 | x/z/index register assignment; the packed-word expression |
| `0x0047e430` | 152/156 | row offset vs x operand order of the edge index |
| `0x0047e600` (node loader) | 1091/1630 | x87 operand order of the extent terms; extent.y kept on the stack |
| `0x00480ad0` (block draw) | 395/438 | the zero offset kept in eax for compares; esi/edi swapped |
| `0x00481300` (block walk) | 114/216 | child index term order |
| `0x00482b40` (rectangle re-test) | 292/327 | as `0x0047e430`, plus the zEnd/start order |
| `0x00482dd0` (direction bit) | 147/333 | the shared notify tail and the early-return placement |
| `0x0047f210` (per-block rebuild) | 426/1586 | x/count-copy registers, frame slots, minY/maxY load order |
| `0x00481b30` (block distance) | 139/393 | which floats stay on the x87 stack |
| `0x00483200` (border walk) | 84/956 | x/z in ebp/ebx (retail ebx/ebp), `level` reload |

| `0x004815e0` (detail update) | 60/1083 | the two axis deltas stay on the x87 stack across the branch in retail; VC6 stores them, which shifts the frame and keeps `push edi` in the prologue |
| `0x00481de0` (visibility walk) | 143/1712 | `extent`/`a1` in ebp/ebx where retail keeps `size`, `a9` and `a1`; the subdivision block's placement |

## Not reconstructed

Gridbase's possible `0x0047d780`. Every Griddraw.cpp function start is now
exact or a documented near miss.

## Remaining uncertainty

All type, member and function names are provisional. `GridTerrain`,
`GridCamera`, `GridVisibilityClipper`, `GridAgeManager` and
`GridVertexSink` are boundary views of classes reconstructed elsewhere, or
not yet reconstructed, as are `GridRenderDevice` (Terrain+0x18, thiscall
virtuals), `GridVertexLighter` (Terrain+0x2c, `0x00484f10`),
`GridManagedTexture` (`0x00510910`) and `GridGameSettings` (`0x0056e26c`);
only their called slots and touched fields are known. The fifth argument of
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
  --symbol '?UnknownFunction47ef80@DrawableGridNode@@QAEHHHHH@Z' \
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

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x0047ca00` `EvictNodeTexture`
- `0x0047ca40` `EvictBlockTexture`
