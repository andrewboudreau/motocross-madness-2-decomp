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

Exact: 302 functions compile strict-exact from GameUi.cpp. 295 are
registered as calibration cases, including the 13 drawing, input and layout
functions in the file's last section ("Drawing and input slots"). Seven sit
at the addresses of the deliberately non-strict compiler-shape probes in
`samples/calibration/CompilerShapeProbe.cpp`; those probes are kept
unchanged as regression probes. The 17 added with the class-size
restructure are the
UIDDLStatic, UIDDLButton and UIDropDownList constructors (`0x00479b50`,
`0x00479bf0`, `0x00479ea0`), UIDDLButton's deleting destructor and
destructor (`0x00479c60`, `0x00479c80`), the deleting destructor and
destructor at `0x0047ae60`/`0x0047ae80` (slot 0 of both UIDDLStatic and
UIVideoStatic; the bodies are shared), UIVideoStatic's constructor,
`0x0047ae90` and slots 10 and 40, and UIProgressBar's constructor,
destructor pair, slots 40 and 48 and `0x0047b3d0`. The
UIDDLListBox constructor case (`0x00479d10`) is re-keyed to its retail argument order
`??0UIDDLListBox@@QAE@HHPAUCameraRect@@PAVUnknownGameUiDialog@@PAVUIDropDownList@@@Z`.

Class layout (sizes from the `new` sites in `0x0046a920`, `0x00479ea0` and
dlgprocs.cpp, and from the constructors):
- UIControl (`UnknownGameUiControl`) is 0x1ec bytes. Its derived classes
  hold their own members: UIButton 0x1f0 (+0x1ec the list a column button
  sorts), UIScrollCtl 0x1f0, UIDDLButton 0x1f4, UIStatic 0x1ec,
  UIDDLStatic 0x1f0, UIVideoStatic 0x200, UIProgressBar 0x204,
  UIStaticText 0x1ec, UIDropDownList 0x21c, UIEditBox 0x230, UIScrollBar
  0x220, UIDDLScrollBar 0x224, UIListBox 0x250, UIDDLListBox 0x254,
  UIMultiState 0x1f8, UIRadioButton 0x1f8.
- UIVideoStatic and UIProgressBar derive from UIStatic (RTTI base class
  descriptors); they moved from DlgProcs.h to GameUi.h, with MediaControl
  (RTTI vtable `0x005551c0`). MainDlg's movie control (+0x7f58) is a
  UIVideoStatic*.
- UIListBox's constructor is `(id, rows, area, owner)` and UIDDLListBox's
  `(id, rows, area, owner, list)`: 0x00475c70 passes its first, third and
  fourth arguments to UIControl, and 0x00479ea0 pushes 100 as the second.
- Methods the dialog files call through `UnknownGameUiControl*` stay
  declared on it (their mangled names are bound by those files); bodies
  that belong to a derived control read it through inline accessors such as
  `UnknownInlineListBox()`. A local `UIListBox* p = static_cast<...>(this)`
  changed VC6's register and address-operand choices in 0x004775f0 and
  0x00477900, the accessor did not; in the sample's 0x00477bc0 near miss it
  is the other way round, so the sample casts directly.
- Two read-only `__declspec(property)` members keep the dialog files'
  `control->field_0x1ec` (a list box's row count) and
  `control->field_0x1fc` (a drop-down list's list box) compiling through
  UIControl pointers (SelectGamePicProcs.cpp, DlgProcs.cpp,
  ProCircuitProcs.cpp, OptionProcs.cpp); all their cases stay exact.
- GameUi.cpp's UnknownGameUiDialog names the parser's members: +0x158
  (scales), +0x15c (popup), +0x170 (popup alignment), +0x174/+0x178
  (resource screen size), +0x18c (500 images), +0x960 (20 sounds),
  +0x9b0/+0x9b4 (their counts), +0x9b8, +0x7f0c and +0x7f10.

Other header facts:
- InputDevice.h befriends UIDialog, UIScrollBar and UIListBox (they read
  deviceKind).
- GameUi.cpp's UnknownGameUiDialog has slot 27's real 14-argument signature
  and names +0x38 (resource name), +0xb8, +0x118 (text colors), +0x95c,
  +0x7f1c and +0x7f40.
- GameObject.h befriends UICtlContainer, UnknownGameUiControl and UIDialog.

Near misses (`samples/ui/GameUiNearMisses.cpp`): 37 functions, listed with
their differences at the top of the sample, among them the UIControl and
UIListBox constructors, the UIControl destructor, several list-box and
scroll slots and the resource parser. UIControl's deleting destructor
(`0x00470430`) is byte-exact but VC6 emits it only alongside the near-miss
UIControl constructor, so it is not registered.

The resource parser `0x0046a920` (16 KB, in the sample): it lists the
resource's sections between "Set_Anim", "Set_Sound" and "Set_Control"
markers into the dialog's named entries, then reads them in four passes
("Set_Info", images, sounds, then "Set_Default" and the controls). 92% of
its instructions agree once stack offsets and relocations are ignored; the
differences are register allocation (retail keeps the current control in
ebx) and a frame 0x80 bytes larger in retail (a buffer at +0x2174 that no
instruction reads), which shifts the stack offsets.

Not done:
- `0x00477110` (adds an image row from a file): retail returns 0/1 but
  GameUi.h declares it `void`, and SelectGamePicProcs.bindings.json binds
  `?UnknownFunction477110@UnknownGameUiControl@@QAEXPBDHHH@Z`. Returning
  int renames it to `...@@QAEHPBDHHH@Z`, so the change waits for that key.
