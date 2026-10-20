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
  - No form found yet. Variants are in `work/s14/cz/gd/`.
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
