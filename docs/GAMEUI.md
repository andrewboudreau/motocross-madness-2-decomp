# gameui.cpp (the UI controls)

`src/reconstructed/GameUi.cpp`, with the classes in `GameUi.h` and
`UIDialog.h`. RTTI names the classes: UIDialog and UIControl
(`UnknownGameUiControl`) and the derived button, static, edit, scroll,
slider, list box, multi-state, animation, timer, frame and drop-down
controls. Method names are provisional.

Extent: `0x00469db0..0x0047b66f`. Evidence:
- **Start:** the UIDialog constructor (vtable `0x00552a9c`). gameobj.cpp's
  last xref is at `0x00469d37`.
- **End:** the vcall thunks up to `0x0047b66f`. The last gameui `__FILE__`
  (`0x0056b738`) xref is inside `0x0047b570`. GhostMod1 and GraphicsTest
  follow, then Grid1.cpp.
- **Vtables:** UIDialog `0x00552a9c` through UIProgressBar `0x00553c68`.
- **`$E`:** its four kVec3 initializers sit at `0x0046e870..0x0046ea5b`.

Exact: 285 functions compile strict-exact from GameUi.cpp. 278 are
registered as calibration cases, including the 13 drawing, input and layout
functions in the file's last section ("Drawing and input slots"). The other 7 sit at the addresses of the
deliberately non-strict compiler-shape probes in
`samples/calibration/CompilerShapeProbe.cpp`. Those probes are kept
unchanged as regression probes, and the exact source for their targets now
also exists in GameUi.cpp.

Header changes:
- GameUi.h now lays out UIControl from +0x2c to +0x250, with unions for
  the derived controls (+0x204 track image/key-click sound, +0x208 sound,
  +0x20c/+0x210 drag position, +0x218 hold time as further union members).
- GameUi.h declares UIListBox slots 10 and 40, UIMultiState slots 28 and 40,
  the UIDropDownList constructor and its 0x0047a400 and 0x0047ab20 methods.
- InputDevice.h befriends UIDialog, UIScrollBar and UIListBox (they read
  deviceKind).
- GameUi.cpp's UnknownGameUiDialog has slot 27's real 14-argument signature
  and names +0x38 (resource name), +0xb8, +0x118 (text colors), +0x95c,
  +0x7f1c and +0x7f40.
- Nine methods that return values now return int. Their binding keys were
  renamed in the dialog files' bindings; only the keys changed.
- GameObject.h befriends UICtlContainer, UnknownGameUiControl and UIDialog.

Near misses (`samples/ui/GameUiNearMisses.cpp`): 39 functions, listed with
their differences at the top of the sample. They include the UIControl and
UIListBox constructors, the UIControl destructor and several list-box and
scroll slots. Four deleting destructors and destructors (`0x00470430`,
`0x00479c60`, `0x00479c80`, `0x0047ae60`, `0x0047ae80`) are byte-exact,
but VC6 emits them only alongside those near-miss constructors. So they
compile exactly only in the sample and are not registered.

Not done:
- `0x0046a920`, UIDialog's 16 KB resource parser (0x9c74-byte frame,
  keyword table compares, one `new` per control kind). Its `new` sites need
  per-class sizes that GameUi.h's flattened UIControl layout cannot give
  (all derived controls are 0x254 bytes here; retail allocates UIButton
  0x1f0, UIStatic 0x1ec, UIEditBox 0x230, UIScrollBar 0x220, UIListBox
  0x250, UIMultiState 0x1f8, UIDropDownList 0x21c), the same blocker as
  the UIDropDownList constructor near miss.
- `0x00477110` (adds an image row from a file): retail returns 0/1 but
  GameUi.h declares it `void`, and SelectGamePicProcs binds that name.
- UIVideoStatic (`0x0047ae30..0x0047b020`) and UIProgressBar
  (`0x0047b020..0x0047b110`, slot 40 `0x0047b110`): both classes are
  declared in DlgProcs.h as GameObject stubs; their real base is UIStatic.
