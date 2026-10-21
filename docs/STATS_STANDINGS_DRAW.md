# StatsOverlay standings drawing

`samples/track/StatsStandingsDraw.cpp` supplies the complete
`StatsOverlay::DrawStandings` operation used by the race standings overlay.
It remains a reviewed **nonexact** candidate: authentic VC6 SP3 emits 1,004
bytes against the complete 1,002-byte retail function at `0x00519a20`, with
605 relocation-resolved matching positions. Original translation-unit
ownership and the semantic member names remain provisional.

## Identity and complete extent

The retained test decodes every byte through `0x00519e09`. It checks all
three returns (`0x00519c99`, `0x00519dd9`, `0x00519e09`), all branch targets,
ten direct calls and ten indirect calls. Six alignment NOPs follow the final
return; they are outside the function. No extent is shortened to fit the
candidate, and this body has no SEH registration or cleanup funclet.

Raw RTTI identifies the primary StatsOverlay table at `0x00558680`, its
COL at `0x0055f6e8`, and its zero-offset base relations. The constructor at
`0x00519370` installs that table. `CreateOverlays` allocates `0x9b8` bytes
at `0x004e0a39` and calls the constructor at `0x004e0a5d`; the existing
`Redraw` calls DrawStandings at `0x005198b7`. The fixture separately pins
the actual VC6 class size, method-pointer ABI and accessed field offsets.
These facts establish the receiver and invocation; they do not make every
provisional field name an original source name.

The two iterator calls now use the existing `BikeRace::NextBike` provider
at `0x004204e0`, through an explicit receiver and return-view cast. The
primary BikeRace identity, `localRacer` at `+0x38`, and the actual iterator
body support this narrow connection. The returned racer remains a local
view. In particular, the enabled flag is reached through its virtual-base
pointer, rather than treating the flag as a primary-object field. Raw
KrustyBike constructor stores, the `0x005546ec` virtual-base table
`(-4, 0x1608)`, and the GameObject COL at offset `0x160c` are checked.
The prepared native racer windows reproduce that layout. This is not a
claim of complete equivalence between all racer view declarations.

## Extraction and object preservation

The old aggregate includes the standalone body at its original position.
Same-path before/after compilation preserves all **37 sections and 162
relocations**, allowing only the two iterator targets at candidate offsets
172 and 220 to change from `UnknownKrustyBikeView::NextBike` to
`BikeRace::NextBike`. Every other function, static initializer, section
attribute, storage byte and relocation is retained. The separate
`UnknownChatView::NextBike` call in the aggregate remains unchanged.
Canonical `TrackOverlay.cpp` is unchanged and its complete object is also
pinned by the regression.

The new standalone object has five nondebug sections and 27 relocations:
40 bytes of compiler directives, a 1,008-byte code section containing the
1,004-byte body plus padding, two writable string sections of 14 and three
bytes, and a four-byte read-only `60.0f` constant. It introduces no BSS,
CRT initializer or duplicate runtime state. The test compiles both the
standalone and aggregate without project include paths, then compares the
standalone object with the actual game-build command output.

Existing NameTag, visibility and clipper aggregate pins were updated only
after independently reproducing their published old pins and proving the
same two relocation changes. Their canonical and camera records remain
unchanged. Private reproduction evidence is in
`work/connections51/stats/live-extraction.json` and
`integration-pin-proof.json`; retained pins are in
`tests/fixtures/stats_standings_objects.json`.

## Executed helper graph

The fixture executes the actual standalone candidate and these source
helpers. Each extracted helper is compared against both its complete
retail extent and its canonical source object, including named relocation
targets and addends.

| Helper | Retail address | Bytes |
|---|---:|---:|
| BikeRace::NextBike | `0x004204e0` | 162 |
| FormatMinutesSeconds | `0x00518640` | 67 |
| CompareScoresDescending | `0x005199f0` | 43 |
| TrackGame::LoadResourceString | `0x00521970` | 189 |
| PCTextureMap::LockSurface | `0x004c79e0` | 99 |
| PCTextureMap::UnlockSurface | `0x004c7a50` | 25 |
| PCTextureMap::Upload | `0x004c7640` | 385 |
| PixelFormatBytesPerPixel | `0x00511970` | 78 |
| Overlay::TintRows | `0x004b6880` | 484 |

The previously reviewed nonexact
[RestoreRect](OVERLAY_RESTORE_RECT.md) also executes unchanged. Its
365-byte source body is checked at 77 matching positions. Test-only
qualified-call adapters dispatch the three PCTextureMap virtual methods
to their actual source bodies; they prevent accidental recursive virtual
dispatch and do not substitute rendering algorithms.

