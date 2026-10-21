# EcoSystem light lookup connection

`EcoSystem::LightingChanged` uses the existing source-defined
`LightManager::FindLightOfType` at `0x4a0190` (52 bytes). Its three calls request
light types 6, 4 and 2. The existing EcoSystem declaration now returns
`LightEmitter*`; explicit result casts retain the separate `UnknownEcoLight`
prefix view used by this caller. The canonical `LightEmitter.h` is unchanged.

The single decorated-name correction is
`?FindLightOfType@LightManager@@QAEPAVUnknownEcoLight@@H@Z` to
`?FindLightOfType@LightManager@@QAEPAVLightEmitter@@H@Z`. Both production and
aggregate bindings use the canonical name. There is no new provider, storage,
calibration target or candidate admission. This connects an existing complete
strict implementation; it does not assert that the two class declarations are
interchangeable complete types or recover their original translation units.

## Receiver and returned-pointer evidence

The Quarry loader allocates 0x11c bytes at `0x4deee0`/`0x4deee5`, passes the
allocation in ECX to the manager constructor at `0x4def05`, and stores the
returned pointer in the event's +0x7c field at `0x4def11`. At `0x4df963` and
`0x4df966` that field supplies the fourth argument to `EcoSystem::Create`.
The complete strict 859-byte Create body stores it at EcoSystem +0x4c.
These selected Quarry instructions establish the producer chain; the large
Quarry loader is not newly classified as a complete strict body.

The manager's raw primary table `0x555144` points to COL `0x55d6e0`, with
object offset zero and literal type name `.?AVLightManager@@`. Its complete
78-byte constructor writes that primary vptr and returns the same receiver.
The unchanged 300-byte LightingChanged body calls the lookup at `0x45a9ab`,
`0x45a9ea` and `0x45a9f8`, using that manager pointer.

The strict 1,040-byte `Scene::Load` body constructs a LightEmitter, stores its
returned primary pointer in the scene's array, reloads it and passes it to
`LightManager::AddLight` at `0x4efe72`. The emitter's primary table `0x5550cc`
points to COL `0x55d690`, offset zero and literal `.?AVLightEmitter@@`.
Its complete 227-byte constructor and 348-byte Create method are strict.
The 23-byte AddLight body stores the supplied pointer in manager +0x30,
indexed by count +0x2c, sets the light's owner at +0xa4, then increments the
count. The 52-byte lookup compares each stored light's type at +0x2c and
returns the stored pointer unchanged in EAX, or zero; it ends in RET 4.

LightingChanged reads only the returned ambient fields +0x54/+0x58/+0x5c
and direction fields +0x70/+0x78. Separate compilations of the genuine headers
check this prefix against LightEmitter. They also check EcoSystem's manager
field at +0x4c. This is bounded pointer and field evidence, not a claim of full
identity for `UnknownEcoLight` or the existing provisional manager view.

## Preservation

The retained regression checks 75 complete strict comparisons, all three
lookup calls and 13 complete canonical/aggregate objects. The published-base
same-path comparison preserves 1,196 nondebug sections and 6,093 relocations
under only the one method-name mapping. Payload bytes, section flags, addends
and relocation destinations are preserved, including partial bodies; the
private comparison excludes no filenames. Portable retained object pins
normalize only their explicitly enumerated source filenames and the exact
`src/reconstructed/ContainerList.h` filename. The latter repairs a private-to-
checkout path difference in the SceneManager pin: all thirteen old private
pins were reproduced first, then all complete object rows agreed across the
two roots under that one additional filename spelling. No code, arbitrary
string, relocation target or region is discarded.

The objects include all three direct EcoSystem.h source consumers:
EcoSystem.cpp, EcoSystemNearMisses.cpp and VegetationCollisionObject.cpp, plus
ten related light, scene, rendering and Quarry sources. A common declaration
header extraction was tested and rejected because it changed the complete
strict Scene::ReadLight body. This change keeps the existing declaration
boundaries and corrects only the evidenced method return type.

The four older whole-object fixtures for FilePrintf, clipper globals, Eco
QuadTree and vegetation each had their published canonical EcoSystem pin
independently reproduced before applying this one name mapping. Only that
132-section/720-relocation record and its explanatory note change. The sample
records and other objects retain their previous pins. No partial score or
function extent is relaxed.

## Native validation

The fixture executes the actual source lookup and AddLight bodies extracted
unchanged from LightEmitter.cpp. Both extracted bodies are checked against
their complete canonical COFF bodies and retail extents, with no imported
helpers or relocations. The reference executes the hash-pinned retail bodies
without patches. The fixture compiles against the actual LightEmitter header;
its separate Eco layout probe avoids combining the existing distinct class
views in one translation unit.

There are 4,480 scenarios: 1,024 lookup cases, 3,200 append cases and 256
boundary cases. Lookup covers signed extreme/negative/zero counts, first,
middle, last and absent matches. Appends cover indices 0 through 49 and four
pointer arrangements, including owner writes aliasing the count or the append
slot. An independent byte-level oracle follows the ordered writes and reloads
the count after the owner write. Reference and source are each compared with
that oracle, including the complete 16 KiB observable prepared memory.

The boundary cases include null receivers, inaccessible type/slot addresses,
zero counts that skip invalid slots, first matches that skip later invalid
pointers, first-write faults, owner-write faults after a successful append,
and final count-write faults after both earlier writes. All 192 access
violations have independently modeled access kind, address and partial stores.
The fault accesses are aligned DWORD operations; split-access fault behavior
is not claimed. Non-fault cases must complete and match the independent result.

The objects are prepared raw storage, not constructed game lifetimes. These
finite method-level cases do not execute complete LightingChanged, scene
loading, shading or a game session. Floating-point prefix fields are opaque
storage here; the tested methods perform pointer/integer operations. No
platform-portable meaning is assigned to invalid pointer arithmetic.

Reproduce with authentic VC6 SP3 and the verified retail image:

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH='.;tests'
python -m unittest test_eco_light_connections -v
```

## Source-build verification

The actual-checkout focused tests pass after reproducing all old portable pins
and explicitly normalizing the observed ContainerList.h filename. The same-path
13-object proof preserves all 1,196 sections and 6,093 relocations under only
the lookup name; it excludes no filenames.

Real build `build-jwoie6g6` compiles all 266 translation units and removes
exactly the obsolete lookup import: **209 -> 208**, with no new unresolved
names. All 266 prior production objects retain 10,715 nondebug sections and
45,819 relocations under that one name, without filename exclusions. Source,
header, configuration, compiler-profile and per-source flag hashes are checked.
Every old calibration and physics row retains its full extent and resolved
matching-position count.

The game still does not link. Startup, integrated menus and racing remain
unverified; the method fixtures do not establish those runtime milestones.
