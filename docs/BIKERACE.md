# bikerace.cpp

`src/reconstructed/BikeRace.h` / `BikeRace.cpp`. Names are provisional.

Evidence:
- **`__FILE__`:** the bikerace.cpp literal at `0x00567c74`, xrefs
  `0x0041807d..0x004233ab`. The bikerace.h literal at `0x00567de0` is used
  at `0x0041cfd8`, by the inline node-list free in the destructor.
- **RTTI:** `BikeRace : GraphicsTest : GameObject` (vtable `0x00550d24`).
  Only the constructor `0x00417c30` and destructor `0x0041cf30` write it.
- **Same object as RaceView.h:** RaceView.h's `EventBikeRaceView`
  describes the same object at the same offsets (strong inference); its
  callers' mangled names are unchanged.

Extent: `0x00417b00..0x004238bb` (strong inference). The plate-number
painter at `0x00417500..0x00417aff` may also belong here and is
unresolved. Its constructor `0x00417500` (loads BikeNumbers128.tga through
`0x0050a590`, digit widths 19 except 13 for "1"), destructor `0x00417570`,
digit blit `0x00417580` and plate renderer `0x00417670` (to `0x00417af2`) are
strict exact in the provisional unit `src/reconstructed/BikeNumberPainter.cpp`
(class name ours, tier 3). The renderer takes a `D3DIMSoultreeObject`: it
clears each material texture named "PROCEDURAL" with a colour-fill Blt,
paints the number centred on x 28 (subtracting the hundreds and tens from
the `number` parameter itself as they are drawn), rebuilds the texture
(slot 15, then `ManagedTexture` `0x00510670` or slot 9), and, when the
digits counted over every plate are fewer than four, rescales the texture
coordinates of the vertices with z >= -0.5 of every surface using that
material (scale 1/0.75/0.65, offsets by digit count). From `0x004238c0` the code is GR_BitString's; BlockAllocator.cpp
starts at `0x00423f70`. The four kVec3 `$E` pairs sit mid-file at
`0x0041cdf0..0x0041cf2b`.

Exact: 42 functions:
- The constructor, the destructor and its deleting wrapper.
- Slots 10, 14, 16, 20, 22, 23 and 24.
- Network, replay, restart, ghost, racer and debug-draw helpers.
- The recorder worker `0x00421d50`, its callback `0x004230e0` and two sort
  helpers.
- KrustyVCR's implicit destructor and its deleting wrapper.
- The eight `$E`.

Source forms needed:
- Slot 10 `0x0041d2b0`: the message buffer is `char[0x100]` (only 0x80 is
  passed), and `Vector3 position` is declared at the top of the block
  after the network-start branch; otherwise VC6 packs the stack
  differently. Its prologue schedules a global load between the fs:[0]
  load and `push -1`; `mcm2tool/resolved_match.py` recognises that shape.
- `0x00421d50`: float comparisons with -1.0f compile to integer compares;
  the loop index of the message-13 branch is initialised before the mode
  test. The replay tick time is the file static `0x00578e8c`.
- Slot 23 `0x0041f5e0` (keys; jump tables `0x0041ff40`/`0x0041ff64` and
  `0x0042001c`): the 0x22 and 0x30 cases format into `char text[0x80]`
  (only the 0x35 case's is 0x100), which gives the 0x494 frame; the
  overlay switch tests and queues the message inside each case (`case 2,
  0, 1/5, 4` in that order) and deletes it only when non-null; the
  three-way value pick is an if/else chain and the on/off strings are
  if/else calls, not a switch or `?:`; the picker scans index
  `raceScene->field_0xb8/0xb4` directly in `for` loops (named count/base
  locals cost the register that retail uses to keep 0 in `ebx`); the
  debug page test compares a local copy of `field_0x26c4` with
  `field_0x0a8`, and the cursor-flag branch is written negated (picker
  first) with the new picker in a local.

Merged bodies (in `BikeRace.cpp` in retail order; each non-exact one carries
a `// Not exact (...)` comment, and each has an emu_diff case that runs it
against retail, 600 runs with 0 mismatches):
- `0x0041eb20` (`AimCamera`, camera target): exact, 1710 of 1710 bytes
  (calibration case). The view's inline `TargetShadowCaster` stores
  `shadowCaster` before `isSettled`. With the other order, the scene-object
  branch's scratch registers were one rotation step off.
- `0x0041f1d0` (`NextCameraTarget`): 313 of 881 bytes (887-byte candidate;
  linked 0.829). Control flow, the three branches and the shared tail call
  `AimCamera(caster)` are retail's (the caster flag lives in the dead
  third-argument slot). Retail keeps 0 in edx from the first instruction
  (`cmp [mem], edx`, re-zeroed after calls); the candidate tests with
  `test reg, reg` until the first call. Retail has three copies of the
  `AimCamera` call. The objects scan's found path calls it and returns
  (230 -> 266), and the racer-slot helper re-reads the player count at
  each test (266 -> 313).
  - A third explicit call after the racers loop makes VC6 push a constant
    0, which is worse.
  - These leave the zero register unchanged: explicit `!= 0` tests,
    nested-if or `!(a && b)` heads, a `for (;;)` racers loop, and the
    caster flag passed through the `objects` parameter.
