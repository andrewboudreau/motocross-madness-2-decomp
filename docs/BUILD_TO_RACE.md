# From the source build to a running race

Static audit of what stands between `python tools/build_game.py` and a race
that loads and runs. Audited at `4dd41c93` on 2026-11-15. No executable was
launched for this audit; the live evidence is an earlier run (see Status).

Evidence tiers follow `AGENTS.md`: **C** confirmed (tool output, bytes, logs),
**S** strong inference, **P** provisional.

```bash
export VC6_ROOT="$(pwd -W)/work/vc6sp3-disc" MCM2_EXE="$(pwd -W)/work/game/mcm2.exe" PYTHONPATH=.
python tools/build_game.py --vc6-root "$VC6_ROOT"   # work/game-build/build-*/mcm2.exe, mcm2.map
python tools/linked_accuracy.py                     # work/linked-accuracy/report.{md,json}
python tools/audit_bindings.py                      # work/binding-audit/report.md
python tools/audit_game_vtables.py                  # work/vtable-audit/report.md
python tools/linked_data_accuracy.py                # work/linked-data/report.md
python tools/emu_diff.py <case> -n 30               # one case at a time
```

## Status

| Claim | Tier | Evidence |
|---|---|---|
| The build compiles 380/380 units and links with no unresolved externals. | C | Build `work/game-build/build-dms49u5e`, exe SHA-256 `5a296a96...f4cb25`. |
| Every retail function has a linked body. | S | 2,698 bound functions scored; vtable audit 7,315 slots ok, no missing override, purecall or stub; the 7 non-vtable call targets my scan could not bind are present in source. |
| No linked body calls a function retail does not call. | C | `linked_accuracy.py`: 0. |
| 803,572 of 1,129,714 scored retail bytes are masked-identical in the linked exe. | C | `linked_accuracy.py`. 189 bodies (326,142 B) are not. |
| Initialised game data, strings, reals and GUIDs equal retail where a symbol is bound. | C | `linked_data_accuracy.py`: 0 different. 479 nonzero retail runs (7,686 B, 198 code-referenced) are covered by no bound symbol. |
| Bindings are consistent. | C | `audit_bindings.py`: 19,383 bindings, 0 findings; 0 link aliases bound to a different VA. |
| Imports equal retail (244 functions, 15 DLLs), names and hints included. | C | Build `build-qz1cs5ms` against `work/game/mcm2.exe` with `pefile`. `blade.dll` and `DINPUT.dll` are imported by undecorated name with retail's hints (2, 3 and 1): `build_game.py` turns VC6 LIB's ordinal entries from `config/game-imports/*.def` into SDK-style imports by name. |
| The exe has no `.rsrc` section. | C | Retail carries 4 icons, a group icon and a version block (8,768 B). No dialogs or strings live in the retail exe. |
| A source build booted to title, loaded a track, rendered a race and returned to the menu. | C | Run of 2026-11-05 20:55 in `work/run/srcbuild` (exe `090daf45...`), `MCM2_AUTORACE`: 24 frames, clean `ExitProcess` after 4 minutes. |
| The loading backdrop no longer stays behind the HUD. | S | Absent in every race frame of that run; no later run confirms it. |
| The current build boots and races. | P | Not run. About 560 commits since the last live run, 192 of them touching `src/bringup` or `config/game_build.json`. |
| Gameplay equals retail. | P | `build_game.py` itself reports `linked_runtime_unverified`. Nothing compares a source-build race with a retail race. |

Section sizes (C): `.text` 1,387,797 against 1,368,518 (+19,279), `.rdata`
89,612 against 89,490, `.data` 1,203,616 against 1,205,408 (-1,792). Same
subsystem, characteristics, stack and `SizeOfImage`. Addresses differ from
retail everywhere, so crash EIPs map through the build's own `mcm2.map`.

CRT start-up (S): the `/MT` CRT comes from the VC6 libraries; the `__xc_a..z`
table is emulated by `tools/static_initializers.py`; every initializer runs
in both images, the game object's (retail 0x4aa7b0) with fixed import answers.

## Boot path

