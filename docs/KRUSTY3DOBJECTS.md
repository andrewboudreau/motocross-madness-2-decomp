# Krusty3DObjects.cpp (RunwayLights, VisualCue, NumberObjectManager, BonusObjectManager)

`src/reconstructed/Krusty3DObjects.h` / `Krusty3DObjects.cpp` /
`Krusty3DObjects.bindings.json` reconstruct part of
`D:\aardvark\VC\krusty2\Krusty3DObjects.cpp`: 36 of the unit's 39
functions are strict exact under `vc6_o2_mt` (VC6 SP3 `/O2 /GR /GX /MT`),
with every relocation resolved by the bindings. Two near misses are parked
in `samples/race/Krusty3DObjectsNearMisses.cpp`.

## Evidence

- Confirmed: the `__FILE__` literal `D:\aardvark\VC\krusty2\Krusty3DObjects.cpp`
  is at `0x0056c98c`. Its code references are `0x0048a688` (line 0x40),
  `0x0048be18`/`0x0048beb2` (lines 0x23d, 0x240), `0x0048c39d`..`0x0048c7b2`
  (lines 0x2d3..0x312), `0x0048c8bf` (line 0x326) and EH funclets at
  `0x0054bb33`..`0x0054bc56`.
- Confirmed RTTI, all single inheritance at offset 0:

  | Class | Bases | Vtable | COL | Own slots |
  |---|---|---|---|---|
  | RunwayLights | GameObject | `0x0055429c` | `0x0055cee0` | 0 `0x0048bce0`, 10 `0x0048a760` |
  | VisualCue | ArcadeObject, GameObject | `0x00554320` | `0x0055cf38` | 0 `0x0048adc0`, 10 `0x0048b100`, 14 `0x0048bc10`, 23 `0x0048afa0` |
  | NumberObjectManager | GameObject | `0x00554390` | `0x0055cf88` | 0 `0x0048bce0`, 10 `0x0048bfc0`, 14 `0x0048c290` |
  | BonusObjectManager | GameObject | `0x00554400` | `0x0055cfd8` | 0 `0x0048c300`, 10 `0x0048ccc0`, 14 `0x0048d620` |

  The constructors write the vptrs (`0x0048a5bf`, `0x0048ad7b`, `0x0048bcb1`,
  `0x0048c2bf`); only BonusObjectManager's destructor (`0x0048c320`) does,
  so only it declares one. `0x0048bce0` is the scalar deleting destructor
  RunwayLights, NumberObjectManager, FogOn and FogOff share (identical
  code folding); it calls `0x0048a5f0`, the folded compiler-generated
  destructor (`jmp 0x00468d60`). It is bound as RunwayLights'.
- Strong inference (extent `0x0048a5b0..0x0048d77b`): the unit starts after
  MouseDevice (`0x0048a550`, docs/INPUT_DEVICES.md) with the RunwayLights
  constructor and ends with its four per-file vector initializers
  `0x0048d640..0x0048d77b` (`.CRT$XCU` `0x00566260..0x0056626c`, right after
  the InputDevice set). Only this unit reads those vectors: the zero vector
  `0x0067c308` from `0x0048a986`, `0x0048b837`, `0x0048c97e`, `0x0048d48e`
  and the y axis `0x0067c328` from `0x0048b3d7`..`0x0048cf9b`. KrustyBike.cpp
  starts at `0x0048d780` (its table `0x0056cca0`).
- `.data`: the bonus animation keys at `0x0056c820` (seven 0x30-byte keys:
  frame, time, scale, position, axis, angle) directly precede RunwayLights'
  type descriptor.
- Callers: NationalRace.cpp builds RunwayLights (`0x004aa8cd`, 0x4c bytes,
  `0x0048a600`, `0x0048ad50`); QuarryStuntEvent.cpp builds VisualCue
  (`0x004e06cd`, 0xfc bytes) and calls `0x0048af20`, `0x0048bc30`,
  `0x0048bc80`; `0x00418dda`/`0x004183c9` build the two managers.

## Layout

- RunwayLights (0x4c): `+0x2c` blink timer, `+0x30` racer (set by
  `0x0048ad50`), `+0x34` terrain, `+0x38` five "pointer.slt" ArcadeObjects.
