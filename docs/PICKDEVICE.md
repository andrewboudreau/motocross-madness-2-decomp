# Display and controller selection (Ph..Pi unit)

Canonical reconstruction: `src/reconstructed/PickDevice.cpp`. The gap is
`0x004ccbd0..0x004cde20`, between PhysicsRigidBody and Pixtrans.cpp. No
`__FILE__` literal or RTTI reaches it, so the file name is ours (tier 3).
PCGame calls `0x004ccd60` to choose the display and `0x004cd610` to choose
the joystick ([PCGAME](PCGAME.md)). Each builder registers a window class,
fills a list box and pumps messages until a window procedure sets its done
flag.

## Matched (6 of 8 starts, strict exact)

- `0x004ccc50` (267 bytes) is the `"VideoCardClass"` window procedure.
  `0x004ccd60` stores it as `lpfnWndProc` (`0x004cce7e`).
- `0x004cd4b0` (346 bytes) is the `"InputDeviceClass"` window procedure.
  `0x004cd610` stores it as `lpfnWndProc` (`0x004cd77a`). Button 1003
  toggles `UseLastController` (`0x00571658`) and saves it with
  `TrackGame::SetRegistryFlag`.
- `0x004ccbd0`/`0x004cd430` (71 bytes each) take the list box selection's
  item data (`LB_GETCURSEL`, `LB_GETITEMDATA`), destroy the window and set
  the done flag. `0x004ccc20`/`0x004cd480` (33 bytes each) set the cancelled
  flag instead.

The globals sit at `0x00689a2c..0x00689a68`, except the controller choice
(`-1`) and `UseLastController` (`1`), which are initialized data at
`0x00571654`/`0x00571658`. Names are ours. The other handles in the block
(`0x00689a34`, `0x00689a38`, `0x00689a40`, ...) belong to the builders and
are not declared yet.

Source forms that mattered:

- Retail pushes the `LB_GETITEMDATA` constants after the `LB_GETCURSEL`
  call. A local `int index = SendMessage(...)` reproduces that. Nesting the
  call as an argument pushes the constants first.
- In the controller procedure the 1003 case falls through with `break` to
  the shared `DefWindowProc` tail. An explicit `return DefWindowProc(...)`
  duplicates the tail.

## Open

- `0x004ccd60` (1743 bytes) and `0x004cd610` (2054 bytes), the builders:
  `RegisterClass`, `CreateWindowEx` for the `STATIC`/`BUTTON`/`LISTBOX`
  controls, `DriverInfo\%s\DisabledFullScreen`/`DisabledHardware` registry
  checks and `UseVideoCardIdx`/`UseControllerId`. Not attempted.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/PickDevice.cpp -o work/PickDevice.obj
python3 tools/match.py --exe "$MCM2_EXE" --target-va 0x004cd4b0 --target-size 346 --obj work/PickDevice.obj --symbol '?ControllerWindowProc@@YGJPAUHWND__@@IIJ@Z' --bindings src/reconstructed/PickDevice.bindings.json
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
