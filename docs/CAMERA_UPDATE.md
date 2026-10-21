# Camera matrix update

## Functional build review

`samples/camera/CameraNearMisses.cpp` contains the complete nonexact
`Camera::UpdateMatrices` body at `0x0042ee30`. Its full 492-byte extent ends
at `0x0042f01b`, before four padding bytes. VC6 emits 492 bytes with 487
matching positions: one stack-allocation instruction is scheduled on the
other side of a projection-matrix copy. The source body is unchanged.
The semantic name is inferred from behavior; this standalone candidate does
not establish original translation-unit ownership.

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
The candidate remains explicitly nonexact in `config/game_build.json`.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_camera_update_behavior.py -v
```
