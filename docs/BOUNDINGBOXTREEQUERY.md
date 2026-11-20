# BoundingBoxTreeQuery.cpp

`src/krusty2/bvh/BoundingBoxTreeQuery.cpp` holds the run-time queries against the
bounding-box trees, extent `0x00424690..0x0042ad2f` (strong inference: the file name is
ours, the unit has no `__FILE__` string and no RTTI; the unit's header comment gives the
evidence). The functions sit in retail order. The five near misses below moved in from
`samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp`, each with a status comment.

| Address | Function | Strict bytes |
| --- | --- | --- |
| `0x00425900` | `BoxCapsuleOverlap` | 2450 of 4314 (4321 compiled) |
| `0x00426be0` | `BoxTriangleQuery` | 1542 of 2576 |
| `0x00427c10` | `SegmentTriangleQuery` | 424 of 1101 (1112 compiled) |
| `0x004280e0` | `TreeTreeQueryNodes` | 450 of 2160 (2228 compiled) |
| `0x0042a640` | `SegmentSegmentDistance` | 73 of 1471 (1458 compiled) |

The other 43 registered targets (`src/krusty2/bvh/targets.json`) are strict exact.

The near misses call retail's out-of-line vector helpers through by-value views
(`Vec3Add`, `Vec3Sub`, `Vec3Scale`, `Vec3LengthRef`, ...), where the exact functions
use the pointer-form providers (`Vec3AddCall`, ...); `config/game_build.json`
`link_aliases` joins each view to its provider. The unit also defines the never-written
`0x00579668` flag that SegmentTriangleQuery reads.

## Behaviour

Differential review against retail (`tools/emu_diff.py CASE`):
`segment_segment_distance` 0x42a640, `segment_triangle_query` 0x427c10,
`box_capsule_overlap` 0x425900, `box_triangle_query` 0x426be0, `tree_tree_query`
0x4280e0 and `box_overlap` 0x424ab0 show no differing run (counts in the commit that
moved the bodies here).

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/bvh/BoundingBoxTreeQuery.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

The run reports `43/48 strict exact`; the five near misses are registered as partial.

## Forms tried

Scores are from `tools/run_physics_samples.py` (VC6 SP3, `vc6_o2_ml`).

- BoxTriangleQuery 0x00426be0 (2576 bytes, 2576 here; linked masked 1.000, strict 0.709):
  the 13-axis box/triangle SAT.  Every instruction aligns with retail, and the frame
  (0x164) matches.  Left: stack-slot assignment only (the transformed vertices sit at
  esp+0x38.. here, esp+0x50.. in retail, and case 1..3 temporaries differ), so
  run_physics_samples' positional byte score is 64% (66% for the 2572-byte shape before).
  Edge projections: retail computes p2 = p0 - Dot(e1) first and keeps it on the x87
  stack (`fsubr [p0] ; fst [esp+0x80]`), then p1 = p0 - d1 (`fst` twice), and the inner
  min/max compares p1 against the unrounded p2 (`fcomp st(1)`).  The old named
  `float p2` was stored with `fstp` and reloaded, rounded to binary32, so at 53-bit
  precision (FPCW 0x027f) the separating test could flip by one ulp (2 of 2250 runs at
  -n 1500; 0 at 24-bit).  QueryMinBFirst/QueryMaxBFirst with (p0 - d2, p0 - d1) give
  retail's order: 0 of 2250 at both 0x027f and 0x007f.  Ruled out for this: p1 assigned
  inside the call (copy-propagated; the inline budget moves, 2532 bytes); p1 or p2 named
  with the other an expression (2528 bytes, or p1 evaluated first); d1 reused as p1; the
  min/max split into statements; a p[3] array; the plain QueryMin/QueryMax on
  expressions (p1 first, or a reversed test that differs at equality); `a > b ? b : a`
  forms; the BFirst helpers for the outer min/max too, or with swapped outer operands
  (16..20 differing instructions).  Earlier rounds (by value, by const reference,
  if/return bodies, macros, three-argument inlines, other argument orders) are in Git.
- TreeTreeQueryNodes 0x004280e0 (2160 bytes, 2228 here): expands the SAT of BoxOverlap
  inline with every helper called out of line except QueryAbs, which this source
  reproduces through an inline copy of the BoxOverlap body (BoxOverlapInline; BoxOverlap
  itself is not expanded in SweptBoxOverlap or TreeBoxOverlap, so it cannot be the inline
  function).  Differences: VC6 here still expands the first a[] constructor, and the
  recursion tails are cross-jumped differently (retail's leaf branches jump into the
  b-children tail of the box branch).  Budget: one more accessor expansion anywhere after
  the box test (a QueryIsLeaf/volume/child accessor; the same ones in the leaf-pair test
  before it change nothing) makes the a[0] constructor a call as in retail, but the
  second array's default construction then goes through the vector constructor iterator
  (??_H), which retail never calls; dearer QueryAbs forms, named sums, an axis reference,
  moving or merging the array declarations and aggregate-initialised arrays do not give
  retail's pattern either.  Tails: `int hit = 0` inside each leaf arm gives retail's leaf
  code (useMotion pushed per arm, hit cleared after the pushes), but VC6 then merges the
  box a-children call into the b-children one and the leaf arms into each other; 24
  hit/return layouts (shared, per-arm, early-return, function-level hit) do not give
  retail's merge (`else if (LeafNodeQuery(...))` changes nothing; an early `return 0`
  for a failed LeafNodeQuery merges every tail, 557/631).  The out-of-line Vec3Call for a[0] (or a[0] and a[1]) only moves the
  expansion to the next constructor (472/177, 469/161 bytes/differing; all three give
  93/235), so one call stays missing in the s15 reference diff; its 0x22/0x86 immediates
  are the jump table after the function's ret (0x00428914), a false positive.
