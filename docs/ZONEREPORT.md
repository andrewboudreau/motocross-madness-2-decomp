# Zone reporting (Kr..Li unit)

Canonical reconstruction: `src/reconstructed/ZoneReport.cpp`. The gap is
`0x0049c2f0..0x0049dd30`, between KrustyVCR and LightEmitter.cpp. No
`__FILE__` literal or RTTI reaches it, so the file name is ours (tier 3).
These are methods of the TrackGame+0x3410 network object
(`UnknownTrackGameObject3410`, `TrackGame.h`). Its lobby COM calls live in
MSZoneInterface.cpp (`0x004aa350..`).

## Matched (4 of 5 starts, strict exact)

- `0x0049c2f0` (763 bytes, `ret 4`). Reads the lobby preset through
  `0x004aa360`, parses it with the krusty2 Parser and stores the tags
  `EventTypeIndex`, `EventTypeLocation`, `BikeManufacturer` and `BikeType`
  (minus one, or -1 when missing) at `+0x24c/+0x250/+0x298/+0x29c`.
  `*flags` gets bits 1/2/4/8. It then calls `0x0049ca60(1)` and copies the
  result into the race settings (`TrackGameMode+0x27f8`) and the player's
  bike choice. SelectGamePicProcs.cpp passes `TrackGameMode+0x1bd4`.
- `0x0049c5f0` (8 bytes) is the implicit `UnknownParser` destructor
  (`add ecx, 4; jmp ~UnknownParserList`). The unwind funclets
  `0x0054c2d0`/`0x0054c2f0` reach it, and the normal paths inline it.
- `0x0049c600` (355 bytes). Polls the lobby rank property (`0x004aa4e0`)
  for up to 20 s with `Sleep(1000)`. It reads the value keyed by the
  player's name as `"%d,%d,%d,%d"` and stores the first number at
  `TrackGameMode+0x1bd0`. It returns 0 on failure or a final -1 or -2.
- `0x0049c770` (739 bytes). EventManager `0x0045e550` calls it. It fills
  the 0x88-byte race status at `+0x2a4` and the player records at `+0x08`
  from `EventManager+0x50` entries, then sends them with `0x004aa670`. The
  record field names come from its debug labels (`FastestLap`,
  `TotalrunningTime`, `LargestSingleStuntPoints`).

Source forms that mattered:

- In `0x0049c2f0` the failure path is an explicit `xor eax, eax`. Writing
  the body as `if (ok) { ...; return 1; } return 0;` reproduces it. An
  early `if (!ok) return 0;` reuses the tested zero instead.
- In `0x0049c600` an `else return 0;` inside the loop keeps the single
  epilogue. An early `if (!ok) return 0;` duplicates it.
- In `0x0049c770` the induction register is the record base only when the
  loop indexes `field_0x2a4.field_0x08[i]` directly. A record pointer gives
  `record+8`.

Header changes: `UnknownParserList` gains its constructor (the shared
one-dword-zero body `0x004aae20`). `UnknownTrackGameObject3410Base` gains
the preset fields and `UnknownZoneRaceStatus`. The three methods return
`int`, so the SelectGamePicProcs and EventManager bindings were renamed.

## Open

- `0x0049ca60` (4810 bytes) builds local tables of track names
  (`"Quarry01"`..). `0x0049c2f0` calls it with 1 and `0x0049c770` with 2.
  Not attempted.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/ZoneReport.cpp -o work/ZoneReport.obj
python3 tools/match.py --exe "$MCM2_EXE" --target-va 0x0049c770 --target-size 739 --obj work/ZoneReport.obj --symbol '?UnknownFunction49c770' --bindings src/reconstructed/ZoneReport.bindings.json
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
