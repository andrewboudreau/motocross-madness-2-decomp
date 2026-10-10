# VC6 SP3 stack-frame layout

How `CL.EXE` 12.00.8804 (`/O2 /GR /GX /MT`, the calibrated profile) assigns
frame slots to a function's locals. Many near misses in this project differ
from retail only in those slots; the rule below says what the original source
must have had more or fewer references to. `tools/frame_layout.py` applies it
(usage at the end). Evidence tiers follow AGENTS.md: everything under
"Facts" was measured on probe objects compiled with the project's VC6; the
compiler's internal data structure is inferred from the measurements, not
read from the compiler.

## Facts

Measured with `char` buffers and scalars whose addresses are passed to an
external function (`use(a)`), `/Z7` CodeView `S_BPREL32` records read back
with `tools/frame_layout.py OBJ --dump`. "Nearest esp" is the lowest address
(`bprel` most negative). Probe sources are listed under "Reproduction".

1. **Declaration order and names play no part.** Ten buffers declared in
   either order, or named `a..z`, `aa..az`, `x`, `xx`, `xxx`.. all lay out
   the same way (p1, p2).
2. **Primary key: static use count, per byte.** With equal sizes the most
   referenced buffer is nearest esp (w1, w2). Across sizes the key is
   uses/size: `char a[0x80]` needs 33 references to outrank an `int` used
   once (32 ties, and ties go to the smaller object), 9 to outrank
   `char[0x10]` used once, 3 to outrank `char[0x40]` used once; `char[0x60]`
   used 7 times outranks `char[0x40]` used 4 times, 6 times does not;
   `char[0x100]` needs 9 references against `char[0x80]` used 4 times,
   `char[0x180]` 13 (q10, q9). Ties in density go to the smaller object when
   only the two are involved, but see fact 6: the tie rule is the quicksort's.
3. **What counts as a use** (every item one probe set, q1/q2/q5/q6):
   - each reference instruction: address-of (`lea`), element reads and
     writes (`a[3] = 1; a[5] = 2; a[7] = 3; use(a)` is 4), scalar reads and
     writes of address-taken ints (r3: `a` with `&a` plus three value reads
     is 4);
   - a `strcpy(buf, "literal")` intrinsic expansion is one use (k4: one
     strcpy plus four `use()` is nearest esp);
   - uses inside inlined bodies count where they land after inlining (e1:
     `inline two(p) { use(p); use(p); }` gives p two uses);
   - loop bodies and conditional branches count once per reference, with no
     execution-frequency weighting (b1-b7, c1, c2: a buffer used three times
     in a loop ranks as 3, once in a nested loop as 1);
   - unreachable code counts nothing (`if (0)`, after `return`: d1-d3);
   - a buffer with only dead stores (`strcpy(d, "")`, `memset`, `d[0] = 0`)
     or only reads whose results are unused is removed before layout: it
     gets no slot at all (x4-x6, z1, z2, z5-z7); `if (!d)` on an array is
     not folded, so its branch stays live (z3).
4. **Scope sharing.** Locals of sibling blocks share one slot (s1-s7; the
   CodeView records carry the same offset). The shared region ranks with the
   *sum* of its members' counts (s3: `{a used 2}{b used 2}` beats `c` used 3)
   and the largest member's size. A nested block's locals do not share with
   the enclosing block's (s6). The other frame-layout agent found the same
   from the register-allocation side: an address-taken local is live for its
   whole scope, so only a block boundary lets two of them share; scalars that
   are never address-taken share by dataflow liveness instead, and register
   spills of compiler temporaries merge into the homes of non-address-taken
   variables but never into one holding an address-taken variable (EcoSystem
   `0x45c040` became exact by reading an index inside a bare block).
