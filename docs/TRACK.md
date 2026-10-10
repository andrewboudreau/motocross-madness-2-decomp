# Track

The literal `__FILE__` `D:\aardvark\VC\krusty2\Track.cpp` (`0x0057508c`) is
referenced by `0x00515e70`, `0x00515ed0`, `0x00516800`, `0x00516870`,
`0x00516980`, `0x00517340`, `0x00517930` and `0x005179f0`. No RTTI names a
class, so `Track` is a name inferred from the file only. Canonical source is
`src/reconstructed/Track.h` / `Track.cpp`; every name is provisional.

The Track.cpp range runs from `0x00515dc0` to `0x00518716`. The tyre code
before it ends at `0x00515c90`. TrackOverlay's classes (InstrumentOverlay
and others) start at `0x00518720`, with their own `__FILE__` at `0x00575290`.

## Ownership evidence

| Evidence | Functions |
|---|---|
| `__FILE__` reference (confirmed) | `0x00515e70`, `0x00515ed0`, `0x00516800`, `0x00516870`, `0x00516980`, `0x00517340`, `0x00517930`, `0x005179f0` |
| String pool between the two `__FILE__`s (strong inference) | `0x00518640` (`"%2.2d:%2.2d"`), `0x00518690` (`"--:--.--"`, `"%2d:%05.2f"`) |
| Position only (provisional) | the tokenizer `0x00515dc0`/`0x00515df0`, `0x00516ca0`, `0x00516ef0`, `0x00517310`, `0x005179a0`, `0x00517da0`, `0x00517ea0`, `0x00518080`, `0x00518130`, `0x00518230` |

The tokenizer is called from gameui.cpp, Parameterblocks.cpp and others. It
sits directly before `0x00515e70`, which is the only reason to place it here.

## Data

The structure sizes come from the `DebugCalloc` sizes at the allocation
sites:
- **Node**, 0x18 bytes (line 108). It holds the flags (bit 0: read from the
  file; bit 2: visited by a walk), a length (+0x04), the first and last
  segment (+0x08, +0x0c) and its links (count +0x10, array +0x14).
- **Segment**, 0x30 bytes (line 141). It holds a point (+0x00..+0x08), the
  horizontal distance to the next point (+0x24), and the previous and next
  segment (+0x28, +0x2c). The strip test `0x00516ef0` reads +0x0c..+0x14 and
  +0x18..+0x20 as two more points, the left and right edges (inferred from
  how the test pairs them with the centre point).
- **Work-list entry**, 0x10 bytes. Only +0x04 (node) and +0x0c (next) are used.

A track position (node, segment, t) is passed by value. The loader
`0x00515ed0` stores the start node at Track+0x00 and the length from start
to finish at Track+0x04.

## Status

The following 14 functions are exact under `vc6_o2_mt`, with every
relocation bound:

