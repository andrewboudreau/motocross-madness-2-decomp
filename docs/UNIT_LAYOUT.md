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
  Without `static` the whole unit is byte-identical.
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

A survey of all 386 compiled `src/` objects ran with the anchors set to
`auto` (bring-up files excluded). 90 objects lay out their `.text` strictly.
25 of them have EH, and their `.text$x` and `.xdata$x` are strict too. Among
them are `Parser`, `TextureMap`, `Tgafile`, `ZoneReport`, `PCControl` and
`ManagedTextureGroup`. A strict object proves its own span. It does not prove
that the object covers its whole retail unit: compare the span with the unit's
extent in [REBUILD_GAPS.md](REBUILD_GAPS.md#translation-units-and-link-order)
first. `ArcadeObject` is an example of an object that is strict but does not
cover its whole unit.

## Blockers

116 objects differ. Each was classified by the first problem in its first
differing section:

- **Unregistered functions (52):** the object emits a function that has no
  retail address, so nothing anchors its bytes.
- **Order (36):** a function lays out at a VA other than its bound one. Most
  of these `src/` files do not follow retail unit boundaries. Some hold
  functions from several retail units, or only part of one, for example
  `FastMath.cpp` (`InitFastMath` is bound before the object's anchor),
  `DebugOverlay.cpp`, `CollisionObject.cpp`, `Bike.cpp` and `GameUi.cpp`.
  To fix these, merge or split the files into their owning units in source
  order. The other order failures come from `static` deferral and from inline
  helpers that VC6 places differently.
- **Bytes (28):** the function sits at the right place but is a near miss.
  This is ordinary matching work (see [REBUILD_GAPS.md](REBUILD_GAPS.md)).

Two blockers cut across these groups:

- **Pick-any helpers:** inline helpers have to be defined in headers that the
  right earlier units include, so that the first object that keeps each helper
  is the retail one.
- **ICF survivors:** we cannot yet predict where a folded function survives.
  For now, `--drop` the folded section with evidence, such as a vtable slot
  that points outside the unit.

The tool does not lay out `.data`, `.rdata` strings or `.bss`. Those need the
same treatment once the code layout is settled.
