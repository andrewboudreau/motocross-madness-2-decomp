# EventManager

RTTI: `EventManager : GameObject : BaseObject` (vtable `0x0055259c`;
0xd08 bytes, the size TrackGame slot 4 allocates). Its constructor
`0x0045c9e0` writes the vtable. Canonical source is
`src/reconstructed/EventManager.h` / `EventManager.cpp`; the TU is not
established. Names are provisional.

TrackGame keeps it at +0x570 (it was the placeholder `TrackGameList`).

## Status

Exact (21 calibration cases):
- the constructor (11 0x50-byte entries at +0x50; -1000 in each component
  of +0x3c4) and both destructors;
- slot 8, which reads "KeepAliveTimeout" (default 20) into +0x2c;
- `0x0045d2b0`, `0x0045d2f0` and `0x0045d340`: the first race-mode object of
  TrackGame+0x558..+0x568 present, its +0x34 view and its +0x6c target;
- `0x0045d390`: whether any is present;
- `0x0045d270`: slot 5 on all three;
- slot 10, the per-frame update: while UI interaction is blocked
  (TrackGame+0x3430) it advances the block timer, ticks the +0x424
  listeners, and pans the camera at +0x3d4 by `frameTime * speed / 7`;
  after 7 seconds it lifts the block;
- slots 22 and 23: a press of control 1, 0x1c or 0x39, or any joystick
  button, ends the block once slot 23 has armed it;
- `0x0045e520`: resets the 11 entries;
- `0x0045e550`: the race-start wait; it scans TrackGame's racer slots
  (+0x2228, 0xf8 apart) for readiness and drives the network object;
- `0x0045e600`: finishes an event. Retail keeps a redundant
  `blocked && !pending` early return between the two main branches; the
  source keeps it because the branch layout depends on it;
- `0x0045f180`: awards points from a table by position;
- slot 24, the network messages (`type`, `data`, sender `player`): types
  5 and 0x89 mark a player ready (and in mode 2 without a race-mode object
  call `0x0045fbd0`); 0x86 copies a remote racer's state into the view's
  racer array; 0xcc shows "<name> <text>" (string 0x13d7) and drops the
  player; 0x8e, from the local player id, shows string 0x13d1 or resets the
  entries. The buffers are function-scope (0x8e reuses the 260-byte one
  with a 128-byte limit), and the best-lap update compares through two
  float locals (`fld; fld; fcompp`);
- `0x0045fbd0`: removes a player, moving the last entry (and, with a
  race-mode object, TrackGame's last 0xf8-byte record) into its place, then
  qsorts the entries with the unsigned comparator `0x0045fbb0`;
- the cdecl comparators `0x0045e930` (standings) and `0x0045d3d0` (racer
  names, through the inline `strcmp` intrinsic).

GameObject's slot 10 takes a float frame time. EventManager adds and scales
it, and retyping the declaration leaves GameObject's and KrustyBikeCamera's
code unchanged.

The race-mode objects, their views and targets are treated as GameObjects.
That is inference from their use (slots 4 and 5, the +0x25 flag bits), and
it fits KrustyBike's primary base chain for the views. Their classes are not
established; `src/reconstructed/RaceView.h` declares them.

Not reconstructed:
- `0x0045cb70`, `0x0045cdc0`, `0x0045d480`, `0x0045e710`, `0x0045e9d0`,
  `0x0045eef0` and `0x0045f9a0`.

Near miss (`samples/game/EventManagerNearMisses.cpp`): the cdecl progress
callback `0x0045cb20` (63 of 67 bytes; retail swaps two registers).

Slot 24's message layouts live in `EventManager.cpp`, and the GUI page and
control classes in the near-miss sample. Declaring either in a shared
header changed VC6's register choice in TrackGame slot 1 (an unrelated
`availPhys + availPageFile` sum): header-only type additions can disturb
other translation units, so re-run the full calibration after header edits.

GameObject slot 24 (and Game slot 17, which forwards to it) now take
`(int type, void* data, int c, int d, int e)`; the change is code-neutral.
The view's +0x38 and +0x3c are racers (`UnknownEventRacer`, in
`RaceView.h`); TrackGame's 0xf8-byte racer records start at +0x215c, after their count.
