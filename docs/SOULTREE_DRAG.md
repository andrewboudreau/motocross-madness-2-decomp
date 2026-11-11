# Physics slot 6: magnitude and budget update

`src/reconstructed/SoultreeDrag.cpp` reconstructs the complete
`SoultreePhysicsBaseObject::ApplyDrag(Vec3*, float*)` body at
`0x0040b410`. It is a strict byte match under `vc6_o2_mt`. The member names and
drag/budget interpretation remain provisional. Original translation-unit
ownership is not established: the body lies in Bike.cpp's address range and is
most likely an inline member kept from the first unit that emitted it. The file
is a reconstruction grouping like `SoultreePhysicsMethods.cpp`, which also holds
slot 36.

## Identity and complete extent

The retail body is **432 bytes**, through `ret 8` at `0x0040b5bd`. The next
method starts at `0x0040b5c0`. All branches remain within this body; its sole call
is `0x0040b4b3 -> 0x00460b50` (`FastSqrt`). It has no indirect calls, exception
registration, allocations, locks, or callbacks.

The regression checks the actual primary slot 6 entries, their COL object
offsets and raw type-descriptor names, and the table metadata independently:

| Class | Primary vtable | Object offset |
| --- | --- | --- |
| SoultreePhysicsBaseObject | `0x557d90` | 0 |
| SoultreePhysicsObject | `0x557f54` | 0 |
| SoultreePhysicsCharacter | `0x5580a8` | 0 |
| Vehicle | `0x558a84` | 0 |
| Bike | `0x550900` | 0 |
| KrustyBike | `0x554548` | 0 |

The physics substep loop `RunSteps` (`0x502f60`, 1921 bytes) invokes slot 6 at
`0x5030a0`; Vehicle slot 49 (`0x52a940`, 3254 bytes) invokes it at `0x52ad5d`.
Both prepare a vector from `weightForce` and a budget from `prevSpeed`, then
call slots 8, 6, and 7 in that order. These are primary receiver calls; they do
not use the separate character or virtual GameObject subobjects. The missing
method is referenced by Bike, KrustyBike, SoulTreePhysics, and Vehicle objects:
one missing provider across four translation units.
The local regression decodes both complete caller extents to verify those call
sites. Full caller preservation is covered by the repository's complete
calibration/physics gates and whole-object comparison, not by those two local
call-site assertions alone.

## Corrected behavior

The old candidate computed squared magnitude, called `FastSqrt`, tested the
result, then discarded it. Its subsequent scaling and budget charge incorrectly
used squared magnitude. The original comment's claim that retail deliberately
used squared length was wrong.

Retail first stores squared magnitude at `[esp+8]` (`0x40b489`). After pushing
the argument and calling `FastSqrt`, `fst [esp+0xc]` at `0x40b4b8` writes to
**the same stack slot**: the argument is still on the stack. `add esp,4` follows
at `0x40b4c2`. The later comparisons and divisions load the overwritten float.
The source now assigns `length = FastSqrt(length)`. It retains the rest of the
operation, including the unscaled branch's budget write before copying
`scratchVector`, and the sequential x/y/z accumulation into the caller's output.

A native counterexample had speed and budget `0.0001f`, velocity `(0,0.75,0)`,
unit mass/time/drag, and previous speed 1. The old candidate left budget bits
`0x38d1b412`; retail left `0x37d2dc5c`. The corrected candidate matches retail.
An independent exact-value oracle also distinguishes the behaviors: speed 1,
velocity `(3,0,0)`, mass 8, time 1, budget 8, drag 1, and initial output `(1,2,3)`
must produce output `(-2,2,3)` and budget `7.625f`.

The repair deliberately changes a previously partial candidate:

| Version | Emitted bytes | Full retail bytes | Strict matching positions |
| --- | --- | --- | --- |
| Original candidate | 432 | 432 | 287 |
| Corrected candidate | 440 | 432 | 126 |
| Reference and clamp forms | 432 | 432 | 426 |
| Current source | 432 | 432 | 432 (exact) |

