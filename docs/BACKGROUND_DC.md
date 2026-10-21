# Background drawing-context selection

## Identity and complete extent

`BackgroundImage::GetDc` supplies the missing DC-selection step beneath dialog
text drawing. The complete strict UIDialog::BeginControlDraw body at
`0x0046ed70` (316 bytes) directly calls it at `0x0046edfa`. BackgroundImage's
class layout, render-target pointer and surface fields are established by the
existing canonical source; semantic member names and original translation-unit
ownership remain reconstructed.

The complete retail body spans **678 bytes**, `0x004049d0..0x00404c75` inclusive.
Its three `ret 28` instructions are at `0x00404c4a`, `0x00404c6b` and
`0x00404c73`; ten NOPs precede ReleaseDc at `0x00404c80`. Every decoded direct
branch stays within this extent. There is no EH registration or switch table.
Three direct calls select existing region/release helpers. Two indirect calls,
at `0x00404c36` and `0x00404c57`, invoke surface GetDC slot 17.

## Functional build review

`samples/render/BackgroundImageNearMisses.cpp` is already a standalone complete
candidate. Its algorithm is unchanged. VC6 SP3 `/O2 /GR /GX /MT` emits
**678 bytes with 115 matching positions** after binding both scratch globals.
It remains nonexact, with zero-register allocation and return-block placement
differences. The old 108/606 masked-byte figure was a different metric; it is
not the complete relocation-resolved score.

A saved same-path object from baseline `938c7031` and the final object have
identical **three nondebug sections and 18 relocations**, including the full
function, data and scratch storage. There are no filename exceptions. The
private reproduction is `work/background-dc36/check_preservation.py`,
`same-path-preservation.json`, `before.obj` and `after.obj`. Only bindings and
source comments changed; no production wrapper or replacement implementation
was introduced.

The sole external requirements are these three existing canonical methods:

| Helper | Retail VA | Complete bytes |
| --- | --- | ---: |
| MarkRegionPending | `0x00404cb0` | 23 |
| SetRegionRect | `0x00404240` | 158 |
| ReleaseDc | `0x00404c80` | 41 |

The retained test proves all three full canonical bodies strict, then compiles
their actual source bodies into an isolated helper object under their original
names. That object must independently pass the same strict comparisons before
execution. Both candidate and reference use these actual helpers. Only the
underlying COM GetDC/ReleaseDC operations are scripted. The complete caller
also remains strict and its emitted source relocation resolves to this method.

## Scratch storage

The sample defines the method's index and rectangle scratch state:

| Source symbol | Retail range | Size |
| --- | --- | ---: |
| g_GetDcRegionIndex | `0x005777a0..0x005777a3` | 4 |
| g_GetDcClipRect | `0x005777a8..0x005777b7` | 16 |

Both retail ranges initially contain zeros in readable, writable `.data`
(characteristics `0xc0000040`). They are **raw-backed bytes**, not the PE
section's zero-fill tail. That observation does not determine their original
COFF section or translation unit. The four-byte gap between the ranges is not
claimed. A private whole-text direct-absolute-reference scan found 15 DWORD
accesses, all in GetDc; this is not proof against possible computed aliases.
The instructions and ranges are retained in `work/background-dc36/global-xrefs.json`.

Ordinary source declarations emit one 20-byte writable `.bss` section: the
rectangle at offset 0 and index at offset 16. The retained test proves exact
size, nonoverlap, zero initialization, section permissions, and absence of
initialization/EH sections. The native fixture checks both globals initially
zero before seeding comparison scenarios. Bindings associate each source
symbol independently with its retail range. No claim is made that source
storage order, inter-symbol gap or final executable layout matches retail.

## Preserved behavior

The method initializes the caller's DC and copy flag before selecting a surface.
When a background copy is valid and frames remain, copyDcNext alternates the
backbuffer and copy surface; only the backbuffer pass decrements frames. With
zero frames, it scans current-frame dirty regions, clips each to the requested
rectangle, and alternates the backbuffer/copy passes using lastDcRegion and
lastDcRegionCopied. Exhaustion resets the region state. The existing helpers
mark pending regions or copy/clamp region rectangles as prescribed.

A null selected surface returns zero. A matching cached surface reuses its DC
without a COM call. Switching surfaces releases the old DC before acquiring
the new one. GetDC's HRESULT is ignored: selecting a nonnull surface returns
one even if GetDC fails, writes null, or leaves its output untouched. The last
case preserves the prior held-DC value unless ReleaseDc has already cleared
it. ReleaseDc ignores its own COM HRESULT and clears its fields after return.
Faults or thrown callbacks retain the stores already performed; no cleanup or
argument validation is added.

## Native evidence and limits

`tests/test_background_dc_behavior.py` compiles the actual standalone candidate
as a separate object and compares it with hash-pinned retail in **21,611 native
scenarios**. It checks the complete CFG, all three direct and two indirect call
sites, real helper/caller bytes, relevant class layouts, scratch storage and
external-symbol closure. Each retail direct-call patch validates its opcode
and original destination before dispatching to the strict source helper.
Retail mapping and patches exist only in the private validation executable.

Every COM boundary and final result compares the complete allocated background
object, target prefix, region table, surfaces, output arguments and both scratch
globals, plus ordered calls, return values and exceptions/fault addresses.
Sequences compare state after every invocation, with up to eight calls. Cases
cover copy/dirty state, signed frame counts, capacities from -1 through 4,
frame indices 0 through 2, region masks, clipping and inverted/empty rectangles,
cached/switching surfaces, positive and negative HRESULTs, written/null/unwritten
DC outputs, callbacks that mutate state, and exceptions at each reached COM
boundary. Twelve alias arrangements include overlapping output integers,
clip/input rectangles, region storage and object fields; invalid-pointer cases
check exact prior state and fault information.

Independent assertions check selected receivers, clipped rectangles, frame
counts, dirty-region helper effects, cache hits without COM calls, release
before acquisition, ignored failures, retained unwritten handles, finite surface
alternation and region exhaustion. Native comparison found no algorithm change
necessary for these contracts.

This is finite source-helper integration with scripted COM surfaces. It does
not exercise real DirectDraw/GDI drawing, complete menus, arbitrary dimensions
or capacities, concurrent/reentrant use, or unrestricted aliasing. The shared
scratch state is preserved; no thread-safety claim or synthetic synchronization
is introduced. Original TU ownership, strict byte identity of GetDc, and a
complete runnable game remain separate questions.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_background_dc_behavior.py -v
```

The admitted real link `work/game-build/build-r6snk7xp` compiles 232 translation
units and removes exactly `BackgroundImage::GetDc`: **311 to 310 unresolved
symbols**, with no newly missing names. The game still does not link; integrated
startup, menus and racing remain unverified.
