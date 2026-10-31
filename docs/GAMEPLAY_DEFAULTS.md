# Independently identified gameplay defaults

`GameplayDefaults.cpp` defines three existing provisional call views through
their genuine declarations. Each complete body matches authentic VC6 SP3 output:

| Existing declaration | Retail address | Complete body |
| --- | --- | --- |
| `BikeA60C::~BikeA60C()` | `0x00464e90` | 1-byte RET |
| `VehicleSpeedState::~VehicleSpeedState()` | `0x00464e90` | 1-byte RET |
| `KbRaceHandler::Fn_004DE580(void*, float, int)` | `0x004de580` | 3-byte RET 12 |

These are separate ordinary definitions, not aliases to another class with the
same small body. Semantic names and original translation-unit ownership remain
provisional. The direct calls establish the individual boundaries below; they do
not establish complete identity with MovingPart, UnknownEngineGearbox, RaceSound
or any other source view.

## Destruction and receiver evidence

The complete strict `Bike` destructor at `0x00409a10` is 276 bytes. Its incoming
receiver is the virtual GameObject base at primary+0x738, not the primary Bike
pointer. It writes the primary vptr through receiver-0x738, then loads the
handlebar pointer through receiver-0x12c: primary+0x60c. A null check guards the
call at `0x00409acc`; ECX receives that unchanged pointer. The same pointer is
then passed to operator delete at `0x00409ad2`.

Bike's loader allocates 0x4c bytes at `0x00407da2`, calls constructor `0x004a23a0`
at `0x00407dce` with the original allocated pointer in ECX, and stores the result
at primary+0x60c at `0x00407dee`. Its three arguments are the literal
`Handlebars`, the model root and steering owner. The complete 48-byte constructor
stores node+0x40, owner+0x44 and zero+0x48. Its existing MovingPart source body is
strict; this corroborates the accessed layout without unifying the two views.
LoadBike remains an existing nonexact function, preserved in full by its whole
object pin. The producer excerpt is not described as a newly strict loader.

The complete strict `Vehicle` destructor at `0x00526380` is 319 bytes. Its
incoming virtual GameObject receiver is primary+0x5c0. Loading receiver-0x140
therefore obtains primary+0x480, the engine-state pointer. The nonnull pointer
is passed unchanged to the empty destructor at `0x0052640f`, then to operator
delete at `0x00526415`. The full strict 1,373-byte LoadVehicle allocates 0x1e4
bytes, invokes the actual gearbox constructor at `0x005260df`, and stores the
returned pointer at primary+0x480. The canonical 1,542-byte gearbox constructor
also matches strictly. No complete field-type equivalence is inferred from
these construction/destruction boundaries.

The retained test checks raw primary and virtual-base COL offsets/type names,
the actual instructions above, and independently compiled header layouts.
Neither subordinate provisional record has a vtable. Their destructor identity
comes from the owning allocation and delete sequence, not shared RET bytes.

## Race callback boundary

KrustyBike's primary vtable `0x00554548` slots 3/4 point to the complete strict
109-byte and 112-byte overrides at `0x0048dbf0` and `0x0048dc60`. They reach the
callback through primary+0x740, then context+0x44, without adjusting that
callback receiver. The strict complete KrustyBike loader retains its context
argument at primary+0x740. The callback's apparent class purpose remains
provisional; no concrete event-handler allocation or broader class identity is
claimed here.

Three direct calls, `0x0048dc0e`, `0x0048dc52` and `0x0048dcc5`, push exactly
three DWORD arguments: code, float bits and the unchanged primary KrustyBike
pointer. One arm supplies float zero; the other two copy the argument's pointed
float bits. The complete target returns while consuming 12 stack bytes and
accesses none of the receiver or argument data. The retained test checks every
actual COFF call relocation against its distinct decorated declaration as well
as the raw retail instructions. It does not invoke unrelated same-address
methods or change virtual slots.

## Narrow declaration ownership and preservation

The existing BikeA60C, VehicleSpeedEntry/VehicleSpeedState and KbRaceHandler
declarations move verbatim into BikeHandlebarView.h, VehicleSpeedState.h and
BikeRaceHandlerView.h. Their original headers include them at the same
declaration positions. There is one definition of each class. The handlebar
header preserves `VehicleSteerState` as the actual alias of forward-declared
`SteeringControl`; it does not invent a class with the alias name.

The new source needs only these narrow headers. It compiles without project
include paths into one 40-byte directive section and three 16-byte code sections,
with no imports, nondebug relocations, data or CRT initialization. This avoids
introducing the unrelated Math3D constants emitted by the broader headers.
No original include order or original source-file grouping is claimed.

All nine identified old canonical/sample include consumers preserve **1,072
nondebug sections and 5,226 relocations** byte-for-byte, including every partial
function, private initializer and constant. There are zero symbol mappings or
excluded regions. Retained portable pins normalize only the nine enumerated
source filenames. The focused test checks **211 complete strict bodies** and
the five direct default-call sites. All old object bytes and destinations remain
required; no partial pin or score is relaxed.

## Finite native ABI checks

The fixture separately links the actual new source object. Its 3,072 calls cover
256 initial page patterns, all three methods and four receiver addresses:
aligned accessible, unaligned accessible, protected-page and null. The callback
also receives a protected-page `who` pointer. Every invocation must complete
without an exception and leave the entire readable page unchanged. Actual
constructor-free, out-of-line call relocations are verified; no wrapper replaces
the implementation and no retail code is dispatched by this executable.

These are deliberately bounded VC6/x86 call-boundary observations on prepared
storage. Null, inaccessible and unconstructed receiver calls are not portable
C++ lifetime claims. The complete RET/RET12 byte comparisons establish the
callee stack contract; the finite native calls additionally observe successful
return and absence of receiver/argument-data access. The fixture does not run
the owning destructors, operator delete, gear simulation, sound callbacks or
an integrated race. Existing constructor/provider dependencies remain separate.

```powershell
$env:PYTHONPATH = '.;tests'
python -m unittest test_gameplay_defaults -v
```

Real game build `build-piwu0zg2` compiles all 265 translation units and removes
exactly these three unresolved names: **213 -> 210**, with no additions. All
264 prior production objects preserve 10,709 nondebug sections and 45,818
relocations without name mappings or filename exclusions. The nine identified
header consumers independently preserve all 1,072 sections and 5,226 relocations
at their actual source paths. Both old and proposed retained pins were checked.

All three definitions also match strictly in the actual game-command object,
whose complete four-section layout has no imports, storage, CRT entries or
nondebug relocations. The actual-path retained test passes 211 strict checks
and 3,072 native calls. Three new calibration rows are strict; two share one
retail address/extent, and no new retail address is claimed.

At this slice the final source build was still unlinked (it has linked and raced since; see [GAME_BUILD](GAME_BUILD.md)). Successful startup, integrated menus
and racing are not established by these default implementations.
