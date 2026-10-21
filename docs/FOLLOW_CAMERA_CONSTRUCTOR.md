# FollowCamera construction

`samples/camera/FollowCameraConstructor.cpp` is a separate readable candidate
for the missing `FollowCamera::FollowCamera(int)` provider. It preserves the
genuine existing class declaration and fixes an observable partial-write
ordering discrepancy. Original translation-unit ownership remains unproven.

## Identity, extent and source status

The complete retail function occupies 564 bytes at `0x00462ee0`, ending with
`RET 4` at `0x00463111` through `0x00463113`. Twelve padding NOPs precede the
next function at `0x00463120`; they are not part of this constructor. The two
direct calls are `0x00462f03 -> 0x004bed80` (PCCamera construction) and
`0x0046302e -> 0x00466e50` (FollowCamera slot 71).

Raw COL type names and base descriptors establish the zero-offset,
nonvirtual chain VehicleCamera, FollowCamera, PCCamera, Camera, GameObject,
BaseObject. The FollowCamera primary table is `0x0055282c`; the existing
declaration has size `0x384`. The complete strict 137-byte VehicleCamera
constructor at `0x0052b920` passes its unchanged primary receiver and flags
to this constructor, then installs its own table at `0x00558c30`.
This supports the constructor call boundary independently of a shared address.
Individual semantic field names remain reconstructed names.

The original constructor in `FollowCameraNearMisses.cpp` remains unchanged,
including its existing 564-byte candidate and 511 matching resolved positions.
The new candidate emits **569 bytes with 100 matching resolved positions**
against the complete 564-byte retail function. Neither candidate is strict.
The original sample and its calibration rows are not replaced or silently
rescored.

The retained negative control compiles that original constructor and runs the
same current fixture. It must fail with exit 36 at the independent partial-write
oracle for `0x2c4` writable receiver bytes. The original compiler schedule
writes later fields before the retail `points` write at `+0x2e4`, changing both
the first fault and the object prefix left behind. The new source uses a small
set of explicit volatile accesses to model the decoded write order. This is
an observed-behavior reconstruction, **not evidence that the original members
had volatile types**. The normal values and original sample remain unchanged.

## Actual initialization and unwind closure

The native test compiles actual GameObject, BaseObject, MemTag, MatrixUtil,
GraphicsState, FastMath and VectorCalls source. Camera, PCCamera and FollowCamera
validation units retain the actual constructor, destructor and reached slot
bodies. Their unrelated virtual bodies terminate the test immediately if
reached; these validation-only boundaries are not game implementations.

The 27 complete strict checks comprise:

| Source behavior | Retail extents |
| --- | --- |
| Camera constructor/destructor | 402 / 37 bytes |
| PCCamera constructor/destructor | 25 / 5 bytes |
| GameObject constructor/destructor/AppendClassName | 149 / 97 / 201 bytes |
| BaseObject constructor/destructor | 16 / 7 bytes |
| DebugMalloc/DebugFree/DebugRealloc/DebugMemcpy | 57 / 96 / 256 / 35 bytes |
| IdentityMatrix | 71 bytes |
| VehicleCamera constructor caller | 137 bytes |
| FollowCamera slots 58, 61, 66, 71 | 1 / 1 / 41 / 192 bytes |
| Four initializer wrappers and their four bodies | 5 / 60 bytes each |

Extracted helper comparisons retain complete bytes, relocation destinations
and associated EH data. Compiler-local labels are compared by actual section
contents and offsets, not accepted merely because their names differ.

The reference constructor runs from the hash-pinned retail image in the private
test executable. Its two direct calls and three relevant virtual entries are
redirected to the same verified source helpers used by the candidate. Its EH
handler jump is checked before redirection to the linked VC6 runtime, and its
cleanup jump is checked before redirection to the actual PCCamera destructor.
The test flushes the instruction cache after these reference-only patches.
This is a shared-strict-helper differential test, not an untouched retail
helper graph. Production source performs no retail mapping or dispatch.

The raw retail unwind record has one state. The first `isPlaced` write at
`+0x275` occurs before the state becomes zero. A fault on that write leaves
the allocated name and FollowCamera vptr in place. Later tested write faults
run the actual destructor chain, free the name and end with the BaseObject
vptr. Both outcomes have independent ownership and prefix-state assertions.

## Allocator boundary and known excluded hazard

The complete MemTag object is checked against a validation build under exactly
three CRT import substitutions: `malloc`, `free` and `_msize`. The wrappers call
the actual linked native CRT for successful allocations, frees and size queries;
they record requests and ownership and can return NULL or throw at a selected
allocation boundary. They do not enlarge allocations or replace game allocator
logic. Full game helper bytes and relocations remain unchanged under those
three substitutions.

Normal construction requests 1 byte, then 12 bytes for `GameObject,` and its
terminator, frees the first block and retains the second. Requested-size versus
actual-size accounting is compared as implemented, including negative deltas
where applicable. Test cleanup releases surviving allocations only after the
observable state has been captured.

