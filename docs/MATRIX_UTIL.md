# Matrix and vector helpers (0x004a10e0-0x004a1d0b)

Canonical source: `src/reconstructed/MatrixUtil.{h,cpp}`; bindings in
`src/reconstructed/MatrixUtil.bindings.json`. No `__FILE__` string attributes
these free functions (nearest references: `Lzw.cpp` before,
`MorphBastardModifier.cpp` after), so the file name is descriptive. The unit
runs from the vector helpers at `0x004a10e0` to its `kVec3` set
(`.CRT$XCU` 175-178, `0x004a1bd0..0x004a1d0b`), which `0x004a10e0` and
`0x004a11e0` read.

The matrix helpers' order and bodies follow the DirectX 5 SDK sample
`d3dutils.cpp` (`ZeroMatrix`, `IdentityMatrix`, `ProjectionMatrix`,
`ViewMatrix`, `RotateZMatrix`, `MatrixMult`); `MatrixInverse` has the shape
of `d3dmath.cpp`'s `D3DMath_MatrixInvert` without the translation row. That is
external context for the provisional names, not proof; retail differs from the
SDK where noted.

| Function | Retail VA | Bytes | Status | Notes |
|---|---|---:|---|---|
| `ReflectAndNormalize` (reflect) | `0x004a10e0` | 247 | strict exact | `normalize(v - 2 (v . n) n)`; FastInvSqrt `0x460c00`; the dot and squared length sum as z + (x + y) |
| `TriangleNormal` | `0x004a11e0` | 281 | strict exact | Cross product through the Vector3 constructor with four parenthesised products; squared length `z*z + ((x*x) + (y*y))` |
| `LinePlaneIntersection` (line/plane) | `0x004a1300` | 211 | strict exact | TriangleNormal, then `from + (to - from) * t`; dots through `Vector3::operator[]` |
| `ZeroMatrix` | `0x004a13e0` | 41 | strict exact | Nested loop folds to `rep stosd` |
| `IdentityMatrix` | `0x004a1410` | 71 | strict exact | `(row == column) ? 1.0f : 0.0f` |
| `ProjectionMatrix` | `0x004a1460` | 156 | strict exact | Takes an aspect ratio (not in the SDK); cot(fov/2) diagonal, Q = 1/(1 - near/far) |
| `ViewMatrix` | `0x004a1500` | 744 | strict exact | See below |
| `RotateZMatrix` | `0x004a17f0` | 106 | strict exact | |
| `MatrixMult` | `0x004a1860` | 123 | strict exact | Both matrices by value; `result(i, j) += a(k, j) * b(i, k)` |
| `MatrixInverse` | `0x004a18e0` | 362 | strict exact | Upper 3x3 by cofactors, `inverse * (...)`; `ZeroMatrix` when the determinant is zero |
| `ProjectPoints` (strided perspective transform) | `0x004a1a50` | 175 | not exact (171/175) | In the unit with a status comment; one `fld`/`fmul` operand pair in the last row |
| (hand-scheduled x87) | `0x004a1b00` | 196 | inline assembly | ebp frame, `fxch` pairing, negative-index loop; not reconstructed |
| `kVec3` set `_$E1`..`_$E11` | `0x004a1bd0..0x004a1d0b` | | strict exact | 8 initializers, globals `0x685030..0x685068` |

Codegen evidence for the types (`Matrix4`, `Vector3`):

- An empty user-declared `Matrix4` constructor makes VC6 copy a by-value
  result straight from the returned pointer (the Camera constructor needs it).
- `MatrixMult` matches only through D3DMATRIX's `operator()(row, column)`
  accessor; plain `m[i][j]` indexing swaps the `fld`/`fmul` operands
  whichever way the product is written.
- `Vector3` has d3dvec.inl's `operator[]` (`(&x)[i]`). `0x004a1300`'s two dot
  products match only when the components are read through it and summed as
  `z + (x + y)`; `0x004a10e0` writes the same association with member access.
  Written operand order of a product never changes the code; the accessor
  form and the association do.
- `MatrixInverse` matches only with the cofactor subexpressions repeated in
  the determinant and the results (the compiler shares them). Naming them as
  locals keeps `c11` in a register and swaps the determinant's `fld`/`fmul`;
  `determinant != 0.0f` (inverse first, `ZeroMatrix` in the else) gives the
  retail block order, and VC6 reuses the dead by-value parameter as the
  `ZeroMatrix` return slot in both.
- `0x004a1a50` keeps typed `Vector3` cursors stepped by the strides in a
  count-down loop; VC6 reorders the sums itself (retail sums z, y, x for w and
  y, z, x / y, x, z for the rows), so the written term order does not matter.

## ProjectPoints forms tried

The notes the near-miss sample carried before `0x004a1a50` moved into
`src/reconstructed/MatrixUtil.cpp` (the shadow passes call it for perspective
shadows):

