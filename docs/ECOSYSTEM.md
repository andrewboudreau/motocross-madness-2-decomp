# EcoSystem

Source: `D:\aardvark\VC\krusty2\EcoSystem.cpp` (literal `0x0056a130`; 35
`.text` xrefs `0x00455ee6..0x0045c509` and nine in the unit's EH funclets
`0x0054a880..0x0054aa32`). Canonical reconstruction:
`src/reconstructed/EcoSystem.h` / `EcoSystem.cpp` /
`EcoSystem.bindings.json`. The near misses sit in the unit in retail order,
each with a `// Not exact` status comment; `0x004570a0` stays in
`samples/ecosystem/VegetationCollisionObject.cpp`. Names are provisional unless a
literal names them (the .est keys: `NumberOfLOD`, `MeanHeight`, ... ).

## Extent

`0x00455da0..0x0045c823` (strong inference). It opens with the `$E`
pairs of its two `PeakHold(5000)` statics (`.CRT$XCU` 103/104,
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

Linear function-start lists show 52 starts in the extent; three are not
functions: `0x004584d0` is inside the "CollisionObject%i" reader
`0x00458360..0x004587ac` (1101 bytes), `0x00458ae0` inside the .esb writer
`0x004587b0..0x00458d99` (1513 bytes), and `0x0045b136` falls in the middle
of an instruction of slot 14, which is one 3967-byte function
`0x0045b060..0x0045bfdf` (the billboard pass is not a separate routine).
That leaves 49 functions.

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
- The definition (`VegetationDefinition`, no RTTI): 0x210 bytes
  (`operator new` at `0x00457771` and `0x0045908b`), constructor
  `0x00455de0`: name / BillboardName / ProbabilityTga (0x80 each), the
  height and radius triples, the Auto-method statistics, the billboard
  texture rectangle, model scales, NumberOfLOD (at most 1; the per-LOD arrays
  have one entry), key colour, two textures, model vertices / indices /
  counts, UsePlanarLighting, BlendLODs, PercentBias and the collision
  definitions (0x24 bytes each) with their CollisionObjects.
- Detail levels (`EcoDetailLevel`, 0x20 bytes, ten per table): the
  3D and fade distances, model flags, billboard range and limit, and three
  render-state switches.

## Exact (45 calibration cases)

Small: the two peak-hold `$E` pairs and the four vector `$E` pairs; the
definition constructor / destructor and its four parameter helpers
(`0x00455f50` random byte, `0x00455f60` random parameter for a height,
`0x00455f90` / `0x00455ff0` height / radius for a parameter byte); the
Vegetation constructor, slot 0 (view depth as a 16-bit sort key), slot 1
(QuadTree cell code from the radius), the quantised placement
`0x004567a0` and the world-position placement `0x004567e0`, the AgeManager eviction callback `0x00456850`, the
collision-count and radius getters `0x00457080` / `0x00457230`; the
EcoSystem constructor, destructor and deleting destructor, the detail-level
setter `0x004594c0`, the geometry draw `0x00457000`, slot 23, the
PlacementBmp placement `0x00459b40` and the two xor stream helpers
`0x0045c6a0` / `0x0045c7b0` (the .esb's fwrite / fread through a running
one-byte key).

Medium and large: the definition load `0x00456050` (1522 bytes: the
billboard texture, the `.slt` model's LOD vertex / face sections through a
parameter block, the x / y extents into the height and radius scales); the
geometry build `0x00456a10` (1512 bytes: the
AgeManager-registered vertex block, rotation from the camera's horizontal
direction, the planar and model-normal lighting paths); the lighting update
`0x0045a9a0`; slot 12 `0x0045aad0`
(the classification pass: AgeManager trim, view matrix publication, the
sorted QuadTree query, the fade helper, the frustum test and the two draw
lists with their 100/20-entry growth); the render-state setter
`0x0045ade0`; the "CollisionObject%i" reader `0x00458360` (1101 bytes);
the .esb writer `0x004587b0` (1513 bytes); the .esb reader
`0x00458f70` (1351 bytes); the creator `0x004594d0` (859 bytes: table
selection, scale from the QuadTree extents, load, textures, collisions,
object array, buffers, placement by method, .esb write); the .esb
placement `0x004598d0`; the texture
preload `0x0045c040` (1619 bytes: the `.est` to `.esb` probe, the archive
entry or a new owned stream, the 256-entry index skim, the per-name
billboard and `.slt` texture loads with the "KeyColorTrees" format).

Source forms that mattered:
- `0x00456050`: the path buffer must not shadow the `name` member (the
  shadowing local was the 163/1513 near miss); the extremes are initialised
  in the order maxX, minX, maxY, minY; every vertex field is addressed as
  `modelVertices[lod][i].field` (retail re-reads the LOD pointer per
  access); and the three face ints are declared at the LOD-loop scope, not
  inside the face loop. At face-loop scope VC6 hoists the three 16-bit
  loads above the first `*index++` store; at LOD-loop scope it keeps
  retail's load / store / load / add order (the address-taken ints then
  outlive the stores, so the stores are treated as possible aliases). The
  manifest's 1513 bytes stop before the `add esp; ret 8` epilogue; the
  function is 1522 bytes.
