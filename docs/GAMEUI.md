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

Exact: 272 functions compile strict-exact from GameUi.cpp. 265 are
registered as calibration cases. The other 7 sit at the addresses of the
deliberately non-strict compiler-shape probes in
`samples/calibration/CompilerShapeProbe.cpp`. Those probes are kept
unchanged as regression probes, and the exact source for their targets now
also exists in GameUi.cpp.

Header changes:
- GameUi.h now lays out UIControl from +0x2c to +0x250, with unions for
  the derived controls.
- Nine methods that return values now return int. Their binding keys were
  renamed in the dialog files' bindings; only the keys changed.
- GameObject.h befriends UICtlContainer, UnknownGameUiControl and UIDialog.

Near misses (`samples/ui/GameUiNearMisses.cpp`): 26 functions, listed with
their differences at the top of the sample. They include the UIControl and
UIListBox constructors, the UIControl destructor and several list-box and
scroll slots. Four deleting destructors and destructors (`0x00470430`,
`0x00479c60`, `0x00479c80`, `0x0047ae60`, `0x0047ae80`) are byte-exact,
but VC6 emits them only alongside those near-miss constructors. So they
compile exactly only in the sample and are not registered.

Not attempted: UIDialog's 16 KB loader `0x0046a920` and about 30 more
large edit, scroll, list-box, drop-down and video/progress functions.
