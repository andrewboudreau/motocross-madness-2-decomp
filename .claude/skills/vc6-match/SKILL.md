---
name: vc6-match
description: Byte-match one MCM2 function against retail with authentic VC6 SP3 /O2. Use when working a near miss from docs/REBUILD_GAPS.md, re-checking a function after a rename or header change, or searching source shapes (x87/fistp-heavy code especially).
---

# VC6 byte-match loop

Goal: readable C++ whose VC6 SP3 `/O2` output equals the retail bytes with relocations resolved.
Rules from AGENTS.md apply: no inline asm, no byte arrays, no extent trimming, inference stays marked as inference.

## Setup (once per shell; Windows-style paths)
```bash
export VC6_ROOT="$(pwd -W)/work/vc6sp3-disc" MCM2_EXE="$(pwd -W)/work/game/mcm2.exe"
export PYTHONPATH=.
```

## 1. Score one function
```bash
python tools/match_one.py SRC VA SIZE [SYMBOL_SUBSTR] [-q]
```
Prints strict bytes matched, instruction alignment, an instruction diff and the exact differing byte offsets
(candidate vs retail). A 2-byte diff is usually one operand or load-order choice, not a shape problem.
Bindings default to `SRC` with `.bindings.json`. `tools/match.py` is the lower-level compare on an existing .obj.

## 2. Diagnose the kind of difference
- Stack displacements only differ: `python tools/frame_layout.py OBJ SYMBOL --va 0xVA --size N --exe "$MCM2_EXE"`.
  VC6 orders locals by (use count desc, size asc, last use asc) then a quicksort; add/remove a use or a dead local.
- Register/load order differs: change temporaries, helper boundaries, statement order (see lessons).
- Different instruction count: inlining or tail-merge differences.

## 3. Search source forms
`tools/sweep_variants.py TEMPLATE --target VA:SIZE=SYMBOL --bindings B --stop-on-exact` renders every combination of the
template's `//@perm`, `//@alt`, `//@opt` and inline `{@a@|b@}` choice points (see the tool's docstring), compiles
them in parallel and ranks by strict bytes. `tools/rank_near_misses.py` ranks the repo's
near misses; `tools/near_miss_drops.py` lists ones whose gap is a single drop.

## Codegen lessons (VC6 SP3 /O2)
- Commutative operand order and plain `a*b` vs `b*a` are canonicalised: do not spend time on it.
- What moves codegen: temporaries, helper boundaries, statement order, compound assignment (`r.x = a*b; r.x -= c*d;`),
  if/else vs `?:`, an early `return` (keeps `push esi`), chained `float w = field = expr;`.
- Tail merging: VC6 cross-jumps identical tails; write switch arms out in full. A cached local can break a merge
  (Terrain slot 14 needs the field cast at every use). A tail-recursive call mirroring a matching sibling reproduces loop removal.
- A struct copy of a record is far worse than field stores. Early stores of flags/width/height are worse.
- `static const T k = T(a,b,c);` gives the stack-temporary-then-copy `$E` body; `k(a,b,c)` stores straight to the global.
- `memset(p,0,N)` inlines as xor + stores; the inline budget is breadth-first over call sites and grows with caller size;
  `#pragma inline_depth(0)` turns inline constructors into calls.
- fistp/`/QIfist`: float-to-int through an `int*`-parameter inline helper; try `extra_flags: ["/QIfist"]` via a
  `candidate_sources` entry in `config/game_build.json` before distorting the source.
