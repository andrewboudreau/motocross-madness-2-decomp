# Vegetation collision-object placement

`Vegetation::GetCollisionObject` selects a definition-owned collision object and
places its sphere, capsule or hull shape at the quantized vegetation position.
The complete retail method is **0x4570a0, 386 bytes**. The unchanged readable
candidate emits **384 bytes with 103 resolved matching positions** and remains
in `samples/ecosystem/VegetationCollisionObject.cpp`. Original source types,
semantic field names and translation-unit ownership are not established by this
extraction.

## Identity and complete control flow

The full function has three `ret 4` instructions at 0x457189, 0x4571c7 and
0x45721f. The last ends at 0x457221; the fourteen following NOPs precede
`GetRadius` at 0x457230. No instruction, cold path or table is trimmed.
The three direct calls are RadiusForParameter at 0x455ff0,
HeightForParameter at 0x455f90, and CollisionObject::SetTransform at 0x435830.
There are no indirect calls or exception handlers in the placement method.

Raw Vegetation primary COL/type-name evidence identifies vtable 0x5524f8,
COL 0x55bef0 and object offset zero. Its QuadTreeObject base is also at zero.
The constructor writes that primary vptr; EcoSystem::PlaceStoredObjects obtains
receivers from the actual vegetation array and calls their quantized placement.
The strict 1,102-byte CollisionObject::QueryCollisions retrieves the same primary
pointer from the broad-phase iterator, checks the registered vegetation type,
and passes it unchanged to 0x4570a0 at 0x439079.

The private combined slice changes the provisional `CollisionVegetation::GetObject`
call to `Vegetation::GetCollisionObject` through the genuine narrow Vegetation
header and an explicit primary receiver cast. Its full 1,102-byte caller remains
strict, with the sole new call relocation at caller offset 522. Complete
canonical and aggregate objects remain identical under that one additional name
before the separately reviewed transform repair. No full class equivalence is
inferred from the shared endpoint.

## Transform arithmetic repair

The method's hull path invokes the existing, nonexact SetTransform provider.
Its complete retail extent is **1,676 bytes**, including its switch table;
the source emits **1,684 bytes**. A private native comparison found a real
finite arithmetic discrepancy in the expanded hull matrix product.

For the first output element, retail adds product terms 2 and 1, then term 0,
then term 3. VC6 reassociates the previous flat source expression into terms 0
and 2, then term 1, then term 3. On the retained deterministic seed 256,
24-bit x87 precision and round-to-nearest, the world matrix's first float is
`0xbfbfc9e7` in retail and `0xbfbfc9e6` in the old source. The failure also occurs
when SetTransform is invoked directly with the exact prepared prestate and matrix
that the outer placement call supplies; it is not attributed to the outer loader.

The repair parenthesizes all sixteen sums in their decoded addition order.
It uses ordinary scalar C++ and the existing compiler flags. It adds no stores,
assembly, machine-code payload, runtime retail dispatch or tolerances. An
instruction-level check independently verifies all sixteen emitted addition
chains, allowing exchange of the first two added operands within this finite
domain. This is reconstructed observable arithmetic, not a claim that the
original source used these parentheses.

SetTransform stays nonexact: its resolved matching positions change from
**902 to 890**, still against the full 1,676-byte target. The decrease is explicit
and justified by the failing behavior control. Every other nondebug section in
both canonical CollisionObject and its sample aggregate is preserved. The old
source is retained as a negative control, with its complete original function
bytes pinned, and must fail at the same seed and exact differing float bits under
the current harness. It stops early; it does not execute the entire matrix.

The compiler also reschedules a later 20-byte window at function offsets
`0x358..0x36b`: extent-y/z copies and argument setup move relative to one another.
Accounting for the two pushes, both versions use the same stack destinations,
source reads, final integer registers and call at `+0x36c`. The remaining suffix
bytes and all relocation records are unchanged. This is not a claim of identical
exception-register or stack context at every intervening instruction.

## Real helper closure

The fixture links the separately compiled, unpatched placement candidate and
actual source parameter helpers, each strict over all 95 bytes and equal to its
canonical emitted body and relocations. It executes a full extraction of the
privately corrected canonical SetTransform, equal over all bytes and relocation
destinations, including the switch table. Internal compiler labels are compared
by their actual destination offsets.

Reached hull dependencies execute actual source:

- CollisionRelativeTransform, 215 bytes, and CollisionMatrixMultiply, 532 bytes.
- MoveBox, 789 bytes, and Vec3DotCall, 29 bytes.
- FastSqrt, 85 bytes, with its actual 97-byte table initializer.

All six are strict against their complete retail extents. CollisionLength
(74 bytes) and Vec3TransformPoint (100 bytes) are also strict linked closure but
are not reached by these hull cases. Radius/height add two further strict helpers.
SetTransform's model-only GetMatrixIn and UpdateModelBounds are explicit
fail-fast, unreachable fixture boundaries. They do not fabricate successful
results. The placement method never calls SetTransform for shape types 1 or 2.

The retail functions and their reached helper graph are unpatched. The actual
source initializer prepares FastSqrt's table, and the same bytes are copied into
the mapped reference table before testing. This does not establish complete CRT
initialization identity. A validation-only copy of the existing `g_EcoSystem`
pointer declaration is supplied to the private executable; production retains
its single canonical definition. Both runs receive the same prepared global
pointer and object addresses. No runtime callback or game-helper mock participates
in the executed paths.

