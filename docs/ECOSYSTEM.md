# EcoSystem

Source: `D:\aardvark\VC\krusty2\EcoSystem.cpp` (literal `0x0056a130`; 35
`.text` xrefs `0x00455ee6..0x0045c509` and nine in the unit's EH funclets
`0x0054a880..0x0054aa32`). Canonical reconstruction:
`src/reconstructed/EcoSystem.h` / `EcoSystem.cpp` /
`EcoSystem.bindings.json`; near misses in
`samples/ecosystem/EcoSystemNearMisses.cpp`. Names are provisional unless a
literal names them (the .est keys: `NumberOfLOD`, `MeanHeight`, ... ).

## Extent

`0x00455da0..0x0045c823` (strong inference). It opens with the `$E`
pairs of its two `UnknownPeakHold(5000)` statics (`.CRT$XCU` 103/104,
`0x0059aef0` and `0x0059aec8`; the vector set 99-102 sits mid-file at
`0x00459830..0x00459b3b`, as in bikerace.cpp), and its last function is the
xor reader `0x0045c7b0`; EventManager.cpp's constructor follows at
`0x0045c830`. The unit's `.bss` is `0x0059ae90..0x0059af18` (the four
vectors, the EcoSystem instance pointer `0x0059aebc`, the two peak holds,
timings, counters, the view matrix pointer and the detail-band pointer);
`0x0059af18` on belongs to EventManager. Its `.data` statics are
`0x0056a128` (-1) and `0x0056a12c` (1, the drawing switch slot 23 toggles),
right before the string literals, and the two detail-band tables
`0x0056a600` (1555) / `0x0056a740` (4444).

Linear function-start lists show 52 starts in the extent; two are labels
inside bigger functions: `0x004584d0` is inside the "CollisionObject%i"
reader `0x00458360..0x004587aa` (1104 bytes) and `0x00458ae0` inside the
.esb writer `0x004587b0..0x00458d99` (1513 bytes). That leaves 50
functions.

## Classes

- `EcoSystem : GameObject (+0), GroundFogableObject (+0x2c)` (RTTI, vtable
  `0x00552508`, overrides 0/12/14/23); 0x5c4 bytes (QuarryStuntEvent
  `0x004df908` allocates it and calls `0x00457250` then the creator
  `0x004594d0`). Layout from the constructor and the loaders: method (+0x30),
  object count (+0x34), the `Vegetation` array (+0x38), two draw lists
  (+0x3c billboards, +0x40 geometry; counts +0x5a0/+0x5a4, capacities
  +0x5b0/+0x5b4), terrain / texture manager / lights (+0x44..+0x4c), the
  billboard vertex and index buffers (+0x50/+0x54, 120 billboards of six
  vertices), 256 definition pointers (+0x58), NorthAngle (+0x458),
  PlacementBmp / ProbabilityTga names (+0x45c/+0x4dc), light colours and
  direction (+0x564..+0x584), the .esb stream (+0x590, owned unless +0x594),
  the Vegetation type id (+0x599), the AgeManager (+0x59c), the coordinate
  scale and its inverse (+0x5a8/+0x5ac), depth scales (+0x5b8/+0x5bc) and the
  detail level (+0x5c0).