- Declaring one extra member in a class can swap two load registers in an unrelated function (Terrain issue #32):
  it is a compiler internal-ordering artifact, so probe it as a header change, not as a source-shape change.
  The count is per function, modulo 64 typedefs: each named declaration or member is 1, a class 8 + members,
  a redeclaration 0 (NAMING.md Wave 18). Removing a view struct is therefore a different lottery in every unit
  that includes it; check exact neighbours (Terrain Render3D, BikeRace CreateRacers) before calling a merge neutral.
- Index sums in strength-reduced loops: `for (j = 0; j < 64; j += 16)` lets VC6 reassociate `[i + j + t]` to
  `t + i + j`; a row counter with a named `int j = row * 16;` keeps `lea r, [i + j]` (Griddraw `0x00481300`).
  Term order and parentheses do not move it; `row << 4` is worse.
  A k-scan (prefix k = 0..63 typedefs, work/w18/probe.py) is flat for all 65 render/eco/track near misses, so most
  order-only rows are not this effect; BikeRace CreateRacers (k = 6..8) is the positive control.
- x87 operand order of a commutative product follows leaf age: copying a member into a named float local just before
  the product (`float minX = data->gridMinX; ... cell * minX`) makes it the newer leaf, loaded first, and the copy
  folds (Griddraw ReadNode `0x0047e600`). If the local stays live it costs a spill instead.
- Block layout follows the flow graph, not the spelling: if/else order, switch, goto and label placement canonicalise.
  A redundant early return/re-test that jump threading later removes changes the graph (AddChatLine `0x004f3720`:
  `if (player != -1 && !GetPlayerName(player, name)) return;` before the system/player split).
- A width conversion between locals (`unsigned char r = intVar`) blocks copy propagation and keeps a callee-saved copy
  plus the stack home alive (UIMultiState::ContainsPoint `0x00478e10`, 231 -> 288/329). Zero reuse from `test/jne`
  only happens on the fall-through edge; at a jump target VC6 emits the immediate.
- Scratch-register rotation: VC6 hands expression temporaries eax, ecx, edx in turn; a named value (a local, an inline
  function parameter, or an inline accessor result like `FileOf(stream)`) takes no turn, so a permutation that starts at
  one call and persists usually means retail had (or lacked) a named value there. A local declared before an argument
  list shifts the whole list; a pointer local held across a call goes to esi; a read-only local is propagated away, a
  reassigned one survives (FileStream Write/CheckHeader, ShowControlToolTip, LeaveRaceForMenu, HandleReceivedMessage).
  The rotation state after an if/else is that of the arm written last. If everything after a branch is one step off and
  the arms themselves match, give the earlier arm its own copy of the function's tail and a `return`, with the other
  arm's statements after the `if`: VC6 tail-merges the copy (same bytes) but the arm order in the IL changes
  (ChatOverlay::DrawNameTag `0x0051e3f0`, 966 -> exact). A value loaded only to be tested takes no turn.
  The state also depends on statement order inside an inline expanded in the earlier arm: VC6 may schedule two
  member stores the same way for both orders, so that arm matches either way and only the next arm shows it
  (BikeRace::AimCamera `0x0041eb20`: `shadowCaster = caster;` before `isSettled = 0;`, 1699 -> exact).
  Probe by adding a throwaway `a = b;` copy at block ends to find which block the wrong arm inherits from.
- A `c ? &a : &b` call argument is evaluated whole, ahead of the pushes. Retail testing first, pushing the shared
  arguments and choosing with `lea/jne/lea` in place means the call was written once per if/else arm; VC6 merges the
  shared pushes on both sides (Vehicle::ApplyBodyContactImpulse `0x005265d0`, 405 -> exact; arm order matters).
- A chained read that runs one register behind (`g->a->flags` in a condition, with later argument lists rotated too)
  can be a pointer local declared inside the block (`ProCircuit* circuit = g->proCircuit;`, CreatePodiumScene
  `0x0045d480`). Folded trivial accessors and typing do not move a float-slot `push ecx` that retail delays.
- Struct copy then `+=` (`pos = dev->pointerPosition; pos.y += 16;`) beats component-wise stores (ShowControlToolTip).
- PoseRotation: the function-local definition count has period 8 there; used named constants count like dead locals.
- An inline accessor in an argument list is evaluated ahead of the other arguments; reading the member in place restores
  right-to-left interleaving and can flip a loop value's callee-saved register (CollectRacers `0x0051d730`).
- Two statements sharing a subexpression: the one written first decides which register accumulates; `int h = f(); a = top + h`
  and `a = top + f()` accumulate in different operands (UIDropDownList `0x0047a400`, UIListBox `0x00477e90`).
- The initialised-local count is a weak lever: flat on most rows; when it varies, usually period 8, and it never moves
  scheduling (push order, `sub esp`, store placement). A named by-value copy into an inline helper is a stronger leaf-age
  lever (HullVsHull `0x00436720`).
- An early clear ages an int local: `count = 0;` at the top of the function, with the local declared at function
  scope, makes its leaf older than values computed later, which flips the operand order of an integer add or
  `[base + index]` it takes part in (Scene::ReadStaticModels `0x004ecd60`, 4534 -> exact) and the order of two
  reloads before a call (`remap = 0;`, GatherModelTriangles `0x0042c260`, 1622 -> 1626). A declaration alone, or
  the initialiser at first use, does not. Sweep one `x = 0;` per local first; pairs can cancel.
- Two strength-reduced induction variables of mirrored expressions (`column * 128 - 256` and `256 - column * 128`)
  spilled in the wrong order: name one (`int x = column * 128 - 256;`) and write the other as `-x`. Every spelling
  of the two written out is canonicalised to the same code (BuildFaceGeometry `0x0043dc60`, 1100 -> exact).
- Two named struct locals with disjoint lives that retail keeps in one slot (while the call-result temporaries
  share another) can be one reused variable: `Vec3 x; if (c) { x = Cross(z, y); ... } ... x = Cross(y, z);`.
  Sibling blocks, inline helpers and scope moves do not reproduce that colouring (SetAxesIn `0x004fc050`,
  1137 -> exact with unchanged instructions).
  The same reuse fixes a use-density slot swap: a tail-path local that VC6 packs into a dead by-value argument
  slot while retail has it in the main vector's frame slot is that vector reused (PathSteeringDirection `0x0040e510`,
  1884 -> 1925). To size such a shortfall, add k probe reads of the variable and see where the layout flips.
