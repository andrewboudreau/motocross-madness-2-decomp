# BikeAI.cpp

`src/krusty2/vehicle/BikeAI.cpp` reconstructs `D:\aardvark\VC\krusty2\BikeAI.cpp`
(`__FILE__` string `0x005678d4`; xrefs `0x00414847`, `0x0041486f`, `0x00416c36`).
Extent (strong inference): `0x0040d070..0x00416e1f`; the unit's header comment gives
the evidence. Every function sits in retail order.

| Address | Function | Status |
| --- | --- | --- |
| `0x0040d070..0x0040d1f0` | static constructors, filter, `SafeNormalize` | exact |
| `0x0040d200` | `PredictPathMotion` (jump prediction along the path) | exact |
| `0x0040e370` | `PredictLanding` (ballistic landing) | exact |
| `0x0040e510` | `PathSteeringDirection` | exact (1935 of 1935 code bytes) |
| `0x0040eca0` | `SimulateLookAhead` (two-wheel look-ahead) | 1479 of 17585 |
| `0x00413160` | `Vec3::operator-=` (COMDAT emitted after `0x0040eca0`) | exact |
| `0x00413190` | `Vec3Magnitude` | exact |
| `0x00413200` | `KrustyBike::SimulateAIFrame` | 2153 of 4112 |
| `0x00414210..0x00414360` | Math3D vector constants and their atexit stubs | exact |
| `0x00414370` | `KrustyBike::UpdateAIRacingLine` | 782 of 4802 |
| `0x00415640` | `QuerySegmentObstacles` | 6063 of 6112 |

`0x00413200` and `0x00414370` are KrustyBike members placed in this unit's code:
the first reads and writes the file-static filter `0x00577ac0`, the second holds the
unit's `__FILE__` xrefs. Names are provisional (tier 3).

## Behaviour before bytes

The four near misses were kept in two copies until now: a matching sample
(`samples/physics/bikeai/BikeAINearMisses.cpp`) and the game build's linked copy
(`src/bringup/Group3_KrustyBikeAI.cpp`), whose bodies follow retail behaviour. The unit
takes the linked copy's forms where the two disagreed in behaviour:

* `0x0040eca0`: every dot product is `BikeAILookDot`, retail's `(x + y) + z`
  grouping. The sample's Math3D `DotProduct` sums `(x + z) + y`, which rounds
  differently when every add rounds to 24 bits (`tools/emu_cases/kb_look_ahead.py` at
  control word `0x007f`: 139 of 200 runs differed). The sample form scored 1542 strict;
  the unit's form scores 1479. `BikeAILookDot` is an `inline` function: its two
  out-of-line calls are retail's `0x0040ae30` (same body as `Vec3DotCall`).
* `0x00413200`: the force smoother takes `(smoother, target, dt)` and reads the time
  constant first, so the target is spilled to a float slot as retail does. With
  `(dt, target)` it stayed in a register and `smoothedForwardAccel` was one ulp off
  (`tools/emu_cases/kb_ai_step.py`: 20 of 100 cases at `0x027f`). This form also scores
  higher (2153 against 2044).
* `0x00414370`: the linked copy's body. It declares the trick delay as a plain
  uninitialised `float` next to `int difficulty`; VC6 gives both the same frame slot,
  so outside free roam the delay reads the difficulty's bits as retail does
  (`kb_ai_racing_line`). The sample spelled that out with a union, which behaves the
  same but scores 775 in this body; the plain locals score 782 and are the more
  likely original (an uninitialised read, not a deliberate pun). The kind 1/5 stores go
  through a `path` pointer, and the point loops are `for` loops with a leading count
  break. The sample's body scored 645.