```text
0x004a1a50 (strided perspective transform, 175 bytes): 171/175; only the z
  term of the last row loads the vector before the matrix element (retail
  loads m[2][2] first). The typed cursors with a count-down loop are what
  give the retail +4/+8 pointer bias and the register use; VC6 reorders the
  sums itself (retail sums z, y, x for w and y, z, x / y, x, z for the
  rows), so the written term order does not matter.
  Also tried without gain: for/while/guarded do-while and count-up loops,
  the increments in the for header or reordered, float*/char* cursors, the
  point copied into locals or a Vector3, a stride or matrix-reference local,
  an inline point-transform helper (pointer, reference or by-value point),
  the rows computed into x/y/z/w locals and the divide written as `/ w`.
  Every extra live local (stride, counter, reference) reorders many products
  at once. Redundant parentheses (docs/VC6_OPERAND_ORDER.md section 3): all
  2112 bracketings, term orders and parenthesised products of the z row,
  and all 4096 combinations of parenthesised products over the four sums,
  in this file and inside src/reconstructed/MatrixUtil.cpp, give 171 at
  best. The Matrix4 operator()(row, col) and Vector3 operator[] accessors
  (which decide MatrixMult's operand order) change nothing either, used
  for every element or only for the z row / its z term. A second matrix or
  point alias (reference/pointer) used in only one or two terms gives
  126..171.
  0..33 dead initialised int locals at the top give 171 at every count
  (unlike the live locals above, they do not reorder it).
  A D3DMATRIX-style matrix (a union of `_11`..`_44` members and m[4][4])
  as the parameter type gives 171 with every operand order of the
  x/z terms (512 forms); reached through a cast local it gives 121.
  One term per statement (`float z = a; z += b; ...`): the z row alone in
  all 24 term orders, with `dstPoint->z = z * w` or `z *= w`, gives 164 at best;
  all four rows accumulated, products in each of their 6 orders and the
  translation last (1296 forms), give 164 at best.
  The operand order inside each of the twelve products (all 4096
  combinations, session 14) gives 171 every time: VC6 canonicalises it.
  Dead reads of matrix->m[2][2] (a dead local before the loop, a dead
  z * m[2][2] product before w or after the z row) give 171: unlike frame
  locals (SoultreeObject::WorldToLocalDirection), these indirect operands
  are not aged by code that VC6 removes (session 18).

  Leaf-age copies (s22, the form that made Griddraw 0x0047e600 exact): one
  operand of a z-row product copied into a float local just before the
  row is a live local here, not a folded one: m[2][2] 147, srcPoint->z
  121, both 115, the x or y operands or the translation 113..142, and
  m[2][2] copied at the loop top or before the loop 53 and 39.
  Round 9 (2026-11-14), loop and cursor shapes: the parameters themselves
  as cursors (43), loop-local cursors read from the parameters (40, 51
  with only the source one), an index loop with `i * stride` (171), the
  target cursor stepped first (171), `while (count-- > 0)` (24) and
  `for (i = count; i > 0; i--)` (122).
  Round 15, all 171: the source and target cursors typed apart (Vector3,
  float* or char* each, eight pairings), the last z read through
  `(&srcPoint->x)[2]` or a float cast (alone or in every row), the cursor
  declarations or increments swapped, the source increment commuted,
  through `&srcPoint->x` or as int arithmetic. Retail schedules `add ecx,
  edi` right after the product that differs, as the candidate does.

k-scan (s22): compiled behind 0..63 prepended typedefs, 0x004a1a50 has the
same code at every count, so its difference is not a declaration-count
effect (docs/VC6_OPERAND_ORDER.md section 2).
```

## ViewMatrix

`0x004a1500` takes `(from, direction, up, roll)` by value. It normalises
`up` and `direction` in place. Each normalisation calls out-of-line helpers:
the dot product at `0x0040ae30` (compared with 1.0), the fast inverse square
root at `0x00460c00`, and `v * s` at `0x005015b0`. Making those inline
changes the code, so they are separate functions in retail.

It sets right = up × direction. Rows 0-2 hold right/up/direction as columns,
row 3 holds the negated dot products with `from`, and a nonzero roll applies
`MatrixMult(RotateZMatrix(-roll), view)`. Camera slot 28 (`0x0042ee30`)
calls it with the camera frame by value.

Required source shapes: the scale goes through a named `scale` local before
the `0x005015b0` call (otherwise VC6 stores the inverse square root straight
into the argument slot); the cross product is stored member by member with
both products of each component parenthesised, which places retail's integer
column copies between the x87 instructions (docs/VC6_OPERAND_ORDER.md
section 3); the row-3 dots sum `z + (x + (y))` through member access
(`DotMembers`). The index-accessor `DotProduct` in that association breaks
`0x004a1300`, and in the `z + (x + y)` association swaps the x and y terms
of the up row's dot here, so the file has both helpers.
