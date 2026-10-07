# gameui.cpp (the UI controls)

`src/reconstructed/GameUi.cpp`, with the classes in `GameUi.h` and
`UIDialog.h`. RTTI names the classes: UIDialog and UIControl
(`UIControl`) and the derived button, static, edit, scroll,
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

Exact: 316 functions compile strict-exact from GameUi.cpp. 309 are
registered as calibration cases, including the 13 drawing, input and layout
functions in the file's last section ("Drawing and input slots"), the ten
promoted from the near-miss sample (UIControl slots 22, 23 and 41, UIScrollBar
`0x004753c0`, UIListBox slots 21 and 56 and `ScrollBy`, UIMultiState slot 48,
UIDDLListBox slot 66 and UIFrame `0x00472bc0`), the UIDialog constructor,
destructor and deleting destructor (`0x00469db0`, `0x0046a070`, `0x00469ff0`)
and `NotifyParent` (`0x0046ff70`). Seven sit
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
`??0UIDDLListBox@@QAE@HHPAUCameraRect@@PAVUIDialog@@PAVUIDropDownList@@@Z`.

Source forms the promoted functions needed (each one byte-exact only this way):
- UIControl slot 23 (`0x00471fd0`, a key or button press) and slot 22
  (`0x00472130`): the result of slot 55/56 goes into a local `handled` that
  one final `if (!handled) return GameObject::...; return 1;` tests. Every
  `return 1` inside the branches, with or without a goto, duplicates the
  epilogue; retail jumps back to one.
- UIControl slot 41 (`0x004715d0`): the zoom's right edge is
  `(int)(width + field_0x80)` with `float width = field_0x88` loaded into a
  local first (both member orders give `fld +0x80`), and the switch has
  cases 1, 4, 5, 9 and 10 (the retail index table maps type 2 to default).
- UIScrollBar `0x004753c0`: `float limit = travel` before the division, so
  the unsigned travel is converted once and held on the FPU stack.
- UIListBox `ScrollBy` (`0x00477730`): the clamp is `UnknownMaxInt(first, 0)`;
  the ternary and `max` forms and the result through another register.
- UIListBox slot 21 (`0x00478040`): VK_END compares and selects
  `rowCount - 1` twice (VC6 keeps it in a `lea`); a local gives `dec`.
- UIListBox slot 56 (`0x00477ff0`): `guiUser->field_0x1d8 == this` in that
  operand order and `field_0x230 += field_0x234; field_0x234 = 0;`.
- UIMultiState slot 48 (`0x00478810`): `if (currentState == 4 &&
  stateTable[..].focusImage) image = focusImage; else image = image;` then
  one `if (image)`.
- UIDDLListBox slot 66 (`0x00479df0`): the event code is this control's own
  `eventCode` (+0x74), not the drop-down list's.
- UIFrame `0x00472bc0`: the palette's two members are passed as inline
  conditionals (`pal ? pal->field_0x708 : 0`, as GUIManager.cpp does); named
  locals swap ecx/ebx.

- UIDialog's constructor `0x00469db0`: the resource name is copied by
  `if (resource) strcpy(resourceName, resource); else strcpy(resourceName,
  "");` (VC6 cross-jumps the two copies and computes the destination before
  the test; a `resource ? resource : ""` argument schedules it after). The
  member stores follow the retail store order; `timerList.Init(2, 1)` is the
  `ContainerList.h` line 59 `new` under EH state 1.
- UIDialog's destructor `0x0046a070`: the timers are released with
  `while ((timer = timerList.Get(i++)) != 0)` (the post-increment copies the
  index into ebx before the bounds test; a `for` with `i++` in its step
  increments after the call).
- `NotifyParent` `0x0046ff70`: every read of the parent goes through the
  `parentDialog` member; a local copy hoists the parent's vtable load above
  the event stores.

