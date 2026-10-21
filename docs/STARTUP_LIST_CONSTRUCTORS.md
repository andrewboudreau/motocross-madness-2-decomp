# Startup list constructors

`src/reconstructed/StartupListConstructors.cpp` supplies two ordinary out-of-line
constructors already declared by the parser and drive-list interfaces. Authentic
VC6 SP3 `vc6_o2_mt` reproduces their complete retail bodies:

| Existing source view | Retail address | Complete bytes | Behavior |
| --- | --- | ---: | --- |
| `UnknownParserList` | `0x004aae20` | 9 | Zero the head pointer; return the receiver in EAX |
| `UnknownDriveList` | `0x004676a0` | 16 | Zero drives, then count; return the receiver in EAX |

The names and original translation unit remain provisional. Shared small code
addresses do not establish class identity. These definitions belong to the
existing views because their actual callers supply the corresponding storage;
no other class sharing either body is substituted. Neither declaration has a
vtable or inheritance, so RTTI is not evidence for these types.

The new object has one 82-byte directive section and two 16-byte code sections,
with no nondebug relocations, undefined symbols, data or CRT initialization.
The 9-byte constructor's remaining seven section bytes are compiler padding.
There are no wrappers, special flags or matching-only declarations.

## Actual producer boundaries

The complete `ReadLobbyPreset` body at `0x0049c2f0` (763 bytes) calls the parser
list constructor at `0x0049c338`. `WaitForLobbyRank` at `0x0049c600` (355 bytes)
calls it at `0x0049c66a`. Both pass a stack address at current ESP+0x14. After
the intervening `lstrlenA@4` call consumes its one argument, each pushes two
arguments for `UnknownParser::Parse` and computes its receiver at new ESP+0x18.
That is original ESP+0x10: the list receiver is precisely parser+4. The retained
check decodes these instructions, verifies the imported function and Parse's
RET 8, and checks the list destructor receivers on normal cleanup paths. Actual
VC6 layout confirms the four-byte list and eight-byte parser with `pairs` at+4.

The complete `TrackGameMode` constructor at `0x00522060` (960 bytes), emitted by
`UiInfo.cpp`, pushes allocation size8 at `0x00522138`. The intervening string
copies introduce no call or stack-pointer change before the allocator call at
`0x005221bb`. The nonnull result becomes ECX and calls `0x004676a0` at
`0x005221d2`. The returned pointer is stored at mode+0x25dc at `0x005221e1`;
`0x005221e7` calls `Rescan`, not the constructor. The null branch is preserved.
The two existing header comments are corrected to distinguish these calls.
Compiled layout confirms the eight-byte list's pointer/count offsets0/4 and
the owning mode's field at+0x25dc.

Five complete before/after objects are unchanged: TrackGame, ZoneReport, Parser,
DirectoryList and UiInfo. Their 321 nondebug sections and 1,256 relocations
retain every payload byte, section flag, raw relocation addend and resolved
destination, with no name mappings or excluded regions. Portable retained pins
normalize only the five explicitly enumerated diagnostic source paths. The
regression also checks 102 complete strict provider/caller bodies. No unrelated
partial fingerprint is weakened.

## Native validation

`tests/test_startup_list_constructors.py` separately compiles the production
object and links it with `tests/fixtures/startup_list_constructors.cpp` using
the authentic VC6 toolchain. The fixture invokes the actual constructors through
placement construction, without copied implementation bodies, fake methods or
retail dispatch. Constructor code and ABI are independently fixed by the full
9/16-byte comparisons; placement construction also checks its returned pointer.

The 1,344 native cases comprise 1,024 ordinary cases across 64 initial byte
patterns and eight aligned/unaligned in-page addresses for each class, plus
320 page-boundary cases. A separate expected full-page snapshot permits only
the exact zeroed field bytes. All ordinary cases and readable boundary cases
must complete without exceptions. The 192 intentional write AV cases require
the precise exception kind, write access and protected-page address. In
particular the eight-byte drive list with only its first four bytes accessible
must zero that first field before faulting on the second. Faulting DWORD stores
are aligned; the test makes no claim about partially completed stores crossing
a page boundary. Unexpected exception kinds are not swallowed.

This finite check establishes the constructors' prepared-storage writes,
guard behavior and call boundary. It does not execute lobby/network processing,
drive enumeration, owning mode startup, list destruction or a complete game
lifetime. Readable unaligned placements are specific VC6/x86 observations, not
a portable object-alignment claim.

Run with `VC6_ROOT`, `MCM2_EXE` and `PYTHONPATH=.;tests` set:

```powershell
python -m unittest test_startup_list_constructors -v
```

Actual game build `build-1wo__8o2` compiles all 263 translation units and removes
precisely these two constructor imports: **216 -> 214**, with no new unresolved
names. Every previous production object is unchanged: 262 objects, 10,704
nondebug sections and 45,813 relocations, without name mappings or filename
exclusions. Both constructors match strictly in the actual game-command object;
its complete three-section layout and absence of imports are independently checked.

The authoritative calibration matrix adds two strict cases, reaching 3,019/3,030
rows. Its 2,980/2,990 unique retail address/extent count is unchanged because
other source views already target these shared bodies; their class identities
remain separate. Physics retains 1,086/1,162 strict results, with no lost strict
matches or partial-score decreases in either matrix.

The game still does not link. These constructor definitions do not establish
successful startup, integrated menus, drive enumeration, lobby processing or racing.