The source reads the magnitude through a `scratchVector` reference (retail's
`[esi]` loads) and clamps with `*speedBudget < charge` (retail's `fcom`). The
scaled branch builds its product in a named temporary, charges the budget and
then assigns it to `drag`:

```cpp
Vec3 scaled = scratchVector * (limit / length);
*speedBudget -= charge;
drag = scaled;
```

VC6 then schedules the budget `fsub` before the copy's z load, as retail does.
Assigning the product to `drag` directly leaves the `fsub` after it (426/432).
Subtracting before computing the product is worse (164/434).

The complete extent is never trimmed. The source also now naturally
stores and reloads the squared magnitude before testing equality with 1, as
retail does. No unrelated method was changed to obtain this emission.

## Native validation

`tests/test_soultree_drag_behavior.py` compiles the canonical source with
authentic VC6 SP3 `/O2 /MT`, separately from the fixture, and requires a strict
match. It also compiles the sample aggregate, which no longer defines slot 6,
and the existing `FastMath.cpp`. Both `FastSqrt` (85 bytes) and
`InitFastSqrtTable` (97 bytes) are independently strict against retail. The
native fixture executes the real initializer and real table-driven helper.

The private comparison maps the hash-pinned retail image and changes only the
verified direct call at `0x40b4b3` to the same strict FastSqrt provider used by
the source candidate. There are no scripted game dependencies. This mapping
exists only in the test executable; production uses ordinary compiled C++.

**78,305 scenarios** compare the whole prepared receiver and guards, output,
budget, guard-page storage, floating-point status/control, and access-violation
kind/address plus partial state:

- Seven independent expected-outcome checks cover both budget branches, the
  magnitude correction, a negative budget, a threshold early return with invalid
  outputs, a visible first output write before the next read faults, and a
  read-only budget that faults before output accumulation.
- 6,912 finite input combinations vary speed, budget, vectors, mass, and time.
- 3,906 combinations cover 31 alias arrangements, seven paths, and 18 pointer
  modes. Output and budget may overlap each other or receiver fields. Guard
  pages test progressive output access and nine progressively valid receiver
  prefixes; a read-only page checks budget write ordering.
- 1,944 cases exercise selected signed zeros, subnormal/normal boundaries,
  threshold/unit neighbors, maximum finite values, infinities and NaNs under
  four rounding modes and three x87 precision modes.
- 32,768 deterministic moderate finite cases and 32,768 finite raw-bit cases
  broaden arithmetic and branch coverage. The latter may produce nonfinite
  intermediate results, which are compared as bytes and floating-point status.

Floating-point exceptions are **masked** throughout. Unmasked FP traps,
exhaustive float/alias combinations, constructors and object lifetime,
concurrent mutation, and integrated physics simulation are not tested. The
fixture uses prepared raw storage and does not claim to instantiate a complete
live game object. Native equality over this finite set is not whole-program
equivalence.

## Preservation and dependencies

The source imports only `FastSqrt` and the normal CRT `__fltused` marker.
It adds no game-owned dependency beyond an existing strict provider. Slot 36
and its unsupplied orientation-vector global remain outside this candidate.

The unchanged Math3D header emits four local vector initializers and associated
storage in this source unit. The regression compares their eight complete
initializer functions with the sample aggregate, checks its four-entry `.CRT$XCU`,
and validates referenced float literals against retail. Their emission is not
evidence for an original translation-unit assignment or whole-object retail
identity. They are ordinary private source globals, not aliases to retail data.

Slot 36 keeps all 305 emitted bytes and eight relocations in the sample
aggregate (pinned by the regression). The exact methods regression
(`tests/test_soultree_physics_methods.py`) checks slot 6 strictly against the
primary vtable entry, together with the other promoted slots.

### Build admission

`src/reconstructed/SoultreeDrag.cpp` is an ordinary source of the game build
(`config/game_build.json` source root `src`). It replaces the earlier
reviewed-nonexact sample admission. The byte match and the finite native
validation above are separate claims. Neither establishes the original TU, and a
linked game, startup and racing remain separate checks.

Reproduce the retained native and neighboring strict checks on Windows:

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_soultree_drag_behavior.py -v
python -m unittest discover -s tests -p test_soultree_physics_methods.py -v
```
