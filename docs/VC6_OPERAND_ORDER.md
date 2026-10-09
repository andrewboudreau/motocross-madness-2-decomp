# VC6 operand order and equal-cost register choice

What VC6 SP3 (the calibrated `/O2` profile of `tools/compile.py`) does with
the two operands of a commutative operation when both cost the same, and
why the result sometimes moves when unrelated declarations are added to a
header. Measured with small probe translation units (reproduction at the
end); every statement below is backed by a probe unless marked as open.

Two separate mechanisms are involved:

1. **Operand order** (`fld a; fmul b` versus `fld b; fmul a`, `[base+index]`
   versus `[index+base]`, which operand is the destination of `add r, r`)
   is deterministic. It follows the *age of the operand's leaf node* in the
   function's intermediate code, not the written order, not identifier
   names, not declaration order.
2. **Load scheduling and register assignment** of two independent
   equal-priority loads is a tie-break that depends on where the function's
   nodes land in the compiler's symbol arena. That position moves with the
   number (not the names) of symbols declared earlier in the translation
   unit, with a period of 32 units (64 typedefs). This is the header
   sensitivity seen in TrackGame slot 1. It changes which register gets
   which value, not the operand order, and it is rare: a scan of every class
   (c) near miss and of 971 registered functions found two sensitive
   functions.

## 1. Operand order

### 1.1 Written order and reassociation

- `a * b` and `b * a` compile identically in every probe; so do `*a + *b`
  and `*b + *a`, `p[i]` with the parameters swapped, and so on.
- Chains are reassociated: `x * y * z`, `x * (y * z)` and
  `v->x*w->x + v->y*w->y + v->z*w->z` are all flattened, and the flattened
  terms are emitted in reverse source order (`z, y, x`). Explicit
  parentheses are honoured: `zz + (xx + yy)` keeps `(xx + yy)` as a unit and
  emits `yy, xx, zz`. This is why squared lengths need
  `z*z + (x*x + y*y)` to reproduce some retail orders.

### 1.2 The leaf-age rule

For a two-operand commutative operation whose operands are both leaves (a
memory operand or a value already on the x87 stack), VC6 loads first the
operand whose leaf node is **newer**. Leaves are created in the order the
intermediate code is generated:

| operand kind | when its leaf is created |
|---|---|
| parameter, `this` member | at function entry, so always oldest; among parameters the *first* parameter counts as the newest (`v->y * w->z` loads `v->y`; `*a * *b` loads `*a`) |
| local variable (`float a;`, `V v;`, `V* p;`) | at its first reference in the body: a store, an address-of (`fill(&a)`) or a read. The declaration itself does not count (`float b, a;` versus `float a, b;` changes nothing) |
| global, member through a pointer, array element | at its first *arithmetic* use in dominating order. Passing it as a call argument (`ext(v->y)`) or taking its address (`fill(&v->y)`) does not create the leaf; a use inside a branch that does not dominate the operation does not count either |
| member of the same base, both new | the larger displacement first (`v->y * v->z` loads `z`; `a[1] * a[2]` loads `a[2]`; `far` members beyond 127 bytes before near ones; declaration order and names irrelevant, measured with unions) |

Probe evidence (operand loaded first is listed):

| probe | result |
|---|---|
| `float a, b; fill(&a); fill(&b); return a * b;` | `b` (newer); with the fills swapped `a`; with only the declarations swapped still `b` |
| `a = ext(1); b = ext(2); ... a = b + a` in a later block | `b`; with the stores swapped `a` |
| `float t = v->z; fill(&t); return v->y * v->z;` | `v->y` (z already has a leaf); with `t = v->y` first, `v->z` |
| `t = w->y; ... u = v->y; ... v->y * w->y` | `v->y` (referenced later); reversed, `w->y` |
| `t = w->far1; u = v->y; v->y * w->far1` | `v->y`: reference order overrides the displacement class |
| `if (c) { t = v->z; } u = v->y; v->y * v->z` | `v->z` (the branch does not dominate) |
| `ext(v->y)` or `fill(&v->y)` before `v->y * v->z` | `v->z` (no leaf was created) |
| `x * q` in a member (`x` this member, `q` parameter) | `q`; `y * v->y` loads `v->y`; after `t = v->y` it loads `y` |
| `a * g` (local, global) | `g` (created at the product); `g * q` loads `g`; `v.z * q` loads `v.z` |
| `V* a = get(1); V* b = get(2); a->y * b->z` | `b->z` (assigned later), independent of declaration order |
| `v->next->y * w->z`, `a[i] * a[0]` | the operand that needs an address computation (`v->next`, `a[i]`) |

Register-sized integers follow the same order: `*a + *b` loads `*a` into the
destination register first; for `s.availPhys + s.availPageFile` the member
with the larger displacement is the destination (`add <pageFile>, <phys>`)
in every state of the tie-break of section 2.

### 1.3 A scalar on the x87 stack times a vector

