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
- The converse for one component: copying it into a local after the call
  (`float nx = n.x; n.x = nx * inv;`) makes that local the newer leaf, so it
  loads first (`fld nx; fmul st(1)`) while the other components keep the
  scalar-first form. This made Tire `0x00515c90` exact; the spellings
  `n.x * inv`, `inv * n.x`, `c.x * inv` and inline float helpers did not.
  In a loop the same works with a vector copy made in the body after the
  scalar's leaf (`ParticleVec3 g(step.x, step.y, step.z);` before
  `v + (n - i) * g`): the DirtChunk, DirtSpray and Spark emitters' slot 10
  (`0x004b8f40`, `0x004b9460`, `0x004b99a0`) became exact with it.
  The copies work inside an inline helper too: Tire's UpdateContactPatch
  (`0x005135f0`) scales its slip vector through a helper that copies `v.x`
  and `v.y` into locals (`fld mem; fmul st(1)` for both, `fmul v.z` last);
  copying all three components or none gives the scalar-first form.
  ManagedTextureGroup `0x0050ef70` needs a local for each member that must
  load first: `u` for `width * u` and a per-axis `uScale` copy for
  `(uScale + pageOffsetU) * width` (one shared `scale` local is CSEd and
  breaks the frame).
  A freshly computed scalar is the newest leaf and is duplicated for the first
  product (`fld st(0); fmul v.x`): Tire's ApplyDrive (`0x00514170`, 560 to
  565) writes `r.x = a / b * v.x;` and then multiplies copies of `v.y` and
  `v.z` by a named `a / b` (`fld mem; fmul st(1)`), as retail does; the
  quotient as an expression in all three products gives the duplicate form
  for all of them.
- Storing the result vector: when an inline helper writes `r.x = v.x * s;`
  .. and returns `r` into a member, VC6 stores x and y before spilling the z
  product (`mov [edx+4]; fstp tmp`). Computing the three products into
  locals first and then filling `r` gives retail's order (`fstp tmp; mov
  [edx+4]`): Tire slot 1 (`0x00515b50`, 305 to 312) and UpdateContactPatch
  (with the same form for `a + b`). The converse holds for products stored
  straight into memory: TireBuildFrame (`0x00515880`, 522 to 622) wrote row
  0 as three float locals stored into the matrix; computing them into a
  CollisionVec3 local first makes VC6 spill the z product through the stack
  and store x and y last, as retail does.

So "a scalar is loaded first when it is a local of the inlined helper"
(docs/PHYSICS_VALIDATION.md) is the same rule: the helper's `k` is the newest
leaf. Bike slot 76 (`v.x *= (s = ...)`) makes `s` newer than `v.x`; a named
`s` first makes `v.x` the newer one.

### 1.4 What the rule does and does not explain

- Track `0x005179f0` (`length = fromStart + length`): retail loads `fromStart`
  first. A dead `float length = 0.0f;` at the top of the function gives `length`
  its leaf before `fromStart` is computed, so `fromStart` is the newer leaf and VC6
  emits `fld fromStart; fadd length`; the function is exact with it. Moving the
  `length = toEnd` store up instead folds `toEnd` away and changes the frame.
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

## 3. Redundant parentheses around a product

VC6 keeps a redundant pair of parentheses as a node of its own, and that
node changes the x87 schedule (and sometimes the operand order) of the
operation that consumes it. Writing `(a * b) - c * d` instead of
`a * b - c * d` is therefore a source-form difference, not a cosmetic one.

### 3.1 The cross-product copy

Probe (vc6_o2_ml, `static Vec3 s_axes[15]`, Math3D.h's Vec3):

```cpp
inline Vec3 C(const Vec3& a, const Vec3& b)
{ Vec3 r; r.x = a.y * b.z - a.z * b.y; r.y = a.z * b.x - a.x * b.z;
  r.z = a.x * b.y - a.y * b.x; return r; }
