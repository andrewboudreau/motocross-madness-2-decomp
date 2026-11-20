# SceneManager.cpp: the track scene loader

`src/reconstructed/SceneManager.cpp` reconstructs part of
`D:\aardvark\VC\krusty2\SceneManager.cpp`. 52 functions match strictly
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
  post-padding starts, minus jump-table targets): 54. Matched: 52; one
  near miss (`0x004edfe0`), one not reconstructed (slot 10).
- `SceneArchiveFinder` (TrackGame+0x574) has its constructor and
  scene-file helpers here: `0x004e99d0`..`0x004ea390`.
- `0x004de580` (`ret 0xc`) and `0x00464e80` (`ret 4`) are shared empty
  bodies. The readers call them directly with what they read, so they are
  declared as provisional `Scene` members.
- The vector constants `0x00689c98`..`0x00689cc8` are built by the
  `_$E1`..`_$E11` pairs at `0x004ecb70`..`0x004ecd20`.
- `0x004f0d20` keeps an unused 0x18-byte resource manager on its stack,
  with constructor `0x004e8e80` and destructor `0x004e8ea0`. It is declared
  under the alias `UnknownSceneResourceManager` of the canonical resource manager.

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
- SceneArchiveFinder: `0x004e99d0`, `0x004e9a10`, `0x004e9ac0`,
  `0x004e9b80`, `0x004e9ba0`, `0x004e9cd0`, `0x004e9e30`, `0x004e9f70`,
  `0x004ea010` and `0x004ea390`.
- The large readers and loaders:
  - `0x004e9cd0` (345 bytes): opens a file through an archive stream;
  - `0x004eb040` (280): restarts a scene entry's motion or object;
  - `0x004eb570` (2102): reads "Light<n>" (`SceneLight`, 0x8c bytes);
  - `0x004ebfc0` (2650): reads "Environment", the terrain and reverb zones
    (`SceneEnvironment`, 0x420 bytes, line 1456);
  - `0x004ef4c0` (1274): reads "Sounds" (emitters and plain sounds);
  - `0x004efb20` (1040): the scene loader that calls the readers and
    creates a `LightEmitter` per emitting light (lines 2784/2798);
  - `0x004eff30` (271): applies a detail level (points `g_ActiveLodSettings` `0x00689f18` at `g_BladeLodSettings` or `g_NonBladeLodSettings`);
  - `0x004f0390` (2442, cdecl): counts a model's textures from its `.slb`
    or `.slt` file (SoultreeObject line 3447, SoultreeMaterial 3472);
  - `0x004f1130` (1541): the texture count pass over models and animations
    (`SceneArchiveFinder` `0x004e9ac0` calls it).
- Static initializers: `_$E1`/`_$E2`, `_$E4`/`_$E5`, `_$E7`/`_$E8` and
  `_$E10`/`_$E11`.

## Views declared in SceneManager.h

- `SceneLight`, `SceneEnvironment`: the light and environment
  records; offsets from the readers, names from the section keys.
- `SceneCollisionCharacterView` is the key-framed entry's CollisionCharacter. It has
  its own vfptr at +0, a vbptr at +4, the GameObject virtual base at +0x268
  and fields +0x0c, +0x10, +0x1a0, +0x1a4 and +0x210. It stays a `struct`
  so the bound mangled names keep `PAU`.
- `SceneCarProceduralView` is the procedural entry's CarProcedural: a
  GameObject with +0x34, +0x38/+0x3c, +0x5c and +0x194, plus slot 27.
  Naming +0x20c as well changes VC6's code for `0x004eb570`.
- `SceneSoultreeObjectView` has `0x004444c0`, `0x004451e0`, `0x004fbd70` and
  `0x004fc660`.

  Together they are what the entry functions touch of the Soultree classes.
- `SceneAnimation` types +0x08 (the procedural object), +0x0c (a
  `Vector3`) and +0x2c (the file name) and adds bit 5.
  `SceneRandomSet` types its index arrays as `signed char*`.
- `LogDefaultParameter`: a second declaration bound to the empty body
  `0x004de580`. In `0x004ebfc0` VC6 cross-jumps the "found" and "default"
  report calls when both call the same function; retail keeps them apart,
  as calls to two folded functions would.
- `Scene::CountTextures` (0x004f1130) now takes (counts, width, hasCube,
  ecosystem); `0x004e9ac0` casts the arguments TrackGame.h declares.

## Shared changes

- `TrackGame.h`: `SceneArchiveFinder` gets its constructor layout
  and the methods above. `0x004ea010` takes `int*` outputs (TrackRecord
  bindings renamed).
