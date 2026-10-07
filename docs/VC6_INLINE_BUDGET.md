# VC6 SP3 inline expansion budget

Several retail functions call out-of-line copies of header inline helpers
(the `Vector3` constructor `0x00404e60`, `operator+` `0x00421cb0`,
`operator*` `0x005015b0`, `DotProduct` `0x0040ae30`, `CrossProduct`
`0x00515600`, normalisation `0x005087b0`) at some sites and expand them at
others. This page records what controls that in VC6 SP3 (`/O2 /GR /GX /MT`),
measured with `tools/inline_budget_probe.py`, so a near miss can be read
correctly: the out-of-line calls are evidence about the original source's
size and inline structure, not about different compiler flags.

## The out-of-line copies are COMDATs

The helpers are not a math library. Each copy lies in the code of a
different translation unit: the constructor among BackgroundImage.cpp's
literals, `operator+`/`operator-` (`0x00421cb0`/`0x00421d00`) in
bikerace.cpp, `+=` and `float * Vector3` (`0x00428060`/`0x00428090`) in
BoundingBoxTreeBuild.cpp, `Vector3 * float` in SoulTreePhysics.cpp,
normalisation in Terrain.cpp, `CrossProduct` in Track.cpp and `DotProduct`
in Bike.cpp (`tools/nearest_source.py`). That is where the linker keeps the
first COMDAT instance of a header inline function in link order; every unit
that failed to expand it at some site emitted its own copy and the linker
folded the rest. The helpers were therefore declared inline in a shared
header, and the question at each call is only whether VC6 expanded it.

## Budget rule

VC6 gives every function one expansion budget and expands inline calls
until it is used up. Measured with a four-statement helper (`I4`: `int b =
0; b += Ext(t + 1); ... return b;`) at N sites of `s += I4(k);`:

| Caller | Expanded |
|---|---:|
| 16, 20, 32 or 40 sites | 15 |
| 96 sites | 24 |
| 96 sites + 50 / 100 / 200 `q += Ext(k);` statements | 37 / 49 / 74 |
| 64 sites of an eight-statement helper | 9 |

- There is a floor: small callers always get about 15 expansions of `I4`
  (roughly 400 tree-node units; about 25 with a two-statement helper, 9
  with an eight-statement one). Above the floor the budget grows with the
  caller's own size, about 1.5 x its node count.
- The size is the front-end tree, not generated code: dead code
  (`if (0) q = 1;`) counts, an empty statement does not, a block `{}` costs
  about half an assignment, `if (c) q = 1;` about 3.5 assignments, a call
  statement about 2. Expansions are charged the same way, so an expanded
  helper with a nested inline call pays for both; `return Vector3(...)`
  operators (about 18 units) are cheaper than a named-result form
  (`Vector3 r; r.x = ...; return r;`, about 22), the three-store
  constructor costs about 12 and a one-line accessor about 3.
- A single site is never refused for its own size: one 33-statement helper
  expands in an otherwise empty function.
- Nothing else moves it: `/Ob1`, `/Ob2`, `/Ox`, `/Og-`, `/G5`/`/G6`, `/Op`,
  `/Za`, `/Zi`, `#pragma inline_depth(255)`, `inline_recursion(on)` and
  taking the helper's address give identical results. `/O1` and `/Os`
  expand none of these helpers; `__forceinline` expands all. The retail
  code expands most sites, so it was built to favour speed (`/O2`/`/Ot`).
- Which sites lose is only partly source order. Direct sites lose from
  the end of the function (the last sites are called). Nested calls lose
  before their enclosing expansion (retail `0x005329e0` expands
  `operator*` but calls the constructor inside it, then calls the next
  `operator*` outright), and their order depends on the shape: the probe's
  `nested_48` expands the inner helper only at the last 15 of 48 wrapped
  sites, a wrapper with two other statements loses it at scattered sites.

## How to read a near miss

- Retail expands more than VC6 does from the reconstruction
  (CarProcedural `0x00430b10`, 14 operators all expanded): the original
  caller was bigger in tree nodes or its helpers cheaper. About 30 trivial
  extra statements (roughly 15% of the floor) make VC6 expand everything
  there and give retail's first 0x19b bytes. Nested operator expressions
  cost a little more than named intermediates (`(a + b) + a` versus
  `t = a + b; v = t + a`), not enough to explain it alone.
- Retail expands less (Wrecker `0x005329e0`, `0x00531740`, slot 10): the
  original spent more budget before that point. The natural operator form
  of `0x005329e0` already gives retail's 0x94-byte frame with every site
  expanded; adding one trivial inline expansion (an empty-bodied accessor)
  makes VC6 call the constructor out of line as retail does, so the
  original differs by about one small inline helper, for example an
  accessor on the probe array or box, before the final scale.
- The order in which sites lose is only partly predictable, so a function
  with several out-of-line helper calls needs its inline structure
  reproduced, not padded. Statements that compile to nothing (dead
  branches, folded arithmetic) still move the budget and are rejected as
  reconstruction devices.
- Whether a later service pack's compiler used a different floor is
  untested; every byte-exact function was produced by SP3, and functions
  limited by the budget are exactly the ones that are not exact.

## Reproduce

```bash
python3 tools/inline_budget_probe.py --vc6-root "$VC6_ROOT"
```

The probe prints, per synthetic function, the number of expanded helper
sites and the helpers called out of line.
