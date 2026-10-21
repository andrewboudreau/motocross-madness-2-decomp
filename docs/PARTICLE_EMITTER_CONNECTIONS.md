# Particle emitter construction and Spark positioning

This slice connects four existing allocation calls to the genuine Dust,
DirtChunk, Steam and Spark particle emitter declarations, and three Spark
position calls to the existing Spark setter. The four complete emitter sources
and bindings move into reconstructed source. The shared ParticleEmitters.h moves
with them; its old sample path includes that single definition. All 28 existing
physics matrix rows route to
the canonical sources, and the duplicate sample bodies are removed. Original
translation-unit ownership remains uncertain. These classes have literal RTTI
identities, but adjacency to Particles.cpp does not establish their original
source file.

The complete existing emitter bodies match retail under authentic VC6 SP3:

| Class | Constructor | Destructor | Deleting destructor | Tick | Slot 27 |
|---|---:|---:|---:|---:|---:|
| Dust | 122 | 11 | 30 | 732 | 43 |
| DirtChunk | 287 | 11 | 30 | 970 | 43 |
| Steam | 199 | 11 | 30 | 590 | 43 |
| Spark | 307 | 11 | 30 | 1,055 | 43 |

Dust's 58-byte position and 31-byte color setters, Steam's 49/18/19/30/34-byte
position/restart/burst/stop/color methods, and Spark's 49-byte position setter
also match completely: 28 complete methods in the four canonical source files.
The unchanged DirtSpray header consumer has five further complete strict bodies.

## Construction and receiver boundaries

Bike::LoadBike allocates 0x80, 0x608 and 0x47c bytes. It passes each allocation
in ECX with flags 1 to Dust at 0x40879f, DirtChunk at 0x408828 and Steam at
0x408896. Their returned pointers are stored at Bike +0x728/+0x72c/+0x730.
Vehicle::LoadVehicle allocates 0x60c bytes, calls Spark's constructor at
0x52621e and stores the returned pointer at Vehicle +0x5ac. Each constructor
calls the real GameObject constructor at 0x468ca0, installs its own primary
vptr and returns the unchanged primary pointer. Raw COLs and base descriptors
place GameObject at nonvirtual offset zero in all four emitter classes.

The caller continues using its existing provisional field views after an
explicit pointer cast. Its primary slot 27 call registers the emitter with a
particle manager and its returned primary pointer is passed to AppendChild.
All 28 vtable slots are accounted for: deleting destructor, Tick and slot 27
belong to the actual emitter, while the other 25 refer to existing strict
BaseObject/GameObject definitions. VC6's ordinary weak deleting-destructor
spelling is checked against its emitted scalar body; no source linker alias
is introduced.

The Spark pointer at +0x5ac flows unchanged into the two Bike calls at
0x405392/0x40563c and the Vehicle call at 0x526ad7. Each call passes twelve
vector bytes by value and returns with RET 12. Explicit ParticleVec3 reference
views retain the original component bits and call evaluation; the complete
caller objects prove unchanged instructions, stack use and relocations under
the verified names. Fields +0x60 and +0x78 retain the existing burst and launch
velocity accesses. Layout probes use both actual headers, including object
sizes and those offsets. Full provisional class equivalence is not asserted.

## Excluded mixed receiver

VehicleImpactSink::SetPosition(Vec3) also targets 0x4b9dc0, but it is not
connected to SparkParticleEmitter. Bike registers its DirtChunk pointer as a
type-2 attachment. The actual attachment builder stores it at record +8, and
the dispatcher passes that record to Vehicle slot 19. That setter receiver is
therefore not a Spark merely because the same body address is used. The
provisional mixed-receiver setter stays unresolved. Its separate ordinary
behavior provider would require its own complete prefix/ABI evidence; no
inferred DirtChunk or DirtSpray method is added by this slice.

## Preservation and dependency closure

