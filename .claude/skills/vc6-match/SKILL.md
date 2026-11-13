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
- In Git Bash, `/` compiler flags in env vars (`MCM2_EXTRA_FLAGS`) need `MSYS_NO_PATHCONV=1`, or they are ignored.

## 4. Record and ship
- Exact: set `"expect": "exact"` in the neighbouring `targets.json`, update tests if they hash objects
  (`tests/` object-pin fixtures), regenerate `PYTHONPATH=. python tools/decompilation_progress.py` then `--check`.
- Not exact: add the ruled-out forms to the sample header or `targets.json` notes so nobody repeats them.
- Before pushing: `tools/static_check.py`, then the full `python -m unittest discover tests` (gate on `OK`), then
  `git pull --rebase`, push, `gh run list --branch main --limit 2`.