Score is the masked linked score from `linked_accuracy.py`. "Emu" means a
`tools/emu_cases` differential case exists.

| Step | Retail VA | State | Notes |
|---|---|---|---|
| CRT entry, static initializers | 0x5380a5 | library; emulated | All initializers run in both images (`static_initializers.py`). |
| `WinMain`, `WinMainBody`, window, `MainWindowProc` | 0x4a0bc0, 0x4a0680, 0x4a0c50, 0x4a0d50 | exact | Icon load returns NULL without `.rsrc` (cosmetic). |
| `GetDXVersion` | 0x448560 | near miss 0.914 | 720-scenario native differential test, no emu case. |
| `FindDataDirectory` | 0x523c90 | exact | Needs the registry values the shim writes. |
| `PCGame::StartUp` | 0x4bfc50 | exact | |
| Enumerate, choose display, joystick, load profile | 0x4c9600, 0x4ccd60, 0x4cd610, 0x4c16f0 | exact | |
| `ProfileDisplays` | 0x4c0d10 | near miss 0.998 | Emu case `pcgame_profile_displays` (replay mode, fake PCGame registry, display and DirectDraw vtables) agrees in 600 of 600 runs, all 12 probes reached. |
| `CreateModeSurfaces` | 0x4ca520 | near miss 0.774 (116 B) | Emu case `pcvideocard_create_mode_surfaces` agrees in 800 of 800 runs, including negative and overflowing budgets. Retail quits here when video memory reads negative. |
| `ProbePartialTextureUploads`, `ProbeNonLocalTextureMemory` | 0x4cab00, 0x4ca5a0 | near miss 0.826 | Emu cases `pcvideocard_probe_partial_uploads` and `pcvideocard_probe_nonlocal` (replay mode, fake COM objects) agree in 600 of 600 runs each. Runs in the hardware test on a fresh profile. |
| `Game` constructor, init, slot 2, render | 0x467990, 0x467b70, 0x467af0, 0x467eb0 | exact | |
| `Game::Tick` | 0x4685d0 | near miss 0.749 | `QueryPerformanceCounter` in place of inline `rdtsc`. Emu: 60 runs, 0 mismatches. |
| `ResourceManager::AddArchive`, `FileStream::Read` | 0x4e9030, 0x461640 | near miss (0.931) | Emu cases `resource_add_archive` and `filestream_read` agree with retail (800 runs each). Every asset goes through these. |
| `KrustyUI::Create`, `UIDialog::Tick` | 0x4988a0, 0x46ef00 | exact | |
| `ReadDialogResource` | 0x46a920 | bring-up 0.967 (16,120 B) | Every menu screen. Emu: 0 mismatches. |
| `MainDlg::HandleEvent` | 0x44b200 | bring-up 0.992 | No emu case; the instruction review finds only stack layout and register differences (bring-up review). |
| `SPRaceInfoDlg::HandleEvent`, rider dialogs | 0x4507a0, 0x4d72c0, 0x4f8820, 0x44fae0 | bring-up 0.966-0.994 | Emu for 0x4507a0; the three rider `Tick`s pass the instruction review (bring-up review). |
| `LeaveMenusForRace`, `LoadingDlg::HandleEvent` | 0x4536e0, 0x451020 | exact | |
| `EventManager::LoadEvent` | 0x45cb70 | exact | |
| `LoadEventObjects` | 0x4de590 | bring-up 0.985 | Emu: 0 mismatches. |
| `Terrain::Load` | 0x5059d0 | near miss 0.985 | Emu: 0 mismatches, but 42 of 60 retail runs took the fail path. |
| `Terrain::PrepareGeometry` | 0x507610 | bring-up, unscored (unbound) | Has an emu case. |
| `BaseQuarryEvent::PrepareGeometry` | 0x4e14a0 | near miss | No emu case. |
| `EcoSystem::GenerateObjects` | 0x459ce0 | bring-up 0.914 | Uses a private `NormalDistribution(100)` in place of the retail shared one. |
| `BikeRace::Create` | 0x417ed0 | bring-up 0.998 | Emu: 0 mismatches. |
| `BikeRace::CreateRacers` | 0x419970 | bring-up 0.999 (13,428 B) | Emu case `bike_race_create_racers` (replay mode) agrees with retail in 400 of 400 runs, all 19 load paths reached. |
| `LoadBike` | 0x4079c0 | bring-up 0.987 | Emu: 0 mismatches. |
| `EventManager::Tick`, `BikeRace::Tick`, `KrustyBike::Tick` | 0x45f200, 0x41d2b0, 0x4977a0 | exact | |
| `KrustyBike::SimulateFrame`, `Vehicle::SimulateFrame` | 0x496790, 0x52a940 | near miss 0.999, 0.998 | |
| Bike AI | 0x40eca0, 0x414370, 0x517340 | bring-up 0.957, 0.951, 0.842 | 34,965 B unit. |
| `SceneTick` | 0x4eab00 | bring-up 0.941 | |
| Visibility walk, `TestBox` | 0x481de0, 0x52f570 | near miss 0.733, bring-up 0.796 | Decide what is drawn. `TestBox`: emu case `visibility_testbox` (item 9; D3DIMSoultreeShadow now calls it directly). |
| `LightVertices`, `TransformPoints` | 0x49e4a0, 0x4a1b00 | bring-up 0.754, 0.336 | Low scores come mostly from inline-asm retail bodies. Emu cases agree with retail on every reached branch (item 9). |
| `DrawCurrentLod` | 0x440f30 | bring-up 0.901 (7,851 B) | Reads a local retail leaves uninitialised; source sets it to 0. |
| Particles, projected shadow | 0x4bac60, 0x4db8c0, 0x4dbd30, 0x4dc2b0 | bring-up 0.712; near miss 0.738-0.837 | |
| Texture creation and copies | 0x4c69c0, 0x4c7b40, 0x486170, 0x50c960, 0x50dad0 | near miss 0.867-0.957 | Emu cases for creation and copies (item 7). The two repacks (0x50c960, 0x50dad0) pass the instruction review (bring-up review); they have no emu case. |
| Pixel converters | 0x4d0d40, 0x4cea10, 0x4ce5f0 | bring-up 0.418-0.715 | 8-bit and half-size texture paths. |
| HUD, radar, scoreboard | 0x51c720, 0x48d1e0 | bring-up 0.867-0.973 | `DrawTrackOutline` (0x51c720): emu case `radar_draw_track_outline`, 600 of 600 runs agree. |
| `PCRenderTarget::Clear`, `AdvanceFrame`, `RenderPost3D` | 0x4c5510, 0x4e8cc0, 0x46f320 | exact | |
| Podium, last-race dialog, replay | | bring-up 0.967-0.986 | Never exercised live. |
| Multiplayer dialogs and session | | bring-up 0.964-0.997 | Never exercised live; lobby is stubbed by the shim. |

