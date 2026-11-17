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
| `trkgame.cpp` | `src/reconstructed/trkgame.cpp` | Renamed from `TrackGame.cpp`; 19 cases exact. |
| `wrecker.cpp` | `src/reconstructed/wrecker.cpp` | All 30 retail functions in order (renamed from `Wrecker.cpp`); `SprayContactParticles` and `PushProbesOutOfBox` moved in from `samples/race/WreckerNearMisses.cpp` and not exact. Both agree with retail under `tools/emu_diff.py` (400 runs each). |
| `ConstraintMethodCollisionModel.cpp` | `src/krusty2/constraint/ConstraintMethodCollisionModel.cpp` | Retail order from the contact callback `0x0043b800`; 19 targets exact. `ApplyContactImpulse` moved in from its sample and is not exact (2515/2619); its forms tried are in the directory README. |
| `RaceStatus.cpp` | `src/reconstructed/RaceStatus.cpp` | `BuildStatusList` (it loads this unit's `__FILE__`) moved in from its sample and a bring-up copy; not exact. 21 cases exact. |
| unnamed (`0x00497cb0..0x004987ec`) | `src/reconstructed/KrustyBikeCamera.cpp` | No `__FILE__` anchor; the file is named for its class (provisional). `Tick` moved in from its sample and a bring-up copy and now passes the unit's own y axis; not exact. 24 cases exact. |
