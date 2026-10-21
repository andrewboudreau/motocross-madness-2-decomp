# UIEditBox::Draw

`samples/ui/EditBoxDraw.cpp` contains the complete readable candidate for the
edit-box drawing operation at **0x004738a0**. It draws the optional background
image, positions text and the caret, requests redraws, and updates the IME
composition position when the display's freeze-frame flag is set. The existing
`GameUiNearMisses.cpp` aggregate includes this one definition.

The method remains **nonexact: 975 candidate bytes versus 973 retail bytes,
206 matching positions after relocation resolution**. This extraction changes
no statements or behavior. The standalone grouping does not establish original
translation-unit ownership.

## Identity and complete extent

The raw UIEditBox primary table is **0x00552de8**, with COL **0x0055c4c0**, object
offset zero, and type descriptor `.?AVUIEditBox@@`. Slot 40 points to 0x4738a0.
The independently strict 286-byte constructor at 0x473630 writes this primary
vptr at 0x473675. The test checks those raw records, the actual header's
four-byte member-function-pointer representation, and relevant object offsets.
Prepared native test storage does not run that constructor.

The decoded body ends with the return at **0x473c6c**; its other return is at
0x473c59. The three bytes before the next entry at 0x473c70 are NOP alignment.
The retained test decodes all 973 bytes, checks every branch destination, all
ten direct call sites and all six indirect call sites, and checks the GDI and
IME import identities. There is no SEH frame in this method. The extent is the
complete retail body, not the candidate's 975-byte denominator or a trimmed
matching prefix.

The six direct game helpers are existing canonical providers:

| Helper | Retail VA | Complete strict bytes |
| --- | --- | ---: |
| UIEditBox::SetEditText | 0x473da0 | 333 |
| BackgroundImage::DrawImage | 0x404480 | 639 |
| UIDialog::BeginControlDraw | 0x46ed70 | 316 |
| UIDialog::DrawAlignedText | 0x46f550 | 354 |
| UIDialog::EndControlDraw | 0x46eeb0 | 75 |
| BackgroundImage::ReleaseDc | 0x404c80 | 41 |

The native fixture also executes these complete strict source helpers:

| Helper | Retail VA | Complete strict bytes |
| --- | --- | ---: |
| UIControl::GetWidth | 0x470410 | 9 |
| UIControl::RequestRedraw | 0x470640 | 21 |
| UIDialog::DrawShadowText | 0x46f3c0 | 93 |
| BackgroundImage::MarkRegionPending | 0x404cb0 | 23 |
| BackgroundImage::SetRegionRect | 0x404240 | 158 |
| PCTextureMap::BlitTo | 0x4c7b00 | 54 |
| DebugMalloc | 0x4a2e20 | 57 |

Each of these thirteen helpers is extracted without body changes for private
linkage, compiled separately, checked against the complete canonical source
object and the retail bytes with resolved relocations. The constructor adds
one further check: **27 strict checks** in total. The extracted SetEditText's
diagnostic filename is independently identified and bound to the canonical
retail filename for comparison; it is not an observed rendering result.

## Source preservation

The fresh aggregate before and after extraction preserves **all 733 nondebug
sections and 3,912 relocations**, including public symbol offsets, storage
flags, code, data, initializers, destination sections and addends. The immediate
same-path comparison has **no exclusions or filename substitutions**.
Private evidence is retained in
`work/connections47/editdraw/live-extraction-proof.json`, with `live-before.obj`
and `live-after-pathfix.obj`. Separate `aggregate-no-include.obj` and
`candidate-no-include.obj` verify the calibration compilation context.

The retained aggregate fingerprint makes only three known source filenames
portable across checkout paths. Its baseline was generated from the saved
pre-extraction object, after proving that the complete unnormalized objects
agree. It does not relax any previous strict or partial function comparison.

The standalone's complete Draw bytes and relocations equal its definition in
the aggregate and its emission through the actual game compile command. Its
nondebug sections are `.drectve` (82 bytes), `.text` (976 bytes, including the
975-byte function and alignment), and `.bss` (one byte for VC6's empty string
literal). There is no CRT initializer or additional recovered function. Relative
project includes allow both aggregate and standalone compilation without the
game build's additional include directories.

## Functional build review

`tests/test_edit_box_draw.py` and
`tests/fixtures/edit_box_draw_compare.cpp` run **14,793 finite native scenarios**
against the pinned retail image and the actual standalone VC6 `/O2` candidate.
Only the fixture uses `/Od`. The source call is explicitly qualified to invoke
the candidate body, avoiding virtual-dispatch indirection through the test
table. Reference patches verify each original E8 opcode and destination before
redirecting it.

