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

GUIManager owns the dialogs, the background image and screen grab, the
tool-tip font, the cursors and up to four GUIUsers; each GUIUser routes
input devices (a `ContainerList<GUIInputDevice*>` at +0x1e0), focus,
hover/capture, IME state, a ToolTip and a GUICursor. HiResMeter is the
"MS Timers" overlay at `0x0067b468` (Game.h's `g_UnknownGlobal56c470`
points to it). KrustyUI.h still declares GUIManager and GUIUser as
`UnknownKrustyUIGui` / `UnknownKrustyUIGuiLayer` views; unifying them
would change KrustyUI's mangled names.

Exact: 96 calibration cases. Near misses
(`samples/ui/GUIManagerNearMisses.cpp`, notes there): the GUI setup
`0x004853b0`, show dialog `0x00485a70`, `0x00485c80` (exact only with
UIDialog padded to 0x7f58, which would shift the derived dialogs'
fields), the screen grab `0x00486170`, the tool-tip layout `0x00486b10` /
`0x00486b80`, `0x00487870` and `0x00488120`. HiResMeter slot 8
`0x00488440` uses `rdtsc` (inline assembly) and is not attempted.
