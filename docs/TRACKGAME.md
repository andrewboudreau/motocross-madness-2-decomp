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

Every TrackGame function is exact (18 calibration cases):
- the constructor and both destructors;
- every override: slots 1, 2, 3, 4, 5, 10, 13, 14, 15 and 18;
- the menu toggle `0x00521860`;
- the string-resource loader `0x00521970`;
- the helpers `0x00521a30`, `0x00521a40` and `0x00521cd0`.

Slot 1 is sensitive to unrelated header content. VC6's register choice for
its `availPhys + availPageFile` sum (edx/eax) changes when declarations are
added to headers TrackGame.cpp includes. The sensitivity is not tied to
names or to the build path; adding unused probe declarations flips it at
irregular counts. After a header edit, re-run the full calibration. If slot 1
regresses, keep new types local to the file that uses them (as
EventManager.cpp and the EventManager near-miss sample do).

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

Slot 4 creates the game objects in order:
- scene, profile list and record objects;
- the audio, quitting with a message when the sound device is unavailable;
- collision, input mapping and debug objects;
- KrustyUI and EventManager, both added under the second root object;
- for network games, the debug connection and the race set-up.

The vtable writes in their constructors confirm three of these classes:
KrustyUI (+0x56c), EventManager (+0x570, previously the placeholder
`TrackGameList`) and DirectoryList (+0x3338). The +0x3410 object's `new`
calls a base constructor (`0x004676a0`) directly and keeps the allocation
pointer. That is what an implicit constructor of a derived class produces,
and its method sits in another TU.
