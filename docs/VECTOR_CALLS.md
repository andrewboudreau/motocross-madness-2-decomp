# Explicit-output vector providers

`src/reconstructed/VectorCalls.cpp` supplies four existing call signatures from
`src/krusty2/math/Math3D.h`. Vehicle, motion, collision, BVH and scene-graph
callers import these names directly. Additional call-view connections below
retain their complete compiled bodies.

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

## Existing call-view connections

Camera, FollowCamera and MatrixUtil call the exact dot provider through their
existing 12-byte `Vector3` views. Bike and BoundingBoxTreeQuery use their existing
`Vec3` objects directly. Their former reference or pointer dot declarations
all bound to `0x0040ae30`; complete caller matches and the provider's full
29-byte body establish the concrete stack arguments and x87 result, rather than
relying on the shared address alone.

FollowCamera, MatrixUtil and Wrecker now call the scale provider with explicit
output storage. This joins their previous hidden-result declarations to the
complete 69-byte provider at `0x005015b0`. The native layout guard verifies both
vector types are 12 bytes with float components at offsets 0, 4 and 8. Pointer
casts cross that evidenced boundary; the surrounding reconstructed types and
inline math formulas are retained. A scoped Wrecker output preserves the
former returned temporary's lifetime. BVH dot inputs that are returned vectors
bind const references within the original call expression, retaining the
existing result-buffer lifetime.

Five previously missing decorated names now import existing providers:

| Former view | Callers | Provider |
|---|---|---|
| `Vec3Dot(const Vector3*, const Vector3*)` | Camera, FollowCamera, MatrixUtil | `Vec3DotCall` |
| `Vec3Dot(const Vec3&, const Vec3&)` | BoundingBoxTreeQuery | `Vec3DotCall` |
| `BikeVec3Dot(const Vec3&, const Vec3&)` | Bike | `Vec3DotCall` |
| `Vec3ScaleCall(const Vector3&, float)` | FollowCamera, Wrecker | explicit-output `Vec3ScaleCall` |
| `Vec3Scale(const Vector3&, float)` | MatrixUtil | explicit-output `Vec3ScaleCall` |

`tests/test_math_call_connections.py` recompiles these six complete production
translation units and both actual providers with authentic VC6. It verifies
209 complete strict caller cases, the provider extents and bytes, real COFF
imports/call relocations, and the vector layout guard. Seven existing Bike
partial functions and both Wrecker sample functions retain their complete
relocation-resolved candidate hashes, extents and comparison scores. These
partial hashes protect unchanged reconstructed code; they are not retail
identity claims. The Wrecker sample retains its unfinished by-value scale view
locally; it is not supplied as another production provider.

```powershell
$env:PYTHONPATH = '.'
python tests/test_math_call_connections.py -v
```

The helper manifest records the four canonical providers. Other vector call
views, incomplete source bodies and game startup/menu/racing validation remain
separate work. In particular, Bike and collision constructor call views remain
unresolved: simply substituting the known inline `Vec3` constructor changes
existing partial bodies. They are not supplied through aliases or wrappers.
