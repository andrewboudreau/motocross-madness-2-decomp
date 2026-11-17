# Camera matrix update

## Functional build review

[Camera.cpp](../src/reconstructed/Camera.cpp) holds the complete nonexact
`Camera::UpdateMatrices` body at `0x0042ee30`, between `0x0042eb10` and
`0x0042f090` as in retail. Its full 492-byte extent ends
at `0x0042f01b`, before four padding bytes. VC6 emits 492 bytes with 487
matching positions: one stack-allocation instruction is scheduled on the
other side of a projection-matrix copy. The source body is unchanged.
The semantic name is inferred from behavior. The test links a validation unit
holding only this definition (`tools/unit_source.py`).

The five direct calls use the existing `ViewMatrix`, `ProjectionMatrix`,
`IdentityMatrix` and `MatrixMult` providers. Hidden result buffers and by-value
matrix/vector arguments matter here. The routine writes the view, projection,
viewport-scaled projection and screen matrices, then computes focal length
using x87 tangent. The intermediate float cast in the half-angle calculation
and the compiler's actual viewport integer conversions are retained.

`tests/test_camera_update_behavior.py` compiles the complete candidate with
authentic VC6 SP3 and checks the hash-pinned retail body, complete extent and
all five direct dependency calls. Its native fixture compares all camera
bytes and adjacent guards, helper arguments and ordering, intermediate state
at callbacks, exception results and floating-point status.

The scripted lane passes **76,288 cases**. These vary viewport dimensions
through zero, signed boundaries and integers beyond float's exact range;
field-of-view inputs include signed zeros, subnormal values, infinities and
NaNs. Helpers can change camera state or throw at each of the five call
boundaries. This verifies observed finite cases, not every possible invalid
pointer or callback behavior.

An additional **9,216 cases** link the actual `MatrixUtil.cpp`,
`VectorCalls.cpp` and `FastMath.cpp` providers and call `InitFastMath()` first.
Both retail caller and candidate use these same source providers, so this
lane checks their integration and argument contracts. The providers' separate
strict byte checks establish their individual retail matches. Neither lane
constitutes rendering or gameplay testing.

Only the private validation fixture maps retail bytes and redirects decoded
calls. The production build uses the C++ candidate and existing C++ providers.
Camera.cpp carries a `// Not exact` status comment on it.

## Forms tried

Camera::UpdateMatrices (0x0042ee30, 492 bytes, 487/492): one
scheduling difference: retail emits `sub esp, 0x40` before the `rep movsd`
that copies the projection matrix (MatrixMult's second argument), VC6 after.
Assigning through a named product, a nested assignment, m[][] stores, the
viewport built by an inline helper and the store order do not move it.
Also unchanged (487): a projection copy (local, Matrix4(...) temporary,
through a pointer or this->), a viewport temporary, `Matrix4 viewport;`
declared apart or at the top, the stores through a float pointer or a
reference, a named 0.5f constant, -viewport(0, 0) for the -0.5 and a block
around the viewport. Chained stores, a projection local assigned after
the viewport, the view matrix built after it and the products nested into
the second MatrixMult call are worse.
The half field of view needs the (float) cast: without it VC6 folds
0.5f * 0.01745329f into one constant, while retail multiplies twice.
0..15 dead initialised int locals at the top of the function leave the `sub esp, 0x40` in place.
In the second MatrixMult both compilers hoist the `sub esp, 0x40` over the
copy of `b`, whose space is left over from the first call; in the first
call `b`'s space comes from `sub esp, 0x28` plus deferred argument pops, and
only retail hoists there. Also worse: building the viewport (identity, or
identity and stores) before the projection (29-40 of 492) and /G6
(42 of 492); a `projection` copy right before the product is inert (487).
Session 18: `Matrix4 viewportScale(IdentityMatrix());` is inert (487) and a
named `Matrix4& screen = screenMatrix[1]` for the second product is worse (350).
2026-11-14: the slot between `mov edi, esp` and `rep movsd` is the address
register's delay slot, and VC6 fills it with a nearby independent
instruction: with a single viewport store left before the product, that
store lands there instead of the `sub esp, 0x40`. Not moved (487 or worse):
dropping the viewport stores, `viewMatrix` as the second operand, one to
eight member stores before, between or after the two products, the product
declared and then assigned, and a named result.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_camera_update_behavior.py -v
```