void f() { s_axes[6] = C(s_axes[0], s_axes[3]); }
```

Each component is stored to a temporary (`fstp [tmp.x]`) and copied with
integer moves while the next component is computed. Without parentheses
VC6 fills the slot after the second `fmul` with the whole copy:

```
fld; fmul; fld; fmul; mov ecx, [tmp.x]; mov [dst.x], ecx; fsubp st(1)
```

With the first product parenthesised (`(a.y * b.z) - a.z * b.y`, or both
products) the `fsubp` comes between the copy's load and store:

```
fld; fmul; fld; fmul; mov ecx, [tmp.x]; fsubp st(1); mov [dst.x], ecx
```

Parentheses around the second product only, around the whole difference,
a constructor-built result, member/`operator[]` access, an out pointer,
by-value parameters, a separate assignment and D3DVECTOR from the VC6
headers all give the first form; /G3, /G4, /G5, /GB, /Op, /Oa, /Ow, /QIfdiv
and the data alignment of the vectors do not move it (/G6 gives a third
schedule).

Retail has the second form at 82 sites of this shape in .text and the
first at 3 (none of them a cross-product copy); with `faddp` instead of
`fsubp` the counts are 0 and 25, the unparenthesised form, which matches
the dot products being written without parentheses. Functions that became
strict exact with the parenthesised form: BoxOverlap `0x00424ab0`,
SetAxesPtr `0x004fbd70`, RotatingShock::SolveContact `0x004fac60`,
SoultreePhysics slot 4 `0x005013d0`, Vehicle slot 35 `0x0040c540` and
FollowCamera slot 46 `0x004654e0`; Wrecker `0x00532580` gained 21 bytes.

### 3.2 Effects on operand order and other instructions

- The parenthesised product can also change the leaf order of the
  surrounding terms: SoultreePhysics slot 4's cross products (constructor
  form) load `a3` first in every term only with `(a.y * b.z) - a.z * b.y`;
  with both products parenthesised 15 bytes differ.
- Math3D.h's CrossProduct needs both products parenthesised for
  PhysicsRigidBody slot 11 `0x004cc630`: the second pair places the
  `lea edx, [esp+0x3c]` of an argument before the torque term's `fsubr`.
  The first-only form leaves that one instruction four places later; every
  other function using CrossProduct scores the same with either form.
- A sum is affected the same way: QueryDot written
  `a.z * b.z + (a.x * b.x + (a.y * b.y))` moves the store of a copied
  component after the inner `faddp` (`mov eax, [tmp]; faddp; fld; fmul;
  mov [dst], eax`), which makes PointInTriangle `0x004278d0` exact; the
  functions that need the `(y + x) + z` grouping keep their bytes.
- Parentheses do not fix the operand-order near misses whose leaves come
  from earlier statements: Camera slot 29 `0x0042eb10` (64 per-component
  placements), Vehicle slot 34 `0x0040c4c0` (256 placement and operand-order
  forms of the y and z terms; slot 35, same helper, is exact) and Wrecker
  `0x00532580` (16 forms applied to all three components).
- The node is not limited to products: `x -= ((float)fmod(x, 100.0));`
  issues an unrelated byte load (`mov al, [esi + 0x153f]`, the next
  statement's test) before the `fsubr`, as retail does; without the extra
  pair the load follows it. KrustyBike `0x00495ff0` became exact with it.
- 4x4 product term order is not a parenthesisation effect. Wrecker slot 10
  `0x005306e0` (one term pair of `_23`): every permutation and bracketing
  of `_23` (528 forms) and of each other single element (11 bracketings
  each) scores at most the plain sum; uniform bracketings of all 16
  elements lose 30 to 700 bytes and the heavier ones exceed the inline
  budget. D3DIMSoultreeShadow slots 27/29 (`0x004468f0`, `0x00446c30`):
  no uniform form of the 16 sums (24 term orders x 11 bracketings x
  product parentheses) beats the plain one; slot 27 alone is reached with
  a non-natural term order in `_21` plus a bracketed `_23`, but slot 29,
  which inlines the same helper, then differs in four rows, so no single
  helper text serves both.
- A greedy search that wraps one product or `(float)` cast at a time in
  redundant parentheses, run over every physics near miss up to 2600
  bytes (111 functions), found no other exact function; gains elsewhere
  were a few bytes and not kept.

### 3.3 Search over the non-physics near misses

Every render, track, camera, race, game, UI and net near miss whose
difference is term order or instruction placement in arithmetic was
recompiled over the parenthesisations of the differing expression (all
bracketings, term orders and parenthesised products; coordinate descent
where the product of the choices is large).

- Exact: TriangleNormal `0x004a11e0` (the squared length in NormalizeVector
  written `z*z + ((x*x) + (y*y))` makes the offset dot load `normal` first;
  of the 64 placements in the constructor-built cross product, exactly those
  with four parenthesised products match, so the count of nodes acts like
  the arena tie-break), ViewMatrix `0x004a1500` (member-stored cross product
  with both products of each component parenthesised places the integer
  column copies, 646 -> 740; the row-3 dots then need `z + (x + (y))`) and
  RadarOverlay slot 23 `0x0051bb60` (`(mapRadius / 180.0f) / zoom` moves the
  fld/fmul/fidiv before a dead test: division nodes behave like products).
- Improved: VehicleCamera `0x0052c030` 1221 -> 1233 (same NormalizeVector
  form), `0x0052c510` 813 -> 817 (`(x*x) + y*y + z*z`), Track `0x00516ef0`
  1023 -> 1032 (a doubly parenthesised factor `((b->x - a->x))` reorders one
  of six inlined edges), TrackOverlay `0x0051c4f0` 321 -> 334 (masked),
  Wrecker `0x00531740` 768 -> 772, BikeRace `0x00417ed0` 2800 -> 2804.
- No change: Camera slot 29 `0x0042eb10`, MatrixUtil `0x004a1a50` (6208
  forms), Wrecker `0x00531da0`, `0x00532580` and slot 10 `0x005306e0`,
  FollowCamera `0x00465720` and `0x004650e0`, Griddraw `0x0047e600`,
  EventManager `0x0045d480` and MorphBastardModifier `0x004a33b0`
  (D3DIMSoulTree `0x00444140`, also unmoved by them, is exact with the
  centre sum written per component). D3DIMSoulTree slot 12 `0x00443aa0` reaches
  826/832 only with a different bracketing per matrix element.
- Integer and address arithmetic never changed: Griddraw `0x00481300` and
  `0x0047e430` (`a + (b + c)`, `(a + b) + c`, parenthesised terms),
  PCTextureMap `0x004c8550` (the imul order, 432 forms) and D3DIMSoulTree
  `0x00440d40` (`&p->groups[j]`, `p->groups + j`, `(p->groups) + j`). The
  parenthesis node matters for the x87 expressions only.

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
    --fn 0x520ab0:Startup@TrackGame --kmax 63
# 0x520ab0 ...: exact only at some k (scheduling tie-break); scores [245, 584]; exact windows [(0, 4), (13, 63)]
```

