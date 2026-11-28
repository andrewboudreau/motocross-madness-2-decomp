# dlgprocs.cpp, InGameProcs.cpp, NetProcs.cpp and OptionProcs.cpp

These files hold dialog procedures. Each one is a slot-29 override of a
UIDialog subclass named by RTTI, plus a few helpers. The sources are
`src/reconstructed/InGameProcs.*` and `NetProcs.*`; `DialogProc.h` declares
what both use. That covers the slot-29 event record, the dlgprocs.cpp calls
`0x004526b0`/`0x00452930`/`0x00453090`, and the inline-constructed
ChoiceDlg (vtable `0x00551cb0`, "messbox2.dtm") and SessionDlg
(`0x00555408`, "messbox1.dtm").

Names: the event record's members (`code`, `controlName`, `kind`, `dialog`,
`gui`, `control`, `key`, `handled`) follow how GameUi.cpp's senders fill
them; `DialogEventKind.h` names the kinds. The dialogs' helper methods are
named after what their bodies do, as their header comments record
(`FillProfileList`, `OpenPage`, `ListGhosts`, `PaintPlateNumber`, ...).
The members inherited from UIDialog (+0x2c `parentDialog`, +0x30
`guiManager`, +0x34 `guiUser`, +0xc4 `openingMenu`, +0x110
`dialogBackground`, ...) come from UIDialog.h's 0x7f58-byte layout
([GAMEUI.md](GAMEUI.md)); the derived classes declare only their own members
from +0x7f58 (`samples/gameui/UIDialogLayoutProbe.cpp` checks their sizes).
The procedures stay `UnknownVirtualSlot29`, overriding UIDialog's virtual
slot 29 (so they mangle as `UAE`); other names are provisional.

**InGameProcs.cpp** (`0x004886e0..0x0048963b`). Evidence: the `__FILE__`
literal at `0x0056c5cc` (xref `0x00488928`). It starts where GUIManager.cpp
ends. The `$E` initializers at `0x00489640` open the next unit
(InputDevice). It holds the slot 29 of ExitDlg, ContinueDlg and VCRDlg,
plus VCRDlg's slot 10, its toggles `0x004894c0` and the replay time format
`0x00488bd0`.

**NetProcs.cpp** (`0x004ae460..0x004af672`). Evidence:
- **`__FILE__`:** the literal at `0x0056eb2c`, xrefs from `0x004ae64d` to
  `0x004af306`.
- **`.data`:** starts at `0x0056eadc` with the baud table, after Net.cpp's.
- **`.bss`:** `0x006886b8..0x006886cc`, between Net.cpp's and
  NetThread.cpp's globals.

It holds the slot 29 of HostJoinDlg, SerialPopupDlg, TCPAddressDlg,
SessionDlg, WaitOrCallDlg, ConnectErrorDlg and PlayerRemovedDlg. It also
has the session join `0x004ae460` and the network drop and ensure
functions `0x004aef40`/`0x004aefa0`. The serial helpers at
`0x004ae2f0..0x004ae454` map baud, parity and other settings. Their original TU
ownership remains unproven. The complete strict group now
lives in `src/reconstructed/SerialAddress.cpp`; see [serial conversion](SERIAL_ADDRESS.md).

Exact: all 16 functions (6 and 10). Source forms needed:
- `_stricmp` calls take the literal first.
- ExitDlg's two notification branches are written out in full, and VC6
  merges their tail.
- WaitOrCallDlg sets `event->field_0x20 = 1` in both branches.

Header changes, all layout-preserving:
- GameUi.h's UIControl derives from GameObject, as RTTI
  UIControl does.
- UIDialog.h, TrackGame.h and RaceView.h gained the members these
  procedures use.
- FollowCamera.h befriends VCRDlg, as Camera.h does for its readers.

**OptionProcs.cpp** (`0x004b1ec0..0x004b5a5d`). Evidence:
- **`__FILE__`:** the literal at `0x0056f57c`, with line pushes 979..1123.
- **RTTI:** the vtables of OptGameSettingsDlg, OptGraphicsDlg,
  OptAdvancedGraphicsDlg, OptSoundDlg, OptControlsDlg, OptGarageDlg,
  OptMessagesDlg, OptionsDlg, GlobalSettingsDlg and ConfirmRestoreDlg,
  whose procedures fill the range.
