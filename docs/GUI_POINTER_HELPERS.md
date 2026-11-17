# GUI pointer helpers

`ToolTip::ShowControlToolTip` is exact in `src/reconstructed/GUIManager.cpp`;
`GUIUser::SetPointerDevice` `0x00488120` sits in the same unit between
`0x00488110` and `0x00488160`, with a `// Not exact` status comment. The test
links a validation unit holding only these two definitions.

## Retail and source evidence

The pinned executable SHA-256 is
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
The complete decoded tooltip body is `0x486b10..0x486b79` (106 bytes), ending in
`ret 4` followed by six alignment NOPs before `ShowText` at `0x486b80`.
Its only call is that existing canonical provider. The pointer ownership body
is `0x488120..0x488153` (52 bytes), with returns at `0x48813c` and `0x488151`,
followed by twelve NOPs before the next function at `0x488160`. Every branch
target is inside these complete extents; neither body has an EH frame.

Authentic VC6 SP3 `/O2 /GR /GX /MT /Z7` emits 106 and 52 bytes, with 106 and 46
relocation-resolved matching positions respectively. The tooltip is exact with
the pointer branch written as a `POINT` copy followed by `position.y += 16`;
component-wise stores scheduled the bottom load early and swapped the call
registers (89). Ownership differences reorder the owner-null and owner-this
checks; that method remains a nonexact candidate in samples.
The object the test compiles (the candidate plus the canonical tooltip body)
defines only those two public methods and imports
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

The test compiles the actual candidate (with the canonical tooltip body) and fixture as separate objects and
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

## Forms tried

0x00488120 (52 bytes, 46/52): retail keeps the `device ? owner : 0` branch
  (jmp + xor) and tests the owner for zero before comparing it with
  `this`. VC6 threads the xor arm into the next test whenever that test is
  the zero test (inline helper, if/else assignment, early-return layouts,
  `owner == 0` first, a result variable, a switch, an assignment in the
  condition, an int-cast ternary, `owner && owner != this`, an `owned`
  flag, nested if/else, goto, a `const` or `void*` owner, a `self` local,
  `!device ? 0 : owner`, `this == owner`); comparing with `this` first
  (below) keeps the branch, so only the order of the two tests differs.
  Also threaded (15/52): an inline owner helper (early returns or a
  ternary) with nested `if (owner) { if (owner == this) ... }`, `!= this`
  or `return owner == this;`, a `(GUIUser*)0` or `device != 0` ternary,
  a parenthesised ternary and an extra `hasOwner` local. Also threaded
  (6-18/52): `owner = 0; if (device) owner = ...` with nested tests, a
  `result` or `same` local, `return owner == this;` after a zero test, a
  nested ternary and a `while (owner)` wrapper.
  Session 14, all threaded (6-17/52): `if (owner) { if (owner != this)
  return 0; return 1; }` before the store block, `if (!owner) {...} else
  if (owner != this) return 0;`, a `result` local over if/else, and
  `owner && owner != this` followed by a second `if (!owner)` store block
  (the duplicated test that matches 0x004705d0 does not help here).
  Session 21, all threaded (6-25/52): a `__forceinline` or early-return
  static OwnerOf helper, the ternary repeated in both tests, the ternary
  as the `if` condition with `device->ownerUser != this` inside, an
  `int busy` flag, a `(int)owner` test, `owner != 0 ? owner != this : 0`,
  a goto to a shared `return 1`, dead `previous`/`old` locals, a copied
  owner local and `return owner == this ? 1 : 0` after the store block;
  /G6 and /ML keep 46, /O1 gives 36. Retail evidence: the unthreaded
  `jmp +2 ; xor r, r ; test r, r` occurs 31 times in .text; at the 13
  other sites inspected the merge also holds a memory store (the value
  stored to a member, as CollisionPoint's `inContact = cond ? 1 : 0`, or
  an inlined `new`'s EH state), where this function merges into a bare
  test, so no matched idiom carries over.
  Session 24, threaded (13-18/52) or unchanged: inline OwnerOf helpers with
  a by-reference or by-pointer out parameter, `(unsigned)owner > 0`,
  `owner != 0 && owner != this` followed by `== this`, a bool HasOwner
  helper, a `bool owned` local, `!owner || owner == this` nesting,
  `device ? owner : (GUIUser*)device`, `owner = (GUIUser*)device; if
  (device) ...`, `owner && owner == this`, `return owner == this` and the
  store block first.
  2026-11-14, threaded or worse (17-35/52), 46 unchanged: inline helpers
  that take the ternary's result as a parameter (an OwnedByOther test, a
  Claim helper holding both blocks, a three-way State helper under a
  switch), a `volatile` owner local, plain nested if/else, a bool result,
  the zero test written twice and the owner slot through a reference.
0x00488120 SetPointerDevice (48/52 bytes, 16 vs retail's 18 instructions): retail materialises
the ternary (`xor edx, edx` arm, then `test edx, edx`); VC6 threads the null arm straight to the
assign path in every spelling tried: owner-first and this-first nesting, `owner != this` early
return, `owner != 0 && owner != this`, if/else instead of `?:`, `owner = 0; if (device) ...`, and a
static inline OwnerOf(device) helper (48-49 bytes, all 16 instructions).
