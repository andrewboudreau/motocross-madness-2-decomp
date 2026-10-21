# Scalar-first vector scale

`src/reconstructed/VectorScaleLeft.cpp` supplies the previously missing
`Vec3ScaleLeft(float, const Vec3&)` implementation. Authentic VC6 SP3 emits all
**69 retail bytes** at `0x428090` exactly, with no relocations to mask. This is
an ordinary C++ definition using the shared 12-byte Vec3 type. Its name and
reconstruction grouping are provisional; the original translation unit is
unproven. No special flags, pragmas, wrappers or linker aliases are used.

## Complete extent and call convention

The retail body has one straight-line path, no calls or branches, and a plain
`ret` at `0x4280d4`. Eleven NOP bytes follow before the next aligned code at
`0x4280e0`. VC6 independently records a 69-byte function extent in its COFF
auxiliary record; the containing code section also includes alignment padding.
The check compares the complete function without trimming it to a target size.

The cdecl stack contains the hidden output pointer, scalar word, and input vector
pointer, in that order after the return address. After reserving 12 local bytes,
retail reads these arguments at `[esp+0x10]`, `[esp+0x14]` and `[esp+0x18]`.
It multiplies x, y and z at offsets 0, 4 and 8 by the scalar and spills the three
results into float locals. Only after reading every input component does it
copy x, y and z to the output. EAX returns the output pointer; the caller removes
the three argument words.

This order preserves exact and partial input/output overlap. Invalid input can
fault before any output write. A partially writable output can retain completed
earlier component stores when a later store faults. The source's ordinary Vec3
temporary reproduces these instructions and their ordering.

## Actual callers and dependency closure

The existing `BoundingBoxTreeQuery.h` prototype already names the required
symbol, `?Vec3ScaleLeft@@YA?AUVec3@@MABU1@@Z`. No caller or header change is needed
to supply it. Both decoded calls are in the complete, strict 1,436-byte
`CapsuleTreeQuery` at `0x4298c0`:

| Call site | Inputs and use |
| --- | --- |
| `0x429a8a` | Triangle branch: scale the triangle normal by distance, then subtract the result |
| `0x429da5` | Point branch: scale the local direction by the clamped parameter, then add the first segment endpoint |

The retained test checks both argument sequences and the immediate use of EAX,
plus exactly two REL32 references to the provider in the actual caller object.
The extra earlier push at `0x429d9d` prepares the surrounding addition; it is not
a fourth argument to this helper. The vector-first scale helper at `0x5015b0`
has a different argument order and is not substituted here.

The provider object exports only this function. Its only undefined symbol is
the ordinary CRT marker `__fltused`; it has no game dependencies, data objects,
CRT initializers or executable relocations. Empty bindings are sufficient for
the complete raw-byte proof. This does not identify unrelated vector classes or
establish ownership of the surrounding retail functions.

## Native validation

`tests/test_vector_scale_left.py` compiles the actual standalone provider with
`vc6_o2_mt` and links that same object into the native fixture. The separately
compiled fixture uses `/Od`. No object bytes are patched and no math is mocked.
Only the private executable maps the hash-pinned retail image; the production
source has no retail-address dispatch or dependency.

The fixture calls the provider through its decoded native hidden-output ABI to
request overlapping output addresses, and also through an ordinary C++
by-value call. Successful calls independently require the returned pointer to
equal the supplied output address. Compile-time checks confirm Vec3 size and
x/y/z offsets. The **135,279** passing cases are:

| Cases | Coverage |
| --- | --- |
| 135,168 | 128 input seeds, 11 output offsets, eight scalar patterns, four rounding modes and three x87 precision modes |
| 108 | Nine null, dual-invalid and progressive guard-page input/output modes across all 12 rounding/precision combinations |
| 3 | Independent known C++ result/input preservation, input-first dual-invalid fault, and first-component store before an output fault |

The input seeds include 16 edge-pattern seeds and 112 deterministic raw-bit
seeds. Patterns cover signed zeros, denormals, finite extremes, infinities and
NaNs. Output offsets include exact aliasing, partial overlap on either side and
separate storage. The complete 32-word backing storage and committed guard page
are compared at the same deterministic addresses, together with the return
pointer, post-call FP status/control, and access-fault kind/address plus the six
x87 exception-status bits at the fault.

Floating-point exceptions remain masked. Unmasked FP traps, complete saved
register context, resuming after faults, and complete BVH traversal or racing
are not covered by this isolated helper test. Complete caller byte matching
provides separate evidence for its two real call boundaries. The existing
`test_vector_calls.py` protects the four earlier vector providers independently.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_vector_scale_left.py -v
python -m unittest discover -s tests -p test_vector_calls.py -v
```

Initial private reproduction is retained under `work/vector-connections39/`:
`review_scale_left.py`, `provider-review.json`, `caller-review.json`,
`decode-review.txt`, `scale_left_compare.cpp` and `native-review.log`. The tracked
regression compiles the canonical source afresh and does not depend on those
private reports. Source/link integration and game runtime remain separate checks.
