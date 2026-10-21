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
  `0x005231f0`, the network race reset `ResetNetworkRace` `0x00522680`, the racer-slot
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

Three return types now follow retail: `0x00522d00` and `FindFileDirectory`
`0x00523a60` return int, and `OpenHelp` `0x00523d30` returns int and takes
ShellExecuteA's parameters. `CopySeriesDirectory` `0x00523bb0` copies the
current series' directory name.
Their binding keys were renamed in the files that call them.

`0x00522720` clears its last 0x14 bytes (+0x360) with an inline
`memset`. VC6 advances the pointer and takes a fresh zero register, as
retail does.

Near misses (`samples/game/UiInfoNearMisses.cpp`):
- the defaults reset `0x00522440` (383 matching positions, 427 bytes): retail keeps 1 in ecx and
  hoists `&field_0x27f8`, and the second settings copy is scheduled
  differently;
- the path lookup `0x005238f0` (190 matching positions, 357 candidate bytes
  versus 355 retail bytes): retail moves fopen's result
  into edx only at the shared found block. Writing the found block twice
  does not get VC6 to merge the copies.

## Functional build review

The standalone `samples/game/UiInfoNearMisses.cpp` translation unit contains
only these two complete bodies. It includes canonical declarations and supplies
no duplicate canonical definitions. Its game-owned dependencies are the
existing defaults helpers, display-mode chooser, CD search/prompt helpers and
`g_TrackGame`; its remaining imports are CRT string and file functions. This
grouping does not reconstruct or claim ownership of the complete original
uiinfo.cpp translation unit. Both bodies remain nonexact candidates.

The full decoded reset spans `0x00522440..0x005225ea` inclusive (427 bytes),
ending in its return before five padding bytes. The full file lookup spans
`0x005238f0..0x00523a52` inclusive (**355 bytes**). Its two complete `ret 8`
instructions start at `0x00523a41` and `0x00523a50`; the latter is followed by
13 padding bytes before the next function at `0x00523a60`. The older 353-byte
extent truncated the final return operand by two bytes. The native test follows
every branch and fallthrough and requires that the reached instruction bytes
cover both complete extents. No source body was changed for this review.

`ResetDefaults` calls ten helper boundaries, including three garage resets.
The display chooser uses `g_TrackGame->mode` as its receiver while writing the
display index belonging to the mode being reset; those need not be the same
object. It then sets race defaults, copies the complete 0x1ec-byte settings
block to both saved variants, clears only bit zero of the bike-unlock byte,
sets camera defaults and four video-detail entries, clears six bonus flags and
sets difficulty and plate number. Fields not written by these operations retain
their existing contents. The different store/copy scheduling does not introduce
additional dependency calls.

`FindDataFile` clears the output before computing path prefixes. Prefix checks
are case-insensitive and do not require a separator boundary; matching the
installed directory, including an empty prefix, skips its length plus one
character. A nonempty CD prefix receives the same treatment. It tries the
installed path with `fopen(..., "r")`. On failure, install type 2 stops the
search; other types call `FindCdDirectory`, then `WaitForCd` only if the first
helper returns zero. A successful helper permits a second open using the
current CD directory and the previously computed suffix. A found path is copied
to the output and its file closed; a close error does not change the success
result. These details follow decoded retail, including their unusual alias and
empty-prefix behavior; no path normalization or new error handling is added.

`tests/test_uiinfo_behavior.py` compiles the unchanged whole candidate TU and
separate native fixtures with authentic VC6 SP3 `/O2 /GR /GX /MT`. It compares
both complete bodies with the hash-pinned private retail image. Only decoded
direct dependency calls in those ranges are redirected inside the fixture;
production does not map or dispatch to retail code.

The scripted executable passes **21,332 comparisons**:

- 1,152 defaults cases vary the initial bytes, use local and either global mode
  as the receiver, and switch the global game during helper callbacks. The
  fixture observes helper receiver identities, argument offsets, call order
  and complete mode/global object bytes. Helpers write distinct deterministic
  patterns, exercising the later stores and settings copies. Independent
  assertions check key defaults, both whole-block copies and all six bonus
  flags.
- 20,180 file-search cases cover case variants, empty directories and names,
  matching and non-boundary prefixes, all open-success combinations, install
  types, positive/negative CD-helper results, changed directories during search,
  ignored close failures and output/name/directory aliases. The fixture records
  complete open paths, verifies mode `"r"`, checks the file identity passed to
  close, and observes the output at each callback. Complete object and string
  buffers are compared after each run.

A second executable links the same candidate object against the **actual VC6
CRT `fopen` and `fclose`**. Its 192 cases use real installed, CD and changed-CD
directories containing distinct files, plus a missing file. All files are
created under Python's private temporary directory. These cases compare the
complete result/state against retail, independently assert the expected path
or empty failure output, and require exclusive file access afterward to check
that the lookup left no open file handle. CD discovery and prompting remain
scripted; the fixture does not interact with real optical drives or user dialogs.

The native proof is finite. Defaults helper internals are covered by their
separate canonical byte matches, not reimplemented as production substitutes
by this fixture. String cases stay within the retail 260-byte temporary buffer;
they do not establish safety for overlong paths, arbitrary invalid pointers or
all possible callback side effects. The candidate's measured 427/357-byte
bodies remain distinct from a strict 427/355-byte reconstruction. Admission to
the source build and successful startup remain separate checks.

Reproduce in PowerShell with private inputs:

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_uiinfo_behavior.py -v
```