5. **Small frames.** When the locals total at most 0x80 bytes, size decides
   first (ascending, nearest esp = smallest) and the count only orders equal
   sizes (q13: `char a[0x44]` used ten times sits above `char b[0x3c]` used
   once at a 0x80-byte frame, below it at 0x84; e14: `int y`, `char b[0x20]`,
   `char a[0x40]` used ten times lay out y, b, a). Every function this
   project cares about has a larger frame; the rule is recorded for probe
   writers.
6. **Tie order is the permutation of a quicksort.** Equally keyed symbols do
   not keep source order. Ten buffers used twice each in the order
   1..10 come out, nearest esp first, as 5 6 2 7 4 8 3 9 1 10; for n = 2..16
   the pattern is "take the middle (`(lo+hi)/2`) of the remaining list, move
   the first remaining element into its place, repeat" (p5, g1 for n = 26).
   With mixed keys the permutation depends on the whole list, not only on
   the tied run: two equally used maxima among five symbols come out in use
   order, among four or three in reverse (m2). All 364 probe layouts with
   equal sizes and 40 with mixed sizes are reproduced by:
   1. list the frame symbols in order of their *last* reference (the
      a/o/r probes separate first-use, last-use and reverse orders);
   2. stable-sort by (density descending, size ascending, last use);
   3. merge sibling-scope members into one entry at the first member's
      position (sum of counts, maximum size);
   4. run this quicksort, whose comparator never answers "equal":
      ```
      sort(lo, hi): mid = (lo+hi)/2; swap(a[mid], a[lo]); pivot = a[lo]
          loguy = lo; higuy = hi+1
          loop: do loguy++ while loguy <= hi && !better(a[loguy], pivot)   // equal stops
                do higuy-- while higuy > lo && !worse(a[higuy], pivot)    // equal continues
                if higuy < loguy break; swap(a[loguy], a[higuy])
          swap(a[lo], a[higuy]); sort(lo, higuy-1); sort(loguy, hi)
      ```
      `better(x, y)` is `uses(x) * size(y) > uses(y) * size(x)`; `worse` is
      the mirror image, so two equal symbols are "not better" both ways.
      The nearest-esp slot goes to the first element of the result, slots
      are packed upwards with no alignment padding (a `char[0x7c]` is
      followed at +0x7c).
   This is the structure of the CRT `qsort` with its pivot at `(lo+hi)/2`
   instead of `lo + size/2` and no small-partition cutoff; which routine the
   backend really calls is not confirmed.
7. **Frames with exception handling** (locals with destructors, `/GX`)
   follow the same order; the EH registration record sits at the top
   (`[ebp-0xc]` in ebp frames, above the locals) and an object with a
   destructor ranks as if it had at least three references more than its
   explicit uses (constructor, destructor and unwind registration; t3-t5:
   an object used once sits nearer esp than a buffer used three times). Compiler temporaries (`$T`, a struct returned
   by value) rank like locals: the hidden return pointer and each use count
   (t1, t2).
8. **Register candidates.** Scalars that live in registers have no slot;
   when spilled, the home slot ranks like any other symbol (r1: four spilled
   ints, most referenced nearest esp). The count the compiler uses for a
   spilled scalar is not the number of instructions that touch its slot,
   which is why `tools/frame_layout.py` reports its reference count as a
   proxy and prints the rule's prediction only as a self-check.

