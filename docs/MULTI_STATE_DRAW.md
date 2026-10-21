# Multistate menu drawing

`UIMultiState::Draw` supplies the missing primary virtual slot 40 used to draw
a menu control's selected image and text. The unchanged readable candidate is
`samples/ui/MultiStateDraw.cpp`, included at its former position in
`GameUiNearMisses.cpp`. It remains nonexact source; extraction does not establish
original translation-unit ownership or promote the provisional field names.

## Full extent, identity and preservation

The complete retail body is **672 bytes**, `0x00478570..0x0047880f`. It contains
returns at `0x004787c9` and `0x0047880f`. Every decoded branch stays within this
interval. The next entry at `0x00478810` is the independently strict 66-byte
`UIMultiState::GetStateTexture`. The old comment's 682-byte figure describes the
candidate, not the retail body; those next ten retail bytes are not part of Draw.

The candidate emits **682 bytes with 139 matching resolved positions** against
the full 672-byte target. Its complete bytes and relocations equal the original
aggregate's function. No algorithm, flags, pragmas, assembly or function extent
was altered to improve this score.

The retained test checks raw COL `0x0055c620`, primary table `0x00553210`, the
literal UIMultiState type name, object offset zero, the UIControl base relation,
and slot 40's actual pointer. The complete 112-byte constructor writes this
primary table. The resource parser's allocation argument at `0x0046c2af` is
`0x1f8`; the returned pointer becomes ECX before its constructor call at
`0x0046c2d5`. The complete 92-byte SetStateEntry body supplies additional state
producer evidence. These are identity/layout checks, not execution of that
large resource parser or the constructors.

Actual UIControl RenderPre3D (`0x00471140`, 83 bytes) and RenderPost3D
(`0x004711a0`, 146 bytes) dispatch through slot 40 at `0x0047117b` and
`0x004711de`. Both full callers remain strict. This connects the candidate to
an existing menu drawing path; it does not establish an integrated menu run.

Private same-path before/after objects preserve **all 733 nondebug sections and
3,912 relocations**, including section flags, public symbol offsets, data,
initializers and every other strict or partial aggregate body, with no filename
exclusions. The retained portable pin normalizes only the enumerated complete
source filenames using the existing object-view utility. The standalone's two
nondebug sections are `.drectve` (40 bytes) and `.text` (688 bytes including
alignment); it adds no data or CRT initializer. It has one exported method and
only six existing game-method imports plus the ordinary CRT `__ftol` and
`__fltused` symbols.

Both the standalone and aggregate compile without extra project include paths.
The test also compares the standalone's complete object with the actual game
build command, including all relocation destinations and addends.

## Actual helper execution

All six direct game helpers execute unchanged readable source:

| Helper | Retail VA | Complete bytes |
| --- | --- | ---: |
| BackgroundImage::DrawImage | `0x404480` | 639 |
| UIDialog::BeginControlDraw | `0x46ed70` | 316 |
| UIAnim::GetCurrentFrame | `0x472f80` | 8 |
| UIDialog::DrawAlignedText | `0x46f550` | 354 |
| UIDialog::DrawControlText | `0x46f6c0` | 454 |
| UIDialog::EndControlDraw | `0x46eeb0` | 75 |

Their real source closure also executes DrawShadowText (93), DrawTextRuns (293),
the four selected-state accessors (20 each), BackgroundImage ReleaseDc (41),
MarkRegionPending (23), SetRegionRect (158), and PCTextureMap BlitTo (54).
Each of these **16 helpers** is compared in its canonical source object and
its validation-only extraction against the complete retail body. Complete
named relocation signatures must also agree between the source contexts.
Together with five producer/caller/boundary comparisons, this is **37 strict
checks**. The selected-state virtual accessors use explicit qualified calls in
validation-only wrappers; no substituted state-resolution algorithm is used.

The reference's six direct game calls are redirected to those same source
helpers after checking their E8 opcodes and original destinations. This is a
shared strict-helper differential test, not an untouched retail helper graph.
The two retail conversion calls at `0x4785e9` and `0x4785fd` use the linked VC6
`_ftol`; the fixture first requires its complete **39-byte** body to equal retail
`0x534a88..0x534aae`. No approximate float conversion is substituted.

Only lower platform operations are scripted: render-target Blit, surface
GetDC/ReleaseDC/Blt, GDI calls, and BackgroundImage::GetDc. Begin/EndControlDraw,
image-region state, actual text layout and lower source ownership decisions
execute normally. All eight GDI imports must be hooked successfully. The
candidate's one indirect call is the actual render-target slot 3 boundary at
`0x4787b7`; all direct and indirect call sites are retained evidence.

## Native state and independent outcomes

`tests/test_multi_state_draw.py` requires **16,181 native comparisons**:

