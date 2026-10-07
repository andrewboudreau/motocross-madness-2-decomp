# uiinfo.cpp (TrackGameMode)

`src/reconstructed/UiInfo.cpp`, with the classes in `TrackGame.h`. Names
are provisional.

Evidence: the `__FILE__` literal at `0x00575780`, xrefs at `0x00522134`
(the constructor, line 71), `0x00522d2d..0x00522d9f` (lines 383-385) and
`0x00523e81` (line 873). Extent: `0x00521f30..0x00524106`. TypeRegistry.cpp
ends at `0x00521f2b`, and VCR's constructor starts at `0x00524110`. The
first function, the racer-slot constructor `0x00521f30`, is placed here by
adjacency (strong inference).

**Layout (confirmed).** TrackGameMode is 0x2dc0 bytes. `0x005231f0` builds
one on its stack, and the methods use offsets up to +0x2dbc. TrackGame
holds it as the member `mode` at +0x578, because its RTTI shows single
inheritance. The fields at TrackGame+0xfc4..+0x3337 now live in
TrackGameMode, named by their offset there (the TrackGame offset minus
0x578). For example, the old `field_0x2d78` is now
`mode.field_0x27f8.field_0x08`. Inside the mode:
- **Three settings blocks** (`UnknownTrackGameModeSettings`, 0x1ec bytes)
  at +0x27f8, +0x29e4 and +0x2bd0.
- **A session-list wrapper** at +0xa98 holding five SessionInfoType
  records. Both constructor unwind funclets call its implicit destructor
  `0x00522420`.
- **The racer choice and its saved copy** at +0x1974 and +0x1a3c.
- **Eight racer slots** at +0x1be4.

The racer-choice struct is strong inference. Adding the session wrapper
alone made VC6 swap a commutative add in TrackGame slot 1 (`0x00520ab0`).
Any further struct definition restored it, so the racer-choice
definition is a real type, but review it with that in mind.

Exact: 34 calibration cases plus `0x00522720`, all of the unit except two near misses:
- The constructor `0x00522060`, the destructor `0x005225f0`, the profile load
  `0x005231f0`, the network race reset `0x00522680`, the racer-slot
  constructor and the session list's unwind destructor.
- The SessionInfoType constructor `0x00523b90`. Its destructor `0x00523c80`
  is folded with InfoType's, so the name owning that address is
  provisional.
- 0x00522060 writes its random value as
  `(rand() * (1.0f / 32768.0f)) * 999.0f`.
- The rest are the racer-slot name and clear helpers, the option-block
  defaults, string resources, garage tables, directory lists, display-mode
  pick, control-file load and save, profile save, the CD prompt and search,
  file paths, install type, help file, bonus tracks and series accessors.

Three return types now follow retail: `0x00522d00` and `0x00523a60` return
int, and `0x00523d30` returns int and takes ShellExecuteA's parameters.
Their binding keys were renamed in the files that call them.

`0x00522720` clears its last 0x14 bytes (+0x360) with an inline
`memset`. VC6 advances the pointer and takes a fresh zero register, as
retail does.

Near misses (`samples/game/UiInfoNearMisses.cpp`):
- the defaults reset `0x00522440` (362 of 427): retail keeps 1 in ecx and
  hoists `&field_0x27f8`, and the second settings copy is scheduled
  differently;
- the path lookup `0x005238f0` (189 of 379): retail moves fopen's result
  into edx only at the shared found block. Writing the found block twice
  does not get VC6 to merge the copies.
