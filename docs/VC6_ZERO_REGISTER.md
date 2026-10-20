# VC6 SP3 zero register

Retail functions often clear a callee-saved register at entry (`xor ebx,
ebx`) and then use it for every null test and zero store (`cmp ecx, ebx`,
`cmp [esi + 0x260], ebx`, `mov [esi + 0x26c], ebx`). Without that register,
VC6 loads and `test`s each operand instead, and every argument offset moves
by the missing push. So whether the zero register exists decides most of a
function's bytes. Measured with small probe units under `vc6_o2_mt`.

## What decides it

- **Stores of 0 count; tests against 0 do not.** In a probe shaped like
  Tire::UpdateShock (null tests, a call, int tests after the call, and two
  zero stores on the failure path), adding `if (active) f = 2;` statements
  after the call never produces a zero register. Adding `q[0] = 0;`
  stores does: two extra zero stores (four in all) give `push edi ; xor
  edi, edi`, and one (three in all) does not.
- **A zero that must survive a call needs a callee-saved register and more
  stores.** With no int test after the call, the same probe keeps 0 in a
  scratch register (`xor edx, edx`) with just the two failure-path stores.
  One int test after the call (`if (active) ...` or `if (crashed) ...`)
  is enough to lose it. A float test there (`if (dt != 0.0f)`) is not.
- **The count is taken before tail merging.** VC6 merges identical tails
  (`field_0x26c = 0; inContact = 0; return;` written on two failure paths)
  into one block after it has decided on the zero register, so retail's
  single clear block can come from two written copies. Tire::UpdateShock
  `0x00513c70` went from 96 to 769 of 797 bytes this way (zero register,
  prologue, argument offsets and block layout as retail). With the clears
  written once after the `if`/`else` chain it compiles without the
  register.

## Measured thresholds (session 14 probe study)

Each probe below changes one factor and checks whether a constant gets a
register. The probes are compiled VC6 SP3 `vc6_o2_mt` output, so they are
confirmed. The model drawn from them is a strong inference: it fits every
probe below and the reproduced near-miss shapes.

**What counts.** A constant becomes a register candidate only when it has
at least two *counted* uses:

- Counted: stores with a displacement or an absolute address. This covers
  dword, short, float `0.0f` (stored as an int), NULL pointer, enum and
  global stores, and the store to a spilled local's home slot.
- Not counted: stores through a bare pointer (`[reg]`, offset 0), pushes,
  and compares or tests. With `*dc = 0; *a = 0;` plus displaced stores,
  only the displaced stores count. Pushes inside a loop do not count
  either.
- Not counted, and not served by the register even when it exists: stores
  to locals the optimizer keeps in memory. Three cases were probed:
  - a scalar whose address escapes to a call (`gp(&i)`);
  - a local struct passed by address (`Vector3 zero(0.0f, 0.0f, 0.0f);
    Length(zero)`);
  - a float scalar local (`float best = 0.0f`).

  Only the address-escape case matters for the call threshold. An escaped
  `i` with `i = 1` and one member store of 1 stays immediate where a
  register `i` (or a spilled one) caches.
- Still counted: an int array local indexed in place (`loc[1] = 0;`),
  `volatile` locals, and the home slot of a register local that was
  spilled.

Constant 1 behaves exactly like 0. Retail also caches -1 (`or ebp,
0xffffffff` in CubeDraw `0x0043dc60`), but that case was not probed.

**Byte constants form their own web.** A byte store does not share a web
with dword stores of the same value: `ch = 0; f(); c = 0;` stays
immediate. Short, float 0.0f, pointer and enum stores do share the int web.

**Threshold by register class.** All counts are counted stores of one
value in one function:

| Range of the stores | Register | Needed |
| --- | --- | --- |
| no call, a scratch register free throughout | `eax`/`ecx`/`edx` | 2 |
| no call, scratch registers busy (compare chains) | callee-saved | 3 |
| spans a call, callee-saved register already pushed by the function and free over the range | that register | 2 |
| spans a call, a callee-saved register that must be newly pushed | `esi`, `edi`, ... | 4 |

- Length, the number of basic blocks and the number of calls do not move
  these numbers. Tests between the stores, however many, do not either.
- The split of stores around a call does not matter: one before and three
  after caches, as do two and two; one and two does not.
- The existing UpdateShock observations fit this table. An int test after
  the call extends the range across the call, which moves the web from
  the first row to the fourth.
- A store in a loop weighs about twice as much: two stores in a loop with
  a call get `ebx`, and one store in a call-free loop gets `edx`.
- The "already pushed" row is what makes the over-cache rows below
  plausible. In a probe with an inline `strcpy` after the stores
  (`repne scasb`/`rep movsd` need `esi`/`edi` anyway), two counted stores
  around a call get `edi`. Without the `strcpy`, four are needed.

**Once allocated:**

