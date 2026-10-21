# GUI startup operation

`samples/ui/KrustyUINearMisses.cpp` contains the complete, unchanged
`KrustyUI::Create(RenderTarget*, int)` candidate. It connects the render
target to the GUI, loads UI strings and settings, restores the profile and
joystick preference, loads the bike/rider tables, and optionally opens the
intro dialog. The retail diagnostic path is
`D:\aardvark\VC\krusty2\krustyui.cpp`; this standalone sample does not claim
to reconstruct that entire original translation unit.

## Complete body and generated support

The full retail body begins at `0x004988a0` and ends with `ret 8` at
`0x00498ced`: **1,104 bytes**. The next function starts immediately at
`0x00498cf0`. Every direct branch is checked against decoded instruction
boundaries. Its three indirect calls are Release through slot 2 and the
registry string/flag callbacks through slots 23 and 22.

The retail exception handler at `0x0054be71` uses the two-state FuncInfo at
`0x00561d60`. The two complete cleanup funclets at `0x0054be40` and
`0x0054be57` are 23 and 26 bytes and call placement delete with allocation
lines 121 and 201. The source's two corresponding unwind states and cleanup
destinations are independently checked. All 36 direct dependency-call/jump
sites across the body, cleanup funclets and handler are verified before the
private reference calls are redirected.

Authentic VC6 SP3 emits **1,102 bytes**, retaining **872 matching positions**
against the full retail extent. The candidate remains explicitly nonexact.
Its source bytes are unchanged from `c8444fa8`; all **30 nondebug COFF
sections**, including generated methods, EH metadata, RTTI, tables and strings,
are identical before and after the current shared-header composition, with
generated labels compared by their section/offset destinations. The private
report is `work/game-build/krusty-create-preservation.json`, with objects
`krusty-create-audit.obj` and `krusty-create-final.obj` in the same directory.

The inline `Intro1Dlg` construction emits its primary vtable and two methods.
All 32 table destinations are checked against retail table `0x005548fc` at
object offset zero. Slot zero selects the deleting wrapper at `0x0049bdb0`,
whose direct destructor call reaches `0x00450fc0`. The full **30-byte deleting
wrapper and five-byte destructor match strictly**, including relocations.
That caller/table evidence supplies the class identity; the shared five-byte
address alone does not. The standalone object has no CRT initializer.

## Functional build review

`tests/test_krusty_ui_create_behavior.py` runs **13,540 native scenarios**
against the pinned complete retail operation. The entire production candidate
object is linked, including its inline-constructor support. The sample uses
VC6 `/O2`; the independent fixture uses `/Od`. The actual linked VC6 CRT
string and numeric conversions and C++ exception dispatcher execute for both
paths. Production has no retail dispatch or fallback.

The fixture compares every byte of the UI, two distinct game views, two GUI
objects, intro storage, users, tooltips, displays, controls, networks,
joysticks and resource managers, plus the full 36-by-16 resource-name array.
It records these states at every dependency boundary and at completion,
together with ordered arguments, receiver identity, allocation ownership,
return value, exception/fault information and x87 status. The external object
storage is a bounded fixture view, not proof of every runtime object's layout.

Two narrowly checked representations are normalized: the diagnostic filenames
of the retail and sample allocator calls, and the intro object's validated
source/retail vptr addresses in comparison snapshots. The executing objects
retain their actual vptrs. Test-only C++ GUI/UIDialog constructors exercise
the genuine compiler constructor/unwind ABI; unused virtual slots fail
immediately if called. They do not claim to implement the real GUI lifecycle.

Coverage includes:

- **12,288 settings combinations**, varying all eight option bits, resource
  lengths through the full 128-byte response buffer, missing/present font
  and scale strings, empty/NONE/mixed-case/127-character fonts, tooltip bold,
  joystick presence, UI animation/sound/tooltips, intro requests and lobby
  state. Every resource-name slot is compared, including truncation at 15.
- Exceptions and four mutation modes at every observed dependency boundary
  in representative complete traces. Mutations switch the global game and
  resource manager, change UI settings, substitute the active GUI, and remove
  network/joystick pointers. This checks the caller's cached values and reloads.
- Both allocation failures, both constructor exceptions and their placement
  deletes, signed volume/filter extremes, and all four x87 rounding modes.
  Independent outcomes require the expected successful state, the explicit
  release on AppendSibling failure, and the correct allocation remaining
  alive after each constructor exception.

Null GUI allocation reaches the scripted SetUp boundary, whose context store
at `+0x18` produces the observed write fault. Removing the joystick during
the registry callback exposes the subsequent null receiver and the scripted
setter's byte write at `+0x5d5`. These dependency effects are modeled from the
existing source; the comparison does not execute the complete real SetUp or
device implementation. Failed intro allocation reaches ShowDialog with null,
whose return is scripted. No unobserved success semantics are inferred from
those cases.

This evidence supports explicit **reviewed-nonexact** admission. GUI rendering,
resource archives, profile files, registry APIs, input devices, actual helper
lifecycles and live menu navigation remain outside the scripted dependency
boundary. The function's finite behavioral evidence does not establish a
working source-built game.

The isolated real build `build-uk70iui4` compiles **207/207 translation units**.
Compared with `build-5te86usb`, unresolved names decrease from **476 to 475**:
only `KrustyUI::Create(RenderTarget*, int)` disappears, with no new missing
names. The executable still does not link, so startup and menus remain
unverified in the running game.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_krusty_ui_create_behavior.py -v
```
