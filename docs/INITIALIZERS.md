# Dynamic initializers (`$E`) and unit attribution

This page lists every registered `$E` case and the unit it belongs to. That
covers 450 cases: `_$E*` symbols in `tools/run_calibration.py`, and `$E`
entries in the `targets.json` files under `src/krusty2` and `samples/physics`.
Most of them are the per-file vector sets. Each set is four thunk/body pairs
that build (0,0,0), (1,0,0), (0,1,0) and (0,0,1) into four 12-byte `.bss`
globals. The rest initialize a unit's own statics.

## Evidence, strongest first

1. **Readers.** The vectors are file statics, so whatever code reads them is in
   the same unit. To find the readers, disassemble all of `.text` linearly and
   collect every absolute memory operand and immediate inside each vector's
   12 bytes. Leave out the `$E` bodies themselves.
2. **`.CRT$XCU` order.** The table is at `0x00566004..0x00566580` and has 352
   entries. It follows link order, and one unit's entries are contiguous.
   Within a unit, the order is source order. A shared header's vectors come
   before the unit's own statics, except when an own static is declared earlier
   (Spheres, see below). An entry that follows another unit's set, with no
   gap, belongs to that set's unit.
3. **`.bss` block.** File statics are laid out in link order. If a set's
   vectors are interleaved with statics that only one unit reads, the set is
   that unit's.
4. **Position.** A set usually closes its unit (CUBE.md, PROCIRCUIT.md,
   RACESTATUS.md). It can also sit mid-file: racesnd, bikerace, dlgprocs,
   BoundingBoxTreeBuild, gameui and SceneManager. OptionProcs shows that a set
   can also open a unit, because its vectors are interleaved with OptionProcs
   statics. So position alone decides nothing.

## Corrections made

| Set | Was | Now | Evidence |
|---|---|---|---|
| `0x004dc4d0..0x004dc60b` (vectors `0x00689b38..0x00689b74`) | `src/krusty2/broadphase/Quadtree.cpp` | `samples/physics/shadow/ProjectedShadow.cpp` | Only the ProjectedShadow constructor reads the zero vector `0x00689b48` (`0x004da591`, `0x004da65f`, 12 operands); no Quadtree code reads any of the four. The set follows ProjectedShadow's last function (slot 13 `0x004dc410`, then the shared stub `0x004dc4c0`). `.CRT$XCU` lists it after ProCircuitProcs.cpp's set. |
| `0x005089a0..0x00508adc` (vectors `0x00689fe8..0x0068a064`) | `samples/physics/shadow/TerrainShadow.cpp` | `src/krusty2/broadphase/Terrain.cpp` | `.CRT$XCU` lists it (300-303) right before Terrain.cpp's own camera and timer initializers at `0x00505480..0x005055e0` (304-314), so they are one unit. The vectors are interleaved in `.bss` with those timers (`0x0068a008`, `0x0068a018`, ...), and Terrain code reads the y axis `0x0068a058` (`0x00507f15`, `0x00508109`, `0x0050843c`, `0x0050868e`). This replaces the former `g_terrainRefDir`. |
| `0x004f1740..0x004f1793` (player records `0x00689d08`) | `src/reconstructed/SceneManager.cpp` | `src/reconstructed/SelectGamePicProcs.cpp` | `.CRT$XCU` entry 270 comes right after SelectGamePicProcs.cpp's set (266-269), not after SceneManager.cpp's (262-265). The array lies inside that unit's vector `.bss` block, and only SelectGamePicProcs code reads it. The initializers sit at the top of the unit, before its first `__FILE__` user `0x004f17a0`. |

All 28 moved cases remain exact. The 16 physics cases were masked matches without
bindings. With the new `ProjectedShadow.bindings.json` and
`broadphase/Terrain.bindings.json` they are now strictly exact: the physics run
went from 414 to 430 strict out of 855. The calibration run stays at 2249 of 2257.

## Audit table

Columns: the `.CRT$XCU` indices, the `$E` text range, the lowest vector or
static address written, the units whose code reads what the set writes, the
unit the set is attributed to (after the fixes), and the verdict.
Unregistered `.CRT$XCU` entries are not listed.