- SegmentSegmentDistance 0x0042a640 (1471 bytes, 0.89 instruction ratio): written with
  explicit call views.  Retail homes the plane parameters, clamps and the first length in
  the dead d0/d1 argument slots (frame 0x60); VC6 here keeps them in a 0x80 frame, and the
  inline cross products load a's components first in the x term.  The natural operator
  form expands more helpers than retail (see the notes at the function).  The t1
  numerator sums (off + xy) + z as retail does (z product computed first and kept);
  the flat form compiled to ((x + y) + z) + off (48 -> 73 strict, 432 -> 434 aligned).

- SegmentTriangleQuery 0x00427c10 (1101 bytes, 1110 here): the natural operator form gives
  retail's exact sequence of inline and out-of-line helpers (five constructor calls,
  operator*(float, Vec3), three Vec3::operator+= calls).  The segment is moved into the
  triangle's frame relative to the sweep record's offset, clipped against the face plane,
  and on a hit inside the triangle the record's offset, contact, scale, count and the
  contact normal list (0x00579068/0x00579014, deduplicated through 0x005299e0 unless
  0x00579668 is set) are updated; the two flag branches are literal (the set-flag branch
  adds the push vector a second time and replaces the contact).  Differences: retail keeps
  the constant 1 in ebp (count, scratch count and result), so its register plan and the
  copy of segment[1] at entry differ.  A z-first push dot in the clear-flag arm
  (QueryDotZFirst) gives 374 (349 with DotProduct in both arms); the same helper in the
  set-flag arm loses (332 alone, 357 in both).  The set-flag arm's dot keeps z*z on the
  x87 stack and adds it last (QueryDotZKept): 424, and retail's (x + y) + z sum.

- BoxCapsuleOverlap 0x00425900 (4314 bytes, 4321 here, 2450 strict, 1283/1300 aligned):
  the decoded structure (box axes, axes crossed with the segment direction, three corner
  directions).  Shape points that moved it from 734 strict / 1140 aligned:
  * six float corner distances squared through QuerySquares and the corner signs through
    QuerySigned spend the inline budget retail spent (squared-corner constructors and most
    normalisation DotProducts are calls as in retail); QuerySigned written as if/return
    (not ?:) is one step cheaper again (1273 -> 1277 equal instructions).
  * one `axis` written component by component and normalised in place
    (axis = Vec3NormalizeInline(axis), named result r): retail keeps the input and the axis
    in one slot.  `axis = Vec3(...)` copies through a temporary (1142); a by-value or
    in-place QueryNormalize(Vec3&) helper gives 1136/1219.
  * each separating-axis test as named `dist` and `ra` (three QueryDot projections) plus the
    z-first half-segment projection: retail's fadd st(1)/faddp st(2)/fstp st(0) sum, the
    (0 + 0) + z*hz order and the out-of-line DotProduct in the sixth normalisation.
  * the box tests read a named |halfSegment| extent first (separate floats per axis).
  * the first six normalisations take |v|^2 from retail's dot (0x0040ae30, (y + x) + z)
    through Vec3Dot; with DotProduct they linked Math3D.h's out-of-line COMDAT copy, which
    sums (z + y) + x (same instructions, a different result at 24-bit precision).  The
    seventh keeps the inline DotProduct (Vec3NormalizeDotInline): its x is 0, so the
    expanded y*y + z*z equals retail's call.  Vec3Dot there too makes all seven calls as in
    retail but leaves that site's v * inv inline (2432 strict, 1293 aligned), scaling by
    the unrounded inverse: 6 of 500 emu runs differ at 53-bit precision.
  Left (17 instructions): the seventh normalisation's DotProduct stays inline (retail calls
  it), two fxch in the fifth/sixth tests, c1z's slot (retail 0x48 beside c1x/c1y) and a
  permuted frame.  Ruled out (equal instructions of 1300): QueryDot for only boxAxes[2]
  (1089) or an explicit z-first boxAxes[2] (1171); QueryDot/DotProduct/a z-first helper
  for the half-segment too (inlines go out of line); `!(... >= ...)`, `<`, if/else and
  parenthesised box tests (1263-1264); a QueryGap corner helper, c1 as a Vec3, the squares