- `0x004567e0` loads each position component before the scale
  (`fld [position]; fmul [scale]`) only when the components are read
  through by-value accessors (`VectorX(*position)`); with `position->x` VC6
  loads the scale first in either written order. KrustyBikeCamera and
  BikeCamera slot 34 need the same accessor for one dot-product term
  ([FOLLOW_CAMERA](FOLLOW_CAMERA.md)).
- The squared length in `0x0045a9a0` only compiles to retail's load order
  as `z*z + (x*x + y*y)` (`EcoSquareMagnitude`).
- `0x00456a10`: the light colour and ambient are six float locals (two
  `Vector3`s are placed as aggregates at the top of the frame; retail
  interleaves their components with the other scalar slots); the planar
  length is `x*x` then `+= z*z`; the quantized x goes through a float local
  (that is what orders the zero register before the global reload); the 3D
  length is `EcoSquareMagnitude` and the light dot product sums as
  `z + (x + y)`; the output vertex is indexed off the re-read
  `geometryBlock` member.
- `0x0045ade0`: the alpha reference for the colour-keyed format is
  `software ? 0 : 0xc0` (VC6 encodes `and ecx, 0xffffff40` as `and cl, 0x40`);
  the stage states repeat the identical pair in both arms of an
  `if (format == 0x115c)` so that jump threading reaches the format tail.
- `0x004587b0`: `strlen` once into a local and `n > 0x103 ? 0x103 : n`; the
  per-object definition byte goes through a local before `fwrite`.
- `0x00458f70`: the stream selection is `if (!stream) { if (!entry) new +
  open else entry stream, flag } else flag`.
- `0x0045c040`: the index read (stream selection, the 256-entry skim,
  the texture loads) is a block of its own in which `collisionCount` is
  declared. VC6 gives an address-taken local the lifetime of its scope, so
  a function-scope `collisionCount` can share nothing, while the block
  keeps it dead in the probe block (its slot then holds the probe `new`'s
  EH temporary, `[esp+0x20]`) and live across the stream's `new` (whose
  temporary takes `ownsStream`'s dead slot) and the second loop (whose
  induction spill gets its own slot). Declaring it mid-function at
  function scope changes nothing: only block scope shortens the lifetime.
- `0x004594d0`: the band table is a conditional expression; the index
  buffer is filled through a running index; `unitsPerCoordinate = size *
  (1.0f / 65536.0f)`.
- `0x00458360`: the loop is zero-based with `i + 1` in every `sprintf`
  (retail keeps a separate `i + 1` induction register next to the index);
  the five `GetPrivateProfileString` defaults inside the loop are the
  shared `""` literal `0x00577738`, not the "NONE" the .est reader uses
  (binding `??_C@_00A@?$AA@`). The retail extent is 1101 bytes plus three
  `nop`s of padding.
- `0x004598d0`: the definition byte is widened to `int` right after its read
  (`int definition = definitionByte;` hoists the zero-extension into a
  register and spills the loop counter as retail does), and the
  `ComputeCode` arguments carry redundant parentheses
  (`ComputeCode((x - radius), (z - radius), (x + radius), (z + radius))`):
  with at least three of the four parenthesised (any three, or all four)
  VC6 issues the `mov ecx, [g_collisionQuadTree]` between the last
  argument's `fsub` and `fstp`; with none, one or two it comes before the
  `fsub` (445/455). A `tree` local for the QuadTree scores 373/455.
