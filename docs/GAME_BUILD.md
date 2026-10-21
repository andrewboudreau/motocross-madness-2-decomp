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

The default profile is `vc6_o2_mt`. Objects are ordered by source basename,
then path. This is a reproducible working order, not proof that reconstructed
files recover original translation units. Ordinary import libraries supply
external DLL functions; the build never resolves game functions to retail
addresses and never forces unresolved references into an executable.

## Current gaps

The 2026-10-21 build compiles 216 C++ files (184 canonical sources and 32
reviewed candidates) and reaches LINK with 377 unresolved external symbols,
down from 855 before the shared-class, global-provider and rendering integration.
This is a measured link failure, not a source-completion percentage. The
current run's `link.log` is authoritative for remaining failures.

The final composition in `build-mta0srax` retains all prior strict results:
2,981/2,992 calibration rows and 1,086/1,162 physics comparisons. This does
not establish game startup, menus or racing while linkage remains incomplete.
The full regression run passes: 485 tests, with 474 passed and eleven skipped
(`work/game-build/tests-round27.log`). Static checking also passes for 207
Python files and 351 JSON files.

The current [collision query-state connections](COLLISION_QUERY_STATE_CONNECTIONS.md)
and [vehicle gearbox connections](VEHICLE_GEARBOX_CONNECTIONS.md) remove eight
obsolete import names. All 215 prior production objects preserve 10,465
nondebug sections and 45,316 relocations under those eight verified name
substitutions. [Cache mip copying](CACHE_TEXTURE_MIPS.md) supplies one reviewed
nonexact candidate after 5,674 native scripted-boundary comparisons. The three
isolated links reduce unresolved names **386 → 383 → 378 → 377**, with no new
missing names. The scratch-array connection remains deferred because it changes
a complete partial caller. Private evidence is in
`work/game-build/round27-link-evidence.json`, `round27-all-objects.json` and
`connections-round27-preservation.json`.

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
2,978 distinct strict-exact calibration cases. A native link/runtime regression checks
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
remains a separate unresolved partial reconstruction. Six [network GUIDs](NET.md#network-identifiers)
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
`UnknownDisplay::ProbePartialTextureUploads`, so that admission leaves the
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
