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
- `0x00448560..0x00448953` is `GetDXVersion`. The startup code
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
| `0x00449270` | 224 | `FormatKeyName`: resource 0x1406 + `GetKeyNameText`, `"Unknown"` |
| `0x00449350` | 43 | name of a row's assignment |
| `0x00449380` | 2492 | assignment display name (three in-extent jump tables) |
| `0x00449d40` | 67 | `TranslatePc98ScanCode`: NEC PC-98 scan-code translation on Windows 9x |
| `0x00449d90` | 30 | `DetectKeyboardLayout`: detects the keyboard subtype and the platform once |
| `0x00449db0` | 44 | `IsWindowsNTPlatform`: `GetVersionEx` platform is NT |
| `0x00449de0` | 77 | `GetJapaneseKeyboardSubtype`: Japanese keyboard subtype from `GetKeyboardType` |
| `0x00449e30` | 36 | `ValueUnlessPc98`: `value`, or 0 with the PC-98 keyboard on Windows 9x |

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
- **`0x00449220`.** It assigns `currentDevice` (+0x00) and returns it. Retail stores the
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

## Functional-build review

The functional source build explicitly includes this candidate through
`config/game_build.json`. It remains in `samples/`, registered as `partial`
in `samples/game/targets.json`; neither inclusion nor behavioral validation
claims byte identity or recovered translation-unit ownership.

The decoded retail body was compared with the VC6 `/O2 /MT` candidate over
the complete 1011-byte retail extent. The candidate is 1006 bytes with a
0x11c-byte local frame versus retail's 0x118. The changed register assignment
spills the DirectInput entry-point address, changes epilogue scheduling and
instruction lengths, and shifts branches. The API arguments, signed HRESULT
tests, output writes, COM slots, object lifetimes and failure cleanup agree.

`tests/test_device_setup_behavior.py` compiles a native VC6 differential
harness and reads the private hash-pinned executable. Four needed retail PE
pages are placed at their original addresses inside test-only reserved
storage; the four API IAT slots are redirected to deterministic mocks. It
compares retail with the candidate source compiled against the same API
mocks. No private code enters the repository or the production build.

The test passed **720 scenarios**, comparing all three output values and
ordered API/COM calls:

- NT major versions 3, 4, 5 and 10, Windows 9x, and the unknown-platform
  fallback that retail also sends down the 9x path;
- build values below, at and above 0x549, including a nonzero high word;
- every observed version/library/entry-point/COM failure and success path;
- exact primary-surface descriptor fields, GUID arguments, release order,
  library release order and caller-visible untouched-output cases.

The original routine does not release the successful Surface3/Surface4
query results; the candidate preserves that behavior. The test provides
finite behavioral evidence, not proof of arbitrary DLL implementations or
full-game startup. Real current-Windows startup remains a game-build gate.

The required version scalar read by `WinMainBody` now has a source
definition in `Main.cpp`: an initialized `.data` DWORD of 0x700, matching
retail `0x0056e270`. Its original defining TU remains unproven. All five
existing Main functions still pass strict relocation-resolved matching.

`src/reconstructed/DirectXGuids.cpp` supplies nine identifiers missing from
the bundled SDK libraries, including the probe's DirectDraw7 and Surface4
IDs. Each has a confirmed 16-byte retail value and readonly storage, verified
by `tests/test_directx_guids.py`. Existing C-linkage callers bind to these
definitions, and the differential harness links this real provider. Its
filename does not assert original TU ownership.

## Keyboard storage

`DeviceSetupState.cpp` supplies the 256-entry PC-98 scan-code map at
`0x00568df8`, the keyboard-subtype detection sentinel at `0x005691f8`
(initially -1), and the zero-filled platform flag at `0x0059ada8`.
`TranslatePc98ScanCode` bounds the index to 0..255 and retains the input
when an entry is -1; `DetectKeyboardLayout` replaces the sentinel and
sets the platform flag. The original defining TU remains unknown.

The data-provider regression in `tests/test_graphics_state.py` verifies all
1,032 bytes, symbol identities, extents and writable storage using authentic
VC6 and the hash-pinned retail image. This supplies the existing input code's
storage; current-Windows keyboard behavior still needs the runnable game.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/DeviceSetup.cpp -o work/DeviceSetup.obj
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
python3 tools/run_physics_samples.py --exe "$MCM2_EXE" --vc6-root "$VC6_ROOT" --profile vc6_o2_mt --root samples/game --source samples/game/DeviceSetupNearMisses.cpp --strict
python3 -m unittest discover -s tests -p test_device_setup_behavior.py -v
```

The differential test requires native Windows, `VC6_ROOT` and `MCM2_EXE`;
otherwise it skips. The partial target check reports the byte mismatch and
must not be interpreted as strict-match acceptance.
