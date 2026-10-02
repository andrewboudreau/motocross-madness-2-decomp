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

Exact (12 calibration cases):
- the constructor and both destructors;
- slots 2, 5, 10, 13, 15 and 18;
- the non-virtual helpers `0x00521a30`, `0x00521a40` and `0x00521cd0`.

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

Not reconstructed: slots 1, 3, 4 and 14, plus `0x00521860` and `0x00521970`.