- `Vegetation : QuadTreeObject` (RTTI, vtable `0x005524f8`), 0x1c bytes
  (`0x00459656` allocates count * 0x1c): three 16-bit coordinates (+0x0c, in
  +0x5a8 units), definition index (+0x12), fade (+0x13), height / radius
  parameters (+0x14/+0x15), the billboard bit (+0x16, a bitfield: the stores
  compile to VC6's xor/and/xor sequence) and the geometry block (+0x18).
- The definition (`UnknownEcoDefinition`, no RTTI): 0x210 bytes
  (`operator new` at `0x00457771` and `0x0045908b`), constructor
  `0x00455de0`: name / BillboardName / ProbabilityTga (0x80 each), the
  height and radius triples, the Auto-method statistics, the billboard
  texture rectangle, model scales, NumberOfLOD (at most 1; the per-LOD arrays
  have one entry), key colour, two textures, model vertices / indices /
  counts, UsePlanarLighting, BlendLODs, PercentBias and the collision
  definitions (0x24 bytes each) with their CollisionObjects.
- Detail bands (`UnknownEcoDetailBand`, 0x20 bytes, ten per table): the
  3D and fade distances, model flags, billboard range and limit, and three
  render-state switches.

## Exact (37 calibration cases)

Small: the two peak-hold `$E` pairs and the four vector `$E` pairs; the
definition constructor / destructor and its four parameter helpers
(`0x00455f50` random byte, `0x00455f60` random parameter for a height,
`0x00455f90` / `0x00455ff0` height / radius for a parameter byte); the
Vegetation constructor, slot 0 (view depth as a 16-bit sort key), slot 1
(QuadTree cell code from the radius), the quantised placement
`0x004567a0`, the AgeManager eviction callback `0x00456850`, the
collision-count and radius getters `0x00457080` / `0x00457230`; the
EcoSystem constructor, destructor and deleting destructor, the detail-level
setter `0x004594c0`, the geometry draw `0x00457000` and slot 23.

Medium and large: the lighting update `0x0045a9a0`; slot 12 `0x0045aad0`
(the classification pass: AgeManager trim, view matrix publication, the
sorted QuadTree query, the fade helper, the frustum test and the two draw
lists with their 100/20-entry growth); the render-state setter
`0x0045ade0`; the .esb writer `0x004587b0` (1513 bytes); the .esb reader
`0x00458f70` (1351 bytes); the creator `0x004594d0` (859 bytes: table
selection, scale from the QuadTree extents, load, textures, collisions,
object array, buffers, placement by method, .esb write).

Source forms that mattered:
- The squared length in `0x0045a9a0` only compiles to retail's load order
  as `z*z + (x*x + y*y)` (`UnknownSquareMagnitude`).
- `0x0045ade0`: the alpha reference for the colour-keyed format is
  `software ? 0 : 0xc0` (VC6 encodes `and ecx, 0xffffff40` as `and cl, 0x40`);
  the stage states repeat the identical pair in both arms of an
  `if (format == 0x115c)` so that jump threading reaches the format tail.
- `0x004587b0`: `strlen` once into a local and `n > 0x103 ? 0x103 : n`; the
  per-object definition byte goes through a local before `fwrite`.
- `0x00458f70`: the stream selection is `if (!stream) { if (!entry) new +
  open else entry stream, flag } else flag`.
- `0x004594d0`: the band table is a conditional expression; the index
  buffer is filled through a running index; `unitsPerCoordinate = size *
  (1.0f / 65536.0f)`.

## Near misses (`samples/ecosystem/EcoSystemNearMisses.cpp`)

Scores are matching / compared bytes in the src unit's context.

| VA | Function | State |
|---|---|---|
| `0x004567e0` | world-position placement | 82/112: VC6 orders `position->x * scale` as `fld scale; fmul x`; retail loads x first. Only a by-value float accessor (`position->X()`) reproduces it; the project's `Vector3` has none. |
| `0x00456890` | fade / distance band | 356/369: retail loads the camera pointer before the first `fmul`. |
| `0x00456a10` | geometry build (1512 bytes) | 73/1451: the whole body is reproduced (rotation from the camera direction, vertex transform, two normal paths, lighting, colours, the dead min/max of x); the frame is 0x4c instead of 0x50 and the local slots differ. |
| `0x004570a0` | collision object placement | 74/349: retail does not fold the definition lookup across the position conversions; the radius and height divide by the definition's mean values. |
| `0x00456050` | definition load (.slt) | 195/1496: `this` / `textures` register roles and the local layout (the loop extremes, a/b/c, faces) differ; the frame is 0x1b8 for 0x1b4. |
| `0x00457480` | .est reader (2626 bytes) | 380/2624 with the handler label bound by hand (the matcher does not recognise the `push ebp; mov ebp, esp; and esp, -8; push -1; push handler` prologue, so the case cannot be registered yet); the only other difference is the probe stream kept in `esi` as well as its EH slot. |
| `0x00457ed0` | collision objects (1150 bytes) | 1094/1153: one instruction, the height load `mov edx, [edi+0x20]` scheduled before the cosine in retail. The vector offset needs `b.x + a.x, a.y + b.y, a.z + b.z` (`UnknownEcoOffset`). |
| `0x00458360` | "CollisionObject%i" reader (1104 bytes) | 113/1112: retail keeps 0 in `ebp` (`cmp eax, ebp`, `push ebp`) and tests the count twice; the local arrays are key, value, section, kind in that order. |
| `0x00458da0` | .txt listing | 457/461: four SIB operands are `[esi + eax]` instead of retail's `[eax + esi]` (array base / induction order); no source form found yet. |
| `0x004598d0` | placement from the .esb | 34/462: the loop keeps `i` in memory and the definition byte zero-extended in a register; local layout. |
| `0x00459b40` | placement from the PlacementBmp | 56/412: `fidiv` for the cell size, the bitmap pointer in `ebp`, pixel pointer kept in memory. |
| `0x0045c6a0` | xor fwrite | 14/254: retail walks a pointer and a 1024 count-down per block, with the block count in a local. |
| `0x0045c7b0` | xor fread | 90/115: register roles (total in `edi`, key in `ebx`) and the byte update order. |

Not attempted: the texture preload `0x0045c040` (1619 bytes: reads the .esb
names, seeks the rest and loads every texture), the Auto-method generator
`0x00459ce0` (3257 bytes: PercentBias cumulative table, TGA probability maps,
slope / aspect / drainage / altitude gaussians, QuadTree code checks) and
the billboard renderer `0x0045b136` (3753 bytes: view-aligned quads with
the band's range, two lit colour sets, the "EcoGen"/"QuadTree" debug
counters and the peak holds).

## Evidence notes

- `0x00456850` is registered with the AgeManager as the geometry's eviction
  callback; the AgeEntry lies after the vertices and the 4-byte-padded
  indices in the same block (`dwords * 4 + sizeof(AgeEntry)`).
- The Vegetation type id comes from `TypeRegistry::FindTypeId("Vegetation")`
  (`0x00459644`); slot 12 compares it with each QuadTree object's +0x08.
- The visibility test is `VisibilityClipper::SphereInFrustum`
  (`0x0052fbb0`, `g_visibilityClipper`) on the camera's +0xec matrix.
- `0x00511ad0(textureFormat)` seeds the generator; `0x0047b8a0` (gameui.cpp
  tail) is the float profile reader with a `double` default.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/EcoSystem.cpp -o work/EcoSystem.obj
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x00455f50` `RandomParameter`
- `0x00455f60` `ParameterForHeight`
- `0x00455f90` `HeightForParameter`
- `0x00455ff0` `RadiusForParameter`
- `0x00456050` `LoadModel`
- `0x004567a0` `PlaceQuantized`
- `0x004567e0` `Place`
- `0x00456850` `EvictGeometry`
- `0x00456890` `TestDistance`
- `0x00456a10` `SetBillboard`
- `0x00457000` `DrawGeometry`
- `0x00457080` `GetCollisionCount`
- `0x004570a0` `GetCollisionObject`
- `0x00457230` `GetRadius`
- `0x00457480` `ReadEst`
- `0x00457ed0` `BuildCollisionObjects`
- `0x00458360` `ReadCollisionObjects`
- `0x004587b0` `WriteEsb`
- `0x00458da0` `WriteListing`
- `0x00458f70` `ReadEsb`
- `0x004598d0` `PlaceStoredObjects`
- `0x00459b40` `PlaceAuthoredObjects`
- `0x00459ce0` `GenerateObjects`
- `0x0045ade0` `SetRenderStates`
