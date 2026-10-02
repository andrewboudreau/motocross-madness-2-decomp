# Matrix helpers (0x004a13e0-0x004a18db)

Canonical source: `src/reconstructed/MatrixUtil.{h,cpp}`; bindings in
`src/reconstructed/MatrixUtil.bindings.json`. No `__FILE__` string attributes
these free functions (nearest references: `Lzw.cpp` before,
`MorphBastardModifier.cpp` after), so the file name is descriptive.

Their order and bodies follow the DirectX 5 SDK sample `d3dutils.cpp`
(`ZeroMatrix`, `IdentityMatrix`, `ProjectionMatrix`, `ViewMatrix`,
`RotateZMatrix`, `MatrixMult`). That is external context for the provisional
names, not proof; retail differs from the SDK where noted.

| Function | Retail VA | Bytes | Status | Notes |
|---|---|---:|---|---|
| `ZeroMatrix` | `0x004a13e0` | 41 | strict exact | Nested loop folds to `rep stosd` |
| `IdentityMatrix` | `0x004a1410` | 71 | strict exact | `(row == column) ? 1.0f : 0.0f` |
| `ProjectionMatrix` | `0x004a1460` | 156 | strict exact | Takes an aspect ratio (not in the SDK); cot(fov/2) diagonal, Q = 1/(1 - near/far) |
| `ViewMatrix` | `0x004a1500` | 744 | not matched | See below |
| `RotateZMatrix` | `0x004a17f0` | 106 | strict exact | |
| `MatrixMult` | `0x004a1860` | 123 | strict exact | Both matrices by value; `result(i, j) += a(k, j) * b(i, k)` |

Codegen evidence for the types (`Matrix4`, `Vector3`):

- An empty user-declared `Matrix4` constructor makes VC6 copy a by-value
  result straight from the returned pointer (the Camera constructor needs it).
- `MatrixMult` matches only through D3DMATRIX's `operator()(row, column)`
  accessor; plain `m[i][j]` indexing swaps the `fld`/`fmul` operands
  whichever way the product is written.

## ViewMatrix (open)

`0x004a1500` takes `(from, direction, up, roll)` by value. It normalises
`up` and `direction` in place. Each normalisation calls out-of-line helpers:
the dot product at `0x0040ae30` (compared with 1.0), the fast inverse square
root at `0x00460c00`, and `v * s` at `0x005015b0`. Making those inline
changes the code, so they are separate functions in retail.

It sets right = up × direction. Rows 0-2 hold right/up/direction as columns,
row 3 holds the negated dot products with `from`, and a nonzero roll applies
`MatrixMult(RotateZMatrix(-roll), view)`. The best candidate (scratch only)
has the right length with about 103 of 744 bytes differing, all in
scheduling of the column stores and the term order of the row-3 sums.
Statement order, term order, `CrossProduct` helper forms and the Normalize
shape have been tried; the remaining difference is not yet explained.
