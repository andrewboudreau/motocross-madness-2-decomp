# EventManager

RTTI: `EventManager : GameObject : BaseObject` (vtable `0x0055259c`;
0xd08 bytes, the size TrackGame slot 4 allocates). Its constructor
`0x0045c9e0` writes the vtable. Canonical source is
`src/reconstructed/EventManager.h` / `EventManager.cpp`. Names are
provisional. Its `new` calls pass
`D:\aardvark\VC\krusty2\EventManager.cpp` as `__FILE__` (`0x0056a940`),
which confirms the original translation unit's name. The TU runs from
`0x0045c830` to `0x0045fe3f` (strong inference: EcoSystem.cpp code precedes
it, and the `$E` initializer pairs that open the next TU start at
`0x0045fe40`).

TrackGame keeps it at +0x570 (it was the placeholder `TrackGameList`).

## Status

Exact (31 calibration cases):
- the constructor (11 0x50-byte entries at +0x50; -1000 in each component
  of +0x3c4) and both destructors;
- slot 8, which reads "KeepAliveTimeout" (default 20) into +0x2c. With
  +0x30 (1.0) it goes to NetworkInterface 0x004ac8d0 (+0x7c, +0x80): the
  keep-alive thread 0x004af6a0 sends 0xcc and drops a player not heard from
  for +0x7c seconds, and `Sleep`s +0x80 * 1000 ms between 0x4b sends;
- `0x0045d2b0`, `0x0045d2f0` and `0x0045d340`: the first race-mode object of
  TrackGame+0x558..+0x568 present, its +0x34 view and its +0x6c target;
- `0x0045d390`: whether any is present;
- `0x0045d270`: slot 5 on all three;
- slot 10, the per-frame update: while UI interaction is blocked
  (TrackGame+0x3430) it advances the block timer, calls primary slot 7 of
  the +0x424 podium characters, and pans the camera at +0x3d4 by
  `frameTime * speed / 7`; after 7 seconds it lifts the block;
- slots 22 and 23: a press of control 1, 0x1c or 0x39, or any joystick
  button, ends the block once slot 23 has armed it;
- `0x0045e520`: resets the 11 entries;
- `0x0045e550`: the race-start wait; it scans TrackGame's racer slots
  (+0x2228, 0xf8 apart) for readiness and drives the network object;
- `0x0045e600`: finishes an event. Retail keeps a redundant
  `blocked && !pending` early return between the two main branches; the
  source keeps it because the branch layout depends on it;
- `0x0045f180`: awards points from a table by position;
- slot 24, the network messages (`type`, `data`, `from`, `to`, `flags`: the
  NetMessage fields NetworkInterface 0x004aced0 passes): type 5
  (`DPSYS_DESTROYPLAYERORGROUP`, whose `dpId` is at +0x08) and 0x89 mark a
  player done (and in mode 2 without a race-mode object
  call `0x0045fbd0`); 0x86 copies a remote racer's state into the view's
  racer array; 0xcc shows "<name> <text>" (string 0x13d7) and drops the
  player; 0x8e, from the local player id, shows string 0x13d1 or resets the
  entries. The buffers are function-scope (0x8e reuses the 260-byte one
  with a 128-byte limit), and the best-lap update compares through two
  float locals (`fld; fld; fcompp`);
- `0x0045cb70`: resets the event, loads the track's "env" and "scn" data
  and creates the race object for mode 0 (`BaseQuarryEvent`, retail
  `__LINE__` 252) or 2 (`NationalRace`, line 270). It clears three character
  slots in a loop that VC6 fully unrolls (a fresh zero register), and
  returns whether a race-mode object exists;
- `0x0045f9a0`: sends the local racer's state and each AI racer's (type
  0x86, 0x24 bytes), then starts the network wait;
- `0x0045eef0`: the per-frame race-end check. It always finishes after 315
  seconds (a float global at `0x0059af54` that advances while no GUI item is
  open); otherwise it finishes by mode: the event object's +0x70 against
  TrackGame+0x2eb0 (modes 0 and 4), all network racers done or a 30/120
  second grace (1, 2, 3, 5 online), or the local racer's finish (offline).
  The online branch's own finish calls are tail-merged with the shared ones;