Only the validation copy of DrawStandings has its direct calls redirected
to this shared source/helper graph and native CRT. The candidate is not
patched. This compares whole-method behavior with common verified helpers;
it does not execute an untouched retail rendering graph.

The actual linked VC6 CRT executes sorting, formatting, conversion and
memory/string operations in the C locale. Full `qsort` (340 bytes),
`shortsort` (78), `swap` (44), `sprintf` (82) and `_ftol` (39) are checked
against retail. Native checks verify qsort's internal destinations and the
sprintf wrapper's `_output`/`_flsbuf` destinations. The formatter's entire
internal graph is not claimed retail-identical. The two runs share the
same linked native CRT. The test reports **30 strict comparisons**:
18 dual helper comparisons, seven constructor/loader/caller comparisons,
and five CRT comparisons.

## Native behavior and limits

`tests/fixtures/stats_standings_compare.cpp` compares complete prepared
state, pixels, guard windows, global selection, callback transcripts,
return/completion state, access-violation address and kind, and floating
point control/status. State is captured at callback boundaries as well as
on return or exception. Ordinary cases explicitly require completion
without exceptions; intentional faults have independent classification
and partial-state assertions.

There are **3,228 scenarios**, including **78 access violations** and
**26 scripted exceptions**:

| Matrix | Cases |
|---|---:|
| 1–11 racers, every local position, score order, disabled nonlocal racer, cached/changed rows | 2,024 |
| Seven race modes, resource/GDI return statuses and cached rows | 56 |
| Three pixel formats, three pitches, whole/strided copy and 0–4 rows | 90 |
| DC/release statuses, upload failure returns, color key and extra mip level | 72 |
| Lock/unlock failure masks | 256 |
| Two prepared backing-buffer aliases and copy shapes | 4 |
| Ties, finite fractional scores, three precisions and four rounding modes | 168 |
| Callback-discovery baseline | 1 |
| Fifteen mutations at each of 26 reached boundaries | 390 |
| Scripted exception at each boundary | 26 |
| Invalid receiver/global/owner/virtual-base/texture pointers | 6 |
| Progressive source/destination copy faults | 16 |
| Progressive sprintf destination faults | 11 |
| Nine finite time values across precision/rounding controls | 108 |

Independent row oracles build expected decimal strings without using
qsort or sprintf, verify ordering, the six-row cap, local-racer inclusion
and cached early returns. Independent pixel oracles model copy, scripted
text marks, actual tint conversion and upload. Additional assertions check
rectangles, fonts, DC arguments, resource fallback, partial DWORD copy
prefixes, formatting prefixes and locks retained when execution faults.
Global-pointer mutation switches to safely different race-mode/time fields;
an independent expected row/count assertion exposes subsequent reloads.

The platform boundary is scripted. GetDC always writes its prepared output,
including when returning nonzero or `0x80004005`. LoadString and text
measurement always write prepared text/SIZE even when returning failure.
SelectObject has a successful prepared font lifecycle. DrawText records
arguments and writes a deterministic white pixel; it does not rasterize
real fonts. DirectDraw surfaces implement finite lock, DC, blit, color-key
and attached-surface contracts. Actual source upload and tint code run
against those surfaces. These tests do not establish real GDI, device,
resource loading or a complete rendered game frame.

Inputs have at most eleven returned racers, at least one enabled local
racer, bounded initialized strings and finite scores/times. The complete
StatsOverlay, TrackGame and texture windows and the relevant racer storage
are prepared directly; constructors and lifetime are not exercised by the
native fixture. Floating-point exceptions are masked, with three precision
and four rounding settings. Missing-local, empty, oversized, NaN, infinite,
unmasked-FP and unbounded copy/string cases are not validated. The two
overlap cases describe finite emitted VC6 behavior, not portable overlapping
memcpy semantics. Fault comparisons cover observed partial memory, lock,
font and DC state, not arbitrary register context or resumption.

## Functional build review

The standalone remains in `samples/` and is explicitly admitted as a
reviewed nonexact candidate. Isolated source build `build-tvjli7lk`
compiled **259/259 translation units**, reducing unresolved names from
**227 to 226**: only `StatsOverlay::DrawStandings` disappeared, with none
added. Independent game-command recompilation reproduces the new object's
five sections and 27 relocations. All 258 prior game objects preserve
10,685 sections and 45,748 relocations under the separately verified
camera and collision-zero connections. This supplies a missing source
operation; the game still does not link and no playable-race claim follows.

Reproduce on Windows with the private toolchain and retail image:

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH = '.;tests'
python -m unittest test_stats_standings_draw -v
```