`out->x = v.x * s; out->y = v.y * s; out->z = v.z * s;` with `s` the result
of a call (in `st(0)`):

- `fld v.x; fmul st(1)` (component first) when the components were not
  referenced before, or only as call arguments, or only inside an inlined
  helper that received the vector by value or by reference (`lensq(v)`):
  their leaves are created at the products, after the call.
- `fld st(0); fmul v.x` (scalar first) for every component that was used
  earlier in an arithmetic expression of the same function body
  (`l = v.x*v.x + v.y*v.y + v.z*v.z`, `l = v.x + v.y + v.z`, `l = v.x * 2.0f`
  only for `x`), i.e. whose leaf is older than the call. In this form the
  last product of the group consumes the scalar (`fmul v.z`).
- A scalar stored to memory first (`fillf(&s)`) is the newest leaf and loads
  first (`fld s; fmul v.x`) for all three components.

So "a scalar is loaded first when it is a local of the inlined helper"
(docs/PHYSICS_VALIDATION.md) is the same rule: the helper's `k` is the newest
leaf. Bike slot 76 (`v.x *= (s = ...)`) makes `s` newer than `v.x`; a named
`s` first makes `v.x` the newer one.

### 1.4 What the rule does and does not explain

- Track `0x005179f0` (`length = fromStart + length`): VC6 loads `length`
  because it is stored after `fromStart`; retail loads `fromStart`, so the
  original referenced `length` before `fromStart` was computed. Moving the
  `length = toEnd` store up makes VC6 fold `toEnd` away and changes the frame
  (429/921); declaring `length` earlier without a reference changes nothing.
  The source form that keeps `toEnd` a separate slot and still references
  `length` first was not found.
- MatrixUtil `0x004a1a50`, SoultreeTransform `0x004fd710` (`v.x * m._11`,
  the only matrix element the transpose does not touch), Bike slot 54 and the
  particle emitters' normalisations (`fld st(0); fmul` on the first
  component only) are not reproduced by any reference-order change tried
  (loop update order, `for` forms, term order, by-value helpers, helper
  copies). Their retail order needs a leaf history the candidates do not
  produce; the inlining of copies is the likely missing piece (an inlined
  by-value parameter is a fresh set of leaves).

## 2. Scheduling tie-break and the declaration count

### 2.1 Reproduction on TrackGame slot 1

`TrackGame::UnknownVirtualSlot1` (`0x00520ab0`) adds
`status.availPhys + status.availPageFile`. Retail and the registered source
give

```
mov edx, [esp+0x24]   ; availPageFile
mov eax, [esp+0x1c]   ; availPhys
add edx, eax
```

Prepending unrelated declarations to TrackGame.cpp (before its includes)
gives, for some counts,

```
mov edx, [esp+0x1c]
mov eax, [esp+0x24]
add eax, edx
```

The destination is still the register holding `availPageFile`: the operand
order is unchanged, the two loads are issued in the other order and the
first-issued load takes the first free register. Which counts flip it:

| prepended declaration | flips at n = | weight (units) |
|---|---|---|
| `int f(int);` | 3..6 and 35..38 | 1 |
| `int f(int, int, int);` | 2, 3 and 18, 19 | 2 |
| `void f();`, `typedef int T;`, `extern int v;`, `int v;`, `static int v;`, `typedef int* T;`, `struct S;` | 5..12 | 0.5 |
| `enum { E = n };` | 3..6 | 1 |
| `struct S { int a; };` | 1, 8, 15 | about 4.5 |
| `struct S { int a, b, c, d; };`, `class C { public: int m(int); };`, `typedef struct { int a; } T;` | 1, 6/7, 14, 17/20 | 5 to 6 |
| comments, `#define`s | never | 0 |
| a function *definition* before the file | always (its symbol, parameter and locals count; the body's size does not) | |

So the quantity is a count of symbol-table entries: half a unit per symbol
(typedef, variable, function, parameter, enumerator, member), about four
units per class type, and the pattern repeats every 32 units. Identifier
names and their lengths do not matter (`z0`, `dummy0` and 80-character
names behave alike). The flip window is four units wide, which is the
distance between the two nodes whose relative position the tie-break
compares; a window appears wherever a pool boundary falls between them.
Removing the tail of the function (everything after the memory check)
leaves the flipped form at every count, and keeping only the tail moves the
window and makes a third form (`ecx` instead of `edx`), so the default state
is set by the rest of the function and the arena only perturbs it.

The same shift does not move the small two-operand probes of section 1 at
any count (0..70 typedefs), nor the x87 operand order of the near misses
below.

### 2.2 How rare it is

`tools/decl_shift_scan.py` compiles a file with 0..63 prepended typedefs
(0..31.5 units, one full period) and reports each bound function's strict
score at every count. Run over the class (c) near misses of
docs/NEAR_MISS_INDEX.md whose files compile standalone with the default
include paths (81 functions in 78 files) and every other bound function of
those files (1356 functions, 1252 of them exact at every count):

- no near miss becomes exact at any count; every one keeps the same score
  at all 64 positions;
- one function changes score: BikeRace `0x00419970` (a partial, 1376 and
  1382 of 13322 bytes);
- TrackGame slot 1 is the only known exact function that flips (exact at
  0..4 and 13..63 typedefs, flipped at 5..12).

Header sensitivity is therefore not the cause of the class (c) near misses;
they are source-form differences under the deterministic rules of section 1.

### 2.3 Practical consequences

- After editing a shared header, re-run the calibration; if a function
  regresses with a register swap of two equal loads, scan it with
  `tools/decl_shift_scan.py` to see its windows, and prefer declaring new
  types in the file that uses them (as EventManager.cpp does).
- Adding or removing declarations to move a near miss into a window is not a
  fix: the scan shows no near miss that has one.

## Reproduction

Probes (compile with `tools/compile.py --compiler vc6`, disassemble the
object with capstone; `tools/disasm_fn.py` or the scratch `probe.py` of the
session):

```cpp
struct V { float x, y, z; float pad[100]; float far1; };
void fillf(float*); float ext(int); int cond(int);
float a_yz(V* v)            { return v->y * v->z; }                       // fld [v+8]
float m_y_far(V* v, V* w)   { return v->y * w->far1; }                    // fld [w+0x19c]
float r1(V* v)              { float t = v->z; fillf(&t); return v->y * v->z; }   // fld [v+4]
float s3(V* v, V* w)        { float t = w->y; fillf(&t); float u = v->y; fillf(&u);
                              return v->y * w->y; }                        // fld [v+4]
float u3(V* v)              { if (cond(1)) { float t = v->z; fillf(&t); }
                              float u = v->y; fillf(&u); return v->y * v->z; }   // fld [v+8]
float dl1()                 { float a, b; fillf(&a); fillf(&b); return a * b; }  // fld b
float dl3()                 { float a, b; fillf(&b); fillf(&a); return a * b; }  // fld a
float t4(V* v, V* w)        { return v->x*w->x + v->y*w->y + v->z*w->z; } // z, y, x
float t5(V* v, V* w)        { return v->z*w->z + (v->x*w->x + v->y*w->y); } // y, x, z
```

TrackGame slot 1 window (needs `$VC6_ROOT`, `$MCM2_EXE`):

```bash
python3 tools/decl_shift_scan.py src/reconstructed/TrackGame.cpp \
    src/reconstructed/TrackGame.bindings.json --include src/reconstructed \
    --fn 0x520ab0:UnknownVirtualSlot1@TrackGame --kmax 63
# 0x520ab0 ...: exact only at some k (scheduling tie-break); scores [245, 584]; exact windows [(0, 4), (13, 63)]
```

A near miss (no window, same score at every k):

```bash
python3 tools/decl_shift_scan.py samples/render/MatrixUtilNearMisses.cpp \
    samples/render/MatrixUtilNearMisses.bindings.json --include src/reconstructed \
    --fn 0x4a1a50:UnknownFunction4a1a50
```

## Remaining uncertainty

- The leaf-age rule is an empirical description of the backend's value
  numbering; the exact pass (local CSE with dominance, or a per-block leaf
  table re-created at joins) is not identified. The "branch does not count"
  observation rests on one probe shape.
- Parameters count as newest-first among themselves (the first parameter
  loads first) while locals count newest-last; a by-value struct parameter's
  members behaved like locals in one probe (`v.x * q` loads `v.x`) and like
  parameters in another (`v.z * w.x` loads `w.x`). Inlined helpers with
  by-value parameters create fresh leaves for the copy; how their age
  compares with the caller's leaves was not measured.
- The arena model (half a unit per symbol entry, four units per class, a
  32-unit period) is inferred from flip windows only; the unit's size in
  bytes and the data structure doing the pointer comparison are unknown.
- The flattened-chain emission order (reverse source order) was measured for
  three-term chains of leaf products; longer chains and mixed chains were
  not.

## Control-flow source shapes (measured while matching PCAudio, Net, Pixtrans and BackgroundImage)

- Retail's unrotated loop exit (`jge exit; jmp top`, with the return block between the
  loop and the following code) comes only from a guarded do-while:
  `int i = 0; if (n > 0) do { ... } while (++i < n);`. `for` and `while` loops, with or
  without `break`/`goto`, are rotated. (`while (1) { if (done) break; ... }` keeps a
  top-tested loop instead, see FontTexture.)
- A trailing `return X;` after an if-block merges with an earlier identical return, so
  failure paths jump back to it; `goto failed` or an early return emits a second copy.
- A helper written as a macro reproduces retail's address recomputation where the same
  helper as an inline function keeps the destination in a register (PCAudio's sound
  name copy).
- `new` of a class whose constructor is defined earlier in the same unit and cannot throw
  needs no EH frame, so such a function only gets retail's frameless prologue when
  compiled inside its own unit.