The finite failure matrix covers first-allocation NULL and exceptions at each
of the two allocation boundaries. A throw at the second boundary leaves the
first allocation alive; that observed leak is preserved and independently
checked. These throws are scripted CRT-boundary behavior, not a claim about
ordinary native `malloc` throwing this exception.

**Second-allocation NULL is excluded from native execution.** The actual
DebugRealloc failure path returns the original allocation. AppendClassName then
needs 12 bytes, whereas the actual CRT extent of the initial 1-byte request can
be only 8 bytes. Running that path would overwrite the CRT heap. This is a
decoded hazard in the existing behavior, not a validated successful allocation
case. No oversized replacement allocator or production repair hides it.

The linked CRT backend is shared by both sides. Complete identity of the retail
CRT allocation, string and exception-runtime internals is not claimed.

## Native validation

The retained test passes **7,392 scenarios** across all twelve combinations of
masked x87 exceptions, 24/53/64-bit precision and nearest/down/up/chop rounding:

| Matrix | Cases | Independent assertions |
| --- | ---: | --- |
| 32 prepared byte patterns and eight flag values | 3,072 | Complete initialized/unwritten object bytes, return value, vptr, name and allocation lifecycle |
| Every writable size from `0x220` through `0x384` | 4,284 | Full guarded page, decoded store prefix, exact first fault address and access kind, completion and unwind ownership |
| First allocation NULL; throws at first and second allocations | 36 | Partial base state, exact failure classification, vptr, allocation requests/events, retained/leaked blocks and accounting |

The guarded-size matrix contains 3,504 access violations. First-allocation NULL
adds twelve access violations; the other allocation cases produce 24 scripted
C++ exceptions. Ordinary successful cases explicitly require completion.

The independent expected object is built from field values and the decoded
store sequence, without executing either constructor. Whole prepared object
storage and the complete 4 KiB guarded page are compared. Pointer observations
that identify native versus retail vtables or separately allocated name blocks
are classified explicitly; this is not raw heap-address identity. Allocation
events also capture the accessible object prefix at each boundary.

The guarded domain guarantees at least `0x220` writable bytes, so the base
camera initialization prefix is available. It does not establish arbitrary
invalid receiver behavior, arbitrary allocator failures, or full object lifetime
outside the reached graph. `_fpreset` and the selected precision/rounding mode
are applied before each run to prevent fault-abandoned x87 temporaries from
affecting the next scenario. Unmasked floating-point exceptions, full fault
register/x87-stack context and exception resumption are not compared. The
constructor's reached arithmetic uses the tested fixed initialization constants;
this is not arbitrary floating-point gameplay coverage.

## Object and storage preservation

The original aggregate retains all **66 nondebug sections and 337 relocations**,
including the original constructor, other functions, EH data, RTTI and startup
artifacts. Its retained filename normalization enumerates exactly the four
existing camera sample filenames; it does not collapse arbitrary paths.
Fresh private-root and actual-root aggregate objects reproduce the same pin.

The new candidate has **31 nondebug sections and 139 relocations**. Its ordinary
TU emits the genuine FollowCamera table/RTTI and constructor EH artifacts, plus
60 bytes of writable private BSS and 16 bytes of `.CRT$XCU`. The four local vector
objects are at BSS offsets 0, 32, 48 and 16 for zero, X, Y and Z. All eight
initializer functions are strict and match the existing aggregate; their CRT
entry targets and destination offsets are checked. These artifacts are included
in the complete object pin, not omitted as incidental overhead. The original
translation-unit grouping remains unknown.

The candidate compiles without project include-path arguments and reproduces
the ordinary game compile command's complete object. A separate directory-root
compilation also reproduces every nondebug section and relocation. No compiler
profile override or matching-only code arrangement is used.

## Reproduction

With `VC6_ROOT` and `MCM2_EXE` pointing to authentic local inputs:

```powershell
$env:PYTHONPATH='.;tests'
python tests/test_follow_camera_constructor.py
```

For the private proposal, replace the test path with
`work/connections60/startup/tree/tests/test_follow_camera_constructor.py`.
The retail SHA-256 is
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.

### Functional build review

The candidate is admitted individually as reviewed nonexact in the functional
build. The original strict and partial samples remain intact. Actual build
`work/game-build/build-5ou5d2p9` compiles 272/272 sources and removes exactly
the missing constructor: **185 -> 184** unresolved names, none added. All 271
previous objects remain unchanged, including 10,833 nondebug sections and
46,129 relocations. The new object's complete 31-section/139-relocation view
matches the private reviewed candidate without filename exclusions. Compiler
flags are unchanged. Every previous calibration and physics extent, score and
strict result is preserved.

Actual-path constructor, loader, tick, controls and D3DIM connection tests pass.
Evidence covers the complete constructor and finite helper, allocator, guard
and unwind domains above; it does not establish a linked game, integrated camera
startup, menus or racing.
