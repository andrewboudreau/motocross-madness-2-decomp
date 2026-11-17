# Original units

The goal is one source file per retail object, named as retail names it
(`racesnd.cpp`, `dirlist.cpp`, `FollowCam.cpp`, ...), with its functions in
retail order. Functions that are not exact live in their unit too, each with a
status comment, instead of in `samples/` or `src/bringup/Group*` files.

## Evidence

- **Confirmed:** retail `__FILE__` strings (`D:\aardvark\VC\krusty2\<unit>.cpp`)
  and the code that loads them (`analysis/source_xrefs.json`). A function that
  loads a `.cpp` path was compiled in that unit; `.h` paths come from inline
  functions and prove nothing about the unit.
- **Strong inference:** LINK lays each object's code out contiguously, so a
  function between two anchors of the same unit belongs to it. Pick-any COMDATs
  (`??_G`/`??_E` destructors, templates, inline methods) are kept by whichever
  object the linker saw first and are never used as anchors.
- **Provisional (`class`):** a method in the gap between two different
  units goes to the neighbour that already holds methods of its class (when
  exactly one does). Classes split across files are common, so check.
- **Provisional (`run`):** a run of functions that the same repo object defines next
  to an attributed function (`run` evidence). Repo objects are not original
  units, so check these before moving code.

Retail order is consistent with alphabetical linking of the original names,
which is also how `tools/build_game.py` orders units.

## Tools

```bash
# Source build, then per-function accuracy (work/ is ignored).
python tools/build_game.py -j 8
python tools/linked_accuracy.py --build work/game-build/build-XXXX --out work/la-after

# Which original unit each function belongs to, and what is defined elsewhere.
python tools/tu_ownership.py --report work/la-after/report.json --out work/tu/report.md --json work/tu/ev.json

# A move must leave every score unchanged: exit status 1 on any loss.
python tools/accuracy_diff.py work/la-before/report.json work/la-after/report.json
```

