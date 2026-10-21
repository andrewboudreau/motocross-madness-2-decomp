# Shared physics math providers

Four reconstruction units now supply seven previously sample-only function
bodies. These paths are organizational names, not recovered retail filenames.
Types and semantic names remain inferred from arithmetic and decoded callers.
The target addresses, extents and relocation-resolved instruction bytes are
confirmed against the verified demo executable.

| Canonical source | Function | Retail VA | Bytes |
|---|---|---:|---:|
| `src/reconstructed/OrientationAngles.cpp` | `OrientationAnglesFromVectors` | `0x004b5a60` | 657 |
| `src/reconstructed/AxisSettle.cpp` | `UnknownAxisSettle_4cb6e0` | `0x004cb6e0` | 1058 |
| `src/reconstructed/CrossProduct.cpp` | `CrossProductCall` | `0x00515600` | 91 |
| `src/reconstructed/Math3D.cpp` | `MatrixZero` | `0x004a13e0` | 41 |
| `src/reconstructed/Math3D.cpp` | `IntegrateAdamsBashforth2` | `0x004a1d10` | 71 |
| `src/reconstructed/Math3D.cpp` | `QuatDerivative` | `0x004a1f90` | 169 |
| `src/reconstructed/Math3D.cpp` | `Vec3Normalize` | `0x005087b0` | 150 |

Existing callers already use four of these exact C++ signatures. KrustyBike
imports the by-value vector angle extractor and the pointer-based axis update;
Vehicle imports the cross product and normalization functions with hidden
`Vec3` return storage. No caller aliases or function signatures were changed.
The three other functions have no currently compiled caller using these exact
names. This slice therefore supplies four currently unresolved names, rather
than claiming seven new connections. `Vec3Normalize` calls the existing
`FastInvSqrt` provider; the remaining external dependencies are VC6 runtime
`__CIacos` and `__fltused`.

The source preserves the angle extractor's three null-pointer checks, its
degenerate-vector branches and near-vertical correction; the other four output
pointers are not checked. The axis helper preserves each axis's operand order,
mask handling and zero/sign cases. Normalization returns an exactly unit vector
unchanged and otherwise uses the original reciprocal-square-root routine.
These are complete bodies, not replacements with simplified edge behavior.

## Initializer and ownership boundary

All four units include the existing `Math3D.h`, which emits four local `Vec3`
constants and four `.CRT$XCU` entries per unit. The constants and initializer
functions have COFF static storage class, so different units do not define
competing external symbols. The original header dependencies and declaration
order are preserved.

AxisSettle's eight initializer thunks/bodies also match retail exactly at
`0x004cbb10..0x004cbc10`, with bindings for vector storage at
`0x00689970..0x006899a0`. Their grouping with the axis function is a strong
inference from the adjacent initializer set and surrounding units; no literal
source name or RTTI confirms original ownership. The other three units' local
initializer copies have no assigned retail ownership. In particular, Math3D
groups functions from separate retail ranges. Seven exact function bodies and
the eight exact AxisSettle initializer functions do not establish whole-unit
retail identity or original initializer ordering in a linked executable.

The existing physics manifests now route all fifteen checks to canonical
source. `tests/test_physics_math_providers.py` compiles the real providers and
two actual caller units with VC6 SP3, verifies the fifteen independent function
extents with every relocation resolved, checks the four supplied caller names,
checks the reciprocal-square-root dependency, and verifies local initializer
scope. No executable code is imported from retail into the production build.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_physics_math_providers.py -v
```

The complete physics matrix and real source compile/link remain separate
integration gates. Other unresolved vector and quaternion aliases are outside
this promotion.
