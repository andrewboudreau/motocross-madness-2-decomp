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
"MS Timers" overlay at `0x0067b468` (Game.h's `g_HiResMeter`
points to it); its timer fields are named after the overlay's column header
(`callsThisTick`, `msThisTick`, `peakMs`, ...). KrustyUI's former
`UnknownKrustyUIGui` / `UnknownKrustyUIGuiLayer` declarations now alias
GUIManager / GUIUser. Their unchanged canonical class declarations live in
`GUIManagerCore.h`; Windows drawing types remain in `GUIManager.h`.
KrustyUI.h only forward-declares the shared classes, so unrelated TrackGame
consumers do not acquire additional GUI definitions.

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
(`samples/ui/GUIManagerNearMisses.cpp`, notes there): the screen grab `0x00486170`, the tool-tip layout `0x00486b10` and
`0x00488120`. HiResMeter slot 8
`0x00488440` uses `rdtsc` (inline assembly) and is not attempted.

## Shared caller connections

KrustyUI, BikeRace, EventManager, TrackGame and dialog consumers now use the
canonical manager declarations. Eight distinct production imports resolve
to existing GUIManager.cpp providers: OpenDialogResource, CloseDialogResource,
FindInputDialog, GetUser, CreateCursors, ShowCursors, ReleaseBackground and
EnableWindowClipper. The separate ShowDialog alias joins the existing canonical
name; the independently validated ShowDialog body is now supplied as described below.
Thus this slice removes nine unresolved identities while supplying eight
previously disconnected implementations.

Retail constructor/allocation evidence fixes GUIManager at `0x3e0` and GUIUser
at `0x1f4`. Consumers preserve userCursor `+0x30`, userToolTip `+0xc0`, the
manager's keyboard device `+0x30c` and four users `+0x330`. Dialog pointers use
UIDialog at the method boundary; still-provisional dialog subclass views use
explicit casts rather than asserting their complete inheritance. TrackGame's
slot-26 dispatch is named by the shared base declaration, MarkPendingRelease.

The startup sample now expresses SetUp's actual ten arguments (`ret 0x28` at
`0x004857ea`) and AppendSibling's separate second argument, -1. Retail calls
SetUp at `0x00498a6b`, then pushes its returned object and calls AppendSibling
at `0x00498a73`; the -1 was pushed before SetUp's argument list. The font-size
argument is a short, and GUIUser::AcceptDevice receives GUIInputDevice* and
returns int. These corrections do not promote KrustyUI::Create: it remains a
nonexact sample, currently 1102 emitted bytes against retail's 1104. The float
font-size adjustment still uses the existing four-byte storage through an
explicit float view; the canonical member layout is unchanged.

`tests/test_gui_connections.py` compiles the real callers and providers,
checks these layouts and retail setup calls, and distinguishes production
connections from sample-only startup calls. Full calibration preserves every
previous strict retail function. No additional dialog bodies or startup
implementation are admitted by this declaration consolidation.

## ShowDialog implementation and validation

`GUIManager::ShowDialog` at `0x00485a70` is implemented in the canonical
`src/reconstructed/GUIManager.cpp`, immediately before AttachDialog. Differential
validation found and corrected a concrete argument error: after a successful
virtual slot-27 call, retail passes the sixth ShowDialog parameter (`c`) to
AttachDialog. The former sample passed the second parameter (`a`). At
`0x00485b79`, the retail load from `[esp+0x38]` with a 32-byte fixed frame
selects `c`; slot 27 still receives `a`.

All 341 compiler-reported function bytes match retail with all eight
relocations resolved. Independent retail control-flow analysis reaches exactly
132 instructions covering `[0x00485a70, 0x00485bc5)`. Both exits are `ret 0x20`,
at `0x00485b96` and `0x00485bc2`; there is no tail or indirect jump. Eleven NOPs
separate the completed body from the separately called AttachDialog entry at
`0x00485bd0`. The old 345-byte sample range ended inside that padding. The
regression asserts the full reachable extent, both returns and the entire gap;
its extent is not chosen by trimming compiler output to a requested size.

`tests/test_gui_show_dialog_behavior.py` compiles the real production translation
unit and a temporary extraction of the same source function with VC6 SP3. It
requires each independently emitted body to match all 341 retail bytes with
relocations resolved and a compiler-reported extent. The extraction is linked
to the finite test fixture so its six dependencies can be scripted without
conflicting with their production definitions. No duplicate sample body remains.
The test verifies the retail hash and control flow, extracts the reference only
into a temporary file, and redirects its eight direct calls to the same scripted
dependencies. The real virtual slot-27 dispatch remains intact. Reference code
and dependency substitutes are used only by the test executable.

The native comparison passes 34,560 scenarios: all low-six-bit flag values,
null/distinct/self parent identities, five current-dialog lookup scripts
(including a different second result), null/self/distinct slot-27 returns,
zero/positive/negative waits, zero/nonzero palette flags and callbacks with or
without state mutations. It compares return identity, ordered calls with all
arguments and copied rectangle/string inputs, and all bytes of the manager,
three dialogs and render target. Distinct `a` and `c` values retain regression
coverage for the discovered bug. The fixture covers both signs of font size,
null/non-null caller rectangles and multiple screen dimensions.

This is finite behavioral evidence with scripted dependencies. It does not
exercise actual rendering, dialog resources or message-loop interaction.
Both manager and dialog background-grab paths still require the missing
CopyScreenToTexture implementation, so this test does not establish working
modal menus or startup.