- A commutative product that loads a struct member before a scalar local (retail the scalar first) means the member's
  leaf is too new: its only earlier reads were copy-propagated or inside an inline helper. A plain store to the member
  earlier in dominating order (`seg.y = 0.0f;` when flattening, dead and emitting nothing) or `m * 0.0f` (folded by VC6)
  creates the leaf; a dead initialiser at the declaration does not (`0x0040e510`, 1933 -> exact). Confirm with a probe read
  first, and apply it only where the store has a meaning the neighbouring code shares.
- sweep_variants templates: an inline choice ending in `{` followed by `@|` parses as nested; keep braces outside choices.
- In Git Bash, `/` compiler flags in env vars (`MCM2_EXTRA_FLAGS`) need `MSYS_NO_PATHCONV=1`, or they are ignored.
- A test the optimiser folds late flips `[base + index]` operand order. When a loop's
  address sums come out `[esi + eax]` for retail's `[eax + esi]` (or the reverse) and
  nothing else differs, look for a missing test of a non-const local whose value is known
  (`int flag = 1; if (!flag) return;`, `mode ? "a" : "w"`, a forwarded byte store read
  back) before the loop. It emits no instruction, but the block it removes changes the
  order the sums are built in. Tests the front end folds (literals, const/enum, inline
  constant arguments, a repeated guard) do not. EcoSystem 0x00458da0 reaches 461/461 this
  way; apply it only with evidence for the test's wording (it is otherwise dead code).

- Frame one scalar slot too big with every instruction aligned: look at the scope of the address-taken
  scalars (`GetInt(..., &x)` out-parameters). VC6 only lets a home be shared once its block is closed, and which
  locals share decides the slot order of the others. Scene::ReadAnimations `0x004edfe0`: the five sound-settings
  scalars as locals of the sound `if` block plus `automatic` as a local of the keyFramed block give retail's
  0xcfc frame and upper slots (5113 -> 5288); either change alone, or moving scalars to the loop body, does
  nothing. Map candidate and retail slots by paired instruction index first to see which variables retail shares.
  Only address-taken scalars respond: MorphBastard LoadParameterFile `0x004a33b0` gains 8 from a function-scope
  `targetValue` assigned in place, while 108 scope combinations of its register locals are flat.

## 4. Record and ship
- Exact: set `"expect": "exact"` in the neighbouring `targets.json`, update tests if they hash objects
  (`tests/` object-pin fixtures), regenerate `PYTHONPATH=. python tools/decompilation_progress.py` then `--check`.
- Not exact: add the ruled-out forms to the sample header or `targets.json` notes so nobody repeats them.
- Before pushing: `tools/static_check.py`, then the full `python -m unittest discover tests` (gate on `OK`), then
  `git pull --rebase`, push, `gh run list --branch main --limit 2`.