- `0x00459b40`: `float cellX = extent; cellX /= bitmap->infoHeader.width;`
  gives retail's `fld [extent]; fidiv [slot]` (the `a / b` spelling
  converts first and emits `fild; fdivr`); both conversions share one slot
  that the row counter then reuses. The pixel pointer is read before the
  divisions, and both loop bounds re-read the bitmap header (a local copy
  would be spilled to its own slot).
- `0x0045c6a0`: the block loop's bound is the expression `block < total /
  0x400` (the quotient stays in a frame slot), the body is a pointer walk
  `c = *data++; c ^= *key; *key += c; *out++ = c;` (the data byte is
  loaded before the key), and the tail loop counts down from the remainder.
- `0x0045c7b0`: the decoded byte is computed into a temporary before the
  key accumulates the encoded byte (`v = *data; x = *key ^ v; *key += v;
  *data = x;`): that is what gives `mov cl, al; add al, dl; xor cl, dl`
  with the key pointer in `ebx`.

## Near misses

Scores are matching / compared bytes with every relocation bound (the
unit's bindings file; `$ehhandler` keys for the EH prologues). All but
`0x004570a0` are compiled inside `EcoSystem.cpp`, so the game build links
the same bodies these scores describe.

| VA | Function | State |
|---|---|---|
| `0x00456890` | fade / distance band | 356/369: retail schedules the camera pointer load and the z store before the first `fmul`. The block is scheduling-invariant: eighteen data-flow-equivalent spellings (locals before or after the camera load, no camera local, the view in a local, one declaration per statement, a position reference, the products computed before the camera, `-=`, int locals and casts, a `Vector3` position, z first, a delta vector) give the same bytes, and every statement reordering scores lower. Helper boundaries do not move it either: inline and static accessors for the eye and the scaled coordinate, pointer, reference and by-value helpers, a struct copy and the difference as a `Vector3` all give the identical 356 or less, and `/G6` scores 267. Both products' operand orders, a `(float)` cast and `-(product) + camera` differences (36 forms) give 356 as well. Block-length ruler (2026-11-28): 1..16 dead `g_probe[i] = i;` stores after the definition load all make it worse (14 differing instructions after normalisation against 2), so retail's block is not longer there. |
| `0x004570a0` | collision object placement | 103/386: retail loads the definition-table entry once into `edi` before the position conversions and keeps it (the candidate re-reads the table per use), and picks other registers for the three shape store blocks. Writing the shape stores through per-statement casts of `object->shapeData` (`ECO_SPHERE->...`) reproduces retail's re-read of the shape pointer for every store, and the hull copy stores y before x. |
| `0x00457480` | .est reader (2629 bytes) | 561/2629 compiled in the unit (587/2648 in the sample, whose out-of-unit definition constructor adds an EH state the unit does not need), three instructions short of exact: VC6 keeps the constant 1 of `method = 1` and `i = 1` in `edi` where retail stores both immediates. The probe stream lives in `esi` as well as its EH slot only with a plain `delete probe;` (the `if (probe)` guard keeps it in memory), which also puts `total`, `i` and the loop pointer in retail's slots; the "VTop" key is read into the field the constructor and `WriteEsb` call `uCenter` and "UCenter" into `vTop` (retail's order). Forms that leave the constant register: `total` / `i` at function or block scope, `i = 1` before or after `total = 0.0f`, `for (int i = 1 ...)`, `float total = 0.0f` at the declaration, an `if / else { }` method chain, `? 1 : 0` flag conversions, nested `if`s and named temporaries for the `strstr` / `_stricmp` / open results; also `unsigned int method` in the class, `long i` / `unsigned int i`, `method = true`, an `else { if (...) return 0; method = 2; }` chain and a separate `int j` for the first loop (all 561/2629 with the same three instructions; the `j` loop scores lower). The constant's registerization is not driven by the variables' types or the statement shape. The manifest's 2646 bytes include eleven `nop`s of padding. |
| `0x00457ed0` | collision objects (1150 bytes) | 1105/1153: the cylinder height load `mov edx, [edi+0x20]` is scheduled before the cosine in retail (every placement of the `.y` store scores lower). The offset is `operator+` shaped (`Vector3(a.x + b.x, ...)`, `EcoSum(shape->start, vertices[j])`; the named-result form scores less); its z component is summed vertex-first as in retail only when the vertex's z is read through a by-value accessor. Retail reads the height right after the `fild` of the angle and `fmul`s the step constant after the `cmp ecx, 0x10`, but stores it through an integer register after the sine; here the load sits right before its store. Local-array store order follows statement order: the `.y` store first (816/1156), `vertices[j + 16].y = 0.0f` first (817/1156), between x and z (1103/1153), and a `float height = shape->height` local (840/1163, goes through the FPU) all score lower. |
| `0x00458da0` | .txt listing | 457/461: four SIB operands are `[esi + eax]` instead of retail's `[eax + esi]` (array base / induction order); no source form found yet (`i[vegetation]`, an `int index = i` copy and `(vegetation + i)->` compile to the same bytes; a `Vegetation* object = &vegetation[i]` local scores 145/461). |
| `0x00459ce0` | Auto-method generator (3257 bytes) | 366/3257 in the unit. The definition TGA is weighted in retail's order, `(channel * probability) * 1/255` (see the source comment: at 24-bit precision the other order differs by one ulp for 20333 of 65025 pairs); the earlier sample form, with the other order, scored 323/3300. The control flow and every expression match; the frame (0xe14) is the same size and the arrays (`tgas`, `cumulative`, the rings) sit at retail's offsets, but the scalar slots differ. Retail's slot order by use count is reproduced except for how VC6's slot sharing groups the temporaries: retail shares the int-to-float conversion temporaries (`fild [esp+0x10]`) with the first loops' 256 count-down and the placement loop's `j` (34 uses at `[esp+0x10]`), `i` of the ring loops with `probability` (28 uses at `+0x14`), and the `fimul` products with their `(int)` results (`+0x28`, 12 uses); here the conversion temporaries share with the `fimul` products (36 uses), `i` with `clusterCount` (27) and `probability` stands alone (11), so `tga`, `heightParameter`, `radius` / `threshold` and the four-use scalars shift by one or two slots. Source forms that mattered: the two `rand()` scales stay separate only through a named local (`random = EcoRandomUnit(); threshold = random * 3.0f`); the draw is `rand() * (100.0f / 32767.0f)` (RAND_MAX), the position `rand() * (1 / 32768.0f) * range`; the TGA sampler divides by a local copy of the QuadTree extent (`x / worldX * width` gives retail's `fld; fdivr`); the ring tables are `Vector3(i * spacing, 0, (mode - 1) * spacing)` constructor temporaries; `tga = 0` precedes the `memset`; the registry query is Game slot 20, not slot 22. |
| `0x0045b060` | slot 14, the draw (3967 bytes) | 3714/3967, same size and frame (1024/1045 instructions aligned). Logic fix: the billboard width scale is `(radius + radius) / (uRight - uLeft)` (retail's `fdivp st(1), st` divides the doubled radius by the span); the earlier sample divided the other way and drew near-zero-width billboards. Shape levers: `vTop` / `uLeft` / `uRight` read into locals in that order (vertex count in `ebp`, `definition` in memory, `edi` / `ebp` / `ebx` popped before the overlay rows as retail does); the alpha test as `isBillboard >= lodCount`; `next++` in the for increment; the page store through a local (`int page = overlay->nextPage++; s_debugPage = page;`); and the lean as `EcoLean(&side, EcoScaleXZ(back, 0.1f))` per side, a returned vector passed by value, which reproduces retail's duplicated x product, the twice-copied z temporary and the 0xf0 frame (a named `Vector3` lean, `back * 0.1f`, float or reference helpers and `-=` keep the lean in registers). The clamps are conditional expressions, the intensities three floats, the dot products `z + (x + y)`. Left: the first loop guard compares `remaining` with the zero register before the offset stores, x87 scheduling in the lit-face block (`+0x8d4`..`+0x931`) and the alpha / leftColor load order at `+0xc1c`. |

With the three large functions decoded there are no unattempted functions
left in the unit.

### Forms tried

The list kept with the near misses while they lived in a separate sample
(Git history has the file). Session numbers refer to earlier matching rounds.

```text
  0x00456890  the camera pointer and the z store are scheduled before the
              first fmul in retail (13 bytes); the block compiles the same
              under every data-flow-equivalent spelling tried (locals
              before/after the camera load, no camera local, the view as
              a local, one declaration per statement, a position
              reference, the products before the camera, `-=`, int
              locals/casts, a Vector3 for the position, z first, a delta
              vector), and every statement reordering only moves further
              away; helper boundaries do not move it either (inline and
              static accessors for the eye and the scaled coordinate,
              pointer/reference/by-value helpers, a struct copy, the
              difference as a Vector3: identical bytes or worse), and
              /G6 scores lower (267); redundant parentheses on the
              differences, the products and the detail-band casts (225
              combinations) do not move it either (356/369)
              nor 1..33 prepended typedefs (session 14, all 356);
              session 17: all 40 valid orders of the definition, camera,
              dx, dz and `*billboard = 0` statements, a render-target
              local, `-(q * u) + eye` and `this->` reads stay at 356 or below
  0x004570a0  retail loads the definition-table entry once into edi before
              the position conversions and picks other registers for the
              shape store blocks (103/386); retail reloads the index
              byte at +0x12 for the definition while keeping the global
              in eax, VC6 here reuses the first table entry. A `table`
              local, direct reads, `(int)` casts, a `first` definition
              local and an inline definition accessor for either read
              (12 forms) all compile to one load
  0x00457480  three instructions: VC6 keeps the constant 1 of `method = 1`
              and `i = 1` in edi where retail stores immediates (561/2629
              compiled inside the unit; the sample TU adds an EH state for
              the definition `new` because the ctor is out of unit).
              `delete probe;` without the `if` keeps the probe in esi as
              retail does; "VTop" is read into uCenter and "UCenter" into
              vTop (retail's order); scope/order/if-shape variants of
              total, i and method did not move the constant, nor did
              unsigned/long types, `true`, a nested else chain or a
              separate loop variable, nor did `for (i = 0; ++i < 256;)`
              or a `while` loop (s14)
  0x00457ed0  one scheduled load (the cylinder height) in the vertex loop
              (1105/1153): retail reads it right after the fild, before
              the `j` spill store; parentheses on the angle, the cos/sin
              casts and products and the height (216 combinations), a
              `(float)j`, every order of the five loop statements and
              split cos/sin locals give 1105 or (mostly much) worse.
              s22: retail holds the height in edx from the top of the
              body to the store after the z stores, so it is read early
              and carried in an integer register. A float local read at
              the top of the body (block or function scope, register,
              const, a one-member struct, a Vector3 member or a
              one-element array) is carried on the x87 stack instead
              (`fld` before the fild, `fstp` for the store; same size);
              read after the angle it spills the angle (10 bytes
              longer). Inline helpers do the same or worse: a y setter
              (reference or pointer), a top/bottom y setter, a rim
              helper taking the angle, the radius or the shape and the
              height by value in every parameter order (48 forms), one
              taking x, y and z by value in every order (24 forms; the
              height then goes through an integer register and a frame
              slot, 10 to 22 bytes longer), Vector3 constructor forms
              (62 bytes longer) and a height accessor. An int-typed
              carrier (evidence only) reproduces this loop but flips
              `start.x + vertices[j].x` in the next loop, so the two
              loops are coupled through the age of their leaves.
              2026-11-14: that flip is a local-definition count. The
              int carrier (`int h = *(int*)&shape->height;` first in
              the body, `*(int*)&vertices[j].y = h;` for the store)
              with k more dead initialised int locals at function
              scope is byte exact for k = 2..7 and has the flipped
              x sum for k = 0, 1 and 8; the spelling of the sum (EcoSum
              either way round, a plain constructor sum in four operand
              orders) does not matter under the carrier. So retail's
              body has the height in an integer temporary before the
              angle and at least two more local definitions than this
              source. Evidence only, not applied: no readable form
              gives the integer temporary. Also measured: the y store
              written first or second (the store is not sunk and the
              induction pointer changes base, 3 bytes longer), a
              `const float&` height, a Vector3 local's y, a double
              angle with a float height (x87 carry again), two Vector3
              locals copied out (46 bytes longer). Stores placed before
              the loop do not move the schedule (the loop body is its
              own block), so this is not the scheduling-window effect
              of the CollisionObject family.
  0x00458da0  four `[eax + esi]` operands come out as `[esi + eax]`
              (457/461). Also unchanged: inline helpers for the
              terminator, the clamped copy, the length clamp or the
              extension; the name head in its own block; `file` and `i`
              declared at the top or at first use (a single reassigned
              `length` local gives 442).
              The `name[length] = 0` byte store decides it:
              without it (or without the whole name head) the loop gets
              retail's `[veg + i]` order, and each further byte store at
              a variable index toggles it back (parity-like; int/short/
              float stores, constant-index stores and loads do not).
              No code-neutral form moved it (~60 tried): loop shapes
              (for/while, pre-increment, unsigned i: 455), `this->`,
              dead/top/loop-scope locals, parentheses on the coordinate
              products (27 combinations) and on the zero store's index,
              array and value, `'\0'`/`(char)0`/`*(name + length)`/
              `length[name]`/end-pointer spellings, clamp spellings
              (if/ternary, <, <=, >=), `sizeof`, pointer arithmetic, a
              reference, split file/strrchr forms, and 0..40 prepended
              typedefs (no arena window). Also unchanged: a `char*`
              alias of `name` in any subset of the four head uses
              and `(vegetation + i)->`; an element pointer/reference
              (180), a vegetation base local (205) and index or
              byte-offset element helpers (257) are far worse. Session
              15, also 457: the discarded RadiusForParameter call (or
              both discarded calls) as a bare statement instead of a
              named local, and `i` declared in the for statement
              Session 18: the toggle is per function, not per TU
              (one to three variable-index byte stores in a function
              placed before this one: 457). A variable-index byte read
              that is used (`if (name[length]) return;`) flips the
              order like a store; discarded reads (`(void)name[length]`,
              a dead char local, `if (0) name[length] = 1;`) are
              optimized away and stay 457, as do an `unsigned char`/
              `signed char` cast on the store, an unsigned `length`, a
              dead `name[length] = 1;` before the store and an
              end-pointer spelling. `name[0x103] = 0;` after the
              extension copy (73-129) and memcpy for strncpy (41) lose.
              Round 10: the toggle is not the byte access. Any test of
              a non-const local holding a known value, written before
              the loop, gives retail's 461/461: VC6 folds it late
              (constant propagation in the optimiser) and emits the
              same instructions with the address sums the other way
              round. `if (name[length]) return;` after the store is
              one such test (store forwarded, test folded). Others, all
              461: `int flag = 1; if (!flag) return;` at the top, after
              the name head, after the fopen guard or after the count
              line; `if (!file || !flag) return;`; `if (flag)
              FilePrintf(count)`; `fopen(name, flag ? "a" : "w")` with
              flag 0; `i = 0; if (i == 0) FilePrintf(count)`; an error
              flag set from `!file`. None is applied: each is dead code
              with no evidence for its wording, so the row stays 457
              until retail's actual test is identified. Tests the front
              end folds do not flip (457): literal `if (1)`, const or
              enum flags at file or function scope, a constant argument
              of an inline head helper (`if (size <= 0) return;`), an
              inline helper returning 1, a repeated `if (!file)`,
              `if (file)` wrappers, `do {} while (0)`, empty ifs, a
              `max` local in the clamp. A folded test inside or after
              the loop does not flip either, and one that emits code
              (`if (!path) return;`, `if (placedCount > 0)` around the
              loop: 452) flips the order but changes bytes. Also 457:
              `i = 0` or `file = 0` early (five places each), a `type`
              local for the definition index, `while`/`do` loops;
              `int length = 0x103; if (pathLength <= length) ..` is 47.
  0x00459ce0  the frame slots: same size, the locals and temporaries
              are assigned in a different order (323/3300); retail
              keeps the first sample's TGA in ebx and reloads
              g_collisionQuadTree after its first __ftol, where VC6
              here caches the tree in ebx (the s15 reference diff's
              missing load)
              Big-gap pass, first divergences in order (not applied:
              this object is pinned by six fixtures and the gains are
              small):
              0xaa  retail zeroes `attempt` (frame slot 0x6c, the
                    loop's `inc edi`) between the argument pushes of
                    GetHeightRange, so `attempt = 0` precedes that
                    call in the source; moved there the store is
                    retail's (331 strict, 696 aligned; `failures`
                    moved instead scores the same but is slot 0x54,
                    zeroed after the call in retail; total first 267).
              0x173 retail's mode 5 block ends `jmp` to the
                    `test ebx, ebx` of `if (mode)` and the mode 6
                    block falls into it; VC6 here threads both past
                    the test (one extra jmp, so the equal size is a
                    coincidence). A switch (sub/dec dispatch, 888
                    strict only by position), two separate ifs,
                    `!= 0`, `> 0`, a goto around the ring block, k
                    declared or zeroed earlier and kept zeroed inside
                    do not stop it; `int k = 0` hoisted out of the if
                    does (712 aligned, 366 strict, 3298 bytes) but
                    emits its xor before the test, where retail has
                    it after.
              0x3ef retail keeps attempt in edi and clusterCount in
                    eax across the draw; here both live in the frame.
                    Six orders of the clusterCount/cluster/position.y
                    initialisations: 310-381 strict, 684-696 aligned.
  0x0045b060  3714/3967, same size and frame (1024/1045 aligned). Logic
              fix (session 16): the billboard width scale is
              2r / (uRight - uLeft); the sample had the quotient upside
              down (retail's fdivp st(1),st divides the doubled radius
              by the span), which drew near-zero-width billboards.
              Shape levers found the same session: vTop/uLeft/uRight
              read into locals in that order (flips the allocation:
              vertexCount in ebp, `definition` in memory, edi/ebp/ebx
              popped before the overlay rows as in retail); the alpha
              test as `isBillboard >= lodCount` (0xff000000 first);
              `next++` in the for increment; verticesBefore, camera,
              trianglesBefore declaration order; the page store through
              a local; and the lean as EcoLean(&side,
              EcoScaleXZ(back, 0.1f)) per side, a returned
              vector passed by value, which gives retail's duplicated
              x product and the twice-copied z temporary (and the
              0xf0 frame). Remaining: the first loop guard compares
              `remaining` with the zero register before the offset
              stores (ours tests it after them: `i = 0` before or after
              the zero locals, `i = vertexCount`, a chained zeroing,
              `remaining > i` and `if (i >= 120) break` do not move it),
              x87 scheduling in the lit-face block (+0x8d4..+0x931) and
              the alpha|leftColor load order at +0xc1c. Ruled out for
              the lean: float/reference helpers, a named Vector3 lean
              (with or without a copy), `back * 0.1f`, `-=`, `0.1f * x`
              and a double factor. Other permutations of the three
              field locals, dropping uRight or vTop and removing the
              offset1/2/3/5 locals are all worse.
k-scan (s22): compiled behind 0..63 prepended typedefs, 0x00456890,
0x004570a0, 0x00457480, 0x00457ed0, 0x00458da0, 0x00459ce0, 0x0045b060
have the same code at every count, so their differences are not a
declaration-count effect (docs/VC6_OPERAND_ORDER.md section 2).
```

## Evidence notes

- Slot 14 reads the camera's +0x198 as the projection scale of the
  billboard size test (`radius * scale / depth > 2`), saves and restores
  render states 4 and 9 through the view's slots 9 / 8, and prints its rows
  with the overlay's `0x00447fa0` / `0x00447f40` on the page `0x0056a128`
  takes from the overlay's +0x26c0; the "Memory %d" row is
  `MemTagStack::GetBytes("EcoSystem")`.
- The generator weighs slope and altitude with the tabulated bell curve
  `0x0056ece0` (`NormalDistribution::Lookup(value, mean, sigma)`,
  `0x004b0070`; src/krusty2/effects), reads the "EcoGen" registry integer
  through Game slot 20 (5 and 6 select the 5x5 / 6x6 rings), and samples
  the probability TGAs' green channel (bits 5..9 of a 16-bit pixel).

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
- `0x0045b060` slot 14 (the draw; `UnknownVirtualSlot14`)
- `0x0045c040` `PreloadEcosystem` (the texture preload, cdecl)