- **Start (strong inference):** a set of kVec3 `$E` at
  `0x004b1ec0..0x004b1ffb` writes bss in this file's run
  (`0x00688778..0x00689110`). ObjectPlacement.cpp's own set ends at
  `0x004b1ebb`.

The option pages edit copies of TrackGame's settings blocks. `0x004b4280`
copies +0xc24, +0xf98, +0xfc4, +0xfe0, +0x1550 and +0x19d4 into the file's
globals with `rep movsd`. OptionProcs.h declares the globals with
TrackGame.h's `TrackGame*Options` types.

Exact: 36 calibration cases. These are the eight `$E`, ten slot-29
procedures, five slot-31 handlers, OptControlsDlg's slots 22 and 23 and
eleven helpers. Source forms needed:
- Name compares use `_stricmp` and `_strnicmp`.
- An inline drop-down accessor reads control +0x1fc, and an inline lookup
  (`BikeClassOf`) walks the bike-class table `g_BikeClassEngineSizes`
  (`0x0056cb6c`: 125, 250, 400, 500, 600).
- `0x004b3aa0` nests `__min(__min(...))`.
- OptControlsDlg slot 22 `0x004b5600` ends each branch with its own
  `EndInputWait(); return 1;`; one shared tail swaps `this` and the list
  box between ebx and ebp.
- `0x004b5760` sets `kind = 1` after `code` in each mouse branch; one store
  at the join swaps the code and row + 1 between esi and edi.

Header additions:
- GameUi.h: control slots 55-66, list rows and fields.
- UIDialog.h, TrackGame.h, KrustyUI.h and JoystickDevice.h: new members.
- InputDevice.h befriends OptControlsDlg.

### Inherited option-dialog synchronization

Five option-dialog classes inherit the existing `UIDialog::SyncValues`
implementation at `0x00470050`. Their former declarations incorrectly
introduced separate overrides with no source definitions. Removing those
declarations makes the compiled primary vtables refer to the supplied base
method without adding wrappers or new function bodies.

| RTTI class | Primary vtable | Slot 31 |
|---|---|---|
| OptionsDlg | `0x00551b18` | `0x00470050` |
| GlobalSettingsDlg | `0x00555604` | `0x00470050` |
| ConfirmRestoreDlg | `0x005559a0` | `0x00470050` |
| OptAdvancedGraphicsDlg | `0x00555580` | `0x00470050` |
| OptMessagesDlg | `0x0055570c` | `0x00470050` |

This connection follows decoded RTTI's direct `UIDialog` base at offset zero,
the primary complete-object locator and slot position, and the complete
24-byte base implementation. That body invokes the optional callback at
`+0xd0` with the dialog and synchronization flag. The shared address alone is
not the inheritance evidence; the class relations and matching slot are also
required. The other five option-page synchronization overrides remain intact.

