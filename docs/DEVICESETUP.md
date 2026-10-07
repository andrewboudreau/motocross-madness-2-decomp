# DeviceSetup (DirectX probe and control layout)

Canonical reconstruction: `src/reconstructed/DeviceSetup.*`. The near miss
is in `samples/game/DeviceSetupNearMisses.*`.

The unit has no RTTI and no `__FILE__` literal, so the following names are
all ours (tier 3):
- the file name `DeviceSetup.cpp`;
- the object `UnknownTrackGameObject33fc` (declared in `TrackGame.h`);
- every function name.

## Extent

The gap `0x00448560..0x00449e60` runs from the end of DebugOverlay.cpp
(`0x00447a67`) to dirlist.cpp (`0x00449ea6`). Its parts:
- `0x00448560..0x0044895b` is `GetDXVersion`. The startup code
  (`0x004a08f7`, [MAIN](MAIN.md)) calls it.
- `0x00448960..0x00449e54` is the control-layout object at TrackGame+0x33fc
  and the key-name helpers after it.

Whether the probe and the control code share a translation unit is not
proven. Both sit between the same two `__FILE__` neighbours.

## Control layout (19 exact)

The object holds an 8 x 14 table of assignments: devices by control rows,
-1 when unassigned. Each assignment is a kind (0 keyboard, 1 mouse,
2 joystick) and a code.

The table is stored in the profile's control file (`"control.ctl"`,
UiInfo.cpp) under:
- `"Setup"`/`"CurrentInputDevice"`;
- `"Controller%d"`/`"Key%d"` values written as `"%d,%d"` or `"NONE"`.

Three private-profile helpers do the file I/O. They live outside this gap and
are only declared here: `0x0047b930`, `0x0047b9e0` and `0x0047ba80`, in the
unattributed range `0x0047b670..0x0047bb20`.

| VA | Size | Role |
|---|---:|---|
| `0x00448960` | 39 | constructor: every assignment -1 |
| `0x00448990` | 66 | reads the current device and every assignment |
| `0x004489e0` | 100 | writes them |
| `0x00448a50` | 356 | parses one assignment |
| `0x00448bc0` | 198 | writes one assignment |
| `0x00448c90` | 39 | sets one assignment |
| `0x00448cc0` | 112 | finds a duplicate on the current device |
| `0x00448d30` | 186 | attaches an axis binding to the joystick, mouse or keyboard |
| `0x00448df0` | 148 | maps a button row through the control mapping |
| `0x00448e90` | 912 | installs the current device (two in-extent jump tables) |
| `0x00449220` | 80 | default device for the active joystick |
| `0x00449270` | 224 | key name: resource 0x1406 + `GetKeyNameText`, `"Unknown"` |
| `0x00449350` | 43 | name of a row's assignment |
| `0x00449380` | 2492 | assignment display name (three in-extent jump tables) |
| `0x00449d40` | 67 | NEC PC-98 scan-code translation on Windows 9x |
| `0x00449d90` | 30 | detects the keyboard subtype and the platform once |
| `0x00449db0` | 44 | `GetVersionEx` platform is NT |
| `0x00449de0` | 77 | Japanese keyboard subtype from `GetKeyboardType` |
| `0x00449e30` | 36 | `value`, or 0 with the PC-98 keyboard on Windows 9x |

Globals:
- `g_Pc98ScanCodes` (`0x00568df8`);
- `g_KeyboardSubtype` (`0x005691f8`);
- `g_IsWindowsNT` (`0x0059ada8`).

Source forms that mattered:
- **`0x00448d30`.** `if (device) call; break;`, then one final `return 0`.
  This reproduces the merged return blocks and the late `push esi`.
- **`AxisOfCode` in `0x00448e90`.** An explicit `case -5` makes the sparse
  switch dense enough for a direct jump table.
- **`0x00449380`.** The inner joystick switch needs `default: return 0`.
- **`0x00449220`.** It assigns `field_0x00` and returns it. Retail stores the
  field, then reloads it.
- **`0x00449de0`.** Two shapes are needed:
  - a returned `subtype = 0` local, which keeps zero in `edi`;
  - `switch ((GetKeyboardType(1) >> 8) & 0xff)`.
- **`0x00449e30`.** `int result = value; ...; result = 0; return result;`.

## GetDXVersion (near miss)

`0x00448560` (1011 bytes) is the DirectX SDK's GetDXVersion sample with
these changes:
- it reports through a third pointer whether `"Blade.dll"` loads;
- it loads that DLL in place of DDRAW.DLL on Windows 9x;
- it frees DINPUT.DLL before testing `DirectInputCreateA`;
- it reports 0x501 for builds after 0x549.

The candidate (1006 bytes) has the same instruction sequence and branch
layout as retail, except for one register swap:
- Retail keeps zero in `edi` and the library handle in `ebp`. VC6 swaps
  them here.
- The swap spills `DirectInputCreate` and adds four bytes to the frame.

Not tried successfully:
- dropping the dead NULL initialisers;
- giving the NT path separate library variables.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/DeviceSetup.cpp -o work/DeviceSetup.obj
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