[UNIT_LAYOUT.md](UNIT_LAYOUT.md) checks a whole unit's byte layout against
its retail range. `tools/unit_source.py` extracts definitions from a unit into a private
validation source (the unit's includes plus the named definitions). Native
behaviour tests use it to link one function of a unit against retail.

## Moving a function into its unit

1. Capture a `linked_accuracy` report of the current tree.
2. Move the definition to its retail position in the unit; merge its bindings.
   Precede a nonexact body with a `// Not exact (...)` comment that gives the
   matching positions and what differs.
3. Repoint calibration cases, emulator cases (`SOURCE`), tests, fixtures and
   `config/game_build.json` (remove the `candidate_sources` entry). Search for
   the bare stem too (`'RaceSound'`): many tests build paths from names.
   Rekey `__FILE__:<unit>` bindings, and repin object-hash fixtures whose
   only change is the `??_C@` name of the `__FILE__` string (section and
   relocation counts must stay the same).
4. Check that every calibration case of the unit stays exact: moving code can
   change VC6 codegen in unrelated functions of the same unit.
5. Rebuild, rerun `linked_accuracy` and require `accuracy_diff.py` to report
   0 regressions; run the affected tests.

## Status

| Unit | File | Notes |
|---|---|---|
| `racesnd.cpp` | `src/reconstructed/racesnd.cpp` | All 16 retail functions; `RefillStream` not exact. `0x004e57d0` is still also reached through the `Group5_KbRaceHandler` bring-up wrapper (the handler's class is unproven: its other method `0x004de580` lies in QuarryStuntEvent.cpp's span). |
| `cursor.cpp` | `src/reconstructed/cursor.cpp` | Renamed from `GameCursor.cpp`; 8 calibration cases exact. |
| `dirlist.cpp` | `src/reconstructed/dirlist.cpp` | Renamed from `DirectoryList.cpp`; 27 cases exact. |
| `gameobj.cpp` | `src/reconstructed/gameobj.cpp` | Renamed from `GameObject.cpp` (the class header stays `GameObject.h`); 43 cases exact. |
| `Texmap.cpp` | `src/reconstructed/Texmap.cpp` | Renamed from `TextureMap.cpp`; 13 cases exact. |
| `Track.cpp` | `src/reconstructed/Track.cpp` | `D:\aardvark\VC\krusty2\Track.cpp` (`0x0057508c`). PlaceNear `0x00517340` moved in from its sample, in retail order; not exact. |
| `trkgame.cpp` | `src/reconstructed/trkgame.cpp` | Renamed from `TrackGame.cpp`; 19 cases exact. |
| `uiinfo.cpp` | `src/reconstructed/UiInfo.cpp` | `D:\aardvark\VC\krusty2\uiinfo.cpp` (`0x00575780`). ResetDefaults `0x00522440` and FindDataFile `0x005238f0` moved in from their sample, in retail order; not exact. 35 cases exact. The file keeps its class-era spelling until the rename pass. |
| `wrecker.cpp` | `src/reconstructed/wrecker.cpp` | All 30 retail functions in order (renamed from `Wrecker.cpp`); `SprayContactParticles` and `PushProbesOutOfBox` moved in from `samples/race/WreckerNearMisses.cpp` and not exact. Both agree with retail under `tools/emu_diff.py` (400 runs each). |
| `ConstraintMethodCollisionModel.cpp` | `src/krusty2/constraint/ConstraintMethodCollisionModel.cpp` | Retail order from the contact callback `0x0043b800`; 19 targets exact. `ApplyContactImpulse` moved in from its sample and is not exact (2515/2619); its forms tried are in the directory README. |
| `RaceStatus.cpp` | `src/reconstructed/RaceStatus.cpp` | `BuildStatusList` (it loads this unit's `__FILE__`) moved in from its sample and a bring-up copy; not exact. 21 cases exact. |
| unnamed (`0x00497cb0..0x004987ec`) | `src/reconstructed/KrustyBikeCamera.cpp` | No `__FILE__` anchor; the file is named for its class (provisional). `Tick` moved in from its sample and a bring-up copy and now passes the unit's own y axis; not exact. 24 cases exact. |
| `SelectGamePicProcs.cpp` | `src/reconstructed/SelectGamePicProcs.cpp` | `SendStartMessage`, `FillBikeRiderLists` and MPBikeRiderDlg `Tick` moved in from their sample and two bring-up copies, in retail order; not exact. MPBikeRiderDlg `ApplyChosenRider` joins the folded bodies at the end. 55 cases exact. |
| `CarProcedural.cpp` | `src/reconstructed/CarProcedural.cpp` | Slot 10 `Tick` and `PositionAtTime` moved in from their sample and a bring-up copy, in retail order; not exact. `Tick` calls the exact `OrientationAxesFromAngles` (OrientationAxes.cpp) instead of the bring-up's private copy. 16 cases exact. |
| `MorphBastardModifier.cpp` | `src/reconstructed/MorphBastardModifier.cpp` | `LoadParameterFile`, `ControllerAngles` and slot 27 `ModifyMesh` moved in from their sample and two bring-up copies, in retail order; not exact. KrustyBike.cpp's `KbMorphMod` view links to the loader through a link alias. 15 cases exact. |
| `EcoSystem.cpp` | `src/reconstructed/EcoSystem.cpp` | `D:\aardvark\VC\krusty2\EcoSystem.cpp`. Six near misses (`TestDistance`, `ReadEst`, `BuildCollisionObjects`, `WriteListing`, `GenerateObjects`, `Render3D`) moved in from `samples/ecosystem/EcoSystemNearMisses.cpp` and the Group4/Group5 bring-ups, in retail order; not exact. Render3D now reads the unit's own draw toggle and counters (the bring-up kept private copies). 46 cases exact; `0x004570a0` stays in `samples/ecosystem/VegetationCollisionObject.cpp`. |
| `TextureCache.cpp` | `src/reconstructed/ManagedTextureGroup.cpp` | `D:\aardvark\VC\krusty2\TextureCache.cpp` (xref `0x0050c4e2`); the file keeps its class name until the rest of the unit (CacheTexture, ManagedTexture) is gathered. `RepackWholePages` and `RepackWithPartialBlits` moved in from their sample and a bring-up copy, in retail order; not exact. The bring-up's `UnknownTimer` pair duplicated PeakHold's exact `Peak`/`Record` and is now linked to them by alias. 15 cases exact. CacheTexture.cpp (the same unit) gained `CopyMipLevels` `0x005102d0` from its sample, at the end in retail order; not exact. |
| `Tgafile.cpp` | `src/reconstructed/Tgafile.cpp` | `D:\aardvark\VC\krusty2\Tgafile.cpp`. `WriteTga` `0x00512990` moved in from its sample, in retail order; not exact. 22 cases exact. |
| unnamed (`0x004a10e0..0x004a1d10`) | `src/reconstructed/MatrixUtil.cpp` | No `__FILE__` anchor; the file is named for its contents (provisional). `ProjectPoints` `0x004a1a50` moved in from its sample, in retail order; not exact. The shadow bring-up's copy (`ShadowTransformPointsProjective`) is replaced by a link alias to it. 18 cases exact. |
| `BackgroundImage.cpp` | `src/reconstructed/BackgroundImage.cpp` | `D:\aardvark\VC\krusty2\BackgroundImage.cpp`. `GetDc` `0x004049d0` (between `0x00404700` and `0x00404c80`) and its two scratch globals moved in from their sample; not exact. 21 cases exact. |
| `PCInputDeviceType.cpp` | `src/reconstructed/PCJoystickDevice.cpp` | `D:\aardvark\VC\krusty2\PCInputDeviceType.cpp` (the readers' DirectInput error reports). `ReadBufferedState` `0x004c3100` and `ReadImmediateState` `0x004c3790` moved in from their samples after `ApplyAxisDeadZone`, in retail order; not exact. 29 cases exact. The keyboard and mouse halves of the unit stay in their class files for now. |
| unnamed (`0x004238c0..0x00423f6d`) | `src/reconstructed/GR_BitString.cpp` | No `__FILE__` anchor; bracketed by bikerace.cpp and BlockAllocator.cpp; the file is named for its class (provisional). The sub-word shifts `0x004239c0`, `0x00423b70` and `ReadBits` `0x00423ef0` moved in from their samples, in retail order; not exact. Every function of the range is here (11 written, plus the deleting destructor VC6 generates); 9 cases exact. |
| unnamed (Camera) | `src/reconstructed/Camera.cpp` | No `__FILE__` anchor; the file is named for its class (provisional). Slot 28 `UpdateMatrices` `0x0042ee30` moved in from its sample, between `0x0042eb10` and `0x0042f090`; not exact (487/492). 32 cases exact. |
| `overlay.cpp` | `src/reconstructed/Overlay.cpp` | `D:\aardvark\VC\krusty2\overlay.cpp`. `RestoreRect` `0x004b6710` moved in from its sample, in retail order; not exact (105/365). 10 cases exact. The file keeps its class spelling for now. |
| `PCGame.cpp` | `src/reconstructed/PCGame.cpp` | `D:\aardvark\VC\krusty2\PCGame.cpp`. ProfileDisplays `0x004c0d10` moved in from its sample, after `0x004c0c60` in retail order; not exact. 36 cases exact. |
| `PCTexMap.cpp` | `src/reconstructed/PCTextureMap.cpp` | `D:\aardvark\VC\krusty2\PCTexMap.cpp` (source xref). CreateSurface `0x004c69c0`, Clone `0x004c71c0` and CopyRectTo `0x004c7b40` moved in from their samples in retail order; not exact. 27 cases, the rest exact. File name keeps the class spelling for now. |
| `PCVideoCard.cpp` | `src/reconstructed/PCVideoCard.cpp` | `D:\aardvark\VC\krusty2\PCVideoCard.cpp`. CreateModeSurfaces `0x004ca520`, ProbeNonLocalTextureMemory `0x004ca5a0`, ProbePartialTextureUploads `0x004cab00` and MeanTextureChannelDifference `0x004cb330` moved in from their samples, in retail order; not exact. 23 cases exact. |
| `Net.cpp` | `src/reconstructed/Net.cpp` | `D:\aardvark\VC\krusty2\Net.cpp`. DispatchMessages `0x004aced0` moved in from its sample, in retail order, with its `MessageAt` helper; not exact. 86 cases exact. |
| `NetThread.cpp` | `src/reconstructed/NetThread.cpp` | `D:\aardvark\VC\krusty2\NetThread.cpp`. ReceiveMessages `0x004af8e0` moved in from its sample, in retail order; the buffer-size static `0x006886dc` is `s_ReceiveBufferSize`. Not exact. |
| `PCAudio.cpp` | `src/reconstructed/PCAudio.cpp` | `D:\aardvark\VC\krusty2\PCAudio.cpp`. The loader thread `0x004bdc00` and `SetPrimaryFormat` `0x004be910` moved in from their samples, in retail order; not exact. Every function of `0x004bb630..0x004bed40` is here; 76 cases exact. |
| `GUIManager.cpp` | `src/reconstructed/GUIManager.cpp` | `D:\aardvark\VC\krusty2\GUIManager.cpp`. CopyScreenToTexture `0x00486170` and SetPointerDevice `0x00488120` moved in from their samples, in retail order; not exact. 102 cases exact. |
| `ResourceManager.cpp` | `src/reconstructed/ResourceManager.cpp` | `D:\aardvark\VC\krusty2\ResourceManager.cpp` (`__FILE__` `0x00572b64`). AddArchive `0x004e9030` moved in from its sample, in retail order; not exact. |
| `recorder.cpp` | `src/reconstructed/Recorder.cpp` | `D:\aardvark\VC\krusty2\recorder.cpp`. The worker thread `0x004e6f80` (first in the unit) and slot 10 `Tick` `0x004e7d60` moved in from their samples, in retail order; not exact. 12 cases exact. The file keeps its old spelling for now. |