No step is missing. Bring-up units cover 129 scored functions (223,846 B, of
which 33,566 B are identical). 117 non-identical functions (135,862 B) come
from reviewed near-miss sources outside `src/bringup`.

## Bring-up review

On 2026-11-16 build `build-kjdvaspt` linked 232 bring-up symbols
(`src/bringup`, by binding). 159 had a clean differential case or a calibration
case. The other 73 were checked:

- **16 bodies with their own logic** are reviewed below: 7 by new differential
  cases (0 of 4,200 runs differ, every probe reached), 1 strict exact and 8
  by reading the instruction diff against retail.
- **46 forwarders** call a canonical or retail body (checked in the linked
  exe: each built body calls out). Their only risk is argument order and
  type conversion. One (`TireNode::LocalToWorldDirection`) has a case; the
  rest are not reviewed one by one.
- **11 aliases** are folded by the linker onto one of those forwarders.

| Body | Retail | Review |
|---|---|---|
| `RadarOverlay::DrawTrackOutline` | 0x51c720 | Case `radar_draw_track_outline`: random track graphs (rings, open chains, links, pre-flagged nodes), modes 0-3; 600 runs. |
| `InlineShock::ClampAndStep` (then the bring-up `BikeQ::ClampAndStepAlong`; BikeAI now calls Shock.cpp's body, which the case checks) | 0x4fa090 | Case `shock_inline_clamp_and_step`; 800 runs. |
| `RotatingShock::ClampAndStep` (then the bring-up `BikeQ::ClampAndStep`; now Shock.cpp's body) | 0x4fa8b0 | Case `shock_rotating_clamp_and_step`, non-finite angles included; 800 runs. |
| `Vec3Normalize` (CollisionVec3) | 0x5087b0 | Case `collision_vec3_normalize`, unit and zero vectors; 800 runs. |
| `TireCross`, `ShockScaleCall` | 0x515600, 0x5015b0 | Cases `tire_cross`, `shock_scale_call` (forwarders; a swapped operand order is caught in 63 of 100 runs); 400 runs each. |
| `LocalToWorldDirection` (TireNode view) | 0x4fd5c0 | Case `tire_node_local_to_world_direction`; 400 runs. The drawing function's view and its case `soultree_local_to_world_direction` were retired: D3DIMSoulTree.cpp calls the SoultreeObject body directly. |
| `Vec3::operator-=` | 0x413160 | Strict exact (33 of 33 bytes). |
| `MainDlg::HandleEvent` | 0x44b200 | Instruction review: retail overlaps the two `connections[5]` arrays (frame 0x6c4 against 0xc64), keeps the constant 1 in `edi`, and swaps the operands of one 16-byte `repe cmpsd` equality test. |
| `MPRaceInfoDlg::HandleEvent` | 0x4513a0 | Instruction review: register choice, and one reload of `g_TrackGame+0x570` with no store or call in between. |
| `PCCentralBikeRiderDlg::Tick`, `MPBikeRiderDlg::Tick`, `SPBikeRiderDlg::Tick` | 0x4d72c0, 0x4f8820, 0x44fae0 | Instruction review: in the by-value `Vector3` copy to the camera, one store to the outgoing argument area moves past a load from the source vector. |
| `EcoSystem::WriteListing` | 0x458da0 | Instruction review: base and index swapped in four address operands. |
| `ManagedTextureGroup::RepackWholePages` | 0x50c960 | Instruction review: stack slots (frame 0x78 against 0x80); register choice in the `Contains` loop; the `plannedLevel` store moves past two loads from `groupManager`; the levels loop rereads its bound, while retail loads it once (the loop writes only the stack). |
| `ManagedTextureGroup::RepackWithPartialBlits` | 0x50dad0 | Instruction review: the dead `dropped = 0` store sits before the empty-list return; two `+0x38` loads and one argument push are reordered. |

Planted bugs are caught: `>=` in the shock's `extending` test (40 and 46 of
200 runs), the wrong hit coordinate when the outline re-enters the map (62 of
200), swapped cross-product operands (63 of 100).

Instruction review covers the aligned code only. Switch tables are unresolved
relocations in the candidate, so their targets were not compared. The cases
check one function at a time on random and edge inputs, not integrated play.

### Near misses without an emu case

`linked_accuracy.py` on `build-kjdvaspt` finds 189 non-identical bodies; 21
(29,020 B) have no `tools/emu_cases` module. Each is accounted for:

- 8 are reviewed above or in item 14: `MainDlg` and `MPRaceInfoDlg`
  `HandleEvent`, the three rider `Tick`s, both repacks and
  `BaseQuarryEvent::PrepareGeometry`.
- 10 have native tests that run the candidate against the retail body
  (`VC6_ROOT` and `MCM2_EXE` required; all 10 pass, none skipped):
  `BackgroundImage::GetDc` (`test_background_dc_behavior`), `GetDXVersion`
  (`test_device_setup_behavior`), `UIEditBox::Draw` and `OnChar`,
  `GUIUser::SetPointerDevice`, `UIListBox::ButtonDown`,
  `MediaControl::Open`, `UIMultiState::Draw`, `Overlay::RestoreRect`,
  `RecordExceptionInfo`.
- `CollisionObject::~CollisionObject` (0x432000) matches all 218 retail
  bytes when compiled alone. The `KrustyBike` vtordisp thunk (0x497c30)
  matches its 8 bytes; retail's 67-byte extent runs on into the padding and
  the deleting destructor after it.
- `HiResMeter::SetRenderContext` (0x488440) differs on purpose. Retail reads
  RDTSC, which VC6 C++ cannot emit, so the build calibrates against
  QueryPerformanceCounter for both stamps (`GAME_TICK.md`). It only scales
  the profiling display.

## Blockers and risks, ranked

1. **The current exe has never been run** (C). The only live proof is ten
   days and about 560 commits old. Until it runs, every claim about booting
   is inference.
2. **Bring-up bodies are approximations** (C for the scores and the per-function
   checks, P for integrated behaviour). Every bring-up body now has a
   differential or calibration case or an instruction review, or is a
   forwarder (see [Bring-up review](#bring-up-review)). The best-guess candidate
   sources are reviewed too (item 14). What is untested is how they behave
   together over real races.
3. **Near misses without an emu case** (C): of 189 non-identical bodies, 21
   have no `tools/emu_cases` module. Each has a native retail test, an
   instruction review or a byte match, or is a documented adaptation (see
   [Near misses without an emu case](#near-misses-without-an-emu-case)).
4. **Low-score render bodies** (C for scores): the visibility walk, particles,
   projected shadow. The pixel halvers, `TransformPoints`, `LightVertices` and
   `TestBox` agree with retail in differential cases (items 8 and 9). A wrong result here shows as missing or miscoloured geometry, not
   as a crash. The 11-05 frames look right for one camera on one track (S).
5. **Environment** (C): the shim accepts the demo's hash, or a source build
   whose `mcm2.map` beside it names the exe's link timestamp (S: built, not
   yet run); the game exits within a
   second under RDP or a disconnected session, as retail does; the EULA must
   be accepted by the user; registry values must exist.
6. **Placeholder globals and guessed initial values** (C that they exist, P
   for effect): `g_visibilityClipper` is an empty static (retail gives no
   size or owner, see `CLIPPER_GLOBAL_CONNECTIONS.md`); the KrustyBike stall thresholds
   and trick tables are file-local copies; `k = 0` in `DrawCurrentLod`.
   `audit_bindings.py` lists 47 `data/unknown` names no object defines and 63
   placeholders (CRT helpers only, benign).
7. **Crash sites from 10-30 to 11-05 builds are not re-verified** (C that
   they occurred): `work/run/srcbuild/errorlog.txt` holds 16 access
   violations, the last at 20:01 on 11-05, before the good run.
8. **No headless entry** (C). The only unattended route is `MCM2_AUTORACE`
   through the shim on a real desktop.
9. **Timing** (S): `Game::Tick` uses a different clock source; frame pacing
   may differ from retail even when everything else is right.
10. **No `.rsrc`** (C): no icon or version info. Cosmetic.

## Worklist

| # | Item | Effort | Why |
|---|---|---|---|
| 1 | User runs the current build with `MCM2_AUTORACE` (test plan below). | 10 min | Turns blocker 1 into a fact either way. |
| 2 | Done: `tools/record_run.py` writes `run_record.json` into the run directory (exe hash, matching build, its commit and dirty flag, last session, frames, error-log changes); `build_game.py` now records the commit in `report.json`; `GAME_BUILD.md` cites the last record. | - | Stops the run status drifting from the tree. |
| 3 | Done: `pcvideocard_create_mode_surfaces` (budget threshold, negative and overflowing budgets), `pcvideocard_probe_nonlocal`, `pcvideocard_probe_partial_uploads` and `pcgame_profile_displays` (replay mode with fake DirectDraw, surface, display and registry vtables) agree with retail in every run, all probes reached. Planted bugs are caught: a `>=` budget test (66), the wrong caps bit (98), a 32-pixel tile step (100), a changed error code (44), nine textures instead of ten (56), a misspelt registry key (274). Callee results are modelled, not run. | - | Start-up bodies whose failure ends the process silently. |
| 4 | Done: `tools/emu_cases/filestream_read.py` (buffered, encoded, wrapped and short reads) and `resource_add_archive.py` (replay mode: header, growth, merge, realloc failure, unwind) agree with retail in 800 of 800 runs each; planted bugs are caught. Both bodies had earlier native differential checks (docs/FILESTREAM.md, docs/RESOURCEMANAGER.md). | - | Every asset load depends on them. |
| 5 | Done: `tools/emu_cases/bike_race_create_racers.py` (replay mode) covers the rider archive, replay ranking, the pro circuit career, single player, ghost, AI (event and random skill) and network loads (local, remote players, remote and host AI). It agrees with retail in 400 of 400 runs, all probes reached; a planted grid-position bug gives 47 mismatches in 200. The behaviour is confirmed against these modelled inputs only: callee results are replayed, not run. | - | Largest bring-up body on the race path. |
| 6 | Done: `terrain_load` models mostly well-formed `.trn` files, so 440 of 600 runs load (was 184), and every failure exit keeps runs (no stream, bad tag or version, palette failure, truncation, null allocation). New probes cover the tag and header checks, palette creation and the loaded exit; a 0x200 width exercises the `width == 0x100` test. 600 of 600 runs agree with retail. Planted bugs caught: height-field argument (434), `>=` width test (46), bounds padding (280), quality default (468), block seek (448). Callee results are modelled, not run. | - | The case used to pass mostly by failing early in both builds. |
| 7 | Done: `texture_create_surface` (shared and mip surfaces, the format fallback lists, DirectDraw creation, palette, conversion, mip build, colour key and render states), `texture_copy_rect_to` (BltFast and the 8/16-bit table paths with real pixels), `texture_clone`, `cache_texture_copy_mip_levels` (mip chains of real depth) and `gui_copy_screen_to_texture` (the captured and dimmed pixels at 1-4 bytes) agree with retail in every run, all probes reached. Planted bugs are caught: a wrong fallback format (40), swapped address states (20), `||` in the mip count (112), a halved-twice rect (486), a `>=` level walk (152), the wrong dim mask (98) or checkerboard phase (58). Retail calls GetCaps through null for a mip-mapped 888/8888 texture (no DirectDraw fallback on that path); the cases avoid that input. Callee results are modelled, not run. | - | Texture corruption is hard to spot by eye. |
| 8 | Done: the converters already had cases (`convert555_to8` 0x4d0d40, `halve565` 0x4cea10, `halve555` 0x4ce5f0; 40-60 runs in the suite). Against the game build object they agree with retail in 2,000 of 2,000 runs each (`-n 2000 --seed 7`): random full 16-bit pixels, every filter path (packed, keyed with all-key blocks, rounded) and random 32 KiB index tables. Not exhaustive: sizes stay small (halvers up to 8x8 output, the converter 8-40 pixels wide, as its caller passes multiples of eight). | - | Pure functions with the lowest scores. |
| 9 | Done: emu cases cover `TransformPoints` (0x4a1b00), `VisibilityClipper::TestBox` (0x52f570) and `LightManager::LightVertices` (0x49e4a0) with its three workers, plus linked forwarders (`shadow_clipper_testbox`, `shadow_transform_ortho`; the `light_manager_light_vertices` forwarder case went with the forwarder, 2026-11-28, and `grid_light_vertices` covers the body). They reach every retail branch edge but one the caller rules out (0x49f99c), with no mismatch in 300-400 runs per FPU control word against the game build and `--compile`. `grid_light_vertices` now reaches the term-array regrow path (allocators stubbed at 0x4a2e20/0x4a2e60, with failures) and the exact-unit-length branches; `visibility_testbox` adds boxes on the clip planes, which moved the centre-test boundary mutants from uncaught to caught. Every planted bug is caught except two that compile to identical code. The low scores hide no behaviour difference found so far. | - | Decide whether low scores hide behaviour differences. |
| 10 | Frame comparison: the same `autorace.bin` on retail and source build, compare frames at fixed times. Retail can now load it: the runtime ports `AutoRaceTick` (item 11), and with the fixed clock both builds reach the race on a fixed frame. | 1 day, needs user runs | First whole-program check of rendering and physics. |
| 11 | Built, not yet run. With `MCM2_STATE_LOG=1` the runtime (`runtime/d3drm/replay.c`) writes `statelog.txt`, one line per presented frame: menu, race clock and each racer's position, velocity, speed, laps, gates, place and total time as raw float bits. `MCM2_FIXED_CLOCK=<ms>` makes the game clock and every `srand` seed repeatable. Retail runs a port of `AutoRaceTick` (`MCM2_AUTORACE_PORT=1` uses it on a source build too). `tools/compare_state_logs.py` aligns two logs at the first race frame and names the first field that differs. `tests/test_runtime_replay.py` checks the retail addresses against the bindings and every offset with VC6 `offsetof`. | - | Cheaper and stricter than images for physics and AI. |
| 12 | Re-run the old crash scenarios and map any new EIP. | 1 h of user time | Closes blocker 7. |
| 13 | Done, except the clipper object: `g_SharedNormalDistribution` (0x56ece0) is defined in `NormalDistribution.cpp`, whose `__FILE__` string follows it in retail `.data` (VC6 lays them out in that order); `Group4_EcoBridges.cpp` is gone. The visibility box strip is one D3DLVERTEX-shaped array, so the constructor's 0x2000ffff colour reaches DrawBox's lines as in retail (constructor 0x52d340 and DrawBox 0x52e1f0 stay exact). The 5,000-entry grid texture tables fill retail's gaps exactly. `g_visibilityClipper` stays an empty static: retail gives no size or owner. | - | Removes known layout differences. |
| 14 | Done: all 19 are reviewed. BuildFaceGeometry was already strict exact and moved into CubeDraw.cpp. 35 differential cases over the other bodies (one new, `halve8`) re-ran clean, 0 of 8392 runs differ. PrepareGeometry (the debug Atmosphere page) differs only in register choice and one independent move (docs/GAME_BUILD.md#best-guess-review). | - | No candidate source is best-guess any more. |
| 15 | Done: `static_initializers.py` answers `GetVersionExA` and the other imports the game object's initializer calls (directory, IMM32, drives, locale, registry, CRT TLS) with fixed values. All initializers run in both images; 11 more data bindings compare and match, among them the trig tables and `g_TrackGameInstance` (docs/REBUILD_GAPS.md). | - | Completes the static-initializer check. |
| 16 | Link a `.rsrc` with user-supplied icons and a version block. | 0.5 day | Icon and file properties; no data may be committed. |
| 17 | Done: VC6 LIB writes no import by undecorated name from a `.def`, but VC6 LINK reads one (WINNT.H `IMPORT_OBJECT_NAME_UNDECORATE`), as in the SDK's import libraries. `build_game.py` sets that name type on the `.def` entries given as `name@N @hint` without NONAME; the hint stays. `DirectInputCreateEx` and both `blade.dll` imports are now by name with retail's hints; the whole import table equals retail. Built and audited, not yet run. | - | Removes a machine-specific ordinal. |
| 18 | Done (not yet run): the shim turns its hooks on for a source build when the `mcm2.map` beside it carries the exe's PE link timestamp (`Timestamp is` line; equal in the last three builds, retail differs). A stale map does not count. `MCM2_FORCE_RUNTIME` stays as an override. | - | Drops `MCM2_FORCE_RUNTIME` from every run. |
| 19 | Exercise podium, replay and return-to-menu with `MCM2_AUTORACE_EXIT`, then a full lap by hand. | 30 min of user time | These paths have never run in a source build. |
| 21 | Done: every bring-up body without a case was reviewed: 7 new emu cases, 1 strict exact, 8 instruction reviews, 46 forwarders (one with a case) and 11 folded aliases (see Bring-up review). | - | Closes the per-function part of blocker 2. |
| 20 | Done: whole-program emulation is out of scope (see Out of scope). | - | `blade.dll`, DirectDraw and COM make a unicorn boot impractical; per-function replay cases give the coverage. |

## Out of scope

Whole-program emulation: booting either exe under unicorn to a race. The boot
path loads `blade.dll`, creates DirectDraw, Direct3D and DirectInput objects
through COM and pumps a window message loop; emulating that means modelling
those libraries rather than checking the game. Coverage comes from the
per-function `tools/emu_cases` (replay mode answers the COM and callee calls),
the static-initializer run, and live runs on the user's desktop.

## Runtime test plan (the user runs these)

Preconditions: `query user` shows an Active console session, not RDP. Game
closed. Staged directory `work/run/srcbuild` holds the demo data and the
runtime DLLs.

1. Copy `work/game-build/build-92m0hh0c/mcm2.exe` (imports by name, item 13 data) and `mcm2.map` into
   `work/run/srcbuild`, and install the current shim with
   `python runtime/build.py --stage work/run/srcbuild`. Move the old
   `frames`, `errorlog.txt` and `mcm2_runtime.log` aside.
2. Run with `MCM2_AUTORACE=1 MCM2_AUTORACE_EXIT=120`. The log should say
   `source build (its mcm2.map matches); hooks on`; if it says the build is
   unknown, add `MCM2_FORCE_RUNTIME=1` and report it.
   `autorace.bin` already exists there, so the saved event loads on its own.
   Accept the EULA and the hardware-test box.
3. Run `python tools/record_run.py`, then read `mcm2_runtime.log`,
   `autorace.log`, `errorlog.txt`, `frames/*.bmp`.

| Outcome | Meaning | Next |
|---|---|---|
| Title, loading bar, race with rider, terrain, HUD; menu after 120 s; clean exit. | Current build matches the 11-05 result. | Items 10, 11, 19. |
| Process ends about 1 s after `DirectDrawCreateEx`, no error log. | Session or display mode, as retail under RDP. | Check the session; run the retail demo in `work/run/demo` the same way. |
| "Installation error. Please reinstall". | Registry values missing. | See `runtime/README.md`, start-up requirement 3. |
| Exit right after `SetDisplayMode`. | `CreateModeSurfaces` or the display profile. | Item 3; check `DriverInfo` values. |
| Access violation in `errorlog.txt`. | Map the EIP through the copied `mcm2.map`; note the function and whether it is bring-up. | Emu case for that function. |
| Stays on the loading screen or backdrop remains behind the HUD. | Load did not finish or no colour clear. | Turn the sky cube off or force `clearTarget`; see "Loading dialog close path" in `GAME_BUILD.md`. |
| Race runs but geometry, textures or lighting look wrong. | Render stand-ins (blocker 4). | Same event on retail; items 7-10. |
| Rider does not move or AI stands still by hand. | Input acquire, or KrustyBike and AI bodies. | Compare with retail; item 11. |

### Comparison runs (items 10 and 11)

Run these after the run above has written `work/run/srcbuild/autorace.bin`.
Each run uses the same variables and the same saved event:

    MCM2_AUTORACE=1 MCM2_AUTORACE_PORT=1 MCM2_AUTORACE_EXIT=60
    MCM2_FIXED_CLOCK=16 MCM2_STATE_LOG=1 MCM2_AUTO_EULA=1

1. Stage retail with `python runtime/build.py --stage work/run/demo` and copy
   `work/run/srcbuild/autorace.bin` into `work/run/demo`.
2. Run retail twice. After each run, copy `statelog.txt` aside (`retail1.txt`,
   `retail2.txt`) and run
   `python tools/compare_state_logs.py retail1.txt retail2.txt`.
   "identical race frames" means the fixed clock makes a run repeatable.
   Otherwise the first differing frame shows where something else varies
   (another thread, input or a timer the clock does not cover). Report it
   before comparing builds.
3. Run the source build in `work/run/srcbuild` with the same variables, then
   `python tools/compare_state_logs.py retail1.txt work/run/srcbuild/statelog.txt`.
   The first differing field (racer, quantity, frame) is where the
   reconstruction's physics or AI leaves retail.

Then, without `MCM2_AUTORACE`: start Voodoo Basin and Donner Pass from the
menus, ride a full lap, bail out once, finish, and leave through the podium.
Note the first screen that differs from retail.

The `ui\*.dtm` and `HT2_WireSeg*.col` open failures in the log are normal;
the retail log shows the same.
