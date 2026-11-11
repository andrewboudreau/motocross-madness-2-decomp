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
python tools/unit_layout.py --source src/reconstructed/Arrow.cpp --start auto \
    --text-x-start auto --xdata-start auto \
    --drop '?SetRenderContext@ArrowManager@@UAEPAVGameObject@@PAX@Z'
python -m unittest discover -s tests -p test_unit_layout.py
```

`--obj` reuses an object, for example from `build_game.py --compile-only`.
Addresses come from the file's `*.bindings.json`, the calibration cases and
`targets.json`. Names that every bindings file agrees on, such as
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
  `0x00462e30`, which lies outside `Arrow.cpp`. Without that section,
  `Arrow.cpp` is byte-identical.

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
| `Arrow.cpp` | `0x00401950..0x00401a30` (ICF section dropped) | from `0x00549650` | from `0x0055fd68` |
| `SoultreeMaterial.cpp` | `0x004fefe0..0x00500220` | `0x0054e280..0x0054e2f8` | from `0x00563930` |
| `cursor.cpp` (`GameCursor.cpp`) | `0x0043ea00..0x0043f160` (the whole doc extent) | from `0x00549ff0` | from `0x00560478` |

`tests/test_unit_layout.py` checks `AgeManager` and `ArcadeObject` when
`VC6_ROOT` and `MCM2_EXE` are set.

The sweep below finds 101 identical units. They include these units except
`Arrow`, which needs its `--drop`. A strict object proves its own span. It
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

There are 225 units (`src/bringup` excluded). 43 are not laid out: 35 have no
registered function, and 8 `src/krusty2` units have registered functions
that are not strict.

| Verdict | Units |
|---|---|
| identical (every group strict, every bound function at its VA) | 101 |
| identical, with functions that have no retail address | 2 (`BikeAI`, `BikeCamera`) |
| differs | 79 |

Eight units became identical through source order alone, without changing
any function bytes. `BackgroundImage`, `CacheTexture`, `ControlInterface`,
`ExceptionHandler`, `PCRenderTarget`, `TrackGame` and `VideoCard` now define
their functions in retail address order. `KeyboardHook`'s hook procedure is
extern. `CollisionPoint` is in retail order too, initializers included. Only
its deleting destructor is still misplaced (see below).

The 79 units that differ carry these finding kinds. A unit can carry several,
and folds or pick-any COMDATs kept elsewhere are not counted:

| Kind | Units | Only kind |
|---|---|---|
| `unregistered` (a function without a retail address) | 65 | 26 |
| `gap_missing` (retail has code our object lacks) | 31 | 5 |
| `order` | 27 | 1 |
| `gap_extra` (more bytes than retail between two bound functions) | 23 | 0 |
| `bytes` (at its VA, bytes differ) | 21 | 0 |
| `static_deferred` | 2 | 0 |
| `icf_duplicate` | 2 | 0 |

Most `order` findings are in units that hold functions from several retail
units, or only part of one, such as `FastMath`, `DebugOverlay`, `Bike` and
`GameUi`. Fixing them takes moves between files. Those moves have not been
made.

Units whose retail range holds functions defined in other `src/` files:

- `ExceptionHandler.cpp`: the retail unit opens with `RecordExceptionInfo`
  (`0x0045ff80`), which is in `src/reconstructed/Main.cpp`. The object
  starts at `hprintf`, so the sweep's anchor does not see this.
- `BikeRace.cpp`: the gap `0x004210f0..0x00421d50` holds vector helpers
  registered in `VectorCalls.cpp` and `VectorValueArithmetic.cpp`.
- `MSZoneInterface.cpp`: the gap `0x004aa150..0x004aa350` holds seven
  `SoultreePhysicsBaseObject` methods registered in
  `SoultreePhysicsMethods.cpp`. Whether they belong to this unit or the
  anchors are misbound is open.

`gap_missing` findings name owners in `samples/` too. Two examples are
`FileStream`'s `CheckHeader` in `samples/io/FileStream.cpp` and the vector
helpers of `BikeRace` in `samples/physics/helpers/VectorOps.cpp`.

Unexplained:

- Deleting-destructor placement. Retail `FontTexture.cpp` has `??_GFontTexture`
  (`0x00467160`) before the constructor. Our object emits it after the
  constructor. Retail `Quantize.cpp` has `??_GColorMapper` (`0x004ddea0`)
  right after `LoadShared`, before `Build16BitTables`. Ours comes later.
  Retail `CollisionPoint.cpp` has `??_GCollisionPoint` (`0x0043b300`) last.
  Ours follows `AddCollisionPoint`, which inlines the constructor that stores
  the vtable. Which construct makes VC6 emit `??_G` where retail has it is
  not known.

Two blockers cut across these groups:

- **Pick-any helpers:** inline helpers have to be defined in headers that the
  right earlier units include, so that the first object that keeps each helper
  is the retail one.
- **ICF survivors:** we cannot yet predict where a folded function survives.
  For now, `--drop` the folded section with evidence, such as a vtable slot
  that points outside the unit.

The tool does not lay out `.data`, `.rdata` strings or `.bss`. Those need the
same treatment once the code layout is settled.
