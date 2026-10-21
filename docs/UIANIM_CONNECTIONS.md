# UI animation texture connections

The cursor and loading-dialog texture calls now use the existing `UIAnim`
implementations in `GameUi.cpp`. This supplies two previously incompatible
external spellings without adding implementations or storage.

| Consumer | Retail call | Existing provider | Complete bytes |
|---|---:|---|---:|
| `GUICursor::RenderPost3D` | `0x0048517a` | `UIAnim::AdvancePastSounds`, `0x004730b0` | 46 |
| `LoadingDlg::HandleEvent` | `0x004511bf` | `UIAnim::GetCurrentTexture`, `0x00472f90` | 20 |

Each receiver is cast at the call boundary. The actual `TextureMap*` return
is converted to the existing 32-bit integer field or argument. Authentic
VC6 emits the same complete 27-byte cursor caller and 582-byte loading
handler. The provisional cursor and dialog-image declarations remain;
this change does not assert complete equivalence between those views and
all of `UIAnim`.

## Receiver and return evidence

The raw UIAnim COL at `0x0055c410` identifies its primary vtable
`0x00552ccc`, with UIAnim and BaseObject at offset zero. The complete
82-byte constructor at `0x00472e30` installs that table. The resource loader
allocates `0xf8` bytes at `0x0046ba99`, passes the returned primary pointer
to that constructor at `0x0046babc`, stores it in the dialog image table at
`0x0046bad5`, and transfers that same pointer into the named section record
at `0x0046bc50`. The loader is still a nonexact candidate; these claims use
its decoded retail instructions, not a claim that its full source matches.

The existing strict 109-byte `UIDialog::FindSectionObject` returns that
section object. The loading handler asks for `BlueBar`, passes the returned
pointer directly as ECX to the texture getter, then passes EAX unchanged to
the progress-bar texture setter. Named sections may also contain sounds or
controls; the image interpretation is specific to this resource use.

For cursors, the resource loader resolves `MouseAnim` into control field
`+0x180`, and the default animation into dialog field `+0x95c`. The complete
543-byte retail dialog tick selects one of those pointers and passes it to
`GUIUser::SetCursorAnimation`. The strict 89-byte GUIUser method and 17-byte
GUICursor setter preserve it in cursor field `+0x5c`. The render caller loads
that field directly into ECX. Accessed cursor-view fields `+8`, `+0xc`,
`+0x10`, and `+0x20` agree with the actual animation fields. This evidence
supports the narrow primary-pointer connection; resource names and semantic
member names do not establish an original class declaration.

## Complete-object preservation

A local `GameUi.h` include gives GUIManager the genuine declaration.
DlgProcs already has it. No shared header, constructor, virtual slot,
initializer or storage definition changes.

Same-path baseline and candidate compilation preserves all nondebug bytes,
section flags, relocation addends and resolved destinations under exactly
these two method-name substitutions:

| Object | Sections | Relocations |
|---|---:|---:|
| GUIManager | 201 | 833 |
| DlgProcs | 420 | 3,040 |
| GUIManagerNearMisses | 209 | 871 |
| DlgProcsNearMisses | 557 | 4,152 |
| Total | 1,387 | 8,896 |

Both published aggregates compile without prerequisite fixes. No function
extent or partial score changes. All seven existing DlgProcs partial pins
remain unchanged. The complete-object retained pins normalize only the
checkout prefix and lexical path separators of the four named source paths,
`ContainerList.h`, and included `GuiScreenTexture.cpp`; no section is omitted.
The independent same-path proof requires no filename normalization.

`tests/test_uianim_connections.py` performs **199 strict comparisons**:
193 existing affected cases, five complete provider/producer methods, and
the extracted getter. The extracted getter must also have identical bytes
and relocations to the actual full GameUi object, and no imports, storage or
CRT initializer. The test verifies raw RTTI, allocation and pointer
propagation, both retail call sites and all four corresponding COFF calls.
Private reproduction evidence is under `work/connections52/uianim/`.

## Native getter validation

The retained fixture executes the actual separately compiled getter and
its untouched 20-byte retail counterpart. There are no dependency redirects
for this function. It checks **3,067 cases**, including **41 access violations**
with independent read-address and access-kind assertions:

- 3,000 combinations of 50 frame indices, six sound-discriminator values,
  five raw texture-pointer values, and separate or shared backing-page placement;
- 66 accessible frame-prefix cases, from zero through 32 bytes, with sound
  and image discriminators where the relevant field is writable;
- one invalid animation receiver.

The independent return oracle requires null exactly for sound discriminator
one. Both complete prepared pages must remain unchanged, including after
the retail run before invoking source. Guard cases distinguish the sound
field read from the later texture-pointer read; sound frames avoid that
second access. The fixture pins the actual class sizes, accessed offsets
and four-byte member-pointer representation.

These are finite observations on prepared raw storage. No UIAnim/UIFrame
constructor, renderer, GUI lifetime or arbitrary invalid pointer behavior
is claimed. The two page placements do not constitute overlapping object
lifetimes. `AdvancePastSounds` and its 200-byte `Advance` helper are checked
as complete strict bodies, but their sound/clock execution is deliberately
not simulated. Full sound playback and timing lifecycle remain separate
validation work.

Run with authentic native VC6 and the private retail image:

```powershell
$env:PYTHONPATH = '.;tests'
python tests/test_uianim_connections.py -v
```

`VC6_ROOT` and `MCM2_EXE` must point to the verified compiler and retail
inputs. This connection does not establish that startup, menus or racing
are operational.