| XCU | `$E` text | Data | Readers | Unit | Verdict |
|---|---|---|---|---|---|
| 0 | 401950-4019cc | 5776d0 | none | Arrow.cpp | own static at the top of the unit |
| 1-5 | 402050-403d4b | 5776d8 | AuralScape | AuralScape.cpp | consistent |
| 18-27 | 40d070-414360 | 5779f0 | BikeAI | BikeAI.cpp | consistent; vectors listed before own statics |
| 32-35 | 41cdf0-41cf2b | 578e50 | BikeRace | BikeRace.cpp | consistent (mid-file) |
| 45-48 | 42d250-42d38b | 579670 | BoundingBoxTreeBuild | BoundingBoxTreeBuild.cpp | consistent (mid-file) |
| 49-52 | 42f250-42f38b | 5796b0 | none | Camera.cpp | consistent by position (after Camera code) |
| 53-56 | 430eb0-430feb | 5796f0 | CarProcedural | CarProcedural.cpp | consistent |
| 66 | 43c8e0-43c92b | 579830 | ConstraintMethodCollisionModel | ConstraintMethodCollisionModel.cpp | consistent |
| 70-73 | 43d750-43d88b | 579860 | cube | Cube.cpp | consistent |
| 74-81 | 43d890-43e9ab | 5798a8 | none | CubeDraw.cpp | consistent (CUBE.md) |
| 86-89 | 4466e0-44681b | 57ef58 | none | D3DIMSoultreeMotnctrl.cpp | consistent by position |
| 90-93 | 4477d0-44790b | 57ef98 | none | D3DIMSoultreeShadow.cpp | consistent; own static `0x00446820` is listed after, at 94 |
| 95-98 | 453d50-453e8b | 59adb0 | dlgprocs | DlgProcs.cpp | consistent (mid-file) |
| 117 | 467100-46712b | 65b478 | FontTexture, Game, TextService | FontTexture.cpp | consistent (shared global) |
| 118-127 | 467850-46798f | 65b490.. | Game | Game.cpp | consistent |
| 128-131 | 46e870-46ea5b | 65b550 | none | GameUi.cpp | consistent (mid-file) |
| 136-141 | 47dda0-4808fb | 6754b0 | Griddraw, Terrain (global grid size) | Griddraw.cpp | consistent |
| 142-146 | 484dd0-48836b | 67b428 | none | GUIManager.cpp | ambiguous position; left as is |
| 167-170 | 49dd30-49de6b | 67c4a8 | none | LightEmitter.cpp | ambiguous: opens LightEmitter.cpp or closes the code at `0x0049bf10..` |
| 171-174 | 4a01d0-4a030b | 67c4e8 | none | Lzw.cpp | ambiguous: opens Lzw.cpp or closes LightEmitter.cpp |
| 184-187 | 4a54f0-4a562b | 6850e0 | MorphBastardModifier | MorphBastardModifier.cpp | consistent |
| 192 | 4a5630-4a56a7 | 68512c | Motnctrl | Motnctrl.cpp | consistent; follows the Motnctrl set at 188-191 |
| 193-197 | 4aa010-4aa7db | 685160 | code at `0x004aa29e` (after the set) | MSZoneInterface.cpp | consistent; opens the unit |
| 198-201 | 4aab00-4aac3b | 688670 | none | Net.cpp | ambiguous: opens Net.cpp or closes NationalRace.cpp |
| 202 (bodies) | 4aff90-4affcb | 6886e8 | none | NormalDistribution.cpp | consistent |
| 203 | 4b00e0-4b015c | 6886f0 | none | Nulls.cpp | own static at the top of the unit |
| 204-207 | 4b07b0-4b08eb | 6886f8 | ObjectPicker | ObjectPicker.cpp | consistent |
| 212-215 | 4b1ec0-4b1ffb | 688778 | none | OptionProcs.cpp | consistent: `.bss` is interleaved with OptionProcs statics |
| 216-219 | 4b9e00-4b9f3b | 689118 | none | samples EmitterVec3Constants.cpp | ownerless placeholder; left as is |
| 232-235 | 4d3340-4d347b | 689a78 | GearRatios | GearRatios.cpp | consistent (PROCIRCUIT.md) |
| 236-239 | 4d49e0-4d4b1b | 689ab8 | none | ProCircuit.cpp | consistent (PROCIRCUIT.md) |
| 240-243 | 4da3e0-4da51b | 689af8 | ProCircuitProcs | ProCircuitProcs.cpp | consistent |
| 244-247 | 4dc4d0-4dc60b | 689b38 | ProjectedShadow | ProjectedShadow.cpp | **fixed** (was Quadtree.cpp) |
| 248-251 | 4e1b80-4e1cbb | 689b80 | none | QuarryStuntEvent.cpp | consistent by position |
| 252-255 | 4e48f0-4e4a2b | 689bc0 | none | RaceSound.cpp | consistent (mid-file) |
| 256-260 | 4e58a0-4e6f7b | 689c00 | none | RaceStatus.cpp | consistent (RACESTATUS.md) |
| 261 | 4e8d30-4e8d5b | 689c78 | none | ResourceManager.cpp | own static at the top of the unit |
| 262-265 | 4ecb70-4ecd5b | 689c98 | none | SceneManager.cpp | consistent (mid-file) |
| 266-270 | 4f1740-4f975b | 689cd8 | SelectGamePicProcs | SelectGamePicProcs.cpp | **fixed** (270 was SceneManager.cpp) |
| 271-274 | 4f9910-4f9a4b | 689e08 | none | SelectiveGravityModel.cpp | consistent by position |
| 283-286 | 502b50-502f0b | 689ed8 | SoulTreePhysics | SoulTreePhysics.cpp | consistent |
| 287-291 | 504940-504b5b | 689f1c | none | Spheres.cpp | consistent; own static 287 is listed before the vectors (declared earlier in source) |
| 292-299 | 504f70-5051eb | 689f60 | SteeringControl (first set only) | SteeringControl.cpp | first set consistent; second set (296-299) unread, could open SurfaceMap.cpp |
| 300-303 | 5089a0-508adb | 689fe8 | Terrain | Terrain.cpp | **fixed** (was TerrainShadow.cpp) |
| 305-314 | 5054c0-5055ff | 68a008.. | Terrain | TerrainSupport.cpp (Terrain.cpp slice) | consistent |
| 315-318 | 50a3a0-50a4db | 68a330 | none | TextureMap.cpp | ambiguous: opens Texmap.cpp or closes TerrainShadow's code |
| 323-326 | 51dba0-51dcdb | 68a400 | TrackOverlay | TrackOverlay.cpp | consistent |
| 327-331 | 521d00-521e6b | 68a450 | none | TypeRegistry.cpp | ambiguous: opens TypeRegistry.cpp or closes trkgame.cpp |
| 332-335 | 5278e0-527a1b | 68a6d8 | Vehicle | Vehicle.cpp | consistent |
| 348-351 | 532f00-53303b | 68acf0 | Wrecker | Wrecker.cpp | consistent |

