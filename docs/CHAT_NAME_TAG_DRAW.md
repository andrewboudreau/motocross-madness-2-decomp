# ChatOverlay name-tag drawing

`ChatOverlay::DrawNameTag(void*, int)` is in [TrackOverlay.cpp](../src/reconstructed/TrackOverlay.cpp) (retail `TrackOverlay.cpp`), in retail
order after `0x0051e390`, and is a calibration case. It draws the
cue/racer text and updates the corresponding
`NameOverlay` width and source rectangle. The source emits **974 bytes**,
all **974 matching retail** with relocations resolved.

## Complete extent and receiver

Retail begins at `0x51e3f0`, ends with `RET 8` at `0x51e7bb`, and includes its
last two return-immediate bytes through `0x51e7bd`. All decoded branches land
inside this full extent. Two padding bytes precede the independent strict
50-byte `CycleNameTagShade` at `0x51e7c0`. The method has no local exception
registration or cleanup funclet. The eight byte differences are register
choices around the racer-value and position formatting branches; no source
rewrite was made to improve that score.

This nonvirtual method is identified through its actual calls, not a shared
vtable address. Complete strict `RedrawNameTags` (`0x51e910`, 308 bytes) calls it
at `0x51e9d2` and `0x51e9ec`, passing the same primary `ChatOverlay` receiver.
That caller obtains and releases the surface DC. `DrawNameTag` itself only
selects/restores its font; it does not acquire or release a DC.

The raw `ChatOverlay` table is `0x558854`, with COL `0x55f838`, object offset
zero and literal name `.?AVChatOverlay@@`. Its strict 282-byte constructor
writes that table at `0x51cdfc`. The allocation sequence in `CreateOverlays`
pushes `0x3e0` at `0x4e08e4`, calls the allocator at `0x4e08eb`, and calls the
constructor on the returned primary pointer at `0x4e090a`.

`NameOverlay` has primary table `0x55860c` and COL `0x55f690`. Its 117-byte
constructor and 242-byte `Create` method are strict. The decoded
`CollectRacers` producer allocates `0x174`, calls those methods, and stores the
returned primary pointer into `nameTags`, starting at `ChatOverlay+0x2d8`.
The extra cue-tag chain stores the same kind of pointer at the count index.
`CollectRacers` (586 bytes) is exact in TrackOverlay.cpp too; these
instruction/producer facts are checked independently of it.
Field names and the partial racer view remain provisional; the regression
pins the actual used layouts independently.

## One shared mutable flag

The DWORD at retail `0x68a444` is in the zero-filled virtual tail of writable
`.data`, with no file-backed bytes. `DrawNameTag` reads it to select the numeric
value branch; canonical `ChatOverlay::RenderPre3D` also reads it. The existing
canonical definition now has ordinary C++ external linkage at its original
position:

```cpp
int s_UnknownGlobal68a444;
```

Its original `.bss+0x40` offset remains unchanged in the 72-byte section with
flags `0xc0400080`. Only the visibility and decorated name change, from local
`_s_UnknownGlobal68a444` to external `?s_UnknownGlobal68a444@@3HA`. It remains
writable storage, not a constant false branch or a new duplicate flag.

The behaviour test links a validation unit sliced from the canonical file
(`tests/track_overlay_unit.py`): the unit's includes, its GDI import
declarations, this flag definition and `DrawNameTag`. The unit therefore
carries the flag as four private zeroed `.bss` bytes; there are no
include-context macros or linker aliases.

The validation unit's `DrawNameTag` has the same complete bytes and named
relocations as the canonical unit's. The canonical unit's complete object
(**231 nondebug sections, 1031 relocations**, with `TrackOverlay.cpp`
literals named by path) is pinned and reproduced by the game compile
command.

## Actual CRT and platform boundaries

The three direct calls, `0x51e439`, `0x51e568` and `0x51e5a1`, call `sprintf`
at `0x534aaf`. Copies, concatenations and length scans elsewhere are emitted
inside the complete candidate body. There are no other direct game helpers.
The six indirect calls use exactly three PE imports: GDI32 `SelectObject` and
`GetTextExtentPoint32A`, and USER32 `DrawTextA`.

The native fixture runs the real linked VC6 `sprintf`. The `LIBCMT.LIB` member
`build/intel/mt_obj/sprintf.obj` is independently checked over its complete
82-byte wrapper, with its named `__output` and `__flsbuf` relocations resolving
to retail `0x539359` and `0x539241`. At runtime, all 82 wrapper bytes are checked
apart from those two relocation operands, whose actual native destinations
are checked against the linked `_output` and `_flsbuf` symbols. The reference
redirects only its three verified `sprintf` calls to this same native routine.
This establishes the wrapper and a shared actual CRT formatting boundary;
it does not claim byte identity of the entire internal CRT formatting graph.
The finite tests set the C locale explicitly.