- `ResourceEntry.h` includes the canonical ResourceManager declaration.
  Scene's `UnknownSceneResourceManager` is a type alias, preserving its
  `0x18`-byte stack objects while connecting the existing constructor and
  destructor. FindItem returns ResourceItem; the other resource method
  signatures also agree with their providers (see [resource evidence](RESOURCEMANAGER.md#existing-provider-connections)).
  The complete shared declaration preserves all affected strict bodies,
  including TrackGame `0x00520ab0`; no separate lifecycle view remains.
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

## Field names

`Scene::defaultStartPosition`/`defaultStartDirection` ("SceneInfo" keys)
and the two `fileName` buffers (animation entry +0x2c, static model +0x10)
are named. The section pointers `+0xa4..+0xbc`, the table, caster, light
and entry fields keep their placeholders because `QuarryStuntEvent.cpp` and
its loader sample read them too; renaming them is a joint change with that
unit. `SceneCollisionCharacterView::field_0x10` cannot take a name without the
member lookup resolving to the virtual base's protected `field_0x10`.

## The model and animation readers

Both sit in retail order between `0x004ecc10` (Stadium) and `0x004edf20`
(shadow receivers), with the views they use declared just above them. They
moved in from `samples/track/SceneManagerNearMisses.cpp`.

- `0x004ecd60` `ReadStaticModels` (4543 bytes with its padding and jump
  table) is strict exact (4536 of 4536 compared positions; a calibration
  case). It reads "StaticModels" and builds each model's physics object
  (constructor `0x005037c0`, slots 37/40), collision points (`0x0043a330`),
  particle emitters (`0x004b8a00`, `0x004b8df0`, `0x004b9310`, slot 27),
  collision object (`0x00431e70`) and sound emitter. Shapes that mattered:
  nested `if`s for the missing-count and no-models messages, `model[0x44]`
  and `slt[0x108]` (retail's frame), the emitter loop inside
  `if (points > 0)` and `if (a2)`, inline keyword lookups over local
  `{name, value}` tables, 0.033333f, and the name length's clamp `count`
  declared at function scope and cleared with the other counters at the
  top (`count = 0;`), which gives the name terminator's address retail's
  operand order.
- `0x004edfe0` `ReadAnimations` (5343 bytes) is a near miss, 5289 of 5335
  bytes. It reads "Animations". Each "Animation<n>" entry
  (`SceneAnimation`, 0x44 bytes) is either key-framed (bit 3; a
  `CollisionCharacter`, 0x294 bytes, constructor `0x004318d0`, loaded by
  `0x004319c0`, motions from `0x004a6b30`) or procedural (a `CarProcedural`,
  0x22c bytes, constructor `0x0042f390`, slot 27). The "RandomSet<n>"
  sections fill `SceneRandomSet`.

The game links both bodies; `config/game_build.json` `link_aliases` joins
the views' constructors and methods to their providers.

### Forms tried

Scene::ReadStaticModels (0x004ecd60, 4536 code bytes): the "StaticModels"
reader, strict exact. The last difference was the model name's terminator
`staticModels->casters[i].fileName[count] = 0`: retail adds the element
offset to the array pointer first (`add edx,ebx; mov [edx+esi+0x10],0`),
the candidate added the count first. `count` is a function-scope local
that is zeroed with `models` at the top: its leaf is then older than the
`i * sizeof` temporary created at the first `casters[i]`, and VC6 adds the
older leaf last (docs/VC6_OPERAND_ORDER.md 1.2). The clear is inferred
from that order (its value is never read; 4534/4536 without it). A dead
initialisation of `length` instead, or `count` declared at function scope without the early
store, leaves the candidate order. Forms that never moved it while `count`
was first assigned at the strcspn clamp: pointer and index spellings, the
clamp forms, inline helpers, an element pointer local, declaration order,
`count`/`length`/`i` types, `'\0'`, `*(p + count)`, extra uses of `count`,
`(p + i)->`, a count-plus-pointer form, char* arithmetic, `i++` with
`i + 1`, a `follow` local for the sound emitter's model (worse), 0..63
prepended typedefs, 0-33 dead locals and the /G3-/G6, /GB, /Gy, /Gf, /Zp,
/Op, /Ox, /vm* flags.
k-scan (s22): compiled behind 0..63 prepended typedefs, 0x004ecd60,
0x004edfe0 have the same code at every count, so their differences are not
a declaration-count effect (docs/VC6_OPERAND_ORDER.md section 2).

Scene::ReadAnimations (0x004edfe0, 5335 bytes): the "Animations"
reader. Instruction count (1479), calls, EH states and every error tail
match. The random-set loop counts with `i++` and prints `i + 1` (no
`next` temporary), and the strcspn result is its own block-local `dot`:
that loop then keeps retail's strength-reduced entries offset in ebx,
stores/reloads `dot` around strlen and stores an immediate 0 at
token[dot], and the "Cannot find" errors keep retail's two sprintf tails
(4393 -> 5113 of 5335). The frame: the five sound-settings scalars
(flags, oneShotDistance, randomTriggerPercent, oneShot, force2D) are
locals of the `if (strcmp(sound, "") && ...)` block and `automatic` is a
local of the keyFramed block (it serves both GetInt calls there). Both
are needed together; either one alone leaves the frame at 0xd00 (5113).
With them the frame is retail's 0xcfc, slot 0 holds retail's {sequence j,
axes, random-set j, temporaries} and every slot from useLighting (0x24)
up is retail's (5288). heading and pitch as loop-body locals give one
more byte (5289); any other single scalar moved to the loop body gives
the same byte, more of them nothing further.
Left (46 bytes): (1) the low scalar slots. Retail: {dot, automatic} 4,
{fps, the random-set entries-offset spill} 8, animations 0xc, i 0x10,
frontWheelsTurn 0x14, lagDistance 0x18, next 0x1c, tires 0x20. Here:
{spill, i} 4, {automatic, dot} 8, animations 0xc, frontWheelsTurn 0x10,
next 0x14, lagDistance 0x18, fps 0x1c, tires 0x20. So retail lets the
spill reuse fps's home and keeps i's home to itself. (2) The
motions allocation tests and reloads motionCount as [ebx + ecx + 0x1c]
(i offset first) where this gives [ecx + ebx + 0x1c].
Without effect on (1), on top of the form above: fps (alone or with
lagDistance, frontWheelsTurn, tires) in the loop body, in a block around
the loop or in a bare block inside the loop body; any order of the
loop-body declarations; a separate random-set index (`for (int k` or a
function local). Before the form above, 72 scope combinations of
automatic, fps, dot, i and axes were flat too; `automatic` as two
separate bare-block locals around the two GetInt calls gives slot 0
retail's contents and moves the spill into the first one's home (5129,
frame still 0xd00). Reusing `automatic` for dot loses the store/reload
shape (4349).
Tried for (2): no `table` local or a named count (5268), `table` taken
before the test (3076) or used for the store too (2688), an entries or
entry pointer local (2736, 2869), `i = 0;` or `next = 0;` at the top
of the function (5289, no change).

## Not reconstructed

- `0x004eab00` (slot 10, 958 bytes): its ebp frame and `fld; fistp` through
  a pointer are an inlined `__asm` float-to-int helper. The bring-up body
  (`src/bringup/Group4_SceneTick.cpp`) keeps the frame with a per-function
  pragma (docs/GAME_BUILD.md#frame-pointer-units).
- `0x004f17a0` lies after the player-record initializers that open
  SelectGamePicProcs.cpp.

## Reproduce

```bash
make vc6-gate VC6_ROOT="$VC6_ROOT"
PYTHONPATH=. python tools/run_calibration.py --compiler vc6 \
    --vc6-root "$VC6_ROOT" --profile vc6_o2_mt
```

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x004e9980` `FindKeyword`
- `0x004eb040` `RestartEntry`
- `0x004eb160` `ReadVector`
- `0x004eb300` `ReadColor`
- `0x004eb480` `ReadFlags`
- `0x004eb570` `ReadLight`
- `0x004ebdb0` `ReadFog`
- `0x004ebfc0` `ReadEnvironment`
- `0x004eca20` `OpenResourceFiles`
- `0x004ecc10` `ReadStadium`
- `0x004ecd60` `ReadStaticModels`
- `0x004edfe0` `ReadAnimations`
- `0x004ef4c0` `ReadSounds`
- `0x004ef9c0` `ReadSoundSettings`
- `0x004f00e0` `CountTexture`
- `0x004f0310` `AddResource`
- `0x004f0390` `CountModelTextures`
- `0x004f0d20` `CountObjects`
- `0x004f0ec0` `ResolveSltPath`
- `0x004f1130` `CountTextures`
- `0x004eaec0` `FindEntryByFileName`
- `0x0043a330` `AddCollisionPoint` (the view in this file; the name
  krusty2's CollisionPoint.cpp gives the same address)