The vegetation patches in `0x00415640` are called through `Vegetation`
(`src/reconstructed/Vegetation.h`, `0x00457080`/`0x004570a0`) directly; the
quadtree object is the vegetation's first base. The call views of the out-of-line
Vec3 constructor (`BikeAIVec3Call`, `0x00404e60`) and of `operator*(const Vec3&,
float)` (`BikeAIVec3Scale`, `0x005015b0`) stay declarations; `config/game_build.json`
links them to the real definitions, and the five obstacle debug segments to
`BikeRaceDebugState.cpp`'s `Vector3` arrays.

Emu cases, all against the unit (`tools/emu_diff.py CASE`, 400 runs each):
`kb_ai_racing_line`, `kb_ai_step`, `kb_look_ahead`, `kb_path_steering` and
`kb_query_segment_obstacles` 0 mismatches; `kb_simulate_look_ahead` 1, the one run in
which retail itself faults (the former bring-up gave the same).

```bash
python tools/run_physics_samples.py --strict   --source src/krusty2/vehicle/BikeAI.cpp   --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

## Forms tried

### 0x0040e510 (exact)

0x0040e510 (AI path steering direction): EXACT, 1935/1935 code bytes.  Retail's
1936th byte (0x0040ec9f) is the nop that aligns 0x0040eca0, not part of the function.
What it took, in the order found:
 * The walk along the path uses its own index (`index = best`): with one variable
   for the nearest search and the walk, VC6 swaps esi/edi against retail (path in
   esi, index in edi).
 * The three perpendiculars are built component-wise (`perp.x = seg.z; perp.y = 0;
   perp.z = -seg.x; perp = SafeNormalize(perp)`) as in 0x0040d200, and one `perp` at
   the enclosing scope serves both intersection blocks and the final shift (retail's
   +0x14; block-local perps 1865, hoisting one 1872).
 * The final step is written per component with the parameter assigned in the first
   product (`step.x = (t = ahead / len + bestT) * seg.x; step.y = t * seg.y; ...`):
   retail keeps the sum on the FPU stack for x (`fst t; fmul x`) and reloads t for y
   and z.  `t = ahead / len + bestT; target = path->points[index] + seg * t;`
   compiles to the same bytes; `seg * (t = ...)` keeps t on the stack for all three.
 * ONE segment vector for the whole function (1884 -> 1925).  Retail keeps the
   single-point tail's flattened direction (0x0040ebdc..) in the slot of the walk's
   segment (+0x20) and the operator- temporaries in the next one (+0x2c); a separate
   `Vec3 toFirst` in the tail is packed into the dead `pos` argument instead and
   leaves `seg` three references short, so VC6's use-density sort swaps the two
   slots.  Declaring `seg` at function scope and reusing it in the tail gives
   retail's frame everywhere.  This is the same lever as SoultreeObject::SetAxesIn
   (0x004fc050): two named struct values with disjoint lives in one retail slot are
   one reused variable.  Diagnostic that pointed at it: three extra reads of `seg`
   after the walk flip the layout to retail's, one does not.
 * The walk's segment length sums `((x*x) + (y*y)) + z*z` (BikeAISegmentLength,
   1925 -> 1933): with the frame right, the redundant parentheses around y*y make
   VC6 load the y product before the x product, as retail does at both walk sites
   (`(x*x + (y*y)) + z*z` does the same; `((x*x) + y*y) + z*z` loads x first, the
   flat sum and BikeAILength lose 13-31 bytes).  0x0040d200 is unaffected (its
   seg.y is 0 and the term folds) and stays exact.
 * `seg.y = 0.0f` after each neighbour-segment difference (1933 -> 1935): the
   segment is flattened before the 2D perpendicular and intersection, as the tail
   and 0x0040d200 do.  The store is dead (IntersectLines reads x and z only) and
   emits nothing, but it creates seg.y's x87 leaf before `t`'s (first referenced as
   `&t` in the same block), so the step's y product loads t first like its z product
   (docs/VC6_OPERAND_ORDER.md 1.2).  The first block's store alone is enough; the
   second is kept for symmetry.  `perp.y = seg.y * 0.0f` (VC6 folds the product) and
   `seg.y = 0.0f; perp.y = seg.y;` give the same bytes.  Evidence tier: the store
   is an inference from the leaf order, not something retail's bytes show directly.
Forms that did not move the y product or the frame before these (all 1884): operand
order and store order of the step rows, the step declaration's position, a dead
`seg.y = 0` after the walk's differences, component-wise differences (worse), walk
lengths written out in the body, by-value or by-pointer length helpers, `seg` read
through a reference, member operators, a separate walk `dir`, block-local segments,
all 24 declaration orders of the Vec3 locals, the nearest-point loop reusing `seg`
(identical code), `Vec3 seg(0.0f, 0.0f, 0.0f)`; block-scoped `t` locals lose 630
bytes; the single-point tail written first lays the code out differently (120).
`perp * side` through an inline with a named result gives the final sum's slots but
swaps its operands (1888 before the reuse).  Declaring bestT before best fixed the
immediate store of bestT = 0.

The inline length helper used here has the same body as the out-of-line copy at
0x00413190 (matched in the canonical file as a plain function).  Retail's
0x00413190 sits after 0x0040eca0, which is where VC6 emitted the COMDAT when that
17 KB function ran out of inline budget, so in the original source it is very
likely one inline function used in both places.

### 0x00415640

0x00415640 (6111 bytes plus one byte of nop padding, obstacle query along a
segment): 6063/6112 strict positions.  Control flow, calls, constants, every
instruction's schedule and the 0x1ec frame size match.  Left: the order of the
one-use operator temporaries above +0x100 in the frame (offset, the dn * k / dn * 10
/ dn * push temporaries of both owner branches, the segment-length temporaries, the
vegetation position and the outPoint temporary): retail keeps the vegetation `p`
alone and lets the outPoint temporary share the step / normalise slots; here `p`
shares the early normalise temporaries.  The start-up zero vectors are a local
cleared with a chained `zero.x = zero.y = zero.z = 0.0f` (stores z, y, x like
retail; 6059 -> 6063 over `Vec3(0, 0, 0)`, which stores x first).
Tried without effect on those: `k * d` / `10.0f * d` / `push * d` in each of the
six update statements (64 combinations, identical code), parenthesised, integer,
named-local, per-block local and negated zero vectors; `p` and the outPoint
temporary at function scope instead of block scope, a named `v` in the segment-
length blocks and an inline `*outPush += BikeAIVec3Call(...)` instead of the named
step (32-variant sweep: function-scope `p` 6051, function-scope outPoint
temporary worse, the others identical).
0..17 dead initialised int locals at the top change nothing; nor do the single-value
`Vec3(0.0f)` zero (same z, y, x stores as the chained form) or block versus function
scope for the `p` and the operator temporaries' named copies (session 14).
Frame symbols (/FAs): retail orders the address-taken block locals `tmp`, `v`, `v`,
`p` where VC6 gives `p`, `v`, `v`, `tmp`, and the six one-use `d * k`/`d * 10`/`d * push`
results plus `offset` come out in another permutation.  One at a time (session 14,
after RunSteps): `p` in the vegetation loop body, its hit block, the vegetation block,
the quadtree block or before the object loop, the outPush step or one `zero` at
function scope, and either `v` there, all leave 6063; both `v`s at function scope give
5919, both `zero`s 5924, `tmp` at function or quadtree-block scope 6005.
Source forms that mattered: unsigned char type-id statics with an unsigned compare
(`mov bl, 0xff`), the max/min helpers with by-value arguments (two spellings,
see BikeAIMaxR), the squared distance and k dot product grouped z + ((x) + y)
(BikeAISquareMagnitude, BikeAIDot3), SquareMagnitude for the unnormalised distance, hoisting s/u, d,
distSq and normal out of the owner branches (shared slots), a named `step` for the
vegetation push, and the call views below for the dot product, the constructor
and operator*(Vec3, float) that retail calls even in this function's first lines.

### 0x0040eca0

0x0040eca0 (17585 bytes, look-ahead simulation of the two wheels): the whole
function is reconstructed; frame 0x968 vs 0x93c (see the frame notes below).  The call sequence matches retail call for call except one
site (0x00412e23: retail inlines operator+ with the out-of-line constructor, the
candidate calls operator+).  This function exhausts VC6's per-function inline
budget: the first part expands every Vec3 operator, from 0x00410d00 the operators
are expanded but their constructor is the out-of-line 0x00404e60, and in the
epilogue whole operators are called (0x00428090, 0x00421cb0, 0x00421d00,
0x005015b0, 0x00413160).  Natural Math3D operators reproduce that pattern; the
budget is consumed per inline expansion, so `right` and the start-up cross product
are written out component-wise (a CrossProduct call there moves the switch-over
points too early).  The loop's `right` cross is built in the named copy `rearCross`
and assigned whole (aligned 3737 -> 3760 of 3966 retail instructions); `right = Vec3(...)`
(3723), a constructed named copy (3722) and `right = CrossProduct(fwd, normalR)`
(3686) do not.  Every dot product is BikeAILookDot, retail's (x + y) + z grouping
(behaviour at 24-bit; it also raised the linked masked score 0.954 -> 0.957); swapping
its arguments is inert at every site.  The same
named copy at dir2's double cross, the loop's flat right, both dir2 rebuilds and sideV's
inner cross loses (3485..3728).  Written-out dots at sideDotF: 3737 (inline budget).  Remaining differences: the start-up `up` vector and the
loop-head drag vectors keep their zero components in memory in retail (the
candidate's are constant-folded), the order of a few temporaries and stores
(0x00411dbc vF += (commuted operands), 0x00412883), and the frame layout.
Retail folds up.y = 1 into the start-up cross but loads up.x/up.z from the copy of
the Vec3(0, 0, 0) temporary, and that code sits after the `side` test; moving the
`up` set-up into an if/else arm (with `up` at function or block scope, either branch
order) still folds it and loses 111 aligned instructions (3628), and a comma
expression in the ternary's false arm still folds it (3740 aligned, 1468 strict).

The game reads (race kind, tree collision, object root, difficulty, race mode) go
through the complete TrackGame class, with no view cast: aligned 3731 of 3966, frame
0x968 vs 0x93c (the cast views gave 3402); 3740 with dir2's double cross in the
BikeAICrossCtor form, 3739 (1542 strict) with x + (y + z) penetration dots.  The read's member-path depth moves the
inline switch-over although the global loads are identical: a flat member of a plain
struct pointer gives 3760, a one-level nested member 3402 (frame 0x978), the complete
two-level TrackGame path 3731 (the 0x00410d00 `aim = fwd +- right` site inlines one more
constructor and the end site 0x00412e79 then calls operator+).

Frame (tools/frame_layout.py after aba3ceb7; the earlier 0x95c reading is superseded):
retail's 0x93c holds xf as a full Matrix4 at +0x568 (identity written in the prologue)
and, just above it, the end site's single-use return buffers (CrossProduct at
0x00412b37, operator- at 0x00412eb2).  Declaring the four post-loop ground/normal
vectors at function scope gives 0x938 with every temporary region at retail's offset
except that 12-byte slot, but the loop's copy-initialisations become assignments and
the inline switch-over moves (aligned 3631).  A bare block around the post-loop
section (groundF..`back -= pen * normalR`) gives 0x92c, or 0x938 with groundF at
function scope, with identical code; neither gains a strict byte because the ~100
locals still sort in a different use-count order (front +0x54 vs +0x50, normalF +0x80
vs +0x70, ...), so neither is kept.  A typedef-prefix scan (k = 0..63 extra
declarations) is inert (3731 throughout).

The notes above `SimulateLookAhead`, `SimulateAIFrame` and `UpdateAIRacingLine` in the
unit record the remaining per-site forms.