UIDialog layout (`UIDialog.h`, 0x7f58 bytes; `samples/gameui/UIDialogLayoutProbe.cpp`
checks every offset and the derived dialogs' sizes at compile time). The
constructor `0x00469db0` initialises every member listed, the destructor
`0x0046a070` releases them, and the GameUi.cpp, GUIManager.cpp and dialog
procedure bodies read them:
- +0x2c `parentDialog`, +0x30 `guiManager`, +0x34 `guiUser`, +0x38
  `resourceName[0x80]` (strcpy'd from the constructor's argument), +0xb8
  `resourceArchive` (slot 27, released through the resource manager),
  +0xbc `isShown` (1), +0xc4 `openingMenu` (the `a` of GUIManager::ShowDialog;
  the dialogs compare it with menu ids), +0xc8 `animatesControls` (1),
  +0xcc/+0xd0/+0xd4 the procedure callbacks, +0xd8 `dialogFont`
  (DeleteObject'd), +0xdc `dialogFontHeight`, +0xe0 `dialogFontFace[0x28]`.
- +0x110 `dialogBackground`, +0x114 `soundGroup`, +0x118 `textColors[10]`
  (0xffffff), +0x140 `dialogPalette` (released when owned, +0x7f0c, and not
  the parent's), +0x144/+0x148/+0x14c GUIManager's show flags and input
  state, +0x154 (1: notify the parent with kind 9 on destruction), +0x158
  `scaleToScreen` (1), +0x15c `isPopup`, +0x160 `screenArea`, +0x170
  `popupAlignment` (0x12), +0x174/+0x178 the resource screen size
  (640x480), +0x17c `dialogResult`, +0x180 (7).
- +0x18c `imageTable[500]` (released) and +0x95c `cursorAnimation`, +0x960
  `soundTable[20]`, +0x9b0/+0x9b4 their counts, +0x9b8 `parentBackground`
  (restored into the background when +0x7f10 is set), +0x9bc `sortingList`,
  +0x9c0 `lastFrameTime`, +0x9c4/+0x9c8 the scales (1.0f), +0x9cc
  `sectionTable[500]` (0x3c-byte `UnknownGameUiSection`, memset to 0),
  +0x7efc `sectionCount`.
- +0x7f00/+0x7f04 the control-draw clip region and replaced font, +0x7f18
  (1), +0x7f1c `joystickCentred` (1), +0x7f20 `dialogTextures`, +0x7f24
  `ContainerList<UITimer*> timerList` (Init(2, 1)), +0x7f3c
  `controlContainer`, +0x7f40 `sendFrameEvent`, +0x7f44 `screenGrab`,
  +0x7f48 `grabRegion` (-1), +0x7f50 `isClosing`, +0x7f54 `closeFrame`.
- Slots 27-31 are UIDialog's own virtuals (vtable `0x00552a9c`): the
  dialog setup `0x0046a300`, `0x0046e8c0`, the procedure `0x00470020`,
  `0x00470040` and `0x00470050`. The derived dialogs override 29 (and 31
  where they store settings), so their procedures mangle as `UAE`.
- The former `UnknownGameUiDialog` (GameUi.cpp) and `UnknownGuiDialog`
  (GUIManager.h) views are gone; the derived dialog classes in DialogProc.h,
  DlgProcs.h, OptionProcs.h, SelectGamePicProcs.h, ProCircuitProcs.h,
  InGameProcs.h, NetProcs.h, TrackRecordDlg.h and UIDialog.h declare only
  their own members from +0x7f58 (they named +0x2c, +0x30, +0x34, +0xc4,
  +0x110 and +0x118 for their own use before; `openingMenu` and
  `textColors[0]` are the UIDialog names).

Class layout (sizes from the `new` sites in `0x0046a920`, `0x00479ea0` and
dlgprocs.cpp, and from the constructors):
- UIControl (`UIControl`) is 0x1ec bytes. Its derived classes
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
- UIDialog.h names the parser's members: +0x158 (scales), +0x15c (popup),
  +0x170 (popup alignment), +0x174/+0x178 (resource screen size), +0x18c
  (500 images), +0x960 (20 sounds), +0x9b0/+0x9b4 (their counts), +0x9b8,
  +0x7f0c and +0x7f10.

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
- UIDialog.h has slot 27's real 14-argument signature (GUIManager's
  `ShowDialog` passes its int `b` and `d` as the area and the name).
- GameObject.h befriends UICtlContainer, UIControl and UIDialog.

Near misses (`samples/ui/GameUiNearMisses.cpp`): 28 functions, listed with
their differences at the top of the sample, among them the UIControl and
UIListBox constructors, the UIControl destructor, UIListBox `0x00477110`
(adds an image row and returns 0/1; retail places the epilogue after the
stream-failure block), several list-box and scroll slots and the resource
parser. Most of the remaining ones differ only in register choice (slots 49
of UIControl and UIButton, UIAnim's constructor and advance, the colour-key
test) or in block placement (UIScrollCtl slot 60, UIFrame's file
constructor); the forms tried are noted in the sample. UIControl's deleting
destructor (`0x00470430`) is byte-exact but VC6 emits it only alongside the
near-miss UIControl constructor, so it is not registered.

The resource parser `0x0046a920` (16 KB, in the sample): it lists the
resource's sections between "Set_Anim", "Set_Sound" and "Set_Control"
markers into the dialog's named entries, then reads them in four passes
("Set_Info", images, sounds, then "Set_Default" and the controls). 92% of
its instructions agree once stack offsets and relocations are ignored; the
differences are register allocation (retail keeps the current control in
ebx) and the frame, 0x9c74 bytes in retail against 0x9bf4 here. The large
arrays (`buffer`, `tip`, the three 50-entry default tables) and the scalars
at the frame's base have the same offsets; the 0x80-byte key buffers
between them do not. Probes (`char` buffers with chosen use counts) show how
VC6 lays a frame out: locals are sorted by static use count, the most used
nearest esp, equal counts by size (bigger higher) and then by a fixed
permutation of the use order (10 equally used buffers come out as uses
10 1 9 3 8 4 7 2 6 5 from the top); declaration order plays no part. Retail
therefore encodes different use counts for the key buffers, and it has one
0x80-byte local that no instruction references, allocated between `toolTip`
and `fxSoundOut` (esp+0x2184 at the frame's base), which shifts the buffers
below it by 0x80.
