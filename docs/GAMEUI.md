# gameui.cpp (the UI controls)

`src/reconstructed/GameUi.cpp`, with the classes in `GameUi.h` and
`UIDialog.h`. RTTI names the classes: UIDialog and UIControl
(`UnknownGameUiControl`) and the derived button, static, edit, scroll,
slider, list box, multi-state, animation, timer, frame and drop-down
controls. Member names follow the evidence listed under "Names"; the rest
stay provisional.

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
- Every method is declared on the control whose members it reads: list box
  rows (0x00476860..0x00477e60, UIListBox +0x1ec..+0x24c), edit box text
  (0x00473820..0x00473fc0, UIEditBox +0x1ec..+0x224; 0x00473ef0 reads only
  UIControl's text but sits among them and is called on edit boxes), scroll
  bar position (0x004751c0..0x004755c0, UIScrollBar +0x1ec..+0x21c),
  multi-state states (0x00478860..0x00478cf0, UIMultiState +0x1ec..+0x1f4),
  the radio group (0x00479310, 0x004793f0: iterate "UIRadioButton"), a
  column button's list (0x00473390, UIButton +0x1ec, a UIListBox* that slot
  56 sorts) and the progress step (0x0047b370, UIProgressBar). Their bodies
  use `this` directly. Dialog files find controls with
  `UIDialog::FindControl` (0x0046ebf0), whose second argument is the control type (1 button,
  2 multi-state, 3 list box, 4 radio button, 6 drop-down list, 7/8 scroll
  bar, 0xb edit box), and cast the result to that control.
- Slots 65 and 66 belong to UIListBox: UIControl's vtable has 65 entries,
  UIListBox's and UIDDLListBox's 67. UIListBox's slot 66 is 0x00477d30
  (the click report, `int* handled`); UIDDLListBox overrides both.
- 0x004755c0 returns +0x1f0. It is called on scroll bars, on list boxes
  (0x00475200, UIScrollBar slot 55, UIDDLScrollBar slot 55: the first row
  shown) and on multi-states and radio buttons (0x004793f0, the dialog
  procedures: the current state). UIScrollBar, UIListBox and UIMultiState
  each declare it; the identical bodies share the one address (the
  UIScrollBar copy is the registered case).
- UIDropDownList has an inline accessor for its button (+0x1ec):
  SelectGamePicProcs.cpp's 0x004f3260, 0x004f69d0 and 0x004f8700 read the
  button through it; reading the field directly lets VC6 merge their two
  branches, which retail does not. It is defined after the class: an
  in-class body changes the reload order in the header-sensitive near miss
  0x004d59a0 (`samples/ui/ProCircuitProcsNearMisses.cpp`).
- A local copy `UIListBox* p = static_cast<...>(this)` changes VC6's
  register and address-operand choices in 0x004775f0 and 0x00477900;
  `this` does not.
- GameUi.cpp's UnknownGameUiDialog names the parser's members: +0x158
  (scales), +0x15c (popup), +0x170 (popup alignment), +0x174/+0x178
  (resource screen size), +0x18c (500 images), +0x960 (20 sounds),
  +0x9b0/+0x9b4 (their counts), +0x9b8, +0x7f0c and +0x7f10.

Names (strong inference from bodies and literals, not original symbols):
- The control loader in `0x0046a920` reads each control's .dtm keys and
  stores them in UIControl members or passes them to setters; those keys
  name the members and setters: "GroupId" (+0x78 `groupId`, used by
  `EnableGroup`/`ShowGroup`), "AttachId" (+0x7c `attachId`, slot 52),
  "TextColor" (`SetFontColor`), "DropColor", "TextDrop", "TextAlign",
  "ShapeBounds", "Anchor"/"RelAnchor" (`SetAnchor`), "Show" (`Show(show,
  1)`), "SoundNorm".."SoundClick" (`SetSound`), "KeyBind", "Pre3D",
  "Permanent", "Moveable", "FontName"/"FontHeight", "ToolTipText", "FX"
  (`StartTransition` 100-106) and the list keys "AutoSort", "SelectColor",
  "Selectable", "AllowWScroll", "SelectBoxColor", "ItemBoxColor".
- Other methods are named for what their bodies do: `FindControl` (name
  and type match over the "UIControl" iterator), `SetText` /
  `SetTextFromResource` (LoadStringA), `EndDialog` (stores the result at
  +0x17c and starts closing), `AddTimer`/`RemoveTimers`, `NotifyParent`,
  the list-box row methods and the UIAnim frame methods.
- `UIDialog::FindSectionObject` (0x0046e9a0) returns the object built for
  a .dtm section (control, image or sound), falling back to the GUI's.
- `DialogEventKind.h` names the dialog event kinds after their senders
  (1 command, 2 list click, 4 create, 5 init, 6 close, 7 timer, 10 edit
  done, 13 list double click, 18 frame, 19 edit change). It is a separate
  header because declaring the enum in DialogProc.h changes VC6's register
  choice in BikeRaceNearMisses' 0x00419970.

Other header facts:
- InputDevice.h befriends UIDialog, UIScrollBar and UIListBox (they read
  deviceKind).
- GameUi.cpp's UnknownGameUiDialog has slot 27's real 14-argument signature
  and names +0x38 (resource name), +0xb8, +0x118 (text colors), +0x95c,
  +0x7f1c and +0x7f40.
- GameObject.h befriends UICtlContainer, UnknownGameUiControl and UIDialog.

Near misses (`samples/ui/GameUiNearMisses.cpp`): 38 functions, listed with
their differences at the top of the sample, among them the UIControl and
UIListBox constructors, the UIControl destructor, UIListBox `0x00477110`
(adds an image row and returns 0/1; retail places the epilogue after the
stream-failure block), several list-box and scroll slots and the resource
parser. UIControl's deleting destructor
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
