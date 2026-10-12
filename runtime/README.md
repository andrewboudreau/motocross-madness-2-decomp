# Runtime

New code (not matching source) that lets the retail demo executable run on
current Windows and, later, hosts reconstructed functions and a replacement
renderer. Nothing here is built from or contains game code or data; the user
supplies the game directory.

```bash
python runtime/build.py                         # VC6 build -> work/runtime/*.dll
python runtime/build.py --stage work/run/demo   # copy the demo once, install the DLLs
work/run/demo/mcm2.exe
```

`--stage` copies `$MCM2_EXE`'s directory (default `work/game/MCM2demov2`) and
never modifies the source directory. The build uses `$VC6_ROOT` or
`work/vc6sp3-disc`; any VC6 install works, since these modules are not matched.
Close the game before rebuilding: Windows keeps the installed `d3drm.dll`
locked while it runs.

Status (Windows 11 26200, 3840x2160 desktop): the demo boots, shows the
title, profile and menu screens, and plays Voodoo Basin and the Donner Pass
stunt quarry at 60 frames a second with keyboard and mouse. It renders with
hardware Direct3D 7 (tested on an RTX 4080) or, if the game's hardware test
disabled that, with its own `blade.dll` software rasterizer. Our code only
emulates the 16-bit full-screen display around the game's renderer.

| Variable | Effect |
|---|---|
| `MCM2_FPS` | Frame limit for the emulated Flip (default 60, `0` for none). |
| `MCM2_NATIVE_FULLSCREEN=1` | Turns the display emulation off (does not work on current Windows; for comparison). |

## Modules

| DLL | Why it exists | Behaviour |
|---|---|---|
| `d3drm.dll` | Retail imports `D3DRMVectorRotate` (hint 17, FollowCamera's only d3drm call). Windows Vista and later ship no `d3drm.dll`, so the stock demo does not start. | Implements the rotation (unit-length result, axis not modified). Checks the executable's SHA-256 against the target build. Writes `mcm2_runtime.log` next to the exe. Records a portable install (below). Patches the exe's imports before `WinMain` (`trace.c`) and emulates the display (`present.c`, `vtrace.c`). |
| `dplayx.dll` | Retail imports `DirectPlayLobbyCreateA` (ordinal 4). On Windows 10/11 the system `dplayx.dll` blocks the loader on the "Windows Features: DirectPlay" prompt (`Fondue.exe`). | Returns `DPERR_UNAVAILABLE`. Only `ConnectUsingLobby` calls it and it handles failure, so lobby-launched multiplayer is off. |

## Start-up requirements (Windows 11)

1. `d3drm.dll` missing: process fails to load. Fixed by the module above.
2. DirectPlay feature prompt: loader waits on `Fondue.exe`. Fixed by the stub.
3. "Installation error. Please reinstall" (string 0x13b3):
   `TrackGameMode::FindDataDirectory` (0x00523c90) reads `InstallType` and
   `HardDriveRootPath` from
   `HKLM\Software\Microsoft\Microsoft Games\Motocross Madness 2 Trial`.
   `InstallType` `Full` also skips the CD check (`FindCdDirectory`).
   `d3drm.dll` writes both (path without trailing backslash) only when
   `HardDriveRootPath` is absent. Unelevated, Windows virtualizes the write to
   `HKCU\Software\Classes\VirtualStore\MACHINE\SOFTWARE\WOW6432Node\...`
   because the exe has no manifest.
4. The trial EULA dialog (`EBUEula.dll`) appears next; the user accepts it.
5. A hardware-test message box follows; OK continues.

## Display emulation

The game asks for exclusive full screen at 640x480x16. Current Windows only
offers 16-bit modes through the DWM8And16BitMitigation shim, under which the
flip chain is lost right after creation and never restored
(`DDERR_SURFACELOST`, then `DDERR_IMPLICITLYCREATED` from Restore on the
implicit back buffer). Setting the `16BITCOLOR` compatibility layer leads to
the same state. Instead, the game's DirectDraw object (from
`blade.dll!DirectDrawCreateEx`, which wraps the system ddraw) stays windowed
on the 32-bit desktop:

- `SetCooperativeLevel(EXCLUSIVE|FULLSCREEN)` becomes `NORMAL`, and the game
  window (which the game sizes to 22x56 for exclusive mode) becomes a
  borderless popup covering the primary monitor.