Both sides use the same scripted GDI callbacks. The fixture verifies original
retail call/IAT targets before installing them. `GetTextExtentPoint32A` always
writes both `SIZE` fields, including scripted failure returns. `DrawTextA`
records text, flags and rectangle values and can mutate the rectangle.
`SelectObject` tracks the selected font and supports scripted null/error
returns. These explicit initialized-output contracts do not establish arbitrary
unwritten Win32 failure outputs, real glyph measurement or real GDI handles.

## Native validation

`tests/test_chat_name_tag_draw.py` protects **97 strict game cases**, the
additional strict CRT wrapper, the canonical object and **5,864 native scenarios**.
The matrix includes 64 access-violation outcomes and eight scripted exceptions:

| Cases | Contract |
| --- | --- |
| 1,008 | Cue/racer branches, counts 0/1/3/10/11, name/position options, seven signed positions and four finite string lengths |
| 2,250 | Signed source/measured/prior widths, equality thresholds, odd margins, signed extrema and rectangle origins |
| 18 | Two branches, three font-selection outcomes and three measurement/draw statuses |
| 201 | Numeric branch, exact quarter values from -25 to 25, independent decimal-string oracle |
| 2 | DrawText rectangle mutation: cue local rectangle versus racer stored rectangle |
| 144 | Twelve finite raw float patterns, three x87 precisions and four rounding controls, exceptions masked |
| 6 | Prepared writable overlaps between tag fields and overlay rectangles |
| 120 | Fifteen mutations at each of four reached callbacks in both branches |
| 8 | An exception at each reached callback in both branches |
| 55 | Null/invalid receivers, game/racer/tag pointers and progressive guarded field/string availability; two valid guard endpoints complete |
| 4 | Progressive source-rectangle writes reaching a guard page after 0/4/8/12 bytes |
| 2,048 | Deterministic varied finite positions, widths and strings, with independent output oracles |

The independent string oracle builds integers and exact quarter decimals using
integer arithmetic rather than `sprintf`. It checks the cue/racer DrawText
flags `0x8124`/`0x8125`, text, lengths, width clamping, strict `width+4 < sourceWidth`
threshold and signed half-margin behavior. Unsigned arithmetic models the
observed x86 wrap for the bounded extreme-width checks. This does not assert
portable C++ behavior for every possible overflowing signed input.

The cue branch draws before measuring and uses a local rectangle. The racer
branch measures first and passes its stored rectangle to DrawText. Separate
mutation oracles protect that distinction and the resulting stored source
rectangle. Font oracles check restoration of the initially returned font even
when the member or selected-font state changes during a later callback.

Every requested callback mutation must be reached. Invalid replacement tag or
racer pointers have explicit fault-versus-completion, access-kind/address and
font-state oracles. Ordinary prepared cases must complete without an exception.
Guard comparisons include all accessible memory and the exact AV address/kind.
The progressive-write oracle independently checks the entire 4KB accessible
page, including the valid rectangle prefix and untouched bytes. Exceptions do
not invent font restoration or the outer caller's DC release.

The fixture snapshots full declared `ChatOverlay` and `NameOverlay` storage,
the full prepared `TrackGame` buffers, the declared partial racer views, both
guard pages, global game pointer/flag, and selected-font state at callbacks and
completion. The actual used sizes/offsets are separately compiled against the
real headers. Raw prepared storage does not establish constructors, lifetimes
or full class equivalence for provisional views. The validation unit's copy
of the canonical flag definition is checked as zeroed writable 4-byte BSS.
Actual game storage ownership is covered by the canonical-object pin above.

All local strings stay within the two 128-byte method buffers; the longer cue
member does not justify unbounded formatting. Access-fault cases stop at guarded
reads rather than claiming arbitrary overflowing buffers. FP status/control are
compared without tolerances, with exceptions masked; NaNs, infinities, unmasked
FP exceptions and exception resumption are not established. Scripted GDI and
prepared state do not demonstrate integrated HUD rendering, startup or racing.

## Functional build review

DrawNameTag is exact and compiles as part of TrackOverlay.cpp in the
functional build. Its game dependencies are the canonical flag and
`g_TrackGame`; the other imports are the actual CRT and OS functions listed
above. This is not integrated HUD or racing validation.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_chat_name_tag_draw -v
```

No private objects, retail bytes, compiler libraries or native executables are
part of the source distribution.
