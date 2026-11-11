# Building the game from C++

`python tools/build_game.py` compiles all C++ files under `src/` plus the
individual reviewed candidates in `config/game_build.json`, then links their
objects with VC6 SP3 and the Windows/DirectX import libraries. The configuration
also records include paths, libraries and each candidate's evidence and
nonexact status. There is no blanket inclusion of `samples/`; uncertain source
stays there until it meets canonical-source acceptance.

```powershell
python tools/build_game.py --vc6-root work/vc6sp3-disc -j 4
python tools/build_game.py --vc6-root work/vc6sp3-disc --compile-only
```

The compiler root defaults to `$VC6_ROOT`, then `work/vc6sp3-disc`. The
compiler, linker and CRT hashes must match the private-input pins. On Linux,
use the installed private environment:

```bash
python3 tools/with_private_env.py -- python3 tools/build_game.py
```

Every invocation creates a fresh output directory under `work/game-build/`.
`latest.json` points to that run through its `build_directory` field and
records source/header hashes, the compiler profile and per-source results.
The header inventory conservatively covers both `src/` and `samples/`,
including helpers used by individually admitted candidates.
Each object has an adjacent compiler log. Import-library and linker logs
are in the build directory. Failed compilation or linking returns nonzero;
an old executable cannot satisfy a subsequent build.