A near miss (no window, same score at every k):

```bash
python3 tools/decl_shift_scan.py samples/render/MatrixUtilNearMisses.cpp \
    samples/render/MatrixUtilNearMisses.bindings.json --include src/reconstructed \
    --fn 0x4a1a50:ProjectPoints
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

## Integer index order in strength-reduced loops (Griddraw 0x0047e430, 0x00482b40)

`g_gridEdges[g_gridRow17[z] + x]` with `x` the inner loop variable compiles to
`mov edx, esi; add edx, [ebp]` (x first), retail's `mov edx, [ebp]; add edx, esi`
(row offset first) needs the inner loop to run on a copy of the starting x
(`for (int n = x; n <= 16; n += step)`). In `0x00482b40` the row also goes
through a pointer (`int* r = &g_gridRow17[z]; i = *r + n`) and `z` is
initialised before `zEnd` (the scheduler then emits `zEnd` first). Worth
trying on other `[base+index]` / `add r, r` order near misses.

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


## Source shapes measured on the physics class (c) near misses

- A vector normalised or scaled through an inline helper loads every component first
  (`fld v.x; fmul st(1)`) only when the argument is an operator temporary
  (`Normalized(a - b)`, `Normalized((a + b) * 0.5f)`); a named local built component by
  component (`Vec3 d; d.x = a.x - b.x; ...` or `Vec3 m(s.x * 0.5f, ...)`) keeps `x`
  scalar-first (`fld st(0); fmul [x]`). Bike slots 54/55.
- A switch whose cases each assign their own source (`dst = a; break;` per case) is
  tail-merged into one copy with the source in a scratch register; selecting a source
  pointer in the cases and copying once after the switch gives a different register plan
  and late callee-saved pushes. Tire `0x512e80`.
- A value read into a local before an if/else stays in its own register; the same member
  expression written inside both branches is CSEd into a register freed by the branch
  test. D3DIMSoultreeCharacter slot 4.
- `switch (n) { case 0: case 1: break; case 2: ...; default: ... }` lets VC6 send `n < 0`
  straight to the default block; the equivalent `if (n < 0 || n > 1) { if (n == 2) ...`
  re-tests `n == 2`. QuadTree `0x4dd600`.
- `while (1) { p = member; if (!p) return 0; ... member = member->next; ...;
  if (!member) return 0; }` gives retail's bottom `je exit; jmp top` whose target reloads
  the member; a guarded do-while on a local cursor jumps past the reload. QuadTree
  `0x4dd600`.
- A scan written as a guarded do-while with the found action inside (`if (cond) { ...;
  return; }`) places the action after the unrotated back edge; the same scan as a `for`
  loop with the action inside gives the rotated `jl top` form. Retail uses both in
  neighbouring functions (Vehicle slots 18/19 versus 20).
- A clamp through a by-value inline helper (`min(v.x, 15.0f)`) keeps the value on the x87
  stack across the compare (`fld; fcom; jne; fstp st(0); fld c`); the in-place forms
  `if (!(v.x < c)) v.x = c` and `v.x = v.x < c ? v.x : c` reload or spill it. Vehicle
  slot 19.
- Identical tails written out in each branch (`case 9: ...; tail; break; case 10: ...;
  tail; break;`) are cross-jumped: the fall-through branch keeps its copy and its own
  epilogue, the other branch jumps into it. A shared `goto` label or a tail after the
  switch places one copy after both branches instead. UIScrollCtl slots 55/60
  (`0x474880`, `0x4749f0`).
- Retail's three separate `EnableImeInput` call sites (one per value) come from
  `if (a && !strcmp(a, s)) f(0); else f(1);`; passing the condition as one argument
  computes it into a register first. GUIUser `0x487870`.
- Two `if (Blt(...)) goto failed;` branches with one `failed: return 0;` keep both tests
  as branches; `if (x) return 0;` followed by `return 1` becomes `neg/sbb/inc`, and two
  identical branch bodies are merged. GameCursor slot 15 `0x43ed40`.
- `if (w >= limit) w = limit;` with `int limit = p->w - 2;` emits `sub reg, 2`; the
  repeated expression `p->w - 2` is CSEd as `add reg, -2`.
- `int count = list->rowCount; int page = list->lastPageRowCount; f(g(count - page))`
  loads the subtrahend into ecx where `int rows = a - b;` (or the inline expression) puts
  it into a callee-saved register. UIDDLScrollBar slot 57 `0x479710`.
- Identical bodies written as separate cases (`case 3: body; break; case 6: body;
  break;`) are merged into one block, but the merge happens after register assignment,
  so later cases keep the alternation of the unmerged form; `case 3: case 6:` shifts it.
  SoultreeMaterial::ApplyRenderStates `0x4ff180`.
- The register that caches a repeated constant follows the constant's first use in the
  source: `field_0x30 = 1;` written first keeps 1 in ecx and places the vptr store
  between the -1 and 1 stores. BackgroundImage constructor `0x403d50`.
- `x*x + y*y + z*z` as one expression squares z first whatever the parenthesisation or
  accessor; retail's x, y, z order comes from accumulating one square per statement
  (`l = x*x; l += y*y; l += z*z;`). MPBikeRiderDlg slot 29 `0x4f78a0`.
- One `return frameList[currentFrame];` at the end, with each branch only updating the
  index, is copied into every exit and indexes through `mov ecx, eax`; early
  `return frameList[--currentFrame]` forms index with eax. UIAnim::Advance `0x472fe0`.
- `-x < 0 ? 0 : -x` compiles to the branch-free `mov edx, 0; neg; sets; dec; and`;
  `x < 0 ? -x : 0` compiles to branches. UIControl slot 39 `0x470f10`.
- Testing `sum + term > limit` before `sum += term` loads the term into its own
  register; `sum += term; if (sum > limit)` adds straight from memory.
  UIListBox::UpdateScrollBars `0x477bc0`.
- `atan2` of two components of an inlined `Vec3` temporary (`Vec3 d = a - b;`) keeps a dead
  duplicate of the x argument around `fpatan` (`fld st(0); fxch st(2); fxch st(1); fxch st(2);
  fpatan`) only when the call sits in a parenthesised cast or a double expression, and the
  form decides where the duplicate is popped: `(float)(atan2(d.x, d.z)) - yaw` pops it right
  after `fpatan`, before the yaw `fsub` (retail KrustyBike `0x0048e280` and the two head-turn
  sites of slot 102 `0x0048eea0`); `(float)(atan2(d.x, d.z) - yaw)` pops it after the `fsub`;
  `(float)atan2(d.x, d.z) - yaw`, `atan2(...) - yaw` and separate float locals for the two
  components give a plain `fpatan`. `(float)(x)` and `(float)x` differ only here: for `sqrt`,
  plain doubles and products the parentheses change nothing.
- Leaves belong to the variable an expression goes through, so an inline helper's parameter
  hides the caller's earlier references. Bike slot 72 `0x00405db0` normalises `*out` in place
  and then forms `CrossProduct(n, *out)`: written in the body, the normalisation gives `*out`'s
  components old leaves and VC6 loads `out` first in the 2nd/3rd terms; through
  `inline void BikeNormalizeInPlace(Vec3* v)` the cross product creates fresh leaves for
  `out` and `n` in its own order (`n.y < out.z < n.z < out.y < out.x < n.x`), which is retail's.
- The reverse also holds: copying by-value `Vec3` parameters into float locals at the top of
  the function (`float ax = a.x, ...; float bx = b.x, ...;`) makes those leaves older than
  everything computed later, so later locals load first (`fld c; fmul bz`, `fld fz; fmul fz`
  before `ay*ay`). OrientationAngles `0x004b5a60` needs this; scalar parameters, POD/const
  parameter types and inline rotation helpers do not give it.
- Not explained by the leaf-age rule: the term order of inlined 4x4 products (Wrecker slot 10
  `0x005306e0` relative._23, D3DIMSoultreeShadow slots 27/29, SoultreeObject::Scale
  `0x004fd340`). Within one product the order of the four terms changes from element to
  element although the elements share their leaves (Scale row 1 is k = 2, 3, 4, 1 with the
  zero elements of `s` loaded first for k = 2 only; rows 2 and 3 are k = 4, 3, 1, 2), so the
  chain is not sorted by any single per-leaf age. A by-value helper copies operands that a
  const-reference helper reads in place (shadow 680/767 with `a` by value); unrelated earlier
  leaves (probe `p2`) and the number of unused locals do not move a chain, but in a loop
  (MatrixUtil `0x004a1a50`) every extra live local (a stride copy, a counter, a matrix
  reference) reorders it, which points at the loop optimiser's renumbering of induction
  addresses rather than at the source text.
- A dead initialiser creates a local's leaf: `float length = 0.0f;` that is never
  read still makes later-computed values newer than `length`. Track `0x5179f0`.
- A vector built by a constructor with by-value parameters and then normalised gives
  x scalar-first and y/z component-first, with a final `fstp st(0)`; per-component
  stores into a named local give all three scalar-first. Track `0x518130`.

## Source-form levers measured on the 2026-10-11 near-miss pass

Each item names the function that became exact (or moved) with it. A form that
fixes one function is a candidate, not a rule: several were tried on siblings
without effect, as noted.

- **Pointer local to an array element.** `Vec3* p = &points[i];` used for the
  stores and for `Transform(p, *p, &m)` fixed x87 operand order and SIB order
  where the element is used across several statements: BoundingBoxTreeBuild
  `0x0042d390` (needed in both vertex loops), GraphicsTest DrawBox `0x0047c270`,
  DrawCircle `0x0047bd10` (+4). It also decides address operand order:
  `T* g = &s->arr[i]` gives `offset + base`, re-indexing `s->arr[i].` at each use
  gives `base + offset` (D3DIMSoulTree `0x004431f0`; not enough for its sibling
  DrawNormals `0x00442fe0`).
- **Byte-offset accessor.** For an array of large structs `messages[index]` gives
  `[edi + eax + N]`; an inline `MessageAt(messages, offset)` with
  `int offset = index * sizeof(NetMessage)` gives `[eax + edi + N]` (Net `0x004aced0`).
- **Reciprocal inside an inline helper.** `inline Vec3 operator/(const Vec3& v,
  float s) { return v * (1.0f / s); }` gives retail's `fld st(0); fmul [x]`; the
  same product in the caller (scale local, per component, `v * (1.0f / s)`) does not
  (Wrecker `0x00531da0`, SoulTreePhysics `0x00502330`).
- **Stores embedded in an expression.** `axis * (1.0f / (axisLength = len))` gives
  `fld st(0); fdivr [1.0]` where a separate store gives `fld [1.0]; fdiv st(1)`
  (Shock `0x004f9f90`). `m.x = v.x * (s = 1.0f / n); m.y = v.y * s; m.z = v.z * s;`
  gives the x-only `fld st(0); fmul v.x` of retail's collision means
  (CollisionObject `0x00437c20`, `0x00438280`, `0x004376f0`, `0x00437ef0`, +4..5 each).
- **Grouped sums through by-reference helpers.** `(a.x*b.x + a.y*b.y) + a.z*b.z` in an
  inline by-reference helper reproduced CollisionObject `0x00439600`; written inline,
  VC6 keeps v.z live. The squared length `v.z*v.z + ((v.x*v.x) + (v.y*v.y))` in an
  inlined helper (any parameter passing) instead of in the body gives `fld x; fmul
  st(1)` in the normalisation (VehicleCamera `0x0052c030`). A flat sum gets VC6's
  term order whatever order it is written in; only explicit grouping forces it.
- **Parentheses around call arguments.** In EcoSystem `0x004598d0` the
  `mov ecx, [g_collisionQuadTree]` of a `ComputeCode((x - r), (z - r), (x + r),
  (z + r))` thiscall moves before the last `fsub/fstp` once there are at least three
  redundant parenthesis nodes across the arguments, whichever arguments carry them.
  Parentheses changed nothing in integer-only code (BikeRace `0x0041eb20`, ZoneReport).
- **Mixing statement rows and expression rows.** `+=` statement rows ignore redundant
  parentheses (4096 combinations, SoultreeTransform `0x004fd660`); writing the last
  row as one parenthesised expression moved the result-pointer `mov ecx, eax`
  ahead of the last `faddp`.
- **One more live symbol.** A named result local (`Vec3 side = ...CrossB(up); axis =
  side;`), a pointer local or a `this` copy gave retail's b.x-first cross-product
  order in VehicleInlines `0x0040c4c0`; an unused int, braces and declaration shifts
  did not. `const Vec3& rear = ..., & front = ...;` declared rear first fixed Bike
  slot 46's register pair (`0x0040b600`).
- **Components read into scalars first.** `float y = v.y, x = v.x, z = v.z;` before a
  square sum or a matrix row sets the x87 load order (the declaration order) and frees the
  product order: CollisionObject StoreHitResult `0x00434bb0` and StoreHitNormal `0x00434cf0`
  matched with that declaration and `z * z + (x * x + y * y)`; DrawTreeMotion `0x004334c0`
  with `float x = v.x, y = v.y, z = v.z;` and a z row `x * m->_13 + (y * m->_23 + z * m->_33)
  + m->_43`. Reading `v.x` in place with the same bracketing stayed 1-3 products off. It did
  not help SoultreeObject::WorldToLocalDirection `0x004fd710` (207 -> 203).
- **Written operand order of a product between two struct copies.** For FollowCamera
  slot 47 `0x00465720` `b.m[k][j] * a.m[i][k]` (not `a * b`) gives retail's hoisted
  `a(0,3)` load, and `m[r][c]` access instead of `operator()` lets VC6 interleave the
  dword copies; only row order 2, 1, 3 with x, y, z components matched (36 swept).
  This is an exception to section 1's canonicalised factor order.
- **Constant identity.** `/ 10.0f` becomes a multiply by the reciprocal constant
  `0x005511cc`; `* 0.1f` uses `0x005507d0`. A relocation mismatch between the two
  says which the source used (VehicleCamera `0x0052c030`).
- **Register priority follows reference counts.** Writing the found call out in both
  branches (`Attach(entry->...); return;` twice), which VC6 cross-jumps
  back into one block, adds a source reference to `this` and moves it from ebp to
  ebx (Parameterblocks `0x004b7220`). Copying a member read into a named local before
  a compare inside a loop swaps the esi/edi assignment of loop-invariant parameters
  (UIDialog::EnableGroup `0x0046ea80`).
- **Store order of a local struct.** The order of field stores picks the registers
  of later vtable/address temporaries: UIDialog slot 28 `0x0046e8c0` matched one of
  1680 orders.
- **Bool joins.** `field = false; return false;` stores the immediate; a `bool result`
  assigned on every path with one `return result` gives `xor al, al; mov [m], al` and
  stops VC6 cross-jumping the final `return false` into another tail (FollowCamera
  `0x00465000`, VehicleCamera `0x0052cc80`). One ternary per component instead of one
  if/else block moves the `pop edi` placement (ToolTip `0x00486b10`, +3).
- **Not moved by any of these:** byte-store parity in EcoSystem `0x00458da0` (each
  variable-index byte store toggles the later SIB order), the `memset` + `.size`
  store placement in PCGame `0x004c0d10`, Tire `0x00513490`'s origin-store registers,
  the D3DIMSoultreeCharacter `0x00445fc0` SIB byte, and the Track `0x00516ef0` edge
  tie-break (~5,500 forms).

## Levers from the second 2026-10-11 pass

These forms made the named functions exact. As above, each one is a candidate to
try on similar code. None is a rule.

- **Partial aliases.** A second name used for only some reads changes the load
  order of those reads. `const Vector3& up = field_0x188;` used for the x/y stores
  and the first Cross argument fixed Camera slot 29 (`0x0042eb10`). The same idea
  fixed VisibilityClipper::ProjectVertices (`0x0052f190`), Wrecker (`0x00532580`),
  D3DIMSoultreeShadow slot 27 (`0x004468f0`, used only in the `_24` row of the
  inlined product) and PCTextureMap::WriteLevel (`0x004c8550`). In WriteLevel the
  alias was used in one branch and changed the operand order in the other branch.
- **Aliases also change address formation.** `T& r = *p;` used for one read
  changes the `offset + base` order of an element address. This fixed
  D3DIMSoulTree ReadLods (`0x00440060`) and TransformVertexGroups (`0x00440d40`).
  A reference to a local struct folds away and has no effect.
- **Local-symbol count orders equal-cost terms (provisional).** In larger
  functions, the order of the equal-cost terms of a 2-, 3- or 4-term sum follows
  how many local symbols were declared before the expression. The period is 32:
  n and n+32 locals give identical code on SoultreeBounds Scale `0x004fd340`.
  Only narrow windows of counts change a given row, and rows that share leaves
  flip together.
  - Each initialised local counts once, whatever its type. A reference counts
    once, for its declaration.
  - Locals in an inline helper add to the caller's count.
  - Uninitialised unused locals, file-scope declarations, extra statements and
    reassignments count zero. A partly stored Vec3 counts several units.
  - Placement does not matter except through the count. The exception is a
    helper shared by two functions: Shadow slot 29 `0x00446c30` becomes exact
    only with locals inside the helper it shares with the exact slot 27, and
    those locals break slot 27, so the change was not adopted.
  - The count does not move differences, cross products, the `fld X; fmul Y`
    operand order inside a product, or store scheduling. A trivial standalone
    function does not respond at all.
  - A named cast pointer in an inline helper that is expanded twice adds two;
    this fixed Wrecker slot 10 (`0x005306e0`).
  - A per-row search over all 24 term orders moved Scale from 596 to 618 bytes.
  - Of 17 term-order near misses swept with 0 to 33 dead locals, only four
    respond: Shadow slot 29, D3DIMSoulTree `0x00443aa0`, SoultreeTransform
    `0x004fd710` and Track `0x00516ef0`. Run that sweep before relying on this
    lever.
- **`||` versus separate `goto` tests.** Writing `if (a) goto failed; if (b) goto
  failed;` with the shared failure block under the last test places that block
  right after the test, so control falls through into it. One `||` condition
  places it elsewhere. This fixed MSZoneInterface `0x004aa360` and `0x004aa4e0`.
  In a switch case, `(A && B) || (C && D)` placed the second arm after the
  following loop, as retail does; if/else forms did not (KrustyUI `0x0049b0d0`).
- **Success in the if-branch.** When retail places the failure block after the
  success path, put the successful call in the if-branch and the failure in the
  else (DlgProcs `0x00452930`).
- **Zero stores can claim a register for the whole function.** Five `= 0` member
  stores made VC6 keep zero in ebx for all of DlgProcs `0x00452930`, which changed
  register choice and tail merging in distant cases. `memset` gives a local `xor`
  instead.
- **One shared return instead of an early return.** Writing the lookup hit and
  the load path as if/else with one `return` stops VC6 delaying the esi/edi saves
  past the early return (FontTexture `0x004673d0`).
- **Duplicate tails and per-branch flags.** Repeating `EndInputWait(); return 1;`
  in one branch adds a use of `this` and swaps ebx and ebp. VC6 still merges the
  duplicate tail, so the bytes match (OptControlsDlg slot 22 `0x004b5600`).
  Setting `kind = 1` in each branch after its code, instead of once at the join,
  fixed MapMovedInput (`0x004b5760`).
- **Naming a pointer local from a field before argument stores.** This makes VC6
  load the argument at function entry instead of hoisting its store above earlier
  byte stores (FileStream ctor `0x00460d10`).
- **Parentheses around a whole initialiser.** `float d = ((float)a - (float)b);`
  changed the order of a `fild` and an unrelated global load (MultiPlayerDlg slot
  24 `0x004f3a70`). Parenthesising only one operand made the result much worse.
- **Exhaustive store-order sweeps.** All 720 orders of six independent stores
  took about 36 s with 12 jobs and found 20 exact orders for UIListBox::AddImageRow
  (`0x004773a0`). An earlier hand-picked set of 40 orders had found none.
- **Check semantics first.** Some diffs that looked like register allocation
  were reconstruction errors:
  - Vehicle `0x005293e0` called a `__thiscall` callee that was declared
    `__stdcall`. The callee's prologue does `mov esi, ecx`.
  - Vehicle slot 55 `0x0052b6d0` wrote to the wrong argument.
  - The loop bound of MultiPlayerDlg slot 24 was one record too small. The
    masked comparison hid this because the end pointer is a relocation, so
    check relocation addends whenever the masked score is full but the strict
    score is not.
- **Not moved:** in PCJoystickDevice `0x004c3790` and FileStream `0x004618e0`,
  retail's eax→ecx→edx scratch-register rotation starts one step later than
  VC6's. No form tried shifted it. In VC6 SP3, two arrays whose element type has
  a destructor never share a stack slot, but retail's 0x0044b200 shares one. The
  `sub esp` before `rep movsd` in Camera `0x0042ee30` was not reproduced either.
- **Assignment inside an x87 compare.** In Griddraw `0x00481b30`,
  `if ((d = a - b) > 0.0f) r = d * d;` compares a copy (`fld st(0) ; fcomp [c]`)
  and keeps `d` on the stack for the product; the same difference written as a
  common subexpression (`if (a - b > 0.0f) r = (a - b) * (a - b);`) or as a local
  assigned before the test compares with `fcom`. Assigning into a local also ends
  single-use forwarding: with `float hi = lo + size;` read once, VC6 folds `hi`
  into `lo + size` (keeping `lo` and `size` on the x87 stack) unless `lo` is
  overwritten first (`(lo = a - lo) < 0.0f`).
- **Result flags threaded into one block.** Setting `notify = 1` in two branches and
  testing it once after the if/else gives one copy of the notified block, with the
  `else if (flag && !changed) return;` test placed after it; gotos into a shared
  block, or the block written in both branches, place that return test inline.
  Griddraw `0x00482dd0`.
- **The scheduler region is the basic block.** Calls do not end it: in
  SPBikeRiderDlg slot 10 (`0x0044fae0`) the by-value Vector3 copy for the
  camera's slot 29 is scheduled together with code before the preceding
  SetFrame call. Adding or removing an instruction anywhere earlier in the same
  block changes the copy's schedule; changes in other blocks do not. A label
  between the two calls (no code) gives retail's schedule exactly; `{}`,
  `if (1)`, `do {} while (0)`, `switch`, an empty inline call and dead copies
  do not split the block. The label is not used in source (it is a
  matching-only form); see samples/ui/DlgProcsNearMisses.cpp.
- **The matrix operand's spelling sets an inlined rotation's sum order.**
  CollisionObject HullVsHull (`0x00436720`, 66 to 932) and HullVsHullSwept
  (`0x00436af0`, 715 to 809) inline the same `out->x = v.x*m[0][0] + ...` helper
  several times. Passing the matrix as `&b->worldTransform` or through a
  `const Matrix4* xf` local (same register, same code otherwise) changes the
  order the three products are summed in, per call: the first use of `xf`
  sums row x x-first, later uses sum every row x-first, and direct uses keep
  the reverse source order. Term order inside the helper does not move it. A
  sweep over the 2^n direct/`xf` choices per call (n = 8) found both
  functions' best forms.

## Levers from session 14

- **A named result copied after an intervening statement.** Tire ApplyDrive
  (`0x00514170`): `push = TireRatioTimes(...); *speed -= amount;` copies each
  component of the helper's return temporary right after its product; retail
  copies push.y after the speed store. `CollisionVec3 scaled = TireRatioTimes(...);
  *speed -= amount; push = scaled;` gives retail's order (x and z still copied
  early, y in the freed `edx`).