9. **Homes in dead argument slots.** A value that needs a memory home but
   is not a frame local (an x87 `float` local, a spilled register
   candidate, a `short` whose address is taken) goes into the slot of an
   argument before the frame grows. The rule, measured on 60 float probes
   (p/q/r/w/z/y/u/k series under "Reproduction") and confirmed on three
   retail functions:
   - **Symbol order**: the frame rule's key, count per byte descending, ties
     by last reference ascending (w1-w8: of two 3-use floats the one whose
     last use comes first takes the first slot; q2/z7: more uses win
     whatever the order; the count is the compiler's, so a compare that
     consumes a value kept on the x87 stack counts although it loads
     nothing).
   - **Slot order**: the slots are handed out in the order of the
     arguments' *first use in the source* (the IR order), not of the
     prologue loads or the emitted instructions. Cube `0x0043d230` stores
     three arguments into members; permuting those three statements
     permutes the homes of the function's `flags`, face pointer and face
     counter one-to-one while the emitted code (scheduled the other way
     round) stays identical. For arguments read in place (floats) the
     first use is the read (p1: `x = a * b; y = c * d` homes x in a, y in
     b; p2 with the statements swapped homes them in c and d, and a and b
     stay unused although they die before y's definition).
   - **Availability**: a slot is free for a symbol when the argument's last
     read and the previous occupants' last references precede the
     symbol's first definition (first fit; r1-r12: four floats defined in
     two waves share two slots in every combination the counts allow;
     q3/q4: an argument read at the end is skipped).
   - **Sub-int parameters** are copied at entry, so their slot is free from
     the first use of the first argument on: GUIManager::SetUp
     `0x004853b0` homes its ControlInterface pointer in the `short
     fontSize` slot, the second slot handed out, although the font size is
     stored halfway through the function; with `int fontSize` the pointer
     takes `target`'s slot.
   - **Commutative expressions hide the source order**: `center->y *
     camera->matrixB[1][0]` and the swapped spelling give the same IR
     order in VisibilityClipper::SphereInFrustum `0x0052fbb0` (center
     first, which homes `depth` in the center slot); a pointer to the
     camera's side-plane column taken before that product (`side`, used
     for `spread`) makes the camera the first-used argument and the
     function exact. Which operand VC6 orders first inside one expression
     is the operand-order question of docs/VC6_OPERAND_ORDER.md.
   - After the arguments the compiler's own temporaries (the `new`
     results of SetUp) share by the same first fit; a slot an occupant
     holds during the symbol's life is skipped.

## Remaining uncertainty

- The order within step 1 is the order of the symbols' last references in
  the compiler's intermediate code; for straight-line probes that is the
  instruction order, and the tool uses the last instruction index. Block
  placement that VC6 moves (loops rotated, cold blocks after the epilogue)
  can make the instruction order differ from it.
- For a `struct` local whose inlined methods read many fields through
  `this`, the measured instruction count (124 for the resource parser's
  `UnknownParameterBlock parameters`) is far above what its slot implies
  (between 7 and 16 uses of a 0x80 buffer at 0x5c4 bytes); the compiler's
  count for such members is not pinned down.
- Small-frame ordering (fact 5) and the exact tie rule among more than two
  equal-density symbols of different sizes are measured on few cases.
- Whether the density is compared exactly or with an integer scale: all
  thresholds measured so far (q9, q10, u-series) match the exact ratio.
- For argument slots, the "first use" of an argument inside a commutative
  expression follows VC6's canonical operand order, which the probes did not
  pin down: `p->f[0] * q->f[1] * r->f[2]` hands out q, r, p in the u-series
  but r, q, p when p is used nowhere else (k1); an earlier separate use of
  the argument is the reliable lever.

## Reproduction

The probe objects were compiled with the project's compiler and the
calibrated profile:

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" probe.cpp -o probe.obj
python3 tools/frame_layout.py probe.obj --dump
```

Probe shapes (one function each, `extern void use(const char*);`):

```cpp
// w1/w2: counts.  nearest esp: t (5 uses) ... p (1 use); reversed for w2
void w1() { char p[0x80]; char q[0x80]; char r[0x80]; char s[0x80]; char t[0x80];
  use(p); use(q); use(q); use(r); use(r); use(r); use(s); use(s); use(s); use(s);
  use(t); use(t); use(t); use(t); use(t); }
