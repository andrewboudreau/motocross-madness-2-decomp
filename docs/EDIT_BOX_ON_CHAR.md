# Edit-box character input

`samples/ui/EditBoxOnChar.cpp` reconstructs the complete
`UIEditBox::OnChar(int)` at `0x00474150`. It handles insertion, Backspace,
Enter, double-byte input state, text measurement, redraw, and dialog
notifications. The readable body is unchanged from the former aggregate
candidate. It remains nonexact: VC6 SP3 emits **1,044 bytes**, with **1,018
matching positions** against the complete **1,044-byte** retail extent.
Original translation-unit ownership is unproven.

## Identity and complete extent

The raw primary UIEditBox vtable is `0x00552de8`; its COL at `0x0055c4c0`
has object offset zero and type descriptor `.?AVUIEditBox@@`. Slot 20 points
to `0x00474150`. Both the retail table and the emitted aggregate table are
checked. The canonical GameUi object previously imported this missing method.

The two `ret 4` instructions are at `0x4742e3` and `0x4744d1`. The later
insertion block at `0x4744d4..0x47453c` jumps back to `0x4743ad`; it must not
be omitted. After three alignment bytes, the four DWORD switch destinations
occupy `0x474540..0x47454f`, followed by the 20-byte index map at
`0x474550..0x474563`. The dispatch indexes these tables for values 8 through
27. Padding precedes the next function at `0x474570`. There is no local SEH
registration in this handler. The retained test checks every decoded branch,
all 11 direct calls, all 12 indirect calls, tables, returns, and boundaries.

## Source and dependency evidence

The original `GameUiNearMisses.cpp` now includes the standalone body. A fresh
same-path before/after comparison preserved **all 733 nondebug sections and
3,912 relocations**, including every other complete strict/partial body,
constants, vtables, and embedded filenames. There were no exclusions or
filename substitutions in that extraction proof. Private reproduction is in
`work/editbox45/preserve.py` and `extraction-proof.json`, with `before.obj` and
`after.obj` produced from the same aggregate source path.

The retained whole-object pin normalizes only the checkout-dependent paths
of three existing `__FILE__` strings: GameUi.cpp, ContainerList.h, and
GameUiNearMisses.cpp. All remaining section bytes, attributes, and relocation
destinations/addends are retained. The standalone function equals the
aggregate function, including its internal-label relocation offsets.

The candidate emits two nondebug sections: compiler directives and a
1,056-byte text section containing the complete 1,044-byte function, its
embedded switch data, and trailing section alignment. It adds no writable
storage, CRT initializer, or exception handler. Its only external names are
five already supplied game methods, `memmove`, and four Win32 imports.
`tools.build_game.compile_source` emits the same complete nondebug object as
the native fixture's candidate build; there are no special compiler flags.

The native fixture extracts the following actual source bodies and compares
each complete body both to retail and to its canonical source object:

| Actual helper | Retail address | Bytes |
| --- | --- | ---: |
| UIControl::OnChar | `0x472320` | 166 |
| GameObject::OnChar | `0x469c00` | 60 |
| UIEditBox::FilterCharacter | `0x473fc0` | 160 |
| UIControl::GetName | `0x470df0` | 7 |
| UIControl::RequestRedraw | `0x470640` | 21 |
| UnknownSoundNotifier constructor | `0x4bb720` | 26 |
| Sound::SetVolume | `0x4bcbe0` | 185 |
| Sound::PlayWithOptions | `0x4bc6b0` | 656 |
| Sound::IsPlaying | `0x4bca80` | 172 |

These are **18 strict comparisons**. The notifier constructor is needed before
PlayWithOptions in the validation helper source so VC6 reproduces the actual
inline construction and complete 656-byte body. The native matrix does not
enter its notifier-allocation branch. The unvisited diagnostic filename in
that body is recognized explicitly when comparing helper-source relocation
names with PCAudio.cpp; it is not exposed in the covered executions.

The fixture executes the eight UI/sound helpers on the reachable paths. The
retail path retains its original game-helper calls. Only three verified
`memmove` and three verified `strchr` calls are redirected to the linked VC6
CRT; their original opcodes and destinations are checked first. This does not
claim independent identity of all retail CRT instructions or their exceptional
partial-copy behavior. The real CRT operates on the tested overlapping buffers.

## Native validation

