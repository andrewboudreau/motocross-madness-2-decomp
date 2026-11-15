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
| Imports equal retail (244 functions, 15 DLLs), except three bound by ordinal. | C | `blade.dll` #3, #4 and `DINPUT.dll` #2 (`config/game-imports/*.def`). The ordinals match the staged `blade.dll` and this machine's `SysWOW64\dinput.dll`. |
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
table is emulated by `tools/static_initializers.py` except one initializer
that calls `GetVersionExA` (retail 0x4aa7b0).

## Boot path

Score is the masked linked score from `linked_accuracy.py`. "Emu" means a
`tools/emu_cases` differential case exists.

| Step | Retail VA | State | Notes |
|---|---|---|---|
| CRT entry, static initializers | 0x5380a5 | library; partly emulated | One initializer not emulated. |
| `WinMain`, `WinMainBody`, window, `MainWindowProc` | 0x4a0bc0, 0x4a0680, 0x4a0c50, 0x4a0d50 | exact | Icon load returns NULL without `.rsrc` (cosmetic). |
| `GetDXVersion` | 0x448560 | near miss 0.914 | 720-scenario native differential test, no emu case. |
| `FindDataDirectory` | 0x523c90 | exact | Needs the registry values the shim writes. |
| `PCGame::StartUp` | 0x4bfc50 | exact | |
| Enumerate, choose display, joystick, load profile | 0x4c9600, 0x4ccd60, 0x4cd610, 0x4c16f0 | exact | |
| `ProfileDisplays` | 0x4c0d10 | near miss 0.998 | No emu case. |
| `CreateModeSurfaces` | 0x4ca520 | near miss 0.774 (116 B) | No emu case. Retail quits here when video memory reads negative. |
| `ProbePartialTextureUploads`, `ProbeNonLocalTextureMemory` | 0x4cab00, 0x4ca5a0 | near miss 0.826 | No emu case. Runs in the hardware test on a fresh profile. |
| `Game` constructor, init, slot 2, render | 0x467990, 0x467b70, 0x467af0, 0x467eb0 | exact | |
| `Game::Tick` | 0x4685d0 | near miss 0.749 | `QueryPerformanceCounter` in place of inline `rdtsc`. Emu: 60 runs, 0 mismatches. |
| `ResourceManager::AddArchive`, `FileStream::Read` | 0x4e9030, 0x461640 | near miss (0.931) | No emu case. Every asset goes through these. |
| `KrustyUI::Create`, `UIDialog::Tick` | 0x4988a0, 0x46ef00 | exact | |
| `ReadDialogResource` | 0x46a920 | bring-up 0.967 (16,120 B) | Every menu screen. Emu: 0 mismatches. |
| `MainDlg::HandleEvent` | 0x44b200 | bring-up 0.992 | No emu case. |
| `SPRaceInfoDlg::HandleEvent`, rider dialogs | 0x4507a0, 0x4d72c0, 0x4f8820, 0x44fae0 | bring-up 0.966-0.994 | Emu for 0x4507a0 only. |
| `LeaveMenusForRace`, `LoadingDlg::HandleEvent` | 0x4536e0, 0x451020 | exact | |
| `EventManager::LoadEvent` | 0x45cb70 | exact | |
| `LoadEventObjects` | 0x4de590 | bring-up 0.985 | Emu: 0 mismatches. |
| `Terrain::Load` | 0x5059d0 | near miss 0.985 | Emu: 0 mismatches, but 42 of 60 retail runs took the fail path. |
| `Terrain::PrepareGeometry` | 0x507610 | bring-up, unscored (unbound) | Has an emu case. |
| `BaseQuarryEvent::PrepareGeometry` | 0x4e14a0 | near miss | No emu case. |
| `EcoSystem::GenerateObjects` | 0x459ce0 | bring-up 0.914 | Uses a private `NormalDistribution(100)` in place of the retail shared one. |
| `BikeRace::Create` | 0x417ed0 | bring-up 0.998 | Emu: 0 mismatches. |
| `BikeRace::CreateRacers` | 0x419970 | bring-up 0.999 (13,428 B) | No emu case. |
| `LoadBike` | 0x4079c0 | bring-up 0.987 | Emu: 0 mismatches. |
| `EventManager::Tick`, `BikeRace::Tick`, `KrustyBike::Tick` | 0x45f200, 0x41d2b0, 0x4977a0 | exact | |
| `KrustyBike::SimulateFrame`, `Vehicle::SimulateFrame` | 0x496790, 0x52a940 | near miss 0.999, 0.998 | |
| Bike AI | 0x40eca0, 0x414370, 0x517340 | bring-up 0.957, 0.951, 0.842 | 34,965 B unit. |
| `SceneTick` | 0x4eab00 | bring-up 0.941 | |
| Visibility walk, `TestBox` | 0x481de0, 0x52f570 | near miss 0.733, bring-up 0.796 | Decide what is drawn. |
| `LightVertices`, `TransformPoints` | 0x49e4a0, 0x4a1b00 | bring-up 0.754, 0.336 | Low scores come mostly from inline-asm retail bodies. |
| `DrawCurrentLod` | 0x440f30 | bring-up 0.901 (7,851 B) | Reads a local retail leaves uninitialised; source sets it to 0. |
| Particles, projected shadow | 0x4bac60, 0x4db8c0, 0x4dbd30, 0x4dc2b0 | bring-up 0.712; near miss 0.738-0.837 | |
| Texture creation and copies | 0x4c69c0, 0x4c7b40, 0x486170, 0x50c960, 0x50dad0 | near miss 0.867-0.957 | No emu cases. |
| Pixel converters | 0x4d0d40, 0x4cea10, 0x4ce5f0 | bring-up 0.418-0.715 | 8-bit and half-size texture paths. |
| HUD, radar, scoreboard | 0x51c720, 0x48d1e0 | bring-up 0.867-0.973 | |
| `PCRenderTarget::Clear`, `AdvanceFrame`, `RenderPost3D` | 0x4c5510, 0x4e8cc0, 0x46f320 | exact | |
| Podium, last-race dialog, replay | | bring-up 0.967-0.986 | Never exercised live. |
| Multiplayer dialogs and session | | bring-up 0.964-0.997 | Never exercised live; lobby is stubbed by the shim. |

