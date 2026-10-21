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

The 2026-10-20 build compiles 174 C++ files (171 canonical sources and three
reviewed candidates) and reaches LINK with 668 unresolved external symbols,
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
the current composition retains all 1,076 strict-exact physics targets and all
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

Validation of this composition passes the toolchain/manual/generated VC6 gate,
2,975 of 2,986 calibration rows (2,973 distinct source/address/extent cases),
and all 1,076 previously strict physics targets among 1,152 comparisons.
The recursive reset's strict case moves from FileStream.cpp to
Parameterblocks.cpp with its address and full 30-byte extent unchanged;
ShowDialog adds one strict case. The native-enabled Python suite passes
396 tests with 11 skipped. These counts retain existing nonexact comparisons;
they do not describe a fully matched or working game.

The source still lacks `Game::Tick` at `0x004685d0`, called by
`TrackGame::Tick`; its retail timing uses `rdtsc` (see [Game](GAME.md)).
Startup still lacks `RecordExceptionInfo`. These are gameplay blockers in
addition to other incompatible class views, missing methods and unresolved
globals. Import libraries alone cannot supply them.

An eventual successful link is only the next gate. Startup/static
initialization, external assets, resources, current-Windows display behavior,
menus and actual racing must still be tested on the source-built executable.
The existing `runtime/build.py` route stages the retail demo executable and
does not prove any of those source-build requirements. Source-byte matching
remains a separate check through the existing VC6 calibration and physics
suites.
