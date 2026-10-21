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
Each object has an adjacent compiler log. Import-library and linker logs
are in the build directory. Failed compilation or linking returns nonzero;
an old executable cannot satisfy a subsequent build.

The default profile is `vc6_o2_mt`. Objects are ordered by source basename,
then path. This is a reproducible working order, not proof that reconstructed
files recover original translation units. Ordinary import libraries supply
external DLL functions; the build never resolves game functions to retail
addresses and never forces unresolved references into an executable.

## Current gaps

The 2026-10-21 build compiles 198 C++ files (178 canonical sources and twenty
reviewed candidates) and reaches LINK with 524 unresolved external symbols,
down from 855 before the shared-class, global-provider and rendering integration.
This is a measured link failure, not a source-completion percentage. The
current run's `link.log` is authoritative for remaining failures.

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
the current composition retains all prior physics targets, with 1,080 now strict-exact, and all
2,973 distinct strict-exact calibration cases. A native link/runtime regression checks
virtual dispatch, child callbacks and iterator mutation guards against the
real BaseObject, GameObject and MemTag implementations.

[Graphics storage](GRAPHICS_STATE.md) now supplies the display lists, camera
limits, gamma ramp, shared texture caches and detail presets. Nine missing
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
with no new missing names. QueryGround remains disconnected: its current
coordinate conversion truncates where retail follows the active x87 rounding
mode. The game still does not link, so these checks do not establish
interactive menus or racing.

Validation of this composition passes the toolchain/manual/generated VC6 gate,
2,975 of 2,986 calibration rows (2,973 distinct source/address/extent cases),
and all prior strict physics targets, with four additions making 1,080 among
1,156 comparisons. Both near-miss comparisons report zero drops.
The recursive reset's strict case moves from FileStream.cpp to
Parameterblocks.cpp with its address and full 30-byte extent unchanged;
ShowDialog adds one strict case. The native-enabled Python suite passes
446 tests with 11 skipped. These counts retain existing nonexact comparisons;
they do not describe a fully matched or working game.

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
