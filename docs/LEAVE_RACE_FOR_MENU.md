# Race-to-menu transition

`EventManager::LeaveRaceForMenu(int)` is byte-exact in
`src/reconstructed/EventManager.cpp` (strict calibration case
`EventManager ?LeaveRaceForMenu@`). Its semantic name remains provisional.
The differential test below compiles that body alone, taken from the
canonical unit, so the harness links without the rest of the event manager.

## Complete extent and identity

The retail function is **531 bytes**, `0x0045e710..0x0045e923`, ending with
`ret 4` at `0x0045e920`. Thirteen NOPs precede the next function. All direct
branches target decoded instructions in the body. A single constructor
unwind state has its 23-byte action at `0x0054ab30` and 10-byte handler at
`0x0054ab47`; FuncInfo `0x00560c80` points to the one-entry unwind map at
`0x00560ca0`. The test checks the complete bodies and all external calls,
including the EH handler's jump to the actual linked VC6 CRT handler.

The sole virtual call is Game slot 19, `SwitchDisplayMode`. Retail loads
the old game vptr before calling `FindDisplayMode`, then reloads the global
game for ECX. The test preserves this distinction and checks an independent
case where the lookup callback changes the global game.

The inline `TransDlg` constructor writes the primary vptr at
`0x0045e8c7`. RTTI identifies `TransDlg`, COL `0x0055c008` has object offset
zero, and all 32 entries of table `0x00552624` agree with the emitted table.
Its 30-byte deleting wrapper and five-byte destructor match strictly.
The table, constructor store and wrapper's direct destructor call support
that role; shared short-function addresses alone do not identify the class.
The 0x7f64 allocation and `Trans.dtm` resource string are checked independently.

Authentic VC6 SP3 `/O2` emits the retail **531 bytes** for the operation,
after resolving every relocation. The tool tip option is stored through an
inline setter (`EnableToolTips`): its parameter keeps the option read out of
VC6's eax/ecx/edx rotation of expression temporaries, which a plain
assignment left one step off retail's from that store to the end (533 bytes,
454 matching positions). The standalone body emits only the operation
and its two generated destructor helpers, with no CRT initializer. Existing
source-built objects provide its game dependencies. All thirteen directly
called game methods and both debug allocation/deallocation functions remain
strict, as does the concrete PCGame display-switch method (sixteen complete
provider bodies checked in `build-5u6vd8ni`). The function does not rely
on a partial helper. Its scripted test still does not execute these actual
providers. The remaining external runtime imports are the normal CRT and
IMM32 dependencies.

## Functional build review

The function returns immediately without a UI. Otherwise it ends the active
input dialog, restores the saved input-method context and clipping when
enabled, clears transition flags, conditionally stops CD audio, and requests
640x480x16 if the current mode differs. It recreates the background, applies
the tooltip setting, conditionally records race results, shows cursors,
creates and shows a transition dialog, assigns its destination menu, and
sets primary sound volume to zero. Callback return values are retained or
ignored exactly where the instructions do.

`tests/test_leave_race_menu_behavior.py` runs **9,437 native scenarios**
against the complete hash-pinned retail function and the canonical body
compiled alone with `/O2`; the independent fixture uses
`/Od`. Only audited external calls in the private reference mapping are
redirected. The candidate's complete machine code runs without patches.
No copied reference code, retail dispatch or test-only implementation enters
the production build.

The fixture snapshots all storage for two games, UIs, GUI managers, users,
tooltips, display and sound views, input dialogs, mode arrays and the
transition dialog at every dependency boundary and after execution. It
compares ordered calls, receiver identity, values, live allocation state,
normal completion, exceptions and access-violation details. The only
normalizations are the independently checked transition vptr and diagnostic
filename identity. The native constructor/vptr/EH machinery executes; GUI
constructor behavior itself is a scripted dependency.

Coverage includes all 1,024 combinations of the UI/dialog/IME/CD/replay/demo
and width/height/depth gates, nine race modes including signed extremes,
three valid mode-array indexes, and varied menu IDs and callback returns.
Every boundary in three representative traces is tested with an exception
and four mutation families: global-game replacement, flag/settings changes,
UI/display/sound replacement, and GUI/tooltip/input-context replacement.

Independent expected outcomes check the successful transition, absent-UI
return, already-correct display/no-results path, and cached-vptr behavior.
Allocator failure reaches the retained null-dialog write fault at `0x7f60`;
constructor failure runs the placement-delete cleanup, while a later
ShowDialog exception leaves the allocated dialog live. Null user, tooltip,
game and display faults are compared. The function does not invent recovery
or cleanup absent from retail.

This is finite evidence with scripted GUI, display, input-method, CD/audio,
result-recording and allocation boundaries. It does not exercise real
windows, mode switches, dialog loading, result-file writes, audio devices
or live menu rendering. Invalid arbitrary pointers and out-of-range mode
array indexes are outside the supplied storage contract. Original source
identity and whole-game behavior are not established by these comparisons.

The build takes the function from `src/reconstructed/EventManager.cpp`; the
former reviewed-nonexact sample entry in `config/game_build.json` is gone.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_leave_race_menu_behavior.py -v
```
