// Near misses for gameui.cpp (canonical file: src/reconstructed/GameUi.cpp,
// included below for its types and matched functions). Check: compile this
// file and compare each function with GameUiNearMisses.bindings.json.
//
// 0x00477e90 UIListBox slot 55 (320 matching positions, full retail and
//   candidate both 342 bytes): included from ListBoxButtonDown.cpp. The old
//   322-position form read point.x before point.y and faulted at address1
//   instead of retail's address5 for point=1. Point copies with no height
//   local keep the decoded y-then-x reads; see docs/LIST_BOX_BUTTON_DOWN.md.
// 0x0047b490 colour-key test (120/220): keeping the key pixel in the red
//   term's variable (`pixel |= green | blue`) keeps it in a register and
//   spills the row counter into the key's argument slot as retail does;
//   VC6 still ORs green|blue first where retail ORs red|green then blue
//   (no term order, parenthesisation or split changes the chain). The
//   strict 120 form tests `x < width` after the inner loop; the
//   structurally closest form (14 instruction diffs, 54 strict: the extent
//   shifts) keeps `found = 1; break;` in the inner loop and walks a row
//   pointer initialised in the for (`for (rows = .., row = bits; ..)`).
//   It still differs in the OR chain, in retail's edi save inside the row
//   loop (VC6 here copies the row pointer before the rows test, so edi is
//   saved in the prologue) and in the row step (retail `lea eax, [edx +
//   edx]; add edi, eax`, VC6 here `lea edi, [edi + edx*2]`; a char* step
//   and `&row[width]` give the same). Retail places `return 0` before
//   `return found` after Unlock; `if (!Unlock) return 0; return found;`
//   becomes neg/sbb/and, early returns for a failed Lock duplicate the
//   epilogue, and goto, result variable and while row loops do not give it.
// 0x004734c0 UIButton slot 28 (256 positions, retail 354/candidate 328)
//   and 0x00478e10 UIMultiState slot 28 (231 positions, retail 329/candidate
//   303; UIMultiState tests its own area in place, without UIButton's RECT
//   copy, which gives retail's 0x84 frame and head): retail keeps the 8-bit
//   result in edi and the 16-bit one in
//   memory, tests Unlock's result and returns separately when Lock fails.
//   Ternary, separate-result and goto forms keep the result in memory; an
//   early return on Lock failure, an empty `if (Unlock)` and an inline
//   unlock-and-return helper compile the same, and an Unlock and return in
//   each format branch (cross-jumped) puts the result in ebp (76/323). A
//   `result = opaque` copy before Unlock (returned once or from both arms of
//   the Unlock test, or as `Unlock ? opaque : opaque`) is propagated away.
//   Also 256 or worse (20-form sweep): an explicit `else opaque = 0` in the
//   8-bit test (246), `opaque = byte != key`, `else if` and `format == 8 &&`
//   chains, crossed with a `result` copy, an ignored Unlock result and an
//   `Unlock(0) != 0` test. Setting `result = 1` and jumping to the unlock in
//   the 8-bit branch (`goto unlock`; the 16-bit branch keeps `opaque = 1`
//   and falls into `result = opaque;`) gives retail's prologue and
//   `mov edi, 1; jmp` (266/354, 332 bytes; 240/303 at 307 bytes for
//   0x00478e10), but VC6 still sets edi for the 16-bit hit instead of
//   writing the memory result, drops retail's dead `test eax, eax` after
//   Unlock and folds the Lock-failure return into `xor eax, eax`: retail
//   never treats `opaque` as a known constant. Not kept (a goto for a
//   partial gain).
//   Session 14: a second `if (format != 8)` test instead of the else (256,
//   same code), the inverted test or a switch (245) and one ternary
//   comparison (249) do not give retail's two result homes, and neither
//   does an inline pixel-test helper taking `opaque` by reference or by
//   pointer (256, same code: the Lock-failure return still folds to 0).
//   A further 28 tail forms (result copies, every if-Unlock variant, an
//   else-if body) all compile to the same 256-position object.
//   Session 22 (measured on 0x00478e10, 231 before): what keeps the copy in
//   edi is a conversion between `opaque` and the returned value, which VC6
//   does not copy-propagate. `unsigned char result = opaque;` before
//   `if (Unlock(0)) return result; return result;` gives retail's whole
//   body: `mov edi, 1; jmp` for the 8-bit hit, the memory store and
//   `mov edi, [esp + 0x10]` for the 16-bit one and the dead `test eax, eax`
//   (288/329, 7 instruction diffs), leaving an `and eax, 0xff` at the return
//   and the Lock-failure return folded into `xor eax, eax`. `short opaque`
//   with `int result = opaque; if (Unlock(0)) return opaque; return result;`
//   gives the same body with `movsx eax, di` (284). No 4-byte pair blocks
//   the propagation: unsigned, long, unsigned long, an enum, void*/char*
//   with casts, `(short)opaque` and `bool result = opaque != 0` stay at
//   231-235. Not kept: the type is unproven and the match is partial.
//   An inline pixel test taking `int& opaque` that returns 1 for the 8-bit
//   hit and sets `opaque` for the 16-bit one gives `mov edi, 1; jmp` too
//   (241); with `if (!Unlock(0)) return result;` falling into the outer
//   `return opaque` it keeps the memory store, the test and the separate
//   Lock-failure return (286/331), but branches on the test and returns the
//   memory copy when Unlock fails, which retail does not (edi). By value
//   the helper loses the 0x84 frame (54-59). Early returns for PtInRect or
//   Lock (72-96) and a single `return opaque` exit (241-250) are worse.
//   Session 24 (0x00478e10): VC6 scalarises `opaque` whether it is an
//   `int[1]`, a member of a one- or two-field local struct, written through
//   an `int*` or bound to an `int&` (all 231, same object as the plain int,
//   with every tail form). Only an aggregate that also holds the surface
//   description keeps it in memory: `struct { int opaque; DDSurfaceDesc2
//   desc; }` gives retail's unfolded `mov eax, [opaque]` for the Lock failure
//   and a real `result` copy in edi, but puts the pair after `format` in the
//   frame, stores 1 for the 8-bit hit too (no `mov edi, 1`), drops the dead
//   test and orders the two failure returns the other way (200-220/329);
//   with `format` in the aggregate as well 134. `volatile` gives 190. The
//   `unsigned char result` form scores 288 unchanged with PtInRect as an
//   early return, as an if/else, or with the Lock-failure return first.
// 0x004738a0 UIEditBox slot 40 (76/968): retail keeps the width and the
//   redraw count in memory. Retail reloads width into edi at the loop top
//   (0x47396d) and consumes that copy in the scroll arms (`sub edi, ebx`),
//   reading [esp+0x14] for the caret limit; the Blit and no-texture arms
//   share one `redrawFrames = redraw` store. With the caret store fixed (see
//   the FIXME below), one `redrawFrames = redraw` after the texture block
//   gives aligned 215 (from 199) and 968 bytes (strict 76, from 206); kept
//   since the linked masked score rises 0.830 -> 0.873. Swept with it
//   (work/tpl EditBoxDraw): width/frames/redraw and dc/more/rect declaration
//   orders, an inverted `if (!dialogBackground)` arm order and per-field
//   caret stores; none beat aligned 215.
// 0x00474150 UIEditBox slot 20 (1018/1044): Backspace reads the length and
//   the cursor into locals after the lead-byte test (retail loads both before
//   the branch and keeps them in ecx/eax across it). Retail gives the cursor
//   eax (loaded after `test eax, eax`), VC6 here edx, and the register
//   choice ripples into the Enter case (`eventCode` in eax). BOOL/char/
//   unsigned locals, function-scope locals, other declaration orders (worse)
//   and the locals before the call (worse) were tried. Also worse
//   (2026-10-11): only one of the two locals (1004 with `length`, 351 with
//   `cursor`), neither (339), the call as the if condition, and the two
//   decrements in the other order (1002). Also <= 1008 (about 60 forms): a
//   `char* text` local, `cursor + controlText`, a `count` local,
//   `&controlText[cursor]` with `length + 1 - cursor`, length-first stores,
//   an Enter-case reorder and all 6 declaration orders x 3 lead-test forms
//   x 3 decrement forms.
//   Session 14: one memmove per arm followed by a second `if (lead)` for
//   the decrements, with only the `length` local, loads both fields before
//   the branch as retail does (strict 1024) but gives the text pointer ebx
//   (9 differing instructions against 8); with both locals it saves ebp
//   (236), one shared memmove or a `step` local drops a call (75-98), and a
//   `text` or `end` pointer local is worse (99-338).
//   Inline helpers (the lever that fixed KrustyUI::Create) are inert here:
//   a memmove wrapper taking (to, text, cursor, length), an inline
//   `TextAt(text, cursor)`, a length-shrinking or two-value setter after the
//   memmove, and a whole remove helper given the loaded locals all give
//   1018; the helper that loads both fields itself, or takes them as direct
//   member arguments, gives 339.
// 0x00478570 UIMultiState slot 40: complete retail 672 / candidate 682, 139
//   resolved matching positions; included from MultiStateDraw.cpp. See
//   docs/MULTI_STATE_DRAW.md for the finite actual-helper validation.
// 0x0047a400 drop-down layout (887/962; the body is in GameUi.cpp, included
//   above, since the Group2_UIDialogResource bring-up retired): the open area is written before
//   the scroll rect's bottom, both from `list.bottom - 2`. That gives
//   retail's left, top, right store order, the late scroll-bottom stores and
//   the scratch registers of every later sprintf/SetName block (28
//   differing instructions, all before the first GetName). Left: retail
//   forms the value once (`mov eax, ebx ; sub eax, 2`) and adds it into
//   top; VC6 here forms it twice with lea, and swaps ebx/edi (zero and the
//   list top). With `scroll.bottom -= 2` ahead of the area and `top +
//   scroll.bottom` (any spelling, operand order, `top +=`, an inline setter,
//   `top + (scroll.bottom -= 2)`, 400 random statement orders) ebx/edi and
//   the `sub eax, 2` match but VC6 stores scroll.bottom before the add,
//   sums into eax and rotates every later scratch register (329, 43
//   instructions). `top + (scroll.bottom - 2)` then `scroll.bottom -= 2`
//   folds the sum into one lea (725); a `bottom` local read from
//   scroll.bottom grows the frame to 0x78 (9 instructions); the area bottom
//   from the list with `scroll.bottom -= 2` kept first gives 702. 0..8 dead
//   initialised locals and a 0..47 typedef prefix change neither family.
// 0x0046a920 UIDialog's resource parser (16 KB, in GameUi.cpp with the
//   Set_Default FontName fix, 1615/16130; 92% of instructions equal
//   with stack offsets and relocations ignored, 74% raw): the control flow,
//   calls and constants follow retail. Retail keeps the current control in
//   ebx across the section loop and the pass counter in memory; VC6 here
//   gives ebx to the pass counter (and to the state/item counters), so the
//   control is spilled. Retail's frame is 0x80 bytes larger: VC6 lays locals
//   out by uses per byte, ties permuted by a quicksort over the last-use
//   order (docs/VC6_FRAME_LAYOUT.md; `python3 tools/frame_layout.py OBJ
//   ReadDialogResource --va 0x46a920` maps the slots). The large arrays,
//   `parameters` and the most used key buffers have retail's offsets;
//   `toolTip` has four references here and two in retail, `defaultFontName`
//   two here and three in retail, the two-use key buffers leave the control
//   loop in another last-use order, three retail scalar slots are spill
//   homes the candidate keeps in registers, and retail has one 0x80-byte
//   local at frame offset 0x2174 (between `toolTip` and `fxSoundOut`) that no
//   instruction references and no source form reproduces (locals used only
//   in dead code get no slot). The jump table makes ckm report an unresolved
//   $L label; compare with sdiff.
//   Session 14 probes (0x80-byte buffers): a buffer used only under a local
//   flag that is always 0, a `const int` flag, a loop that never runs, an
//   inline expansion whose condition is a constant 0 argument, a dead
//   pointer to it or `sizeof` gets no slot. A condition VC6 cannot fold
//   (`n > 3 && n < 2`) keeps both the slot and the code.

#include "../../src/reconstructed/GameUi.cpp"

#include <imm.h>

#include "../../src/reconstructed/PCVideoCard.h"
#include "../../src/reconstructed/KeyboardDevice.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/Palette8.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x00477e90
#include "ListBoxButtonDown.cpp"

// ---------------------------------------------------------------------------
// Drawing, input and loading near misses

// cdecl 0x0047b490: whether `texture` has a pixel of colour `key`.
#include "TextureHasColor.cpp"


// UIDialog::Tick (0x0046ef00) is strict exact; it now comes in through
// GameUi.cpp, which includes src/reconstructed/UIDialogTick.cpp.

#include "ControlHitTest.cpp"

// 0x004738a0: draws the image, the text scrolled to keep the caret in view,
// and the caret.
#include "EditBoxDraw.cpp"

// 0x00474150: typed characters: Backspace, Enter (kind 10) and inserted
// characters, double-byte ones in two steps; then measures the text and
// sends kind 0x13.
#include "EditBoxOnChar.cpp"

// 0x00478570: draws the state's image, then its text.
#include "MultiStateDraw.cpp"
