# Unit layout

Can one `src/` file, compiled like `tools/build_game.py` (profile `vc6_o2_mt`),
be laid out the way LINK placed its retail object, with every byte of the
range equal? `tools/unit_layout.py` answers that for one object. It places the
object's sections in LINK's order, resolves relocations against retail
addresses, and compares the result with the retail range, fill bytes included.

```bash
export VC6_ROOT=... MCM2_EXE=work/game/mcm2.exe PYTHONPATH=.
python tools/unit_layout.py --source src/reconstructed/ArcadeObject.cpp \
    --start auto --text-x-start auto --xdata-start auto
python -m unittest discover -s tests -p test_unit_layout.py
```

`--obj` reuses an object, for example from `build_game.py --compile-only`.
Addresses come from the file's `*.bindings.json`, the calibration cases and
`targets.json`. A case or target symbol matches like the matchers do: the
exact name, else a unique substring (`'??_GBaseObject'`). A
`<Stem>.targets.json` entry without `source` registers `<Stem>.cpp` next to
it, the pairing `run_samples.py` compiles. Names that every bindings file agrees on, such as
`___CxxFrameHandler`, fill in the rest. The `auto` anchors work like this:
`.text` starts at the first bound function that is not pick-any. `.text$x`
starts from a `<fn>$ehhandler` binding. `.xdata$x` starts from the `$T`
address that retail's handler stub loads. Every function gets a verdict: its
laid-out VA either agrees with its bound VA or it does not.

## Rules

Confirmed by strict byte comparison of the units below:

- `/O2` puts every function in its own `.text` COMDAT section (`/Gy`), aligned
  to 16. The compiler pads each section with `0x90` inside the section. LINK
  fills the gaps between sections with `0xCC` (checked in `.text$x`).
- Within one object the sections follow the object's section-table order.
  For extern functions and methods this is source order.
- VC6 emits a `static` function after the first function that references it.
  Retail `AgeManager.cpp` has its qsort comparator at `0x00401180`, before
  `EvictStale` (`0x004011b0`), so in retail it was not `static`, or it was
  emitted differently. With `static` our object puts it after `EvictStale`.
  Without `static` the whole unit is byte-identical. Moving the `static`
  definition does not help. `KeyboardHook.cpp` with the hook procedure and
  both static helpers defined first still emits `SetTaskSwitchAllowed`,
  `InstallTaskSwitchHook`, hook procedure, `RemoveTaskSwitchHook`. Retail has
  the hook procedure at `0x005053d0`, before `InstallTaskSwitchHook`
  (`0x00505420`). It lays out exactly only as an extern function defined in
  that place.
- A dynamic initializer `_$E<n>` is emitted where its object is defined.
  `CollisionPoint.cpp`'s four vector constants (`0x0043b050..0x0043b18b`) sit
  between `UpdateCollisionPointWorldPositions` and `CollisionRejectFrom`.
- Bodies that `/OPT:ICF` folds within one unit are bound to the same VA. The
  tool keeps the first section and reports the later ones as `folded` (for
  example `CharacterDefaults`, `GameplayDefaults`, `UIControlDefaults`).
- Pick-any COMDATs are deleting destructors (`??_G`) and inline methods. Such a
  COMDAT keeps its section-table position in the first object that keeps it.
  `ArcadeObject`'s `??_G` sits right after its constructor in both builds.
  `SoultreeMaterial.cpp` also defines `GetTextMode`/`SetTextMode`, but retail
  keeps them at `0x0043e9b0`/`0x0043e9e0`, so an earlier object kept them. The
  tool reports such sections as `elsewhere` and skips them.
- `.text$x` holds the unwind funclets and the `$ehhandler` stub
  (`mov eax, $T; jmp ___CxxFrameHandler`). It sorts after all of `.text`,
  starting at `0x005495d0`. It follows object order: `ArcadeObject`'s range
  `0x005495d0..0x0054964e` is followed directly by `Arrow`'s at `0x00549650`.
  Each object's `.text$x` sections follow the order of their parent functions.