- The constant's definition is hoisted to the common dominator of its
  uses, often the function entry.
- Every push and compare of that value inside the range then uses the
  register (`push edi`, `cmp esi, eax`).
- When every callee-saved register is taken, the web is split at the
  call. Stores after the call get a scratch register (`xor eax, eax`) and
  stores before it stay immediate.
- Register choice follows priority. Webs that save more per instruction of
  range pick first, and callee-saved registers go in the order `esi`,
  `edi`, `ebx`, `ebp`. Example: in a loop, the pointer takes `esi`, the
  counter `edi` and the zero `ebx`.

**Variables holding the constant.**

- VC6 propagates named locals that only hold a constant (`int one = 1`,
  `const int`, `bool`, a local copy of a global read once), so they change
  nothing.
- A *register* variable initialised to the constant (`int i = 0` for a
  later loop, a counter `n = 0`) is coalesced with the constant web. Its
  register serves the constant stores until the variable changes.
  Example: two counted stores around a call use `i`'s `esi`, because that
  register is already pushed. One store stays immediate.
- After `test reg, reg; jne`, VC6 also reuses the known-zero `reg` for a
  following 0 store or push.

**Reproduction.**

- The probe harness is `work/s14/cz/probe.py SRC.cpp [FUNC...]
  [--summary]` (ignored). It compiles with `tools/compile.py --compiler
  vc6 --profile vc6_o2_mt` and lists `xor`/`mov reg, imm` setups and the
  remaining immediate 0/1 stores per function.
- The generators are `work/s14/cz/p*.py` and `p*.cpp`. Each new probe is a
  few one-line functions on `struct S { int a, b, ...; }` that vary one
  factor.

## Near-miss rows checked against the thresholds

- BackgroundImage GetDc `0x004049d0` (VC6 caches, retail does not):
  - The candidate has two counted zero stores, `surface = 0` (home
    `[esp + 8]`) and `copyDcNext = 0` (`[esi + 0x60]`). `*dc`/`*a` are
    offset 0, so they do not count.
  - `edi` is pushed anyway (surface's register at the end), so the
    "already pushed, 2" row applies. In the candidate, `edi` is the first
    live-range piece of `surface`, coalesced with the zero.
  - Retail keeps `surface` in memory until the final join. The rule
    therefore says retail's source has at most one counted zero store on
    that path. The reorders of `surface`'s initialisation (declaration
    initialiser, before or between the `*dc`/`*a` clears) leave the
    caching in place.
  - Diagnostic only, not a source candidate: letting `&surface` escape to
    a call on a dead path makes both zero stores immediate, as in retail.
    It also drives `surface` fully into memory (masked 108 to 375/606,
    but 692 bytes instead of 678). Retail keeps a final register piece
    (`mov edi, [esp + 8]` at the join and `edi` after the `ReleaseDc` call),
    so retail's `surface` is not address-escaped. The search for the second
    counted store stays open.
- EcoSystem ReadEst `0x00457480`: the index row's direction is VC6
  caching 1 in `edi` and retail storing immediates, not the reverse.
  - The two counted stores are `method = 1` (`[esi + 0x30]`) and `i = 1`
    (the spilled home `[esp + 0x18]`, reloaded at once). `edi` is pushed
    for the string expansions.
  - The probe `if (!g(1)) s->b = 1; else if (!g(2)) s->b = 2; else
    return; s->c = g(3); s->d = 1; strcpy(...)` reproduces the
    candidate exactly (`mov edi, 1` hoisted to the entry, `push edi` for
    `g(1)`). Without the `strcpy` it stores immediates.
  - An inline setter, `== 0` tests and other first-call arguments do not
    change it. Collecting the branch value in a local (`m = 1 / 2; s->b =
    m;`) removes the cache but moves the store out of the branches, which
    is not retail's shape.
  - Diagnostic only: letting `&i` escape on a dead path makes `method = 1`
    immediate, as in retail, but it moves `this` to `ebx` and retail
    passes `i` by value everywhere, so retail's `i` is not address-escaped
    either. A loop-scoped `int i` is a register variable in the probes, and
    the 1 is coalesced into it. Retail reloads `i` from `[esp + 0x18]` on
    every use.
  - So both "bracketing" rows are the same direction (VC6 over-caches with
    two counted stores and an already-pushed register). Neither is solved.
