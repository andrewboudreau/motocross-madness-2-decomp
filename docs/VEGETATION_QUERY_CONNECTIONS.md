# Vegetation query connections

This slice connects the vegetation count and radius calls in
`CollisionObject::QueryCollisions` to existing strict implementations, and uses
the existing `g_EcoSystem` pointer storage. It does not supply the still-missing
`CollisionVegetation::GetObject` method or admit the sample
`Vegetation::GetCollisionObject` implementation.

## Scope and declarations

| Old import | Existing provider | Retail extent |
|---|---|---|
| `CollisionVegetation::GetObjectCount()` | `Vegetation::GetCollisionCount()` | `0x457080`, 22 bytes |
| `CollisionVegetation::GetRadius()` | `Vegetation::GetRadius()` | `0x457230`, 24 bytes |
| `g_vegetationGrid` | `g_EcoSystem` | pointer cell `0x59aebc` |

`Vegetation::GetRadius` calls the complete strict 95-byte
`UnknownEcoDefinition::RadiusForParameter` at `0x455ff0`. There is no new
implementation, forwarding method, linker alias, storage or initialization.

The existing `UnknownEcoQuadTreeObject`, `UnknownEcoCoordinates` and
`Vegetation` definitions move verbatim into narrow headers. `EcoSystem.h`
includes each at its previous declaration position. Pointer-only dependencies
are forward declared. `EcoSystemGlobal.h` exposes the existing canonical
pointer declaration. This avoids importing EcoSystem's separate collision and
lighting views into the physics caller. It is a dependency boundary, not a
claim about recovered original headers or complete provisional-type identity.

The collision code retains its existing `CollisionVegetationGrid` view for the
single float at primary EcoSystem offset `0x5a8`, casting the canonical global
at that access. The unconnected `GetObject` call and other provisional
interfaces remain unchanged.

## Receiver and storage evidence

Raw RTTI establishes Vegetation's primary vtable `0x5524f8`, COL `0x55bef0`,
object offset zero, and nonvirtual QuadTreeObject base at zero. EcoSystem's
primary vtable `0x552508` has COL `0x55bf60` at offset zero. These are actual
primary pointers; no secondary GameObject adjustment is applied.

The type-ID checks are tied to their producers. CollisionObject construction
passes literal `Vegetation` at `0x5678f8` to the shared registry and stores the
result at `0x568611`. EcoSystem::Create passes the same literal and stores its
result at EcoSystem `+0x599`. Vegetation construction copies that byte to its
primary prefix `+8`. Placement writes the six-byte quantized position at
`+0xc`, and actual primary array elements are supplied to QuadTree::Insert.

QueryCollisions obtains the raw primary pointer from NextObject, checks its
`+8` type ID, then zero-extends the X and Z words at `+0xc` and `+0x10`.
It reads the existing global and multiplies by `+0x5a8`. Both radius calls and
both count calls receive the same unchanged primary pointer. The first radius
result is deliberately discarded, matching retail. Count uses the definition
byte at `+0x12`, the global definition table at `+0x58`, and its count at
`+0x204`. Radius also reads the parameter byte at `+0x15`.

EcoSystem construction stores its actual primary pointer to `0x59aebc` and
initializes `+0x5a8` to 1.0; destruction clears the same pointer. No second
singleton or substitute global is introduced.

## Complete preservation

The private same-path trial preserves all **501 nondebug sections and 2,618
relocations** in four complete objects under exactly the three import names:

| Object | Sections | Relocations |
|---|---:|---:|
| CollisionObject | 135 | 722 |
| CollisionObject near-miss aggregate | 136 | 732 |
| EcoSystem | 132 | 720 |
| EcoSystem near-miss aggregate | 98 | 444 |

Every payload byte, section flag, relocation kind, raw addend and resolved
section/offset destination is retained. The same-path proof has no filename
exclusions. Portable retained pins normalize only the four explicitly listed
complete source filename prefixes. Both EcoSystem objects need no applicable
symbol mapping.

The retained test passes **119 full strict comparisons**, including the full
1,102-byte QueryCollisions caller, both primary constructors, placement and
global lifecycle producers, both getters and the actual radius helper. Three
of these checks independently compare the exact source-extracted native
getter/helper bodies to retail; they also match the complete canonical
provider object's bytes and named relocations. The native object imports only
`__fltused` and owns the original extracted four-byte zero-filled pointer
definition, with no CRT initializer. The ten joined relocation sites are
checked across the canonical and aggregate query bodies.

Eight previous whole-object pins were independently reproduced before any
update. Only the two CollisionObject zero-storage pins change, under precisely
the three new external names; all six EcoSystem pins in the existing tree,
FilePrintf and clipper suites remain unchanged. All **26** affected existing
collision resolved partial fingerprints retain their complete extents,
position counts and digests. No partial score is relaxed.

A quoted-include audit finds only canonical EcoSystem.cpp and its near-miss
aggregate including EcoSystem.h; no other header includes it. CollisionObject
and its aggregate newly consume the narrow declarations. Final all-game
object preservation, actual link delta and publication are still required
before tracked integration.

## Native validation and limits

The executable uses the unchanged source-extracted 22-byte count getter,
24-byte radius getter and 95-byte radius helper, with actual headers and the
actual source global definition. No game helper is scripted, no instructions
are patched, and no retail code is dispatched. Complete strict equality is the
retail authority; native checks provide independent behavioral oracles.

**65,807 native cases** comprise:

- 65,536 radius cases covering every definition index and every parameter byte.
  Definitions have distinct finite integer radius ranges. An independent
  integer numerator divided by 128 supplies the expected exactly representable
  binary float, including negative radii and the 127/128 split.
- 256 count cases cover every definition index and eight count bit patterns,
  including zero, negative values, INT_MIN and INT_MAX.
- 15 null/guarded read cases independently require the exact read address,
  access type, AV exception and incomplete result. They cover receiver,
  global, table-entry and count/radius-field boundaries, including a fault
  after a preceding successful x87 load.

Every invocation checks the complete three prepared pages, all 256 definition
records and the shared pointer for unexpected writes. Ordinary cases require
successful completion and the independent expected return bits. The pointer
is zero before setup, as required by its actual source definition.

This is finite native VC6 behavior on prepared raw storage, not construction
or lifetime validation. The matrix uses masked floating-point exceptions,
round-to-nearest and 53-bit precision with exact moderate binary results. It
does not establish arbitrary floating-point values, unmasked traps, resumed
faults, threading, full QueryCollisions behavior or GetObject/shape-provider
closure. After guarded read faults the fixture resets x87 state as test
cleanup; it does not claim the transient register stack is preserved.

## Reproduction and real-link result

From the repository root, with verified `VC6_ROOT` and `MCM2_EXE`:

```powershell
$env:PYTHONPATH = '.;tests'
python -m unittest test_vegetation_connections -v
python -m unittest test_collision_object_zero -v
```

The same-path proof, eight old-pin reproductions, 26 unchanged partial digests
and frozen mirror hashes are in `work/connections55/vegetation`. The actual-path
retained checks pass. Real game build `build-q2_5u7lt` compiles all 262 translation
units and removes precisely the three joined imports: **219 -> 216**, with no
additions. All 262 prior production objects preserve 10,704 nondebug sections
and 45,813 relocations under exactly these three names, without filename
exclusions. The four affected canonical/aggregate objects independently retain
all 501 sections and 2,618 relocations at their actual source paths.

The source-built game remains unlinked; startup, integrated menus and racing
have not been tested. GetObject remains unresolved and is outside this slice.