The five verified external-name mappings preserve all seven affected complete
objects: Bike, Vehicle and the five existing emitter translation units. The four
promoted outputs are compared against their complete original sample objects;
the untouched DirtSpray sample also retains its complete object. Same-path comparison
preserves 517 nondebug sections and 2,124 relocations, with no filename
exclusions. All partial bodies and their relocation destinations remain
unchanged. In particular LoadBike and Bike's impact method remain nonexact;
this slice does not reclassify them as complete strict callers.

Fresh live-root objects were also compared with the private mirror. The only
emitted __FILE__ strings in this seven-object set are the exact Bike.cpp and
Vehicle.cpp paths. Portable retained pins normalize only the seven explicitly
listed source spellings; no header, arbitrary path, payload region or target
is ignored. Those cross-root rows are identical under the same five names.

Every game import in the four emitter objects has an actual definition in the
existing source-built object set, including GameObject construction/destruction,
base virtuals, ParticleManager::AddParticle, FastSqrt and the TrackGame pointer.
The remaining external library names are type_info metadata, __fltused and rand.
Their presence is not counted as new game providers. The retained checks prove
the complete direct game helper extents; they do not independently establish
all CRT implementation bytes or a running particle system. The new emitter
objects contain ordinary RTTI type descriptors, tables and constants, with no
BSS or CRT initializer sections.

The retained regression passes 268 strict comparisons: 33 emitter methods
including the unchanged DirtSpray consumer, 130 strict caller targets, 100
inherited vtable entries, four direct game helpers and the extracted setter.
Repeated inherited entries are comparisons, not 100 distinct functions.
The test also pins seven whole objects and all seven changed direct calls.

The four cpp/binding pairs are promoted to src/reconstructed, rather than
classified as reviewed nonexact samples. All 28 existing exact physics targets
follow those canonical paths. There is no additional calibration target and
no inferred original translation-unit assignment. Existing name-bearing caller
pins require only the five evidenced external spellings to change; resolved
partial bytes, extents and scores are retained. Five existing object/partial
fixtures and the Steering LoadBike named fingerprint are refreshed only after
independently reproducing every old affected pin. The resolved model-render
partial fingerprints remain unchanged. The legacy VectorAddAssign serializer
retains a filename symbol spelling; its exact actual-path literal identity is
accounted for separately in the private transition proof, without relaxing the
retained serializer.

## Native position evidence

The actual 49-byte Spark setter is extracted unchanged and independently
checked against both its complete source object and retail body. It has no
callee or data imports. The native fixture executes that source body and the
untouched hash-pinned retail body on the same prepared page, with no game helper
replacement. It compares each separately to an independent byte-level oracle.

There are 3,200 cases, including 1,024 independently modeled access faults.
Input bits cover signed zero, infinities, NaNs, subnormals and deterministic raw
patterns. By-value arguments are captured from separate storage or overlapping
current/previous-position regions before invocation. The oracle copies current
position to previous position in the decoded component order, then writes the
three captured arguments. Complete 4 KiB page snapshots, completion, exception
kind, read/write access and fault address must match. Success cases must
complete. Guard boundaries exercise progressive stores, a read-only receiver
page and a null receiver. Accesses are aligned DWORDs; split-access faults are
not claimed.

These tests use prepared storage, not constructed emitter lifetimes. They do
not execute constructors, random initialization, Tick, registration, particle
rendering or a game session natively. Their admission rests on complete strict
source/body and dependency evidence. The bounded setter experiment has the
runtime scope described above.

## Source-build verification

Real build `build-o5ke14xh` compiles all 270 translation units and removes
exactly the five verified constructor/setter names: **208 -> 203**, with none
added. All 266 prior game objects retain 10,715 nondebug sections and 45,819
relocations under those names, without filename exclusions. The four new game
objects retain all 101 reviewed sections and 256 relocations.

All nine affected actual-checkout retained tests pass, including the legacy
vector-add fingerprint. Old fingerprints were reproduced before updating only
the verified symbol names. Both global matrices preserve every previous strict
result, full extent and partial score after explicit source-path migration:
3,023/3,034 calibration and 1,086/1,162 physics comparisons are strict. The game
still does not link; [current build status](GAME_BUILD.md) tracks runtime gates.

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH='.;tests'
python -m unittest test_particle_emitter_connections -v
```