The reference Draw body and candidate share the thirteen independently strict
source helpers. This is not execution of an untouched complete retail helper
graph. GetWidth and RequestRedraw execute through observed virtual callbacks.
The complete actual DrawAlignedText body is present; its DrawTextRuns dependency
fails closed if reached. Draw always supplies null runs and alignment flags 9,
so that alternative is outside this operation's tested call contract.

The finite matrix consists of:

- 3,072 cases covering all 64 alignment bitmasks, focus, image/background paths,
  display freeze, stock/custom caret brush, and one/two scripted background passes.
- 260 reached callback mutations: twenty boundary indices by thirteen changes
  to focus, text and width, screen geometry, margins, font, redraw state, brush,
  display freeze, background presence, surface ownership, and shadow/color state.
- Twenty reached scripted exceptions, with exception code, event count and lack
  of normal completion asserted.
- 64 initial-null-text, DC/output, background result and IME context/open-state
  combinations; and 144 further combinations covering COM statuses zero, one,
  and failing HRESULT `0x80004005`, plus zero/success GDI and IME return values.
- 11,200 independent signed-layout oracles: 64 alignment masks, seven widths,
  five measured text widths and five font heights. These require the expected
  scroll, text positions and shadow count, caret rectangle, one balanced
  non-background DC acquisition/release, and normal completion.
- 24 prepared text aliases in edit/dialog storage across image, copy-valid and
  dirty-region states; eight access-fault cases; and one independent initial
  centered/caret-clamp oracle.

The access-fault cases include four guard-page boundaries at successive caret
DWORD stores. They require the same write fault address and independently
assert the completed prefix of the caret rectangle. Other cases cover invalid
dialog, GUI-user and surface pointers, and a text buffer crossing a guard page
after two bytes. The text fault must retain the acquired DC. No exception
handler is added to production; only the fixture catches and records faults.

Comparisons include all prepared control/dialog/user/game/display/render/
texture/background/region/surface/text storage, guarded storage, the complete
observed scratch globals, allocator accounting, native allocation contents,
ordered callback arguments and before/after state snapshots, completion,
return, exception code and access kind/address. All ordinary cases require
normal completion and return one, in addition to differential equality.
The scratch rectangle pointer is classified as null, global scratch, or the
caller's temporary rectangle; temporary stack addresses themselves are not
compared. The rectangle values consumed by callbacks are recorded.

The dialog buffer now covers the complete header size, **0x7f58 bytes**, with
compile-time checks for `drawClipRegion` at **+0x7f00** and `previousFont` at
**+0x7f04**. The earlier 0x580-byte fixture omitted these actual BeginControlDraw
writes from its reset and snapshots. This test-storage correction leaves the
production candidate unchanged. All 14,793 scenarios run with the enlarged
buffer; the 11,200 signed-layout cases also independently require font restoration,
clip-region deletion, and the expected final values in both late fields.

### Finite boundaries and limitations

The lower **BackgroundImage::GetDc** operation is scripted; it is separately
reviewed in [BACKGROUND_DC.md](BACKGROUND_DC.md). Here it always writes the DC
and `more` outputs and records prepared ownership state. The original Draw,
BeginControlDraw, EndControlDraw, DrawImage and ReleaseDc logic executes.
Actual surface COM methods and fourteen native GDI/IME imports are scripted.
There is no real device, font rasterizer, window or IME interaction, no actual
menu rendering, and no assertion of whole-game startup or UI lifetime behavior.

The platform contracts explicitly supply initialized outputs even on selected
failures. In particular, text measurement writes SIZE when returning FALSE,
and surface GetDC writes its output for nonzero/failing HRESULTs. Unwritten
outputs and invalid external handle implementations are not covered. The IME
callback observes only initialized composition style and current position;
the unused, uninitialized `rcArea` is never compared. Production does not gain
extra zero-initialization.

Null-text cases execute real DebugMalloc and the linked native VC6 CRT allocator,
with prepared accounting state and a 64-byte allocation. The fixture primes
the native heap and requires equal allocation addresses and all 64 bytes in
these finite sequential runs. This is not allocator replacement or general
heap-reuse/low-memory evidence; allocation failure is not tested. The fixture
does not validate every possible string, signed-overflow input, asynchronous
callback, repeated API scheduling, or exception-context resumption.

The individually reviewed candidate is explicitly admitted by the source-game
configuration. The isolated actual build `work/game-build/build-rc6bus6k`
compiled **253/253 translation units** and reduced unresolved names from
**246 to 245**, removing only `UIEditBox::Draw` and adding none. All 251 prior
production objects remained identical under the separately reviewed composition
changes. This is provider admission and link progress; the game still does not
link, and startup, integrated menus and racing remain unverified.

Reproduce from the repository root:

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH='.'
python -m unittest discover -s tests -p test_edit_box_draw.py -v
```
