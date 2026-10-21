# GUI pointer helpers

The complete nonexact candidates in `samples/ui/GuiPointerHelpers.cpp` supply
`ToolTip::ShowControlToolTip` and `GUIUser::SetPointerDevice`, two names missing
from the source-built game's linker report. They are extracted unchanged from
`GUIManagerNearMisses.cpp`, which includes the standalone source at the original
position. This grouping does not establish an original translation unit.

## Retail and source evidence

The pinned executable SHA-256 is
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
The complete decoded tooltip body is `0x486b10..0x486b79` (106 bytes), ending in
`ret 4` followed by six alignment NOPs before `ShowText` at `0x486b80`.
Its only call is that existing canonical provider. The pointer ownership body
is `0x488120..0x488153` (52 bytes), with returns at `0x48813c` and `0x488151`,
followed by twelve NOPs before the next function at `0x488160`. Every branch
target is inside these complete extents; neither body has an EH frame.

Authentic VC6 SP3 `/O2 /GR /GX /MT /Z7` emits 106 and 52 bytes, with 89 and 46
relocation-resolved matching positions respectively. Tooltip differences are
load scheduling and register allocation; ownership differences reorder the
owner-null and owner-this checks. These remain nonexact candidates in samples.
The separately compiled object defines only those two public methods and imports
only canonical `ToolTip::ShowText` plus CRT `__fltused`. No new game dependency
is introduced. Existing `GUIManager.h` declarations preserve the field offsets;
these direct methods do not require a new vtable or class-layout claim.

The tooltip uses the device position plus 16 in y when present. Otherwise it
uses signed integer division toward zero to center the control horizontally,
with its bottom as y. It passes the original text pointer and duration 1.0.
The ownership method rejects another user, returns success without any write
for a device already owned by this user, and assigns a free device. Clearing
this user's pointer does not clear the former device's owner. The candidate
preserves these behaviors, including inconsistent preexisting ownership state.

## Functional build review

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_gui_pointer_helpers.py -v
```

The test compiles the actual candidate and fixture as separate objects and
compares 19,606 native scenarios against the hash-pinned retail functions.
It compares complete user, device, tooltip, control and dialog storage, return
values, text-pointer identity, callback arguments, state at the call boundary,
callback side effects and exceptions. Ownership cases vary old and new devices,
null/free/same/other owners, seeded surrounding state, and an invalid device.
Tooltip cases cover signed odd/even/inverted rectangle widths, integer extremes,
8,192 deterministic varied coordinates, pointer/no-pointer placement, null and
non-null text, mutation and C++ exception callbacks, and four null/invalid
pointer-chain faults including access kind and address.

Only the tooltip's direct `ShowText` call is redirected to a scripted boundary;
that renderer's behavior and real input devices are outside this fixture.
All retail execution and patching remain private validation code. A separate
check compiles the aggregate and compares both complete emitted bodies and
all relocation sites/types/names against the standalone object. The original
aggregate before extraction was also checked locally: both bodies and their
one total relocation were byte-identical after extraction. This is finite
behavioral evidence, not arbitrary invalid-object guarantees or a running game.