// p5/n10: ten equal buffers used in order -> nearest esp first 5 6 2 7 4 8 3 9 1 10
// u1/u3: char a[0x80] x4 vs char b[0x40] x2 -> b first; x5 -> a first
// u30/u31: int y x1 vs char a[0x80] x32 -> y first; x35 -> a first
// s3: char c[0x80] x3; { char a[0x80] x2 } { char b[0x80] x2 } -> a/b (shared) first
// q13 f4/f5: char a[0x44] x10, char b[0x3c] x1 -> b first; b[0x40] -> a first
// m2: 1 1 1 2 2 -> d e a b c ; 1 1 2 2 -> d c a b ; 1 2 2 -> c b a
// argument slots (stdcall floats, frameless; homes read from the S_BPREL32
// offsets, +8 = first argument):
// p1: float x = a*b (4 uses), y = c*d (3) -> x in a, y in b; p2 (y first) -> x in c, y in d
// q3: `return x + a` keeps a live -> x in b, y in c; q4 (b live) -> x in a, y in c
// w1..w8: 3 uses each, every def/test/last-use order -> the earlier last use takes a
// r1..r12: x, y early, u, v late (u = x*3, v = y*5), counts varied -> first fit
//   (r3: x4 y3 u3 v4 -> {x,v} a, y b, u c; r2: x3 y4 u4 v3 -> {y,v} a, {x,u} b)
// u1/u5/u13, k1/k4: S* p, q, r pointer arguments, order of the handed-out slots
```

`tests/test_frame_layout.py` holds the measured permutations and thresholds
and checks the tool's model against them.

## tools/frame_layout.py

```bash
python3 tools/frame_layout.py OBJ SYMBOL --va 0xVA [--size N] [--exe "$MCM2_EXE"] [--cv-name NAME] [--json]
```

`OBJ` is the candidate object (VC6, `/Z7`), `SYMBOL` a substring of its COFF
symbol, `--va` the retail function. The report shows:

- the candidate's locals (CodeView offsets and `.debug$T` sizes, converted
  to offsets from the frame base = esp after the prologue, nearest esp = 0),
  with scope-sharing members at equal offsets;
- the frame accesses of both functions, `[esp+N]` with the push depth
  tracked (callee cleanup from the callees' mangled names in the candidate's
  relocations, carried over to the aligned retail calls; `add esp` look-ahead
  otherwise) and `[ebp-N]` in ebp frames;
- the candidate-to-retail slot map, voted from instruction-aligned accesses
  (a sync walk that keeps repeated `strcpy` expansions in step), the retail
  slot order and retail slots that no candidate local maps to (a retail
  local the candidate lacks, or one whose references the candidate lost);
- each local's reference count in the candidate, the layout the rule
  predicts from those counts (if it differs from the candidate's actual
  layout the proxy count is off for the flagged symbols), and the +k/-k
  count changes under which the rule reproduces the retail order (a greedy
  search; "no adjustment found" means the orders differ in more than counts,
  usually in the scalars' spill homes);
- the argument slots each side writes (`values homed in argument slots`):
  the argument index, the instruction after which the argument is dead and
  the runs of accesses that start at each write. Two functions whose runs
  sit in different argument slots differ in the order the source first uses
  those arguments (fact 9), or in the rank of the homed values.

`--dump` lists the locals of every procedure in an object (probe objects).
The parsing and prediction code is importable (`parse_cv_procs`,
`parse_cv_types`, `track_frame`, `align`, `predict_layout`,
`suggest_adjustments`, `predict_arg_homes` for fact 9 with the slots given
in first-use order, `argument_slot_homes`) and covered by
`tests/test_frame_layout.py`.

## Applied to gameui's resource parser `0x0046a920`

Candidate `samples/ui/GameUiNearMisses.cpp` (frame 0x9bf4 against retail's
0x9c74). The tool maps every 0x80-byte key buffer and every array to a retail
slot; three retail scalar slots (`0x3c`, `0x4c`, `0x5c`) have no candidate
local (spill homes of values the candidate keeps in registers) and one
0x80-byte retail slot at `0x2174` has no reference at all. Differences the
rule attributes to the source:

- `toolTip` has four references here and sits in the four-use group; retail
  keeps it among the two-use buffers (`0x20f4`), directly below the
  unreferenced slot.
- `defaultFontName` has two references here (its `strcpy(.., "")` and the
  `"FontName"` default) and three in retail (`0x18f4`, among the three-use
  buffers).
- the two-use group's order (`soundNorm soundFocus soundPush soundClick
  soundClick#2 fontName#2 mouseAnim fxAnimOut toolTip [dead] fxSoundOut
  fxSoundIn anchor soundFocus#2 soundNorm#2 fxAnimIn soundPush#2` in retail)
  differs from the candidate's: the last-use order of these per-control key
  buffers inside the control loop is not the candidate's.
