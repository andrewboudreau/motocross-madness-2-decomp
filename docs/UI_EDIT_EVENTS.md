# Edit-box release and redraw methods

`UIEditBoxEvents.cpp` supplies two missing UIEditBox methods as ordinary C++.
The class's primary table `0x00552de8` independently identifies slot 56
`ButtonUp` at `0x00474b10` (144 bytes) and slot 38 `UpdateNeedsRedraw` at
`0x00479100` (107 bytes). Its complete-object locator and retail base descriptors
place UIControl, GameObject and BaseObject at nonvirtual offset zero. The
compiled GameUi translation unit selects these exact UIEditBox symbols.

The bodies are folded with UIScrollCtl::ButtonUp and
UIStaticText::UpdateNeedsRedraw respectively. Their existing readable source
provides the reconstruction, with only the qualified class names changed.
Full independent byte checks establish both UIEditBox methods under VC6 SP3;
shared addresses alone do not establish class identity. No alias, wrapper or
alternate receiver replaces either definition. Original TU ownership is not
established by this reconstructed source grouping.

ButtonUp reports a command when the edit box is hovered, argument zero is
supplied and its current state is two. It initializes seven event fields and
dispatches to the owning dialog. A handled event returns one; otherwise the
real UIControl::ButtonUp runs. That base method rechecks current hover/state,
calls virtual SetState and PlayControlSound when appropriate, clears heldButton
and returns zero. The owner is reloaded after SetState, so callback changes can
affect the subsequent sound argument. No event cleanup or extra field
initialization is invented.

UpdateNeedsRedraw calls the actual anchor-chain helper before considering the
control's hidden/empty status. Hidden or empty controls, or those with a hidden
or transitioning final anchor, decrement redrawFrames. A negative result is
clamped to zero and sets needsRedraw. Other fields remain unchanged. Native
checks include VC6's emitted wrap behavior at INT_MIN; this does not claim
portable signed-overflow semantics or safe traversal of cyclic anchor chains.

The [retained test](../tests/test_ui_edit_events.py) proves both complete methods,
both retail/compiled primary slots, and five real helper bodies. Each helper is
checked in the complete production GameUi object and in its separately compiled
unchanged source: ButtonUp (84 bytes), FindLastAnchor (32), GetName (7), SetState
(18) and RequestRedraw (21).

**4,992 native cases pass.** The 3,840 release cases exercise hovered/unhovered
states, signed argument extremes, handled/unhandled results, event mutations of
hover/state/owner, owner changes during SetState, and exceptions at dialog,
state and sound callbacks. Actual SetState invokes a forwarding virtual
RequestRedraw observer that executes the actual helper. Dialog dispatch and
sound are scripted boundaries; only initialized event fields are inspected.
The 1,152 redraw cases cover text presence/length, hidden state, zero through
three anchors, final-anchor flags, signed counter boundaries and complete
guarded storage. Independent expectations check return values, callback counts,
sound arguments and every changed byte, including state retained on exceptions.

Calls use prepared storage with actual headers and explicit layout assertions;
this does not establish a constructed control lifecycle, real dialog behavior,
audio or rendered menus. No retail code is executed by the production source.

The isolated real build `work/game-build/build-_6pth9ao/report.json` compiles
225 translation units and removes exactly these two missing names,
**346 → 344**, without new unresolved symbols. Existing source objects are
unchanged by adding the separate provider. Overall linkage remains incomplete.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_ui_edit_events.py -v
```