No step is missing. Bring-up units cover 129 scored functions (223,846 B, of
which 33,566 B are identical). 117 non-identical functions (135,862 B) come
from reviewed near-miss sources outside `src/bringup`.

## Blockers and risks, ranked

1. **The current exe has never been run** (C). The only live proof is ten
   days and about 560 commits old. Until it runs, every claim about booting
   is inference.
2. **Bring-up bodies on the load and race path are unreviewed
   approximations** (C for the scores, P for behaviour). The large ones:
   `ReadDialogResource`, `LoadEventObjects`, `CreateRacers`, `LoadBike`, the
   KrustyBike and bike-AI units, `DrawCurrentLod`. 19 candidate sources are
   marked best-guess-unreviewed in `config/game_build.json`.
3. **Start-up and texture near misses have no differential case** (C):
   `CreateModeSurfaces`, the two texture probes, `PCTextureMap::CreateSurface`,
   `CopyRectTo`, `CopyScreenToTexture`, `FileStream::Read`, `AddArchive`,
   `ProfileDisplays`. Of 189 non-identical bodies, 34 (53,289 B) have none.
4. **Low-score render bodies** (C for scores): pixel halvers, `TransformPoints`,
   `LightVertices`, `TestBox`, the visibility walk, particles, projected
   shadow. A wrong result here shows as missing or miscoloured geometry, not
   as a crash. The 11-05 frames look right for one camera on one track (S).
5. **Environment** (C): the shim accepts only known exe hashes
   (`MCM2_FORCE_RUNTIME=1` for a source build); the game exits within a
   second under RDP or a disconnected session, as retail does; the EULA must
   be accepted by the user; registry values must exist; `dinput.dll` is bound
   by ordinal, which holds on this machine only.