- Griddraw DrawVisibleBlock `0x00480ad0`: retail's `xor eax, eax` at the
  top, with `cmp esi, eax` / `cmp ecx, eax` and `indexStart = eax`, is the
  "compares use an allocated constant" case.
  - With both starts initialised at the top (`int vertexStart = 0; int
    indexStart = 0;`), VC6 gives retail's `cmp reg, eax` compares. But it
    hoists the `indexStart` store out of the `block == 0` branch (355/438).
  - Assigning `vertexStart = 0; indexStart = 0;` inside the branch (no
    top initialiser) gives retail's esi/edi roles: 14 instead of 28
    differing instructions. It is one byte short (60/438 strict, offsets
    shift), and the zero stays in the branch.
  - The missing byte is a scheduling change, not a zero encoding. In the
    `block != 0` arm, the in-branch form loads `vertexStart` after `lea
    esi, [esi + edx*4]` as `movsx eax, word ptr [esi - 0xc]` (no SIB, 4
    bytes). Retail loads it before the `lea` as `movsx eax, word ptr
    [esi + edx*4 - 0xc]` (5 bytes).
  - The in-branch form also holds `ranges` in `eax` with `test eax, eax`,
    because no zero lives at the top. Both effects come from the zero web
    starting inside the branch.
  - Reordering the four `block != 0` assignments or adding a `prev` pointer
    does not restore the order: 374 and 378 of 438 with 19 and 16 diffs.
  - Swapping the arms and `block == vertexStart` compares do not help
    either. No form found yet. Variants are in `work/s14/cz/gd/`.
- Griddraw RebuildBlockBuffers `0x0047f210` and CubeDraw BuildFaceGeometry
  `0x0043dc60`: both functions allocate the constant as retail does. The
  rows differ in which register it gets (`esi`/`edi` swapped with a pointer
  web; `ebp` vs scratch `edx` for the face counter's zero, with -1 in
  `ebp`/`ebx`), which is a priority order between webs. Both rows also
  have slot-order differences.
- EcoSystem GenerateObjects `0x00459ce0`: both use `xor edi, edi`. Retail
  materialises it before the call at 0x508970 for a zero store to a local
  (`[esp + 0x84]` inside the pushes) that the candidate makes after the
  call into another slot. The register is the same, so this row is store
  placement and frame layout, not caching.
- D3DIMSoulTree UpdateMaterialTexelRatios `0x00440810`: both have the
  entry zero register `ebx`. The `best = 0.0f` and `Vector3 zero` stores
  stay immediate in both, as the memory-local rule predicts.
  - The candidate's extra `xor esi, esi` / `mov [esp + 0x20], esi` is a
    separate face counter (frame 0x84 vs 0x80). Retail starts the face
    offset from the zero register (`mov esi, ebx`) and steps `ebx` by 6.
  - So this is an induction-variable difference, not constant caching.
- PCTextureMap Clone `0x004c71c0`: the 1 is cached in both. Retail puts
  it in `ebx` and a pointer in `ebp`; VC6 here swaps them, which is web
  priority. The candidate is 590 bytes against retail's 538, so the extent
  has to be fixed first.
- GridNode `0x00484d70`: its `xor ecx, ecx ; mov cx, ...` is the
  zero-extension idiom for an unsigned short load, not a cached constant.
- KrustyBike `0x00493660`: the choice is function wide (a diagnostic early
  `return` moves even the entry zero register to `ebx`). See the
  KrustyBike near-miss sample header.

## How to read a near miss

- Retail has the zero register and VC6 here does not: look for zero stores
  the original made on several paths (per-branch clears, per-path
  resets) that VC6 merged or that the reconstruction hoisted into one
  place. Spelling of the tests (`!= 0`, `||` order, nested `if`s) does not
  matter.
- VC6 here has it and retail does not (BackgroundImage `0x004049d0`,
  EcoSystem `0x00457480`, input `0x004c3100`): count the displaced stores
  of that constant and check whether the register is already pushed for
  other work. Two counted stores are enough when it is. The
  reconstruction probably writes one more counted store, or one more
  register-candidate variable initialised to the constant, than the
  original did.

The probes are throwaway units compiled with `tools/compile.py --compiler
vc6 --profile vc6_o2_mt` and disassembled with the repository's COFF tools.
- Bike slot 38 `0x00405190`: retail clears `ebx` at entry for the zero stores
  (hasBody, notifyOther), the `crashState` compare, the case 0 store and the
  0x3e8 `push`, and later reuses `ebx` for `otherVel`.
  - At the join of the no-body cases retail reloads `ebp` (ctx) and `ebx`
    (otherVel) from `[esp + 0x1c]`, which is out's home. That is what VC6 emits
    for register variables left uninitialised on those paths, so retail's
    `otherVel` probably has no initialiser.
  - `Vec3* otherVel;` reproduces the reloads and the shared home (strict 280 to
    1127 of 2874). But the zero web then gets scratch `eax`, the switch index
    moves to `ecx`, and the case 0 compare becomes `test` (aligned 689 to 642).
    With `otherVel = 0` the zero gets `ebp` beside `otherVel`'s `ebx`.
  - Inert under the uninitialised form: every order of the six scalar
    declarations, passing hasBody or other as the 0x3e8 argument, and redundant
    `hasBody = 0` stores in case 0x65 or after the slot 3 call. What gives retail's
    zero web a callee-saved register is still open.
