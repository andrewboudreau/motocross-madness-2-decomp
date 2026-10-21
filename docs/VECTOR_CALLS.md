# Explicit-output vector providers

`src/reconstructed/VectorCalls.cpp` supplies four existing call signatures from
`src/krusty2/math/Math3D.h`. Vehicle, motion, collision, BVH and scene-graph
callers already import these names. Their declarations and compiled callers
are unchanged.

| Provider | Retail VA | Complete bytes |
|---|---:|---:|
| `Vec3AddCall` | `0x00421cb0` | 69 |
| `Vec3SubtractCall` | `0x00421d00` | 69 |
| `Vec3ScaleCall` | `0x005015b0` | 69 |
| `Vec3DotCall` | `0x0040ae30` | 29 |

All four bodies match every byte under VC6 SP3 `vc6_o2_mt`. Their full extents
come from decoded retail returns and COFF function auxiliary records; none has
a branch, call or relocation. The object has only the CRT `__fltused` dependency,
exactly four external function definitions, and no dynamic initializers.

The three vector-producing functions receive their output pointer first,
followed by input pointers and, for scaling, a float. They return that same
pointer in EAX and use caller stack cleanup. This is the machine interface of
the retail vector operators' hidden result storage. A local `Vec3` holds each
float component before copying to the output, preserving the retail stores
when input and output buffers overlap. The dot product returns through x87
ST(0), with the y and x products added before the z product. The source's
parentheses preserve this grouping without an intermediate float store.

These C++ names, explicit-output declarations and source grouping are
reconstruction choices; they do not establish original exported names, a retail
translation unit or the original vector type. Existing inline operator COMDATs
remain in other source objects under their own C++ names. This unit supplies
the four formerly unresolved call signatures directly, with complete arithmetic
bodies; it does not forward through wrappers or linker aliases. The probe-only
`samples/physics/helpers/VectorOps.cpp` is not a production provider.

The inline `DotProduct` in `Math3D.h` remains separate. Its emitted body groups
the z and y products before x, and is not interchangeable with `Vec3DotCall`.
For example, `(2^80, -2^80, 1)` dotted with `(1, 1, 1)` produces 1 with the
retail grouping; adding 1 to `-2^80` first loses it even at x87 extended
precision. No shared inline formula is changed by this connection.

`tests/test_vector_calls.py` compiles the actual production object and actual
Vehicle/Motnctrl callers, checks all four import names against their retail
bindings, and compares each full function with zero ignored bytes. It links
the same object into a native fixture that compares 31,873 cases against the
hash-pinned private retail functions: distinct and identical input pointers,
in-place and shifted overlapping outputs, complete surrounding storage,
returned output pointers, signed zeros, subnormals, infinities, NaN payloads,
varied float bit patterns, and the cancellation example above. Only the test
maps retail code; the production unit contains ordinary C++.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python tests/test_vector_calls.py -v
```

The helper manifest records all four canonical functions. This closes four
real linker names; other vector call views, incomplete source bodies and game
startup/menu/racing validation remain separate work.
