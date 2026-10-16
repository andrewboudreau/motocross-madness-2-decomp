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

## How to read a near miss

- Retail has the zero register and VC6 here does not: look for zero stores
  the original made on several paths (per-branch clears, per-path
  resets) that VC6 merged or that the reconstruction hoisted into one
  place. Spelling of the tests (`!= 0`, `||` order, nested `if`s) does not
  matter.
- VC6 here has it and retail does not (BackgroundImage `0x004049d0`,
  input `0x004c3100`): the reconstruction probably writes more zero stores
  than the original did, for example a clear repeated per branch that was
  a single statement.

The probes are throwaway units compiled with `tools/compile.py --compiler
vc6 --profile vc6_o2_mt` and disassembled with the repository's COFF tools.
