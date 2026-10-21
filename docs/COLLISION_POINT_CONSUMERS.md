# Collision-point consumers

The physics-step and vehicle callers now use the existing CollisionPoint
implementations at three verified call boundaries. No new algorithm, wrapper,
linker alias or replacement object layout is introduced.

| Caller | Retail call site | Existing provider | Complete provider bytes |
| --- | --- | --- | --- |
| SoultreePhysicsBaseObject slot 31 | `0x501e70` | CollisionPoint::UpdateRelativeMotion, `0x43a640` | 1003 |
| SoultreePhysicsBaseObject::RunSteps | `0x5030d6` | UpdateCollisionPointWorldPositions, `0x43aff0` | 88 |
| Vehicle slot 49 | `0x52ae13` | UpdateCollisionPointWorldPositions, `0x43aff0` | 88 |

Both providers already exist in `src/krusty2/collision/CollisionPoint.cpp` and
remain complete strict matches. `CollisionPoint.h` now declares the existing
free function. The two callers include that header after their existing
includes. Source ownership is unchanged. The reduced SoultreeContact and
VehicleContact views remain provisional; these joins do not identify every
field or establish complete class equivalence.

## Point-relative receivers and array producers

The physics base holds a point-pointer array at `+0x12c` and a count at
`+0x130`. Slot 31 loads each stored pointer directly into ECX, checks owner
`+4` and penetration `+0x98`, and updates contact state `+0xb4`. Its call at
`0x501e70` passes four 12-byte xyz vector pointers and a float, returning void
with `ret 20`. The explicit casts preserve the same receiver and all argument
addresses. In particular, the fourth argument aliases the point's `+0x38`
relative-position field. UpdateRelativeMotion writes that field before reading
the fourth vector; no temporary vector copy is introduced.

Retail Bike::LoadBike calls AddCollisionPoint at `0x4089c2`, `0x4089fb` and
`0x408a38`, using this same array and count. The complete 565-byte producer at
`0x43a330` allocates `0xc4` bytes, initializes the CollisionPoint vptr to
`0x5511d8`, stores the returned primary pointer into `points[*count]` at
`0x43a51c`, assigns owner `+4` at `0x43a550`, and increments the count at
`0x43a55d`. These allocation and storage instructions establish receiver
identity independently of shared function addresses.

The array also accepts secondary CollisionPoint subobjects. Retail Vehicle's
160-byte AddWheel adds `0xb8` to a nonnull wheel at `0x525d7d`, then passes
that adjusted pointer and the same array/count to AddExistingCollisionPoint
at `0x525da1`. The 109-byte producer at `0x43a570` stores that pointer and
clears its owner field. No additional adjustment belongs at the consumers.

Raw Tire RTTI supplies independent multiple-inheritance evidence: the secondary
CollisionPoint table `0x5584fc` has COL `0x55f530`, literal type name
`.?AVTire@@`, and object offset 184. The CollisionPoint base descriptor
`0x55f560` has PMD `(184, -1, 0)`. Tire's constructor writes CollisionPoint's
base vptr at `[this+0xb8]` at `0x512f66`, then Tire's derived secondary vptr
there at `0x513135`. Its primary CollisionObject and secondary GameObject
views are separate. Thus the array can contain both standalone primary points
and already-adjusted secondary point pointers.

UpdateCollisionPointWorldPositions takes the original array address and count
without copying or advancing them in the caller. It skips points without an
owner or active contact, then updates world position through the existing
scene transform. Slot 31 similarly skips ownerless wheel points and clears
their contact state. This scope does not reconstruct the separate Tire path.

## Complete object preservation

The only imported-name substitutions are:

```text
?UpdateRelativeMotion@SoultreeContact@@QAEXPAUVec3@@000M@Z
  -> ?UpdateRelativeMotion@CollisionPoint@@QAEXPBUCollisionVec3@@000M@Z
?SoultreeResetContacts@@YAXHPAPAVSoultreeContact@@@Z
  -> ?UpdateCollisionPointWorldPositions@@YAXHPAPAVCollisionPoint@@@Z
?VehContactsC@@YAXHPAPAUVehicleContact@@@Z
  -> ?UpdateCollisionPointWorldPositions@@YAXHPAPAVCollisionPoint@@@Z
```

A same-path private trial first preserved eight complete caller/provider/header
consumer objects. After applying the changes, canonical-path compilation was
compared against the actual published baseline `build-6bsh80s7`: **828 nondebug
sections and 4,016 relocations** remain unchanged across SoulTreePhysics,
Vehicle, CollisionPoint, CollisionObject, Bike, ConstraintMethodCollisionModel,
CollisionCharacter and BoundingBoxTreeQuery. Comparison includes section
characteristics, every raw byte and relocation addend, relocation type/site,
and destination identity. Defined local destinations use section plus offset;
only the three external names above are substituted. Debug records are excluded.
No raw-byte or filename exception is used in the canonical-path comparison.

The complete 127-byte slot 31 and 1,921-byte RunSteps callers remain strict.
Vehicle slot 49 remains a nonexact 3,254-byte body with 3,229 matching positions;
its full bytes and relocation destinations are unchanged. No partial body is
trimmed, rescored or repinned to hide a difference.

The gearbox regression also fingerprints external symbol names. Its slot 49
expectation is updated for the approved world-position provider name at the
REL32 relocation at body offset 1,236. Independent comparison preserves all
3,254 raw bytes and all 56 relocation records under that single substitution;
the resolved fingerprint, extent and matching score are unchanged. The private
`gearbox-fingerprint-review.json` records both raw-name digests and the sole
renamed relocation.