| VA | Bytes | Body |
|---|---:|---|
| `0x00515dc0` | 46 | tokenizer constructor |
| `0x00515df0` | 118 | tokenizer: next token |
| `0x00515e70` | 89 | frees a node array (the loader's failure path) |
| `0x00516800` | 108 | frees a node, its segments and its links |
| `0x00516870` | 262 | frees every node reachable from the start |
| `0x00517310` | 39 | whether a segment belongs to a node |
| `0x00517930` | 99 | frees a work list |
| `0x005179a0` | 67 | whether b is at or after a on the same node |
| `0x00517da0` | 256 | distance along the track from a to b |
| `0x00517ea0` | 465 | moves a position a distance along a node path |
| `0x00518080` | 174 | the point at a position |
| `0x00518230` | 1033 | horizontal distance from a point to a segment's strip edge (mode 10, 11 or 17: either edge or the nearer) |
| `0x00518640` | 67 | formats whole seconds as `mm:ss` |
| `0x00518690` | 134 | formats a lap time, or `--:--.--` when unset |

Near miss: the segment direction `0x00518130` (246/250 bytes,
`samples/track/TrackNearMisses.cpp`). Retail keeps the inverse length on the
x87 stack until a final `fstp st(0)`; VC6 here consumes it in the last
multiply.

Near miss: the path search `0x005179f0` (929/929 bytes, two differ). Where
the walk reaches b's node, retail loads the two addends of
`length + fromStart` in the other order; no source order tried changes it.

Near miss: the closest position on one node `0x00516ca0` (576/586 bytes).
VC6 here reuses the projection's `p - segment` differences in two of the
clamping branches, where retail recomputes them.

Near miss: the strip test `0x00516ef0` (1042/1042 bytes, 98.18%). It is a
point-in-polygon parity test over six edges of the strip between a segment
and the next. On two of the edges retail multiplies the cross-product
factors in the other order. An inline helper that takes the point by value
and the edge points by pointer gives the other four edges' load order; a
macro over plain floats loses it.

Near miss: the placement search `0x00517340` (1509 bytes, to `0x00517925`;
the block at `0x00517610` is part of it: it jumps back to `0x00517559` and
returns with `ret 0x28`). The candidate is 1533 bytes and follows retail
instruction for instruction, but VC6 here gives the candidate list a frame
slot and assigns the path loop's registers differently. Candidates are
0x10-byte records (Track.cpp line 889): a TrackPos and a next pointer. The
fifth parameter is a work list of nodes, although RaceStatus.cpp's binding
declares it `int`.

The closest-position search `0x00516980` (786 bytes) walks every node
reachable from the start node (the depth-first list of `0x00516870`) with
`0x00516ca0`'s segment loop; its third parameter is the optional distance
output (`float*`; RaceStatus.cpp passes 0). It is a near miss in
`samples/track/TrackNearMisses.cpp` (465 of 776 compared bytes) with the same
blocker as `0x00516ca0`: VC6 reuses the `p - segment` differences in the
t < 0 and t == 0 branches, which retail recomputes.

The loader `0x00515ed0` (2342 bytes) reads a chunk stream (an id byte,
then the chunk's data; ids 1-16 are decoded in
`samples/track/TrackNearMisses.cpp`). It is a draft near miss there (117 of
2400 compared bytes): dispatch, calls and arithmetic follow retail, but the
frame is 0x64 against retail's 0x48 and retail keeps `this` in ebp.

## Source shapes that mattered

- `0x00518230` (called from BikeAI.cpp `0x00415326`/`0x0041535c` with modes
  10 and 11): the edge differences come from Track.cpp's file-static
  inline `operator-`, which returns its result through an inline
  `MakeTrackVec3(x, y, z)` (an aggregate temporary filled in place
  schedules the copy into the direction before the last component is
  stored). The helpers stay in Track.cpp: TrackVec3 constructors or
  operators declared in Track.h shift VC6's operand-order tie-break in TUs
  that include it (TrackGame slot 1 `0x00520ab0`, Scene::ReadLight
  `0x004eb570` and SelectGamePicProcs `0x004f17a0` stop matching). The
  perpendicular is a TrackVec3 local whose `.x`/`.z` are written before the
  `0x0047b800` call (a plain float is pushed directly). The first edge's closest point is `d = d * t; a += d;
  a -= p` component-wise in place; the second edge uses `d2 * t + a2` and
  accumulates its squared distance in the `u` out-parameter local
  (`u = dx * dx; u += dz * dz`), which is why it lives in memory.
- `0x00515df0`: `*p++ = 0; if (*p == 0) p = 0; else while (strchr(...)) p++;`
  gives retail's late `push edi`.
- `0x00516870`: a `while (list)` loop, not do-while.
- `0x00518080`: writing the clamp into the by-value `pos.t`, plus a local
  difference vector.
- `0x00517da0`: a `goto` to the shared path-search call when the segment
  walk runs out, and a second, separate call for different nodes.
- `0x00517ea0`: a by-value copy `TrackPos cur = pos` of the parameter.
  It keeps `cur.t` in memory and the node and segment in registers, while
  the distance travelled stays on the x87 stack. Separate node, segment and
  t locals put t on the x87 stack instead.
- `0x005179f0`: one `TrackPos` local reused for both 0x00517da0 calls (two
  locals take two stack slots).
- `0x00518130`: the squared length must be `(y*y + x*x) + z*z`.
- Boolean values computed with `fcompp` come out in the inverted sense in
  retail: `(lhs <= rhs) == (p.z >= a->z)`, not `(lhs > rhs) != ...`.

## Reproduce

```bash
python tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt \
  --vc6-root "$VC6_ROOT" --exe work/game/mcm2.exe --jobs 8
```

The `# Track.cpp` block at the end of `CASES` in `tools/run_calibration.py`
lists the Track cases.