6. **Placeholder globals and guessed initial values** (C that they exist, P
   for effect): `g_visibilityClipper` is an empty static; the grid texture
   tables are capped at 5,000 entries; the visibility box and entry arrays
   are separate where retail overlaps them; the KrustyBike stall thresholds
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
| 2 | Keep a record of the last exe hash and commit that ran, with its frames, in the ignored run directory; cite it from `GAME_BUILD.md`. | 30 min | Stops the run status drifting from the tree. |
| 3 | Emu cases for `CreateModeSurfaces`, `ProfileDisplays` and the two texture probes, with fake DirectDraw vtables in replay mode. | 1 day | Start-up bodies whose failure ends the process silently. |
| 4 | Emu cases for `FileStream::Read` and `ResourceManager::AddArchive`. | 0.5 day | Every asset load depends on them. |
| 5 | Emu case for `BikeRace::CreateRacers`. | 1 day | Largest uncovered bring-up body on the race path. |
| 6 | Strengthen `terrain_load` so most runs take the success path. | 0.5 day | The case currently passes mostly by failing early in both builds. |
| 7 | Emu cases for `PCTextureMap::CreateSurface`, `CopyRectTo`, `CopyScreenToTexture`, `Clone`, `CopyMipLevels`. | 1-2 days | Texture corruption is hard to spot by eye. |
| 8 | Emu cases for the pixel converters (0x4d0d40, 0x4cea10, 0x4ce5f0) over full input ranges. | 0.5 day | Pure functions with the lowest scores; cheap and exhaustive. |
| 9 | Emu or native differential for `TransformPoints`, `LightVertices` and `TestBox`. | 1 day | Decide whether low scores hide behaviour differences. |
| 10 | Frame comparison: the same `autorace.bin` on retail and source build, compare frames at fixed times. | 1 day, needs user runs | First whole-program check of rendering and physics. |
| 11 | Log a per-frame state hash (rider position, speed, lap time) from `AutoRaceTick` in both builds. | 1 day | Cheaper and stricter than images for physics and AI. |
| 12 | Re-run the old crash scenarios and map any new EIP. | 1 h of user time | Closes blocker 7. |
| 13 | Replace the placeholder globals in `Group1_Globals.cpp` and `Group4_EcoBridges.cpp` with retail-shaped data. | 1-2 days | Removes known layout differences. |
| 14 | Review the 19 best-guess candidate sources, largest on the race path first. | 3-5 days | Each is a body nobody has checked against retail behaviour. |
| 15 | Add `GetVersionExA` to `static_initializers.py`. | 2 h | Completes the static-initializer check. |
| 16 | Link a `.rsrc` with user-supplied icons and a version block. | 0.5 day | Icon and file properties; no data may be committed. |
| 17 | Import `DirectInputCreateEx` by name if a matching import library can be built. | 0.5 day | Removes a machine-specific ordinal. |
| 18 | Add the source-build hash route to the shim (accept an exe that has `mcm2.map` beside it). | 2 h | Drops `MCM2_FORCE_RUNTIME` from every run. |
| 19 | Exercise podium, replay and return-to-menu with `MCM2_AUTORACE_EXIT`, then a full lap by hand. | 30 min of user time | These paths have never run in a source build. |
| 20 | Split whole-program emulation off as out of scope; note it here. | none | `blade.dll`, DirectDraw and COM make a unicorn boot impractical; per-function replay cases give the coverage. |

## Runtime test plan (the user runs these)

Preconditions: `query user` shows an Active console session, not RDP. Game
closed. Staged directory `work/run/srcbuild` holds the demo data and the
runtime DLLs.

1. Copy `work/game-build/build-dms49u5e/mcm2.exe` and `mcm2.map` into
   `work/run/srcbuild`. Move the old `frames`, `errorlog.txt` and
   `mcm2_runtime.log` aside.
2. Run with `MCM2_FORCE_RUNTIME=1 MCM2_AUTORACE=1 MCM2_AUTORACE_EXIT=120`.
   `autorace.bin` already exists there, so the saved event loads on its own.
   Accept the EULA and the hardware-test box.
3. Read `mcm2_runtime.log`, `autorace.log`, `errorlog.txt`, `frames/*.bmp`.

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

Then, without `MCM2_AUTORACE`: start Voodoo Basin and Donner Pass from the
menus, ride a full lap, bail out once, finish, and leave through the podium.
Note the first screen that differs from retail.

The `ui\*.dtm` and `HT2_WireSeg*.col` open failures in the log are normal;
the retail log shows the same.