The default profile is `vc6_o2_mt`. A reviewed candidate may explicitly request
`/QIfist` and `/Op` through `extra_flags` (`/Op` only for the
[four-pose blend](#four-pose-motion-blend)). The first `/QIfist` users were the motion-start
conversion helper, which preserves the incoming rounding mode on a
floating-point trap, and the ProjectedShadow near-miss file
([below](#projectedshadow-near-miss-bodies)), plus the fistp samples further
down. Bring-up units under `src/bringup/` may take `/QIfist` (and
no other flag) through a `bringup_flags` entry with a reason; see
[Bring-up fistp units](#bring-up-fistp-units). The EBP frame of retail's
fistp helper comes from a per-function pragma in the source, not a flag; see
[Frame-pointer units](#frame-pointer-units). Every actual per-source command is recorded in the report. Other source
files retain the default flags. Objects are ordered by source basename,
then path. This is a reproducible working order, not proof that reconstructed
files recover original translation units. Ordinary import libraries supply
external DLL functions; the build never resolves game functions to retail
addresses and never forces unresolved references into an executable.

## Current gaps

The 2026-10-21 build compiles 274 C++ files (216 canonical sources and 58
reviewed candidates) and reaches LINK with 177 unresolved external symbols,
down from 855 before the shared-class, global-provider and rendering integration.
This is a measured link failure, not a source-completion percentage. The current
run is `work/game-build/build-vnogj0uu`; its `link.log` is authoritative.

The [terrain quality storage](TERRAIN_QUALITY_STORAGE.md) supplies two writable
ten-row tables with all 320 initial bytes checked against retail `.data`.
Original translation-unit ownership and hardware/software names remain
provisional. The existing Terrain selector and setter consume the actual
shared tables; the new provider adds no functions, relocations or initializer.

The isolated real link removes exactly two globals: **180 -> 178**, none
added. All 273 previous objects retain their complete 10,866 nondebug sections
and 46,268 relocations with no symbol mappings or filename exclusions.
The retained test passes 24 full strict checks and 5,130 native cases,
including eight independently modeled access faults. Literal and deliberately
mutated writable table values cover valid indices 0..9 and complete prepared
receiver state. Full Terrain lifetimes and the nonexact loader remain untested.

The [race detail connection](RACE_DETAIL_CONNECTION.md) joins both detail-key
calls to the existing `BikeRace::SetDetailLevel` provider. The complete retail
loader, constructor, primary RTTI offset and call instructions establish the
receiver and argument boundary. No class layout or provider body changes.

The next real link removes exactly one provisional method spelling:
**178 -> 177**, none added. All 274 objects preserve 10,868 nondebug sections
and 46,268 relocations under that one name, with no filename exclusions.
The prior Eco light object pin was independently reproduced before this
name transition. All six race-event partial fingerprints remain unchanged.

The retained race-detail test passes 35 strict checks and 4,520 native cases,
including 200 independently modeled write faults. Actual source detail and
model helpers execute with the real preset tables against independent state
and fault oracles. These are finite prepared-storage cases with masked nearest,
64-bit x87 settings, not full event handling or constructed race lifetimes.
The existing Eco light and race-event tests also pass at the actual source paths.

Evidence: `work/game-build/round64-terrain-link-proof.json`,
`round64-race-link-proof.json`, `round64-terrain-retained.log` and
`round64-race-retained.log`.

Calibration retains **3,023/3,034** strict rows (3,007 distinct source/address/
extent cases). Physics retains **1,086/1,162** strict comparisons. Every old
score, full extent and strict result remains unchanged after each slice.
Evidence: `work/game-build/connections-round64-terrain-preservation.json`
and `connections-round64-preservation.json`.
The game still does not link; startup, integrated menus and racing are unverified.

Many remaining declarations use provisional class views or different method
signatures from the available implementation. Those need evidence-based
type/caller integration, alongside genuinely missing functions and globals.
Resolving an address in a matching bindings file does not supply a C++ linker
definition. The FastMath provider is now promoted from samples, with its
runtime-initialized lookup-table storage. All seven functions retain strict
matches over 523 bytes with resolved relocations. A separate native VC6
compile/link/run regression exercises initialization, square roots and trig
tables (`tests/test_fastmath_link.py`, enabled by `VC6_ROOT` on Windows).

The physics and game hierarchies now share one [GameObject](GAMEOBJECT.md)
declaration. This removes incompatible virtual names and parameter types;
the current composition retains all prior physics targets, with 1,086 now strict-exact, and all
3,007 distinct strict-exact calibration cases. A native link/runtime regression checks
virtual dispatch, child callbacks and iterator mutation guards against the
real BaseObject, GameObject and MemTag implementations.

[Graphics storage](GRAPHICS_STATE.md) now supplies the display lists, camera
limits, gamma ramp, shared texture caches and detail presets. Thirteen missing
DirectX GUIDs and the required-version scalar also have source definitions.
The [DirectX startup probe](DEVICESETUP.md#functional-build-review) remains a
nonexact candidate in `samples/`, explicitly included after a native
720-scenario differential test against retail. This is behavioral evidence;
its 1006 emitted bytes still differ from the 1011-byte retail body.

Thirteen [provisional views](TRACKGAME_GLOBAL.md) and the BikeAI sample, which reads the complete
`TrackGame`, now use the same `g_TrackGame` pointer defined by
MSZoneInterface, eliminating ten unresolved names in the current build. The
views still describe only the offsets each caller uses; their consolidation
does not establish complete class layouts. Vehicle's steering state now uses
the existing `SteeringControl` class and its five supplied bodies, including
the destructor confirmed by Vehicle's delete sequence. `SetAxisFromPoints`
is now [strict exact](STEERING_AXIS_FROM_POINTS.md) in the same canonical unit. Six [network GUIDs](NET.md#network-identifiers)
also have verified source definitions. These three integrations remove 21
unresolved symbols from the preceding 811-symbol build.

The next source connections remove another seven symbols: the exact
40-byte `CollisionObject::Configure` now lives in canonical source, scene
callers use the existing collision methods' opaque-pointer signatures, and
MorphBastardModifier uses the existing Soultree matrix methods. All prior
strict matches remain intact. These are narrow method connections; duplicate
class layouts elsewhere still require independent reconciliation.

The five complete [stream candidates](FILESTREAM.md#functional-build-review)
now supply `Read`, `Seek`, `Write` and the header methods. They remain explicitly
nonexact in `samples/`; a native differential test covers 12,390 scenarios,
and their inclusion preserves the existing strict matches. Three
[keyboard/platform globals](DEVICESETUP.md#keyboard-storage) supply the
PC-98 scan-code map, detection sentinel and platform flag, with all 1,032
initial bytes checked against retail. Together these remove eight more
unresolved symbols. Real disk/archive integration remains untested.

The collision consumers now share the canonical class, preserving its `+0xc`
secondary base and all observed field offsets. Motion loaders use the supplied
stream/resource methods and the existing resource-manager pointer. The stream
constructor has one resource-manager-pointer signature, and the exact input
binding-ID allocator is now supplied by ControlInterface.cpp. Soultree uses
the quadtree pointer maintained by VisibilityQuadTree; terrain writes Griddraw's
existing size/level words; visibility statistics use MemTagStack; and shutdown
uses the existing font list. Bike and TGA readers use the verified empty
literal instead of undefined array names. This composition removes another
31 unresolved symbols, with prior strict matches preserved. Corrected names
for still-missing methods are not counted as resolutions.

The next connections supply nine collision/resource/BVH/quadtree names and
eight motion parameter-reader names. Four [physics math units](PHYSICS_MATH.md)
supply seven exact bodies, four of which satisfy current callers. Eleven
additional Soultree physics methods now have canonical definitions verified
against their primary-vtable slots at object offset zero. Together these
remove another 32 unresolved symbols while preserving all prior strict
matches. Their source grouping does not establish original TU ownership;
unassigned header initializer copies remain documented separately. The
shared parameter stream retains its inline reset behavior, which is
required by the current strict SelectSection match. Soultree slot 35 still
depends on the already-missing SetAxesIn implementation.

Soultree's collision-file loaders now use that same stream implementation and
SceneManager's archive finder. BikeRace uses the existing [ChatOverlay](TRACKOVERLAY.md)
class for its input, visibility and message calls. Four complete
[collision helpers](COLLISION_HELPERS.md) now have canonical providers; three
are imported by current production callers. These connections remove eleven
unresolved names without introducing any new ones, preserving all strict
calibration and physics matches. Native layout, retail call-site and provider
checks cover these connections.

The [parameter reader](PARAMETERBLOCKS.md) now shares its stream declaration
and resource-manager interfaces with existing implementations, preserving
SelectSection's complete body and the reset accessor. The
[GUI connections](GUIMANAGER.md#shared-caller-connections) supply eight existing
provider identities and consolidate the ShowDialog alias. Thirteen
[Vehicle methods](VEHICLE_METHODS.md) now have canonical definitions verified
against their own primary-vtable slots. Bike's zero-vector reads use the local
storage already supplied by its strict initializer. These changes remove
31 unresolved names without adding any new ones.

The [archive loader](RESOURCEMANAGER.md#functional-build-review) now supplies
AddArchive as an explicitly reviewed nonexact candidate. Its complete body
passes 17,292 native differential scenarios, including constructor unwind
paths; VC6 emits 798 bytes against retail's 804. The corrected
[ShowDialog](GUIMANAGER.md) has a complete strict 341-byte body in canonical
source and passes 34,560 native differential scenarios. Its extent follows
independent retail control-flow evidence; the old 345-byte range included
padding. These two providers remove two more unresolved symbols. Scripted
dependency tests do not establish working archive loading or menus in the
source-built game.

The shared [ResourceManager declaration](RESOURCEMANAGER.md) now connects seven
more existing providers, including Scene's local manager lifecycle. Complete
[collision transforms](COLLISION_TRANSFORMS.md) supply MoveBox and the rigid
inverse, and [SurfaceMap](SURFACEMAP.md) supplies its three material-caller
imports. Its six bodies use the actual shared base, texture and rendering
interfaces while retaining the caller's `0x70` allocation. These slices remove
another twelve unresolved names with none added. Native checks cover resource
layouts and imports, 229 collision geometry/alias cases, and SurfaceMap's
ownership, material copying and virtual-call interfaces.

The shared [texture loader](TEXTURE_LOADER_CONNECTIONS.md) and material
declarations connect eight more existing providers. Garage lighting uses four
actual LightEmitter/LightManager providers, and verified [meter pointer
storage](HIRESMETER_STORAGE.md) supplies the existing global instance's address.
The complete [game tick and calibration](GAME_TICK.md#functional-build-review)
are reviewed nonexact candidates with a consistent QPC profiling backend.
Their 39,376 native differential scenarios retain gameplay's ReadClock timing;
4,096 separate native calls exercise the actual QPC backend. These slices
remove fifteen missing symbols without introducing new unresolved names.

Bike and KrustyBike now use the existing [Wrecker providers](BIKE_WRECKER_CONNECTIONS.md),
preserving the `+0x604` pointer, `0x53c` allocation and animation-handle bits.
Wrecker in turn uses the existing [Soultree and character interfaces](WRECKER.md),
including the virtual GameObject base and model-node offset. These connections
remove 27 missing names; two renamed Soultree methods required separate
functional admission below. Full headers are included
only where their complete definitions are needed, avoiding the separate render
and physics Soultree declarations colliding in other translation units.

The complete [crash reporter](EXCEPTIONHANDLER.md#functional-build-review)
supplies one more missing startup provider as a reviewed nonexact candidate.
Its actual standalone object passes 13,838 differential scenarios, including
faulting code/stack reads and both SEH regions. An ordinary-C++ OS backend
replaces the retail FS stack-base load and passes 1,536 main/worker/fiber checks.
A separate native integration test links the reporter, backend and canonical
helpers unchanged, writes a real report and verifies exception propagation
and one-shot behavior. This tests the reporting path on the current host;
it does not establish game startup.

Six [vehicle camera/audio model connections](VEHICLE_MODEL_CONNECTIONS.md)
now use the existing scene-node providers while preserving their base offsets
and vector layouts. Three [scene profiling consumers](DEBUGOVERLAY.md#scene-profiling-connections)
use the existing overlay and peak-hold methods, removing ten more incompatible
names. The visibility timer now reserves the actual 12 bytes required by its
fields instead of an empty class's one-byte allocation. Complete caller and
provider matches, native layout assertions and real COFF imports cover these
connections. A local RaceView type alias preserves the declaration order
required for ProCircuitProcs' complete strict AddCircuitRow body.

The complete [Soultree transform candidates](SOULTREE_TRANSFORMS.md)
provide WorldToLocalDirection, SetMatrixIn and SetAxesIn. They remain explicitly
nonexact in samples; the transpose helper retains its existing strict match.
Their actual standalone objects pass 86,016 native differential scenarios,
covering frame/parent branches, finite fractional inputs, overlapping matrices
and vectors, complete state changes and ordered calls. The test scripts node
cache dependencies and checks the hidden result-buffer return pointer; it does
not establish recursive scene-cache integration or universal NaN equivalence.
These three methods remove three additional missing names without introducing
new unresolved dependencies.

Four [inherited render-node methods](D3DIMSOULTREE.md#inherited-model-helpers)
and two [Bike math aliases](BIKE_MATH_CONNECTIONS.md) now reach existing
providers while preserving complete caller and provider matches. The complete
[Character playback and VUE loader](MOTION_PLAYBACK.md#functional-build-review)
unit is a reviewed nonexact candidate. Its native comparisons cover 835,584
playback/state cases and 33,280 loader/ownership/unwind cases; the three
previously strict pose helpers remain strict. Playback's extrapolation now
preserves retail's vector-call rounding and integer frame-rate precision.
These changes remove eight unresolved names and introduce none. Four additional
[camera](FOLLOW_CAMERA.md) and [Wrecker math connections](WRECKER.md#shared-model-and-motion-calls)
use the existing vector constructor, cross product and normalization providers.
All 67 strict caller cases and both Wrecker partial comparisons remain unchanged;
the real link removes four more names with none added.

Four complete [vector call providers](VECTOR_CALLS.md) now supply the explicit
output signatures used by motion, collision and vehicle code. Their full
69/69/69/29-byte bodies are strict exact and pass 31,873 native arithmetic,
return-pointer and aliasing cases. Playback and pose tests now link these
actual providers. The [bike-class engine-size table](DIALOGPROCS.md) supplies
another missing name with all 20 writable data bytes verified. Together these
remove five unresolved names without adding any.

The complete [dialog tick](GAMEUI.md#functional-build-review) and
[display profiling pass](PCGAME.md#functional-build-review) are now explicitly
reviewed nonexact candidates. Their native comparisons pass 73,728 and 8,486
scenarios respectively, including mutating UI callbacks and display resource
ownership/unwind paths. Dialog ticking removes one missing name. Profiling
supplies its entry point but initially exposes a reference to
`PCVideoCard::ProbePartialTextureUploads`, so that admission leaves the
missing-symbol count unchanged. Dependencies in these tests are scripted;
real input, registry and display-driver integration remain untested.

Four complete display candidates now supply [mode-surface creation and channel
comparison](DISPLAY_SURFACE_HELPERS.md), [non-local texture memory probing](NONLOCAL_TEXTURE_MEMORY.md)
and [partial texture uploads](PARTIAL_TEXTURE_UPLOADS.md). They pass 200,448,
144,944, 12,912 and 26,600 native comparisons respectively, including complete
state, dependency calls, pixels, ownership, constructor unwind and observed
fault paths as applicable. All four remain nonexact. Complete control-flow
decoding corrects the memory-probe and channel-comparison retail extents to
486 and 629 bytes; the older ranges ended inside instructions. The original
aggregate sample includes the extracted bodies and preserves its normalized
instruction bytes and relocation structure. Each admission reruns the real
link: together they remove four missing symbols and introduce none.

Five more [math call-view connections](VECTOR_CALLS.md#existing-call-view-connections)
join camera, matrix, Bike, Wrecker and BVH callers to the existing dot/scale
providers. All 209 strict caller cases and nine complete partial-function
candidate hashes remain unchanged. The real link drops from 572 to 567
unresolved names, with none added.

Three individually reviewed candidate units then supply four missing bodies:
[camera matrix updates](CAMERA_UPDATE.md#functional-build-review),
[primary-buffer PCM format setup](PCAUDIO.md#functional-build-review), and
[menu defaults and data-file lookup](UIINFO.md#functional-build-review).
The camera passes 76,288 scripted comparisons and 9,216 cases with initialized
source math providers; audio passes 172,032 state/payload/callback cases.
The menu unit passes 21,332 scripted comparisons plus 192 searches through
actual VC6 CRT file I/O. These bodies remain nonexact. The file-lookup retail
extent is corrected to 355 bytes to include its complete final return.
Real links after each admission remove one, one and two missing names
respectively, reaching 563 with no added unresolved dependencies. This is
bounded behavior and integration evidence, not startup, menu or racing proof.

Five option dialogs now [inherit the supplied synchronization method](DIALOGPROCS.md#inherited-option-dialog-synchronization)
shown by their retail primary vtables. Removing the incorrect override
declarations preserves all 593 strict cases in the affected/provider units and
removes five unresolved names. Seven race consumers now call the canonical
[`BikeRace::NextBike`](NEXTBIKE_CONNECTIONS.md), closing four more names. The complete class declaration
is shared through a narrower header; its fields and bases are unchanged.
Twelve [writable bike-state scalars](KRUSTY_BIKE_STATE.md) provide their exact retail initial values,
including loader-zeroed storage and the distinction between file initialization
and later configuration writes. They remove twelve more missing names.

The complete [tooltip and pointer-device helpers](GUI_POINTER_HELPERS.md#functional-build-review)
are admitted as one explicitly nonexact candidate unit after 19,606 native
comparisons. Their full 106/52-byte bodies and aggregate relocations remain
unchanged. Their admission removes two more names. Real links after these four
slices measure **563 → 558 → 554 → 542 → 540**, with no new unresolved
dependencies.

The [loading-progress dialog](DIALOGPROCS.md#loading-progress-dialog-connection)
now supplies its evidenced UIDialog receiver to the existing FindControl
provider, removing one missing name while preserving 506 strict cases.
Twelve [terrain and drawing globals](TERRAIN_DRAW_STATE.md) supply exact writable
initial storage, including loader-zeroed words, and preserve 79 strict caller
cases. Three [terrain method connections](TERRAIN_CONNECTIONS.md) then join
texture eviction, detail selection and segment visibility callers to existing
complete providers. All 178 strict cases and both partial-body fingerprints
are preserved. The isolated real links measure **540 → 539 → 527 → 524**,
with no new missing names. QueryGround's subsequent caller connections and
bounded cell-conversion repair are described below. The game still does not
link, so these checks do not establish interactive menus or racing.

Two [empty lifecycle providers](EMPTY_LIFECYCLE_PROVIDERS.md) now supply the
GUIManager palette-related hook and the scene-finder destructor used during
shutdown. Each complete one-byte body matches retail, and each method identity
is supported independently by its callers. Their shared retail address does
not establish shared source identity. Both remain out of line, preserving
existing caller bytes. The real link removes exactly those two names,
**524 → 522**, without exposing additional dependencies.

Terrain loading now [uses the supplied stream methods](TERRAIN_CONNECTIONS.md#terrain-stream-calls)
for its 14 reads, three seeks and one tell. All 84 non-debug COFF sections retain
their bytes and relocation destinations under the three external symbol
renames. The loader remains nonexact; the existing stream behavior regression
passes 12,390 cases. Three [bike rider-node connections](BIKE_NODE_CONNECTIONS.md)
then use the existing FindByName, GetSubtreeBounds and SetMatrixIn providers.
They preserve 189 strict cases and all 15 complete partial-body hashes; the
reviewed nonexact matrix provider retains its 86,016-case native proof.
These isolated real links remove six names, **522 → 519 → 516**, with none added.

The complete [streamed-audio loader thread](AUDIO_LOADER_THREAD.md#functional-build-review)
is now an explicitly reviewed nonexact candidate. Its 37,387 native comparisons
cover queue mutation, locks, pending-buffer transfers, callback failures and
observed fault states. Extraction preserves both full aggregate bodies and
their relocations. The real link removes its missing callback name,
**516 → 515**, with no new unresolved dependencies. Synchronization and sound
helpers are scripted in that fixture; actual scheduling and playback remain
untested.

The terrain and grid caches now share the existing [AgeManager](AGEMANAGER.md)
and its two-argument eviction callback signature. All nine affected objects
preserve their non-debug section bytes and relocation destinations under the
canonical symbol renames. Separating the entry record from the manager's
pointer-only declaration preserves Terrain's sensitive VC6 load scheduling.
The isolated real link removes six names, **515 → 509**, with none added.

Five [race-control connections](RACE_CONTROL_CONNECTIONS.md) use the existing
BikeRace providers for player departure, replay pause/save, restart and start
countdown. All 206 registered strict comparisons and four complete partial
fingerprints remain unchanged; 993 native cases cover actual event routing
and pause behavior. The next real link removes five names, **509 → 504**,
with no new unresolved dependencies.

The complete [FollowCamera tick](FOLLOW_CAMERA_TICK.md#functional-build-review)
is admitted as an explicitly nonexact candidate after 674,335 native
comparisons. Its full 3,672-byte body retains 3,659 matching positions; axis
initializers and the actual FastSqrt provider remain strict. The aggregate's
code, storage and relocations are preserved with the documented diagnostic
filename changes. The final real link removes one name, **504 → 503**.
Terrain, rotation and other camera methods are scripted test boundaries, so
rendered camera behavior remains untested.

Terrain loading and pooled-surface allocation now use the existing
[texture constructors and group registration](TERRAIN_TEXTURE_CONNECTIONS.md).
The three connections preserve 106 strict cases, both complete caller objects,
and their existing partial functions. The real link removes three names,
**503 → 500**, with no added unresolved dependencies.

Seven [vector constructor definitions](VECTOR_CONSTRUCTORS.md) now supply the
existing provisional call types used by camera, collision, BVH, Soultree and
vehicle code. Each complete 25-byte body is strict. The original call sites
remain out of line, all existing strict cases and 28 complete partial hashes
are preserved, and 72,240 native cases check raw floating-point words,
placement and overlapping argument storage. Their shared retail address does
not establish shared class identity or original TU ownership. The next real
link removes exactly seven names, **500 → 493**.

The complete [camera-control handler](FOLLOW_CAMERA_CONTROLS.md#functional-build-review)
is admitted as a reviewed nonexact candidate after 195,339 native comparisons.
Its 4,046-byte body handles keyboard/joystick translation, orbit controls,
pitch/distance limits and yaw wrapping. The full retail extent is corrected
to 4,058 bytes, including the final return. Aggregate code, storage and
relocations remain unchanged. Its only game dependency is the existing global
game pointer; the real link removes one name, **493 → 492**, with none added.
Actual input devices and rendered camera operation remain untested.

The [terrain query caller connections](TERRAIN_QUERY_CONNECTIONS.md) replace
five provisional names with the canonical provider across nine units and 23
calls, **492 → 487**. All caller code is preserved. QueryGround remains partial;
the subsequent cell-conversion repair below addresses its truncation defect.
Full ground sampling still requires behavioral proof.

Three [Character motion connections](CHARACTER_MOTION_CONNECTIONS.md) route
rider lookups and flag/garage animation selection through existing strict
providers, **487 → 484**. They retain primary receiver pointers and the exact
32-bit motion-pointer handles. The focused checks preserve 181 strict results
and twelve complete partial fingerprints; 3,840 native cases execute the actual
lookup and state-update bodies, including missing-name faults.

The complete [recorder tick](RECORDER_TICK.md#functional-build-review) is
admitted as reviewed-nonexact after 147,749 native comparisons, **484 → 483**.
Its 2,448-byte candidate handles forward, pending and reverse playback against
the complete 2,412-byte retail body and jump table. Extraction preserves all
13 aggregate functions and 45 nondebug sections. Locks and existing helper
calls are scripted; actual threads, replay files and recording remain untested.

Across that build, all 204 pre-existing objects preserve their 10,373
nondebug sections and 44,807 relocations under the eight verified external-name
substitutions. Named COFF sections identify reordered RTTI records; local
targets retain their section identity and offset. The new recorder candidate
is the only added object. Private evidence is in
`work/game-build/round19-all-objects.json` and `round19-link-evidence.json`.

The [garage preview connections](GARAGE_PREVIEW_CONNECTIONS.md) join character
construction, motion advance and plate painting to five existing strict
providers, removing six provisional names: **483 → 477**. Ten affected objects
preserve all section bytes and relocations under those substitutions; 192
native painter lifetime cases exercise construction, release and unwind.

The [terrain conversion repair](TERRAIN_CELL_CONVERSION.md) corrects cell
selection under active x87 rounding and passes 278,411 native comparisons
through the first cell request. The partial QueryGround changes deliberately;
full interpolation and unmasked exception behavior remain unverified. The
same slice supplies the strict nine-byte GroundFogableObject constructor,
removing only that missing name: **477 → 476**.

The complete [KrustyUI startup operation](KRUSTYUI_CREATE.md#functional-build-review)
is admitted as a reviewed nonexact candidate after 13,540 native comparisons.
It retains its full 1102-byte body against 1104 retail bytes, including both
allocation unwind states and the inline intro-dialog table. GUI, resource,
profile and device boundaries remain scripted; this does not establish
rendered menus. The isolated real link removes only Create, **476 → 475**,
with no new unresolved names.

All 204 pre-existing objects outside Terrain preserve their 10,292 nondebug
sections and 44,347 relocations under the six verified garage substitutions.
Terrain additionally preserves 83 complete sections, 445 relocations and all
29 other full function extents; its prior 22 strict targets remain exact.
Whole-object evidence is in `work/game-build/round20-all-object-preservation.json`.
The final build's 207 source hashes, 208 header hashes and configuration hash
match the reviewed files; link deltas are in `round20-link-evidence.json`.

Validation passes the toolchain/manual/generated VC6 gate, 2,979 of 2,990
calibration rows (2,977 distinct source/address/extent cases), and 1,086 strict
physics targets among 1,162 comparisons. All prior strict cases remain, with
one new constructor case. Its nine-byte retail body is shared, so unique
address/extent counts remain 2,971 of 2,982. Both near-miss comparisons report
zero strict-score drops. QueryGround's explicitly non-gating masked score
changes from 65/2711 to 54/2707; its behavior repair does not claim matching.
These counts do not describe a fully matched or working game.

The [terrain and collision-tree sampling connections](TERRAIN_SAMPLING_CONNECTIONS.md)
use the existing corner, dot-product and triangle-normal providers, **475 → 471**.
The [garage shadow connections](GARAGE_SHADOW_CONNECTIONS.md) supply seven
existing constructor, attachment and lighting providers, **471 → 464**.
Two new complete 33-byte in-place scale definitions match strictly and remove
their missing Bike and Terrain call signatures, **464 → 462**. Native checks
cover 69,699 corner-sampling cases, 48,792 scale cases and 3,776 shadow cases;
complete terrain interpolation and rendered garage shadows remain untested.

The complete [recorder worker](RECORDER_WORKER.md#functional-build-review)
is admitted as reviewed-nonexact after 24,595 native event, data, lock, failure
and termination scenarios. Both complete extents are 2,148 bytes, with 2,100
matching positions. Existing ring/file helpers and synchronization boundaries
are scripted; actual scheduling and integrated recording/playback remain
untested. Its isolated real link removes one name, **462 → 461**.

Across these four slices, all 207 pre-existing objects preserve 10,409 complete
nondebug sections and 44,995 relocations under eleven documented external-name
substitutions. Only VectorScaleAssign and RecorderWorker add objects. All 209
source hashes, 209 header hashes and configuration hashes match the final build.
Evidence is in `work/game-build/round21-all-objects.json` and
`round21-link-evidence.json`. The final calibration has 2,981 strict rows among
2,992 cases, representing 2,978 distinct source/address/extent cases and
2,972 unique retail address/extent pairs among 2,983 targets. All prior strict
calibration and physics results remain, with no strict-score near-miss drops.

The [input connections](RACE_INPUT_CONNECTIONS.md) route nine collision,
terrain and visibility calls to the existing event predicate, **461 → 458**.
All six complete caller/provider bodies remain strict; 6,720 native cases
execute the predicate and keyboard modifier helper. The
[Bike contact connections](BIKE_CONTACT_CONNECTIONS.md) use the existing
allocation and removal functions, **458 → 456**, preserving Tire's `+0xb8`
secondary contact pointer and passing 13,056 native helper cases. The
[race descendant switches](RACE_INPUT_CONNECTIONS.md#named-descendant-switches)
now use GameObject's existing methods, **456 → 454**; all eighteen calls and
the complete 5,980-byte BikeRace tick remain strict.

The complete [race-to-menu transition](LEAVE_RACE_FOR_MENU.md#functional-build-review)
is admitted as reviewed-nonexact after 9,437 native state, callback, allocation,
fault and unwind cases. Its 533-byte body retains 454 matching positions
against the complete 531-byte retail body. All sixteen dependency providers
are strict, but the native comparison scripts those GUI, display, input-method,
result-recording and audio boundaries. Actual menu/device transitions remain
untested. The isolated real link removes only LeaveRaceForMenu, **454 → 453**.

This composition preserves all 209 pre-existing objects, 10,427 complete
nondebug sections and 45,122 relocations under seven documented symbol
substitutions. Only the transition candidate adds an object. All 210 source
hashes, 209 header hashes and configuration hashes match the final build.
Private evidence is `work/game-build/round22-all-objects.json` and
`round22-link-evidence.json`. All prior strict calibration and physics results
remain, with no strict-score near-miss drops.

The separate nonexact CreateRacers sample changes one seven-byte window:
two independent register initializations exchange order. All other bytes and
relocation destinations are unchanged, with no branch entering that window's
interior. Its diagnostic comparison changes from 13,396 to 13,390 matching
positions out of 13,428 bytes. The two partial baseline expectations are
updated only after verifying equivalent register/flag effects; this sample
is outside the strict calibration and physics counts.

The [bike modifier connections](BIKE_MODIFIER_CONNECTIONS.md) use the existing
GhostMod1 and MorphBastardModifier constructors and model attachment/fade
methods, **453 → 447**. All six connections preserve receiver offsets and
complete object bytes; native checks exercise 3,712 attachment, removal and
fade cases. The [terrain construction/rendering connections](TERRAIN_CONSTRUCTION_CONNECTIONS.md)
use eight existing matrix, palette, grid-block and shared-node providers,
**447 → 439**, preserving every complete caller body and relocation destination.

The complete [movie-opening candidate](MEDIA_CONTROL_OPEN.md#functional-build-review)
is admitted as reviewed-nonexact after 11,125 native COM, output, descriptor,
callback, alias and failure scenarios. Its 656-byte body retains 53 matching
positions against the complete 646-byte retail body. Actual SDK/canonical
GUID values are checked; COM, OS and helper boundaries are scripted. Live
decoding, drivers and downstream Release cleanup remain untested. Its isolated
real link removes only Open, **439 → 438**.

This composition preserves all 210 pre-existing production objects, 10,447
complete nondebug sections and 45,217 relocations under fourteen verified
external-name substitutions. Only the Open candidate adds an object. All 211
source hashes, 212 header hashes and configuration hashes match the final build.
Private evidence is `work/game-build/round23-all-objects.json` and
`round23-link-evidence.json`. All prior strict calibration and physics cases
remain, with no strict-score near-miss drops.

Other incompatible class views, missing methods and unresolved globals
continue to block linking. Import
libraries alone cannot supply them.

An eventual successful link is only the next gate. Startup/static
initialization, external assets, resources, current-Windows display behavior,
menus and actual racing must still be tested on the source-built executable.
The existing `runtime/build.py` route stages the retail demo executable and
does not prove any of those source-build requirements. Source-byte matching
remains a separate check through the existing VC6 calibration and physics
suites.

## Best-guess admissions

Goal: a source-built game that links, runs and races. Near-miss bodies in
`samples/` whose behavior has not been differentially tested are admitted
with status `best-guess-unreviewed` (distinct from `reviewed-nonexact`) so the
link frontier can move. They are guesses until each is reviewed; none of them
is a strict match and none replaces a `src/` definition.

Selection is a greedy link-only search over the compiled objects: add the
candidate that lowers the unresolved count most, reject any that creates a
duplicate (LNK2005) definition. Whole near-miss files often duplicate `src/`
functions (GameUi, DlgProcs, KrustyBike, SelectGamePicProcs, BikeRace,
ProCircuitProcs, Wrecker, ProjectedShadow and others), so those need
function-level extraction rather than file admission. Admitting code also
exposes new dependencies: all 44 files together raise the count from 175 to
249, the 13 chosen files lower it from **175 to 152** with no strict match
changed (separate objects; `src/` is untouched).

## Collision and contact near-miss bodies

Seven collision and contact functions had been linked from bring-up
stand-ins. These were inline-arithmetic rewrites that compiled, but their
linked bodies scored poorly against retail. The game build now links the
near-miss bodies from `samples/` instead:

- `samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp` is a
  `best-guess-unreviewed` candidate source. It includes the canonical
  `src/krusty2/bvh/BoundingBoxTreeQuery.cpp` (an `included_sources` entry) for
  its file statics: the vertex array, relative frames, motion matrix and axis
  scratch. It adds the five near-miss bodies and defines the never-written
  0x00579668 flag. The bring-up copy `Group0_BvhQuery.cpp` was deleted.
- `samples/physics/contact/ContactImpulse.cpp` and
  `samples/physics/constraint/ConstraintMethodCollisionModelNearMisses.cpp`
  are `best-guess-unreviewed` candidate sources. The second file includes
  `ConstraintMethodCollisionModel.cpp` (an `included_sources` entry).
- `Group0_ContactImpulse.cpp` and `Group3_ConstraintCollision.cpp` were
  deleted.
- The bodies call retail's out-of-line vector helpers under view-specific
  names. `link_aliases` binds each name to the exact provider at the same
  retail address:

  | Address | Provider |
  |---|---|
  | 0x0040ae30 | `Vec3DotCall` |
  | 0x00421d00 | `Vec3SubtractCall` |
  | 0x00421cb0 | `Vec3AddCall` |
  | 0x005015b0 | `Vec3ScaleCall` |
  | 0x00428090 | `Vec3ScaleLeft` |
  | 0x00428060 | `Vec3AddAssignCall` |
  | 0x00404e60 | `CollisionVec3` constructor |
  | 0x00435ec0 | `CollisionLength` |
  | 0x005299e0 | `VehVec3Equal` |
  | 0x0043c890 | `CollisionDivide` |
  | 0x0043ca20 | `CollisionRotateVector` |
  | 0x00515600 | `CrossProductCall` |
  | 0x005087b0 | `Vec3Normalize` |

  The `SoultreeObject` methods `LocalToWorldPoint`, `WorldToLocalDirection`
  and `GetPositionIn` are bound the same way. `CollisionObject.cpp` uses a
  `CollisionVec3` overload of `BoxCapsuleOverlap`, which is aliased to the
  `Vec3` definition.
- `included_sources` validation now resolves a quoted include against the
  owner's directory and then the configured include paths, as CL.EXE does.

The table gives each linked-body score (`work/s20/linked_vs_sample.py`
metric) before and after the change. The extent runs to the next map
symbol with a higher address.

| VA | Function | Stand-in | Now linked | Sample object |
|---|---|---|---|---|
| 0x00426be0 | `BoxTriangleQuery` | 0.157 | 0.960 | 0.967 |
| 0x004280e0 | `TreeTreeQueryNodes` | 0.165 | 0.902 | 0.916 |
| 0x0042a640 | `SegmentSegmentDistance` | 0.432 | 0.943 | 0.943 |
| 0x00427c10 | `SegmentTriangleQuery` | 0.697 | 0.946 | 0.946 |
| 0x00425900 | `BoxCapsuleOverlap` | 0.697 | 0.821 | 0.821 |
| 0x0043bdb0 | `ApplyContactImpulse` | 0.497 | 0.984 | 0.984 |
| 0x005004a0 | `ContactSolveImpulse` | 0.584 | 0.960 | 0.960 |

`BoxTriangleQuery` and `TreeTreeQueryNodes` score a little lower linked than
as an object. Their switch jump tables sit inside the linked extent and decode
as unmatched instructions. The link still covers every translation unit with
no unresolved symbols. The vtable audit is unchanged (extra_override 2,
ok 7315).

How the new bodies behave differently from the stand-ins (from reading the
source; not differentially tested):

- `ContactSolveImpulse` and `ApplyContactImpulse` use the same formulas and
  update order. They now call retail's out-of-line helpers instead of inline
  arithmetic, so only x87 rounding can differ.
- The `BoxTriangleQuery` stand-in could not reach the shared vertex array, so
  it tested only the triangle face axis. It over-reported overlap, which
  weakened pruning but did not change collision results, because
  `LeafPairQuery` still decided each hit. The new body runs the full 13-axis
  separating-axis test: the same hits with fewer leaf queries.
- The `TreeTreeQueryNodes` stand-in probed through `LeafNodeQuery`,
  `MoveBox` and `BoxOverlap`. `MoveBox` builds the same swept union as
  `SweptBoxOverlap`, so the stand-in was equivalent apart from rounding. The
  new body calls `SweptBoxOverlap` or the inline box-overlap test directly on
  the relative frame and motion statics.
- `SegmentTriangleQuery`, `SegmentSegmentDistance` and `BoxCapsuleOverlap`
  differ only in floating-point summation order. For example, the push dot
  product now adds z first, and the length is taken through 0x00435ec0.
  Duplicate points are still removed by exact equality, now through
  `VehVec3Equal`.

The existing native tests (`test_bvh_vector_connections`,
`test_collision_query_connections`, `test_constraint_scene_connections`,
`test_collision_helper_providers`) compile and check the unchanged sample and
`src/` objects. No test runs the game-linked bodies.

## Near-miss bodies replacing bring-up copies

Many bring-up files in `src/bringup/` carried copies of `samples/` near
misses. The copies were edited to reach their callees through forwarders or
private reimplementations. Each such edit changed the linked body, so the
copies scored below their samples. For every near miss in
`docs/NEAR_MISS_INDEX.md` and every non-exact `targets.json` entry, the linked
body and the sample object were scored against retail. Where the sample was
clearly more faithful, the game now links the sample's text:

- Whole near-miss files are now candidate sources with status
  `reviewed-nonexact`:
  - `samples/track/SceneManagerNearMisses.cpp`
  - `samples/race/Krusty3DObjectsNearMisses.cpp`
  - `samples/render/Grid1NearMisses.cpp`
  - `samples/race/WreckerNearMisses.cpp`

  The Wrecker file includes `src/reconstructed/Wrecker.cpp` (an
  `included_sources` entry). Its canonical functions score as before.
- The following bring-up files were deleted:
  - `Group4_SceneRead.cpp`, `Group4_SceneBridges.cpp` and
    `Group4_SceneEmitters.cpp`
  - `Group0_BonusObjectManager.cpp` and `Group1_VisualCueTick.cpp`
  - `Group3_GridShared.cpp`
  - `Group1_WreckerContacts.cpp`
  - `Group6_KrustyBikeCtor.cpp`
  - `Group3_KrustyUIShadow.cpp`
  - `Group0_SoultreeBridge.cpp` and `Group0_SoultreeBridge.h`
- `Group4_EcoBridges.cpp` lost its five forwarders. It keeps only the shared
  normal-distribution table.
- Some samples also carry functions that other units of the build provide,
  so they cannot be linked whole:
  - `BikeRaceNearMisses`
  - `BoundingBoxTreeBuildNearMisses`
  - `EcoSystemNearMisses`
  - `KrustyBikeNearMisses`
  - `KrustyBikeCameraNearMisses` (see below)

  Their bring-up copies now use the sample's text for the functions listed
  below.
- `src/bringup/Group4_OrientationAxes.cpp` includes the exact
  `samples/physics/helpers/OrientationAxes.cpp` (0x004b5d00). This gives the
  Scene readers and `KrustyBike` the retail function in place of private
  copies. Which unit owns it is still unsettled (tier 3).
- `Character::Method_0x004a9050`, the four-pose blend that `UpdateRiderPose`
  calls on the rider and the bike, is a candidate sample
  ([below](#four-pose-motion-blend)); the bring-up reimplementation in
  `Group3_KrustyBike.cpp` was removed.
- The sample bodies call retail's callees under view names. `link_aliases`
  binds each view name to the provider at the same retail address. Each pair
  was checked for the following:
  - the allocation size pushed to `operator new`, for constructors;
  - the pushed argument count and by-value vector size;
  - the absence of a `this` adjustment.

  | Retail | View name | Provider |
  |---|---|---|
  | 0x00431e70, 0x004320f0, 0x00432720, 0x00432800 | `UnknownSceneCollisionObject` constructor and methods | `CollisionObject` |
  | 0x004318d0, 0x004319c0 | `UnknownSceneObject` constructor and `Load` | `CollisionCharacter` |
  | 0x004328b0, 0x004329a0, 0x00432a20 | `CollisionObject` hull, sphere and capsule shapes taking `Vector3` | the `CollisionVec3` forms |
  | 0x00507c10, 0x00508970 | `UnknownEcoTerrain::QueryGround`, `GetHeightRange` | `Terrain` |
  | 0x0048fa60 | `UnknownBikeRaceRacer` constructor (0x1638 bytes) | `KrustyBike` |
  | 0x004b5d00 | `OrientationAxesFromAngles` (`Vector3`/`int` and `Vec3`/`void` forms) | `Group4_OrientationAxes.cpp` |
  | 0x004fc050, 0x004fc740, 0x004fdab0, 0x004fb8c0 | `TreeSceneNode` axes, position, dirty mark, matrix | `SoultreeObject` (`SetAxesIn`, `SetPositionInFrame`, `MarkSubtreeDirty`, `SetMatrixIn`) |
  | 0x004fc690 | `SoultreeObject::TranslateIn` taking `Vector3` | the `Vec3` form |
  | 0x00505600 | `Grid1Terrain::AcquireOwnedObject` | `Terrain::AcquireOwnedObject` |
  | 0x004210f0 | `UnknownKrustyBikeView::WriteRelativePositions` | `BikeRace` |
  | 0x00404df0 | `KbTrack::SelectLapGates` | `TrackGameViewOwner` |
  | 0x00404e60, 0x0040ae30, 0x00421cb0, 0x005015b0 | Wrecker and shadow vector call views | `Vec3Call`, `Vec3DotCall`, `Vec3AddCall`, `Vec3ScaleCall` |
  | 0x004da7b0 | `UnknownKrustyUIShadow::Init` | `ProjectedShadow::Init` |
  | 0x004c5f00, 0x004da520 | `ShadowTexture`, `ShadowCameraProxy` constructors | `PCTextureMap`, `ShadowCamera` |
  | 0x0042e930, 0x004e8cf0 | `SetLensScale`, `ShadowRenderTarget::SetOwner` | `Camera::SetFieldOfView`, `RenderTarget::SetCamera` |
  | 0x004a1860 | `ShadowMatrixMultiplyInto` | `MatrixMult` |

  Each alias was checked against both `*.bindings.json` files. The view's
  address must equal the provider's. That check found one bug that predates
  this work. `GatherModelTriangles` calls 0x004fc740, which sets the
  position, but the former forwarder called `SoultreeObject::SetPositionIn`
  (0x004fc890), which translates by the vector. So the model was never moved
  to the origin before its triangles were gathered. The masked score cannot
  see a wrong call target.

  The full list is in `config/game_build.json`. The Scene readers also
  depend on the emitter, physics object, collision point, character, motion
  and LOD-object constructors and methods.

Linked scores before and after, from the `linked_accuracy.py` masked score.
The sample column is the standalone object's score.

| VA | Function | Before | After | Sample | Now from |
|---|---|---|---|---|---|
| 0x004ecd60 | `Scene::ReadStaticModels` | 0.940 | 1.000 | 1.000 | sample file |
| 0x004edfe0 | `Scene::ReadAnimations` | 0.955 | 0.998 | 0.998 | sample file |
| 0x00419970 | `BikeRace::CreateRacers` | 0.963 | 1.000 | 0.999 | bring-up copy |
| 0x0042c260 | `GatherModelTriangles` | 0.985 | 1.000 | 1.000 | bring-up copy |
| 0x0048ccc0 | `BonusObjectManager::Tick` | 0.952 | 1.000 | 1.000 | canonical src (exact since) |
| 0x0048b100 | `VisualCue::Tick` | 0.932 | 0.932 | 0.932 | sample file |
| 0x0047caa0 | `DrawableGridNodeSharedTextures::SelectTexture` | 0.967 | 0.981 | 0.981 | sample file |
| 0x005329e0 | `Wrecker::PushProbesOutOfBox` | 0.922 | 1.000 | 1.000 | sample file |
| 0x00531740 | `Wrecker::SprayContactParticles` | 0.923 | 0.995 | 0.995 | sample file |
| 0x00457ed0 | `EcoSystem::BuildCollisionObjects` | 0.888 | 0.994 | 0.994 | bring-up copy |
| 0x00459ce0 | `EcoSystem::GenerateObjects` | 0.899 | 0.918 | 0.918 | bring-up copy |
| 0x00456890 | `Vegetation::TestDistance` | 0.953 | 0.991 | 0.991 | bring-up copy |
| 0x00497e20 | `KrustyBikeCamera::Tick` | 0.938 | 0.979 | 0.979 | bring-up copy |
| 0x0048eea0 | `KrustyBike::UpdateRiderPose` | 0.958 | 0.971 | 0.971 | bring-up copy |
| 0x00493660 | `KrustyBike::Fn_00493660` | 0.987 | 0.989 | 0.989 | bring-up copy |
| 0x004da7b0 | `ProjectedShadow::Init` | 0.892 | 0.992 | 0.992 | bring-up copy (sample file since) |
| 0x004db0d0 | `ProjectedShadow::ComputeBounds` | 0.924 | 0.985 | 0.985 | bring-up copy (sample file since) |

Bytes at masked score 1.000 rose from 772244 to 795446. About 2300 of the
increase is not from switched bodies:

- exact providers such as the `CollisionObject` shapes and the
  `UnknownBikeRaceRacer` constructor are now bound, because their alias names
  appear in the map;
- `OrientationAxesFromAngles` (274 bytes) is newly linked.

The link covers every translation unit with no unresolved symbols.

Not switched, and why:

- `D3DIMSoultreeShadow` slot 30 (0x00446f40) is now linked from the sample;
  see [Shadow slot 30 rounding](#shadow-slot-30-rounding).
- ProjectedShadow is now linked from the whole sample file; see
  [ProjectedShadow near-miss bodies](#projectedshadow-near-miss-bodies).
- `KrustyBikeCamera::Tick`: the bring-up copy now has the sample's shape, but
  the sample file is not linked. Its slot 46 vector is a zero-initialised
  static, where retail's 0x0067c3e8 is (0, 1, 0).
- `KrustyBike::FindPlacement` (0x00491d10) and the Recorder worker
  (0x004e6f80): the samples score below the linked bodies.
- `FollowCamera`'s constructor (0x00462ee0, 0.932 against 0.955): the
  linked body is a reviewed candidate that corrects observable behaviour,
  and it is kept. `ProbePartialTextureUploads` (0x004cab00) was listed here
  at 0.786 against 0.801; its linked body now scores 0.826.
- Owned by other work and left alone: `CarProcedural::Tick` and
  `BaseQuarryEvent::LoadEventObjects`.

None of the new bodies has been differentially tested. The switched bodies
make the same calls in the same order as the copies they replace. The
differences are:

- the vector helpers (the Wrecker dot product now sums in retail's order);
- the debug tag strings, because `__FILE__` is now the build path;
- `Vegetation::TestDistance`, which reads the shared detail band pointer
  that `EcoSystem.cpp` sets instead of recomputing it.

## Low-scoring linked bodies audited against retail

Each body was checked against the full retail extent. The scores are linked
masked scores from `linked_accuracy.py`, before and after.

| VA | Function | Provider | Before | After | Built/retail insns |
|---|---|---|---|---|---|
| 0x00481de0 | `DrawableGridNode::WalkVisibilityQuadrants` | GriddrawNearMisses | 0.725 | 0.733 | 499/500 |
| 0x004a9e80 | `KbPlayer::Fn_004A9E80` | Group3_FollowedRacer | 0.735 | 0.987 | 78/78 |
| 0x00423ef0 | `GR_BitString::ReadBits` | BitStringRead | 0.218 | 0.505 | 50/45 |
| 0x004a1b00 | `TransformPoints` | Group0_Math | 0.275 | 0.336 | 67/76 |
| 0x004d0d40 | `Convert555To8Fast` | Group0_Pixtrans | 0.369 | 0.418 | 51/83 |
| 0x0043dc60 | `DrawableCube::BuildFaceGeometry` | CubeDrawNearMisses | 0.622 | 0.622 | 240/252 |
| 0x004f3720 | `MultiPlayerDlg::AddChatLine` | Group5_MultiPlayerDlg | 0.804 | 0.804 | 110/114 |

- **Behaviour fix, WalkVisibilityQuadrants.** After the four quadrant calls
  the count test compared `depth + 1 == 8`. Retail reloads `esi` from the
  halved `size` slot at 0x4820fe and tests `cmp esi, 8` at 0x482370, so the
  condition is `size == 8 || size == 16`. Child blocks are walked with size
  16 at depth 0, so the old test skipped the first subdivision, the halved
  size 8. There, an empty count did not clear the mask, and a new count did
  not set `changed`.
- **Fn_004A9E80.** The race view at +0x34 is reread at every use, as retail
  does, and the best run is compared through locals. Only the known
  `fstp` scheduling difference is left (samples/race/FollowRacerNearMisses.cpp).
- **ReadBits, TransformPoints, Convert555To8Fast.** No behaviour difference
  was found. The source now has retail's shape where C++ can express it:
  - ReadBits uses one result accumulator and no unreachable shift masks;
  - TransformPoints uses end-biased pointers and negated offsets;
  - Convert555To8Fast reads two pixels per dword.

  Retail's `bswap`, its ebp frames and its packed stores look hand-written
  (inference), so these scores are capped.
- **BuildFaceGeometry and AddChatLine.** Every switch case, call, string and
  format was checked; they agree with retail. What remains is block layout.
- **Halve555/Halve565** (0.715/0.709) were not changed. An earlier audit
  found no behaviour difference; the gap is the packed path.

## ProjectedShadow near-miss bodies

`src/bringup/Group5_ProjectedShadow.cpp` used to provide ComputeBounds, RenderShadow,
Present and TintCasterVertices. It rounded with `X87FloatToInt` and called
helpers that the game build already provides, because the sample could not
link or round like retail:

- The sample calls declared-only views (`ShadowMatrixMultiplyInto`,
  `ShadowVec3Call`, `ShadowTexture`, `ShadowCameraProxy`,
  `ShadowRenderTarget::SetOwner`) that had no definitions in the game build.
- Retail rounds with inline asm (`fstp [t]; fld [t]; mov eax, out;
  fistp dword ptr [eax]` in an ebp frame). Without `/QIfist`, a cast goes
  through `__ftol` and truncates.

Now `samples/physics/shadow/ProjectedShadowNearMisses.cpp` is a
`best-guess-unreviewed` candidate source compiled with `/QIfist`. It includes the
canonical `src/krusty2/shadow/ProjectedShadow.cpp` (an `included_sources`
entry), and the bring-up file was deleted. The sample also provides
`ProjectedShadow::Init` (0x004da7b0), which the `UnknownKrustyUIShadow::Init`
view is aliased to.

`ProjectedShadowFloatToInt` copies its argument through `memcpy`. That makes
VC6 store and reload it as binary32, as retail's helper does. It then casts to
`__int64`, which `/QIfist` compiles as a 64-bit `fistp` under the current
rounding mode. Any result outside `int` becomes `0x80000000`, which is the
32-bit store's integer indefinite. Exception flags are not reproduced.
Without `/QIfist` (calibration and the physics runner, which score shape
only), the cast truncates through `__ftol`.

`link_aliases` binds each view to the exact provider at the same retail
address:

| Address | View | Provider |
|---|---|---|
| 0x004a1860 | `ShadowMatrixMultiplyInto` | `MatrixMult` |
| 0x00404e60 | `ShadowVec3Call` constructor | `Vec3Call` constructor |
| 0x004c5f00 | `ShadowTexture` constructor | `PCTextureMap` constructor |
| 0x004da520 | `ShadowCameraProxy` constructor | `ShadowCamera` constructor |
| 0x0042e930 | `ShadowCameraProxy::SetLensScale` | `Camera::SetFieldOfView` |
| 0x004e8cf0 | `ShadowRenderTarget::SetOwner` | `RenderTarget::SetCamera` |

Linked-body scores (`tools/linked_accuracy.py`, masked):

| VA | Function | Bring-up | Now linked |
|---|---|---|---|
| 0x004db0d0 | `ComputeBounds` | 0.924 | 0.985 |
| 0x004db8c0 | `RenderShadow` | 0.545 | 0.738 |
| 0x004dbd30 | `Present` | 0.837 | 0.837 |
| 0x004dc2b0 | `TintCasterVertices` | 0.511 | 0.759 |

RenderShadow and TintCasterVertices are capped mainly by retail's ebp frame,
which comes from the inline asm and changes register allocation. The
[frame pragma](#frame-pointer-units) gives TintCasterVertices the frame (0.774); it lowers
RenderShadow (0.733), which stays frameless. Present
differs in how retail loads the 16-bit colour key and shadow pixel, and in
the byte operations it uses to derive the format-9 nibbles. The canonical
exact functions are unchanged, and the bytes at score 1.000 are unchanged
(772631).

Behaviour was checked against the retail disassembly and differentially
([below](#bring-up-fistp-units)). The checks covered GetPart argument order, the transform choice
(mode 3 uses 0x004a1b00), the area cull at `<= 0`, the `1 < p < size - 1`
bounds, the clip min/max, minDepth, and the ReleaseParts guard on
`+0x27c != -1`. In TintCasterVertices they covered the inclusive size bounds
and the signed height compared with the zero-extended key. In Present they
covered the format-9 sums and outputs, the row tail and the clear loops. No
behaviour bug was found. The rounding now follows the control word, whereas
`X87FloatToInt` and the sample's old helper both assumed round-to-nearest-even.

## Shadow slot 30 rounding

`D3DIMSoultreeShadow` slot 30 (0x00446f40) and `TerrainShadow` slot 30
(0x00509aa0) round the six clipped corner coordinates with retail's inline
fistp helper (`fstp [t]; fld [t]; mov eax, out; fistp dword ptr [eax]` at
0x00447301 and 0x00509eef). Both samples now call `FistpFloatToInt`
(`samples/common/FistpFloatToInt.h`) and are `candidate_sources` compiled with
`/QIfist`. The helper stores and reloads the argument as binary32 through
`memcpy`, then casts; `/QIfist` makes the cast a `fistp qword` under the
current rounding mode. The 64-bit store equals retail's 32-bit store for every
result inside `int`. ClipPolygon leaves every corner inside the clip
rectangle, so the values stay in range.

`src/bringup/Group5_D3DIMSoultreeShadow.cpp` was deleted; the sample provides
slots 29 and 30. Before linking, two sample bugs were fixed:

- `(int)(v - 0.5f)` truncated through `__ftol`, so a corner with a fraction
  below one half landed one pixel low where retail's `fistp` gives `floor(v)`.
- The outcode array was a one-entry placeholder. Retail's 0x0057efd8 array is
  indexed up to the 3000-vertex limit and runs to the vertex array at
  0x00581eb8 (0x2ee4 bytes). It is now `int[3000]`; the extent is inferred
  from the neighbouring symbol, since nothing else in retail references it.

`link_aliases` binds `D3DIMShadowCasterView::TransformVertexGroups` to
`D3DIMSoultreeObject::TransformVertexGroups` (0x00440d40, the same retail
body) and `g_d3dimShadowVertexArray` to `g_d3dimShadowVertices` (0x00581eb8).

TerrainShadow's old `TerrainShadowFloatToInt` corrected the `__ftol`
truncation to round-to-nearest-even in software. That equals retail in the
default rounding mode, so its behaviour does not change; the shape does
(683 instructions against retail's 518, now 491).

| VA | Function | Before | Now linked |
|---|---|---|---|
| 0x00446c30 | `D3DIMSoultreeShadow` slot 29 | 0.945 | 0.945 |
| 0x00446f40 | `D3DIMSoultreeShadow` slot 30 | 0.571 | 0.862 |
| 0x00509aa0 | `TerrainShadow` slot 30 | 0.595 | 0.718 |

Ruled out for the helper (D3DIM slot 30, masked, `/QIfist`): the
range-checked `__int64` cast of `ProjectedShadowFloatToInt` (0.807), and a
plain cast without the `memcpy` (0.870, but it skips the binary32 rounding of
the argument). Behaviour was checked against the retail disassembly and
differentially ([below](#bring-up-fistp-units)): the clip rectangle set order, colour, outcode senses,
mode 3 transform 0x004a1b00, the cull at `<= 0`, the minDepth test, the
ShadowFillTriangle arguments, vertex store order, the 3000 limit and the loop
bounds.

## Inline fistp rounding samples

Two more bring-up bodies that rounded with `X87FloatToInt` moved into
`/QIfist` candidate samples that call `FistpFloatToInt`, as in
[Shadow slot 30 rounding](#shadow-slot-30-rounding):

| VA | Function | Sample | Before | Now linked |
|---|---|---|---|---|
| 0x0048d780 | `KrustyBike::Fn_0048D780` (AI trick pick) | `samples/physics/bikeai/KrustyBikeTrickPick.cpp` | 0.474 | 0.773 |
| 0x0051af00 | `DropTextOverlay::Setup` | `samples/track/DropTextOverlaySetup.cpp` | 0.501 | 0.632 |

The bodies were removed from `src/bringup/Group3_KrustyBikeAISupport.cpp`
and `src/bringup/Group4_Overlay.cpp`. `DropTextOverlay::Tick` (0x0051b070)
stays a bring-up body, because `dropSeconds` near zero could push its
drop outside `int`, where the 64-bit store would differ from retail's. It now
rounds through the range-checked helper ([below](#bring-up-fistp-units)) and
lives in `src/bringup/Group4_DropTextOverlayTick.cpp`
([Frame-pointer units](#frame-pointer-units)).

Both were checked against the retail disassembly. The trick pick keeps the
row search (`t > minTime`), the group switch and second-group fallback, the
aiKind byte, the `slack < 0.1` failure and the tables at
0x0056cc60..0x0056d060. Its index lies in [-0.5, 15.5). Named `pick` and
`hold` locals keep retail's product order (rand times 1/32768 first). As one
expression, VC6 folds 1/32768 * 0.8 into a single constant, so the old body's
hold time could differ from retail's in its last bit; that was the only
behaviour difference found. Setup's offsets, scale constants, LOGFONT bytes,
face and weight choice and failure path agree; reading
`g_TrackGame->ui->gui` at each use follows retail's reload. The differential
run then found Setup's text position one pixel off under 24-bit precision
(VC6 reassociated `targetWidth * (1/640) * x`); explicit parentheses fixed it
([below](#bring-up-fistp-units)).

Ruled out (masked): `rand() / 32768.0f` and an inline `Random01` helper
compile like the single expression (0.757); a `gui` local in Setup scores
0.613. Retail's ebp frame, its shared failure epilogue and the inline
helper's pointer spill come from the inline asm. The
[frame pragma](#frame-pointer-units) now supplies the frame (Setup 0.853, the
trick pick 0.804); the epilogue and the spill remain.

## Bring-up fistp units

Bring-up bodies that are not candidate samples round through
`FistpFloatToInt` too. Their units are compiled with `/QIfist` through
`bringup_flags` in `config/game_build.json`. Each entry names one existing
`src/bringup/*.cpp` file, gives `/QIfist`, and has a reason;
`tools/build_game.py` rejects anything else. Bring-up units are compiled only
by the game build (calibration and the physics runner do not see them), so
the flag changes no scored calibration object. `src/bringup/X87FloatToInt.h`
stays for the canonical and sample callers listed below.

`/QIfist` applies to a whole translation unit, so each flagged unit was
compiled without the flag and its `__ftol` calls were listed. In the flagged
units, every `__ftol` call comes from a fistp site. Retail-truncating casts
were moved out, not converted:

- `Group4_GridLighting.cpp` light colours (`(int)(colour * 255)`, retail
  `__ftol` at 0x0049e7b1 and the other worker sites) now live in
  `Group4_GridLightColor.cpp`, which is compiled without the flag. They were
  already called out of line before the split.
- `Character::Method_0x004a9050` left `Group3_KrustyBike.cpp` (it is now the
  candidate sample described under [Four-pose motion blend](#four-pose-motion-blend)).
  `Group3_KrustyBike.cpp` keeps the 15 and 3 `__ftol` truncations of 0x00492ad0 and 0x00493660, which retail also makes.

`/QIfist` stores 64 bits, so a site is safe when its value stays inside `int`
or when only its low bits are kept. `FistpFloatToIntChecked` casts to
`__int64` and stores 0x80000000 (the 32-bit integer indefinite) outside
`int`. It is used where no range argument holds. The range arguments
(inference from the data flow) are written in each file's header comment.

| VA | Function | Unit | Helper and range | Before | Now linked |
|---|---|---|---|---|---|
| 0x0048d1e0 | `KbScoreBoard::ShowValue` | `Group4_ScoreBoard.cpp` | plain; small scores | 0.485 | 0.681 |
| 0x0049e4a0 | `GridVertexSink::LightVertices` | `Group4_GridLighting.cpp` | plain; colour bytes times facing values | 0.458 | 0.745 |
| 0x0049e6b0 | `LightAny` worker | same | same | 0.209 | 0.774 |
| 0x0049f1c0 | `LightOne` worker | same | same | 0.168 | 0.705 |
| 0x0049f8d0 | `LightAmbientPlusOne` worker | same | same | 0.206 | 0.714 |
| 0x004a9050 | `Character::Method_0x004a9050` | `CharacterMotionFourPose.cpp` (sample, `/QIfist /Op`) | plain; frame index | 0.103 | 0.727 |
| 0x004bac60 | `ParticleManager::Render3D` | `Group1_ParticleRender.cpp` | plain; only the low byte is kept | 0.608 | 0.675 |
| 0x004eab00 | `Scene::Tick` | `Group4_SceneTick.cpp` | plain; [-0.5, count - 0.5) | 0.637 | 0.686 |
| 0x00507610 | `Terrain::PrepareGeometry` | `Group3_TerrainPrepareGeometry.cpp` | plain; focal length in pixels | 0.781 | 0.929 |
| 0x0051b070 | `DropTextOverlay::Tick` | `Group4_DropTextOverlayTick.cpp` | checked; `dropSeconds` may be near zero | 0.315 | 0.607 |
| 0x0052f570 | `VisibilityClipper::TestBox` | `Group4_Visibility.cpp` | checked; a corner near the eye plane | 0.485 | 0.689 |

Scores are masked linked-body scores with `/QIfist` alone; the
[Frame-pointer units](#frame-pointer-units) table gives them with the frame
pragma.
Bound functions come from
`tools/linked_accuracy.py`. The unbound workers, Render3D, PrepareGeometry
and DropTextOverlay::Tick were scored the same way against their retail VAs.

The 0x004a9050 gain is mostly from inlining the motion sampling
(`G3SampleMotion` is now `static inline`, as retail's 677-instruction body
implies; with the old emulation, inlined, it was 0.269). No other bound score
changed.

Left alone:

- `NameOverlay::Tick` (0x00518f30) is in canonical `TrackOverlay.cpp`. That
  file has 18 truncating `__ftol` casts in other functions, and calibration
  compiles it without `/QIfist`. Its `fadeTime` could be zero, so it would
  need the checked helper. It keeps `X87FloatToInt`.
- `GridNode::IntersectSegment` (0x00483d40, `GridNodeNearMisses.cpp`) calls
  `__ftol` four times next to its four fistp sites, as retail does. The
  sample would need the rounding moved to another unit, for little gain
  (0.832). It keeps `X87FloatToInt`; differential emulation later found it
  behaves as retail (see
  [bodies checked by emulation](#collision-and-terrain-bodies-checked-by-emulation)).
- `KrustyBike::SelectRiderPose` (0x0048e3e0) was left here at first. It has
  since moved to a candidate sample; see
  [Rider pose selection rounding](#rider-pose-selection-rounding).

Behaviour: each site now rounds under the current x87 control word, as
retail's helper does. `X87FloatToInt` read the same mode through `_control87`,
so the rounding itself is unchanged for values inside `int`. The old helper's
`_control87` call also forced the converted value and any live float
temporaries out to memory, though. Without that call VC6 may keep an
intermediate in an 80-bit register, or reassociate a sum or product, where
retail rounds each step to binary32.

Every linked fistp body was therefore run against retail with
`tools/emu_diff.py` (unicorn; cases in `tools/emu_cases/`). Each case runs the
retail function and the compiled candidate on the same random and edge inputs
(fractions near .5, zero, negative and boundary values) and compares the
return value, every byte written and every logged call. Each input runs under
precision control 53 bits (0x027f, which the CRT start-up sets through
`_controlfp(_PC_53, _MCW_PC)`) and 24 bits (0x007f). Inference from DirectX
semantics: the game creates DirectDraw with `DDSCL_FPUSETUP`
(`PCVideoCard.cpp`), so gameplay most likely runs at 24 bits.

| VA | Function | Runs | Differed before | Fix | Now linked |
|---|---|---|---|---|---|
| 0x0048d1e0 | `KbScoreBoard::ShowValue` | 6000 | 18 of 2000 (53-bit) | eye vector built in a local, then copied | 0.681 |
| 0x0048d780 | trick pick (sample) | 6000 | 0 | none | 0.773 |
| 0x004a8bf0 | `Character::Method_0x004a8bf0` (motion start) | 6000 | 0 | none | unchanged |
| 0x004a8c50 | `Character::Method_0x004a8c50` (motion blend) | 3000 | 0 | none | unchanged |
| 0x004a9050 | four-pose blend (sample, `/Op`) | 600 | 0 (204 of 600 without `/Op`) | none here | 0.727 |
| 0x004bac60 | `ParticleManager::Render3D` | 1000 | 234 of 300 (53-bit), 63 of 300 (24-bit) | light-frame grouping (cross, normalize, length) | 0.675 |
| 0x004db8c0 | ProjectedShadow `RenderShadow` | 3000 | 0 | none | 0.738 |
| 0x004dc2b0 | ProjectedShadow `TintCasterVertices` | 4000 | 0 | none | 0.759 |
| 0x00446f40 | `D3DIMSoultreeShadow` slot 30 | 3000 | 0 | none | 0.862 |
| 0x00509aa0 | `TerrainShadow` slot 30 | 3000 | 0 | none | 0.718 |
| 0x004eab00 | `Scene::Tick` | 4000 | 0 | none | 0.686 |
| 0x00507610 | `Terrain::PrepareGeometry` | 2000 | 28 of 1000 per word, all out of `int` | none (see below) | 0.929 |
| 0x0051af00 | `DropTextOverlay::Setup` (sample) | 4000 | 96 of 2000 (24-bit) | parenthesised scale | 0.632 |
| 0x0051b070 | `DropTextOverlay::Tick` | 6000 | 0 | none | 0.607 |
| 0x0052f570 | `VisibilityClipper::TestBox` | 6000 | 333 of 2000 (24-bit) | split product arrays, parenthesised x scale | 0.689 |

"Runs" counts both control words after the fix; every listed site now
differs in none of them. Render3D's differences were in the light frame and
`lengthSquared`, not in its fistp results. In-range fistp results never
differed at any site; the negative controls (the trick pick, the drop tick,
`Scene::Tick` and the shadows without `/QIfist`, the four-pose blend without
`/Op`) all differ, so the cases see the rounding. `/Op` was tried at each
differing site and was not needed (it lowers Render3D to 0.590), so
`bringup_flags` does not admit it.

PrepareGeometry's 28 differences are focal lengths outside `int`, which the
plain helper stores as 64 bits where retail stores the indefinite. The
checked helper would cost 0.929 to about 0.88; the plain helper stays on the
inference that focal lengths in pixels stay inside `int`. The ScoreBoard case
also drops out-of-`int` scores (retail then indexes its digit tables out of
range). Unicorn honours precision and rounding control but not unmasked x87
exceptions, so only masked control words were compared.
`tests/test_emu_diff.py` repeats the clean runs and the negative controls when
unicorn, `VC6_ROOT` and `MCM2_EXE` are available.

#### Second sweep: non-exact linked bodies

The same harness was then pointed at linked bodies that are not byte-exact
(physics-runner rows whose linked source is the sample or `src` body, and
linked calibration near misses), with pure computation first. Retail helpers
run as retail code; only calls with side effects outside the function
(rendering, `rand`, allocation) are stubbed. These differed from retail:

| VA | Function | Runs | Differed before | Cause and fix | Now linked |
|---|---|---|---|---|---|
| 0x0051c4f0 | `RadarOverlay::IntersectLineCircle` | 800 | 131 of 800 | the two-hit choice measured against the output buffer (uninitialised in `DrawTrackOutline`) instead of the fourth argument | 0.939 |
| 0x0040a520 | `Bike::Method_0x0040a520` (steer torque) | 600 | 140 of 600 | both left-bar arms stored the subtraction into `torque` and dotted it, where retail stores `offset` and dots `base` | 0.989 |
| 0x00436720 | `CollisionObject::HullVsHull` | 600 | 29 of 600 (24-bit) | second contact-normal rotation grouped `(y + z) + x` in place as retail | 0.978 |
| 0x00427c10 | `SegmentTriangleQuery` (sample) | 800 | 32 of 800 | end point's y row grouped `(y + z) + x` (`TransformPointYZX`) | 0.955 |
| 0x00517340 | `Track::PlaceNear` (sample, via `KbRaceSub::PlaceNear`) | 600 | 44 of 600 | path loop's second `DistanceAlong` took (candidate, start); retail passes (start, from) | 0.842 (0.868 before) |
| 0x00491d10 | `KrustyBike::FindPlacement` | 600 | 112 of 600 | stored-vector path returned the pointer; retail leaves the bits of `outPosition->z` in eax | 0.947 |
| 0x00414370 | `KrustyBike::Fn_00414370` (AI racing line) | 1000 | 114 of 1000 | outside free roam retail reads the trick delay from the difficulty's frame slot; an anonymous union says so | 0.899 |
| 0x0048eea0 | `KrustyBike::UpdateRiderPose` | 1500 | 28 of 1500 | retail indexes the 16-entry `s_kbTrickFrame` (0x0056d060) with chained tricks (16..31) too and reads the next 16 words of `.data`; the bring-up copy now has 32 entries holding those words ([below](#chained-trick-frame-overread)) | 0.955 |
| 0x005097d0 | `TerrainShadow` slot 28 | 600 | 0 | latent: the orthographic scale read an unset `fov` that VC6 happened to place in `extent`'s slot; it now reads `extent` (retail `fmul [esp+0x4c]` at 0x00509949) | 0.986 |
| 0x00426be0 | `BoxTriangleQuery` (sample) | 4500 | 2 of 2250 at 53-bit (0 at 24-bit) | retail keeps `p2 = p0 - Dot(e1)` on the x87 stack (`fsubr ; fst`) and compares p1 against it unrounded; the named `float p2` was stored and rounded to binary32. `QueryMinBFirst`/`QueryMaxBFirst` on `(p0 - d2, p0 - d1)` evaluate and compare in retail's order | 1.000 (0.979 before) |

These agreed with retail in every run at both control words (runs in
brackets): `SegmentSegmentDistance` 0x0042a640, `BoxCapsuleOverlap`
0x00425900 (888), `TreeTreeQueryNodes` 0x004280e0 (504), `QuadTree::Insert`
0x004dcac0 and `Remove` 0x004dcf20 (800 each),
`VisibilityQuadTreeNode::Traverse` 0x0052d610 (1200), the CollisionObject
hull/model pairs and their swept forms 0x00436af0..0x00438550,
`CollisionRelativeFrame` 0x00432260, `UpdateModelBounds` 0x00434eb0 and
`SegmentTouchesObject` 0x00439820 (600 each), the vehicle constructor
0x005257a0, wheel impulse 0x00527a20, lean 0x005286a0 and body contact impulse
0x005265d0, `ResolveContactImpulse` 0x00500220, `ContactSolveImpulse`
0x005004a0, `ConstraintMethodCollisionModel::ApplyContactImpulse` 0x0043bdb0,
`SoultreeObject::SetMatrixIn`/`SetAxesIn` 0x004fb8c0/0x004fc050 (600 each),
`RotatePose` 0x004a7dc0, `PoseRotation` 0x004a7fd0, Character slot 7
0x004a70c0, `TransformPoints` 0x004a1b00, `Halve555`/`Halve565`/`Halve8888`,
`Convert555To8Fast` 0x004d0d40, `GR_BitString::ReadBits` and both shifts,
`MeanTextureChannelDifference` 0x004cb330 (600), `TextureHasColor` 0x0047b490
(400), `ShadowFillTriangle` 0x00461e60 (400), `CarProcedural::PositionAtTime`
0x00430b10 (800), `DrawableGridNode::ComputeViewerDetail` 0x004815e0 (1000),
`UIButton`/`UIMultiState::ContainsPoint` 0x004734c0/0x00478e10 (600 each),
`D3DIMSoultreeObject::Scale` 0x004fd340 (800),
`ProjectedShadow::ComputeBounds` 0x004db0d0 (600),
`KrustyBike::IsSpeedGainBelowLimit` 0x004919a0, `Fn_00492AD0` and `Vegetation::GetCollisionObject` 0x004570a0
(1000 each).

`BoxTriangleQuery` 0x00426be0 differed only at 53-bit precision, and only in
boundary cases (the old shape needs about `-n 600` to show it). Retail at
0x00427505..0x00427561 computes p2 first, `fst`s it to a spare slot and keeps
it on the x87 stack, so the inner min/max compares p1 against the unrounded p2
(`fcomp st(1)`). Without that extra precision the two shapes agree, which
matches 0 differences at 24-bit. The fix keeps every instruction aligned with
retail and the 0x164 frame; only stack-slot assignment differs (linked strict
0.676 -> 0.709). The ruled-out source forms are in the sample's header.

Negative controls: the pre-fix shapes of the radar, `PlaceNear`, `FindPlacement`,
`HullVsHull` (29 of 600), `SegmentTriangleQuery` (20 of 600) and the bike
steer (50 of 200) all differ; so do a `ModelVsModel` reciprocal for its divide
(13 of 600), `>=` and dropped-clear variants of the quadtree and visibility
traversal, and `extentX` in TerrainShadow slot 28 (8 of 200).

`ProjectedShadow::Present` (Direct3D calls) and
`MorphBastardModifier::ControllerAngles` (orchestration) are not emulated.

#### Call-record/replay cases

The large orchestration bodies run in the replay mode of `tools/emu_diff.py`
(a case declares `REPLAY`). Every call that leaves the body, direct or
virtual, is logged with its target, ECX, stack words and const pointees,
then skipped. The callee result and out-parameter writes are synthetic and
seeded from (seed, target, occurrence), so both bodies see the same callee
behaviour. The comparison checks the call sequence, the body's writes and its
return. Callee behaviour the body depends on is declared per case, for example
`PredictLanding`'s landing time and `SimulateLookAhead`'s airborne flag and
suspension travel. Agreement here says the body's own logic matches; it says
nothing about the callees.

| VA | Function | Case | Cases per control word | Differ |
|---|---|---|---|---|
| 0x0052a940 | `Vehicle::SimulateFrame` (slot 49) | `vehicle_simulate_frame` | 100 | 0 |
| 0x00413200 | `KrustyBike::Fn_00413200` (AI physics step) | `kb_ai_step` | 300 | 0 after the fix below (20 of 100 at 53-bit before) |
| 0x00493660 | `KrustyBike::Fn_00493660` (remote state interpolation) | `kb_net_interpolate` | 200 | 0 |
| 0x004685d0 | `Game::Tick` (samples/game/GameTick.cpp, read-only) | `game_tick` | 200 | 0 |
| 0x0040eca0 | `SimulateLookAhead` (AI two-wheel look-ahead) | `kb_look_ahead` | 200 | 0 at 53-bit; 0 at 24-bit within 256 ulps or 1e-5 (139 of 200 inexact, worst 142 ulps) |
| 0x004a3c80 | `MorphBastardModifier::ControllerAngles` | `morph_controller_angles` | 500 | 0 after the two fixes below (1 and 8 of 40 at 53/24-bit before) |
| 0x004a4c60 | `MorphBastardModifier::ModifyMesh` (slot 27) | `morph_modify_mesh` | 200 | 0 |

`Fn_00413200` smoothed the forward acceleration with `(dt, target)` and
kept the target in an 80-bit register. Retail loads `dt` first and spills the
target to a float slot, so at 53-bit precision the target is rounded to binary32.
The old shape put `smoothedForwardAccel` (bike+0x590) and the smoother's
value 1 ulp off. `BikeAISmoothValue` now takes `(target, dt)` and returns the
smoother's store, as `VehSmooth` does. This also raised the
`match_one` score (masked 1862 to 1971 of 3744, aligned 1097 to 1102 of 1120).
The pre-fix shape is kept as a negative control in `tests/test_emu_diff.py`.

`kb_look_ahead` is the replay counterpart of the third sweep's
`kb_simulate_look_ahead` ([below](#third-sweep)), which runs the callees as
retail code and so only compares at 53-bit. Here every callee except the
vector math is replayed. That isolates the body at 24-bit too, and it reaches
what the other case cannot steer: a failed `PredictPathMotion`, path braking
to a stop and both steering signs. It found the same default side axis and
final rear ground sample as the third sweep, whose fixes are the ones in the
source. All three pre-fix shapes differ here at n = 30: the side axis in 6 of
30 runs per control word, the ground sample in 27 of 30, and the lean blend
(0x00412aa1) in 13 of 30 at 53-bit only.

At 24-bit the inlined three-term dots sum in another order than retail's, so
the case declares `FLOAT_ULPS = {0x007f: 256}` and `FLOAT_ABS = 1e-5`. With
those bounds, `run_diff` counts an outcome as `within_ulps` when its only
differences are whole float32 words within bound; the 53-bit runs must still
match exactly.

Replay extensions used here (generic, unit-tested):
- `inout` lists pointees that are logged on entry and left out of the
  synthetic out-writes, so an effect sees what the body passed. The terrain
  query keeps x and z and writes y.
- A `pointees` entry may be `(index, bytes)`.
- `FLOAT_ULPS` is an int or a `{fpcw: ulps}` dict, and `--ulps N` overrides it.

`ControllerAngles` replays only `SoultreeObject::GetMatrixIn` 0x004fca80,
which writes the controller's world matrix. The vector and matrix helpers and
the angle helper 0x004a3be0 (acos) run as retail code. The inputs cover random
and skewed rotations, half turns, small angles and the identity. They also
cover exact rotations about one principal axis with an identity rest pose,
which reach every block's zero-cross fallback axis. Two rounding differences
in the linked Group 3 copy were found and fixed:
- **Controller product.** Retail sums each element of `world * inverse` from
  the last column (0x004a3cde: `w03*i30 + w02*i20` first). VC6 canonicalises a
  flat four-term sum to start from `w00*i00`, whatever order the terms are
  written in, so the source now groups each sum from the right:
  `a + (b + (c + d))`. Reverting element (0, 0) alone differs in 4 of 60 runs
  at 24-bit.
- **x block rotation.** Retail computes `t*y` and `t*z` once, stores them as
  binary32 and builds the rows from them (0x004a4395, 0x004a43c0). The out-of-line
  `AxisAngleMatrix` 0x0042de90 instead shares `t*x`, so its text inlined does
  not reproduce the block. The source now names `ty` and `tz`. VC6 still
  multiplies the first use of each by the unrounded product (`fst` and
  continue), which no run has told apart. Reverting this alone differs in 2 of
  60 runs per control word.

With both fixes `morph_controller_angles` agrees in 1000 of 1000 runs. Both
pre-fix shapes are negative controls in `tests/test_emu_diff.py`. The
`match_one` alignment drops from 812 to 803 of 896, with 84 to 93 differing
instructions; the function was not exact before.
`samples/render/MorphBastardModifierNearMisses.cpp` keeps the old form for its
score.

`ModifyMesh` replays CopyMesh 0x004a5290, whose effect allocates the copy
with its own vertices, and UpdateChannels 0x004a4bb0, so the channel values
are inputs. It also replays the empty stub 0x00464e80 and `operator new`
0x004a3010. That last call is noted by size and line rather than logged,
because its `__FILE__` pointer is the bring-up file's own. The inputs cover:
- values at or below the first target, at or above the last, between two
  targets and on a target;
- targets with no deltas, and a group that does not start at vertex 0;
- an existing mesh copy and existing delta arrays;
- the draw-unchanged return.

ModifyMesh agrees in all 200 runs. Scaling the blend weight's `upper` by
1.0001 differs in 16 of 30.

Three bodies were reviewed without a replay case:
- `PredictLanding` 0x0040e370 and `BikeRace::CreateRacers` 0x00419970 have a
  linked masked score of 1.000.
- `BaseQuarryEvent::LoadEventObjects` 0x004de590 (`match_one` 1961 of 2209
  aligned) makes 229 calls and reads half a dozen globals, which is too much
  setup for a replay case. A reading of the full `match_one` diff found only:
  - register and stack-slot allocation;
  - load scheduling around calls;
  - an integer budget sum added in another order;
  - an `add` whose flags replace a `test`;
  - the racer-ignore loop rotated, with the same five `AddIgnoredOwner`
    calls in the same order.
  It found no change to a branch condition, a call argument or the one float
  expression (shadow grey, `ambientRGBA[1] * 255`).

  This reading is not an emulation result.

`Game::Tick` agrees except for a difference that is there by design. The
candidate reads the profiling counter through `QueryPerformanceCounter`
where retail executes RDTSC, so the case leaves that call out of the log and
the counter-derived timer words out of the write comparison.

Perturbation controls all differ:
- a `SimulateFrame` constant 0.85 to 0.86 (22 of 120);
- the `Fn_00493660` replay lag 0.27 to 0.28 (4 of 400);
- the `Fn_00413200` steer division turned into a multiplication (12);
- the `Game::Tick` frame clamp 0.2 to 0.25 (96).

The candidate-sample twins follow the linked fixes where the score allows:
`samples/physics/bikeai/BikeAINearMisses.cpp` takes the trick-delay union
(588 -> 589 bytes); `samples/physics/krustybike/KrustyBikeNearMisses.cpp` keeps
`FindPlacement`'s fall-off-the-end copy path, because the explicit
`return *(int*)&outPosition->z` reloads z there (451 -> 435).

#### Chained-trick frame overread

`UpdateRiderPose` 0x0048eea0 reads `s_kbTrickFrame` as
`mov ecx, [ecx*4 + 0x0056d060]` from the sign-extended `+0x433` byte, in the
hold-window arm, which only checks `motionIndex >= 0` (the clip-length clamp
above it checks `< 16`). Writers of `+0x433` (confirmed by decoding): 0 and -1
(head turn), `Fn_0048D910(idx)` with idx 0..15 (0 or 8 plus
`ClassifyInputDirection()`, 0..7, at 0x0048e584, message code - 1 for codes 1..16 at
0x00495276/0x004955f8), and `Fn_0048D990(idx)`, which stores `idx + 16`. Its
callers pass `motionIndex` when it is below 16, `motionIndex - 16` when it is
above 15, or (0x0049562c) `motionIndex` when it equals message code - 17;
senders encode `motionIndex + 1`, and the head turn as 17 (0x004927b9,
0x004930f1). So retail peers
keep the index in 0..31, and chained tricks 16..31 read the 64 bytes at
0x0056d0a0..0x0056d0e0 (tier 1 bytes; owners tier 2):

| Index | VA | Word | Owner |
|---|---|---|---|
| 16 | 0x0056d0a0 | 1 | `s_kbTimeTo60Armed` (KrustyBike Tick, xrefs 0x00497b59/9f/bf) |
| 17 | 0x0056d0a4 | 0 | no code reference; probably alignment before the TypeDescriptor |
| 18-23 | 0x0056d0a8 | 0x00558f4c, 0, ".?AVKrustyBike@@" | KrustyBike TypeDescriptor (`rtti_classes.json`) |
| 24 | 0x0056d0c0 | 0 | name NUL and pad |
| 25-28 | 0x0056d0c4 | "RiderMorph.mfb" | `??_C` literal (xref 0x00491109) |
| 29-31 | 0x0056d0d4 | "Interpolate" (first 12 bytes) | `??_C` literal (xref 0x00490b57) |

The window `frame > t && frame < t + 3` therefore opens for chained tricks 0
(frames 2-3), 1, 3 and 8 (frames 1-2) and never for the others. The table
before it is the trick pick's (clip lengths 0x0056cc60 and 12 rows of 0x50
bytes 0x0056cca0..0x0056d060), not a second frame group, and the word after it
is a variable that `Tick` writes, so the original is most plausibly a 16-entry
table read out of bounds (inference). The linked build's bring-up copy
`s_g3TrickFrame` has 32 entries, the last 16 being those retail words; its
128 bytes equal retail's 0x0056d060..0x0056d0e0 in the linked `.rdata`
(0x00558cd8 in the current build). Only the debug overlay's TimeTo60
stopwatch clears `s_kbTimeTo60Armed`, which would move chained trick 0's
window to frames 1-2 in retail; the copy keeps the initial 1.
`tools/emu_cases/kb_update_rider_pose.py` now draws indices 0..31 and forces
the hold window for chained tricks in about 30 % of trick cases: 0 of 1500
differ at both control words; the 16-entry object differs in 28 of 1500.

No other KrustyBike table is indexed this way: the clip-length clamp is
bounded, the trick pick's row and trick indices stay inside its rows by
construction, and the member handle tables `animSetA..D` (+0x1420..+0x1520,
16 each) see only 0..15 under the invariants above. A peer that sent a code
above 32 could push `motionIndex` past 31 and `animSetC`/`animSetD` out of
bounds; retail peers do not. Chained trick 0 is sent as code 17, the head-turn
code, so receivers never chain it (their chain test needs a code above 17), a
retail protocol quirk.

#### Third sweep

The third pass took the remaining bound linked bodies below a masked score of
1.0, ranked by size times the missing score and weighted toward gameplay. It
left out pure Direct3D bodies and the orchestration bodies that the
call-record/replay cases cover. Each case is in
`tools/emu_cases/` and runs at both control words unless noted; runs count
both control words. These agreed with retail in every run (runs in brackets):

- Physics and collision: `Tire::UpdateContactPatch` 0x005135f0 (600),
  `Tire::UpdateSuspensionProbe` 0x00514550, `RotatingShock::SolveContact`
  0x004fac60, `Wrecker::PushProbesOutOfBox` 0x005329e0 and
  `SprayContactParticles` 0x00531740 (120 each), `BoxOverlap` 0x00424ab0 (80),
  `CollisionObject::TestHullAgainst` 0x00438b90, `TestModelAgainst` 0x00438c10
  and `FreeShape` 0x00432430 (200 each), `GatherModelTriangles` 0x0042c260,
  `Vegetation::TestDistance` 0x00456890, `KrustyBike::SimulateFrame`
  0x00496790 and `EcoSystem::BuildCollisionObjects` 0x00457ed0 (120 each).
- AI and race: `PathSteeringDirection` 0x0040e510, `QuerySegmentObstacles`
  0x00415640, `BuildStatusList` 0x004e5d00, `PCLastRaceDlg::PayOutRace`
  0x004d8c40, `TrackGameViewOwner::PickFollowedRacer` 0x004a9d20 and
  `KrustyBike::SelectRiderPose` 0x0048e3e0 (600 each),
  `TrackGameMode::ResetDefaults` 0x00522440 (200), `KbPlayer::Fn_004A9E80`
  0x004a9e80 (100).
- Cameras and input: `Camera::UpdateMatrices` 0x0042ee30 (120),
  `FollowCamera::Tick` 0x00465c20 (160), `ProcessCameraInput` 0x00463a30
  (200), the constructor 0x00462ee0 (40) and `LoadCameraFile` 0x004650e0
  (160), `KrustyBikeCamera::Tick` 0x00497e20 (100),
  `PCJoystickDevice::ReadBufferedState` 0x004c3100 (160) and
  `ReadImmediateState` 0x004c3790 (100), `VCRInterface::Tick` 0x004e7d60
  (200).
- Terrain, shadows and grid: `ShadowTransformPointsProjective` 0x004a1a50,
  `TerrainShutdownObject::LookupCell` 0x00484d70, `GridVertexCache::GetVertex`
  0x0047de90, `ShadowCaster::GetVertexCount` 0x00444fe0 (600 each),
  `DrawableGridNode::WalkVisibilityQuadrants` 0x00481de0 (400),
  `GridVertexSink::LightVertices` 0x0049e4a0 with its three workers linked
  (600).
- Network and overlays: `ReceiveMessages` 0x004af8e0 and
  `HandleReceivedMessage` 0x004afa90 (600 each),
  `NetworkInterface::DispatchMessages` 0x004aced0 (400),
  `RadarOverlay::DrawRacers` 0x0051bed0 and `StatsOverlay::DrawStandings`
  0x00519a20 (600 each; GDI, the surface calls and `sprintf` are stubbed).

Every case has at least one negative control that differs, for example: the
contact-patch dot regrouped `x+y+z` (8 of 200), the tire and shock cross
products' arguments swapped (36 and 26 of 120), `TestModelAgainst` with
`case 5` (18 of 120), `KrustyBike::SimulateFrame` with the frame limit 0x1488
(4 of 120), `PathSteeringDirection` without its `fabs` (16 of 200),
`PayOutRace` capped by the repair percentage (52 of 120), `LookupCell`'s z
compared as `unsigned short` (16 of 200), the projective y divided by `1/w`
(46 of 120), `LightVertices`' rounding offset `+0.5` (8 of 300),
`ReceiveMessages` with `size > 4` (38 of 120), `DispatchMessages` with
`type >= 0x40` (96 of 120) and `DrawStandings` capped at five rows (14 of 120).
A few controls were equivalent rewrites and gave 0, for example
`DrawStandings`' `own > shown`.

Open: the follow-camera constructor copies axes from `.bss` that both images
leave zero, so its case cannot tell a swapped axis component.

Not emulated: `EventManager::LeaveRaceForMenu` 0x0045e710 (masked 1.0; every
call is GUI, sound, display or allocation, so a case would test only its
stubs).

Bodies that the replay cases also cover were checked here first. These
differed from retail and are fixed:

| VA | Function | Runs | Differed before -> after | Cause and fix | Masked |
|---|---|---|---|---|---|
| 0x004210f0 | `BikeRace::WriteRelativePositions` | 300 | 22 -> 11 (53-bit), 11 -> 0 (24-bit) | the grid-step normalisation summed the square `x+y+z` and scaled inline; retail calls `Vec3Dot` (0x0040ae30, `(y+x)+z`) and `Vec3Scale` (0x005015b0) | 0.958 -> 0.981 |
| 0x004210f0 | (same) | 300 | 11 -> 0 (53-bit) | grid-slot sums and scales stayed on the x87 stack through inline operators; retail rounds each scaled vector to binary32 in the out-of-line constructor (0x00404e60) or calls the cdecl add and subtract helpers 0x00421cb0/0x00421d00. Per-site call views (`Vector3Call`, `BikeRaceAddCall`, `BikeRaceSubtractCall`, link-aliased) say so | (same) |
| 0x0040eca0 | `SimulateLookAhead` | 100 (53-bit) | 77 -> 0 | three causes: with no side axis retail crosses the up-cross with `fwd` again before `SafeNormalize` (0x0040ee7c); the final rear ground query starts from the last step's point and keeps the last step's normals, where the source read an unset local; the lean blend scales each component by the unrounded reciprocal (0x00412aa1) | 0.954 -> 0.954 |

`SimulateLookAhead`'s `match_one` alignment fell from 3739 to 3672 of 3966
instructions with these fixes. Its case runs at 53-bit only: at 24-bit 41 of
60 runs differ by one ulp because VC6 reorders the inlined three-term dot
products (retail sums x, y, z at 0x00411a44). One generated input (index 130)
does not return within 200M instructions in either image; the clean count
stays below it. `BikeRace::AimCamera` 0x0041eb20 and `NextCameraTarget`
0x0041f1d0 (200 each), `VisualCue::Tick` 0x0048b100 (600) and
`EcoSystem::GenerateObjects` 0x00459ce0 (30) agreed in every run.

`EcoSystem::GenerateObjects` weighted a definition's texture channel as
`(channel / 255) * probability`; retail multiplies `(channel * probability)`
first and then scales by 1/255. The two agree at 53-bit; at 24-bit 20333 of the
65025 channel and probability pairs differ by one ulp (exhaustive check), which
can flip a draw that lands on the threshold. The source now uses retail's order
(`UnknownEcoSampleTgaWeighted`; `fild; fmul weight; fmul 1/255` as at retail
0x0045a7e9), accepting VC6's different register allocation of the body.

A gameplay sweep added replay cases for linked bodies below 1.000 masked that no
case covered: `net_create_address` (`NetworkInterface::CreateAddress`
0x004abf10), `race_sound_refill_stream` (`RaceSound::RefillStream` 0x004e4d10),
`sound_set_primary_format` (`PCSoundInterface::SetPrimaryFormat` 0x004be910),
`recorder_thread` (the VCR worker 0x004e6f80, all eleven events),
`bailout_dlg_handle_event` and `bunny_dlg_handle_event` (the career bail-out and
loan dialogs 0x004d8fc0 and 0x004d9860) and `sp_race_info_handle_event`
(`SPRaceInfoDlg::HandleEvent` 0x004507a0, results lists and replay tape load).
Each agrees in 400 runs at both precisions, and edited negative controls differ.
One bug: in the modem branch, `CreateAddress` tagged the modem name as
`DPAID_Phone` and the phone number as `DPAID_Modem`. The bindings had swapped the
two GUIDs as well (0x00556900 holds DPAID_Phone and 0x00556920 holds DPAID_Modem),
so neither the case nor the masked score could see the swap. With both fixed,
the modem name is tagged `DPAID_Modem` and the number `DPAID_Phone`, as in retail,
so a dial-up session gets the right modem and number. The following bodies were
reviewed by disassembly and showed register or scheduling differences only:
`LoadBike`, `SetRenderContext`, `SetPointerDevice`, the KrustyBike deleting
destructor, `QuarryOverlayHooks`, `LoadEventObjects`, `SendStartMessage`, the
audio loader thread 0x004bdc00, `LoadVue`, `CheckHeader`, TerrainShadow slot 27
and `MainDlg::HandleEvent`. A scan of the linked bodies' float constants found no
wrong values. Still open: the vector lengths in `CreatePodiumScene` 0x0045d480 and
`BikeRace::Create` 0x00417ed0. Retail rounds z*z to a float temporary and adds it
to x*x + y*y; the linked build sums (x*x + z*z) + y*y at extended precision. The
effect is ulp-level, on podium and flag-girl placement. Eight source variants did
not reproduce retail's order.

### Grid lighting workers

The three workers in `Group4_GridLighting.cpp` were rewritten from the
retail disassembly on 2026-11-11 (masked linked scores 0.236, 0.240 and
0.213 before; the table gives the scores after). They are unbound, so they
were scored against their retail VAs as above.

- Retail's out-of-line calls are kept: `Vec3TransformNormal` (0x0042a450)
  and the unary minus (0x0049f180) for directional lights in `LightAny` and
  `LightOne`; `Vec3Dot` (0x0040ae30), `FastInvSqrt` (0x00460c00) and
  `Vec3Scale` (0x005015b0) or the `Vector3` constructor (0x00404e60) for the
  normalisations. The workers take `const Matrix4*`, and a link alias maps the
  `Vector3` view of `Vec3TransformNormal` to the canonical `Vec3` symbol.
- The inverse translation, point transform, rotation and dot products use
  retail's fadd groupings, which differ per worker. They are written with
  parentheses in the source. These sums round differently in the last bit;
  at Direct3D's 24-bit precision that can move a colour byte.
- `LightAmbientPlusOne` has one vertex loop per light type and takes both
  lights' colours where the search finds the ambient light, as retail does.
- The facing value is stored as binary32 before the colour multiplies
  (`Group4Vis_Binary32`). Retail's inline-asm helper forces that value to
  memory (`fst [facing]`); under `/QIfist` VC6 would keep the unrounded
  register value. That changes exact ties by one, for example a 0.6/0.8 axis
  normal at 53-bit precision.
- `Group4Vis_Dot` and `Group4Vis_TransformPointAny` are `__forceinline`.
  `LightAny` expands them at every site, but its later sites fall outside
  VC6's inline budget.

Differential check: the scratch emulator ran the linked objects and retail's
bytes for `LightVertices` on random scenes. The scenes covered 0 to 5 lights
of each type, rigid and non-rigid matrices, axis normals, strides 0xc, 0x10
and 0x20, and both the colour-array and no-colour-array paths. Each scene ran
at control words 0x27f and 0x7f.

| Variant | Runs | Colour mismatches |
|---|---:|---:|
| Committed source | 6000 | 0 |
| Without the binary32 store | 3000 | 12 |
| With `/Op` | not counted | 253 |

`tools/emu_cases/grid_light_vertices.py` now runs the committed source the
same way (third sweep).

The binary32 store fixed the ties. `/Op` is therefore not needed. It rounds
the intermediates of `LightVertices`' modulation, which retail keeps in
registers.

Much of the remaining codegen gap was the frame. Retail's asm blocks force an
EBP frame, and `/QIfist` code uses ESP addressing. Keeping the frame pointer,
`match_one` aligned instructions for `LightAny`, `LightOne`,
`LightAmbientPlusOne` and `LightVertices` rise from 252, 166, 188 and 80 to
512, 368, 405 and 113 (of 728, 581, 697 and 167). All four are framed in
retail and they are the unit's only functions, so one frame pragma region
covers all four ([Frame-pointer units](#frame-pointer-units)).

## Frame-pointer units

Retail's inline-asm fistp helper gives every function that contains it an EBP
frame (`push ebp; mov ebp, esp`). This is inference from VC6's handling of
`__asm`, consistent with the retail prologues below: the helper's callers are
framed, their neighbours are not. Plain C++ under `/QIfist` addresses locals
through ESP. The source keeps the frame pointer for those functions alone:

```cpp
// Retail's inline-asm fistp helper gives this function an EBP frame; the
// pragma keeps the frame pointer for it alone (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
int DropTextOverlay::Tick(float frameTime) { ... }
#pragma optimize("", on)
```

With authentic VC6 SP3 the pragma applies per function: functions outside the
region compile to the same bytes as without it. That includes the Math3D.h
`$E` initializers, which blocked unit-wide `/Oy-` for the units that include
the header. A function takes the pragma when:

1. it is framed at its retail VA and contains the fistp helper (a static
   helper with no retail body, which retail expands inside such a function,
   may share the region);
2. a COFF comparison with and without the pragma shows that no other function
   in the object changes (relocations masked, `$L` labels normalized);
3. its masked and strict linked scores (`tools/linked_accuracy.py`; unbound
   bodies scored the same way at their retail VAs) do not drop, and its
   `tools/emu_diff.py` case, where there is one, differs in no more runs.

Start the region at the function, after the inline helpers it calls: a static
inline helper takes the optimization state of its definition. With the region
starting before `KbAtLeast`/`KbAtMost`/`KbRange`, `SelectRiderPose` scores
0.725 / 0.682 (as with `/Oy-`); starting at the function, 0.729 / 0.686.
`tools/build_game.py` still accepts `/Oy-` next to `/QIfist`, but no unit
uses it: the eight units that took it give the same bytes with the pragma.

Scores are masked / strict, before (`/QIfist` alone) and after the pragma.
Emu counts the `tools/emu_diff.py -n 1000` runs that differ (control words
0x027f and 0x007f; the four-pose case runs 3000), each case compiled with the
configured flags.

| Unit | Function (retail VA) | Decision | Masked | Strict | Emu |
|---|---|---|---|---|---|
| `Group1_ParticleRender.cpp` | Render3D 0x004bac60 | pragma (was `/Oy-`, same bytes) | 0.675 -> 0.712 | 0.413 -> 0.446 | 0 / 2000 |
| `Group3_TerrainPrepareGeometry.cpp` | PrepareGeometry 0x00507610 | pragma (was `/Oy-`) | 0.929 -> 0.979 | 0.777 -> 0.925 | 56 / 2000, all out of `int` (below) |
| `Group4_GridLighting.cpp` | LightVertices 0x0049e4a0, LightAny 0x0049e6b0, LightOne 0x0049f1c0, LightAmbientPlusOne 0x0049f8d0 (one region) | pragma (was `/Oy-`) | 0.745 -> 0.752, 0.774 -> 0.823, 0.705 -> 0.719, 0.714 -> 0.718 | 0.497 -> 0.639, 0.345 -> 0.590, 0.302 -> 0.367, 0.293 -> 0.353 | 0 / 600 (third sweep) |
| `Group4_DropTextOverlayTick.cpp` | DropTextOverlay::Tick 0x0051b070 | pragma (was `/Oy-`) | 0.607 -> 0.674 | 0.517 -> 0.587 | 0 / 2000 |
| `Group4_ScoreBoard.cpp` | ShowValue 0x0048d1e0, with `Group4Ovl_ShowModel` (no retail body) | pragma (was `/Oy-`) | 0.681 -> 0.715 | 0.334 -> 0.455 | 0 / 2000 |
| `Group4_Visibility.cpp` | TestBox 0x0052f570 | pragma (was `/Oy-`) | 0.689 -> 0.697 | 0.342 -> 0.443 | 0 / 2000 |
| `DropTextOverlaySetup.cpp` (sample) | Setup 0x0051af00 | pragma (was `/Oy-`) | 0.632 -> 0.853 | 0.272 -> 0.701 | 0 / 2000 |
| `CharacterMotionFourPose.cpp` (sample) | Method_0x004a9050 | pragma (was `/Oy-`) | 0.727 -> 0.758 | 0.168 -> 0.414 | 0 / 3000 |
| `Group4_SceneTick.cpp` | Scene::Tick 0x004eab00 | pragma | 0.686 -> 0.701 | 0.334 -> 0.363 | 0 / 2000 |
| `KrustyBikeTrickPick.cpp` (sample) | Fn_0048D780 0x0048d780 | pragma | 0.773 -> 0.804 | 0.599 -> 0.776 | 0 / 2000 |
| `KrustyBikeSelectRiderPose.cpp` (sample) | SelectRiderPose 0x0048e3e0 | pragma | 0.682 -> 0.729 | 0.652 -> 0.686 | 0 / 600 (third sweep) |
| `CharacterMotionStart.cpp` (sample) | Method_0x004a8bf0 | pragma | 0.633 -> 0.667 | 0.200 -> 0.381 | 0 / 2000 |
| `CharacterMotionBlend.cpp` (sample) | Method_0x004a8c50 | pragma | 0.793 -> 0.879 | 0.348 -> 0.618 | 0 / 2000 |
| `TerrainShadow.cpp` (sample) | slot 30 0x00509aa0 | pragma | 0.718 -> 0.736 | 0.442 -> 0.450 | 0 / 2000 |
| `D3DIMSoultreeShadowNearMisses.cpp` (sample) | slot 30 0x00446f40 | pragma | 0.862 -> 0.891 | 0.598 -> 0.665 | 0 / 2000 |
| `ProjectedShadowNearMisses.cpp` (sample) | TintCasterVertices 0x004dc2b0 | pragma | 0.759 -> 0.774 | 0.330 -> 0.346 | 0 / 2000 |
| `GridNodeNearMisses.cpp` (sample) | GridNode::IntersectSegment 0x00483d40 (no `/QIfist`: retail keeps four truncating `__ftol` casts) | pragma; then the leaf cell as an index and out->y stored before out->z | 0.832 -> 0.903 -> 0.916 | 0.449 -> 0.493 -> 0.520 | 0 / 3000 (three round-to-nearest control words; at -n 2000 one input faults in both bodies, as before) |
| `ProjectedShadowNearMisses.cpp` (sample) | RenderShadow 0x004db8c0 (framed in retail) | none: the pragma lowers both scores | 0.738 (0.733 with it) | 0.452 (0.340 with it) | 0 / 2000 |
| `Group4_Overlay.cpp` | ChatOverlay::Setup 0x0051cf80 (frameless) | no flags, no pragma | 1.000 | 0.964 | n/a |

The functions of these units that are frameless in retail keep their bytes,
for example `TerrainShadow` slots 27 and 28 (0.984 / 0.979, 0.986 / 0.950),
`D3DIMSoultreeShadow` slot 29 (0.945 / 0.936) and ProjectedShadow `Present`,
`ComputeBounds` and `Init` (0.837 / 0.438, 0.985 / 0.689, 0.992 / 0.841).
`DropTextOverlay::Tick` was split from `Group4_Overlay.cpp` for its unit-wide
flags; with the frame in the source it could move back, which has not been
done.

terrain_prepare_geometry differs in 0 runs with in-range focal lengths only.
With its out-of-`int` values (every 25th case) it differs in 56 of 2000 runs,
the same 56 as with `/QIfist` alone (see above).

## Rider pose selection rounding

`KrustyBike::SelectRiderPose` (0x0048e3e0, slot 89, 2744 bytes including its
two jump tables) was the last linked body that called `__ftol` (0x00534a88)
where retail does not. Retail rounds the pose index three times with its
inline fistp helper (0x0048e61f, 0x0048e6f0, 0x0048e7c5). The canonical body
emulated that with `KbFloatToInt`, a round-to-nearest-even built on `__ftol`.

Canonical `src/krusty2/vehicle/KrustyBike.cpp` cannot take `/QIfist`. It
truncates through `__ftol` in five other functions, as retail does:
0x00492670 (3 casts), 0x00495c00 (4), 0x00495ff0 (2), InitForceFeedback (1)
and UpdateForceFeedback (1). So the body moved to
`samples/physics/krustybike/KrustyBikeSelectRiderPose.cpp`. The game build
compiles it with `/QIfist` (`config/game_build.json` `candidate_sources`),
and the canonical definition was removed. The sample rounds with
`FistpFloatToIntChecked`. The argument is
`(0.75 - steerValue) * 3.99 - 0.5`, and no range argument bounds
`steerValue`, so out-of-`int` values must store 0x80000000 as retail's 32-bit
fistp does.

The full extent was checked against the retail disassembly. This includes the
4-entry table at 0x0048ee74 (crossover cases 1-4) and the 5-entry table at
0x0048ee84 (cases 1-5). Thresholds, comparison senses, stored fields, pose
indices and calls agree with the source. Rounding was the only behavioural
difference.

The old and new bodies were compared with retail by unicorn emulation: the
compiled object was placed in the retail image and run over 3000 random
states for each of six x87 control words. The trick paths (0x0048d910,
0x0048e280) were included. The old body differed in 3334 of 18000 cases:

| Control word | Old body mismatches (of 3000) |
|---|---:|
| 0x007f (24-bit, nearest) | 308 |
| 0x027f (53-bit, nearest) | 454 |
| 0x037f (64-bit, nearest) | 454 |
| 0x0e7f (53-bit, truncate) | 533 |
| 0x047f (24-bit, down) | 778 |
| 0x087f (24-bit, up) | 807 |

There were three causes:

- **Out-of-range values.** Infinities, NaN and very large steer values gave a
  wrong index where retail stores 0x80000000 and adds the pose base. This
  accounts for the 308 cases in every mode.
- **Precision.** At 53- and 64-bit precision the argument was not rounded to
  binary32 first. There were 146 more cases at 0x027f; in 91 of them
  |steer| <= 2, and the pose index moved by one at a boundary (two through
  the pose state).
- **Rounding mode.** The old body ignored the rounding mode, which caused the
  extra cases in the directed modes.

The `/QIfist` sample matches retail bit for bit in all 18000 cases. **`/Op` is
not needed**: the helper's binary32 store already rounds the argument. With
`/QIfist /Op` the candidate grows to 2768 bytes without any behaviour change.

Cast audit of the sample unit:

- Without `/QIfist`: three `__ftol` relocations, all of them the fistp sites
  in SelectRiderPose.
- With `/QIfist`: none. The unit's `_$E` initializers have no casts.

The other functions in the canonical unit compile to the same bytes as
before; only internal `$L` label numbers changed.

Linked masked score: 0.650 -> 0.682 (637 instructions against retail's 703;
the old body had 715). `tools/linked_accuracy.py` now lists no body that calls
a function retail's body does not call.

Code shape (`tools/match_one.py`):

| Variant | Strict | Aligned | Candidate bytes |
|---|---:|---:|---:|
| Old canonical | 138/2744 | 439/718 | 2708 |
| Checked helper, `/QIfist` (linked) | 152/2744 | 442/718 | 2552 |
| Plain `FistpFloatToInt` | | 450/718 | 2452 |

The plain helper keeps the low dword outside `int`, so it was not adopted.
The rest of the gap is retail's ebp frame from the inline asm, which also
moves the shared clamp exits. The [frame pragma](#frame-pointer-units) now supplies the frame
(0.729 masked, 0.686 strict, linked). An `int` return with `return 0` was ruled out:
about 101 strict masked, 2880 bytes, `xor eax,eax` on every exit, and the
0x5b8 branch no longer shares the switch tails.

The physics runner compiles the sample without `/QIfist`, so it scores only
shape there (`expect: partial`).

## Four-pose motion blend

`Character::Method_0x004a9050` (thiscall, `ret 0x1c`, 2138 bytes) is the
blend that `KrustyBike::UpdateRiderPose` (0x0048eea0) calls on the rider and
on the bike. It is now `samples/physics/motion/CharacterMotionFourPose.cpp`, a
`reviewed-nonexact` candidate compiled with `/QIfist /Op`. The
reimplementation in `src/bringup/Group3_KrustyBike.cpp` was removed.

What it does (decoded from the retail instructions):

- Each of the four motions is sampled at the same phase. `position =
  (frameCount - 1) * phase` is stored as binary32. `frameIndex =
  fistp(position - 0.5f)` uses the current rounding mode, `fraction =
  position - frameIndex`, and `next` is the frame itself on the last frame.
- The first motion and its sampled frame become `currentMotion` (+0x18) and
  `currentFrame` (+0x1c).
- For each pose of `currentMotion` (`+0xac`, reloaded every iteration):
  - motions a and b go through the clamped interpolation that 0x004a8c50
    also expands;
  - c and d go through `InterpolatePose` (0x004a8470);
  - the results are mixed `(a, b)` and `(c, d)` by `1 - weight`, then by
    `1 - lean`;
  - the result goes to vtable +0x10 with a second argument of 0.

Flags:

- `/QIfist` makes `FistpFloatToInt` a `fistp`.
- `/Op` stores `position` as binary32, as retail's asm helper does. Without
  it, VC6 keeps the product in an x87 register, and the poses differ from
  retail in their last bits. This sample without `/Op`, and a bring-up body
  that inlined the sampling under `/QIfist` alone, each differed in about a
  third of the emulated runs (204 of 600 for the bring-up body).

The old bring-up body got the same rounding by accident: its `X87FloatToInt`
calls `_control87`, which spills `position`.

A differential run under unicorn found no behaviour difference. It ran retail
0x004a9050 and the relocated candidate object inside the retail image, using
retail's helpers, a stub vtable slot and random motions, with 1 to 61 frames,
1 to 5 poses, edge phases and weights outside [0, 1]. It covered:

- 3400 runs for the candidate, at 53-bit and 24-bit precision control;
- 1000 runs for the old bring-up body.

Both matched retail bit for bit: the recorded poses and the `currentMotion`
and `currentFrame` fields.

| VA | Function | Bring-up | Now linked |
|---|---|---|---|
| 0x004a9050 | `Character::Method_0x004a9050` | 0.103 | 0.727 |

Retail's ebp frame and pointer `fistp` come from the inline asm; the
[frame pragma](#frame-pointer-units) supplies the frame (0.758). The other known differences
are dead code:

- Retail calls CharacterPose's generated constructor (0x004a8930) three
  times in the first interpolation expansion and four times in the second.
- Only the second expansion stores its unused weighted sums.

Strong inference: the inline declares four pose locals, and the first
expansion inlined its first constructor.

Ruled-out forms are listed in `samples/physics/motion/targets.json`.

## Collision and terrain bodies checked by emulation

Three gameplay-critical linked bodies were run against retail under unicorn.
Each harness places the compiled candidate function in the retail image,
binds its calls to retail's helpers and runs both bodies on the same random
inputs. The fast-math tables (.bss) come from retail's own initializers. The
first runs used ad hoc harnesses. They are now `tools/emu_diff.py` cases:
`terrain_query_ground`, `collision_set_transform` and
`gridnode_intersect_segment`. `tests/test_emu_diff.py` runs them with small
counts.

Each case catches a known-bad object with the default seed:

- QueryGround: the pre-d3c99491 `Terrain.cpp`.
- SetTransform: one reassociated transform-point row.
- IntersectSegment: round-half-up at the midpoint ties.

```bash
python tools/emu_diff.py terrain_query_ground --compile -n 500
python tools/emu_diff.py terrain_query_ground --object OLD_TERRAIN.obj -n 40
```

The tables below come from the first runs. The cases compare status, EAX,
ESP, callee-saved registers, the x87 stack top, calls and every byte written.
Unlike the first harnesses, they do not compare the x87 status flags or the
control word.

| VA | Function | Cases | Behavior found | Linked before | After | Built/retail insns |
|---|---|---|---|---|---|---|
| 0x00507c10 | `Terrain::QueryGround` (via the `TireWorld` view) | 6000 | two bugs, fixed | 0.864 | 0.899 | 1009 → 995 / 970 |
| 0x00435830 | `CollisionObject::SetTransform` | 3000 | none | 0.833 | 0.833 | 513 / 512 |
| 0x00483d40 | `GridNode::IntersectSegment` | 6000 | none | 0.832 | 0.832 (0.916 with the [frame pragma](#frame-pointer-units)) | 1349 / 1387 (now 1369) |

Coverage and results:

- **QueryGround.** GetCellCorners (0x00483910) is the same deterministic
  stub in both runs. Inputs: cell ties, diagonals, huge, NaN and infinite
  coordinates, optional outputs, and every x87 rounding and precision mode.
  The first harness never reached the NaN, infinity and int-edge inputs,
  because a later branch overwrote them. The committed case does reach them,
  and they agree (500 seeds in each of six modes). `kVec3YAxis` is a file
  static with a constructor, so the case binds it to retail's 0x0068a058.
  The [two repairs](TERRAIN_CELL_CONVERSION.md) are the corner set of the
  smooth (0, 1, 3) normal and the height addition order. 431 cases differed
  before them and none after. That includes the x87 status flags, control
  word and callee-saved registers.
- **SetTransform.** All five shape types run. The hull, model and mesh
  payloads are random, and model elements number 0 to 5, some disabled. The
  shape's scene-node `GetMatrixIn` (0x004fca80) is a deterministic stub.
  Everything the call writes into the object and shape memory matched,
  bit for bit. The decoded addition order of the hull product, recorded in
  the source, is right.

  The codegen gap is the operand order inside each product (`fld a ; fmul b`
  against `fld b ; fmul a`). VC6 picks that order itself. Swapping the
  operands in the source, reordering the sum terms or swapping the inline
  helper's parameters changed no byte.
- **IntersectSegment.** Inputs are random 1- to 3-level grid trees, with
  child pointers both shared and null. Segments are lattice, tie and
  axis-aligned, and run in every x87 rounding and precision mode. Children
  recurse through a stub vtable slot back into the body under test. The
  return value, out point, node, rounded hit and x87 status all match.

  Under directed rounding, the midpoint `fistp` can round a cell index out of
  range. Retail faults at the child and cell reads it indexes (0x004844ef,
  0x004844ff, 0x004846a6). Both bodies then index before the row tables and fault on the
  same inputs; none fault under round-to-nearest. The case binds the
  file-static row tables to retail's copies (0x0056c2ac, 0x0056c2f0), so both
  bodies see the same memory past them. With that binding, status, calls and
  writes agree in every run (500 seeds in each directed mode). Only EAX/ESP
  at the fault differ. The case therefore compares only round-to-nearest by
  default. The sample is now `reviewed-nonexact`.

  The codegen gap is retail's EBP frame and its four inline `fistp`
  sequences. Both come from the inline-asm helper, which the rules exclude.
  Moving `GridRoundToInt` into a `/QIfist` unit would leave the call in
  this body unchanged.

## Game and race bodies checked against retail

These linked bodies were compared with retail over their full extents. The
scores are masked `linked_accuracy.py` scores. No source changed, so each
score is the same before and after. "Static" means every differing
instruction was classified by hand: calls, call arguments, constants,
branches, field offsets and every x87 store that rounds. "Emulation" means
the body was run with `tools/emu_diff.py` under control words 0x027f and
0x007f.

| VA | Function | Method | Behaviour difference | Linked score |
|---|---|---|---|---|
| 0x004685d0 | `Game::Tick` | static | only the documented QPC counter in place of RDTSC | 0.749 |
| 0x0041f1d0 | `BikeRace::NextCameraTarget` | static, all paths | none | 0.829 |
| 0x004650e0 | `FollowCamera::LoadCameraFile` | static (see FOLLOW_CAMERA_LOAD_FILE.md) | none | 0.842 |
| 0x00459ce0 | `EcoSystem::GenerateObjects` | static | none | 0.918 |
| 0x0048b100 | `VisualCue::Tick` | static | none | 0.932 |
| 0x0042fd80 | `CarProcedural::Tick` | emulation, 400 cases (`car_procedural_tick`) | none | 0.941 |
| 0x004cab00 | `PCVideoCard::ProbePartialTextureUploads` | static | none | 0.786 (0.826 after s21) |

- **CarProcedural::Tick.** Retail rounds the steering angle to a float slot
  (`fst [esp+0x28]`) and then compares that copy with ±π/6. The candidate
  compares the unrounded x87 value. Both bounds are floats and rounding is
  monotonic, so the stored angle is the same either way. Emulation agrees:
  0 of 800 runs differ, with the clamp reached in 176 runs and the low
  clamp in 80.

  Every stub sink logs the exact bits it receives: the sensor frame, the
  wheel and body axes, the placement and the push-out. Changing the upper
  clamp bound by one ulp made 34 of 400 runs differ.

  The code-shape gaps are the clamp spill and the order of the inline axis
  cross. Its operands are the constant axes, so every product is exact. A
  `const float&` clamp parameter gives the spill on the member rather than a
  local. That raises `match_one.py` from 506 to 573 of 2599 masked bytes,
  but it is not adopted because it is still not retail's shape. Six other
  spellings, using a float local, a chained assignment or `const&` bounds,
  changed no byte.
- **VisualCue::Tick.** `0.5f - screenY` is computed as `fld y; fsubr 0.5`
  rather than `fld 0.5; fsub y`, which is the same value. The z component of
  each placement is accumulated on the x87 stack in both bodies. The x and
  y products are rounded through the same stack slots in both.
- **ProbePartialTextureUploads.** Retail shares one release-and-delete tail
  between the success path and the failure paths. The audited candidate
  duplicated the epilogue; since s21 it returns `result` (as retail's eax
  does) and has retail's single epilogue. The release order (image, rendered, expected),
  the `-2`/`-3`/`-4` results and the `CreateSurface` arguments agree.
- **Track::PlaceNear** (0x00517340) was dropped from this pass. Another
  audit found and fixed its second `DistanceAlong` argument order by
  emulation. A static diff hides an argument swap like that one when the
  pushes come from equal-looking stack slots, so such bodies need emulation.

## Source-built run status

The source-built `mcm2.exe` (391/391 translation units, zero unresolved
symbols) links, starts on current Windows with the runtime shim, loads a track
and renders a race (rider, HUD, radar, speedometer) next to the retail
`blade.dll`/assets. Verified by launching it with `MCM2_FORCE_RUNTIME=1` (the
shim otherwise leaves its hooks off for an unknown executable hash) and reading
`frames/frame_*.bmp` and `mcm2_runtime.log`. This is a boot-to-race smoke test
only; the best-guess bodies above remain unreviewed and long play is untested.

Two environment details matter for the source build:

- The generated import libraries (`config/game-imports/*.def`) import
  `blade.dll`/`DINPUT.dll` by ordinal; retail imports by name. The runtime's
  import hooks therefore match these three by ordinal too, which is what lets
  its 16-bit display-mode emulation apply (otherwise
  `PCVideoCard::SetDisplayMode(640, 480, 16)` finds no mode and
  `PCGame::StartUp` exits silently).
- The EULA dialog (`EBUEula.dll`) is shown by the game itself and must be
  accepted by the user.

No matched `src/` code was unmatched: the only edits to existing sources are
the `g_CollisionScratchPoints` extern retype in `CollisionShapeTests.h`, its
bindings key, and four cast call sites in `CollisionObject.cpp`.