## Remaining ambiguity

Six sets have no readers: GUIManager, LightEmitter, Lzw, Net, Texmap and
TypeRegistry. Each sits after the previous unit's last `__FILE__` user and
before the attributed unit's first. Their `.CRT$XCU` and `.bss` neighbours
fit either unit. They stay where they are. Moving one of them needs a reader
or a `.bss` interleave like OptionProcs's.

## Reproduction

```bash
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
python3 tools/run_physics_samples.py --strict --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

The audit itself was a linear capstone pass over `.text`, using
`mcm2tool.pe.PEImage`. It collected absolute operands in `.data`, and read the
`.CRT$XCU` table as the zero-delimited pointer array around the registered
`$E` stub addresses.

## Later additions

| `$E` text | Unit | Readers | Verdict |
|---|---|---|---|
| `0x004fee70..0x004fefab` (vectors `0x00689e88..0x00689ec0`) | soultree.cpp | none | consistent; closes the unit |
| `0x00429400..0x0042953b` + five empty pairs `0x00424690..0x0042472f` | BoundingBoxTreeQuery.cpp (unattested) | this unit | consistent; vectors listed first |
| `0x00466f10..0x004670fb` (XCU 113-116, vectors `0x0065b438..0x0065b468`) | FollowCam.cpp | FollowCam code only | consistent; closes the unit |
| `0x00442e00..0x00442f3b` (XCU 82-85) | D3DIMSoulTree.CPP | not traced | mid-file |
