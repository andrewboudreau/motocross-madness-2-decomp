# QuarryStuntEvent.cpp

`src/reconstructed/QuarryStuntEvent.cpp`, with the class in `QuarryEvent.h`.
Names are provisional.

Evidence:
- **`__FILE__`:** the literal at `0x00572154`; its first xref is at
  `0x004de2cd`.
- **RTTI:** the `BaseQuarryEvent` vtable at `0x0055766c` points into the
  range for slots 0, 10, 12, 14, 23, 24, 27, 29, 30 and 34.
- **MemTag strings:** "entering/exiting BaseQuarryEvent::Create" confirm
  the class name.
- **`.CRT$XCU`:** its four kVec3 `$E` are entries `0x005663e4..0x005663f0`.

Extent: `0x004de2a0..0x004e1fa7`. Quantize.cpp ends at `0x004de29f` and
RaceSound's constructor starts at `0x004e1fb0`.

Exact: 25 calibration cases:
- The constructor, the destructor and its deleting wrapper.
- `Create` `0x004de3b0`.
- Slots 10, 14, 23, 24, 27, 29, 30 and 34. Slots 14 and 34 are bodies that
  Wrecker's and Scene's vtables also use.
- The three memory-estimate helpers, the message toggle `0x004e0c30` and
  the clock reset `0x004e1f00`.
- The eight `$E`.

Source forms needed:
- The kVec3 statics are defined between slot 12 and slot 24, where their
  `$E` code sits. Defining them at the top changes the constructor.
- The render target is read through direct casts; an inline accessor
  changes the destructor's registers.

Near misses (`samples/game/QuarryStuntEventNearMisses.cpp`): the memory
estimate `0x004e0560` (23 of 331 positions; retail's frame is one
`push ecx` slot where VC6 here allocates 0xc bytes, which shifts every
stack offset) and slot 12 `0x004e14a0` (166 of 1756; the prologue's
register choice shifts the rest). The scores are the same with HEAD's
headers.

The loader `0x004de590` (7952 bytes) is a near miss in
`samples/game/QuarryStuntEventLoaderNearMiss.cpp`, which carries its own
`UnknownQuarry*` views. About 95% of the normalised instructions align.
These match:
- the call sequence;
- the allocation sizes and `__FILE__` lines (252..1251);
- the EH states, the error paths and the 0x650 frame.

These differ:
- the stack slots of most locals: retail shares more. Its `+0xc` (23
  uses) holds `stream` (13 here at `+0x8`) together with the
  `shadowLight` / `textureBytes` / `all` class (16 here at `+0xc`); its
  `+0` (12) is `needed` / `iterator` (19 here at `+0x4`); its `+0x4` (12)
  joins `visibility` / `forced` (13 here, `+0x10`) with `step` (`+0x68`
  here); `flareLight` / `margin` (11, `+0`) is retail's `+0x14`, `i` /
  `iterator` (8, `+0x14`) its `+0x1c`; the 19-use temporary `+0x2c` is
  retail's `+0x30`, `+0x44` (10) its `+0x48`, `+0x60` (11) stays,
  `sceneSteps` moves from `+0x48` to `+0x64`, `skyTextures` / `hasCube`
  share retail's `+0x68`. VC6 shares a slot between locals whose scopes
  are disjoint (an address-taken local lives for its whole block), so the
  original declares these in inner blocks;
- register choices and expression shapes in the memory-budget tree
  (`0x004de7e0..0x004ded5e`);
- the zero register retail keeps around the collision links.

Facts the loader establishes:
- The three 0x18-byte `UnknownTextureFormatChoice` records hold the
  terrain, model and sky texture settings: +0x14 is the halving level, set
  from total/available memory, the `DriverInfo\<driver>\VideoMemoryMB`
  registry value and the "MarginPercentage"/"MinMarginKBytes" settings.
- The model record goes to Scene `0x004ea7e0`/`0x004efb20`, the race
  `0x00417ed0`, slot 27 and the podium; the terrain record to `0x005059d0`;
  the sky record to SkyCube `0x004fb230`.
- The visibility quadtree `0x0052d460` returns its GameObject base at
  +0x874.

QuarryEvent.h now types BaseQuarryEvent's fields (size still 0xa4).
TrackOverlay.h, SceneManager.h, RaceView.h and TrackGame.h gained members.
SceneManager.h names CollisionCharacter +0x210 and CarProcedural
+0x38/+0x3c for the loader's collision links.