| Cases | Finite matrix |
| ---: | --- |
| 256 | Image, background, source-blit, selected-state/own text, text runs, two states, one/two draw passes |
| 8,960 | Signed dimensions and margins, both text paths, all 64 alignment values, five font heights, shadow on/off |
| 1,575 | Signed image dimensions and exact quarter-scale values, independent scaled-rectangle and color-key flag oracle |
| 80 | Actual image helper's redraw-frame transitions, negative/zero/positive frames, dirty/copy-valid and color-key combinations |
| 384 | Begin failure, null DC, GDI FALSE, render Blit failure, zero/nonzero/failing HRESULT across image/text paths |
| 1 | Baseline establishing all 35 reachable callback boundaries |
| 455 | Thirteen mutations at each of those 35 reached boundaries |
| 35 | Scripted exception at each reached boundary |
| 8 | Explicit valid text/field overlap configurations |
| 8 | Absent state-table paths that return without drawing |
| 18 | Negative, zero and positive finite text lengths |
| 4 | Independent text-run tab, color, shadow, byte and final scratch-state oracles |
| 288 | Twelve finite raw float patterns, signed dimensions, all three precision and four rounding controls |
| 4,096 | Deterministic varied finite raw float scales and signed dimensions |
| 11 | Invalid/guarded table, frame, dialog, surface and text accesses |
| 2 | Independent exceptions before DC acquisition and after acquisition but before font assignment |

The actual source candidate/helpers use `/O2`; only the fixture uses `/Od`.
The float cases mask all x87 exceptions, compare final `_statusfp` exception
flags and control settings, and include denormals and finite scales whose
products/conversions overflow. They do not establish unmasked exception
resumption, x87 register-stack context, NaNs/infinities or all float inputs.
The independent scale oracle uses exact quarter arithmetic on moderate signed
integers. Other float cases use full differential observations without a numeric
tolerance.

The fixture compares prepared receiver/state/image/frame/region storage,
ordered callback arguments and text bytes, per-boundary before/after snapshots,
all relevant source scratch globals, result/completion and exact AV kind/address.
The clip pointer is compared directly: this method passes its prepared receiver's
screen rectangle to BeginControlDraw, which may select the global scratch
rectangle. Callbacks also record consumed rectangle values. Both dialog buffers use actual
`sizeof(UIDialog)` (**0x7f58**), so font and clip writes at `+0x7f04/+0x7f00`
are reset and observed. Native layout assertions cover every hardcoded field,
including color-key `+0x30`, font handle, dialog font, state-table count, selected
state, image/frame arrays and text-run layout. This closes the smaller dialog
window inherited during the initial private prototype; the smaller window is
not the final retained proof.

Independent oracles require signed text/image coordinates, actual image flags
`0x1000000`/`0x1008000` at both render-target and COM boundaries, normal completion
for ordinary prepared cases, redraw restoration, expected text-run commands,
DC acquisition/release and held counts. Signed-layout cases also require the
saved font `0x9999`, clip region `0x2222`, font restoration and clip deletion.
The actual DrawControlText helper passes its text color as DrawTextRuns' shadow
argument; the fixture preserves this observed behavior rather than replacing
it with the control's separate shadow flag.

Mutation cases change the rectangle, selected state/table, source-blit flag,
text pointer/length, margins, image width, redraw count, dialog, backing surface,
colors and held DC fields. Each requested callback must be reached. Exceptions
compare partial state; independent early-throw oracles require no release and
the expected held DC before/after acquisition. There is no invented unwinding
cleanup. Invalid pointer expectations follow decoded helper order: for example,
a dialog pointer of 1 first produces a read fault at `0x111`, because the strict
helper loads its background field before the later saved-font store.

## Limits and reproduction

Objects are prepared storage, not constructed game objects. Full dialog bytes
and the explicitly prepared windows for other types are compared; this is not
proof of every larger class's complete lifetime. The scripts always write DC
and measured SIZE outputs, including when returning failure. They do not model
arbitrary unwritten Win32 outputs. Background GetDc deliberately follows a
finite acquisition/iteration script; it is not the real backend's lifetime or
allocation protocol. Draw loops are bounded to supplied finite scripts. Valid
text-run lists are terminated and fit prepared arrays. The few overlap cases
prove only those concrete address arrangements.

No real GDI drawing, COM driver, live window, constructed dialog, actual GPU
surface, asynchronous operation or full menu frame is exercised. Retail mapping
and direct-call redirects exist only in the private validation executable.

### Functional build review

This complete candidate was individually reviewed and admitted as nonexact
source. The isolated `work/game-build/build-v5nqn0ru` compiled **256/256**
translation units and reduced unresolved names **240 to 239**, removing only
`?Draw@UIMultiState@@UAEHXZ` and adding none. Independent recompilation reproduces
the actual new object (two sections/eight relocations) using its game command.
All 254 prior objects are preserved under exactly the two separately reviewed
palette getter name mappings, with no filename exclusions. The full game still does not
link; startup and menu execution remain separate completion requirements.

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH='.;tests'
python -m unittest test_multi_state_draw -v
```

Private preparation and same-path preservation evidence are under
`work/connections49/multistate/`; `aggregate-preservation.json` records the
complete original aggregate comparison. The retained test and fixture reproduce
the full function/helper/ABI/layout and finite native evidence.
