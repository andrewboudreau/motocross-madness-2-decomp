# Program entry (WinMain and the main window)

Canonical reconstruction: `src/reconstructed/Main.*`. The unit has
no RTTI and no `__FILE__` literal. The file name and every name except
`WinMain` are ours (tier 3). `Main.cpp` is chosen to fit the alphabetical
link-order bracket Lzw.cpp .. the matrix unit; it is not attested.

## Extent and evidence

The unit runs `0x004a05e0..0x004a10d5`, between Lzw.cpp (`0x004a03ac`) and
the matrix unit. Padding runs to `0x004a10e0`.

Confirmed:
- `0x004a0bc0` is `WinMain`. The CRT start-up calls it (`0x00538180`), and it
  is stdcall with four arguments.
- Its `.data` starts with `g_AppActive` (`0x0056de98`, initialised to 1).
- Its literals follow in reverse order of use, from `"tst"` (`0x0056de9c`)
  to `"blank.cur"` (`0x0056decc`).

Inference: the five functions form one unit. The evidence is that run of
literals plus the `.bss` pair `0x00685024`/`0x00685028`.

## Status (5 of 5 exact)

| VA | Size | Function |
|---|---:|---|
| `0x004a05e0` | 151 | EnumWindows callback (inline `strcmp`) |
| `0x004a0680` | 1344 | start-up and message loop |
| `0x004a0bc0` | 130 | `WinMain` (`__try`, scope table `0x005551b0`) |
| `0x004a0c50` | 242 | window class and window |
| `0x004a0d50` | 901 | window procedure, with its two jump tables and two byte index tables |

### EnumWindows callback (`0x004a05e0`)

It looks for a window titled like the game (TrackGame+0x3a0). When it finds
one, it restores that window and reports it through `lParam`.

### Start-up (`0x004a0680`)

1. `g_MemTagStack->Push("Startup")`.
2. Copies the names `"Rainbow"`/`"Demo"` into TrackGame+0x320/+0x3a0, or
   string resources 0xbbc/0xbbd when they load.
3. Exits if another instance runs.
4. `setlocale(LC_ALL, "")`.
5. Stores the current directory at Game+0x1cc.
6. Calls slot 1.
7. Creates the window.
8. Calls slot 2.
9. Calls `GetDXVersion` with `&TrackGame+0x544` as the third pointer.

Then it stops with a message box and `exit(0)` in four cases:

| Case | String resource |
|---|---|
| The platform is neither Windows 9x nor NT | 0xbb8 |
| The DirectX version is below `0x0056e270` (0x700) | 0xbb9 |
| On NT, PCGame slot 27 (`SetRegistryFlag("IsAdmin", 1)`) fails | 0x14dc |
| On NT, `GetTempFileName(".", "tst")` + `fopen(..., "w")` in the current directory fails | 0x14dc |

After the checks:
- `PCGame::StartUp` failures destroy the window and show its message. They
  still fall into the loop, which receives `WM_QUIT`.
- The message loop calls Game slot 10 while the application is active. When
  slot 10 returns 0, it shuts down through slot 15.

### Window class and window (`0x004a0c50`)

Its parameters:
- class `CS_DBLCLKS`;
- icon `IDI_APPLICATION` via `LoadImage`;
- cursor `"blank.cur"`;
- `BLACK_BRUSH`;
- class name TrackGame+0x320;
- window `WS_EX_APPWINDOW`, `WS_POPUP | WS_VISIBLE`, at screen size.

It hands the window to `0x00447910` (DebugOverlay.h).

### Window procedure (`0x004a0d50`)

| Message | Handling |
|---|---|
| `WM_SETFOCUS`/`WM_KILLFOCUS` | (un)acquire the input devices (`0x004bf490`) |
| `WM_MOVE`/`WM_SIZE` | slot 6 when minimised, otherwise `PCGame::SetWindowRect` with the client rectangle in screen coordinates |
| `WM_CLOSE` | slot 15 |
| `WM_ACTIVATEAPP` | slot 5/6, re-acquire, the IMM32 helper `0x0052ff20` |
| `WM_CHAR`/`WM_KEYDOWN` | slots 35/36 |
| `WM_SYSCOMMAND` | swallows `SC_KEYMENU`, `SC_SCREENSAVE`, `SC_MONITORPOWER` and 0xf190 while active |

## Remaining uncertainty

- `0x00537104` is bound as `setlocale` from its `(0, "")` arguments and CRT
  position. The library routine itself has not been compared.
- `0x0056e270` (the required DirectX version) sits next to `g_TrackGame` and
  is only declared here.

Source forms that mattered:
- The name copies use `count = length > 0x7f ? 0x7f : length`. An `if` form
  compares against the register holding 0x7f.
- `exit` is `__declspec(noreturn)` in the VC6 headers. That keeps the
  platform value live in `eax` across the error blocks.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/Main.cpp -o work/Main.obj
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