- `SetDisplayMode`/`GetDisplayMode` record and report the mode;
  `EnumDisplayModes` reports a 16-bit twin of each 32-bit mode.
- Surfaces created without a pixel format get RGB 565. On a 32-bit desktop
  ddraw rejects 16-bit system-memory surfaces that name no surface type (the
  game's `3DDEVICE|SYSTEMMEMORY` render target and its 2D images) with
  `DDERR_INVALIDPIXELFORMAT`; adding `DDSCAPS_OFFSCREENPLAIN` makes them
  valid, and blade's `CreateDevice` then succeeds.
- The primary is an `EmulatedPrimary`, a small `IDirectDrawSurface7` over two
  RGB 565 system-memory surfaces. Flip waits for the next frame slot
  (`MCM2_FPS`), then draws the back buffer into the window with
  `StretchDIBits`, aspect-correct with black bars.

`apphelp.dll` redirects the system IDirectDraw7 vtable slots 6, 12, 20 and 21
before our hooks see them, so patching blade's inner ddraw object has no
effect (the log says "already redirected"); all emulation happens on blade's
outer object. The system shim database entry for MCM2 (matched on `MCM2.ICD`,
`MCM2.EXE`, `RIDERS\RIDER01.TGA`) applies only IgnoreAltTab and a
VirtualRegistry INDEO5 fix.

## Hardware Direct3D

`PCGame::StartUp` (0x4bfc50, `src/reconstructed/PCGame.cpp`) reads the
`Renderer` setting (default `HAL`, hardware Direct3D 7); the display chooser
(0x4ccd60) falls back to the Blade device when the card's profile under
`DriverInfo\{driver GUID}` says `DisabledHardware = 1`. Two things kept the
hardware path from working on a current card:

- Video memory sizes are signed ints. `CreateModeSurfaces` (0x4ca520) fails
  when the flip chain does not fit in "video memory - 2 * mode size", and 2 GB
  or more reads as negative, so the game quit right after `SetDisplayMode`.
  `GetAvailableVidMem` and `GetCaps` now report at most 256 MB.
- The emulated primary's back buffer was in system memory, which a HAL device
  cannot render to. For a 3D flip chain requested in video memory it is now
  an RGB 565 video-memory surface; `CreateDevice(IID_IDirect3DHALDevice)`
  succeeds on it and Present reads it back.

With both fixes a fresh profile (no `DriverInfo` key) passes the game's
hardware test on an RTX 4080: every entry gets `DisabledHardware = 0` and
`TotalVideoMemory = 268435456`, and the game creates the HAL device.
Profiles written before these fixes may still hold `DisabledHardware = 1` and
a `TotalVideoMemory` of 2 GB or more; set them to 0 and 268435456 (or delete
`DriverInfo` so the game profiles the card again). With hardware rendering a
race uses about 22% of one core, against about 50% with Blade.

## Timing and input

- The game's clock (0x4bfa80) wraps every `timeGetTime` in
  `timeBeginPeriod(1)`/`timeEndPeriod(1)`. On current Windows each pair calls
  `NtSetTimerResolution`, which took about 94% of the process's time. The
  import hooks raise the resolution once and ignore the rest.
- Without vertical-blank waits the emulated Flip ran at about 1,200 frames a
  second. With the 60 Hz limit the menus use about 13% of one core and a race
  about 50%; the race clock still keeps wall time.
- Keyboard and mouse are DirectInput devices (mouse: exclusive|foreground,
  relative axes, cursor drawn by the game). Both work unchanged.
  `GetCursorPos` is only a fallback when no input axes exist; its hook maps
  the point into the 640x480 image anyway.
- The trial allows Voodoo Basin (Nationals) and the Donner Pass Stunt
  Quarry, in Practice mode; other tracks report "Track unavailable in Trial
  Version".

## Plan

1. M0 boot: done.
2. M1 hooks: `jmp` patches from `d3drm.dll`, first hand-written, then VC6
   objects from `src/` linked against retail addresses from `*.bindings.json`.
3. M2 trace: done for DirectDraw, surfaces and DirectInput (`vtrace.c`).
4. M3/M4: DirectX 7 subset on OpenGL behind `blade.dll!DirectDrawCreateEx`,
   2D (CPU surfaces, GDI through DIB sections) then 3D, replacing the
   software rasterizer.
5. M5: windowed mode, resolution choice, movies.