- `.xdata$x` (the FuncInfo tables, aligned to 8) is merged into `.rdata`. It
  is also contiguous in object order: `ArcadeObject` at `0x0055fd08`, then
  `Arrow` at `0x0055fd68`.
- `/OPT:ICF` moves folded functions out of their unit. For example,
  `ArrowManager::SetRenderContext` (vtable `0x00550490`, slot 8) resolves to
  `0x00462e30`, which lies outside `Arrow.cpp`. `Arrow.bindings.json` binds it
  there, the tool skips the section as `elsewhere`, and `Arrow.cpp` is
  byte-identical.
- `??_H@YGXPAXIHP6EX0@Z@Z` (the vector constructor iterator) is a pick-any
  COMDAT in 31 of the laid-out objects. A static array of a type with a trivial
  constructor leaves it behind (for example `AuralScape`'s
  `s_UnknownListenerPositions5776f8[2]`, whose `_$E13` is a bare `ret` in
  both builds). Nothing outside `.debug` references it, and its bytes occur
  nowhere in retail `.text`. Unbound pick-any sections like that are skipped
  as `removed`. That `/OPT:REF` removed them is strong inference: no retail
  copy exists, and LINK drops unreferenced COMDATs.

Inference, not yet proven:

- Retail `ArcadeObject.cpp` ends with `8b 41 04 c3` at `0x00401940`
  (`BaseObject::GetRefCount`, slot 3). Our object does not produce it. If
  `GetRefCount` is made inline in `BaseObject.h`, it is emitted right after the
  constructor, not at the end of the unit. It may be an ICF survivor, a
  separate object, or another emission rule. This is open.
- ICF survivors do not simply come first in link order. `0x00462e30` lies
  between the units around `EventManager` and `FollowCam`.

## Units that lay out exactly

Each of these was laid out strictly, with all relocations resolved:

| Unit | `.text` | `.text$x` | `.xdata$x` |
|---|---|---|---|
| `AgeManager.cpp` | `0x00401000..0x00401260` | none | none |
| `ArcadeObject.cpp` | `0x00401260..0x00401940` (plus the open `0x00401940`) | `0x005495d0..0x0054964e` | `0x0055fd08..0x0055fd68` |
| `Arrow.cpp` | `0x00401950..0x00401a30` (ICF section bound elsewhere) | from `0x00549650` | from `0x0055fd68` |
| `SoultreeMaterial.cpp` | `0x004fefe0..0x00500220` | `0x0054e280..0x0054e2f8` | from `0x00563930` |
| `cursor.cpp` (`GameCursor.cpp`) | `0x0043ea00..0x0043f160` (the whole doc extent) | from `0x00549ff0` | from `0x00560478` |

`tests/test_unit_layout.py` checks `AgeManager` and `ArcadeObject` when
`VC6_ROOT` and `MCM2_EXE` are set.