- the scalars at the frame's base (`0x0..0x178`) are spill homes whose
  counts the disassembly does not reveal; the rule cannot be applied to them
  from the object alone.

No source form was found that reproduces the dead slot (a local used only in
dead code gets no slot, fact 3), so the parser stays a near miss; the sample
header records these findings.

## Applied to the scan of every near miss

`docs/NEAR_MISS_INDEX.md` classifies every near miss in the repository by
whether it differs from retail only in frame slots. Two of them became exact:

- krustyui.cpp `0x0049a8b0`: the manufacturer index and the bike count (both
  4 bytes, both three references) were swapped. Declaring the entry `count`
  inside each file's `if` block makes the two counts share one slot (fact 4)
  and that changes the tie order of the whole list, which moves the index
  below the bike count. A function-scope `count` or shared loop indices do
  not.
- Vehicle.cpp `0x00525e20`: not a layout change. The read at `[esp+0x48]`
  that an uninitialised local reproduced is the second parameter's slot (the
  engine name), which VC6 had homed elsewhere.

The three slot-only cases were homes in dead argument slots (fact 9) and are
exact:

- Cube `0x0043d230`: the member stores of `group`, `stream` and `baseOffset`
  come in that order; the earlier candidate stored `baseOffset` before
  `stream`, which handed the slots out in the other order (all six
  permutations give six different layouts and the same code).
- GUIManager::SetUp `0x004853b0`: `fontSize` is a `short`; its slot is free
  from entry and is the second one handed out.
- VisibilityClipper::SphereInFrustum `0x0052fbb0`: `const float* side =
  &camera->matrixC[0][0]`, taken before the depth product and used for the
  side-plane spread, makes the camera the first-used argument.

- Locals of sibling inline expansions share frame slots, like sibling blocks. Writing
  a rigid inverse, two matrix products and an axis rebuild as inline helpers gives
  Wrecker slot 10 `0x5306e0` retail's 0x128 frame; written out in the body the
  frame is 0x134.
- An inlined helper taking a struct by value copies the argument only when it can
  change under the call (it aliases the output); an unmodified local is read in place.
  This reproduces retail's two `rep movsd` copies in Wrecker slot 10.
- A struct assigned from an inline-constructed value (`sum = Vec3(0, 0, 0)`, `dst = sum * s`
  with an operator that returns `Vec3(...)`) goes through a 12-byte temporary that copies
  field by field; `Vec3 sum(0, 0, 0)` or per-component stores build in place and leave that
  slot out. The ModelVsHull family (`0x437c20`, `0x437ef0`, `0x438280`, `0x4376f0`) gets
  retail's frame from both temporaries sharing one slot; its inverse is written into the
  product's own local (`MatrixMultiply(&rel, rel, b)`), one 64-byte slot.
- An inline helper that returns the base type (`Vec3`) from an out-of-line constructor of a
  derived call type (`return BikeOolVec3(...)`) constructs into a temporary and copies it into
  the helper's return slot; returning the derived type binds the constructor result directly.
  Retail's chain of such helpers in Bike slot 46 (`0x40b600`) shows the copies all landing in
  one slot at frame 0.
