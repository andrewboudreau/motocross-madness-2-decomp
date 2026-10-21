# Bike contact-list connections

`Bike::LoadBike` and `Bike::~Bike` now call the existing `AddCollisionPoint`
and `RemoveCollisionPoint` providers directly. The retired local function
names described the same decoded allocation/list boundaries with provisional
pointer types. Explicit casts preserve those addresses without wrappers or
new method implementations.

| Provider | Retail VA | Complete strict extent | Bike calls |
| --- | --- | --- | --- |
| `AddCollisionPoint` | `0x0043a330` | 565 bytes | Three in the loader |
| `RemoveCollisionPoint` | `0x0043a5e0` | 85 bytes | Two in the destructor |

The allocation helper's source attribution is anchored by retail's
`CollisionPoint.cpp` string at `0x00568730`. Its complete body inlines the
constructor, allocates `0xc4` bytes, stores the new primary pointer into the
list, copies the local position, records owner/threshold/load parameters,
and increments the count. Its final integer parameter retains the 32-bit
track/surface-owner address. The loader's two `BikeA5F8*` results and integer
tail-contact handle keep the same pointer bits. Native layout checks establish
the accessed `BikeA5F8`/`CollisionPoint` offsets `0x14`, `0x2c`, `0xa4`, and
`0xb4`, and the three 12-byte vector components. They do not claim equivalence
of the complete provisional classes.

The wheel contact is a **secondary** subobject. Retail Tire RTTI reached
through vtable `0x005584fc` gives object offset `0xb8` and the nonvirtual
CollisionPoint PMD `(0xb8, -1, 0)`. The destructor passes the existing
`wheel + 0xb8` address unchanged. Removal shifts the tail of the pointer list,
clears its vacated slot, and stops after the first match; it does not delete
the embedded contact. The shared header now declares this already-defined
helper alongside `AddCollisionPoint`.

## Verification

`tests/test_bike_contact_connections.py` verifies the private image hash,
RTTI, all five direct caller destinations, provider COFF imports, the two
complete strict providers, and native layouts. It preserves 68 strict
provider/caller cases and all seven complete Bike partial fingerprints,
including the 6,745-byte loader candidate (6,428 matching positions). No
partial body is admitted or relabeled exact by this connection.

The native fixture links the actual complete production `CollisionPoint.obj`.
It executes both helpers against the private retail bodies in 13,056 cases:
capacity rejection, successful allocations, null allocation, allocator
exceptions, pointer identity, initialized object storage, duplicate/null/list
entries, missing removals, and vacated-slot clearing. Floating-point inputs
include signed zero, finite values, infinities, and NaNs. Independent expected
outcomes cover ordinary construction, first-match removal, and both failure
paths. A null allocation preserves retail's write fault at address `8` rather
than adding a guard.

Only retail's verified debug-allocation call at `0x0043a350` is redirected.
The fixture supplies the known zero/y-axis global values and normalizes the
single stored vtable address after checking the expected retail/production
identity. All other allocation/list bytes, returned pointer, call parameters,
exception details, and x87 status/control are compared. Unused dependencies
needed to link the full object fail fast. Floating-point exceptions are
masked; each run resets the x87 stack as an allocation fault may interrupt
the function with live stack values. This is a helper-boundary test, not a
complete motorcycle simulation.

The isolated real build compiled 209/209 translation units and changed
458 unresolved names to 456: exactly the two retired Bike names disappeared,
with none added. All 209 pre-existing objects preserved their complete
10,427 nondebug sections and 45,122 relocations under those two name
substitutions. Comparison includes raw bytes, padding, relocation addends,
types, and destinations; compiler-local targets use destination-section
identity and offset. Private reports are under
`work/game-build/contact-round22/`.

With private `VC6_ROOT` and `MCM2_EXE` configured:

```powershell
python -m unittest discover -s tests -p test_bike_contact_connections.py -v
```
