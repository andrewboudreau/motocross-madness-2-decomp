# uiinfo.cpp (TrackGameMode)

`src/reconstructed/UiInfo.cpp`, with the classes in `TrackGame.h`. Names
are provisional.

Evidence: the `__FILE__` literal at `0x00575780`, xrefs at `0x00522134`
(the constructor, line 71), `0x00522d2d..0x00522d9f` (lines 383-385) and
`0x00523e81` (line 873). Extent: `0x00521f30..0x00524106`. TypeRegistry.cpp
ends at `0x00521f2b`, and VCR's constructor starts at `0x00524110`. The
first function, the racer-slot constructor `0x00521f30`, is placed here by
adjacency (strong inference).

**Layout finding (confirmed).** TrackGameMode is really 0x2dc0 bytes.
`0x005231f0` builds one on its stack, and the methods use offsets up to
+0x2dbc. So TrackGame's members +0xfc4..+0x3337 belong to TrackGameMode.
That range includes the five SessionInfoType records at mode+0xa98 and the
eight racer slots at mode+0x1be4. TrackGame's RTTI is single inheritance,
so TrackGameMode is a member, not a base. To keep the existing layout and
the field names other files use, UiInfo.cpp reaches those members through
`UnknownModeOwner(this)`, the owning TrackGame; VC6 folds this to the same
displacements. Moving the fields into TrackGameMode would touch about 500
references in about 24 files. It would unblock the constructor, the
destructor, `0x00522680`, `0x005231f0`, `0x00521f30` and `0x00523b90`.

Exact: 26 calibration cases. These are the racer-slot name and clear
helpers, the option-block defaults, string resources, garage tables,
directory lists, display-mode pick, control-file load and save, profile
save, the CD prompt and search, file paths, install type, help file, bonus
tracks and series accessors.

Three return types now follow retail: `0x00522d00` and `0x00523a60` return
int, and `0x00523d30` returns int and takes ShellExecuteA's parameters.
Their binding keys were renamed in the files that call them.

Near misses (`samples/game/UiInfoNearMisses.cpp`): the defaults reset
`0x00522440`, `0x00522720` and the path lookup `0x005238f0`. They differ
in constant registers, store scheduling and register use.
