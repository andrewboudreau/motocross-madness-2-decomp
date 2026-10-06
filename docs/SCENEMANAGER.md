# SceneManager.cpp: the track scene loader

`src/reconstructed/SceneManager.cpp` reconstructs part of
`D:\aardvark\VC\krusty2\SceneManager.cpp`. 51 functions match strictly
under the default VC6 profile. The only RTTI name is `Scene` (`Scene : GameObject : BaseObject`). All other
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
  - It ends at or before `0x004f173f`. The player-record initializers
    `0x004f1740`..`0x004f1793` open SelectGamePicProcs.cpp (see
    [INITIALIZERS](INITIALIZERS.md)).
- Vtable:
  - `Scene`'s vtable is `0x0055786c`; the object is 0x8cc bytes, from
    `operator new(0x8cc)` at `0x004e9a30`/`0x004e9aea`.
  - Overridden slots: 0 (deleting destructor `0x004ea7c0`), 5, 7, 10, 14, 22
    and 23. Slot 10 (`0x004eab00`) is not reconstructed.
- Function starts in the extent (call targets, data references and
  post-padding starts, minus jump-table targets): 54. Matched: 51; one
  near miss, two not reconstructed.
- `UnknownTrackGameObject574` (TrackGame+0x574) has its constructor and
  scene-file helpers here: `0x004e99d0`..`0x004ea390`.
- `0x004de580` (`ret 0xc`) and `0x00464e80` (`ret 4`) are shared empty
  bodies. The readers call them directly with what they read, so they are
  declared as provisional `Scene` members.
- The vector constants `0x00689c98`..`0x00689cc8` are built by the
  `_$E1`..`_$E11` pairs at `0x004ecb70`..`0x004ecd20`.
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
  `0x004e9b80`, `0x004e9ba0`, `0x004e9cd0`, `0x004e9e30`, `0x004e9f70`,
  `0x004ea010` and `0x004ea390`.
- The large readers and loaders:
  - `0x004e9cd0` (345 bytes): opens a file through an archive stream;
  - `0x004eb040` (280): restarts a scene entry's motion or object;
  - `0x004eb570` (2102): reads "Light<n>" (`UnknownSceneLight`, 0x8c bytes);
  - `0x004ebfc0` (2650): reads "Environment", the terrain and reverb zones
    (`UnknownSceneEnvironment`, 0x420 bytes, line 1456);
  - `0x004ef4c0` (1274): reads "Sounds" (emitters and plain sounds);
  - `0x004efb20` (1040): the scene loader that calls the readers and
    creates a `LightEmitter` per emitting light (lines 2784/2798);
  - `0x004eff30` (271): applies a detail level (table `0x00689f18`);
  - `0x004f0390` (2442, cdecl): counts a model's textures from its `.slb`
    or `.slt` file (SoultreeObject line 3447, SoultreeMaterial 3472);
  - `0x004f1130` (1541): the texture count pass over models and animations
    (`UnknownTrackGameObject574` `0x004e9ac0` calls it).
- Static initializers: `_$E1`/`_$E2`, `_$E4`/`_$E5`, `_$E7`/`_$E8` and
  `_$E10`/`_$E11`.

## Views declared in SceneManager.h

- `UnknownSceneLight`, `UnknownSceneEnvironment`: the light and environment
  records; offsets from the readers, names from the section keys.
- `UnknownSceneObject` (+0x0c, +0x10, +0x1a0, methods `0x004a8b40` and
  `0x004a6bb0`), `UnknownSceneAnimatedObject` (a GameObject with +0x34,
  +0x5c, +0x194) and `UnknownSceneLodObject` (`0x004444c0`, `0x004451e0`):
  what the entry functions touch of the Soultree classes.
- `UnknownFolded4de580`: a second declaration bound to the empty body
  `0x004de580`. In `0x004ebfc0` VC6 cross-jumps the "found" and "default"
  report calls when both call the same function; retail keeps them apart,
  as calls to two folded functions would.
- `Scene::UnknownFunction4f1130` now takes (counts, width, hasCube,
  ecosystem); `0x004e9ac0` casts the arguments TrackGame.h declares.

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

- `0x004eb570`:
  - a `Length(const Vector3&)` helper that copies y, x, z into locals and
    sums `z * z + (x * x + y * y)` keeps them on the x87 stack as retail
    does;
  - `operator-` returns a named temporary (`Vector3 r(...); return r;`),
    which makes `Normalize` multiply x first;
  - the "T"/"F" flags go through an inline `TF(char)`; written inline, VC6
    keeps the other literal in a register;
  - the message buffer is 0x204 bytes, as in the other readers.
- `0x004ebfc0`: the zone flags use block-scoped `dust`/`dirt` locals, so
  TerrainZone1 and the zone loop get them in swapped frame slots.
- `0x004efb20` indexes `field_0xac[i]` each time; a light pointer local
  changes the register assignment.
- `0x004ef4c0` nests the zone checks under `if (field_0xc0)`; the no
  AuralScape message is the `else`.
- `0x004f1130` sets the key-framed flag in each branch (`1`, `0`, `1`) and
  tests it afterwards; VC6 threads the jumps into retail's layout.

## Not reconstructed

- `0x004eab00` (slot 10, 958 bytes): its ebp frame and `fld; fistp` through
  a pointer are an inlined `__asm` float-to-int helper.
- `0x004ecd60` (4543 bytes) is a near miss in
  `samples/track/SceneManagerNearMisses.cpp` (4534 of 4536 compared
  positions): the "StaticModels" reader, which builds each model's physics
  object (constructor `0x005037c0`, slots 37/40), collision points
  (`0x0043a330`), particle emitters (`0x004b8a00`, `0x004b8df0`,
  `0x004b9310`, slot 27), collision object (`0x00431e70`) and sound emitter.
  Only the address of the name terminator differs (retail adds the element
  offset to the array base first). The views of those classes live in the
  sample. Shapes that mattered: nested `if`s for the missing-count and
  no-models messages, `model[0x44]` and `slt[0x108]` (retail's frame), the
  emitter loop inside `if (points > 0)` and `if (a2)`, inline keyword
  lookups over local `{name, value}` tables, and 0.033333f.
- `0x004edfe0` (5343 bytes): the "Animations" counterpart.
- `0x004f17a0` lies after the player-record initializers that open
  SelectGamePicProcs.cpp.

## Reproduce

```bash
make vc6-gate VC6_ROOT="$VC6_ROOT"
PYTHONPATH=. python tools/run_calibration.py --compiler vc6 \
    --vc6-root "$VC6_ROOT" --profile vc6_o2_mt
```