- VisualCue (0xfc): ArcadeObject to `+0xa8`; `+0xa8` view, `+0xac` terrain,
  `+0xb0..+0xb8` field of view and screen offsets, `+0xbc` toggle,
  `+0xc0`/`+0xc4`/`+0xc8` racer indices, `+0xcc` eleven racers, `+0xf8` count.
- NumberObjectManager: `+0x2c` racer, `+0x30` visible, `+0x34` camera,
  `+0x38` the five digit models ("one.slt" .. "five.slt").
- BonusObjectManager: `+0x2c` racer, `+0x30` key, `+0x34` time, `+0x38`
  camera, `+0x3c` "Base Bonus Frame" node, `+0x40` digit models `[10][5]`
  (places 0 and 1 only for digit 0), `+0x108` shown digits, `+0x11c` "x",
  `+0x120` decimal point, `+0x124` fraction digits `[10][2]`, `+0x174` shown
  fraction digits, `+0x17c..+0x1d0` the current key and its deltas,
  `+0x1dc..+0x1f0` the size factor state.

## Source shapes retail needs

- RunwayLights `0x0048a600`: `int i = 0;` before the zero position (the
  counter is stored first).
- NumberObjectManager `0x0048bd00`: the local name table is declared after
  slot 8 and the racer store, in the order "one.slt" .. "five.slt".
- NumberObjectManager slot 10: the camera test wraps the body
  (`if (owner == field_0x34) { ... }`); the two branches repeat the
  placement, and VC6 merges only their last call.
- BonusObjectManager `0x0048c330`: the "x" and decimal point positions are
  assigned before their `sprintf`.
- BonusObjectManager `0x0048cad0`: each next-key vector is built before the
  current one is stored.
- RunwayLights slot 10 normalizes with `z*z + (x*x + y*y)` and a free
  `operator*=(Vector3&, float)`.
- ArcadeObject.h gained the camera fields of UnknownArcadeView, and the
  parameter types below. D3DIMSoulTree.h gained SoultreeObject `+0x14c`,
  `+0x170..+0x180`, and `0x004fc690`, `0x004fd090`, `0x004fd910` and
  `0x004fd990`.

## Not matched

| VA | Size | Status |
|---|---:|---|
| `0x0048ccc0` | 1308 | near miss, 1299/1308: BonusObjectManager slot 10. Retail multiplies `fld [camera+0x198]; fmul [this+0x1e4]`; VC6 gives that order only when the camera value is read through a by-value accessor (otherwise it loads `+0x1e4` first, 1295/1308). Left: retail loads the camera pointer before the length store |
| `0x0048b100` | 2796 | VisualCue slot 10 (jump table `0x0048bbec` on TrackGame `+0x2d74`). Data flow decoded in the samples file; retail inlines the placement block four times without the out-of-line Vector3 constructor, VC6 here does not |
| `0x0048d1e0` | 864 | BonusObjectManager value display: rounds with the `fld; fistp [mem]` inline-assembly helper the project rules exclude |

## Remaining uncertainty

All member and method names are provisional. The racer, view, camera and
game-global structs in the header are this unit's views of objects whose
classes live elsewhere (RaceView.h, TrackGame.h, Camera.h); QuarryEvent.h
keeps its own partial VisualCue view.

## Reproduce

```bash
python tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" --profile vc6_o2_mt \
  src/reconstructed/Krusty3DObjects.cpp -o work/k3d.obj
python tools/match.py --exe "$MCM2_EXE" --target-va 0x0048c330 --target-size 1385 \
  --obj work/k3d.obj --symbol '?UnknownFunction48c330@BonusObjectManager@@QAEHPAXHH@Z' \
  --bindings src/reconstructed/Krusty3DObjects.bindings.json --json
```

ArcadeObject's `0x00401310` takes its two screen fractions (`c`, `d`) as
floats. Those are stored at +0x48/+0x4c, which are now float. `0x00401520`
and D3DIMSoultreeObject's `0x004fbd70` take two axes by pointer, then two
ints. Each function has a single declaration; ArcadeObject.cpp stays
exact, and its binding keys and calibration symbols use the new names.
