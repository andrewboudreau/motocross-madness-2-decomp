# VC6 SP3 delayed callee-saved pushes

Many retail functions start with a guard that returns before any callee-saved
register is pushed:

```
mov eax, [esp + 4]
sub esp, 0x24
test eax, eax
jne body
xor eax, eax
add esp, 0x24
ret
body:
mov eax, [esp + 0x2c]
push ebx
push ebp
...
```

If VC6 compiles the reconstruction with the pushes ahead of the guard
(`test eax, eax; push ebx; push esi; jne ...`, with pops on the guard's return
path), every argument offset after that point moves and most of the function
mismatches. The deciding factor is the number of other `return` statements,
not how the guard is written. Measured with probe units under `vc6_o2_mt`
(`tools/compile.py --compiler vc6 --profile vc6_o2_mt`, disassembled with the
repository's COFF tools).

## What decides it

- **A guard followed by one exit is delayed.** `if (!a1) return 0;`, then
  straight-line work or a loop, then a single `return 1;` gives the shape
  above, with all callee-saved pushes after the guard.
- **Another `return` puts its registers at entry.** If a later branch
  returns on its own (`if (mode == 1 && count == 1) { ...; return 1; }`
  before the loop), VC6 pushes the registers that branch uses before the
  guard and pops them on the guard's path. Registers used only on the path
  to the final `return` are still pushed late. When the branch uses ebx and
  esi, those two move to the entry; when it uses only ebx (the branch
  reduced to one call), only ebx does. In both cases ebp and edi stay at
  the loop.
- **Equal return values do not help.** A branch returning 2 behaves like one
  returning 1, so two `return 1` statements are not treated as one exit.
- **Spelling of the guard and the branch does not matter.** `a1 == 0`,
  nested `if (mode == 1) if (count == 1)`, `mode == 1` alone and `*pts`
  for `pts[0]` all keep the pushes at entry.
- **The `if`/`else` form restores it.** Writing the branch and the loop as
  `if (...) { ... } else { loop }` followed by one `return 1;` gives
  retail's delayed pushes.

## How to read a near miss

- If retail delays its pushes past a guard and the reconstruction does not,
  look for an early `return` (or a `return` inside a loop or branch) that
  the original wrote as `if`/`else` or as a `break` to a shared exit.
  CollisionPoint `0x0043ad80` went from 47 to 597 of 615 bytes this way;
  Pixtrans `0x004cee30` gets retail's `push ebp` after its palette checks
  once its two loops share one `return 1`.
  ManagedTextureGroup `0x0050ef70` returned 1 early when TestKey was off
  and again when no page was selected; nesting the body under
  `if (testKey)` and `if (page)` with one trailing `return 1;` moved its
  masked score from 93 to 951 of 1678 bytes (it is exact now).
- If retail pushes before the guard and the reconstruction delays the pushes,
  the original probably had more exits that the reconstruction folded into
  the final one. Parser `0x004b87f0` (33 of 408 bytes) became exact by
  writing each `break` out of its search loop as `return S_OK;`. Retail still
  has one exit block, because VC6 merges the identical returns after deciding
  where to push.
- If retail saves a different register at entry than the reconstruction
  (ebx at entry and edi late, against edi at entry), look for two locals
  that VC6 put in one register because their lifetimes meet but do not
  overlap. DebugOverlay `0x00447de0` copied a parameter into the row `y`
  and then set the counter `i = 0`; VC6 gave both edi. Setting `i` first
  keeps the parameter in ebx (which then holds the loop's constant colour)
  and delays `push edi`, making the function exact.