## Finite native matrix

The retained private test passes **39,936 cases**, including **3,048 access
faults**, across three x87 precision settings and four rounding modes with all
floating-point exceptions masked:

- 18,432 successful sphere/capsule cases cover every radius-parameter byte,
  complementary height bytes, unsigned coordinate boundaries and negative,
  zero and positive quarter-unit scales.
- 24 shape-type 1/2 cases require successful return and untouched storage.
- 2,256 progressive sphere/capsule destination guards check complete readable
  page prefixes independently, in addition to full differential observations.
- 24 shape-pointer alias cases overwrite object+0x54 with the known radius-scale
  bits `0x3f800000`; the next reloaded shape pointer must fault at the independently
  expected height-field address. The candidate must not cache the old pointer.
- 18,432 hull cases include a direct SetTransform call with a simple target
  matrix, a direct call with the outer method's exact prestate and matrix, and
  the complete outer method. Half use identity rotations and exact binary-fraction
  translations; half use deterministic moderate finite matrix components.
- 768 hull guards stop the previous-body matrix copy at every byte boundary.
  Independent oracles include the outer method's prior y/z writes, every completed
  copied DWORD, the full readable page, the fault address and unchanged object
  storage.

The fixture compares all prepared ecosystem, definition, vegetation, collision
object, shape, tree-bound and page bytes; return pointers; exception kind/access
and address; and captured x87 status/control. Ordinary cases require completion
without an exception. Independent oracles additionally check parameter scaling,
quantized translation, unchanged bytes, primary return identity, body-matrix
translation and previous-body propagation. No float tolerance is used.

The prepared definition index is seven and the collision-object array has two
valid entries. All radius bytes are tested, but this is not every possible mean,
minimum or maximum definition value. Wider hull matrix components are finite
arithmetic inputs, not a claim that every tested matrix is a physically valid
rigid transform. NaNs, infinities, overflow, subnormals, arbitrary raw float bits,
unmasked exceptions and complete exception-register state/resumption are outside
the contract. Float status is captured before `_fpreset`; resetting before each
run and after captured observations prevents an access fault's live x87 operands
from contaminating the next run or oracle. This isolation is not game behavior.

Prepared raw objects do not establish complete C++ construction, destruction,
concurrent mutation or an integrated collision/gameplay session. There is no
scripted callback mutation claim: the reached parameter helpers are actual
nonvirtual code, and shape reloads are tested through observable storage aliasing.

## Complete object preservation

Same-path candidate extraction preserves the entire EcoSystem sample aggregate:
**98 nondebug sections and 444 relocations**, without filename exceptions.
The standalone has only the compiler directive section and its 384-byte text
section with five relocations. Its no-project-include compile and actual game
compiler command produce identical complete objects.

The private canonical CollisionObject repair preserves 134 of 135 sections;
its aggregate preserves 135 of 136. Across their 722 and 732 relocations, only
the complete SetTransform section changes. All other method bodies, data,
initializers, CRT entries, flags, symbol offsets and relocation destinations are
unchanged under the one explicitly proven GetObject/GetCollisionObject name
connection. The combined four-object comparison covers 501 sections and 2,618
relocations; only the two complete SetTransform sections differ. Retained portable
pins normalize only independently enumerated checkout-local diagnostic filenames.

## Functional build review

The candidate is explicitly admitted as reviewed nonexact in `config/game_build.json`.
Real game build `build-9s7mr4ai` compiles all 264 translation units and removes
exactly the previously unresolved `CollisionVegetation::GetObject` name:
**214 -> 213**, with no additions. The caller uses the genuine Vegetation
declaration at the independently verified primary-pointer boundary.

All 263 prior production objects account for 10,707 nondebug sections and
45,813 relocations. Under that single call-name connection, only the complete
SetTransform section changes; its section flags and relocation records remain
identical. There are no filename exclusions. The four affected canonical and
aggregate objects account for all 501 sections and 2,618 relocations: only the
two SetTransform copies differ. Actual-path old fingerprints were reproduced
before applying the scoped retained-test updates.

The actual game-command candidate has two complete sections and five relocations,
and its complete body equals the extracted aggregate method. Five focused suites
pass at the actual paths: runtime 13 strict checks/39,936 cases/3,048 access faults;
getters 119 strict/65,807 native cases; existing zero storage 18 strict; query
193 strict/31 complete partial fingerprints/10,535 native cases; and vector
constructors 355 strict/28 complete partial fingerprints/72,240 native cases.

The full matrices retain every prior strict result. The only partial-score
decrease is the explicitly reviewed SetTransform correction, 902 to 890 resolved
positions in the canonical physics row. Retained tests separately pin the same
correction in both canonical and aggregate objects. It is not a strict match.
No compiler flags change. The game still does not link, and startup, integrated
menus and racing remain unverified.

Reproduce with authentic private inputs:

```powershell
$env:VC6_ROOT = '<repository>/work/vc6sp3-disc'
$env:MCM2_EXE = '<repository>/work/game/mcm2.exe'
$env:PYTHONPATH = '.;tests'
python -m unittest test_vegetation_collision_object test_vegetation_connections test_collision_object_zero test_collision_query_connections test_vector_constructors -v
```
