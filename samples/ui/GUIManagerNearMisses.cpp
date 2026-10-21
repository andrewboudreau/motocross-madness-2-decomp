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
// 0x00486b10 (106 bytes, 89/106): one ternary per component (below; two
//   separate if/else statements compile the same) gives retail's block
//   order and `pop edi` placement (one if/else block: 86/106). Retail still
//   stores the centred x before loading the control's bottom into eax and
//   passes the text in eax and &position in edx; VC6 here hoists the bottom
//   load into edx beside the cdq and swaps the call registers. Tried: y
//   before x (81), `!device` ternaries/ifs, a POINT, a pointer to the array
//   or `*p++` stores, `&position[0]`, x/y/bottom/text locals, casts or
//   parentheses on the text, a centre helper, a ShowText call in each
//   branch, and the centre written as `width / 2 + left`, a `width` or
//   `left` local, or `left` then `+=`; parentheses around either whole
//   ternary, around its arms or around the centre leave it unchanged.
//   Swept (tools/sweep_variants.py): a `CameraRect& area` alias on every
//   subset of the four area reads x a text local x x/y order; best 89.
//   Also <= 89 (48 forms): a `GUIUser* user` local, x and y as if/else
//   assignments of locals (with a `half` local), a text local, a `tip = this`
//   alias and an `int* p = position` local.
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

#include "../../src/reconstructed/GUIManager.cpp"

#include "GuiScreenTexture.cpp"

#include "GuiPointerHelpers.cpp"
