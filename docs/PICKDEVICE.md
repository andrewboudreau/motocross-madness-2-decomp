# Display and controller selection (Ph..Pi unit)

Canonical reconstruction: `src/reconstructed/PickDevice.cpp`. The gap is
`0x004ccbd0..0x004cde20`, between PhysicsRigidBody and Pixtrans.cpp. No
`__FILE__` literal or RTTI reaches it, so the file name is ours (tier 3).
Its literals and two `.data` flags are contiguous at
`0x00571590..0x00571688` and its window handles and flags fill `.bss`
`0x00689a2c..0x00689a68`. PCGame calls `0x004ccd60` to choose the display and `0x004cd610` to choose
the joystick ([PCGAME](PCGAME.md)). Each builder registers a window class,
fills a list box and pumps messages until a window procedure sets its done
flag.

## Matched (8 of 8 starts, strict exact)

- `0x004ccc50` (267 bytes) is the `"VideoCardClass"` window procedure.
  `0x004ccd60` stores it as `lpfnWndProc` (`0x004cce7e`).
- `0x004cd4b0` (346 bytes) is the `"InputDeviceClass"` window procedure.
  `0x004cd610` stores it as `lpfnWndProc` (`0x004cd77a`). Button 1003
  toggles `UseLastController` (`0x00571658`) and saves it with
  `TrackGame::SetRegistryFlag`.
- `0x004ccd60` (1743 bytes) chooses the display. It returns the single
  usable display without asking (unless `PCGame+0x544` forces the dialog) or
  the `"UseVideoCardIdx"` one when `useLast` is set. Otherwise it builds the
  `"VideoCardClass"` window with `STATIC`/`BUTTON`/`LISTBOX` children, lists
  the Blade entry (string 0x1463, item data -1) and every usable display's
  description (`DriverInfo\<guid>\DisabledFullScreen`/`DisabledHardware`/
  `DisabledSoftware` flags), pumps `PeekMessage` and saves
  `"UseVideoCardIdx"`. `*blade` is set when the Blade entry was chosen.
- `0x004cd610` (2054 bytes) chooses the joystick. With `useLast` it returns
  the index whose instance GUID (formatted `"{%08lX-%04X-...}"`) equals the
  registry `"UseControllerId"`. Otherwise it runs the `"InputDeviceClass"`
  dialog listing up to 8 joysticks by product name (or `"None"`), returns
  the index (-2 on cancel or failure) and saves the GUID.
- `0x004ccbd0`/`0x004cd430` (71 bytes each) take the list box selection's
  item data (`LB_GETCURSEL`, `LB_GETITEMDATA`), destroy the window and set
  the done flag. `0x004ccc20`/`0x004cd480` (33 bytes each) set the cancelled
  flag instead.

The globals sit at `0x00689a2c..0x00689a68`, except the controller choice
(`-1`) and `UseLastController` (`1`), which are initialized data at
`0x00571654`/`0x00571658`. Names are ours. The other handles in the block
(OK/Cancel buttons, labels, the check box) are the builders' child windows.

Source forms that mattered:

- Retail pushes the `LB_GETITEMDATA` constants after the `LB_GETCURSEL`
  call. A local `int index = SendMessage(...)` reproduces that. Nesting the
  call as an argument pushes the constants first.
- The builders format the GUID with an inline helper taking `UnknownGuid`
  by value (retail copies the GUID into a 16-byte temporary first).
- In `0x004ccd60`, `usable = 0` is declared before the `"UseVideoCardIdx"`
  read, and the two `LB_SETCURSEL` calls are written out in both branches
  (`lastIndex > -1 ? lastIndex + 1 : entries - 1` merges the pushes
  differently).
- In `0x004cd610` the `MSG` is declared at function scope (inside the loop
  it overlaps the GUID temporary's slot), and in the "no joysticks" branch
  `selected = 0` precedes the registry write.
- `PCInputDevice.h` declares the two builders friends of `PCInputDevice`
  (they read `deviceInfo.instanceGuid` and `deviceInfo.productName`).
- In the controller procedure the 1003 case falls through with `break` to
  the shared `DefWindowProc` tail. An explicit `return DefWindowProc(...)`
  duplicates the tail.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/PickDevice.cpp -o work/PickDevice.obj
python3 tools/match.py --exe "$MCM2_EXE" --target-va 0x004cd4b0 --target-size 346 --obj work/PickDevice.obj --symbol '?ControllerWindowProc@@YGJPAUHWND__@@IIJ@Z' --bindings src/reconstructed/PickDevice.bindings.json
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
