# GUIManager.cpp

`src/reconstructed/GUIManager.h` / `GUIManager.cpp`. Evidence: the
`__FILE__` literal `D:\aardvark\VC\krusty2\GUIManager.cpp` (lines 672,
1140/1144, 1639, 1890/1898), containerlist.h lines 59/71, and RTTI for
GUICursor : GameCursor, GUIManager, UIDlgContainer, ToolTip,
GUIInputDevice, GUIUser and HiResMeter. The TU runs from `0x00484dd0` to
`0x004886df` (strong inference: its `.CRT$XCU` initializer entries
`0x0056623c..0x0056624c`, its `.data` from "GroundFog" `0x0056c338` to the
HiResMeter strings, and InGameProcs.cpp starting at `0x004886e0`). Methods
whose bodies are unambiguous are named for them (`OpenDialogResource`,
`GrabBackground`, `CreateFilledTexture`, `CreateCursor`, `AcceptDevice`,
`ToolTip::ShowText`, ...). Methods that other files call through
KrustyUI.h's alias keep their provisional names, and so do the remaining
members.
Named members: `setupPalette` (+0x38) and `setupCallback` (+0xd4) hold
`SetUp`'s palette and last argument; `paletteChanged` (+0x1f0) is set by
`0x00486150` with the palette, cleared by slot 10 and makes `0x00485bd0` wait
three frames. GUIUser: `pointerX`/`pointerY` (+0x34/+0x38, copied from the
pointer device by `0x00487620`), `controlDown` (+0x3c, 32 flags that UIDialog
slots 23 and 22 set and clear by control index), `pendingHoverControl` and
`pendingFocusControl` (+0x1cc/+0x1d0, stored by `0x00487730`/`0x00487790`
and applied by `UpdateFocus` through `SetHoverControl`/`MoveFocus`) and
`hoverControl` (+0x1d8, what those two methods last set).

GUIManager owns the dialogs, the background image and screen grab, the
tool-tip font, the cursors and up to four GUIUsers; each GUIUser routes
input devices (a `ContainerList<GUIInputDevice*>` at +0x1e0), focus,
hover/capture, IME state, a ToolTip and a GUICursor. HiResMeter is the
"MS Timers" overlay at `0x0067b468` (Game.h's `g_UnknownGlobal56c470`
points to it); its timer fields are named after the overlay's column header
(`callsThisTick`, `msThisTick`, `peakMs`, ...). KrustyUI.h still declares GUIManager and GUIUser as
`UnknownKrustyUIGui` / `UnknownKrustyUIGuiLayer` views; unifying them
would change KrustyUI's mangled names.

Exact: 100 calibration cases, among them `OpenDialogResource` `0x00485c80`
(its `new` allocates the 0x7f58-byte UIDialog that UIDialog.h now declares)
and `SetUp` `0x004853b0` (its font-size parameter is a `short`: the slot of a
sub-int parameter is freed at entry, which is where retail homes the
ControlInterface pointer; docs/VC6_FRAME_LAYOUT.md) and the focus move
`0x00487870`, whose IME switch is written as
`if (a && !strcmp(a, "0123456789")) EnableImeInput(0); else EnableImeInput(1);`
(retail has one call site per value), and the tool-tip `ShowText`
`0x00486b80`: the width limit is a named local
(`int limit = target->targetWidth - 2;`, retail's `sub eax, 2`; the repeated
expression is CSE'd as `add eax, -2`) and the padded width and height are
written back into the `SIZE` (`size.cx = width; size.cy += 2;`), two dead
stores retail keeps.
The dialog is a plain `UIDialog*` everywhere (the former `UnknownGuiDialog`
view is gone; `FindSectionObject` returns the dialog's `Sound*`). Near misses
(`samples/ui/GUIManagerNearMisses.cpp`, notes there): show dialog `ShowDialog` `0x00485a70`, the screen grab `0x00486170`, the tool-tip layout `0x00486b10` and
`0x00488120`. HiResMeter slot 8
`0x00488440` uses `rdtsc` (inline assembly) and is not attempted.
