# KrustyUI

RTTI: `KrustyUI : GameObject : BaseObject` (vtable `0x0055488c`; 0x1364
bytes, the size TrackGame slot 4 allocates). Its `operator delete` calls
pass `D:\aardvark\VC\krusty2\krustyui.cpp` as `__FILE__` (`0x0056d67c`),
which confirms the original translation unit's name. Canonical source:
`src/reconstructed/KrustyUI.h` / `KrustyUI.cpp`. Names are provisional.
TrackGame keeps it at +0x56c (`ui`).

## Extent

`0x004987f0..0x0049bf0b`. The first function is the constructor; the unit
ends with the shared dialog deleting destructor and its vector set:

- `0x0049bdb0` is `Exit1Dlg`'s scalar deleting destructor (strong
  inference): krustyui.cpp instantiates `Exit1Dlg` (`0x0049a4a0`), the
  compiled body matches, and the linker kept this copy for 70 dialog vtables.
  It calls the folded implicit destructor `0x00450fc0` (`jmp 0x0046a070`).
- `0x0049bdd0..0x0049bf0b` initialize the per-file vectors (0,0,0)
  `0x0067c418`, (1,0,0) `0x0067c428`, (0,1,0) `0x0067c458` and (0,0,1)
  `0x0067c3f8`. `0x00498cf0` reads the zero vector (`0x00498eac`), its own
  function-local statics sit between them in `.bss` (`0x0067c408`,
  `0x0067c438`, `0x0067c448`), and `.CRT$XCU` entries 163-166 follow
  the set at 159-162 (`0x004986b0..0x004987a0`, just before the
  constructor).

## Status

Exact (43 functions, including the open-menu routine `0x00499b20`):
- the constructor `0x004987f0`. Eight 8-byte `{int, char}` records at
  +0x634 have an inline constructor, so VC6 emits the clearing loop before
  the vtable store; the body then clears the other fields in retail order;
- the scalar deleting destructor `0x00498880` and the destructor
  `0x0049b470`. The destructor runs `0x004999b0`, then frees the
  DebugMalloc'd buffers at +0x48, +0x50, +0x58 and +0x60 (retail lines
  1306–1309);
- `0x004999b0`: the shutdown step. It calls the cdecl `0x005053b0(0)`,
  calls the GUI's `0x00485d50` unless Game+0x2d5 bit 1 (shutdown) is set,
  and releases +0x464;
- `0x0049b530`: tells the "ProgressBar" control of the +0x490 page 1;
- `0x0049bb80`: frees +0x60 (line 1453) and clears it and +0x64;
- the cdecl GUI progress callback `0x0049bda0`, which tail-calls the mode
  object's `0x00523580`;
- the overrides of slots 4 and 5 (base tail calls), 10, 18 (returns 1),
  23 (clears +0x4a4), 24 (message 0x101 sets the network object's +0x10)
  and 25 (also forwards the value to the +0x464 scene);
- `0x004999f0` and `0x00499a20`, which show and hide the +0x464 scene and
  set the owner render target's camera (+0x468, or none);
- `0x00499b00` and `0x00499b10`, which call the GUI's `0x00486630` with 1 and
  0;
- `0x0049a4a0`, which turns "MediaControl" on, hides the GUI and opens
  `Exit1Dlg` ("Exit1.dtm", line 836);
- `0x0049ba70`, which appends a 0x54-byte entry (five ints and a 64-char
  name) to the +0x60 list through gameui.cpp's resize helper `0x0047b570`.
  It returns `++count - 1`;
- `0x0049b020`, which fills an array with distinct random short strings from
  `0x0068a498`, probing a DebugCalloc'd used-table (lines 1120/1157);
- `0x0049a540`: reads `presets.pb` (`Category_%d` limits, `HPSum`,
  `MinRange`, `Weight`, `RPM%05d` bands and three presets per class) into
  the garage tables, or sets the defaults;
- `0x0049b560` / `0x0049b7f0`: copy a bike or rider name and return 1 when
  it is an archive entry or a file (prefixed `Res\` without a directory);
  otherwise they pick a random available bike (+0x50) or rider (+0x58);
- `0x0049bbb0` / `0x0049bc50`: record the race in the high-score tables at
  TrackGame+0x3400 (table 0, then 1 and 2 by laps 5/10 or, in modes 0 and 4,
  by the +0x140 setting 5.0/10.0);
- `0x00498cf0` (3114 bytes): builds the garage scene at +0x464 on the
  first call with 1 or -1 (camera, light manager and three lights, the
  "UIgarage.mcf" and "UIRider.mcf" characters, a ProjectedShadow and its
  D3DIMSoultreeShadow) and with 2 or -1 later loads every bike model
  ("Animations\UIbike\%s", "WaitB") with its plate number once. Its six
  static vectors have an empty destructor (a `Vector3` subclass), which
  gives the six empty exit handlers `0x00499920`–`0x00499970` (also exact,
  `_$E14`..`_$E19`). Source shapes: one function-scope 260-byte buffer, the
  render target read through a cast each time (a local adds a register
  copy), and an inline helper for "add the light when there is one" —
  written out three times, VC6 forms the bike loop's addresses as
  [offset + list] instead of [list + offset]. The views it calls are local
  to `KrustyUI.cpp`;
- `0x0049bdb0` and the eight `$E` thunks/bodies `0x0049bdd0..0x0049bed0`.

The random helper `RandomUnit()` returns `(float)(rand() * (1.0f / 32768))`.
The explicit cast is what keeps VC6 from folding the scale into a following
constant (`* 36`) or reordering it after a variable factor (`* (count - 1)`).

Near misses (`samples/ui/KrustyUINearMisses.cpp`, own bindings file):

- `0x004988a0`, TrackGame slot 4's initialiser (872 of 1104 bytes). It loads
  36 short strings (resources 5000–5035) into `0x0068a498`, the GUI scale
  (0xfed) and font (0xff2, else "Arial"), creates the GUI (line 121), loads
  `ui`, `ui\wait.tga` and `ui\uires.res`, restores "MRUProfile" and
  "JoystickFilter", and offline opens `Intro1Dlg` (line 201). From the scale
  store on, retail picks different registers.
- `0x0049a8b0`, which loads `bikes.pb` and `riders.pb` (1891 of 1897 bytes).
  Only manufacturers 3 and 7, their bike 2, and riders 42 and 44 are read.
  Retail keeps the manufacturer index at `[esp+0x1c]` and the bike count at
  `[esp+0x20]`; VC6 here swaps them whatever the declaration order, scope or
  names.
- `0x0049b0d0`, which picks distinct random bikes and riders for the AI
  (880 of 916 bytes). Retail places the "class 1 above 0.2" arm after the
  probe loop; every if/else, goto and ternary form tried keeps it before.

The GUI page and control classes are declared in
`src/reconstructed/GameUi.h`, not `KrustyUI.h`. Declaring them in a header
that TrackGame.cpp includes disturbs TrackGame slot 1 (see
[TrackGame](TRACKGAME.md)). The +0x48/+0x50/+0x58 list entries are views
local to KrustyUI.cpp for the same reason.

`0x00499b20` (2432 bytes, exact): opens menu `id` through a sparse switch
that allocates 18 dialog classes (MainDlg, LoadingDlg, NetProcs/InGameProcs/
ProCircuitProcs dialogs, ...). Their headers gained inline constructors
(`UIDialog(1, "<name>.dtm")`) padded to the retail allocation size 0x7f58.