`tests/test_dialog_sync_inheritance.py` reparses the pinned retail RTTI and
slot words, compiles the six affected/provider units with authentic VC6, and
checks their actual vtable relocations to `UIDialog::SyncValues`. A compile
fixture checks the inherited member-pointer ownership and all five object
sizes, including OptionsDlg's first page at `+0x7f58`. All **593 strict
calibration cases** in those units remain exact, including the complete base
provider. The real link removes exactly five missing decorated names, reducing
563 unresolved symbols to 558, with none added. This verifies the source
connection; interactive menu behavior remains a separate runtime gate.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_dialog_sync_inheritance.py -v
```

**dlgprocs.cpp** (`0x0044b0a0..0x00455d9f`, `src/reconstructed/DlgProcs.*`).
Evidence:
- **`__FILE__`:** the literal at `0x0056963c`, xrefs `0x0044b317..0x00455151`.
- **Literals and RTTI:** its literals run from "LstProfiles" (`0x00569300`)
  to "GoLink" (`0x0056a094`), followed by its dialogs' type descriptors.
- **Start:** dirlist.cpp ends at `0x0044b09f`. The first functions are
  qsort comparators that only this file pushes.
- **Past the last xref:** DemoDlg, TransDlg and Exit1Dlg have their slots in
  `0x00455a10..0x00455d9f`.
- **End:** `0x00455da0..0x00455ddf` initializes EcoSystem's bss and is left
  to EcoSystem.cpp.

Its four kVec3 `$E` pairs sit mid-file (`0x00453d50..0x00453e8b`), and
earlier code reads them.

Exact: 89 calibration cases. They cover the slot 29 procedures and helpers
of the main, single-player, profile, intro, credits, loading, ghost and
replay, choice, edit-box, demo, transition and exit dialogs; the six
Load*Dlg constructors; the cdecl menu helpers `0x004526b0` and
`0x00453090`; and the eight `$E`. SPBikeRiderDlg slot 22 `0x0044fab0` and
`0x004500d0` are bodies that the MP and pro circuit bike dialogs share.

ChoiceDlg `0x00455700` now has typed parameters, so InGameProcs.bindings.json
carries its new mangled name.

Also exact (8, added in the large-function wave): SPEventDlg slot 29
`0x0044d950`, its helpers `0x0044dfc0` and `0x0044e3e0` and slot 31
`0x0044e6c0`; SPBikeRiderDlg slot 29 `0x0044ef70` and `0x004500e0`; profile
creation `0x00451b80`; race start `0x004536e0`. Source forms needed:
- A per-branch scoped buffer (the 32-byte seed text in slot 29) keeps VC6
  from merging the branch tails.
- The bike-class apply loop is the `UNKNOWN_APPLY_BIKE_CLASS` macro in
  DialogProc.h, reusing the caller's loop counter.
- The bike view distance is `(x*x + y*y) + z*z`.
- `0x004536e0`'s switch cases are written in the order 3, 5, 0, 4, 1, 2.

Near misses (in `src/reconstructed/DlgProcs.cpp`, retail order, each with a
`// Not exact` status comment; forms tried are at the end of this file):
- MainDlg slot 29 `0x0044b200`: retail shares one frame slot between its two
  ConnectionInfoType[5] arrays; VC6 here does not (frame 0xc64 vs 0x6c4).
  Retail's unwind map (FuncInfo `0x00560770`) gives both arrays `[ebp-0x5ac]`
  in states 1 and 0xc. No source shape or optimisation flag tried makes VC6 SP3
  share two EH-tracked arrays; only /GX- does, by dropping their states.
- SPBikeRiderDlg slot 10 `0x0044fae0`, 1181 of 1235 bytes: only the
  scheduling of the by-value vector copies for the camera call differs
  (no argument spelling changes it; a Vector3 copy constructor is worse).
- SPRaceInfoDlg and MPRaceInfoDlg slot 29 (`0x004507a0`, `0x004513a0`):
  register use inside the per-racer loops differs.

The shared bike-class engine-size table now has a canonical source definition
in `src/reconstructed/BikeClassData.cpp`: five 32-bit integers
`125, 250, 400, 500, 600` at retail `0x0056cb6c`. Its existing declaration and
six consuming translation units are unchanged. `tests/test_graphics_state.py`
checks all 20 retail bytes, writable COFF data storage, the exported symbol
and absence of dynamic initialization. The file groups reconstructed storage;
it does not assert original translation-unit ownership.

## Loading-progress dialog connection

`KrustyUI::progressDialog` at +0x490 now has type `UIDialog*`. LoadingDlg's
582-byte event handler at 0x00451020 stores its receiver there at 0x0045113f
and clears it at 0x004511f2. Retail RTTI identifies LoadingDlg's UIDialog base
at offset zero. The implicit derived-to-base assignment preserves that address;
the obsolete empty `UnknownGameUiPage` declaration is removed.

EventManager and KrustyUI therefore call the existing full 200-byte
`UIDialog::FindControl` at 0x0046ebf0. Their binding destinations are unchanged.
`tests/test_progress_dialog_connection.py` checks retail RTTI, lifecycle stores,
native member/layout/conversion assertions and actual caller imports. All 506
strict cases across EventManager, KrustyUI, DlgProcs and GameUi remain exact
with relocations resolved. Run it with `VC6_ROOT` and `MCM2_EXE` configured:

```powershell
python -m unittest discover -s tests -p test_progress_dialog_connection.py -v
```

The isolated real build compiled 197/197 units and reduced unresolved symbols
from 540 to 539, removing only the obsolete FindControl name. This connects
the loading-progress receiver; it does not establish working menus or startup.

## Near misses: forms tried

Scores are matching bytes under `vc6_o2_mt` with relocations resolved.

MainDlg::HandleEvent (0x0044b200, 5232 bytes): the same cases, calls and
strings; retail shares one frame slot between the two ConnectionInfoType[5]
arrays (frame 0x6c4), VC6 here gives each array its own slot (frame 0xc64), so
every later frame offset differs. Retail also pushes the constant 1 where VC6
here keeps it in edi. One array at function scope is constructed at entry
(worse) and one array in a braced kDialogCommand case is worse. In probes VC6
SP3 shares disjoint slots for POD arrays, constructor-only arrays and scalar
objects with destructors, but never for two arrays whose element has a
destructor (sibling blocks, else-if chains, switch cases, sequential blocks,
try, same name, inline virtual InfoType destructor), so the source shape that
lets retail share them is still unknown. Retail's unwind map (FuncInfo
0x00560770, 16 states) places both arrays at [ebp-0x5ac] (states 1 and 0xc)
and every `new` temporary at [ebp-0x6d0]. Also 0xc64 here: each array block
moved into a static inline helper (both inline, slots still separate), a
wrapper struct holding the array, scalar destructor objects in place of the
arrays, the blocks as switch cases, sequential blocks, nested depths, a loop
body, a try block, the helper returning early or allocating first, and the
flags /Od /Ox /O1 /Og /Oy- /Ob0 /Ob2 /Gy /Oa /Ow /GF /Gf /Zi /Gs. Only /GX-
shares the slot (it drops the unwind states). ButModem returns from inside its
loop once the modem connection is set up: retail destroys that block's array
on two paths (three ??_M calls in all, the found path jumping straight to the
epilogue), which `break` merged into one (tools/reference_diff.py, session
14). With `return` both OpenExitDialog calls and every g_kbGame load stay
apart as in retail; aligned instructions 1293 -> 1342 (regmasked hunks 44 ->
37), strict unchanged because the frame still differs.