The sweep below finds 130 identical units, these five among them. A strict object proves its own span. It
does not prove that the object covers its whole retail unit: compare the
span with the unit's
extent in [REBUILD_GAPS.md](REBUILD_GAPS.md#translation-units-and-link-order)
first. `ArcadeObject` is an example of an object that is strict but does not
cover its whole unit.

## Sweep of the strict units

`tools/unit_layout_sweep.py` lays out every `src/` unit whose registered
functions (calibration cases and `targets.json` entries) are all strict exact.
It writes `report.json` and `report.md` to the ignored
`work/unit-layout-sweep/`. Each laid-out unit gets a verdict. Every `.text`
section that keeps a unit from being identical gets a finding with its cause.
The module docstring lists the finding kinds.

```bash
export VC6_ROOT=... MCM2_EXE=work/game/mcm2.exe PYTHONPATH=.
python tools/unit_layout_sweep.py --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
python tools/unit_layout_sweep.py --source src/reconstructed/KeyboardHook.cpp
python -m unittest discover -s tests -p "test_unit_layout*.py"
```

There are 226 units (`src/bringup` excluded). 42 are not laid out: 35 have no
registered function, and 7 `src/krusty2` units have registered functions
that are not strict.

| Verdict | Units |
|---|---|
| identical (every group strict, every bound function at its VA) | 130 |
| identical, with functions that have no retail address | 0 |
| differs | 54 |

OrientationAxes.cpp, promoted from samples/, is identical on its own. 28 units moved to identical through registration and the `/OPT:REF` rule,
without source changes. Every newly bound function compares strict exact
(relocations resolved) at its VA. The evidence for each VA is a vtable slot,
or a call from a function that is already bound:

- Registration fixes. Calibration symbols now match by unique substring, and
  `<Stem>.targets.json` pairs with `<Stem>.cpp`. This moved `BaseObject` and
  `PCCamera`, and bound `BikeAI`'s last function.
- Shared dialog bodies. The deleting destructor of every dialog class in
  `KrustyUI`, `ProCircuitProcs` and `SelectGamePicProcs` is vtable slot 0, at
  `0x0049bdb0`. Its `call` at `0x0049bdb3` binds each `??1<Dlg>` to
  `0x00450fc0` (`jmp 0x0046a070`, `??1UIDialog`). One retail copy serves all of
  them.
- Single folds and pick-any copies, each bound from a vtable slot:
  - `ShadowReceiver` slots 14 and 27-30: `0x00468c90`, `0x004aa190` and
    `0x0044d710`.
  - `ProjectedShadow::OnControlPress`: `0x004dc4c0`.
  - `QuadTreeObject::ComputeQuadTreeCode` (`soultree.cpp`): `0x00434ce0`.
  - `SoultreeQuadTreeRenderer::SetRenderContext`: `0x004452e0`.
  - `ArrowManager::SetRenderContext`: `0x00462e30`.
  - `VehicleCamera::IsSubjectCrashed`: `0x00417490`. This is
    `BikeCamera.cpp`'s copy; `KrustyBikeCamera` binds it as kept elsewhere.
  - `PCCentralBikeRiderDlg` and `MPBikeRiderDlg` control handlers.
  - `InfoType`, at `0x0044d710` and `0x0044d720`.
  - `MPOptionsDlg::SyncValues`: `0x00464e80`.
  - `??_GD3DIMSoultreeCharacter`: `0x00446680`, the target of the vtordisp
    thunk at `0x00446670`.
- `/OPT:REF`. Skipping the unreferenced `??_H` is part of the move for 16
  units: `ObjectPlacement`, `GraphicsTest`, `PhysicsRigidBody`,
  `D3DIMSoultreeShadow`, `AuralScape`, `CarProcedural`, `DirectoryList`,
  `EventManager`, `FollowRacer`, `GameUiHelpers`, `KrustyUI`, `KrustyVCR`,
  `ObjectPicker`, `RaceStatus`, `MorphBastardModifier` and `TrackRecord`.
  `D3DIMSoultreeShadow` and `KrustyUI` also needed the bindings above.

For several of these units, the identical span is only part of the retail
unit. Most of their functions are bound outside it (`icf_elsewhere`):

- `AuralScape` anchors on `_$E2` at `0x00403c10` and keeps 57 functions
  elsewhere.
- `D3DIMSoultreeMotnctrl` anchors on `_$E2` at `0x004466e0`; its constructor
  is elsewhere.
- `KrustyUI`'s shared `??_G` (`0x0049bdb0`) lies before its anchor
  (`0x0049bdd0`).

Compare the span with the unit extent before counting such a unit as
covered.

Three fragment files have no retail counterpart for their unbound
functions. They are `SoultreePositionValue.cpp` (inside retail `soultree.cpp`),
`VehicleDefaults.cpp` (in the FollowCam range) and `VehicleWheelAttachment.cpp`
(in the `Track.cpp` range). Each one includes `Math3D.h`, so it emits its own
eight file-static vector initializers (`_$E1`..`_$E11`). Retail has other
functions at the laid-out positions. Those initializers belong to the full
retail objects that the fragments were split from. The fragments stay
`differs`.

Eight units became identical through source order alone, without changing
any function bytes. `BackgroundImage`, `CacheTexture`, `ControlInterface`,
`ExceptionHandler`, `PCRenderTarget`, `TrackGame` and `VideoCard` now define
their functions in retail address order. `KeyboardHook`'s hook procedure is
extern. `CollisionPoint` is in retail order too, initializers included. Only
its deleting destructor is still misplaced (see below).

The 54 units that differ carry these finding kinds. A unit can carry several.
Folds, pick-any COMDATs kept elsewhere and `/OPT:REF` removals are not
counted:

| Kind | Units | Only kind |
|---|---|---|
| `unregistered` (a function without a retail address) | 32 | 2 |
| `gap_missing` (retail has code our object lacks) | 30 | 6 |
| `order` | 30 | 4 |
| `bytes` (at its VA, bytes differ) | 20 | 0 |
| `gap_extra` (more bytes than retail between two bound functions) | 17 | 0 |
| `static_deferred` | 2 | 0 |
| `icf_duplicate` | 2 | 0 |

The two units with only `unregistered` are the fragments
`SoultreePositionValue` and `VehicleWheelAttachment`. Four units have only
`order`:

- `CollisionPoint`: its `??_G`, see below.
- `BlockAllocator`: retail has `??1BlockAllocator` at `0x00423fb0`, before
  `Alloc`.
- `Particles`: retail has `ParticleManager::Create` at `0x004ba4d0`, before
  `AddParticle`.
- `CubeDraw`.

Most `order` findings are in units that hold functions from several retail
units, or only part of one, such as `FastMath`, `DebugOverlay`, `Bike` and
`GameUi`. Fixing them takes moves between files. Those moves have not been
made.

Units whose retail range holds functions defined in other `src/` files.
None of them was moved; the reasons are below.

- `ExceptionHandler.cpp`: the retail unit opens with `RecordExceptionInfo`
  (`0x0045ff80`). `Main.cpp` only calls it. Its reconstruction is
  `samples/game/RecordExceptionInfo.cpp`, a reviewed near miss: retail reads
  `FS:[4]` at `0x004602ef` with inline assembly, which this project does not
  use (see [EXCEPTIONHANDLER.md](EXCEPTIONHANDLER.md)). The object starts at
  `hprintf` and is identical from there.
- `BikeRace.cpp`: the gap `0x004210f0..0x00421d50` holds
  `WriteRelativePositions` (`0x004210f0`, a near miss in
  `samples/race/BikeRaceNearMisses.cpp`) and the vector helpers at
  `0x00421cb0..0x00421d50`. The helpers are registered in `VectorCalls.cpp`
  and `VectorValueArithmetic.cpp`; the latter is identical on its own. They have
  callers in earlier units and fold across several vector types. That fits
  header-inline operators that are kept out of line here and folded by ICF
  (strong inference). `BikeRace.cpp` already declares `static inline`
  `operator+`/`operator-`. Moving the helpers in as out-of-line definitions
  would not make the unit identical: `WriteRelativePositions` and the unit's
  `bytes` findings stay.
- `MSZoneInterface.cpp`: the gap `0x004aa150..0x004aa350` holds seven
  `SoultreePhysicsBaseObject` methods (vtable `0x00557d90`), registered in
  `SoultreePhysicsMethods.cpp`.
  - Confirmed: they were compiled in `MSZoneInterface.obj`. They lie between
    `_$E10` (ends `0x004aa14b`) and `SetDirectPlay` (`0x004aa350`). Slot 36
    (`ResetBodyOrientation`) reads `kVec3YAxis` at `0x00685190`, which is this
    unit's file-static `Math3D.h` constant.
  - Strong inference: they are inline members that this object kept as
    pick-any COMDATs, not `MSZoneInterface.cpp` definitions. The other five
    methods of the same vtable sit in the `Bike`, `FollowCam` and `Terrain`
    ranges, as described in
    [SOULTREE_PHYSICS_METHODS.md](SOULTREE_PHYSICS_METHODS.md).
  - Moving them into `MSZoneInterface.cpp` as out-of-line definitions would
    be a matching-only form, so it was not done. The unit stays `differs`
    until the inline headers are in place.

`gap_missing` findings name owners in `samples/` too, for example the vector
helpers of `BikeRace` in `samples/physics/helpers/VectorOps.cpp`.

Five units differ only through `gap_missing`. None can move to identical by
moving a strict body, because every gap holds a function that is not strict
exact (2026-11-12 sweep):

| Unit | Missing retail range | What is there |
|---|---|---|
| `BoundingBoxTreeBuild.cpp` | `0x0042c260..0x0042c8c0` | `GatherModelTriangles`, near miss (`samples/physics/bvh/BoundingBoxTreeBuildNearMisses.cpp`) |
| `D3DIMSoulTree.cpp` | `0x00440f30..0x00442e00`, `0x00442f40..0x00442f70` | `DrawCurrentLod`, near miss (`samples/render/D3DIMSoulTreeNearMisses.cpp`), and the atexit `ret` stubs of its three function-local statics. The stubs are strict exact in that sample, but they are emitted with `DrawCurrentLod` and cannot move without it |
| `DebugOverlay.cpp` | `0x00447a00..0x00447de0` | `LoadFont`, near miss (`samples/render/DebugOverlayNearMisses.cpp`) |
| `MSZoneInterface.cpp` | `0x004aa150..0x004aa350` | the seven `SoultreePhysicsBaseObject` methods, left for the reason above |
| `ResourceManager.cpp` | `0x004e9030..0x004e9360` | `AddArchive`, near miss (`samples/render/ResourceManagerNearMisses.cpp`) |

`FileStream.cpp` left this list on 2026-11-13: `CheckHeader` (`0x00460db0`) is
now strict exact in `src/reconstructed/FileStream.cpp`.

In the other 24 units with `gap_missing`, the only strict-exact owners in a
gap are the `BikeRace` vector helpers and `Vehicle` slots 78 and 79
(`src/reconstructed/VehicleMethods.cpp`, at `0x00492220`/`0x00492270` inside
`KrustyBike.cpp`). `KrustyBike.cpp` also has near-miss gaps, `order` and
`unregistered` findings, so moving the two slots would not make it identical.

Deleting-destructor placement. These points are confirmed with VC6 SP3 `/O2`
probes using the `config/game_build.json` profile:

- `??_G<Class>` is emitted directly after the first function in the object
  whose body stores `<Class>`'s vtable. This is the out-of-line constructor,
  the destructor if it is defined first, or any function that expands an
  inline constructor (`new C(v)` with an in-class constructor).
- In-class inline virtual functions are emitted between that function and
  `??_G`.
- The following do not move `??_G`:
  - a function before it that uses `delete`, `delete[]`, `new[]`, an explicit
    destructor call, `dynamic_cast` or `typeid`;
  - a local or global object;
  - an inline constructor or inline virtual defined after the first user;
  - an implicit constructor;
  - `/YX` or `/Zi`.

Minimal reproduction:

```cpp
class A { public: A(int v); virtual ~A(); int x; };
void First(int* p) { *p = 1; }
A::~A() { x = 0; }       // stores the vtable first
A::A(int v) : x(v) {}
// order: First, ??1A, ??_GA, ??0A   (swap the two definitions: ??0A, ??_GA, ??1A)
```

What this means for retail:

- `FontTexture.cpp` has `??_GFontTexture` (`0x00467160`) before the
  constructor. `Quantize.cpp` has `??_GColorMapper` (`0x004ddea0`) right after
  `LoadShared`. By the rule, a function that stores the vtable came just
  before each of them, but no such function is in the image. One possibility
  is an unused function, such as a second constructor, that `/OPT:REF`
  dropped. This is provisional. Adding one would invent code, so it was not
  done.
- `CollisionPoint.cpp` has `??_GCollisionPoint` (`0x0043b300`) last. That
  contradicts the rule: retail `AddCollisionPoint` stores the vtable at
  `0x0043a368`, and our object puts `??_G` after it. This is open.

Two blockers cut across these groups:

- **Pick-any helpers:** inline helpers have to be defined in headers that the
  right earlier units include, so that the first object that keeps each helper
  is the retail one.
- **ICF survivors:** we cannot yet predict where a folded function survives.
  For now, bind the folded function to its survivor when there is evidence,
  such as a vtable slot that points outside the unit. The tool then skips it
  as `elsewhere`.

The tool does not lay out `.data`, `.rdata` strings or `.bss`. Those need the
same treatment once the code layout is settled.
