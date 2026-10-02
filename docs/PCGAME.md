# PCGame

RTTI: `PCGame : Game` (vtable `0x00555e50`, 38 slots; `TrackGame : PCGame`
derives from it). The literal `__FILE__` `D:\aardvark\VC\krusty2\PCGame.cpp`
(slot 31) names the translation unit. Canonical source is
`src/reconstructed/PCGame.h` / `PCGame.cpp`. Names are provisional.

PCGame is the Windows layer. It holds:
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

The following are exact (34 calibration cases):
- the constructor, both destructors and the time stamp function
  `0x004bfa80`;
- slots 2, 5, 6, 7, 13, 14, 15, 19, 31, 32, 34, 35, 36 and 37 (every
  override);
- the non-virtual `0x004c0470` and `0x004c0760`;
- every registry slot, 20-29.

The registry slots read and write values under
`HKEY_LOCAL_MACHINE\<+0x4b8>`. A `Sub\Value` name selects a subkey.
`UNKNOWN_SETTING_PATH` stands for the path-building code that every one of
them repeats. Slots 25-27 are one folded body.

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

Near miss: the profile loader `0x004c16f0` (767/771,
`samples/game/PCGameNearMisses.cpp`). Its copy loop swaps the SIB base and
index registers.

Not reconstructed:
- the profiling pass `0x004c0d10` (1790 bytes, with an EH frame);
- the start-up function `0x004bfc50` (1504 bytes), which calls the profile
  helpers.