`tests/test_edit_box_on_char.py` compiles the actual standalone candidate and
`tests/fixtures/edit_box_on_char_compare.cpp` separately, then compares them
with a private mapping of the hash-pinned retail executable. No retail mapping,
helper replacement, or fixture code is part of the game build.

The test passes **20,962 scenarios**:

| Finite matrix | Cases |
| --- | ---: |
| Independent insertion, Backspace, handled Enter, and unfocused outcomes | 4 |
| Character values -128..255, four cursor positions, DBCS/filter/capacity modes | 12,288 |
| Lengths 0..8, every contained cursor, capacities 0..11, pending/drop states | 2,160 |
| Special/extreme int inputs, focus, handled/base gate, and child traversal | 1,056 |
| Click/full sounds, audio/group state, buffer status and COM failures | 576 |
| Nine baseline paths and exception/mutation at their 99 encountered boundaries | 1,494 |
| Null/invalid pointer combinations | 18 |
| Progressive text/control/dialog guard-page boundaries | 192 |
| Text aliasing the control name/tail and accepted-character buffer | 3,072 |
| DBCS capacity/pending/drop oracles | 96 |
| Volume clamp/failure, DC failure, guard fault and failed-measurement oracles | 6 |

Ordinary input, capacity, alias, focus/handled/child, and sound matrices require
completion without exceptions. Independent bytewise text/cursor/length oracles
cover the ordinary single-byte paths. Separate assertions cover DBCS pending
state, volume clamping and ignored sound failure, output measurement, and
progressive access faults. They supplement differential comparison rather than
relying only on both executions agreeing.

Every scripted boundary records its receiver, arguments, relevant byte payload,
return value, and the complete prepared world before and after the callback.
Both executions compare final complete storage, the readable guard page,
return/completion state, exception code, and access-violation kind/address.
The callback matrix mutates cursor, length/capacity, text and dialog pointers,
focus, pending state, sound selection, font/surface pointers, filtering and
notification code. Some requested mutation indices become unreachable after a
prior control-flow change; coverage is bounded to the recorded execution.

Actual-header assertions establish the fields used by the fixture and the
four-byte member-call ABI. Prepared raw storage represents controls, dialogs,
children, GUI state, sounds, render targets and surfaces. Constructors and
lifetime management are not exercised. The child and redraw virtual boundaries
are scripted; redraw then executes the actual strict RequestRedraw body.

## Functional build review

The complete candidate is suitable for individual reviewed-nonexact admission
under the finite evidence above. It is not a strict reconstruction or a claim
that menus now run end to end. The intended new source is only
`samples/ui/EditBoxOnChar.cpp`; its aggregate includes the same body to avoid
duplicate maintenance. It adds the existing missing
`?OnChar@UIEditBox@@UAEHH@Z` definition without introducing another unresolved
game-method dependency.

The platform and sound limits are explicit:

- CharPrevA and IsDBCSLeadByte are scripted with a bounded lead-byte/pair
  contract. This is not a test of every Windows code page, malformed multibyte
  sequence, or installed locale. Prepared characters include double-byte pairs
  and cursor positions inside a pair; these can produce negative measurement
  lengths, which are recorded and handled by the stated measurement contract.
- GDI and surface operations are scripted. GetDC writes a synthetic DC only
  on success; failure skips measurement. Measure always writes both SIZE
  members, including when it returns FALSE, and provides a deterministic
  byte-based width. Retail and candidate ignore that return status. Windows
  failures that leave SIZE unwritten are excluded: no deterministic width is
  claimed for uninitialized output. No actual font rendering or device is used.
- Dialog callbacks inspect only the six initialized common event fields and
  handled flag, plus key and field_0x18 for the base kind-11 event. Enter and
  edit-change event padding/uninitialized fields are neither initialized in
  production nor compared as if defined.
- Sound helpers execute with a bounded static-buffer contract, flags 0x20,
  prepared group/status state, and a finite duplicate list. Buffer status,
  volume, and duplicate Play are scripted COM methods. The matrix covers
  inactive audio, disabled/paused groups, volume clamp/failure and duplicate
  playback failure, but not streaming/notifier creation or the separate
  Sound::Play body. Validation-only definitions for those unvisited external
  dependencies terminate the test if reached; they are never game providers.
- Exceptions cover scripted boundary faults and access violations, including
  partial writes. No register-context, x87 stack, arbitrary exception-resumption,
  thread/scheduler, or actual OS-resource behavior equivalence is claimed.

Reproduce on Windows with authentic VC6 SP3 and the private executable:

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_edit_box_on_char.py -v
```
