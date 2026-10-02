# TrackGame

RTTI: `TrackGame : PCGame : Game` (vtable `0x00558938`). The translation unit
is trkgame.cpp (literal `__FILE__` references). Canonical source is
`src/reconstructed/TrackGame.h` / `TrackGame.cpp`. Names are provisional.

## It is the global game object

The pointer at `0x0056e26c` is statically initialised to `0x006851a0`. A
dynamic initializer (`0x004aa7c0`) constructs the object there with
`0x00520870`, which writes TrackGame's vtable. `0x004aa7e0` registers
`0x00521ae0` with atexit to destroy it.

The class the camera and input code reads through that pointer was
previously the placeholder `UnknownObject56e26c`; it is now TrackGame and
derives from PCGame.

## Status

Exact (17 calibration cases):
- the constructor and both destructors;
- slots 1, 2, 3, 5, 10, 13, 14, 15 and 18;
- the menu toggle `0x00521860`;
- the string-resource loader `0x00521970`;
- the helpers `0x00521a30`, `0x00521a40` and `0x00521cd0`.

Slot 14 ignores input while an IME is open, then:
- control 0x1d (with modifier 0x45) or controls 0x3d/0xc5 toggle menu 0x190;
- control 1 opens menu 0x191.

Its two `goto` labels reproduce retail's shared blocks: the toggle, and the
`return 1` it shares.

The constructor:
- loads `lang.dll` (it becomes the string resource instance);
- switches the user locale's decimal separator to "." (slot 15 restores the
  saved one);
- sets the registry key to "Software\Microsoft\Microsoft Games\Motocross
  Madness 2 Trial";
- reads four network/record packet intervals (milliseconds, stored in
  seconds).

The destructor:
- opens the store web page and/or string 0x14df's page when requested;
- deletes the owned objects;
- frees `lang.dll`;
- on Windows NT, re-enables the screen saver.

Slot 1 runs the start-up checks:
- the +0x578 object's two checks (the second retried from a message box);
- a warning below 64 MB of available memory;
- on NT, turns the screen saver off;
- runs EBUEula.dll's `EBUEula` entry point on `EULA.rtf`.

Slot 3 adds ten `Res\*.res` archives to the resource manager at
`0x00572b44`. Its code sits among ResourceManager.cpp's literals; no RTTI
names it.

Not reconstructed: slot 4, the 1543-byte game set-up that creates the menus,
input mapping, network and race objects.