- `0x004210f0` (`WriteRelativePositions`, the start grid): 466 of 2981
  bytes (2960-byte candidate; linked 0.981). The call sequence is retail's.
  Retail calls the out-of-line `Vector3` constructor `0x00404e60` at most
  grid sites, the dot `0x0040ae30` and scale `0x005015b0` in `Normalize`,
  and `operator+` / `operator-` (`0x00421cb0` / `0x00421d00`) in event 4.
  The unit declares those as the helpers above the body, linked to their
  canonical symbols. Register choice and scheduling still differ.
  - The inline form (inline constructors and `Scale`) reached masked 0.958
    but expands the mode 2/3 tail where retail calls; VC6's inline budget
    decides which sites expand.
  - The side test is `p1.x * p0.z - p0.x * p1.z`, retail's load order and
    p0/p1 slots.
- `0x00417ed0` (`Create`, the setup; 6745 bytes with the jump table at
  `0x0041992c`; ret 0x24; QuarryStuntEvent.cpp's loader calls it after the
  constructor): 2800 of 6756 positions; linked 0.998. All 166 calls are in
  retail order, with EH states 0..0x10 (`new` lines 0x193..0x367) and the
  0x4e8-byte frame with the buffers at retail's offsets.
  - **The length helper stays `x*x + y*y + zz`.** Squaring x and y into
    `xx` / `yy` locals (EventManager `0x0045d480`'s shape) scores 2821 of
    6760 but rounds differently: at 24-bit precision it moves the flag
    girl's and box's offsets in 2 of 400 emulated runs. That form is a
    pre-fix shape in tests/test_emu_diff.py, so behaviour wins over bytes.
  - **The spill.** Retail spills `z*z` of both vector lengths to a stack
    temporary (`fstp [t]; faddp; fadd [t]`). The only other copies in the
    binary are EventManager `0x0045d480`'s two squared lengths at
    `0x0045dc05` and `0x0045dc99`, where `fstp st(0)` pops the duplicated
    component first.
  - **The girl block.** Retail orders the girl block's temporaries
    differently, so most esp offsets below +0x44 differ.
  - **The `volatile` probe.** `volatile float zz = v.z * v.z` in the length
    helper reproduces the `fstp [t]`, the `fadd [t]`, the slot (`angle`'s
    dead `[esp+0x34]`) and the rest of the frame (6727 of 6768). VC6 then
    evaluates the square before `x * x` and `y * y`, where retail evaluates
    it last. `volatile` is not retail's code and is not used.
  - **Forms tried without effect.** None of these stores `z*z`:
    - named squares in any subset, sum bracketing (the 144 bracketings of
      docs/VC6_OPERAND_ORDER.md section 3), `+=` accumulation, a `float[3]`
      or a struct of squares;
    - inline square helpers, casts, and a double result variable;
    - the `/G3`-`/G6` settings.

    An all-double sum scores 6625 only because its longer x87 code
    realigns the stream. It is not retail's float code.
- `0x00419970` (`CreateRacers`, the racer loader; 13428 bytes; ret 0x14;
  frame 0xab0): 13400 of 13428 bytes; linked 1.0. The body is decoded
  completely: pro circuit, network, offline, ghost and AI racers, collision
  pairing, and the 10 EH states. The facts that put every buffer and scalar
  at retail's offset:
  - The 0x40-byte own texture names and `ranking` are function-scope.
  - The `used` memset (`rep stosd` at `0x0041a456`) sits inside the
    multiplayer block.
  - `RandomSkill` takes `const float&` base and range and writes through a
    `float&`. Retail reads them from memory after the `rand()` calls, and
    spills the random offset into the skill's own slot.
  - The pro circuit block caches `g_TrackGame` in a local and takes each
    computer racer as `&circuit->racers[n]`.
  - The network loops, the grid entries and the event AI records and front
    end tables (`EventAi`, `UiBike`, `UiManufacturer`, `UiRider`) read
    through `g_TrackGame` at each use. With cached locals, VC6 merges the
    failure tails differently.
  - The offline player branch comes first in the source: its `new` carries
    line 0x4ea against the network block's 0x53b. The network player's body
    sits inside its search loop before a `break`.
  - The replay player and replay AI blocks share one function-scope
    `recordedBike` / `recordedRider` pair. Each network player loop declares
    its own `for (int k`. The network block's `i` is function-scope and is
    reused by the network AI loop.

  What remains:
  - one prologue store slot (`+0x3d`);
  - register choices at `0x00419b0d` and `0x00419b39`;
  - a load/store order at `0x00419bf0`;
  - the SIB order of one curve copy (`0x0041b124`);
  - the own texture pair swapped in the frame (retail has `ownRiderTexture`
    at 0x400 and `ownBikeTexture` at 0x440). The two buffers have five uses
    each and VC6 breaks the tie the other way. Declaration order (16
    combinations), names and array forms do not move it.

  `xor edi, edi` / `mov ebx, 1` at +538 depend on the unit's declaration
  count, a TU-wide effect that header edits move. The forward declarations
  KrustyBikeCore.h gained for `KrustyBike::Load`'s game types put them in the
  swapped window (13394); moving `EventAiRacerRecord` into EventManager.h
  and retiring the podium views moved them back (13400;
  BIKE_RACE_STATE_CONNECTIONS.md).

Facts the two loaders establish:
- Racers are KrustyBike (constructor `0x0048fa60`, 0x1638 bytes, the
  GameObject virtual base at +0x160c), set up by `0x0048fc80` (25
  arguments; its 12th is a char AI index). KrustyBike slot 45 takes no
  argument and slot 50 three (`ret 0`, `ret 0xc`).
- KrustyVCR `0x0049c070` takes a char second argument and a name (the
  player name, `TrackGameMode+0`) fourth; `0x0049c1e0` reads a record back.
- The track file name comes from `strrchr(path, '.')` (`0x00534a60`).
- `0x00515ed0` is Track's loader (start and finish probes); `new Track`
  inlines a constructor that clears +0.
- A replay played while KrustyUI's `playingDemo` (+0x4a8) is set gets a
  DemoDlg (vtable `0x00550e18`, 0x7f58 bytes, bikerace.cpp line 0x2fb),
  constructed inline as `UIDialog(1, "credits.dtm")` then the vtable write
  at `0x00419329`; otherwise a VCRDlg (line 0x2ff). `Create` uses the
  canonical DemoDlg from DemoDlg.h.

## Naming

Field names in `BikeRace.h` come from the evidence each declaration's
comment records (literal keys such as "RPMLowerLimit" and "Weight", the
callers' data flow, network message fields); every renamed field keeps its
`// +0xNN` offset. The renames changed no calibration bytes. The tag ball's
state record and fields FollowRacer.cpp (race lane) shares (`timeStep`,
`position`, `runningTime`), the racer's `startPosition` and
`receivedStates` (+0x11c8, each a received `state`) and the ball's
`receivedStates` (+0x4fc, each holding a `record`) were renamed with it;
the inline accessor for the Tag event's ball (TrackGame+0x568, then +0xdc) is
`Ball()`; Wave 13 in NAMING.md renamed the type from UnknownBikeRaceGhost to `TagBall`.
BikeRace's `lodDebugPage` (+0x0a8) is the debug overlay page the
"Soultree LOD" readout gets from `NewPage`, and `chatOverlay` (+0x19c) the
mode's chat overlay (its methods are ChatOverlay's `0x0051dce0`..`0x0051dd70`).
Fields that `GameUiHelpers.cpp` or `KrustyVCR.cpp` also read (the gate
node's centre, direction, half extents and next gate, the model's current
detail level, the camera view's shadow caster and following flag, the VCR
header and its racers) keep their placeholder names until those units are
renamed with them.

Function names (Tier 3, from each declaration's recorded behaviour; the
calibration cases and bindings were rekeyed, bytes unchanged):
`0x00417b00` `PickerHitCallback` (free; the debug picker's hit callback),
`0x00417b30` `SortDescending` (free; sorts keys descending with values),
`0x00417bc0` `RankByPoints`, `0x0041d0d0` `AppendNode`, `0x0041d170`
`BuildNodeList`, `0x0041ea60` `ReplaceMeshId`, `0x0041eb20` `AimCamera`,
`0x00421d50` `RecordFrame` and `0x004230e0` `RecorderCallback` (free; the
RecorderCallbackProc passed to the VCR) and `0x0041f1d0`
`NextCameraTarget` (InGameProcs.cpp keeps its own address-named
declaration), `0x0041d2a0` `SetSceneTime`, `0x0041ea10` `AnyRacerRemote`,
`0x00420650` `RestartRace`, `0x00420bd0` `RestartReplay`, `0x00422ec0`
`ClearRacerParts`, `0x00423040` `ClearAiRacerParts`, `0x00423140`
`DrawTrack` and `0x00423790` `SetDetailLevel`. `0x004210f0` keeps its placeholder because RaceView.h (UI
lane) declares it for a camera sample. The ground shadow's `0x004dab00`
`AddCaster` and `0x004dab90` `ClearCasters` take the names krusty2 gives
those addresses.
