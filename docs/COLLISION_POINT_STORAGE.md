# Shared CollisionPoint vector storage

CollisionPoint's zero and Y-axis reads now refer directly to its existing
initialized objects. Two obsolete external names no longer leave unresolved
references. The source defines no additional vectors and adds no initializer,
translation unit, wrapper or linker alias. The separate normalization call-view
import remains unresolved.

## Source and identity

`src/krusty2/collision/CollisionPoint.cpp` already defined four constructor-bearing
vector constants. The zero object at retail `0x005797f0` and Y object at
`0x00579810` now have external const linkage, declared through the incomplete
`CollisionPointConstVec3` type in `CollisionTypes.h`. The full type and unchanged
copy-initialization expressions remain in their existing source file. The X/Z
objects remain local.

The point methods, shared inline constructor and three near-miss sample reads
use explicit `const CollisionVec3` views of those same 12-byte xyz objects. This
connects observed storage; it does not establish the original header vector type
or make all reconstructed vector classes interchangeable. Source-file attribution
and the initializer grouping retain the evidence recorded in CollisionPoint.cpp.

| Existing object | Retail VA | Emitted BSS offset | Initial value | Initializer thunk / body |
|---|---:|---:|---|---|
| `kCollisionPointZero` | `0x005797f0` | 32 | `(0,0,0)` | `0x0043b050` / `0x0043b060` |
| `kCollisionPointXAxis` | `0x00579800` | 48 | `(1,0,0)` | `0x0043b0a0` / `0x0043b0b0` |
| `kCollisionPointYAxis` | `0x00579810` | 0 | `(0,1,0)` | `0x0043b0f0` / `0x0043b100` |
| `kCollisionPointZAxis` | `0x005797e0` | 16 | `(0,0,1)` | `0x0043b140` / `0x0043b150` |

The table's BSS offsets describe the compiled source object, not retail absolute
addresses. All four offsets are unchanged. The complete BSS section remains
60 bytes, unbacked, with characteristics `0xc0400080` (writable uninitialized
data). The retail zero/Y addresses are also writable, unbacked storage before
CRT startup. C++ const linkage does not change that initialization phase.

All eight initializer functions remain strict: each 5-byte thunk reaches its
existing 60-byte body. The single 16-byte `.CRT$XCU` section retains flags
`0xc0300040`, zero addends and its original ordered entries
`_$E2`, `_$E5`, `_$E8`, `_$E11`. Each body still writes three DWORDs at offsets
0, 4 and 8 of the same object. In particular, the Y-axis is initialized to one
in its middle component during startup; it is not replaced by loader-zero data.

## Complete preservation

`tests/test_collision_point_storage.py` checks all 16 registered strict functions
in both the canonical and aggregate objects: **32 strict comparisons**. It also
pins both complete objects, including both near-miss functions, all data and CRT
sections, section flags, and every relocation location/type/addend/destination:
**62 nondebug sections and 239 relocations**. No partial body is shortened or
rescored to accept this connection.

Changing the variables' linkage changes COFF section placement and public names.
The comparison identifies sections by their defined symbols and exact offsets,
including static initializer names; anonymous sections retain kind and order.
Only the two old external storage references join the already-existing BSS
objects. No arbitrary targets are collapsed. Complete recognized `__FILE__`
literals are checkout-relative in the retained regression; actual same-checkout
before/after comparisons need no filename exclusion.

A private include-closure trial also compiled 55 affected production sources and
the point sample before and after: all **2,739 sections and 11,129 relocations**
were preserved under those two joins. Shadow checkout filename prefixes were
explicitly normalized for that private comparison. Its scope is recorded in
`work/contact-connections41/implementation/sources.json` and
`whole-object-proof.json`; it is separate from the parent's final composed game
object comparison.

## Actual source validation

The existing consumer fixture no longer rewrites zero/Y COFF relocations. Both
older bike-contact and scene-contact fixtures also removed their substitute
storage definitions and manual assignments. Their native startup checks read the
actual exported objects after the real source CRT initializers have run and
require all six expected float values.

The focused suites pass:

- `test_collision_point_consumers.py`: 26 strict checks and **39,557** actual
  source contact, mixed-array, alias, FP and fault comparisons.
- `test_bike_contact_connections.py`: 68 strict checks, seven unchanged complete
  partial fingerprints and **13,056** native allocation/contact cases.
- `test_collision_point_scene_connections.py`: 39 strict checks, two unchanged
  complete partial fingerprints and **19,541** native allocation/cache/transform
  cases.

These suites retain their documented finite domains and scripted boundaries.
The normalizer's validation-only ABI binding remains explicit in the consumer
fixture; this storage change neither supplies that production import nor expands
native coverage to an entire physics frame or game session.

The final combined source build `work/game-build/build-jduiaxj0` compiles all
240 units and reduces unresolved names from 284 to 282 for this isolated
storage step. The game still does not link; startup, menus and racing remain
separate completion requirements.

## Reproduce

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_collision_point_storage.py -v
python -m unittest discover -s tests -p test_collision_point_consumers.py -v
python -m unittest discover -s tests -p test_bike_contact_connections.py -v
python -m unittest discover -s tests -p test_collision_point_scene_connections.py -v
```

Private `work/contact-connections41/object_view.py` provides the complete-object
view for actual before/after build comparison. It performs no filename
normalization by default and reports every requested filename exception.
Private evidence is not required to run the retained tests.
