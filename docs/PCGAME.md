# PCGame

RTTI: `PCGame : Game` (vtable `0x00555e50`, 38 slots; `TrackGame : PCGame`
derives from it). The literal `__FILE__` `D:\aardvark\VC\krusty2\PCGame.cpp`
(slot 31) names the translation unit. Canonical source is
`src/reconstructed/PCGame.h` / `PCGame.cpp`. Names are provisional.

PCGame is the Windows layer. It holds:
- the Direct3D device GUID (+0x2f8);
- the window rectangle (+0x308);
- the instance and window handles (+0x318, +0x31c);
- the registry names "Rainbow Studios", "Rainbow Demo" and
  `SOFTWARE\Rainbow Studios\Demo` (+0x320, +0x3a0, +0x4b8);
- the OS version (+0x424);
- the IME state (+0x538..+0x540).

## Game's display and render target

Two of Game's members are now typed:
- **+0x0c is the display (`Display.h`).** PCGame slot 31 hands it to
  PCRenderTarget `0x004c4f80`, which stores it at RenderTarget+0x04 through
  `0x004e8ca0`. This is the same object Camera reads display modes from.
- **+0x10 is a RenderTarget.** Slot 31 allocates 0xbc4 bytes and runs
  PCRenderTarget's constructor `0x004c4ee0`. Slot 13 calls PCRenderTarget
  `0x004c5d00`, and slot 19 calls RenderTarget's set-camera `0x004e8cf0`.

PCGame reaches the PCRenderTarget members through `PCTarget()`, a cast
accessor.

PCRenderTarget+0x4c is a surface: PCGame slot 5 calls its methods 24 and 27.
Those are the IDirectDrawSurface IsLost/Restore positions, which is
inference.

## Status

The following are exact (35 calibration cases):
- the constructor, both destructors and the time stamp function
  `0x004bfa80`;
- slots 2, 5, 6, 7, 13, 14, 15, 19, 31, 32, 34, 35, 36 and 37 (every
  override);
- the non-virtual `0x004c0470` and `0x004c0760`;
- every registry slot, 20-29.

The registry slots read and write values under
`HKEY_LOCAL_MACHINE\<+0x4b8>`. A `Sub\Value` name selects a subkey.
`UNKNOWN_SETTING_PATH` stands for the path-building code that every one of
them repeats. Slots 25-27 are one folded body. Names: 20
`GetRegistryInt`, 21 `GetRegistryFloat`, 22 `GetRegistryFlag`, 23
`GetRegistryString`, 24 `GetRegistryBinary`, 25 `SetRegistryInt` (its
values are read back with slot 20), 27 `SetRegistryFlag` (its keys are read
back with slot 22), 28 `SetRegistryString`, 29 `SetRegistryBinary`. Slot
26 has no known caller and keeps its provisional name.

`0x004bfa80` is placed in PCGame.cpp by position only: it sits immediately
before the constructor.

Slot 7 sets the render target's states (numbers matching Direct3D render
state IDs) and texture stage 0's filters. Slot 34 filters the display modes:
- duplicates that differ only in refresh rate ("HighestRefreshOnly");
- modes other than 16-bit;
- modes that do not fit video memory with "MinimumTextureMB" spare.

Display profiles live under `DriverInfo\<driver name>`. They hold a
display's saved device identifier, its mode list and per-mode flags, video
memory, and the disabled/partial-blit settings. The following are exact:
- `0x004c1610` saves a profile and `0x004c16b0` saves one for every display
  in the global array `0x0068a754` (count `0x0068a764`);
- `0x004c1410` reports whether any profile is stale;
- `0x004c1a00` deletes them all.

The display's identifier (+0x5c0) is 0x430 bytes, the size of
DDDEVICEIDENTIFIER2 (inference). Its description at +0x7c0 is what Game
slot 8 prints.

The profile loader `0x004c16f0` (771 bytes, exact) reads the cache limit,
video memory and mode count, compares the saved mode list with the
display's and copies the saved per-mode flags across, then the partial blit,
8-bit texture, AGP and disabled settings. Its copy loop counts the remaining
modes down with a named `count - remaining` index: an ascending index swaps
the SIB base and index registers of the four stores (the same difference as
EcoSystem's `0x00458da0`).

Near miss: the profiling pass `0x004c0d10` (1790 bytes,
`samples/game/PCGameNearMisses.cpp`, 1776 of 1790 positions; it returns 1
from both exits). Only the scheduling of one surface-description store
differs (notes in the sample). The
pass uses the display's DirectDraw-shaped interface (+0x190) for the
following, which are inference:
- SetDisplayMode (method 21);
- GetAvailableVidMem (method 23);
- CreateSurface (method 6), to test whether ten 256x256 textures fit.


## Start-up (`0x004bfc50`)

The start-up function runs before Game's initialiser `0x00467b70`, which it
calls last; its argument is the failure message buffer. In order, it:
1. Maps the "Renderer" setting to a device GUID. HAL, RGB, MMX, Ramp, Null and
   Ref are confirmed by their values, which equal the DirectX IIDs of those
   names. "Blade" (`0x00556040`) is not a DirectX GUID and sets +0x2d0.
2. Runs slot 37 and enumerates the displays (`0x004c9600`).
3. When any profile is stale, asks through a message box (string 0x13d8),
   then re-profiles.
4. Runs the profiling pass and chooses the display (`0x004ccd60`).
5. Reads its `Allow*` flags into +0x2d4 bits 3-7.
6. Selects the joystick (`0x004cd610`) on the ControlInterface.

PCRenderTarget+0x54 receives that GUID.

## Names

Function names (tier 3, from behaviour): `0x004bfc50` `StartUp`,
`0x004c0470` `SetWindowRect`, `0x004c0760` `LimitDisplayModes`,
`0x004c0d10` `ProfileDisplays`, `0x004c1410` `IsAnyProfileStale`,
`0x004c1610` `SaveDisplayProfile`, `0x004c16b0` `ProfileEveryDisplay`,
`0x004c16f0` `LoadDisplayProfile`, `0x004c1a00` `DeleteDisplayProfiles`.
Fields: `deviceGuid` (+0x2f8, "Renderer"), `companyName` (+0x320,
"Rainbow Studios") and the texture-stage filters `magFilter`, `minFilter`,
`mipFilter` (+0x54c..+0x554, D3DTSS_MAGFILTER/MINFILTER/MIPFILTER in slot
7). Windows members now use `windowRect` (+0x308), `instanceHandle`
(+0x318), `windowHandle` (+0x31c), `applicationName` (+0x3a0),
`resourceInstance` (+0x420), `osVersion` (+0x424), `registryKey` (+0x4b8),
`imeLibrary` (+0x538), `inputContext` (+0x53c) and `previousInputContext`
(+0x540). `displayProfilesStale` names bit 0 at +0x548. UI, input and
TrackGame callers use these provisional names; the declarations retain
the decoded offsets and layout.