SPBikeRiderDlg::Tick (0x0044fae0, 1235 bytes, 1181 match): the same flow and
x87 code; only the scheduling of the by-value Vector3 copies passed to the
camera's slot 29 differs (retail takes `edi = esp` after the camera and vtable
loads and loads z before storing y). A pointer, reference, `this->`, explicit
copy, cast or camera local for the argument leaves it; a user-defined Vector3
copy constructor copies before the camera is evaluated (worse). Also
unchanged: inline helpers taking the camera and the vector (const reference,
by value, pointer), a `Vector3(viewTarget)` temporary, parentheses, and the
declaration count (0..63 prepended typedefs); a `Vector3(x, y, z)` temporary
is much worse. In probes VC6 interleaves the copy like retail only when the
destination pointer lands in a scratch register; with it in a callee-saved
register (edi, `this` live afterwards) it takes the pointer first and copies
x, y, z in order, whatever the struct type. VC6 gives retail's order when the
callee is one load (EventManager slot 10 0x0045f200,
`podiumCamera->LookAt(podiumFocus)`, is exact) or when the camera comes from a
separate tested block, a volatile local or a register source pointer, all of
which add code; with the two-load chain g_TrackGame->ui->garageCamera it takes
edi = esp first. Also 1181: compiler flags (-G3/-G5/-Ox), float[3]/union
arguments, `(camera = ...)->` and a function-scope camera, the target declared
at any scope; `eye[1]` gives retail's `add edi, 0xc` but moves earlier
registers (183). The same copy differs in 0x004f8820 (SelectGamePicProcs) and
0x004d72c0 (ProCircuitProcs). Also 1181 or worse (11 forms): a comma
expression, `1 ? camera : 0`, `&*camera`, `camera + 0`, inline helpers taking
the KrustyUI and the vector (by value or const reference), an inline camera
getter with or without a redundant test, a `*(Vector3*)&` argument and a
qualified `Camera::LookAt` call. Mechanism (Fable probe, 2026-10-16): the
difference is VC6's Pentium scheduler, whose region is the whole basic block
(calls are not barriers). A label (or `goto l; l:`) between the SetFrame call
and the slot-29 call in both branches makes all three dialogs' copies exact
(0x0044fae0 1235/1235). No label-free construct split the block ({}, if (1),
do/while (0), switch, empty inline call, (void)x, dead copies), so the
original had some code-free block boundary there; a bare label is a
matching-only form and is not applied. Session 14: an inline helper wrapping
both calls with `goto look; look:` between them is also exact (1235/1235), the
same label moved into a helper, so not applied. Without the label the helper
(const reference or pointer target, void or int result, one helper per call)
gives 1181, by value 74; so do branches that fold after inlining or
propagation (an `int look = 1` parameter or local, `static const` flag, `if
(look) {}`) and `for (;;) { ...; break; }`, `while (1)` and `if (LookAt(..))
{}` around the slot-29 call; tests on `this`, `&viewTarget` or a target
pointer add code (109-523). Session 14 copy forms, all 1181 unless noted: a
`const Vector3&` local passed by value, that reference copied into a named
local first, a named copy assigned after its declaration, and a `pointer ?
viewTarget : viewTarget` argument in the drag branch only (pointer is known
there, so the test is threaded away). The same ternary in both branches, or on
previewDragging or frameTime, keeps a real test (75-295). Repeated `if
(pointer)` or `if (dx == dx)` tests ahead of the slot-29 call are also
threaded away and leave the block unsplit (1181); an `if (!previewDragged)`
test on the member is reloaded (801). The reference, named-copy and
threaded-ternary forms also leave 0x004f8820 (1214) and 0x004d72c0 (1233)
unchanged. 2026-11: the scheduler works on blocks of a bounded tuple count,
and both Tick branches sit exactly at the bound; retail behaves as if each
held one more tuple (one extra store ahead of the slot-29 call gives retail's
order too, and is as matching-only as the label). Looked for that tuple in the
inline maths the branches expand, all 1181 unless noted:
VectorDifference/VectorSum taking pointers, returning const, building `Vector3
r(...)`, through float locals or operator[] (by-value parameters 267-397, a
named result filled member by member 490-514); the `turned *= viewDistance`
scale as an inline helper taking a value or pointer (a `const float&`
parameter 376) or through a `float distance` local. None changes the slot-29
schedule.

SPRaceInfoDlg::HandleEvent (0x004507a0, 1652 bytes) and

MPRaceInfoDlg::HandleEvent (0x004513a0, 2004 bytes): the same calls, strings
and list columns; inside the per-racer loops retail reloads
TrackGame/EventManager into other registers (VC6 here keeps them live across
the loop), so the loop bodies differ from the first loop on. In 0x004513a0's
first loop (595/2004) retail reloads the EventManager for the +0x04 sprintf
argument while VC6 here reuses it. Unchanged (2026-10-11): `continue` on the
+0x24 test, an `EventManager* const&` alias, an entry reference or pointer, a
TrackGame local, an `(int)` cast and `(results + i)->`. Probe (2026-10-12):
reading the first loop's position through `(EventManager*
volatile&)g_TrackGame->eventManager` gives 1659/1668 for 0x004507a0 (every
later loop then matches); only the load's place differs (retail schedules it
after `lea ecx`, which a volatile load does not allow), so retail's load is an
ordinary one that VC6 did not carry across the branch. Not adopted. Casts
through int, char* + 0x570, `const&`, `void**` and `results + i` stay at 589.
An inline `Events(TrackGame*)` accessor at all or any group of the
EventManager reads of 0x004513a0 changes nothing (583, 2026-11-13).
0x004507a0's first loop again (wave 26, all 589): an `EventManager**` local
assigned in the loop test (inside the comparison or before a comma) and
dereferenced in the body, which is retail's `lea` and `mov edx, [eax]` written
out, is forwarded away; so are `(*&eventManager)->`, `(&results[0])[i]`, the
comparison reversed, an `(int)` cast and an unsigned comparison. A TrackGame
local assigned in the test is kept and is far worse (142 of 1672).