The integrated real build `build-3jal2tv5` compiles all **235** source units and
reduces unresolved names **307 to 304**, removing precisely those three imports
and adding none. Its complete old-object comparison preserves **10,582
nondebug sections and 45,548 relocations** under the same three substitutions.
That is source/link evidence; the game still does not link or run.

Private reproducible evidence resides in `work/contact-connections38/`:
`renames.json`, `objects.json`, `live-objects.json`, `focused-functions.json`,
`prove_live_objects.py`, `check_functions.py`, and the retail producer/RTTI
instruction reports. Native boundary validation is recorded separately by the
focused collision-point-consumer regression. Its small native executable uses
an explicit validation-only mapping for the existing CollisionVec3 normalization
call view. The zero/Y-axis constants now use ordinary source linkage to the
actual existing CollisionPoint objects and their original CRT initializers; see
[shared contact storage](COLLISION_POINT_STORAGE.md). The normalizer mapping is
not a production connection. Strict matching of the complete providers does not
establish closure of every remaining dependency.

## Native validation

`tests/test_collision_point_consumers.py` compiles the real source objects with
authentic VC6 SP3. The native executable uses the complete CollisionPoint object,
an unchanged extraction of slot 31, and the actual math and scene helpers.
The extracted slot 31, CollisionLength, UpdateWorldMatrix and LocalToWorldPoint
bodies are checked against both their full production objects and retail.
There are **26 complete strict checks**, including the two contact producers,
the zero/Y-axis initializer bodies and thunks, and the complete strict callers.
The full nonexact Vehicle slot 49 retains its baseline resolved fingerprint.

The fixture compares **39,557** finite scenarios:

| Cases | Scope |
| --- | --- |
| 32,768 | Slot 31 and world-position refresh: 64 input seeds, counts -1 through 6, 16 owner/contact/cache modes, and duplicate-entry variants |
| 6,144 | Direct relative-motion calls: 64 input seeds, eight argument-alias arrangements, four rounding modes and three x87 precision modes |
| 640 | Invalid receiver, list, point, owner and vector addresses, including progressively readable contact prefixes ending at a protected page |
| 5 | Independent expected outcomes: active/state-2/ownerless handling and recomputed lever arm, empty-list early exit, null-point access fault, inactive-point preservation, and a known translated world position/cache update |

Point arrays mix standalone prepared storage with pointers at `+0xb8` into
Tire-sized prepared storage. A separately compiled native layout probe using
the actual Tire declaration confirms that static conversion to CollisionPoint
adds `0xb8`; raw retail RTTI, constructor writes and array-producer instructions
provide independent identity evidence. No Tire or physics-object constructor,
destructor, virtual-base lifetime or complete provisional-view equivalence is
claimed. Odd-index secondary points are also exercised with prepared nonnull
owners to test the generic pointer boundary; that does not claim the normal
wheel producer initializes owners that way.

Both runs use the same deterministic addresses. Comparisons include the entire
physics receiver, point and scene-node storage with surrounding guards, pointer
array, vector arguments, writable guard-page prefix, post-call floating-point
status/control, and access-fault kind/address plus the six x87 exception-status
bits at the fault. The fourth slot-31 argument aliases the relative-position
field that the provider first overwrites. Direct provider cases additionally
alias all four inputs with normal, position, direction and slip fields. All
floating-point exceptions remain masked; unmasked FP faults, complete saved
register contexts and execution resumed after faults are outside this test.

The private retail image is hash-pinned. Its slot-31 call still executes the
complete retail relative-motion body; neither tested provider is replaced.
Only decoded direct calls from the two retail providers are redirected to the
verified source helpers: vector construction, rejection, normalization, dot,
length/division, FastSqrt/FastInvSqrt, and LocalToWorldPoint. Actual strict table
initializers run before the matrix. Scene helpers perform their actual cache and
matrix operations, with no scripted math or virtual callbacks.

The native link has one explicit validation-only accommodation. The existing
CollisionVec3 normalization import is bound to the actual 150-byte Vec3 provider,
whose hidden-output pointer and source pointer occupy the same cdecl stack words
and whose return register is the same output pointer. That change affects only
private COFF symbol metadata; every section payload and instruction byte remains
unchanged. The zero/Y-axis constants use actual exported source storage with no
private relocation editing or fixture definitions. The test checks their storage
destinations, all three destination addends, strict initializer bodies/thunks and
`.CRT$XCU` links. The normalizer remains an unresolved production dependency.
Allocation entry points are fail-fast and
unreached here; the existing bike-contact suite covers real source allocation
and removal, and the existing scene-contact suite covers the larger independent
transform/cache matrix. Full RunSteps and Vehicle-frame execution is not part
of this native fixture.

Reproduce the focused test and the complementary existing provider suites:

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_collision_point_consumers.py -v
python -m unittest discover -s tests -p test_collision_point_scene_connections.py -v
python -m unittest discover -s tests -p test_bike_contact_connections.py -v
```

## Remaining scope

The old provisional collaborator declarations are retained to preserve compiler
input order; no implementation is supplied for those old names. No shared
vector class identity is inferred from compatible xyz layouts. The adjacent
VehAddContact join remains deferred: its provisional integer threshold and the
canonical float parameter require an independently proved bit-preserving call
boundary, not a numeric cast. Integrated contact scheduling and racing remain
unverified.