- `0x0045cdc0`: ends the race. With 1 it only calls slot 4 on the race
  objects and racers. Otherwise it saves the replay as
  `Record\<scene>_<racer>_<yyyyMMdd>_<HHmm>.vcr`, keeping only file-name
  characters and adding a "M/d/yyyy h:mm tt" description. In mode 4 it
  saves the `.gho` ghost through the view's `0x00524d00`. Then it releases
  the first race-mode object. The racers' slot 4 goes through their virtual
  GameObject base (vbptr at +0x04, matching KrustyBike's RTTI);
- `0x0045e9d0`: ranks the racers into positions (+0x784), by +0x754 with
  `0x0045e930` in modes 1, 2, 3 and 5 online, else by +0x768 or +0x764
  with TrackOverlay's float comparator `0x005199f0`. It copies them into
  the entries and awards points (`0x0045f180`). In mode 2 it also qsorts
  the standings (`0x0045d3d0`) and stores each racer's place in its
  player's entry. The block-scoped arrays share one frame slot. The `== 2`
  test is a one-case switch (`sub eax, 2; jne`);
- `0x0045fbd0`: removes a player, moving the last entry (and, with a
  race-mode object, TrackGame's last 0xf8-byte record) into its place, then
  qsorts the entries with the unsigned comparator `0x0045fbb0`;
- the cdecl comparators `0x0045e930` (standings) and `0x0045d3d0` (racer
  names, through the inline `strcmp` intrinsic);
- `0x0045fce0` and `0x0045fdc0` (both called by the podium scene with a
  rectangle, `0x0045fce0` also by `0x00418d5e` through TrackGame+0x570):
  they query the collision quadtree (`0x0068aba4`). The first
  `dynamic_cast`s each object to `Vegetation`, collects up to 1000 with
  their codes (Vegetation slot 1, `0x00456720`) in a 0x1f40-byte local
  array (`__chkstk`) and removes them after the query; the second calls
  GameObject slot 4 on the GraphicsTest base (+0xc) of each
  `CollisionObject`. The quadtree classes are TU-local views because the
  `src/krusty2` declarations cannot share a TU with these headers;
- `UnknownEventEntry`'s constructor `0x0045c830`, reset `0x0045c840`
  (position 1, empty name from the shared `""` literal `0x00577738`) and
  racer copy `0x0045c8b0` (id, name truncated to 15 characters, the fields
  the header lists; with racer +0x4a0 set it clears them and sets position
  99; the finished flag is 1 when TrackGame+0x2d74 is 0 or 4). The three
  +0x34 dwords are written as separate statements: a 3-iteration loop is
  not unrolled here. Racer +0x750 is declared `float` because the copy
  moves it unchanged into entry +0x08, which TrackRecord.cpp `0x0051f3c0`
  loads with `fld`.

GameObject's slot 10 takes a float frame time. EventManager adds and scales
it, and retyping the declaration leaves GameObject's and KrustyBikeCamera's
code unchanged.

The race-mode objects are `BaseQuarryEvent`s. 0x0045cb70 stores a new
`BaseQuarryEvent` at TrackGame+0x55c and a `NationalRace` (RTTI
`NationalRace : BaseQuarryEvent`) at +0x564. Both are declared in
`src/reconstructed/QuarryEvent.h`. Their views and targets are treated as
GameObjects.
That is inference from their use (slots 4 and 5, the +0x25 flag bits), and
it fits KrustyBike's primary base chain for the views. Their classes are not
established; `src/reconstructed/RaceView.h` declares them.

Not reconstructed:
- `0x0045d480` (4247 bytes): the podium scene. It calls `0x0045cdc0(1)`,
  creates a `PCCamera` (constructor `0x004bed80`) at +0x3d4 and, for places
  1 to min(+0x4c, 3), a `D3DIMSoultreeCharacter` (0x240 bytes, constructor
  `0x004455b0`) at +0x424 from "%s\Winner.mcf" (or "Winnerd.mcf") with a
  "Podium3/4/5_%02d" motion kept at +0x430; it also uses "CrowdLoop.wav"
  and clears the podium area with `0x0045fce0` / `0x0045fdc0`. Heavy x87
  code with many inline vector temporaries; not attempted. A nonzero result makes `0x0045e600` block UI interaction for slot 10.

Near misses (`samples/game/EventManagerNearMisses.cpp`):
- the cdecl progress callback `0x0045cb20` (63 of 67 bytes; retail swaps
  two registers);
- `0x0045e710`, which leaves the race for a menu: it restores the UI and
  640x480x16 and shows a `TransDlg` ("Trans.dtm", retail line 1064).
  454 of 533 bytes; retail picks eax/edx where VC6 here picks ecx/eax
  after the `new`.

Slot 24's message layouts live in `EventManager.cpp`, and the GUI page and
control classes in the near-miss sample. Declaring either in a shared
header changed VC6's register choice in TrackGame slot 1 (an unrelated
`availPhys + availPageFile` sum): header-only type additions can disturb
other translation units, so re-run the full calibration after header edits.

GameObject slot 24 (`0x00469620`, passing all five arguments to the
children in order), Game slot 17 (`0x00468ba0`, forwarding them to +0x2f4's
slot 24) and KrustyUI slot 24 (`0x00499a70`) take
`(int type, void* data, int from, int to, int flags)`, the NetMessage
fields NetworkInterface `0x004aced0` passes. KrustyUI sets the network
object's +0x10 for type 0x101 (`DPSYS_HOST`). The DirectPlay system message
constants are shared in `src/reconstructed/DirectPlayMessages.h`.
The view's +0x38 and +0x3c are racers (`UnknownEventRacer`, in
`RaceView.h`); TrackGame's 0xf8-byte racer records start at +0x215c, after their count.
