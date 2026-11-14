// Near misses for GUIManager.cpp (canonical file:
// src/reconstructed/GUIManager.cpp, included below for its types). Check:
// compile this file and compare each function with
// src/reconstructed/GUIManager.bindings.json.
//
// 0x004853b0 (SetUp) is exact in src/reconstructed/GUIManager.cpp with a `short`
//   font-size parameter (docs/VC6_FRAME_LAYOUT.md: dead argument slots).
// 0x00486170 (896 bytes): same logic; retail keeps three more stack locals
//   (frame 0xb0 against 0xa4) and allocates registers differently around
//   the row copy and the 8-bit dim loop. Retail has two return tails
//   (`mov eax, edi` for the new texture, `mov eax, ebx` with ebx cleared
//   at the top for the Lock failure) and never uses that zero for the
//   null tests or `push 0`; VC6 here caches 0 in ebx for both. A
//   `return texture` in the success branch gives the two tails (304, 73
//   instruction diffs) but folds the failure return to `xor eax, eax`;
//   the texture's zero assigned at its declaration, after the format
//   read or just before the Lock test does not change that; an early
//   `return texture` or `return 0` on a Lock failure gives 294. The row
//   copy written without the row pointer (`memcpy(bits + y * width *
//   bytesPerPixel, ...)`) reproduces retail's copy loop instruction for
//   instruction (38 instruction diffs against 70) but scores 249/901;
//   the 8-bit dim loop as `for (x = line & 1; x < width; x += 2)
//   bits[line * width + x] = 0;` gives 68.
//   Session 14: `texture = 0` after the bytes-per-pixel read puts the
//   `xor` after that call as retail does (strict 300, same hunks), but the
//   zero still serves the null tests. Retail returns the new texture from
//   edi, which it zeroes before the new-result test and shares with the EH
//   state store; here ebx holds the texture and the null arm is
//   `xor ebx, ebx` after the branch.
// 0x00486b10 is exact in src/reconstructed/GUIManager.cpp: the pointer branch
//   is a POINT copy followed by `position.y += 16`. Every component-wise form
//   (ternaries, if/else, locals, helpers, inline setters and constructors,
//   256 combinations of eight value locals) had stayed at 89/106 or below.
// 0x00488120 (52 bytes, 46/52): retail keeps the `device ? owner : 0` branch
//   (jmp + xor) and tests the owner for zero before comparing it with
//   `this`. VC6 threads the xor arm into the next test whenever that test is
//   the zero test (inline helper, if/else assignment, early-return layouts,
//   `owner == 0` first, a result variable, a switch, an assignment in the
//   condition, an int-cast ternary, `owner && owner != this`, an `owned`
//   flag, nested if/else, goto, a `const` or `void*` owner, a `self` local,
//   `!device ? 0 : owner`, `this == owner`); comparing with `this` first
//   (below) keeps the branch, so only the order of the two tests differs.
//   Also threaded (15/52): an inline owner helper (early returns or a
//   ternary) with nested `if (owner) { if (owner == this) ... }`, `!= this`
//   or `return owner == this;`, a `(GUIUser*)0` or `device != 0` ternary,
//   a parenthesised ternary and an extra `hasOwner` local. Also threaded
//   (6-18/52): `owner = 0; if (device) owner = ...` with nested tests, a
//   `result` or `same` local, `return owner == this;` after a zero test, a
//   nested ternary and a `while (owner)` wrapper.
//   Session 14, all threaded (6-17/52): `if (owner) { if (owner != this)
//   return 0; return 1; }` before the store block, `if (!owner) {...} else
//   if (owner != this) return 0;`, a `result` local over if/else, and
//   `owner && owner != this` followed by a second `if (!owner)` store block
//   (the duplicated test that matches 0x004705d0 does not help here).
//   Session 21, all threaded (6-25/52): a `__forceinline` or early-return
//   static OwnerOf helper, the ternary repeated in both tests, the ternary
//   as the `if` condition with `device->ownerUser != this` inside, an
//   `int busy` flag, a `(int)owner` test, `owner != 0 ? owner != this : 0`,
//   a goto to a shared `return 1`, dead `previous`/`old` locals, a copied
//   owner local and `return owner == this ? 1 : 0` after the store block;
//   /G6 and /ML keep 46, /O1 gives 36. Retail evidence: the unthreaded
//   `jmp +2 ; xor r, r ; test r, r` occurs 31 times in .text; at the 13
//   other sites inspected the merge also holds a memory store (the value
//   stored to a member, as CollisionPoint's `inContact = cond ? 1 : 0`, or
//   an inlined `new`'s EH state), where this function merges into a bare
//   test, so no matched idiom carries over.
//   Session 24, threaded (13-18/52) or unchanged: inline OwnerOf helpers with
//   a by-reference or by-pointer out parameter, `(unsigned)owner > 0`,
//   `owner != 0 && owner != this` followed by `== this`, a bool HasOwner
//   helper, a `bool owned` local, `!owner || owner == this` nesting,
//   `device ? owner : (GUIUser*)device`, `owner = (GUIUser*)device; if
//   (device) ...`, `owner && owner == this`, `return owner == this` and the
//   store block first.

#include "../../src/reconstructed/GUIManager.cpp"

#include "GuiScreenTexture.cpp"

#include "GuiPointerHelpers.cpp"
