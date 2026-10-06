# SceneManager.cpp: the track scene loader

`src/reconstructed/SceneManager.cpp` reconstructs part of
`D:\aardvark\VC\krusty2\SceneManager.cpp`. 46 functions match strictly
under the default VC6 profile and are registered as calibration cases. The
only RTTI name is `Scene` (`Scene : GameObject : BaseObject`). All other
type, member and function names are provisional.

## Evidence

- `D:\aardvark\VC\krusty2\SceneManager.cpp` (`0x00572ccc`) is the
  `__FILE__` of allocations from `0x004e9a2a` to `0x004f0ee5`. Examples:
  - line 1392 in `0x004ebdb0` (the fog);
  - line 2136 in `0x004edf20` (the shadows);
  - line 3727 in `0x004f0ec0`.

  It is also the `__FILE__` of the unwind funclets at `0x0054db53`..`0x0054df86`.
- Extent:
  - The file starts with the keyword lookup `0x004e9980`, which only
    `0x004eb602` calls. `0x004e9700`..`0x004e9960` are left to
    ResourceManager.cpp.
  - It ends with the player-record initializers `0x004f1740`..`0x004f1793`.
    `0x004f17a0` is not reconstructed.
- Vtable:
  - `Scene`'s vtable is `0x0055786c`; the object is 0x8cc bytes, from
    `operator new(0x8cc)` at `0x004e9a30`/`0x004e9aea`.
  - Overridden slots: 0 (deleting destructor `0x004ea7c0`), 5, 7, 10, 14, 22
    and 23. Slot 10 (`0x004eab00`) is not reconstructed.
- `UnknownTrackGameObject574` (TrackGame+0x574) has its constructor and
  scene-file helpers here: `0x004e99d0`..`0x004ea390`.
- `0x004de580` (`ret 0xc`) and `0x00464e80` (`ret 4`) are shared empty
  bodies. The readers call them directly with what they read, so they are
  declared as provisional `Scene` members.
- The vector constants `0x00689c98`..`0x00689cc8` are built by the
  `_$E1`..`_$E11` pairs at `0x004ecb70`..`0x004ecd20`. The
  seven-record `PlayerInfoType` array at `0x00689d08` produces
  `_$E13`..`_$E16`.
- `0x004f0d20` keeps an unused 0x18-byte resource manager on its stack,
  with constructor `0x004e8e80` and destructor `0x004e8ea0`. It is declared
  as the view `UnknownSceneResourceManager` (see below).

## Matched functions

- Scene: constructor, deleting destructor, destructor, slots 5/7/14/22/23,
  and `0x004ea7e0`, `0x004eaec0`, `0x004eafd0`, `0x004eb000`.
- Readers:
  - `0x004eb160` (vector), `0x004eb300` (RGB), `0x004eb480` (flag list);
  - `0x004ebdb0` (Fog), `0x004eca20` (ResourceFiles), `0x004ecc10` (Stadium);
  - `0x004ef9c0` (sound settings), `0x004f0d20` (scene file), `0x004f0ec0`
    (SLT path).
- `0x004edf20` (shadow receivers), `0x004f00e0` (texture-width histogram, 549
  bytes including its jump and byte tables) and `0x004f0310`.
- UnknownTrackGameObject574: `0x004e99d0`, `0x004e9a10`, `0x004e9ac0`,
  `0x004e9b80`, `0x004e9ba0`, `0x004e9e30`, `0x004e9f70`, `0x004ea010` and
  `0x004ea390`.
- Static initializers: `_$E1`/`_$E2`, `_$E4`/`_$E5`, `_$E7`/`_$E8`,
  `_$E10`/`_$E11` and `_$E13`..`_$E16`.

## Shared changes

- `TrackGame.h`: `UnknownTrackGameObject574` gets its constructor layout
  and the methods above. `0x004ea010` takes `int*` outputs (TrackRecord
  bindings renamed).
- `UnknownResourceManager.h`:
  - `0x004e9030` returns the archive (KrustyUI and TrackGame bindings renamed);
  - `0x004e9830`, `0x004e9430` and `0x004e96b0` are declared.
- Why the constructor, destructor and fields are on a view: declaring them
  on the shared class flips one register choice (`add edx,eax` vs
  `add eax,edx`) in TrackGame `0x00520ab0`. That function's TU only uses the
  class through a pointer. Adding just the constructor, one ordinary method
  or one field does not flip it; adding the destructor or four named fields
  does. That suggests VC6 is sensitive to the TU's declarations rather than
  to layout.
- `TextureMap.h`: declares `0x00461cb0`.
- `mcm2tool/resolved_match.py`:
  - The `$ehhandler` binding also accepts prologues where VC6 schedules
    `mov r, [esp+disp8]` argument loads between `push -1` and the handler
    push (`0x004ea7e0`).
  - Covered by `tests/test_eh_frame_relocations.py`.

## Source shapes that mattered

- VC6 does not tail-merge identical success blocks when they use different
  registers. `0x004eb160` and `0x004eb300` repeat the parse block in each
  branch; a shared found-flag gives the right size but the wrong registers.
- `0x004eb480` puts the success branch first.
- `0x004ebdb0` reproduces a retail bug: the haziness range check tests the
  visibility again.
- A single-bit bitfield store from a condition gives retail's xor/and/xor
  (Stadium flag at +0x6a0).
- Dead parameter slots hold locals:
  - `looping` in `0x004ef9c0`;
  - the width in `0x004f00e0`;
  - the new shadow in `0x004edf20`.

## Not reconstructed

- `0x004eb040` and `0x004eff30` need the Soultree character classes.
  `0x004eff30` also needs the table pointer at `0x00689f18`.
- `0x004e9cd0` is claimed by a physics sample.
- Large functions:
  - `0x004eab00`, `0x004eb570`, `0x004ebfc0`, `0x004ecd60`, `0x004edfe0`;
  - `0x004ef4c0`, `0x004efb20`, `0x004f0390`, `0x004f1130`, `0x004f17a0`.

## Reproduce

```bash
make vc6-gate VC6_ROOT="$VC6_ROOT"
PYTHONPATH=. python tools/run_calibration.py --compiler vc6 \
    --vc6-root "$VC6_ROOT" --profile vc6_o2_mt
```
